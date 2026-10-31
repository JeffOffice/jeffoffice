#pragma once
// JeffCrunch's main window: the ribbon, the name box and formula bar, the grid, the sheet tabs, and the status bar, with the File screen over them. It opens a workbook on a
// worker (WorkbookLoader), shows the first rows at once, and when the whole workbook is read works its formulas out a few milliseconds at a time. A person types into cells (over the
// cell, in it with F2 or a double click, or in the formula bar), clears them with Delete, undoes and redoes, and saves the workbook as it was read with only what changed put in.

#include "app/appwindow.h"
#include "app/anchordialogs.h"
#include "app/celleditor.h"
#include "app/filterpopup.h"
#include "app/finddialog.h"
#include "app/slicetuner.h"
#include "app/sortdialog.h"
#include "app/formatdialog.h"
#include "app/widgets.h"
#include "core/color.h"
#include "core/auditing.h"
#include "core/editsession.h"
#include "core/formulaspot.h"
#include "core/filterlist.h"
#include "core/findrun.h"
#include "core/gotospecial.h"
#include "core/loader.h"
#include "grid/gridview.h"
#include "grid/pagelayoutview.h"
#include "grid/valuestore.h"
#include "sheet/chartmake.h"
#include "sheet/pagesetup.h"
#include "sheet/tablestyle.h"

#include <QElapsedTimer>
#include <QHash>
#include <QKeySequence>
#include <QPointer>
#include <QTimer>
#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <unordered_map>

class QComboBox;
class QDialog;
class QDockWidget;
class QPrinter;
class QSpinBox;
class QThread;
class QUndoStack;
class QLabel;
class QLineEdit;
class QMenu;
class QPlainTextEdit;
class QSlider;
class QVBoxLayout;
class QToolButton;
class QUrl;

namespace jp {

struct SelectDataRequest;
struct ObjectProperties;
namespace office { struct Chart; }
class CrunchFileScreen;
struct PageSetupRequest;
namespace grid {
struct SheetPrint;
class PageLayoutView;
class PageLayoutSource;
}
namespace office::sheet { struct PageSetup; }
class FormulaHelper;
class HelpView;
class HelpWindow;
class Ribbon;
struct RibbonParts;
namespace crunch {
class PrintJob;
struct PrintChoices;
struct PrintJobOptions;
class SheetObjects;
class SheetTabBar;
}

class CrunchWindow : public AppWindow {
    Q_OBJECT
public:
    explicit CrunchWindow(QWidget *parent = nullptr);
    ~CrunchWindow() override;

    QAction *act(const QString &id) const override;
    QStringList actionIds() const override;
    Ribbon *ribbon() const override { return m_ribbon; }
    grid::GridView *view() const { return m_view; }
    QString ribbonErrors() const { return m_ribbonErrors; }

    // ---- the file
    QString filePath() const { return m_path; }
    // The name the title shows: the file's, or "Book1" for a window with none.
    QString displayName() const;
    bool hasWorkbook() const { return bool(m_loaded); }
    // Opens a workbook: in this window if it holds none, else in a window of its own. The reading goes on in the background (loadFinished says when it is over); false, with the
    // reason shown, if the file can't be read at all.
    bool openFile(const QString &path);
    bool isLoading() const { return m_loading; }
    // Makes the window hold a new, empty workbook (one sheet, "Sheet1", and a name like "Book1" until it is saved), read the way a file is: the calculation, the worker and the editing session
    // are the same. loadFinished says when it is ready. A window that is already showing a workbook is not changed (false).
    bool newBlank();
    // Whether the window holds a new workbook nobody has changed or saved (or is still getting it ready): a file opened in it takes its place.
    bool isUntouchedBlank() const;
    // For a window made only to hold the file it is asked to open: when that file can't be read, the window closes (after the message) instead of staying empty.
    void setClosesIfOpenFails(bool closes) { m_closeIfOpenFails = closes; }
    // The path a Save writes to: the file's own for a workbook of the XML form. For one read from the older binary form (.xls) it is a new name with the ending .xlsx: the person is told
    // in plain words that workbooks are saved as .xlsx and the file is not changed, and asked for the name (the file dialog, or the picker a test sets); the file it was read from is never
    // the answer. Empty if there is no workbook or the person cancels.
    QString pathToSave();
    // The loader that reads the workbook and works it out (a test holds its worker back to make what it checks independent of speed).
    crunch::WorkbookLoader &workbookLoader() { return m_loader; }
    bool isCalculating() const { return m_calculating; }
    // A copy or cut is still being turned to text (a large selection is done a few milliseconds at a time); the clipboard has it when this is false.
    bool isCopying() const { return m_copy != nullptr; }
    // ---- editing
    // The entry in progress (typed over a cell, or in it, or in the formula bar) is finished: the cell takes it (one step of Undo), and the selection goes the way the key says. False,
    // with the reason shown, and the entry still there, if the cell can't take it (a formula that is not one).
    bool isEditing() const { return m_edit.active; }
    CellEditor *cellEditor() const { return m_cellEditor; }
    crunch::EditSession *session() const { return m_session.get(); }
    // What the notes and links ask through, and how an address is opened (a test gives its own: nothing is ever opened but by a click or a command).
    AnchorHooks &anchorHooks();
    // Whether opening a .csv, .tsv, or .txt file asks how it is read (the program turns it on; a window made for a test leaves it off, and the file is detected). Open Text File asks always.
    void setPromptForText(bool on) { m_promptForText = on; }
    // The person's language: how numbers are typed and shown (the decimal mark, the grouping of digits, the order of a date). The window begins with the system's.
    void setLocale(const QLocale &locale);
    // The objects over the cells (null before the window is built).
    crunch::SheetObjects *objects() const { return m_objects.get(); }
    QUndoStack *undoStack() const;
    bool commitEdit(CellEditor::Way way = CellEditor::Way::Stay);
    void cancelEdit();

    // ---- Find and Replace, Go To (crunchwindow_find.cpp)
    // The dialog of Find and Replace (null until it is first shown), and whether a search is going on (it is made a few thousand cells at a time).
    FindDialog *findDialog() const { return m_findDialog; }
    bool isSearching() const { return bool(m_findRun) || bool(m_replacePlan) || m_applying == 2; }
    // Goes to what the text names: a cell, a range, a cell of another sheet, whole rows or columns, a defined name. False, with the reason shown, if it names nothing.
    bool goToReference(const QString &text);
    // Select by Kind: the first range of the cells found is selected and the rest are tinted (the next click takes the tint away). False, with the reason shown, if there are none.
    bool goToSpecial(const jp::crunch::SpecialOptions &options);
    // What Go To asks for (a reference) and Select by Kind (the kind of cells): dialogs unless these stand in for them (tests); false gives it up.
    void setGoToAsker(const std::function<bool(QString *)> &asker) { m_goToAsker = asker; }
    void setSpecialAsker(const std::function<bool(jp::crunch::SpecialOptions *)> &asker) { m_specialAsker = asker; }

