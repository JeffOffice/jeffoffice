#pragma once
// Opening a workbook without holding the window up: a worker reads the package and the workbook's own parts (the sheets' names, the styles, the shared strings), then the first
// rows of the sheet that was open, which the window shows at once (the first screen); then it reads every sheet whole and puts the formulas in the calculation, and goes on to work
// them out, on the same thread, telling the window each batch of values as it has them (the window puts them in its ValueStore, a few thousand at a time). Nothing of the calculation
// is done on the window's thread, so no formula of any size holds the window up. The window shows the first rows while the rest is read, and stops the worker when it is closed or
// asked for another file. The worker stays when the formulas are worked out: it waits, and works out what the window changes (see WorkbookLoader::hold).

#include "grid/gridsheet.h"
#include "grid/valuestore.h"
#include "sheet/calculation.h"
#include "sheet/notes.h"
#include "sheet/sheetreader.h"

#include <QObject>
#include <QPointer>
#include <QString>
#include <QStringList>

#include <atomic>
#include <condition_variable>
#include <functional>
#include <map>
#include <memory>
#include <mutex>

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
    // The notes of every sheet as the file has them (by cell), read with the sheets; the editing session takes them (an empty list for a workbook that is not from an .xlsx file).
    std::vector<std::map<quint64, jp::office::sheet::NoteInfo>> notes;
    std::vector<bool> notesTruncated;   // a sheet that has more notes than are read (jp::office::sheet::kMaxModelNotes): its notes are shown, not changed
    QString problem;                                        // what could not be read whole, in words ("" when all of it was)
    QStringList notRead;                                    // what a workbook of the older binary form has that is not read (charts, pictures, ...), in words
    qint64 firstScreenMs = 0, loadMs = 0;                   // how long each took
    std::atomic<qint64> cellsRead{0};                       // cells of the sheets read so far (set by the worker as it reads, and read by anyone)
    std::atomic<bool> workerEnded{false};                   // the worker has gone: it will touch nothing of this again
    std::atomic<quintptr> calcThread{0};                    // the thread that worked the formulas out (QThread::currentThreadId), 0 if none has: a test checks it is not the window's
    std::atomic<bool> calcIdle{false};                      // the worker has nothing to work out and waits (it stays, to work out what the window changes); false while it works
};

// What the calculation has worked out since the last batch: the values (which the window puts in its store), how far it has come, and whether it is the last.
struct CalcBatch {
    std::vector<jp::grid::ValueStore::Item> items;
    int percent = 0;               // of the formulas, worked out
    bool last = false;             // the calculation is over (finished, or stopped)
    bool stopped = false;          // ... because it was stopped
    qint64 epoch = 0;              // for a last batch: how many changes the window had made when the calculation finished (WorkbookLoader::changesMade): all of them are worked out
    qint64 workerCpuMicros = 0;    // the worker thread's processor time since it began
    int generation = 0;            // how many times rows, columns, or cells had been moved (WorkbookLoader::structureGeneration) when the values in it were worked out: cells they name are where
                                   // they were then
};

// A slice of the calculation (on the worker): the clock ends it after a formula once this many microseconds have passed, and the worker tells the window what it has worked out (a batch
// every so many values or milliseconds, and whenever it stops), looks at the stop flag between slices and, inside a formula, as it takes its steps.
constexpr qint64 kSliceMicros = 5000;
constexpr int kBatchValues = 2048;       // the most values a batch holds (the window puts them in its store, and makes their texts again, in a millisecond or two)
constexpr int kBatchMillis = 30;         // and the longest a value waits to be told
constexpr int kMaxBatchesWaiting = 3;    // the worker waits for the window to take what it has been sent when this many have not been

// The sheet to show first: the one the file says was open if it is a worksheet that is shown, else the first that is; -1 for none.
int firstSheetToShow(const jp::office::sheet::WorkbookInfo &info);
// The rows read for the first screen: from the row the sheet's view begins at (and its frozen rows), this many (the most any screen shows, at the smallest zoom, and a good many more).
constexpr qint64 kFirstScreenRows = 1000;

// The two steps of a read, as the worker does them (and a tool does them one after the other). `cancelled` is asked every few thousand cells: true stops it.
// The first: opens the package and the workbook's own parts, and reads the first kFirstScreenRows rows of the sheet to show first. False, with a message, if the file can't be opened.
bool loadFirstScreen(const QString &path, LoadedBook *out, const std::function<bool()> &cancelled, QString *why, const std::shared_ptr<const jp::office::sheet::CsvImport> &csv = nullptr);
// The second: reads every sheet whole and registers the formulas in the calculation (out->calc). `progress` is told the sheet being read and the cells read of it. False if
// stopped; what could not be read whole is in out->problem. `cancelFlag`, if given, is looked at as the formulas are registered too; `pace`, if given and above 0, is the microseconds
// to wait at each step of the read (see WorkbookLoader::setPace).
bool loadWhole(LoadedBook *out, const std::function<bool()> &cancelled, const std::function<void(int sheet, qint64 cells)> &progress, const std::atomic<bool> *cancelFlag = nullptr,
               const std::atomic<int> *pace = nullptr);

