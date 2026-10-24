#include "core/loader.h"

#include "office/package.h"

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QMetaObject>
#include <QThread>

#include "core/threadtime.h"

#include <algorithm>
#include <functional>
#include <mutex>
#include <vector>

namespace jp::crunch {

using namespace jp::office;
using namespace jp::office::sheet;

namespace {
QString tr(const char *text) { return QCoreApplication::translate("jp::crunch::Loader", text); }
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

// Workers that were stopped and are still going: waited for when the program ends (a thread must not be destroyed while it runs).
struct Janitor {
    std::mutex mutex;
    std::vector<std::shared_ptr<QThread>> threads;
    void add(const std::shared_ptr<QThread> &t)
    {
        std::lock_guard<std::mutex> lock(mutex);
        threads.erase(std::remove_if(threads.begin(), threads.end(), [](const std::shared_ptr<QThread> &x) { return x->isFinished(); }), threads.end());
        threads.push_back(t);
    }
    ~Janitor()
    {
        for (const auto &t : threads) t->wait();
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

void WorkbookLoader::cancel()
{
    if (m_state) {
        m_state->cancel = true;
        m_state->stopCalc = true;
    }
    m_thread.reset();   // (the janitor keeps it until it is over)
}

void WorkbookLoader::stopCalculation()
{
    if (m_state) m_state->stopCalc = true;
}

void WorkbookLoader::setPace(int micros)
{
    m_pace = micros;
    if (m_state) m_state->pace = micros;
}

void WorkbookLoader::startThread(std::function<void()> work)
{
    m_thread = std::shared_ptr<QThread>(QThread::create([work] {
        ++g_workers;
        work();
        --g_workers;
    }));
    janitor().add(m_thread);
    m_thread->start();
}

void WorkbookLoader::start(const QString &path)
{
    cancel();
    m_state = std::make_shared<State>();
    m_state->pace = m_pace;
    m_result = std::make_shared<LoadedBook>();
    m_result->path = path;
    const std::shared_ptr<LoadedBook> result = m_result;
    const std::shared_ptr<State> state = m_state;
    const QPointer<WorkbookLoader> self(this);
    startThread([path, result, state, self] { run(path, result, state, self); });
}

bool WorkbookLoader::recalculate()
{
    if (!m_result || !m_state || !m_result->calc || m_state->cancel.load()) return false;
    m_state->stopCalc = false;
    m_state->redo = true;
    // (a worker that is still calculating takes it at its next slice; one that has just finished, or has finished, does not, and a new one is begun for it)
    if (!m_state->calcActive.load() && m_state->redo.exchange(false)) {
        const std::shared_ptr<LoadedBook> result = m_result;
        const std::shared_ptr<State> state = m_state;
        const QPointer<WorkbookLoader> self(this);
        startThread([result, state, self] { calculate(result, state, self, true); });
    }
    return true;
}

bool loadFirstScreen(const QString &path, LoadedBook *out, const std::function<bool()> &cancelled, QString *why)
{
    QElapsedTimer total;
    total.start();
    out->path = path;
    auto book = std::make_shared<Workbook>();
    if (!book->openFile(path, why)) return false;   // (either kind: the package of XML parts, or the older binary form)
    out->book = book;
    out->notRead = book->skippedFeatures();
    const int shownSheet = firstSheetToShow(book->info());
    if (shownSheet < 0) {
        *why = tr("The workbook has no sheets.");
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

bool loadWhole(LoadedBook *out, const std::function<bool()> &cancelled, const std::function<void(int, qint64)> &progress, const std::atomic<bool> *cancelFlag, const std::atomic<int> *pace)
{
    QElapsedTimer total;
    total.start();
    CalcOptions options;
    options.keepApart = true;                  // what the engine cannot work out keeps the result the file stored
    options.workOutWhatHasNoResult = true;     // and a formula the file stored no result for is worked out (a program that wrote the file may not have)
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
    if (!whole) out->problem = tr("Some of the workbook could not be read whole: %1.").arg(calc->notWhole().join(QStringLiteral(", ")));
    // the grid's geometry of each sheet
    out->geometry.assign(size_t(out->book->info().sheets.size()), nullptr);
    for (int i = 0; i < out->book->info().sheets.size(); ++i) {
        if (cancelled()) return false;
        if (out->book->info().sheets[i].kind != SheetKind::Chartsheet) out->geometry[size_t(i)] = jp::grid::GridSheet::makeGeometry(calc->sheet(i), out->book->styles());
    }
    out->calc = std::move(calc);
    out->loadMs = out->firstScreenMs + total.elapsed();
    return true;
}

QString fileKind(const QString &path)
{
    static const QStringList kinds = {QStringLiteral("xlsx"), QStringLiteral("xlsm"), QStringLiteral("xls")};
    const QString ending = QFileInfo(path).suffix().toLower();
    return kinds.contains(ending) ? ending : QStringLiteral("other");
}

void WorkbookLoader::calculate(const std::shared_ptr<LoadedBook> &result, const std::shared_ptr<State> &state, const QPointer<WorkbookLoader> &self, bool invalidateFirst)
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
    state->calcActive = true;
    result->calcThread = quintptr(QThread::currentThreadId());
    bool again = invalidateFirst;
    for (;;) {
        std::vector<jp::grid::ValueStore::Item> items;
        calc->setCancel(&state->stopCalc);
        calc->setResultSink([&items](int sheet, quint32 row, quint32 col, const jp::calc::Value &v) {
            jp::grid::ValueStore::Item it;
            it.sheet = sheet;
            it.row = row;
            it.col = col;
            it.value = v;
            items.push_back(std::move(it));
        });
        if (again) calc->invalidateAll();
        again = false;
        QElapsedTimer sinceSent;
        sinceSent.start();
        auto flush = [&](bool last, bool stopped) {
            auto batch = std::make_shared<CalcBatch>();
            batch->items.swap(items);
            const qint64 total = std::max<qint64>(1, calc->formulaCount());
            batch->percent = last && !stopped ? 100 : int(std::max<qint64>(0, 100 - 100 * calc->dirtyCount() / total));
            batch->last = last;
            batch->stopped = stopped;
            batch->workerCpuMicros = jp::threadCpuMicros();
            sendBatch(batch);
            sinceSent.restart();
            // (the window is given time to take what it has: a worker that is far ahead of it only makes more for it to hold)
            while (!last && state->pending.load() >= kMaxBatchesWaiting && !state->stopCalc.load()) QThread::usleep(500);
        };
        bool stopped = false;
        for (;;) {
            const Calculation::Progress p = calc->recalculate(0, nullptr, kSliceMicros);
            if (const int pace = state->pace.load()) QThread::usleep(pace);
            if (p.cancelled || state->stopCalc.load()) {
                stopped = true;
                break;
            }
            if (state->redo.exchange(false)) {
                calc->invalidateAll();
                continue;
            }
            if (p.finished) break;
            if (items.size() >= size_t(kBatchValues) || sinceSent.elapsed() >= kBatchMillis) flush(false, false);
        }
        calc->setResultSink(nullptr);
        calc->setCancel(nullptr);
        flush(true, stopped);
        state->calcActive = false;
        // (asked to work everything out again as it ended: it does, on this worker)
        if (!stopped && state->redo.exchange(false) && !state->cancel.load()) {
            state->calcActive = true;
            again = true;
            continue;
        }
        break;
    }
}

void WorkbookLoader::run(const QString &path, std::shared_ptr<LoadedBook> result, std::shared_ptr<State> state, QPointer<WorkbookLoader> self)
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
    if (!loadFirstScreen(path, result.get(), cancelled, &why)) {
        send([self, why] { if (self) Q_EMIT self->finished(false, why); });
        return;
    }
    if (cancelled()) return;
    send([self] { if (self) Q_EMIT self->firstScreen(); });
    if (!loadWhole(result.get(), cancelled, [&](int sheet, qint64 cells) { send([self, sheet, cells] { if (self) Q_EMIT self->progress(sheet, cells); }); }, &state->cancel, &state->pace)) return;
    state->calcActive = true;   // (a request to work everything out again, from here on, is for this worker to take)
    send([self] { if (self) Q_EMIT self->finished(true, QString()); });
    // and the formulas are worked out, here, as the window shows what it has
    calculate(result, state, self, false);
}

} // namespace jp::crunch