    // ---- Sort (crunchwindow_sort.cpp)
    // Sorts the cells of the range (or, with none valid, the selection, or the block around the cell with the cursor) in the order the spec says; Sort A–Z and Z–A, and Sort…, come
    // here. The work is done a slice at a time, the order on a thread of its own, and the step on the window's thread; sortFinished says when it is over.
    bool sortRange(const jp::office::sheet::CellRange &range, const jp::crunch::SortSpec &spec);
    bool isSorting() const { return bool(m_sortPlan) || m_applying == 1; }
    // A big sort or replace is put in a slice at a time; a test makes the slices all one size (the cells each does).
    void setFixedSlices(qint64 cells);
    bool isApplying() const { return m_applying != 0; }
    // What Sort… asks for: given the range and the spec it begins from, the spec the person chose (a dialog unless this stands in for it: tests); false gives it up.
    void setSortAsker(const std::function<bool(const jp::office::sheet::CellRange &range, jp::crunch::SortSpec *spec)> &asker) { m_sortAsker = asker; }
    // And what is asked when cells next to the selection hold data that would not be sorted with it: true sorts the whole block, false only what is selected (a message unless this stands in).
    void setExpandAsker(const std::function<bool()> &asker) { m_expandAsker = asker; }

    // ---- Pick a Function (crunchwindow_function.cpp)
    // Begins a formula with the function (or puts it at the cursor of the formula being typed): "=NAME()" with the cursor inside the brackets. False if there is no such function, or the
    // entry being made is not a formula.
    bool insertFunction(const QString &name);
    // What Pick a Function asks (the function's name): a dialog unless this stands in for it (tests); false gives it up.
    void setFunctionAsker(const std::function<bool(QString *)> &asker) { m_functionAsker = asker; }

    // ---- the Formulas tab (crunchwindow_formulas.cpp, crunchwindow_names.cpp, crunchwindow_calc.cpp, crunchwindow_audit.cpp)
    // The help while a formula is typed (the list of functions and names that fit, and the arguments of the function being typed).
    FormulaHelper *formulaHelper() const { return m_helper; }
    // How many times the sheet's whole picture was made again (a test counts them: a small change does not make it again).
    int sheetShows() const { return m_sheetShows; }
    // Names: the list, a name made for the cells selected, names made from the labels around them, and a name put in the formula being typed (or begun). What the dialogs ask is
    // asked of these instead when a test sets them; false gives it up.
    void showNames();
    bool defineNameCommand();
    bool nameFromCellsCommand();
    bool insertName(const QString &name);
    QMenu *namesMenu();
    // For tests: how many times the sorted list the menu shows was made (it is made again only when a name or the sheet in view changes).
    int namesMenuBuilds() const { return m_menuBuilds; }   // the names to choose from, as a menu (not shown); the caller owns it
    void setNameAsker(const std::function<bool(crunch::EditSession::NameDraft *)> &asker) { m_nameAsker = asker; }
    void setEdgesAsker(const std::function<bool(int *)> &asker) { m_edgesAsker = asker; }
    // How the workbook calculates: the mode (a change of it is one step of Undo), the circles, and the work that waits when it is done by hand.
    void setCalculationMode(jp::office::sheet::CalcMode mode);
    bool calculationDue() const { return m_calcDue; }
    // The formulas found in circles when the last calculation ended (0 when the workbook goes round them, or has none), and the list of them.
    qint64 circularCount() const { return m_circular; }
    bool showCircularReferences();
    void setIterationAsker(const std::function<bool(crunch::CalcSettings *)> &asker) { m_iterationAsker = asker; }
    // Checking formulas: arrows from the cells the formula in the cell with the cursor reads (one more step back each time), arrows from the cell to the formulas that read it (one more step
    // on each time), and the arrows taken away. They are taken away by any change to the workbook. False, with the reason in the status line, when there is nothing to draw.
    bool tracePrecedents();
    bool traceDependents();
    void removeArrows();
    int arrowCount() const { return int(m_trace.size()); }
    // The circular references named once in a message each time they begin (not again while they stay).
    int circularNotices() const { return m_circularNotices; }

    // ---- the message of a rule of validation (crunchwindow_cells.cpp)
    // The note shown beside the cell with the cursor when a rule of validation for it has a message to show (its title and text); null while there is none.
    QWidget *inputMessage() const { return m_inputMessage && m_inputMessage->isVisible() ? m_inputMessage.data() : nullptr; }

    // ---- the filter (crunchwindow_filter.cpp)
    // Turns the filter on (over the block around the cell with the cursor, or the cells selected) or off; and sets the sheet's filter to `filter`, as one step (a large one is worked out a
    // slice at a time; filterFinished says when it is over). The drop-down of a button of the filter is a FilterPopup, kept in `filterPopup` while it is up.
    void toggleFilter();
    bool applyFilter(const jp::office::sheet::AutoFilter &filter, const QString &text, int table = -1);
    bool isFiltering() const { return bool(m_filterPlan); }
    FilterPopup *filterPopup() const { return m_filterPopup; }
    // ---- tables (crunchwindow_tables.cpp)
    // The gallery of looks as a widget (what the Table Looks menus hold), and a choice from it. For tests: the questions Insert > Table and Change Table Size ask are answered by these.
    QWidget *makeTableLooksPicker(QWidget *parent);
    void tableGalleryChosen(const QString &style);
    bool showTotalsMenu(qint64 row, qint64 col);
    void setTableMakeAsker(std::function<bool(QString *, bool *)> asker) { m_tableMakeAsker = std::move(asker); }
    void setTableSizeAsker(std::function<bool(jp::office::sheet::CellRange *)> asker) { m_tableSizeAsker = std::move(asker); }
    void showFilterPopup(qint64 row, qint64 col);
    // What the filters that ask (Equals…, Between…, Largest or Smallest…, Your Own Condition…) ask the person: given the question, the column's name, whether it holds dates, and the condition it has (or null),
    // the condition chosen. A dialog unless this stands in for it (tests); false gives it up.
    void setFilterAsker(const std::function<bool(jp::FilterAsk, const QString &column, bool dates, const jp::office::sheet::FilterColumn *current, jp::office::sheet::FilterColumn *chosen)> &asker)
    {
        m_filterAsker = asker;
    }

