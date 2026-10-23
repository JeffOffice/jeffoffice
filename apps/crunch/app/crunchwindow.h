#pragma once
// JeffCrunch's main window: the ribbon, the name box and formula bar, the grid, the sheet tabs, and the status bar, with the File screen over them. It opens a workbook on a
// worker (WorkbookLoader), shows the first rows at once, and when the whole workbook is read works its formulas out a few milliseconds at a time. It shows; it does not edit yet.

#include "app/appwindow.h"
#include "core/loader.h"
#include "grid/gridview.h"
#include "grid/valuestore.h"

#include <QElapsedTimer>
#include <QHash>
#include <QKeySequence>
#include <QTimer>
#include <atomic>
#include <functional>
#include <memory>

class QDockWidget;
class QLabel;
class QLineEdit;
class QSlider;

namespace jp {

class CrunchFileScreen;
class HelpView;
class HelpWindow;
class Ribbon;
struct RibbonParts;
namespace crunch {
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
    bool isCalculating() const { return m_calculating; }
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
    // The most values one batch had (what taking a batch costs the window is in proportion to it, whatever the machine's speed).
    qint64 longestBatchItems() const { return m_longestBatchItems; }
    // For tests: slows the worker (see WorkbookLoader::setPace).
    void setWorkerPace(int micros) { m_loader.setPace(micros); }
    const crunch::LoadedBook *loaded() const { return m_loaded.get(); }
    // The workbook as read so far, held by whoever keeps it (a test, after the window is gone).
    std::shared_ptr<crunch::LoadedBook> loadedBook() const { return m_loaded; }

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
    // The first rows are on the screen.
    void firstScreenShown();
    // The workbook is read whole (or could not be opened).
    void loadFinished(bool ok);
    // Every formula has its value.
    void calculationFinished();

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
    void stopCalculation();
    void setStatus(const QString &text);
    void goToName();
    void copySelection();
    void copyStep();
    void freezeAtSelection();
    void notice(const QString &title, const QString &text, bool warning = true);
    QString helpContext() const;
    HelpWindow *showHelpOver(const QString &topic);

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
    qint64 m_longestBatchMicros = 0, m_batches = 0, m_longestBatchItems = 0;
    std::function<void(const QString &, const QString &)> m_notifier;
    QString m_loadProblem;
    QString m_formulaFull;    // what the cell with the cursor holds, whole (the formula bar shows a part of a long text until it has the focus)
};

} // namespace jp