// What a file's suffix is called in the usage statistics: one of the formats JeffCrunch opens, else "other" (never the suffix as it is: it is whatever the file is called).
QString fileKind(const QString &path);

class WorkbookLoader : public QObject {
    Q_OBJECT
    struct State;
public:
    explicit WorkbookLoader(QObject *parent = nullptr);
    ~WorkbookLoader() override;
    // Begins reading `path` (the one being read, if any, is stopped first).
    // `csv`, if given, is what a person chose for a text file of values (how it is split into fields and columns); the file is detected without it.
    void start(const QString &path, const std::shared_ptr<const jp::office::sheet::CsvImport> &csv = nullptr);
    // Stops the read and the calculation; the signals of one that was stopped are not sent. It does not wait for the worker (which sees it within a few thousand cells, or in the
    // step of a formula it is taking); a worker that is still going when the program ends is waited for then.
    void cancel();
    bool isRunning() const;
    // Stops the calculation only (what it has worked out is told; the rest keeps what the file stored); calculationStopped is then the last batch's `stopped`.
    void stopCalculation();
    // Works every formula out again, on the worker; false if there is nothing to work out (no workbook read whole).
    bool recalculate();

    // ---- changing the cells, on the window's thread
    // The calculation and the cells are the worker's while it works the formulas out; the window reads the cells directly to draw them (reading beside the worker is no
    // trouble), so a change to them is made by the window with the worker held back: it leaves the formula it is on (the evaluator looks at a flag as it takes its steps, so that is
    // a few microseconds, or the one search for the readers of a cell, which is a millisecond or so for a cell that two hundred thousand formulas read) and waits until the hold is
    // let go. The hold is for the change only; the work it makes (finding what reads the cells, working it out) is the worker's, and its results come back in batches like any.
    class Hold {
    public:
        ~Hold();
        Hold(const Hold &) = delete;
        Hold &operator=(const Hold &) = delete;
        // The calculation, for the window to change cells in.
        jp::office::sheet::Calculation *calculation() const { return m_calc; }
        // Cells were changed: the worker works out what that makes when the hold is let go, and sends the first of it at once.
        void changed() { m_changed = true; }
        // Rows, columns, or cells were moved: values the worker has worked out and not yet told, and batches on their way, name cells by where they were (see CalcBatch::generation).
        void structureChanged();

    private:
        friend class WorkbookLoader;
        Hold(std::shared_ptr<State> state, jp::office::sheet::Calculation *calc);
        std::shared_ptr<State> m_state;
        jp::office::sheet::Calculation *m_calc;
        bool m_changed = false;
    };
    // Holds the worker back and gives the calculation; null if the workbook is not open whole yet (or the worker is gone).
    std::unique_ptr<Hold> hold();
    // Whether hold() would give one.
    bool canHold() const;
    // For tests: the next `count` calls of hold() give null, as when the worker is not serving for a moment (a large count: for as long as the test wants).
    void refuseHolds(int count);
    // How many holds have been taken (for tests: a read that needs the worker held back says so by taking one).
    qint64 holdsTaken() const;
    // How many changes have been made (holds let go with a change, and requests to work everything out again): a last batch whose epoch is this many has all of them worked out.
    qint64 changesMade() const;
    // How many times cells have been moved by an edit (a hold let go after structureChanged).
    int structureGeneration() const;
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
        std::atomic<bool> serving{false};     // the workbook is read whole and the worker is ready to work formulas out for the window
        std::mutex model;                     // the worker has it while it works formulas out (a slice at a time); the window has it while it changes cells
        std::condition_variable wake;         // the worker waits on it (with `model`) for something to work out
        std::atomic<int> holds{0};            // the window wants the model: the worker lets go of it at once and keeps off it
        std::atomic<bool> halt{false};        // the calculation's flag to stop: set while it is stopped (stopCalc) or a hold is wanted (holds)
        std::atomic<qint64> changes{0};       // changes made (see changesMade)
        std::atomic<qint64> holdsMade{0};     // holds taken (see holdsTaken)
        std::atomic<int> refuse{0};           // holds to refuse (see refuseHolds)
        std::atomic<int> structure{0};        // times cells were moved (see structureGeneration)
        std::atomic<bool> urgent{false};      // a change was made: the first values it makes go to the window as soon as they are there
        void updateHalt() { halt.store(stopCalc.load() || holds.load() > 0 || cancel.load()); }
    };
    static void run(const QString &path, const std::shared_ptr<const jp::office::sheet::CsvImport> &csv, std::shared_ptr<LoadedBook> result, std::shared_ptr<State> state, QPointer<WorkbookLoader> self);
    static void serve(const std::shared_ptr<LoadedBook> &result, const std::shared_ptr<State> &state, const QPointer<WorkbookLoader> &self);
    void startThread(std::function<void()> work, const std::shared_ptr<State> &state);
    // Everything the worker does stops (and it is woken to see so).
    static void stopAll(State &state);
    std::shared_ptr<QThread> m_thread;
    std::shared_ptr<LoadedBook> m_result;
    std::shared_ptr<State> m_state;
    int m_pace = 0;
};

} // namespace jp::crunch