    // ---- saving
    // Saves the workbook where it was read from (the file is made whole in a place of its own, and then put where the old one is), or, with a path, there. The work is done on a thread of its own;
    // `saved` says when it is over. False if it did not begin (nothing to save, or the reason is shown).
    bool save();
    // For tests and tools: the bytes a save would write now, made at once on this thread from the same snapshot a save takes. Nothing is written, and the session is not told it was saved (Undo
    // keeps all its steps), so it can be called after every edit; false, with the reason, when a save could not be made.
    bool snapshotBytesForTest(QByteArray *bytes, QString *why);
    bool saveAs(const QString &path);
    // Writes the values of the sheet in view as a .csv file (see crunchwindow_csv.cpp); `csvExported` says when it is over. A path ending in .csv given to save() or saveAs() comes here.
    bool exportCsv(const QString &path);
    bool isModified() const { return m_session && m_session->isModified(); }
    bool isSaving() const { return m_saving; }
    // What the person is asked when the window is closed with changes that are not saved: Save, Don't Save, or Cancel (a message box, unless this stands in for it: tests).
    enum class Answer { Save, Discard, Cancel };
    void setAsker(const std::function<Answer(const QString &title, const QString &text)> &asker) { m_asker = asker; }
    // What is asked when the file to be saved over was changed by another program since it was read: save a copy instead (Save As), write over it, or give the save up. The name is the
    // file's. A message box with the three, unless this stands in for it (tests).
    enum class ChangedAnswer { SaveCopy, Overwrite, Cancel };
    // What Fill > Series asks for: the way the series goes, its kind, the unit of a date series, the step, and where to stop; a message dialog unless this stands in for it (tests).
    struct SeriesRequest {
        bool inRows = false;
        crunch::EditSession::FillOptions options;
    };
    void setSeriesAsker(const std::function<bool(SeriesRequest *)> &asker) { m_seriesAsker = asker; }
    // What Insert and Delete ask when cells are selected (not whole rows or columns): how the cells after the block move, or that the whole rows or columns go. A dialog unless this stands in
    // for it (tests); false gives it up.
    struct ShiftRequest {
        bool inserting = true;
        enum class Way { Right, Down, Left, Up, Rows, Columns } way = Way::Down;
    };
    void setShiftAsker(const std::function<bool(ShiftRequest *)> &asker) { m_shiftAsker = asker; }
    // What Column Width and Row Height ask for: the size (the width a person is shown, in characters of the default font's widest digit; the height in points), with the one it has now. A dialog
    // unless this stands in for it (tests); false gives it up.
    struct SizeRequest {
        bool column = true;
        double value = 0;
        bool standard = false;   // the sheet's default (Default Width, Default Row Height), not the size of the selected columns or rows
    };
    void setSizeAsker(const std::function<bool(SizeRequest *)> &asker) { m_sizeAsker = asker; }
    // What Move or Copy asks: which sheets (their places), before which sheet (its place; -1: at the end), and whether to make copies. A dialog unless this stands in for it (tests); false
    // gives it up.
    struct MoveCopyRequest {
        QVector<int> sheets;
        int before = -1;
        bool copy = false;
    };
    void setMoveCopyAsker(const std::function<bool(MoveCopyRequest *)> &asker) { m_moveCopyAsker = asker; }
    // The sheets the tabs have picked (their places in the workbook), the one with the cursor among them.
    QVector<int> selectedSheets() const;
    // Gives a sheet a new name (what the tab's name line does when it is finished): the formulas that name it follow. The reason is shown if the name is refused.
    bool renameSheetTo(int sheet, const QString &name);
    // What is asked when a value does not fit the rule of validation of its cell and the rule is a warning or only tells: the rule's title, its message, and its style (Validation::Style, 1
    // warning, 2 information); true goes on. A message box unless this stands in for it (tests). A rule that stops is only told (through the notifier).
    // For tests: what asks whether to go on with a step that Undo cannot hold (its text), standing in for the question.
    void setBigStepAsker(const std::function<bool(const QString &text)> &asker) { m_bigStepAsker = asker; }
    void setRuleAsker(const std::function<bool(const QString &title, const QString &text, int style)> &asker) { m_ruleAsker = asker; }
    // And Paste Special: how to paste. A dialog unless this stands in for it (tests); false gives it up.
    void setPasteAsker(const std::function<bool(crunch::EditSession::PasteOptions *)> &asker) { m_pasteOptionsAsker = asker; }
    void setChangedAsker(const std::function<ChangedAnswer(const QString &name)> &asker) { m_changedAsker = asker; }
    // What Format Cells asks: given the format of the cell with the cursor, what the person changed (a dialog unless this stands in for it: tests); false gives it up.
    void setFormatAsker(const std::function<bool(const FormatCellsDialog::Input &, FormatCellsDialog::Result *)> &asker) { m_formatAsker = asker; }
    // What is asked before a step that throws something away (merging cells that hold values): the title and the sentence; true goes on. A message box unless this stands in for it (tests).
    void setConfirmAsker(const std::function<bool(const QString &title, const QString &text)> &asker) { m_confirmAsker = asker; }
    // The Format Painter: whether it is waiting for cells to paint, and whether it stays on after painting (a double click).
    // How many times the buttons of formatting were made to show a cell's format (they are not, for a cell that has the format they already show).
    int formatStateRuns() const { return m_formatStateRuns; }
    bool formatPainterActive() const { return m_painter.mode != PainterMode::Off; }
    bool formatPainterLocked() const { return m_painter.mode == PainterMode::Locked; }
    // The format the dialog starts from (the cell with the cursor), and the dialog's changes made.
    FormatCellsDialog::Input formatInput() const;
    bool applyFormatResult(const FormatCellsDialog::Result &result);
    // Where Save As writes: asked of a file dialog, unless this stands in for it (tests): the name it starts with, and the filter; an empty path gives it up.
    void setSavePicker(const std::function<QString(const QString &start, const QString &filter)> &picker) { m_savePicker = picker; }
    // The kind of file the stand-in for the file dialog says was picked (what the dialog gives as its selected filter).
    void setPickedFilter(const QString &filter) { m_pickedFilter = filter; }
    // What a short message to the person is shown with: a message box, unless this stands in for it (tests): the title and the sentence.
    void setNotifier(const std::function<void(const QString &title, const QString &text)> &notifier) { m_notifier = notifier; }

    // ---- the sheets
    // The sheets shown as tabs, by their place in the workbook (hidden ones are not here), and the one in view.
    QVector<int> shownSheets() const { return m_tabSheets; }
    int activeSheet() const { return m_sheet; }
    void setActiveSheet(int sheetIndex);
    // The calculation, once the workbook is read whole: it is the worker's (which works the formulas out on its own thread, and tells the window what it has), so it is only to be
    // looked at when the calculation is not going (a test, after calculationFinished).
    const jp::office::sheet::Calculation *calculation() const { return m_haveCalc && m_loaded ? m_loaded->calc.get() : nullptr; }
    // What the calculation has worked out so far, as the grid reads it.
    const jp::grid::ValueStore &values() const { return m_values; }
    // The longest time (the thread's own, in microseconds) the window has spent in taking one batch of values from the calculation: what the calculation costs the window's thread.
    qint64 longestBatchMicros() const { return m_longestBatchMicros; }
    qint64 batchesTaken() const { return m_batches; }
    // The worker thread's own processor time since it began, as the last batch said (microseconds).
    qint64 workerCpuMicros() const { return m_workerCpuMicros; }
    // The most values one batch had (what taking a batch costs the window is in proportion to it, whatever the machine's speed).
    qint64 longestBatchItems() const { return m_longestBatchItems; }
    // For tests: slows the worker (see WorkbookLoader::setPace).
    void setWorkerPace(int micros) { m_loader.setPace(micros); }
    const crunch::LoadedBook *loaded() const { return m_loaded.get(); }
    // The workbook as read so far, held by whoever keeps it (a test, after the window is gone).
    std::shared_ptr<crunch::LoadedBook> loadedBook() const { return m_loaded; }

    // ---- how the sheets print (crunchwindow_page.cpp)
    // What the Page Setup dialog is given and gives back: a dialog unless this stands in for it (tests); false gives it up.
    // For tests: stands in for the file picker of the Picture button of the Header and Footer tab.
    void setPictureAsker(const std::function<QString()> &asker) { m_pictureAsker = asker; }
    void setPageSetupAsker(const std::function<bool(PageSetupRequest *)> &asker) { m_pageSetupAsker = asker; }
    // Opens Page Setup on a tab (0 Page, 1 Margins, 2 Header and Footer, 3 Sheet); the change is one step of Undo.
    void pageSetupDialog(int tab = 0);

