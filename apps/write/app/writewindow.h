#pragma once
// JeffWrite's main window: the ribbon, the pages, and the status bar, with the File screen over them.

#include "app/appwindow.h"
#include "core/reviewops.h"
#include "core/styleops.h"
#include "core/tableops.h"
#include "core/writedoc.h"

#include <QHash>
#include <QKeySequence>
#include <QTimer>
#include <functional>
#include <memory>

class QActionGroup;
class QDockWidget;
class QLabel;
class QMenu;
class QPrinter;
class QSlider;
class QToolButton;

namespace jp {

class ColorButton;
class FontCombo;
class HelpView;
class HelpWindow;
class MeasureSpin;
class Ribbon;
struct RibbonParts;
class SizeCombo;
class StylesPane;
class WriteFileScreen;
class WriteFindPane;
class ReviewPane;
namespace write {
class PageView;
class Ruler;
class SpellPane;
}
namespace recovery {
struct Recovered;
}

// What Print and Export to PDF are asked to do: which pages and, for printing, where and how many copies.
struct WriteOutputOptions {
    enum class Range { All, Current, Pages };
    Range range = Range::All;
    QString pages;        // "1-3, 5": for Range::Pages
    QString printer;      // the name of the printer; empty is the default one
    bool toFile = false;  // a PDF file, asked for where it is printed, in place of a printer
    int copies = 1;
    bool collate = true;  // the whole set again for each copy, else each page's copies together
    bool grayscale = false;
};

class WriteWindow : public AppWindow {
    Q_OBJECT
public:
    explicit WriteWindow(QWidget *parent = nullptr);
    ~WriteWindow() override;

    QAction *act(const QString &id) const override;
    QStringList actionIds() const override;
    Ribbon *ribbon() const override { return m_ribbon; }
    write::PageView *view() const { return m_view; }
    // Brings the commands up to date with the selection at once (the window does it a moment after a change; a test does not wait for the moment).
    void refreshNow() { refreshUi(); }
    write::WriteDoc *document() const { return m_doc.get(); }
    QString ribbonErrors() const { return m_ribbonErrors; }

    // ---- the file
    // The name the title shows: the file's, or "Document1" for one never saved.
    QString displayName() const;
    QString filePath() const { return m_path; }
    bool isModified() const;
    // Replaces the document shown (as a new window's blank one, or one just read); the path is "" for an unsaved one.
    void setDocument(std::unique_ptr<write::WriteDoc> doc, const QString &path);
    // Opens a .docx in this window if it holds a new document nobody has touched, else in a window of its own. False, with the reason
    // shown, if the file can't be read.
    bool openFile(const QString &path);
    // Saves to the file the document came from, else asks where (Save As). False if nothing was written.
    bool save();
    bool saveAs();
    // Writes the file; the document is saved and takes the file's name. On failure says why and leaves it unsaved.
    bool saveTo(const QString &path);
    // What Insert > Picture asks for a file with: the file dialog, unless this stands in for it (tests): the path chosen, empty for none.
    void setPictureChooser(const std::function<QString()> &chooser) { m_pictureChooser = chooser; }
    write::Ruler *ruler() const { return m_ruler; }
    // What Save As asks for a name with: the file dialog, unless this stands in for it (tests): the name to suggest and the filter to
    // start on in, the path chosen out (empty for none).
    void setSavePathChooser(const std::function<QString(const QString &suggested, const QString &filter)> &chooser) { m_pathChooser = chooser; }
    // What a short message to the person (a sentence said at Save, or on a refusal to write over a file) is shown with: a message box, unless this stands in for it
    // (tests): the title and the sentence.
    void setNotifier(const std::function<void(const QString &title, const QString &text)> &notifier) { m_notifier = notifier; }
    // What a printer is made ready with after the window has set it up (tests: a printer that writes to a file stands in for a real one).
    void setPrinterPreparer(const std::function<void(QPrinter &)> &preparer) { m_printerPreparer = preparer; }
    // AutoRecover: a copy of unsaved work (the timer writes it at the interval Options sets), and the copies a run that crashed left.
    void autoRecover();
    QString recoveryCopy() const { return m_recoveryCopy; }
    void offerRecovery(bool askIfNone = false);
    WriteWindow *openRecovered(const recovery::Recovered &r, QString *error = nullptr);

