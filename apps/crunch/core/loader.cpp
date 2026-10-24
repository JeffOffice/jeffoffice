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
        m_state->halt = true;
        m_state->wake.notify_all();
    }
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
    if (!m_result || !m_state || !m_result->calc || m_state->cancel.load() || !m_state->serving.load()) return false;
    m_state->stopCalc = false;
    m_state->updateHalt();
    m_state->redo = true;
    m_state->urgent = true;
    ++m_state->changes;
    m_state->wake.notify_all();
    return true;
}

qint64 WorkbookLoader::changesMade() const { return m_state ? m_state->changes.load() : 0; }

WorkbookLoader::Hold::Hold(std::shared_ptr<State> state, jp::office::sheet::Calculation *calc) : m_state(std::move(state)), m_calc(calc)
{
    ++m_state->holds;
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

std::unique_ptr<WorkbookLoader::Hold> WorkbookLoader::hold()
{
    if (!m_result || !m_state || !m_result->calc || m_state->cancel.load() || !m_state->serving.load()) return nullptr;
    return std::unique_ptr<Hold>(new Hold(m_state, m_result->calc.get()));
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
    if (!whole) out->problem = QCoreApplication::translate("jp::crunch::Loader", "Some of the workbook could not be read whole: %1.").arg(calc->notWhole().join(QStringLiteral(", ")));
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
    calc->setResultSink([&items](int sheet, quint32 row, quint32 col, const jp::calc::Value &v) {
        jp::grid::ValueStore::Item it;
        it.sheet = sheet;
        it.row = row;
        it.col = col;
        it.value = v;
        items.push_back(std::move(it));
    });
    QElapsedTimer sinceSent;
    sinceSent.start();
    std::unique_lock<std::mutex> lock(state->model);
    // (the values are sent without the model: sending can wait for the window to take what it has)
    auto flush = [&](bool last, bool stopped, qint64 epoch) {
        const qint64 total = std::max<qint64>(1, calc->formulaCount());
        const qint64 dirty = calc->dirtyCount();
        lock.unlock();
        auto batch = std::make_shared<CalcBatch>();
        batch->items.swap(items);
        batch->percent = last && !stopped ? 100 : int(std::max<qint64>(0, 100 - 100 * dirty / total));
        batch->last = last;
        batch->stopped = stopped;
        batch->epoch = epoch;
        batch->workerCpuMicros = jp::threadCpuMicros();
        sendBatch(batch);
        sinceSent.restart();
        state->urgent = false;
        // (the window is given time to take what it has: a worker that is far ahead of it only makes more for it to hold)
        while (!last && state->pending.load() >= kMaxBatchesWaiting && !state->stopCalc.load() && !state->cancel.load() && state->holds.load() == 0) QThread::usleep(500);
    };
    bool first = true;   // (the first pass is made even when nothing is to be worked out, so that the window is told it is over)
    for (;;) {
        // wait for something to work out (the wait ends at least every so often, in case a request came between looking and waiting)
        if (!first) {
            result->calcIdle = true;
            state->wake.wait_for(lock, std::chrono::milliseconds(50), [&] { return state->cancel.load() || state->redo.load() || (!state->stopCalc.load() && calc->isDirty()); });
        }
        if (state->cancel.load()) break;
        if (state->redo.exchange(false)) calc->invalidateAll();
        if (!first && (state->stopCalc.load() || !calc->isDirty())) continue;
        first = false;
        result->calcIdle = false;
        bool stopped = false;
        qint64 epoch = 0;
        for (;;) {
            const Calculation::Progress p = calc->recalculate(0, nullptr, kSliceMicros);
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
        flush(true, stopped, epoch);
        result->calcIdle = true;
        while (state->holds.load() > 0 && !state->cancel.load()) QThread::usleep(20);
        lock.lock();
    }
    calc->setResultSink(nullptr);
    calc->setCancel(nullptr);
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
    state->serving = true;
    send([self] { if (self) Q_EMIT self->finished(true, QString()); });
    // and the formulas are worked out, here, as the window shows what it has; then the worker stays, to work out what the window changes
    serve(result, state, self);
}

} // namespace jp::crunch