    // ---- printing (crunchwindow_print.cpp)
    // A job for what the options say, of the workbook as it is now: the sheet in view is the active sheet, and the selection the grid's if the options name none. Null, with the reason in
    // `why`, when there is nothing to print or the workbook is not open whole.
    std::unique_ptr<crunch::PrintJob> makePrintJob(crunch::PrintJobOptions options, QString *why = nullptr);
    // Puts the job's pages (all, or those of the range text) in a PDF file; the work is done here, with the wait cursor. False, with the reason, if it could not be done.
    bool exportPdfTo(const QString &path, crunch::PrintJobOptions options, const QString &pageRange = QString(), QString *why = nullptr, bool *cancelled = nullptr);
    // For tests: stands in for the progress bar of an export or a print (it is given the pages done and the total, and says false to cancel).
    void setProgressHook(const std::function<bool(int, int)> &hook) { m_progressHook = hook; }
    // File > Export: asks for a name and puts the sheet in view (or every sheet that is shown) in a PDF file. True when the file is made; false, with the reason shown, if it is not or the
    // person gives it up.
    bool exportPdf(bool wholeWorkbook);
    // Prints what the choices say, to the printer or into a PDF file the person names (what asks for the name is the Save As picker). True when it was done; false, with the reason shown.
    bool printWith(const crunch::PrintChoices &choices);
    // What the Print screen has chosen (kept while the window is, so the screen comes back as it was left).
    crunch::PrintChoices &printChoices() { return *m_printChoices; }
    // The printer the Print screen's dialog sets up and the pages go to (made when first asked for).
    QPrinter &printer();
    // For tests: given the printer before the pages are sent (to give it a file, say).
    void setPrinterPreparer(const std::function<void(QPrinter &)> &preparer) { m_printerPreparer = preparer; }
    // The lines on the sheet in view where its pages begin follow the sheet as it is now (View > Page Breaks turns them off). They are worked out when the Print screen is shown or something
    // is printed, and kept up to date until the window is closed.
    std::function<bool(int, int)> progressFunction(const QString &title, const QString &text);
    void armPageBreaks();
    void updatePageBreaks();
    // How the sheet in view is shown: 0 the cells as they are, 1 the sheet cut into the pages it prints on (the printed cells light and the rest shaded, the pages numbered, the lines
    // where they begin and the edges of the printed cells dragged), 2 the pages themselves. What the person chose for each sheet is kept with the workbook and saved.
    void setViewKind(int kind);
    int viewKind() const { return m_viewKind; }
    // For tests: a line where a page begins is dragged from a row (column) to another, as the mouse would ask.
    void moveBreakForTest(bool column, qint64 from, qint64 to) { breakLineMoved(column, from, to); }
    // For tests and the window: the view of the pages, and the strip of text being typed in a header or footer.
    grid::PageLayoutView *pageView() const { return m_pageView; }
    QPlainTextEdit *zoneEditor() const { return m_zoneEditor; }
    // The pages of the sheet in the Page Layout view have been laid out (a job is held for drawing them).
    bool pageLayoutReady() const { return bool(m_layoutPages); }
    crunch::PrintJob *layoutJobForTest() const { return m_layoutJob.get(); }
    // The lines are worked out in slices of counted work, a turn of the event loop each; an edit, or a workbook opened over this one, drops what was begun.
    void schedulePageBreaks();
    void cancelPageBreaks();
    void stepPageBreaks();
    // For tests: how the last lines were worked out, in how many slices and the most work (grid/axis.h) any one did.
    struct PageBreakWork {
        int slices = 0;
        qint64 most = 0;
    };
    PageBreakWork pageBreakWork() const { return m_breakStats; }
    bool pageBreaksUnderWay() const { return bool(m_breakJob); }
    void stepPageBreaksNow() { stepPageBreaks(); }
    static qint64 pageBreakSliceWork();
    // The lines for a sheet's pages: a row's line for each page that begins a band of rows, and a column's likewise (a break set by hand is marked, unless the sheet is fitted to pages).
    static void pageBreakLines(const jp::grid::SheetPrint &print, const jp::office::sheet::PageSetup &setup, std::vector<jp::grid::GridView::PageBreakLine> *rows,
                               std::vector<jp::grid::GridView::PageBreakLine> *cols);