    // The File screen over the window, on a page ("new", "open", "print", "export", "about"; Open when none is named), and back.
    void showFileScreen(const QString &page = QString());
    void hideFileScreen();
    bool printOutput(const WriteOutputOptions &options);
    bool exportPdf();
    // The pages that an output covers (from 0), for the options and the document.
    QVector<int> outputPages(const WriteOutputOptions &options);

    // The Spelling pane on the right of the window, begun at the caret.
    void showSpelling();
    void showHelp(const QString &topic = QString());
    HelpView *helpView() const { return m_help; }

    // The Styles pane on the right (made when first asked for, so null before).
    void showStylesPane();
    StylesPane *stylesPane() const { return m_styles; }

    // The Reviewing pane on the right (made when first asked for, so null before): Review > Reviewing Pane, with the page ("comments" or "changes") it is to show.
    void showReviewPane(bool changes = false);
    ReviewPane *reviewPane() const { return m_review; }
    // Display for Review: how the document shows its tracked changes and comments. Not saved with the document.
    enum class ReviewView { SimpleMarkup, AllMarkup, NoMarkup, Original };
    ReviewView reviewView() const { return m_reviewView; }
    void setReviewView(ReviewView view);

    // The Format Painter: picked up (Once, or Locked by a double click on its button) it holds the look of the selection, and paints it on each selection made with the mouse until it
    // paints once, or until Esc. Turning it on copies the look of the selection as it is then.
    enum class PainterMode { Off, Once, Locked };
    PainterMode painterMode() const { return m_painter; }
    void setPainterMode(PainterMode mode);
    // Paints the selection with the look held (what the end of a selection by the mouse does while the painter is on).
    void paintFormatNow();

    // The Find and Replace pane on the right (made when first asked for, so null before); `replace` puts the cursor in Replace with.
    void showFindPane(bool replace = false);
    WriteFindPane *findPane() const { return m_find; }

protected:
    void closeEvent(QCloseEvent *e) override;
    void dragEnterEvent(QDragEnterEvent *e) override;
    void dropEvent(QDropEvent *e) override;
    void openDropped(const QStringList &paths);
    void resizeEvent(QResizeEvent *e) override;
    bool eventFilter(QObject *o, QEvent *e) override;

private:
    QAction *mk(const QString &id, const QString &text, const QString &iconName, const QKeySequence &key, const std::function<void()> &fn, bool checkable = false);
    void createActions();
    void createFormatActions();
    void createLayoutActions();
    void createViewActions();
    void createFileActions();
    void createHelpActions();
    void createReviewActions();
    void applyReviewView();
    void refreshReviewUi();
    void updateReviewState();
    void newComment();
    void gotoChange(bool forward);
    void gotoComment(bool forward);
    int resolveShown(bool accept);
    QVector<write::Revision> shownRevisions() const;
    void addReviewContextMenu(QMenu &menu);
    void createEditingActions();
    void createHeaderFooterActions();
    void createNoteActions();
    void createFieldActions();
    void updateFieldsNow();
    void createTableActions();
    void createPictureActions();
    void createStyleActions();
    void showStyleDialog(bool create);
    void pictureRibbonParts(RibbonParts &parts);
    void insertPictureFromFile();
    void insertPictureFromClipboard();
    void showPageSetupDialog();
    void refreshPictureUi();
    void addPictureContextMenu(QMenu &menu);
    void tableRibbonParts(RibbonParts &parts);
    void tableCommand(const std::function<void(write::WriteDoc &, QTextCursor &)> &fn);
    void insertTableNow(const write::NewTable &table);
    void refreshTableUi();
    bool cursorInTable() const;
    void addTableContextMenu(QMenu &menu);
    void headerFooterModeChanged();
    void buildRibbon();
    void ribbonParts(RibbonParts &parts);
    void buildStatusBar();
    void refreshUi();
    void updateTitle();
    bool maybeSave();
    void notice(const QString &title, const QString &text, bool warning = false);
    void dropRecoveryCopy();
    void changeSection(const std::function<void(flow::Section &)> &change);
    void applyIndentFields();
    void showFontDialog();
    void showParagraphDialog();
    void showZoomDialog();
    void showContextMenu(const QPoint &globalPos);
    // Where a PDF is to be written: asked for, or what the stand-in (tests) gives. Empty when none was chosen.
    QString pdfPathToWrite(const QString &title);
    QString helpContext() const;
    HelpWindow *showHelpOver(const QString &topic);
    void updateStatus();
    void updateWords();

