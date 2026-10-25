#pragma once
// JeffCrunch's main window: the ribbon, the name box and formula bar, the grid, the sheet tabs, and the status bar, with the File screen over them. It opens a workbook on a
// worker (WorkbookLoader), shows the first rows at once, and when the whole workbook is read works its formulas out a few milliseconds at a time. A person types into cells (over the
// cell, in it with F2 or a double click, or in the formula bar), clears them with Delete, undoes and redoes, and saves the workbook as it was read with only what changed put in.

#include "app/appwindow.h"
#include "app/celleditor.h"
#include "core/editsession.h"
#include "core/loader.h"
#include "grid/gridview.h"
#include "grid/valuestore.h"

#include <QElapsedTimer>
#include <QHash>
#include <QKeySequence>
#include <QPointer>
#include <QTimer>
#include <atomic>
#include <functional>
#include <memory>

class QComboBox;
class QDockWidget;
class QPrinter;
class QSpinBox;
class QThread;
class QUndoStack;
class QLabel;
class QLineEdit;
class QSlider;

namespace jp {

class CrunchFileScreen;
struct PageSetupRequest;
namespace grid { struct SheetPrint; }
namespace office::sheet { struct PageSetup; }
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
    // The objects over the cells (null before the window is built).
    crunch::SheetObjects *objects() const { return m_objects.get(); }
    QUndoStack *undoStack() const;
    bool commitEdit(CellEditor::Way way = CellEditor::Way::Stay);
    void cancelEdit();

    // ---- saving
    // Saves the workbook where it was read from (the file is made whole in a place of its own, and then put where the old one is), or, with a path, there. The work is done on a thread of its own;
    // `saved` says when it is over. False if it did not begin (nothing to save, or the reason is shown).
    bool save();
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
    };
    void setSizeAsker(const std::function<bool(SizeRequest *)> &asker) { m_sizeAsker = asker; }
    // What is asked when a value does not fit the rule of validation of its cell and the rule is a warning or only tells: the rule's title, its message, and its style (Validation::Style, 1
    // warning, 2 information); true goes on. A message box unless this stands in for it (tests). A rule that stops is only told (through the notifier).
    // For tests: what asks whether to go on with a step that Undo cannot hold (its text), standing in for the question.
    void setBigStepAsker(const std::function<bool(const QString &text)> &asker) { m_bigStepAsker = asker; }
    void setRuleAsker(const std::function<bool(const QString &title, const QString &text, int style)> &asker) { m_ruleAsker = asker; }
    // And Paste Special: how to paste. A dialog unless this stands in for it (tests); false gives it up.
    void setPasteAsker(const std::function<bool(crunch::EditSession::PasteOptions *)> &asker) { m_pasteOptionsAsker = asker; }
    void setChangedAsker(const std::function<ChangedAnswer(const QString &name)> &asker) { m_changedAsker = asker; }
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
    void createDisabledActions();
    void buildRibbon();
    void ribbonParts(RibbonParts &parts);
    void buildCentral();
    void buildStatusBar();
    void updateTitle();
    void updateBars();
    void showBlank();
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
    void notice(const QString &title, const QString &text, bool warning = true);
    QString helpContext() const;
    HelpWindow *showHelpOver(const QString &topic);

    // the page commands and the Page Layout tab (crunchwindow_page.cpp)
    void createPageActions();
    void pageRibbonParts(RibbonParts &parts);
    void connectPageSetup();
    void updatePageActions();
    // Changes how the sheet in view prints: one step of Undo named `text`; what is wrong is told.
    void changePage(const QString &text, const std::function<void(jp::office::sheet::PageSetup *)> &change);
    std::function<bool(PageSetupRequest *)> m_pageSetupAsker;
    std::function<void(QPrinter &)> m_printerPreparer;
    std::function<bool(int, int)> m_progressHook;
    std::unique_ptr<crunch::PrintChoices> m_printChoices;
    std::unique_ptr<QPrinter> m_printer;
    bool m_breaksArmed = false;
    QTimer m_breaksTimer;
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
    QString m_formulaFull;    // what the cell with the cursor holds, whole (the formula bar shows a part of a long text until it has the focus)
};

} // namespace jp
