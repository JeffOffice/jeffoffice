#include "core/loader.h"

#include "office/package.h"

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QMetaObject>
#include <QThread>

#include "core/threadtime.h"

#include <algorithm>
#include <chrono>
#include <functional>
#include <mutex>
#include <vector>

namespace jp::crunch {

using namespace jp::office;
using namespace jp::office::sheet;

namespace {
}

int firstSheetToShow(const WorkbookInfo &info)
{
    auto shown = [&](int i) { return i >= 0 && i < info.sheets.size() && info.sheets[i].state == SheetState::Visible && info.sheets[i].kind == SheetKind::Worksheet; };
    if (shown(info.activeTab)) return info.activeTab;
    for (int i = 0; i < info.sheets.size(); ++i)
        if (shown(i)) return i;
    for (int i = 0; i < info.sheets.size(); ++i)
        if (info.sheets[i].state == SheetState::Visible) return i;
    return info.sheets.isEmpty() ? -1 : 0;
}

namespace {

// Workers that were stopped and are still going: waited for when the program ends (a thread must not be destroyed while it runs). A worker that was never stopped (its window was never
// closed: a program that ends with a window open, as one that saves a picture of it and quits does) is told to stop first, or the program would wait for it for ever.
struct Janitor {
    struct Worker {
        std::shared_ptr<QThread> thread;
        std::function<void()> stop;
    };
    std::mutex mutex;
    std::vector<Worker> workers;
    void add(const std::shared_ptr<QThread> &t, std::function<void()> stop)
    {
        std::lock_guard<std::mutex> lock(mutex);
        workers.erase(std::remove_if(workers.begin(), workers.end(), [](const Worker &x) { return x.thread->isFinished(); }), workers.end());
        workers.push_back({t, std::move(stop)});
    }
    ~Janitor()
    {
        for (const Worker &w : workers)
            if (w.stop) w.stop();
        for (const Worker &w : workers) w.thread->wait();
    }
};

Janitor &janitor()
{
    static Janitor j;
    return j;
}

} // namespace

WorkbookLoader::WorkbookLoader(QObject *parent) : QObject(parent) {}

WorkbookLoader::~WorkbookLoader() { cancel(); }

bool WorkbookLoader::isRunning() const { return m_thread && m_thread->isRunning(); }

namespace {
std::atomic<int> g_workers{0};
}

int WorkbookLoader::workersRunning() { return g_workers.load(); }

void WorkbookLoader::stopAll(State &state)
{
    state.cancel = true;
    state.stopCalc = true;
    state.halt = true;
    state.wake.notify_all();
}

void WorkbookLoader::cancel()
{
    if (m_state) stopAll(*m_state);
    m_thread.reset();   // (the janitor keeps it until it is over)
}

void WorkbookLoader::stopCalculation()
{
    if (!m_state) return;
    m_state->stopCalc = true;
    m_state->updateHalt();
    m_state->wake.notify_all();
}

void WorkbookLoader::setPace(int micros)
{
    m_pace = micros;
    if (m_state) m_state->pace = micros;
}

void WorkbookLoader::startThread(std::function<void()> work, const std::shared_ptr<State> &state)
{
    m_thread = std::shared_ptr<QThread>(QThread::create([work] {
        ++g_workers;
        work();
        --g_workers;
    }));
    janitor().add(m_thread, [state] { stopAll(*state); });
    m_thread->start();
}

void WorkbookLoader::start(const QString &path, const std::shared_ptr<const jp::office::sheet::CsvImport> &csv) { begin(path, csv, QByteArray()); }

void WorkbookLoader::startBlank() { begin(QString(), nullptr, blankWorkbookBytes()); }

void WorkbookLoader::begin(const QString &path, const std::shared_ptr<const jp::office::sheet::CsvImport> &csv, const QByteArray &blank)
{
    cancel();
    m_state = std::make_shared<State>();
    m_state->manualAllowed = m_manualAllowed;
    m_state->idFrame = m_idFrame;
    m_state->pace = m_pace;
    m_result = std::make_shared<LoadedBook>();
    m_result->path = path;
    const std::shared_ptr<LoadedBook> result = m_result;
    const std::shared_ptr<State> state = m_state;
    const QPointer<WorkbookLoader> self(this);
    startThread([path, csv, blank, result, state, self] { run(path, csv, blank, result, state, self); }, state);
}

bool WorkbookLoader::recalculate()
{
    if (!m_result || !m_state || !m_result->calc || m_state->cancel.load() || !m_state->serving.load()) return false;
    m_state->stopCalc = false;
    m_state->updateHalt();
    m_state->redo = true;
    m_state->urgent = true;
    ++m_state->changes;
    m_state->wake.notify_all();
    return true;
}

bool WorkbookLoader::recalculateSheet(int sheet)
{
    if (!m_result || !m_state || !m_result->calc || m_state->cancel.load() || !m_state->serving.load() || sheet < 0) return false;
    m_state->stopCalc = false;
    m_state->updateHalt();
    m_state->sheetCalc = sheet;
    m_state->urgent = true;
    ++m_state->changes;
    m_state->wake.notify_all();
    return true;
}

void WorkbookLoader::setManual(bool manual)
{
    if (!m_state) return;
    m_state->manual = manual;
    m_state->wake.notify_all();
}

qint64 WorkbookLoader::changesMade() const { return m_state ? m_state->changes.load() : 0; }

qint64 WorkbookLoader::holdsTaken() const { return m_state ? m_state->holdsMade.load() : 0; }

WorkbookLoader::Hold::Hold(std::shared_ptr<State> state, jp::office::sheet::Calculation *calc) : m_state(std::move(state)), m_calc(calc)
{
    ++m_state->holds;
    ++m_state->holdsMade;
    m_state->updateHalt();
    m_state->model.lock();
}

WorkbookLoader::Hold::~Hold()
{
    if (m_changed) {
        ++m_state->changes;
        m_state->urgent = true;
        // an edit made after a stop (Esc) is worked out: the stop was of the calculation that was going
        m_state->stopCalc = false;
    }
    --m_state->holds;
    m_state->updateHalt();
    m_state->model.unlock();
    m_state->wake.notify_all();
}

bool WorkbookLoader::canHold() const { return m_result && m_state && m_result->calc && !m_state->cancel.load() && m_state->serving.load(); }

void WorkbookLoader::refuseHolds(int count)
{
    if (m_state) m_state->refuse = count;
}

std::unique_ptr<WorkbookLoader::Hold> WorkbookLoader::hold()
{
    if (!m_result || !m_state || !m_result->calc || m_state->cancel.load() || !m_state->serving.load()) return nullptr;
    if (m_state->refuse.load() > 0) {
        --m_state->refuse;
        return nullptr;
    }
    return std::unique_ptr<Hold>(new Hold(m_state, m_result->calc.get()));
}

bool loadFirstScreen(const QString &path, LoadedBook *out, const std::function<bool()> &cancelled, QString *why, const std::shared_ptr<const jp::office::sheet::CsvImport> &csv, const QByteArray &blank)
{
    QElapsedTimer total;
    total.start();
    out->path = path;
    auto book = std::make_shared<Workbook>();
    // (the read can be stopped while a large document's styles and settings are read, and while its sheets are)
    Limits limits;
    limits.cancelled = cancelled;
    limits.csv = csv;
    if (!blank.isEmpty()) {
        office::Package package;
        if (!package.readLazy(blank, why) || !book->open(package, why, limits)) return false;
    } else if (!book->openFile(path, why, limits)) {
        return false;   // (any kind: the package of XML parts, the older binary form, or another format)
    }
    book->tables();   // (read now: the worker looks at the tables as it works formulas out, and the window changes them, neither of which may be the first to read them)
    out->book = book;
    out->notRead = book->skippedFeatures();
    const int shownSheet = firstSheetToShow(book->info());
    if (shownSheet < 0) {
        *why = QCoreApplication::translate("jp::crunch::Loader", "The workbook has no sheets.");
        return false;
    }
    // the rows the sheet's view shows (and its frozen rows), from where it begins: not the first rows of the sheet, which it may be scrolled far from
    auto preview = std::make_shared<Sheet>();
    book->readSheet(shownSheet, preview.get(), [&cancelled](qint64) { return !cancelled(); }, -1, kFirstScreenRows);
    out->preview = preview;
    out->previewSheet = shownSheet;
    out->previewGeometry = jp::grid::GridSheet::makeGeometry(*preview, book->styles());
    out->firstScreenMs = total.elapsed();
    return true;
}

bool loadWhole(LoadedBook *out, const std::function<bool()> &cancelled, const std::function<void(int, qint64)> &progress, const std::atomic<bool> *cancelFlag, const std::atomic<int> *pace, int idFrame)
{
    QElapsedTimer total;
    total.start();
    CalcOptions options;
    options.keepApart = true;                  // what the engine cannot work out keeps the result the file stored
    options.workOutWhatHasNoResult = true;     // and a formula the file stored no result for is worked out (a program that wrote the file may not have)
    options.goRoundCircles = true;             // and a circle is gone round when the workbook is set to
    options.workOutTables = true;              // and the columns of tables are worked out (a formula that names them is no longer kept as the file has it)
    if (idFrame > 0) options.idFrame = true;
    if (idFrame < 0) options.idFrameOff = true;
    auto calc = std::make_unique<Calculation>(*out->book, options);
    calc->setCancel(cancelFlag);
    QElapsedTimer sinceTold;
    sinceTold.start();
    calc->setProgress([&](int sheet, qint64 cells) {
        if (cancelled()) return false;
        out->cellsRead = cells;
        if (pace && pace->load() > 0) QThread::usleep(ulong(pace->load()));
        if (progress && (cells == 0 || sinceTold.elapsed() > 100)) {
            sinceTold.restart();
            progress(sheet, cells);
        }
        return true;
    });
    const bool whole = calc->load();
    calc->setProgress(nullptr);
    calc->setCancel(nullptr);
    if (cancelled()) return false;
    if (!whole) out->problem = QCoreApplication::translate("jp::crunch::Loader", "Some of the workbook could not be read whole: %1.").arg(calc->notWhole().join(QStringLiteral(", ")));
    // the grid's geometry of each sheet
    out->geometry.assign(size_t(out->book->info().sheets.size()), nullptr);
    out->notes.assign(size_t(out->book->info().sheets.size()), {});
    out->notesTruncated.assign(size_t(out->book->info().sheets.size()), false);
    jp::office::sheet::NoteReadContext notesContext;   // (a comments part that more sheets than one name is read once, and the characters are counted for the workbook)
    for (int i = 0; i < out->book->info().sheets.size(); ++i) {
        if (cancelled()) return false;
        if (out->book->info().sheets[i].kind != SheetKind::Chartsheet) out->geometry[size_t(i)] = jp::grid::GridSheet::makeGeometry(calc->sheet(i), out->book->styles());
        if (out->book->info().sheets[i].kind == SheetKind::Worksheet) {
            bool truncated = false;
            out->notes[size_t(i)] = readSheetNoteInfos(*out->book, i, &truncated, jp::office::sheet::kMaxModelNotes, &notesContext);
            out->notesTruncated[size_t(i)] = truncated;
        }
    }
    out->calc = std::move(calc);
    out->loadMs = out->firstScreenMs + total.elapsed();
    return true;
}

QByteArray blankWorkbookBytes()
{
    using office::PackageWriter;
    const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
    const QString main = QStringLiteral("http://schemas.openxmlformats.org/spreadsheetml/2006/main");
    const QByteArray head = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
    PackageWriter w;
    w.addPart(QStringLiteral("/xl/workbook.xml"), QByteArray(), QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"));
    w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/xl/workbook.xml"));
    w.addPart(QStringLiteral("/xl/worksheets/sheet1.xml"),
              head + "<worksheet xmlns=\"" + main.toUtf8() + "\"><sheetViews><sheetView workbookViewId=\"0\"/></sheetViews><sheetFormatPr defaultRowHeight=\"15\"/><sheetData/></worksheet>",
              QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"));
    const QString sheetId = w.addRelationship(QStringLiteral("/xl/workbook.xml"), rel + QStringLiteral("worksheet"), QStringLiteral("/xl/worksheets/sheet1.xml"));
    w.addPart(QStringLiteral("/xl/styles.xml"),
              head + "<styleSheet xmlns=\"" + main.toUtf8() + "\"><fonts count=\"1\"><font><sz val=\"11\"/><name val=\"Calibri\"/><family val=\"2\"/></font></fonts>"
                     "<fills count=\"2\"><fill><patternFill patternType=\"none\"/></fill><fill><patternFill patternType=\"gray125\"/></fill></fills>"
                     "<borders count=\"1\"><border><left/><right/><top/><bottom/><diagonal/></border></borders>"
                     "<cellStyleXfs count=\"1\"><xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\"/></cellStyleXfs>"
                     "<cellXfs count=\"1\"><xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"0\"/></cellXfs>"
                     "<cellStyles count=\"1\"><cellStyle name=\"Normal\" xfId=\"0\" builtinId=\"0\"/></cellStyles></styleSheet>",
              QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml"));
    w.addRelationship(QStringLiteral("/xl/workbook.xml"), rel + QStringLiteral("styles"), QStringLiteral("/xl/styles.xml"));
    w.addPart(QStringLiteral("/xl/workbook.xml"),
              head + "<workbook xmlns=\"" + main.toUtf8() + "\" xmlns:r=\"" + rel.chopped(1).toUtf8() + "\"><bookViews><workbookView/></bookViews><sheets><sheet name=\"Sheet1\" sheetId=\"1\" r:id=\""
                     + sheetId.toUtf8() + "\"/></sheets><calcPr calcId=\"191029\"/></workbook>",
              QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"));
    return w.finish();
}

QString fileKind(const QString &path)
{
    static const QStringList kinds = {QStringLiteral("xlsx"), QStringLiteral("xlsm"), QStringLiteral("xltx"), QStringLiteral("xltm"), QStringLiteral("xlam"), QStringLiteral("xls"), QStringLiteral("ods"), QStringLiteral("fods"), QStringLiteral("csv"), QStringLiteral("tsv"), QStringLiteral("txt")};
    const QString ending = QFileInfo(path).suffix().toLower();
    return kinds.contains(ending) ? ending : QStringLiteral("other");
}

// The worker, once the workbook is read whole: it works the formulas out for the window, a slice at a time, and tells it the values in batches; when there is nothing to work out it waits,
// and works out what the window changes (a hold let go with a change) and what F9 asks for, until it is cancelled.
void WorkbookLoader::serve(const std::shared_ptr<LoadedBook> &result, const std::shared_ptr<State> &state, const QPointer<WorkbookLoader> &self)
{
    Calculation *calc = result->calc.get();
    if (!calc) return;
    auto sendBatch = [&](const std::shared_ptr<CalcBatch> &batch) {
        if (state->cancel.load()) return;
        ++state->pending;
        QMetaObject::invokeMethod(QCoreApplication::instance(), [state, self, batch] {
            if (!state->cancel.load() && self) Q_EMIT self->calcBatch(batch);
            --state->pending;
        }, Qt::QueuedConnection);
    };
    result->calcThread = quintptr(QThread::currentThreadId());
    std::vector<jp::grid::ValueStore::Item> items;
    calc->setCancel(&state->halt);
    calc->setResultSink([&items, calc](int sheet, quint32 rowId, quint32 colId, const jp::calc::Value &v) {
        jp::grid::ValueStore::Item it;
        it.sheet = sheet;
        it.row = rowId;
        it.col = colId;
        it.epoch = calc->structEpoch(sheet);   // (the cell is named by ids, which an edit of rows or columns does not change: a value in flight lands where the cell is; an edit that moves cells stamps them)
        it.value = v;
        items.push_back(std::move(it));
    });
    QElapsedTimer sinceSent;
    sinceSent.start();
    std::unique_lock<std::mutex> lock(state->model);
    bool everCircular = false, lastDue = false;
    qint64 lastCircular = 0;
    std::vector<Calculation::CircularCell> lastCells;
    // (the values are sent without the model: sending can wait for the window to take what it has)
    auto flush = [&](bool last, bool stopped, qint64 epoch) {
        const qint64 total = std::max<qint64>(1, calc->formulaCount());
        const qint64 dirty = calc->dirtyCount();
        std::vector<quint32> epochs;   // (read with the model: the window changes the sheets and their epochs when it has it)
        for (int i = 0; i < calc->sheetCount(); ++i) epochs.push_back(calc->structEpoch(i));
        lock.unlock();
        auto batch = std::make_shared<CalcBatch>();
        batch->items.swap(items);
        batch->percent = last && !stopped ? 100 : int(std::max<qint64>(0, 100 - 100 * dirty / total));
        batch->last = last;
        batch->stopped = stopped;
        batch->epoch = epoch;
        batch->workerCpuMicros = jp::threadCpuMicros();
        batch->epochs = std::move(epochs);
        batch->due = last && lastDue;
        batch->circular = last ? lastCircular : 0;
        if (last) batch->circleCells = lastCells;
        sendBatch(batch);
        sinceSent.restart();
        state->urgent = false;
        // (the window is given time to take what it has: a worker that is far ahead of it only makes more for it to hold)
        while (!last && state->pending.load() >= kMaxBatchesWaiting && !state->stopCalc.load() && !state->cancel.load() && state->holds.load() == 0) QThread::usleep(500);
    };
    bool first = true;   // (the first pass is made even when nothing is to be worked out, so that the window is told it is over)
    qint64 toldEpoch = -1, lastCalcEpoch = -1, circles = 0;   // (by hand: the changes the window has been told of; and the formulas found in circles since the pass began)
    for (;;) {
        // wait for something to work out (the wait ends at least every so often, in case a request came between looking and waiting)
        if (!first) {
            result->calcIdle = true;
            state->wake.wait_for(lock, std::chrono::milliseconds(50), [&] {
                return state->cancel.load() || state->redo.load() || state->sheetCalc.load() >= 0 || (state->manual.load() ? state->changes.load() != toldEpoch : !state->stopCalc.load() && calc->isDirty());
            });
        }
        if (state->cancel.load()) break;
        bool asked = false, askedAll = false;
        if (state->redo.exchange(false)) {
            calc->invalidateAll();
            asked = askedAll = true;
        }
        if (const int sheet = state->sheetCalc.exchange(-1); sheet >= 0) {
            calc->invalidateSheet(sheet);
            asked = true;
        }
        // (by hand: nothing is worked out but what was asked for; the window is told, once for each change, whether formulas wait)
        const bool paused = state->manual.load() && !asked;
        if (paused && !first && state->changes.load() == toldEpoch) continue;
        if (!paused && !first && (state->stopCalc.load() || !calc->isDirty())) continue;
        first = false;
        result->calcIdle = false;
        bool stopped = false;
        qint64 epoch = 0;
        if (paused) {
            // (by hand: a formula typed or edited is worked out at once, with what it needs that is not worked out; what reads it waits for the key)
            epoch = state->changes.load();
            for (;;) {
                const Calculation::Progress p = calc->recalculateFresh(0, nullptr, kSliceMicros);
                if (p.finished || state->cancel.load()) break;
                if (p.cancelled) calc->settle();
                lock.unlock();
                while (state->holds.load() > 0 && !state->cancel.load()) QThread::usleep(20);
                lock.lock();
            }
        } else for (;;) {
            const Calculation::Progress p = calc->recalculate(0, nullptr, kSliceMicros);
            circles += p.circular;
            if (p.cancelled) {
                if (state->stopCalc.load() || state->cancel.load()) {
                    stopped = true;
                    break;
                }
                // a hold is wanted: what the stop left to put right is put right, the model is let go, and taken again when the window has done
                calc->settle();
                lock.unlock();
                while (state->holds.load() > 0 && !state->cancel.load()) QThread::usleep(20);
                lock.lock();
                continue;
            }
            if (state->redo.exchange(false)) {
                calc->invalidateAll();
                continue;
            }
            if (p.finished) {
                epoch = state->changes.load();
                break;
            }
            // between slices: what has been worked out is sent when there is enough of it (or the first of an edit is there), and the model is let go for a moment
            const bool send = items.size() >= size_t(kBatchValues) || sinceSent.elapsed() >= kBatchMillis || (state->urgent.load() && !items.empty());
            if (send) {
                flush(false, false, 0);
            } else {
                lock.unlock();
            }
            if (const int pace = state->pace.load()) QThread::usleep(pace);
            while (state->holds.load() > 0 && !state->cancel.load()) QThread::usleep(20);
            lock.lock();
            if (state->stopCalc.load() || state->cancel.load()) {
                stopped = true;
                break;
            }
        }
        if (state->cancel.load()) break;
        toldEpoch = std::max(toldEpoch, epoch);
        if (circles > 0) everCircular = true;
        // (formulas wait when the workbook is worked out by hand and something was changed since all of it was last worked out; one sheet worked out leaves it so while anything else waits)
        if (paused || askedAll || !state->manual.load()) {
            if (!paused) lastCalcEpoch = epoch;
            else if (lastCalcEpoch < 0) lastCalcEpoch = epoch;
        }
        lastDue = state->manual.load() && state->changes.load() > lastCalcEpoch && !(asked && !askedAll && !calc->isDirty());
        lastCells.clear();
        lastCircular = everCircular && !stopped ? calc->circularCells(&lastCells, 10000) : 0;
        flush(true, stopped, epoch);
        result->calcIdle = true;
        while (state->holds.load() > 0 && !state->cancel.load()) QThread::usleep(20);
        lock.lock();
    }
    calc->setResultSink(nullptr);
    calc->setCancel(nullptr);
}

void WorkbookLoader::run(const QString &path, const std::shared_ptr<const jp::office::sheet::CsvImport> &csv, const QByteArray &blank, std::shared_ptr<LoadedBook> result, std::shared_ptr<State> state, QPointer<WorkbookLoader> self)
{
    // A signal goes to the window only if the read has not been stopped (and the loader is still there), and is sent from the window's own thread.
    auto send = [state, self](const std::function<void()> &fn) {
        if (state->cancel.load()) return;
        QMetaObject::invokeMethod(QCoreApplication::instance(), [state, self, fn] {
            if (!state->cancel.load() && self) fn();
        }, Qt::QueuedConnection);
    };
    auto cancelled = [state] { return state->cancel.load(); };
    struct Done {
        std::shared_ptr<LoadedBook> book;
        ~Done() { book->workerEnded = true; }
    } done{result};
    QString why;
    if (!loadFirstScreen(path, result.get(), cancelled, &why, csv, blank)) {
        send([self, why] { if (self) Q_EMIT self->finished(false, why); });
        return;
    }
    if (cancelled()) return;
    send([self] { if (self) Q_EMIT self->firstScreen(); });
    if (!loadWhole(result.get(), cancelled, [&](int sheet, qint64 cells) { send([self, sheet, cells] { if (self) Q_EMIT self->progress(sheet, cells); }); }, &state->cancel, &state->pace, state->idFrame)) return;
    // (a workbook that is worked out by hand is shown with the results its file holds, unless the file asks for them to be worked out when it is opened)
    state->manual = state->manualAllowed.load() && result->book && result->book->info().calcMode == jp::office::sheet::CalcMode::Manual && !result->book->info().fullCalcOnLoad && !result->book->info().calcUnfinished;
    state->serving = true;
    send([self] { if (self) Q_EMIT self->finished(true, QString()); });
    // and the formulas are worked out, here, as the window shows what it has; then the worker stays, to work out what the window changes
    serve(result, state, self);
}

} // namespace jp::crunch