    ReviewPane *m_review = nullptr;
    QDockWidget *m_reviewDock = nullptr;
    ReviewView m_reviewView = ReviewView::AllMarkup;
    bool m_showInsertionsAndDeletions = true, m_showFormatting = true, m_showComments = true;
    int m_balloons = 0;   // 0 revisions inline, 1 in balloons, 2 only comments and formatting in balloons
    QActionGroup *m_viewGroup = nullptr, *m_balloonGroup = nullptr;
    QToolButton *m_trackLabel = nullptr;
    bool m_updatingReview = false;
    // Whether the text has changes and comments (looked for again when the text changed: at once in a short document, else a moment after the last edit).
    bool m_hasRevisions = false, m_hasComments = false;
    int m_reviewStateRevision = -1;
    const void *m_reviewStateDoc = nullptr;
    QTimer m_reviewStateTimer;
    std::unique_ptr<write::WriteDoc> m_doc;
    write::PageView *m_view = nullptr;
    Ribbon *m_ribbon = nullptr;
    WriteFileScreen *m_fileScreen = nullptr;
    HelpView *m_help = nullptr;
    QDockWidget *m_helpDock = nullptr;
    QDockWidget *m_spellDock = nullptr;
    write::SpellPane *m_spellPane = nullptr;
    WriteFindPane *m_find = nullptr;
    QDockWidget *m_findDock = nullptr;
    QLabel *m_pageLabel = nullptr, *m_wordLabel = nullptr, *m_zoomLabel = nullptr;
    QSlider *m_zoomSlider = nullptr;
    bool m_noteLimitShown = false;   // the status bar has said that the document holds the most notes
    QList<FontCombo *> m_fontCombos;
    QList<SizeCombo *> m_sizeCombos;
    QList<ColorButton *> m_colorButtons;
    QList<MeasureSpin *> m_indentFields;
    QWidget *m_styleGallery = nullptr;
    StylesPane *m_styles = nullptr;
    QDockWidget *m_stylesDock = nullptr;
    PainterMode m_painter = PainterMode::Off;
    qint64 m_painterPicked = 0;   // when the painter was picked up (milliseconds since the epoch)
    write::FormatBrush m_brush;
    QList<MeasureSpin *> m_pictureFields;   // the height and width of the Picture Format tab
    write::Ruler *m_ruler = nullptr;
    QList<MeasureSpin *> m_tableFields;     // the row height and column width of the Table Layout tab
    QWidget *m_tableStyleGallery = nullptr;
    QHash<QString, QAction *> m_actions;
    QString m_ribbonErrors;
    QString m_path;
    QString m_untitled;
    QString m_recoveryCopy, m_recoveredFrom;
    QTimer m_refreshTimer, m_recoverTimer, m_wordTimer;
    quint64 m_serial = 0;
    bool m_updatingFields = false;
    std::function<QString(const QString &, const QString &)> m_pathChooser;
    std::function<QString()> m_pictureChooser;
    std::function<void(const QString &, const QString &)> m_notifier;
    std::function<void(QPrinter &)> m_printerPreparer;
    bool m_macrosNoted = false;     // (the person was told that the macros are not saved)
    QString m_protectedPath;        // the file a document that has had something left out (see WriteDoc::leftSomethingOut) was opened from: not written over
};

} // namespace jp