    // ---- the File screen, Help
    void showFileScreen(const QString &page = QString());
    void hideFileScreen();
    void showHelp(const QString &topic = QString());
    HelpView *helpView() const { return m_help; }
    QString statusText() const;
    // The words of the status line at the left (loading, calculating, ready) for a test to read.
    QLabel *statusLabel() const { return m_status; }
    QLineEdit *nameBox() const { return m_nameBox; }
    QLineEdit *formulaBar() const { return m_formulaBar; }
    crunch::SheetTabBar *sheetTabs() const { return m_tabs; }

Q_SIGNALS:
    // The workbook was saved (to the path), or could not be.
    void saved(const QString &path, bool ok);
    // The values of the sheet in view were written in a .csv file, or could not be.
    void csvExported(const QString &path, bool ok);
    // The first rows are on the screen.
    void firstScreenShown();
    // The workbook is read whole (or could not be opened).
    void loadFinished(bool ok);
    // Every formula has its value.
    void calculationFinished();
    // A sort is over: it was made (true), or it was refused, given up, or could not be made.
    void sortFinished(bool made);
    // A filter was put in (true), or was refused, given up, or could not be.
    void filterFinished(bool made);
    // The window was asked to open a file it has open (or another window was), and has been brought to the front.
    void broughtForward();

protected:
    void closeEvent(QCloseEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
    bool eventFilter(QObject *o, QEvent *e) override;
    void dragEnterEvent(QDragEnterEvent *e) override;
    void dropEvent(QDropEvent *e) override;

private:
    QAction *mk(const QString &id, const QString &text, const QString &iconName, const QKeySequence &key, const std::function<void()> &fn, bool checkable = false, bool enabled = true);
    void createActions();
    void buildRibbon();
    void ribbonParts(RibbonParts &parts);
    void buildCentral();
    void buildStatusBar();
    void updateTitle();
    void updateBars();
    void showBlank();
    void dropWorkbook();
    static QString nextUntitledName();
    void showSheet(int sheetIndex, bool keepView);
    void rememberView();
    void fillTabs();
    void onFirstScreen();
    void onProgress(int sheet, qint64 cells);
    void onFinished(bool ok, const QString &message);
    void onCalcBatch(const std::shared_ptr<crunch::CalcBatch> &batch);
    bool stopCalculation();   // true if a save that was waiting for the calculation was cancelled
    // the commands on a block of cells (crunchwindow_ops.cpp)
    void createOpsActions();
    QStringList m_opsActions;   // the commands that work when a workbook is open and nothing is being saved
    void autoSum(const QString &function);
    void fillSelection(crunch::EditSession::FillDirection direction);
    void fillDragged(const jp::grid::GridRange &source, const jp::grid::GridRange &extent, bool ctrl);
    void fillSeriesDialog();
    // inserting and deleting rows, columns, and cells (crunchwindow_cells.cpp)
    void createCellsActions();
    void insertCommand();
    void deleteCommand();
    void insertLines(bool rows);
    void deleteLines(bool rows);
    void insertOrDeleteCells(bool inserting);
    bool askShift(ShiftRequest *request);
    void onCellsMoved(const jp::office::sheet::StructEdit &edit);
    void showContextMenu(const QPoint &globalPos);
    jp::office::sheet::CellRange selectedRange() const;
    std::function<bool(ShiftRequest *)> m_shiftAsker;
    std::function<bool(SizeRequest *)> m_sizeAsker;
    std::function<bool(const QString &, const QString &, int)> m_ruleAsker;
    std::function<bool(const QString &)> m_bigStepAsker;
    // Does an edit that moves cells with the window saying it is working (the wait cursor, and the status), and, if the step is too big for Undo to hold, asks first.
    crunch::EditSession::Outcome doStructural(const std::function<crunch::EditSession::Outcome()> &edit);
    bool askAboutRule(const crunch::EditSession::Outcome &outcome);
    // the list a rule offers on the cell with the cursor (crunchwindow_cells.cpp)
    void updateDropDown();
    void updateInputMessage();
    QPointer<QWidget> m_inputMessage;
    void showChoices();
    // what Paste Special asks for: the options (a dialog unless this stands in for it: tests); false gives it up
    std::function<bool(crunch::EditSession::PasteOptions *)> m_pasteOptionsAsker;
    crunch::ClipData m_clip, m_pendingClip;   // the last copy or cut made here that the clipboard still holds, and the one being made
    jp::grid::GridRange m_pendingBorder{-1, -1, -1, -1};
    QString m_pendingHtml;
    bool m_settingClipboard = false;
    // the width of columns and the height of rows
    void onSizesChanged(int sheet);
    void resizeDragged(bool column, qint64 index, int size);
    void fitDragged(bool column, qint64 index);
    void setSizeDialog(bool column);
    void defaultSizeDialog(bool column);
    void fitSelection(bool column);
    void hideSelection(bool column, bool hide);
    void widenForEntry(int sheet, quint32 row, quint32 col);
    // the columns (rows) a command on sizes is for: those of the selection
    void selectedLines(bool column, quint32 *first, quint32 *last) const;
    // editing
    void createEditing();
    void beginEdit(const QString &initial, CellEditor::Mode mode, bool fromBar);
    void placeEditor();
    void finishEdit(CellEditor::Way way);
    void onCellsChanged(int sheet, const std::vector<jp::grid::ValueStore::Item> &cells);
    void clearSelection();
    void undoRedo(bool redo);
    void updateEditActions();
    bool editingAllowed();
    // saving
    bool startSave(const QString &path);
    void onSaved(bool ok, const QString &message, const QString &path, const QByteArray &bytes, const std::shared_ptr<crunch::EditSession::Snapshot> &snapshot);
    bool askAboutChanges();
    QString saveKind() const;
    // The name from the file dialog (or what stands in for it), with the ending the kind of file picked in it asks for.
    QString askForSavePath(const QString &caption, const QString &start, const QString &filter);
    // Whether `path` is the file this workbook was read from, if that was a file of another form (.xls, .ods, .csv, ...): that file is never written, for the whole session.
    bool isTheOriginal(const QString &path) const;
    // Whether the workbook has no file of its own form to be saved over yet: it was read from the older binary form, and no .xlsx has been written from it.
    bool savesAsANewFile() const;
    void setStatus(const QString &text);
    void goToName();
    // Find and Replace, Go To (crunchwindow_find.cpp)
    enum class SearchFor { Next, All, ReplaceAll };
    void createFindActions();
    void showFindDialog(bool replace);
    void startSearch(SearchFor what);
    void stepSearch();
    void finishSearch();
    void cancelSearch();
    void replaceCurrent();
    void goToFound(int sheet, quint32 row, quint32 col);
    void goToDialog();
    void goToSpecialDialog();
    jp::crunch::FindOptions searchOptions(bool forReplace) const;
    QPointer<FindDialog> m_findDialog;
    std::unique_ptr<jp::crunch::FindRun> m_findRun;
    QTimer m_findTimer;
    SearchFor m_searchFor = SearchFor::Next;
    qint64 m_searchChanges = 0;
    int m_searchSteps = 0;
    void stepReplace();
    void cancelReplace();
    void finishReplace(bool done);
    std::shared_ptr<jp::crunch::EditSession::ReplacePlan> m_replacePlan;
    QTimer m_replaceTimer;
    SliceTuner m_searchTuner, m_replaceTuner, m_listTuner, m_filterTuner, m_sortTuner, m_applyTuner;
    // a big change put in a slice at a time (crunchwindow_find.cpp): 1 a sort, 2 a replace
    void runApply(int what, const std::function<jp::crunch::EditSession::Outcome(qint64)> &step, const std::function<void(const jp::crunch::EditSession::Outcome &)> &done);
    void stepApply();
    int m_applying = 0;
    std::function<jp::crunch::EditSession::Outcome(qint64)> m_applyStep;
    std::function<void(const jp::crunch::EditSession::Outcome &)> m_applyDone;
    QTimer m_applyTimer;
    bool m_sortSmall = true;
    bool m_settingMarks = false;
    // Pick a Function (crunchwindow_function.cpp)
    void createNameActions();
    void createCalcActions();
    void createAuditActions();
    void createFormulaHelper();
    const crunch::CompletionIndex &completionIndex();
    QString completionDetail(const crunch::Completion &c) const;
    bool insertTextInFormula(const QString &text, bool asFunction);
    void followCalcBatch(const crunch::CalcBatch &batch);
    void updateCalcStatus();
    void sendCalcSettings();
    void announceCircles();
    void refreshCheckMarks();
    struct TraceArrow {
        int fromSheet = 0;
        jp::office::sheet::CellRange from;
        int toSheet = 0;
        quint32 toRow = 0, toCol = 0;
    };
    std::vector<TraceArrow> m_trace;
    struct TraceRoot {
        bool set = false;
        int sheet = 0;
        quint32 row = 0, col = 0;
    } m_traceRoot;
    std::vector<crunch::TraceCell> m_traceUp, m_traceDown;   // the formulas the next step back and the next step on begin from
    bool m_tracedUp = false, m_tracedDown = false;
    bool startTrace(bool up);
    QLabel *m_calcNote = nullptr;
    std::vector<int> m_menuNames;
    qint64 m_menuGeneration = -1;
    int m_menuSheet = -2, m_menuBuilds = 0;
    FormulaHelper *m_helper = nullptr;
    crunch::CompletionIndex m_completions;
    qint64 m_completionsGeneration = -1;
    int m_completionsUndo = -1;
    int m_sheetShows = 0;
    int m_completionsSheet = -2;
    std::function<bool(crunch::EditSession::NameDraft *)> m_nameAsker;
    std::function<bool(int *)> m_edgesAsker;
    std::function<bool(crunch::CalcSettings *)> m_iterationAsker;
    bool m_calcDue = false, m_manualNow = false;
    qint64 m_circular = 0;
    std::vector<jp::office::sheet::Calculation::CircularCell> m_circleCells;   // the first of them, in the order of the sheets and cells
    int m_circularNotices = 0;
    bool m_circularNoticed = false;
    void createFunctionActions();
    void insertFunctionCommand();
    std::function<bool(QString *)> m_functionAsker;
    // tables (crunchwindow_tables.cpp): how they are drawn, their header buttons, the commands that change them, and their ribbon tab
    void createTableActions();
    void tableRibbonParts(RibbonParts &parts);
    void updateTableLooks();                                   // the styles of the sheet's tables, given to the grid
    void tableHeaderButtons(std::vector<jp::grid::GridView::HeaderButton> *buttons);
    void onTablesChanged(int sheet);
    void updateTableTab();                                     // the Table tab shows for a cell in a table, and its controls show that table
    int selectedTable() const;                                 // the table the cursor is in, or -1
    void insertTableCommand();
    void tableDialog(int mode);
    void addTableMenu(QMenu *menu);   // the table's commands in the cell menu, when the cursor is in a table
    bool pointAtTableCell(qint64 row, qint64 col);   // a click on a cell of a table while a formula is typed writes a reference by the table's names
    jp::office::sheet::TableStyleBook m_tableStyleBook;
    bool m_tableStylesFor = false;
    std::function<bool(QString *, bool *)> m_tableMakeAsker;    // test hook: the table's cells' header choice
    std::function<bool(QString *)> m_tableNameAsker;
    std::function<bool(jp::office::sheet::CellRange *)> m_tableSizeAsker;
    // the filter (crunchwindow_filter.cpp)
    void createFilterActions();
    void updateFilterButtons();
    void stepFilter();
    void finishFilter(bool done);
    void cancelFilter();
    void stepList();
    void listFinished();
    void askFilter(jp::FilterAsk what, quint32 column, int table = -1);
    jp::office::sheet::AutoFilter currentFilter() const;
    std::shared_ptr<jp::crunch::EditSession::FilterPlan> m_filterPlan;
    QString m_filterText;
    QTimer m_filterTimer, m_listTimer;
    QPointer<FilterPopup> m_filterPopup;
    const QObject *m_listPopup = nullptr;   // (the popup the listing belongs to, as a plain address: read while that popup is being destroyed, when it is no longer a FilterPopup)
    std::unique_ptr<jp::crunch::FilterListRun> m_listRun;
    qint64 m_listChanges = 0;
    int m_listSteps = 0;
    std::function<bool(jp::FilterAsk, const QString &, bool, const jp::office::sheet::FilterColumn *, jp::office::sheet::FilterColumn *)> m_filterAsker;
    // sorting (crunchwindow_sort.cpp)
    void createSortActions();
    void sortSelection(int mode);   // 0 A to Z, 1 Z to A, 2 the dialog
    bool sortRangeOf(jp::office::sheet::CellRange *range, int *key, bool *expanded, int *header = nullptr);
    void stepSort();
    void finishSort(bool ordered);
    void cancelSort();
    std::shared_ptr<jp::crunch::EditSession::SortPlan> m_sortPlan;
    QTimer m_sortTimer;
    QPointer<QThread> m_sortThread;
    std::atomic<bool> m_sortCancel{false};
    bool m_sortOrderedOk = false;
    bool m_sortOrdering = false;
    std::function<bool(const jp::office::sheet::CellRange &, jp::crunch::SortSpec *)> m_sortAsker;
    std::function<bool()> m_expandAsker;
    std::function<bool(QString *)> m_goToAsker;
    std::function<bool(jp::crunch::SpecialOptions *)> m_specialAsker;
    void showNameInBox();
    // the clipboard (crunchwindow_clip.cpp)
    void createClipboardActions();
    void copySelection();
    void cutSelection();
    void copyOrCut(bool cut);
    void copyStep();
    void publishClipboard(const QString &text);
    void pasteClipboard(const crunch::EditSession::PasteOptions &options);
    void pasteSpecial();
    void onClipboardChanged();
    void forgetClip();
    void cancelCut();
    void widenForCells(int sheet, const jp::office::sheet::CellRange &cells);
    QString htmlOfClip(const crunch::ClipData &clip) const;
    void freezeAtSelection();
    // the sheets (crunchwindow_sheets.cpp): the tabs, and the commands that add, copy, rename, delete, move, hide, and color them
    void createSheetActions();
    void onSheetsChanged();
    void onTabMoved(int from, int to);
    void refreshStripes();
    void insertSheetCommand(bool atEnd);
    void deleteSheetsCommand();
    void moveCopyCommand();
    void hideSheetsCommand();
    void setTabColorCommand(const jp::office::sheet::ColorSpec &color);
    void showTabMenu(const QPoint &globalPos);
    QMenu *buildTabColorMenu(QWidget *owner);
    QMenu *buildUnhideMenu(QWidget *owner);
    std::function<bool(MoveCopyRequest *)> m_moveCopyAsker;
    QWidget *m_sheetTabRow = nullptr;
    // formatting (crunchwindow_format.cpp): the commands of the Home tab and the Format Cells dialog
    enum class PainterMode { Off, Once, Locked };
    void createFormatActions();
    void formatRibbonParts(RibbonParts &parts);
    // The buttons of formatting are made to show the format of the cell with the cursor. Unless `force`, nothing is done again for a cell that has the same cell format as the last one shown
    // (the cursor moving down a column of cells, or an entry that is kept: the buttons already show it).
    void updateFormatState(bool force = true);
    bool formatSelection(const jp::office::sheet::FormatChange &change, const QString &text);
    FormatView viewOfActiveCell() const;
    void toggleFont(int what);
    void setFontSize(double size);
    void growFont(int steps);
    void chooseFillColor(const ColorRef &c);
    void chooseFontColor(const ColorRef &c);
    void alignCommand(int kind);
    void indentCommand(int delta);
    void orientationCommand(int degrees);
    void numberFormatCommand(int kind);
    void decimalsCommand(int delta);
    void borderCommand(int kind);
    void mergeCommand(crunch::EditSession::MergeKind kind);
    void clearCommand(int what);
    void applyCellStyle(const QString &name);
    void formatPainterCommand();
    void paintFormats();
    void cancelFormatPainter();
    void formatCellsDialog(int tab);
    bool confirm(const QString &title, const QString &text);
    ColorScheme workbookColors() const;
    jp::office::sheet::ColorSpec colorSpecOf(const ColorRef &c) const;
    QMenu *buildBordersMenu(QWidget *owner);
    QMenu *buildStylesMenu(QWidget *owner);
    QMenu *buildOrientationMenu(QWidget *owner);
    std::function<bool(const FormatCellsDialog::Input &, FormatCellsDialog::Result *)> m_formatAsker;
    std::function<bool(const QString &, const QString &)> m_confirmAsker;
    QList<QPointer<FontCombo>> m_fontCombos;
    QList<QPointer<SizeCombo>> m_sizeCombos;
    QPointer<ColorButton> m_fillButton, m_fontColorButton;
    QPointer<QComboBox> m_numberBox;
    QStringList m_formatActions;                // the commands that format cells (enabled when the workbook can be changed)
    jp::office::sheet::BorderStyle m_lineStyle = jp::office::sheet::BorderStyle::Thin;
    jp::office::sheet::ColorSpec m_lineColor;   // the color of the lines the border commands draw (none: automatic)
    int m_lastBorder = 0;                       // the border command the button of the Borders menu repeats
    struct Painter {
        PainterMode mode = PainterMode::Off;
        crunch::ClipData clip;
        QElapsedTimer started;
    } m_painter;
    struct FormatShown {                         // what the buttons of formatting show now: the workbook, the cell format of the cell with the cursor, and what else decides it
        const void *book = nullptr;
        quint32 xf = ~quint32(0);
        quint32 flags = 0;
        bool operator==(const FormatShown &o) const { return book == o.book && xf == o.xf && flags == o.flags; }
    } m_formatShown;
    int m_formatStateRuns = 0;
    int m_formatOpen = -1;                       // whether the commands of formatting were last made usable (1), or not (0); -1 before the first time
    bool m_formatting = false;                   // a formatting command is working (the state of the buttons is not read back into it)
    void notice(const QString &title, const QString &text, bool warning = true);
    QString helpContext() const;
    HelpWindow *showHelpOver(const QString &topic);

    // the views of a sheet: the cells, the page breaks, the pages (crunchwindow_views.cpp)
    void createViewActions();
    void addViewButtons(QStatusBar *bar);
    void showViewKind(int kind);
    void breakLineMoved(bool column, qint64 from, qint64 to);
    void printEdgeMoved(int area, int side, qint64 to);
    void sendBreakPages(const crunch::PrintJob &job, const jp::office::sheet::PageSetup &setup);
    int m_viewKind = 0;
    bool m_showingSheet = false;   // a sheet is being given to the grid (its zoom is not the person's change)

    // the Page Layout view: the pages one below the other, with the header and footer in the margins to be typed in (crunchwindow_pagelayout.cpp)
    void createPageLayoutActions();
    void buildPageLayoutView(QWidget *parent, QVBoxLayout *into);
    void showPageLayout(bool on);
    void takeLayoutJob(std::unique_ptr<crunch::PrintJob> job);
    void dropLayoutJob();
    void syncPageViewSelection();
    void zoneClicked(int page, bool header, int zone);
    void endZoneEdit(bool keep);
    void insertHeaderCode(const QString &code);
    void insertHeaderPicture();
    void updateHeaderFooterActions();
    double viewZoom() const;
    void setViewZoom(double zoom);
    grid::PageLayoutView *m_pageView = nullptr;
    std::shared_ptr<crunch::PrintJob> m_layoutJob;
    std::shared_ptr<grid::PageLayoutSource> m_layoutPages;
    int m_layoutSheet = -1;
    QPlainTextEdit *m_zoneEditor = nullptr;
    struct ZoneEdit {
        int page = -1;
        bool header = true;
        int zone = 0;
        int form = 0;   // 0 the pages in general, 1 the even pages, 2 the first page
        bool keep = false;
        jp::office::sheet::HeaderPicture picture;
        bool hasPicture = false;
    } m_zone;
    qint64 m_pressRow = 0, m_pressCol = 0;

    // the page commands and the Page Layout tab (crunchwindow_page.cpp)
    void createPageActions();
    void pageRibbonParts(RibbonParts &parts);
    void connectPageSetup();
    void updatePageActions();
    // Changes how the sheet in view prints: one step of Undo named `text`; what is wrong is told.
    void changePage(const QString &text, const std::function<void(jp::office::sheet::PageSetup *)> &change);
    std::function<bool(PageSetupRequest *)> m_pageSetupAsker;
    std::function<QString()> m_pictureAsker;
    std::function<void(QPrinter &)> m_printerPreparer;
    std::function<bool(int, int)> m_progressHook;
    std::unique_ptr<crunch::PrintChoices> m_printChoices;
    std::unique_ptr<QPrinter> m_printer;
    bool m_breaksArmed = false;
    QTimer m_breaksTimer, m_breaksStepTimer;
    std::unique_ptr<crunch::PrintJob> m_breakJob;
    const crunch::LoadedBook *m_breakBook = nullptr;
    int m_breakSheet = -1;
    PageBreakWork m_breakStats;
    QStringList m_pageActions;
    bool m_updatingPage = false;
    QComboBox *m_fitWidthBox = nullptr, *m_fitHeightBox = nullptr;
    QSpinBox *m_scaleBox = nullptr;

    // the charts, pictures, and shapes over the cells (crunchwindow_drawings.cpp)
    void setupObjects();
    void openObjects();
    void prepareObjects(int sheet);
    void connectObjects();
    void deleteObject(int id);
    std::unique_ptr<crunch::SheetObjects> m_objects;
    quint64 m_objectsToken = 0;
    bool m_chartSheetShown = false;   // the sheet in view is a chart sheet: no command that works on cells is on


    // ---- the charts: making and changing them (crunchwindow_charts.cpp)
public:
    // What New Chart asks (a dialog unless this stands in for it: tests): the kind, and how to read the cells (-1 as their shape says, 2 the other way); false gives it up.
    struct InsertChartChoice {
        jp::office::sheet::ChartKind kind = jp::office::sheet::ChartKind::Column;
        int inRows = -1;
    };
    void setInsertChartAsker(const std::function<bool(InsertChartChoice *)> &asker) { m_insertChartAsker = asker; }
    // Chart Data: given the chart and what a dialog needs, gives the chart as it should be; false gives it up.
    void setSelectDataAsker(const std::function<bool(const SelectDataRequest &, jp::office::Chart *)> &asker) { m_selectDataAsker = asker; }
    // Chart Kind: given each series' name and what it is drawn as, gives what each is to be drawn as.
    void setChangeTypeAsker(const std::function<bool(const QStringList &, const QVector<jp::office::sheet::SeriesKind> &, QVector<jp::office::sheet::SeriesKind> *)> &asker) { m_changeTypeAsker = asker; }
    // Properties of an object: its size and what it does when cells change.
    void setPropertiesAsker(const std::function<bool(const ObjectProperties &, ObjectProperties *)> &asker) { m_propertiesAsker = asker; }
    // The commands of charts, for tests and the menus: a chart of the kind from the cells selected; the picked object moved to an anchor; the model of the picked chart changed.
    bool insertChart(jp::office::sheet::ChartKind kind, int inRows = -1);
    void insertChartDialog();
    // The dialogs, made but not shown, for whoever shows them (the window runs them; a tool takes pictures of them). Null if there is nothing for them to work on.
    QDialog *makeInsertChartDialog();
    QDialog *makeSelectDataDialog();
    QDialog *makeChangeTypeDialog();
    bool moveObject(int id, const jp::office::sheet::DrawingAnchor &anchor);
    bool changePickedChart(const QString &text, const std::function<void(jp::office::Chart *)> &change);
    void selectDataDialog();
    // A picture of a chart of the kind from the cells selected (a part of them), at a size in pixels; null if there is nothing to chart.
    QImage chartPreviewImage(jp::office::sheet::ChartKind kind, int inRows, const QSize &pixels);
    void changeChartTypeDialog();
    void propertiesDialog();
    // The picked object, or -1.
    int pickedObject() const;
    // The chart buttons beside the picked chart (the elements, the looks), and what the context menu of an object has; for tests.
    QToolButton *chartElementsButton() const { return m_chartPlus; }
    QToolButton *chartLooksButton() const { return m_chartBrush; }
    QMenu *buildObjectMenu(int id, QWidget *owner);
    QMenu *buildChartElementsMenu(QWidget *owner);
    QMenu *buildChartLooksMenu(QWidget *owner);
    // Cut, Copy, and Paste of the picked object (a chart), within this workbook and between workbooks.
    bool copyObject(bool cut);
    bool pasteObject();
    // Puts the picked object in front of or behind the others (+1 forward, -1 backward, 2 to the front, -2 to the back).
    bool reorderObject(int how);

private:
    void createChartActions();
    void chartRibbonParts(RibbonParts &parts);
    SelectDataRequest selectDataRequest(int id);
    void updateChartState();
    void placeChartButtons();
    bool chartPlanFor(jp::office::sheet::ChartKind kind, int inRows, jp::office::sheet::ChartPlan *plan, bool forPreview = false);
    jp::office::sheet::DrawingAnchor newObjectAnchor(double width, double height);
    void objectContextMenu(int id, const QPoint &global);
    std::function<bool(InsertChartChoice *)> m_insertChartAsker;
    std::function<bool(const SelectDataRequest &, jp::office::Chart *)> m_selectDataAsker;
    std::function<bool(const QStringList &, const QVector<jp::office::sheet::SeriesKind> &, QVector<jp::office::sheet::SeriesKind> *)> m_changeTypeAsker;
    std::function<bool(const ObjectProperties &, ObjectProperties *)> m_propertiesAsker;
    QString m_workbookId;   // which workbook this is, among all that are open in any program: random for each one opened (what a copy says it came from)
    jp::office::sheet::CellRange m_previewBlock;   // the block of cells the pictures of the New Chart dialog are drawn from, found once for all of them
    QToolButton *m_chartPlus = nullptr, *m_chartBrush = nullptr;
    QTimer m_chartButtonsTimer;
    QStringList m_chartActions;

    grid::GridView *m_view = nullptr;
    Ribbon *m_ribbon = nullptr;
    CrunchFileScreen *m_fileScreen = nullptr;
    HelpView *m_help = nullptr;
    QDockWidget *m_helpDock = nullptr;
    QLineEdit *m_nameBox = nullptr, *m_formulaBar = nullptr;
    QWidget *m_formulaRow = nullptr;
    crunch::SheetTabBar *m_tabs = nullptr;
    QLabel *m_status = nullptr, *m_stats = nullptr, *m_zoomLabel = nullptr;
    QSlider *m_zoomSlider = nullptr;
    QHash<QString, QAction *> m_actions;
    QString m_ribbonErrors;
    QString m_path;
    QString m_untitled;       // the name of a workbook that has no file yet ("Book1")
    bool m_blank = false;     // the workbook is a new one (made by newBlank) that has not been saved
    bool m_closeIfOpenFails = false;   // the window was made only to hold one file: if the file can't be read, the window goes
    QString m_pendingTyping;           // what was typed before a new workbook was ready: the entry begins with it when it is
    QString m_originalPath;   // the file a converted workbook was read from, kept after the first save moves m_path to the .xlsx
    // the workbook, as it is read
    crunch::WorkbookLoader m_loader;
    std::shared_ptr<crunch::LoadedBook> m_loaded;
    bool m_loading = false, m_calculating = false, m_haveFirstScreen = false, m_haveCalc = false;
    int m_sheet = -1;                            // the sheet in the grid, by its place in the workbook
    QVector<int> m_tabSheets;                    // the workbook's place of each tab
    QHash<int, grid::GridView::ViewState> m_views;   // what was done in each sheet, kept when another is shown
    bool m_switchingTabs = false;
    // what an empty window shows
    jp::office::sheet::Sheet m_emptySheet;
    jp::office::sheet::Styles m_emptyStyles;
    jp::office::sheet::SharedStrings m_emptyStrings;
    // a copy of a large selection, made a few milliseconds at a time
    std::unique_ptr<grid::TextWalk> m_copy;
    QTimer m_copyTimer;
    // the values the calculation (on the worker) has told
    jp::grid::ValueStore m_values;
    qint64 m_longestBatchMicros = 0, m_batches = 0, m_longestBatchItems = 0, m_workerCpuMicros = 0;
    std::function<void(const QString &, const QString &)> m_notifier;
    QString m_loadProblem;
    // editing
    std::unique_ptr<crunch::EditSession> m_session;
    CellEditor *m_cellEditor = nullptr;
    struct EditState {
        bool active = false;
        int sheet = 0;
        quint32 row = 0, col = 0;
        QString original;      // what the formula bar showed before (Esc puts it back)
    } m_edit;
    bool m_syncingEditors = false, m_undoing = false, m_finishing = false;
    qint64 m_tabOriginCol = -1, m_tabRow = -1;   // where a run of Tabs began (Enter then goes back to that column)
    // saving
    bool m_saving = false, m_closeAfterSave = false, m_closing = false;
    QPointer<QThread> m_saveThread;
    QString m_saveAfterCalculation;
    QPointer<QObject> m_csvRun;   // a .csv being written
    std::function<Answer(const QString &, const QString &)> m_asker;
    std::function<ChangedAnswer(const QString &)> m_changedAsker;
    std::function<bool(SeriesRequest *)> m_seriesAsker;
    std::function<QString(const QString &, const QString &)> m_savePicker;
    QString m_pickedFilter;
    // Whether the file at the window's path is not what was read (or last written): another program changed it.
    bool fileChangedOnDisk() const;
    // The window that has the file open (or is reading it), if any.
    static CrunchWindow *windowWith(const QString &path);
    // ---- notes and links (crunchwindow_notes.cpp)
    void createNoteActions();
    void connectAnchors();
    void refreshAnchors();
    void updateNoteActions();
    void editNote();
    void deleteSelectedNotes();
    void toggleNoteShown();
    void showAllNotes();
    void goToNote(bool next);
    void hoverCell(qint64 row, qint64 col);
    void hideHoverTip();
    void showHoverTip();
    QString hoverTipText(bool *isNote) const;
    void insertLink();
    void removeSelectedLinks();
    void followSelectedLink();
    void followLinkAt(quint32 row, quint32 col);
    void goToPlace(const QString &place);
    void cellClicked(qint64 row, qint64 col, Qt::KeyboardModifiers modifiers);
    bool openAddress(const QUrl &url);
    // ---- opening a text file of values (crunchwindow_text.cpp)
    bool askTextImport(const QString &path, std::shared_ptr<const jp::office::sheet::CsvImport> *chosen);
    bool m_promptForText = false, m_forceTextPrompt = false;
    // ---- automatic row heights (crunchwindow_autoheight.cpp)
    void scheduleAutoHeights();
    void stepAutoHeights();
public:
    // How many slices the measuring of the rows has taken, and the most rows measured in one (for tests: a slice is charged by the work it does, so a row of very long text takes one alone).
    int autoHeightSlices() const { return m_autoSlices; }
    int mostRowsMeasuredInASlice() const { return m_autoMostRowsInASlice; }
private:
    int m_autoSlices = 0, m_autoMostRowsInASlice = 0;
    void noteRowsChanged(int sheet, const std::vector<jp::grid::ValueStore::Item> &cells);
    void noteWidthsChanged(int sheet);
    std::map<int, std::set<quint32>> m_autoDirty;                       // by sheet: the rows an edit changed, not measured yet
    std::map<int, std::unordered_map<quint32, int>> m_autoSeen;         // by sheet: the rows measured, and the generation of the widths they were
    std::map<int, int> m_autoGen;                                       // by sheet: how many times the widths changed
    QTimer m_autoTimer;
    bool m_applyingHeights = false;
    // ---- protection (crunchwindow_protect.cpp)
    void createProtectActions();
    void connectProtection();
    void updateProtectActions();
    void protectSheet();
    void protectWorkbook();
    void guardSelection();
    void askPassword(const QString &what, const jp::office::sheet::PasswordHash &hash, const std::function<void()> &onRight);
    void hashPassword(const QString &typed, const std::function<void(const jp::office::sheet::PasswordHash &)> &done);
    struct PasswordJob;
    void runPasswordJob(const QString &text, const std::function<int(const std::atomic<bool> *)> &work, const std::function<void(int)> &done);
    void stopPasswordJob();
    std::shared_ptr<PasswordJob> m_passwordJob;
    void undoRedoNow(bool redo);
    grid::Selection m_lastSelectable;
    bool m_revertingSelection = false;
    AnchorHooks m_anchorHooks;
    QTimer m_hoverTimer;
    QPointer<QWidget> m_notePopup;
    bool m_hoverTipShown = false;
    QString m_formulaFull;    // what the cell with the cursor holds, whole (the formula bar shows a part of a long text until it has the focus)
};

} // namespace jp
