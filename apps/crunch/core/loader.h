#pragma once
// Opening a workbook without holding the window up: a worker reads the package and the workbook's own parts (the sheets' names, the styles, the shared strings), then the first
// rows of the sheet that was open, which the window shows at once (the first screen); then it reads every sheet whole and puts the formulas in the calculation, and goes on to work
// them out, on the same thread, telling the window each batch of values as it has them (the window puts them in its ValueStore, a few thousand at a time). Nothing of the calculation
// is done on the window's thread, so no formula of any size holds the window up. The window shows the first rows while the rest is read, and stops the worker when it is closed or
// asked for another file.

#include "grid/gridsheet.h"
#include "grid/valuestore.h"
#include "sheet/calculation.h"
#include "sheet/sheetreader.h"

#include <QObject>
#include <QPointer>
#include <QString>

#include <atomic>
#include <functional>
#include <memory>

class QThread;

namespace jp::crunch {

// What the reading has made so far. The worker fills it in order: the workbook, the first rows, and last the calculation; it is not touched by the worker once `finished` is told.
struct LoadedBook {
    QString path;
    std::shared_ptr<jp::office::sheet::Workbook> book;
    std::shared_ptr<jp::office::sheet::Sheet> preview;     // the first rows of one sheet
    int previewSheet = -1;                                  // which (from 0, in the workbook's order)
    std::unique_ptr<jp::office::sheet::Calculation> calc;   // every sheet, whole, with its formulas
    // What the grid makes of each sheet's file (sizes, merged ranges, frozen panes), made here, on the worker, so that showing a sheet takes the window a moment: of the first rows
    // (made with them), and of every sheet by its place in the workbook (made with the whole read).
    std::shared_ptr<const jp::grid::GridGeometry> previewGeometry;
    std::vector<std::shared_ptr<const jp::grid::GridGeometry>> geometry;
    QString problem;                                        // what could not be read whole, in words ("" when all of it was)
    qint64 firstScreenMs = 0, loadMs = 0;                   // how long each took
    std::atomic<qint64> cellsRead{0};                       // cells of the sheets read so far (set by the worker as it reads, and read by anyone)
    std::atomic<bool> workerEnded{false};                   // the worker has gone: it will touch nothing of this again
};

// What the calculation has worked out since the last batch: the values (which the window puts in its store), how far it has come, and whether it is the last.
struct CalcBatch {
    std::vector<jp::grid::ValueStore::Item> items;
    int percent = 0;               // of the formulas, worked out
    bool last = false;             // the calculation is over (finished, or stopped)
    bool stopped = false;          // ... because it was stopped
    qint64 workerCpuMicros = 0;    // the worker thread's processor time since it began
};

// A slice of the calculation (on the worker): the clock ends it after a formula once this many microseconds have passed, and the worker tells the window what it has worked out (a batch
// every so many values or milliseconds, and whenever it stops), looks at the stop flag between slices and, inside a formula, as it takes its steps.
constexpr qint64 kSliceMicros = 5000;
constexpr int kBatchValues = 4096;       // the most values a batch holds (the window puts them in its store, and makes their texts again, in about two milliseconds)
constexpr int kBatchMillis = 30;         // and the longest a value waits to be told
constexpr int kMaxBatchesWaiting = 3;    // the worker waits for the window to take what it has been sent when this many have not been

// The sheet to show first: the one the file says was open if it is a worksheet that is shown, else the first that is; -1 for none.
int firstSheetToShow(const jp::office::sheet::WorkbookInfo &info);
// The rows read for the first screen: from the row the sheet's view begins at (and its frozen rows), this many (the most any screen shows, at the smallest zoom, and a good many more).
constexpr qint64 kFirstScreenRows = 1000;

// The two steps of a read, as the worker does them (and a tool does them one after the other). `cancelled` is asked every few thousand cells: true stops it.
// The first: opens the package and the workbook's own parts, and reads the first kFirstScreenRows rows of the sheet to show first. False, with a message, if the file can't be opened.
bool loadFirstScreen(const QString &path, LoadedBook *out, const std::function<bool()> &cancelled, QString *why);
// The second: reads every sheet whole and registers the formulas in the calculation (out->calc). `progress` is told the sheet being read and the cells read of it. False if
// stopped; what could not be read whole is in out->problem. `cancelFlag`, if given, is looked at as the formulas are registered too; `pace`, if given and above 0, is the microseconds
// to wait at each step of the read (see WorkbookLoader::setPace).
bool loadWhole(LoadedBook *out, const std::function<bool()> &cancelled, const std::function<void(int sheet, qint64 cells)> &progress, const std::atomic<bool> *cancelFlag = nullptr,
               const std::atomic<int> *pace = nullptr);

// What a file's suffix is called in the usage statistics: one of the formats JeffCrunch opens, else "other" (never the suffix as it is: it is whatever the file is called).
QString fileKind(const QString &path);

class WorkbookLoader : public QObject {
    Q_OBJECT
public:
    explicit WorkbookLoader(QObject *parent = nullptr);
    ~WorkbookLoader() override;
    // Begins reading `path` (the one being read, if any, is stopped first).
    void start(const QString &path);
    // Stops the read and the calculation; the signals of one that was stopped are not sent. It does not wait for the worker (which sees it within a few thousand cells, or in the
    // step of a formula it is taking); a worker that is still going when the program ends is waited for then.
    void cancel();
    bool isRunning() const;
    // Stops the calculation only (what it has worked out is told; the rest keeps what the file stored); calculationStopped is then the last batch's `stopped`.
    void stopCalculation();
    // Works every formula out again, on the worker that is calculating, or on one begun for it; false if there is nothing to work out (no workbook read whole).
    bool recalculate();
    // For tests: how long the worker waits (in microseconds) at each step of its work (every 4,096 cells read, and between slices of the calculation), so that a test can do what it does
    // while the worker is at work. 0 is no wait.
    void setPace(int micros);
    // Workers that have not ended (all loaders'): a window that is closed leaves its worker to stop by itself.
    static int workersRunning();
    // The result so far; the calculation is in it once `finished` has been sent.
    std::shared_ptr<LoadedBook> result() const { return m_result; }

Q_SIGNALS:
    // The workbook is open and the first rows of a sheet are read.
    void firstScreen();
    // The sheet being read, and the cells of it so far.
    void progress(int sheet, qint64 cells);
    // The read is over: `ok` false, with a message, when the file could not be opened (nothing is shown then) or no sheet could be read; true when the workbook is open (what
    // could not be read whole is in LoadedBook::problem). The calculation begins on the worker at once.
    void finished(bool ok, const QString &message);
    // What the calculation has worked out (the last batch says so).
    void calcBatch(const std::shared_ptr<jp::crunch::CalcBatch> &batch);

private:
    struct State {
        std::atomic<bool> cancel{false};      // everything stops
        std::atomic<bool> stopCalc{false};    // the calculation stops (set too when everything does)
        std::atomic<bool> redo{false};        // work every formula out again at the next slice
        std::atomic<int> pending{0};          // batches sent and not yet taken
        std::atomic<int> pace{0};
        std::atomic<bool> calcActive{false};  // a worker is calculating (or about to): a request to work everything out again is its to take
    };
    static void run(const QString &path, std::shared_ptr<LoadedBook> result, std::shared_ptr<State> state, QPointer<WorkbookLoader> self);
    static void calculate(const std::shared_ptr<LoadedBook> &result, const std::shared_ptr<State> &state, const QPointer<WorkbookLoader> &self, bool invalidateFirst);
    void startThread(std::function<void()> work);
    std::shared_ptr<QThread> m_thread;
    std::shared_ptr<LoadedBook> m_result;
    std::shared_ptr<State> m_state;
    int m_pace = 0;
};

} // namespace jp::crunch
