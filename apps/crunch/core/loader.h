#pragma once
// Opening a workbook without holding the window up: a worker reads the package and the workbook's own parts (the sheets' names, the styles, the shared strings), then the first
// rows of the sheet that was open, which the window shows at once (the first screen); then it reads every sheet whole and puts the formulas in the calculation, which the window
// then works out a few milliseconds at a time. The window shows the first rows while the rest is read, and stops the worker when it is closed or asked for another file.

#include "grid/gridsheet.h"
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
};

// A slice of the calculation: the clock ends it after a formula once this many microseconds have passed (a slice is then that, and one formula more), so that a window never waits
// for long. The steps one formula may take are bounded to match: a formula past it keeps the result the file stored.
constexpr qint64 kSliceMicros = 4000;
constexpr qint64 kFormulaSteps = 40000;

// The sheet to show first: the one the file says was open if it is a worksheet that is shown, else the first that is; -1 for none.
int firstSheetToShow(const jp::office::sheet::WorkbookInfo &info);
// The rows read for the first screen (the most any screen shows, at the smallest zoom, and a good many more).
constexpr qint64 kFirstScreenRows = 1000;

// The two steps of a read, as the worker does them (and a tool does them one after the other). `cancelled` is asked every few thousand cells: true stops it.
// The first: opens the package and the workbook's own parts, and reads the first kFirstScreenRows rows of the sheet to show first. False, with a message, if the file can't be opened.
bool loadFirstScreen(const QString &path, LoadedBook *out, const std::function<bool()> &cancelled, QString *why);
// The second: reads every sheet whole and registers the formulas in the calculation (out->calc). `progress` is told the sheet being read and the cells read of it. False if
// stopped; what could not be read whole is in out->problem.
bool loadWhole(LoadedBook *out, const std::function<bool()> &cancelled, const std::function<void(int sheet, qint64 cells)> &progress);

class WorkbookLoader : public QObject {
    Q_OBJECT
public:
    explicit WorkbookLoader(QObject *parent = nullptr);
    ~WorkbookLoader() override;
    // Begins reading `path` (the one being read, if any, is stopped first).
    void start(const QString &path);
    // Stops the read; the signals of a read that was stopped are not sent. It does not wait for the worker (which sees it within a few thousand cells, or at the end of the part of
    // the file it is reading); a worker that is still going when the program ends is waited for then.
    void cancel();
    bool isRunning() const;
    // The result so far; the calculation is in it once `finished` has been sent.
    std::shared_ptr<LoadedBook> result() const { return m_result; }

Q_SIGNALS:
    // The workbook is open and the first rows of a sheet are read.
    void firstScreen();
    // The sheet being read, and the cells of it so far.
    void progress(int sheet, qint64 cells);
    // The read is over: `ok` false, with a message, when the file could not be opened (nothing is shown then) or no sheet could be read; true when the workbook is open (what
    // could not be read whole is in LoadedBook::problem).
    void finished(bool ok, const QString &message);

private:
    struct State {
        std::atomic<bool> cancel{false};
    };
    void run(const QString &path, std::shared_ptr<LoadedBook> result, std::shared_ptr<State> state, QPointer<WorkbookLoader> self);
    std::shared_ptr<QThread> m_thread;
    std::shared_ptr<LoadedBook> m_result;
    std::shared_ptr<State> m_state;
};

} // namespace jp::crunch
