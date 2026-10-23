#include "core/loader.h"

#include "office/package.h"

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QMetaObject>
#include <QThread>

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

void WorkbookLoader::cancel()
{
    if (m_state) m_state->cancel = true;
    m_thread.reset();   // (the janitor keeps it until it is over)
}

void WorkbookLoader::start(const QString &path)
{
    cancel();
    m_state = std::make_shared<State>();
    m_result = std::make_shared<LoadedBook>();
    m_result->path = path;
    const std::shared_ptr<LoadedBook> result = m_result;
    const std::shared_ptr<State> state = m_state;
    const QPointer<WorkbookLoader> self(this);
    m_thread = std::shared_ptr<QThread>(QThread::create([this, path, result, state, self] { run(path, result, state, self); }));
    janitor().add(m_thread);
    m_thread->start();
}

bool loadFirstScreen(const QString &path, LoadedBook *out, const std::function<bool()> &cancelled, QString *why)
{
    QElapsedTimer total;
    total.start();
    out->path = path;
    Package package;
    if (!package.readLazyFile(path, why)) return false;
    auto book = std::make_shared<Workbook>();
    if (!book->open(package, why)) return false;
    out->book = book;
    const int shownSheet = firstSheetToShow(book->info());
    if (shownSheet < 0) {
        *why = tr("The workbook has no sheets.");
        return false;
    }
    // the first rows of the sheet that was open
    auto preview = std::make_shared<Sheet>();
    book->readSheet(shownSheet, preview.get(), [&cancelled](qint64) { return !cancelled(); }, kFirstScreenRows - 1);
    out->preview = preview;
    out->previewSheet = shownSheet;
    out->previewGeometry = jp::grid::GridSheet::makeGeometry(*preview, book->styles());
    out->firstScreenMs = total.elapsed();
    return true;
}

bool loadWhole(LoadedBook *out, const std::function<bool()> &cancelled, const std::function<void(int, qint64)> &progress)
{
    QElapsedTimer total;
    total.start();
    CalcOptions options;
    options.keepApart = true;          // what the engine cannot work out keeps the result the file stored
    options.workLimit = 2000000;       // steps one formula may take (about as long as a screen may wait for it)
    auto calc = std::make_unique<Calculation>(*out->book, options);
    QElapsedTimer sinceTold;
    sinceTold.start();
    calc->setProgress([&](int sheet, qint64 cells) {
        if (cancelled()) return false;
        if (progress && (cells == 0 || sinceTold.elapsed() > 100)) {
            sinceTold.restart();
            progress(sheet, cells);
        }
        return true;
    });
    const bool whole = calc->load();
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
    QString why;
    if (!loadFirstScreen(path, result.get(), cancelled, &why)) {
        send([self, why] { if (self) Q_EMIT self->finished(false, why); });
        return;
    }
    if (cancelled()) return;
    send([self] { if (self) Q_EMIT self->firstScreen(); });
    if (!loadWhole(result.get(), cancelled, [&](int sheet, qint64 cells) { send([self, sheet, cells] { if (self) Q_EMIT self->progress(sheet, cells); }); })) return;
    send([self] { if (self) Q_EMIT self->finished(true, QString()); });
}

} // namespace jp::crunch
