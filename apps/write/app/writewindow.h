#pragma once
// JeffWrite's main window: the ribbon, the pages, and the status bar, with the File screen over them.

#include "app/appwindow.h"
#include "app/infopage.h"
#include "core/textexport.h"
#include "app/filetasks.h"
#include "core/reviewops.h"
#include "core/styleops.h"
#include "core/tableops.h"
#include "core/writedoc.h"
#include "core/compareops.h"
#include "core/homeops.h"
#include "app/versionstore.h"
#include "app/optionsdialog.h"

#include <QHash>
#include <QImage>
#include <QSizeF>
#include <QPointer>
#include <QKeySequence>
#include <QSet>
#include <QTimer>
#include <functional>
#include <memory>

class QActionGroup;
class QDockWidget;
class QFrame;
class QLabel;
class QMenu;
class QPrinter;
class QSlider;
class QSplitter;
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
class NavigationPane;
class WriteOutlineView;
class WriteFileScreen;
class WriteFindPane;
struct MailUi;
class ReviewPane;
namespace write {
class PageView;
class Ruler;
class VerticalRuler;
class SpellPane;
class AccessPane;
}
namespace recovery {
struct Recovered;
}

// What Print and Export to PDF are asked to do: which pages and, for printing, where and how many copies.
struct WriteOutputOptions {
    enum class Range { All, Current, Pages, Selection };
    Range range = Range::All;
    QString pages;        // "1-3, 5": for Range::Pages
    QString printer;      // the name of the printer; empty is the default one
    bool toFile = false;  // a PDF file, asked for where it is printed, in place of a printer
    int copies = 1;
    bool collate = true;  // the whole set again for each copy, else each page's copies together
    bool grayscale = false;
    enum class Sides { All, Odd, Even };
    Sides sides = Sides::All;   // only the pages whose numbers are odd or even, of the pages the range covers
    bool reverse = false;       // last page first
    bool byHand = false;        // both sides of the paper with a printer that has no duplex: the odd pages, a prompt to turn the stack over, then the even pages last first
    bool pdfArchive = false;    // Export to PDF: a PDF/A-1b file, for keeping (fonts embedded, no transparency, and the standard's mark in its metadata)
    int perSheet = 1;           // pages on a sheet: 1, 2, 4, 6, 9, or 16 (each scaled to fit its place)
    QSizeF scaleTo;             // points, upright: each page is scaled to fit this paper (empty: the page's own)
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
    // Fills a menu with what a right click offers where the pointer was (or the caret): the commands for text, and for a link, a field, a note, a picture, a table, or a change under it.
    void fillContextMenu(QMenu &menu);
    // Brings the commands up to date with the selection at once (the window does it a moment after a change; a test does not wait for the moment).
    void refreshNow() { refreshUi(); }
    write::WriteDoc *document() const { return m_doc.get(); }
    // Review > Compare: this document and the file at `revisedPath` made into a new document of tracked changes (this one is the original), shown in a window of its own. How many changes it
    // holds; -1 when a file can't be read or compared (the reason is told).
    bool openTranslation(const QString &text, const QString &target, const QString &server);
    void setUrlOpener(const std::function<bool(const QUrl &)> &opener) { m_urlOpener = opener; }
    int sortSelection(const write::SortOptions &options);
    int compareWith(const QString &revisedPath, const write::CompareOptions &options);
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
    // `how`: as it is (the default), read-only, as a copy (an unsaved document made from the file; the file is never written), or in a window of its own even if this one holds nothing.
    bool openFile(const QString &path, OpenHow how = OpenHow::Normal);
    // Shows the file dialog and opens what is chosen the way `how` says.
    void browseAndOpen(OpenHow how = OpenHow::Normal);
    void setReadOnly(bool on) override;
    // Saves to the file the document came from, else asks where (Save As). False if nothing was written.
    bool save();
    bool saveAs();
    // Save a Copy: asks for a name as Save As does, writes the document there, and goes on with the original (its name, and the changes it has not saved).
    bool saveCopy();
    // Save as Template: asks for a name and puts a template made from the document in My Templates (File > New); the document is as it was.
    bool saveAsTemplate();
    // Writes the file; the document is saved and takes the file's name (not for a copy, which is only written). On failure says why and leaves it unsaved.
    bool saveTo(const QString &path, bool asCopy = false);
    // What Insert > Picture asks for a file with: the file dialog, unless this stands in for it (tests): the path chosen, empty for none.
    // What a followed link is opened with: the system's browser or mail program, unless this stands in for it (tests): the address.
    // What Insert > Text from File asks for a file with: the file dialog, unless this stands in for it (tests): the path chosen, empty for none.
    void setTextFileChooser(const std::function<QString()> &chooser) { m_textFileChooser = chooser; }
    // What asks the person before a link is followed that needs it (tests stand in for the message box): the question; true to go on.
    void setLinkAsker(const std::function<bool(const QString &)> &asker) { m_linkAsker = asker; }
    // What asks for a field's new code when it is edited (tests stand in for the box): the field's code now; the new code, empty for none.
    void setFieldCodeAsker(const std::function<QString(const QString &)> &asker) { m_fieldCodeAsker = asker; }
    // What asks for the hyphenation zone and limit (tests stand in for the box): changes them in place, true to take them.
    void setHyphenationAsker(const std::function<bool(double &, int &)> &asker) { m_hyphenationAsker = asker; }
    // What asks for the alignment tab to put in a header or footer (tests stand in for the box): its alignment, what it is measured from, and its leader, changed in place; true to take them.
    void setAlignmentTabAsker(const std::function<bool(QString &, QString &, QString &)> &asker) { m_alignmentTabAsker = asker; }
    void setLinkOpener(const std::function<void(const QString &)> &opener) { m_linkOpener = opener; }
    void setPictureChooser(const std::function<QString()> &chooser) { m_pictureChooser = chooser; }
    write::Ruler *ruler() const { return m_ruler; }
    // Shows the vertical ruler when View > Vertical Ruler is on and the pages are on show (not in Outline view or Focus).
    void syncVerticalRuler();
    // What Save As asks for a name with: the file dialog, unless this stands in for it (tests): the name to suggest and the filter to
    // start on in, the path chosen out (empty for none).
    void setSavePathChooser(const std::function<QString(const QString &suggested, const QString &filter)> &chooser) { m_pathChooser = chooser; }
    // What a short message to the person (a sentence said at Save, or on a refusal to write over a file) is shown with: a message box, unless this stands in for it
    // (tests): the title and the sentence.
    void setNotifier(const std::function<void(const QString &title, const QString &text)> &notifier) { m_notifier = notifier; }
    // What asks how a plain text file is read (at Open) or written (at Save As), unless this stands in for it (tests): the form to start from and out, and the file's bytes when reading; true to go on.
    void setTextFormChooser(const std::function<bool(write::TextForm *, const QByteArray *)> &chooser) { m_textFormChooser = chooser; }
    // What asks the person before a save to a format that holds less than the document (the parts it would lose, listed): Keep saves it there anyway, Docx
    // saves a .docx beside it instead, Cancel saves nothing. A message box, unless this stands in for it (tests). Without it, and with a notifier standing in
    // for the messages, the save goes ahead and says afterward what was left out.
    enum class FormatChoice { Keep, Docx, Cancel };
    void setFormatAsker(const std::function<FormatChoice(const QStringList &)> &asker) { m_formatAsker = asker; }
    // The pages JeffWrite adds to Options after the shared ones (Save, Display); what they set applies to this window at once and to each new one.
    QVector<OptionsPage> optionsPages();
    // The folder the Open and Save As boxes start in when no file leads them: the one Options > Save names, else Documents.
    static QString defaultFolder();
    // Starts an untitled document from one of the templates JeffWrite comes with (by its id, see core/templates.h): in this window if it holds a new document nobody has touched,
    // else in a window of its own. False for an id that names none.
    bool newFromBuiltInTemplate(const QString &id);
    // The two runs a print on both sides by hand is made of: the odd pages of the range, then the even ones, last first.
    QVector<WriteOutputOptions> handDuplexRuns(const WriteOutputOptions &options) const;
    // What asks the person to turn the stack over between the two runs: a message box, unless this stands in for it (tests); false to stop there.
    void setStackTurner(const std::function<bool()> &turner) { m_stackTurner = turner; }
    // Email: makes a message as a .eml file (asking where to put it) with the document, the document as a PDF, or its pages as pictures; the person opens it in an email program.
    enum class EmailWhat { Document, Pdf, Pages };
    bool exportEmail(EmailWhat what);
    // The pages (by index) as a PDF, in memory; empty, with the reason, when they can't be made.
    QByteArray pdfBytes(const QVector<int> &pages, QString *error, bool archive = false);
    // Version history: named copies of the document kept on this computer (see app/versionstore.h). Each needs a file the document has been saved as.
    QVector<write::DocVersion> versions() const;
    bool saveVersion(const QString &name);
    // The document becomes the version (what it was is kept first, as a version, so that nothing is lost); it is then changed, to be saved.
    bool restoreVersion(const QString &file);
    // The version and the document compared, in a new window (as Review > Compare makes it); the changes found, -1 when it can't be done.
    int compareWithVersion(const QString &file);
    // Version History on the Info page: the list with its buttons.
    void versionHistory();
    // Export to PDF with the pages the options name (the range only: the printer choices are for Print); false, with nothing written, when no page is named.
    bool exportPdfPages(const WriteOutputOptions &options);
    // Asks which pages (all, the current page, or a range), then exports them.
    bool exportPdfAsk();
    // A copy of the document with only the selection left in it (its page setup, styles, headers, and footers stay), for printing the selection; null when nothing is selected.
    std::unique_ptr<write::WriteDoc> selectionDocument();
    // The page `index` of what `options` prints, as a picture `width` pixels wide (the selection's own pages for Range::Selection), and its size in points.
    QImage outputPageImage(const WriteOutputOptions &options, int index, int width);
    QSizeF outputPageSize(const WriteOutputOptions &options, int index);
    // Where editing stopped: the place is kept with the file's name when the window closes and when the file is saved, and the file opens there again.
    void rememberPlace();
    // The last three places the text was edited, the latest first (Shift+F5 goes to each in turn).
    QList<int> editPlaces() const { return m_editPlaces; }
    void resetEditPlaces();
    void goToLastPlace();
    // What a printer is made ready with after the window has set it up (tests: a printer that writes to a file stands in for a real one).
    void setPrinterPreparer(const std::function<void(QPrinter &)> &preparer) { m_printerPreparer = preparer; }
    // Automatic recovery: a copy of unsaved work (the timer writes it at the interval Options sets), and the copies a run that crashed left.
    void autoRecover();
    QString recoveryCopy() const { return m_recoveryCopy; }
    void offerRecovery(bool askIfNone = false);
    WriteWindow *openRecovered(const recovery::Recovered &r, QString *error = nullptr);

    // ---- the File screen's Info page: properties, protection, and Inspect
    // The page, for the document as it is now.
    QWidget *buildInfoPage(QWidget *parent);
    // What Inspect Document looks for and can take out; the box shows them.
    QVector<InspectItem> inspectItems();
    void inspectDocument();
    // Lists what an .odt file (the older kind this program writes) can't hold of the document.
    QStringList incompatibilities();
    void checkCompatibility();
    // Mark as Final: the document is saved, then held for reading only; off, it can be edited again.
    void setMarkedFinal(bool on);
    // Encrypt with Password: asks for the password and keeps it; the file is written protected when it is saved.
    void encryptWithPassword();
    bool isPasswordProtected() const { return m_fileProtected; }
    // The reason Mark as Final can't be done for this document, "" when it can.
    QString markFinalBlocked() const;
    // Takes the personal information out of the properties, and keeps it out (a later change of a property does not name the person again).
    int removePersonalInformation();
    // Applies properties the person edited (the dates and counts are brought up to date).
    void applyProperties(const office::PackageProps &props);

    // The File screen over the window, on a page ("new", "open", "print", "export", "about"; Open when none is named), and back.
    void showFileScreen(const QString &page = QString());
    void hideFileScreen();
    bool printOutput(const WriteOutputOptions &options);
    bool exportPdf();
    // Save as Plain Text: asks for the encoding, the line ends, and the file; and the writing itself (an empty path asks nothing and writes nothing).
    bool exportTextAsk();
    bool exportText(const QString &path, write::TextEncoding encoding, write::LineEnds ends, int codePage = 1252);
    // Save as Web Page (the page with its looks and pictures in one file) or as Filtered Web Page (the structure alone): asks for the file, and the writing itself.
    bool exportWebAsk(bool singleFile);
    bool exportWeb(const QString &path, bool singleFile);
    // Asks how a text file is read or written (the encoding and line ends, with a preview of `sample`, the file's bytes, if given); false if cancelled. Tests stand in for the dialog.
    bool askTextForm(const QString &title, const QString &intro, write::TextForm *form, const QByteArray *sample = nullptr);
    // Asks what the message holds (the document, a PDF, or the pages), then makes it as exportEmail does.
    bool exportEmailAsk();
    // The pages that an output covers (from 0), for the options and the document.
    QVector<int> outputPages(const WriteOutputOptions &options);

    // The Spelling pane on the right of the window, begun at the caret.
    void showSpelling();
    void showHelp(const QString &topic = QString());
    HelpView *helpView() const { return m_help; }

    // The Navigation pane on the left (made when first shown, so null before): View > Navigation Pane.
    void showNavigationPane(bool show = true);
    NavigationPane *navigationPane() const { return m_nav; }
    // Focus mode: the ribbon, ruler, status bar, and panes hidden and the window full screen (View > Focus, F11; Esc leaves it).
    void setFocusMode(bool on);
    bool focusMode() const { return m_focusMode; }
    // Read Mode (View > Read Mode): Focus mode with the pages side by side, each fitted to the window, and nothing typed; Esc, F11, or the command again leaves it.
    bool readMode() const { return m_readMode; }
    void setReadMode(bool on);
    // Outline view (View > Outline): the paragraphs as a tree to promote, demote, and move, in place of the pages; the Outlining tab is there while it is.
    bool outlineMode() const { return m_outlineOn; }
    void setOutlineMode(bool on);
    // Draft (View > Draft): the pages run on without edges, headers, or footers; Print Layout and Outline each end it.
    void setDraftView(bool on);
    void setWebView(bool on);
    WriteOutlineView *outlineView() const { return m_outline; }
    // View > Split: a second pane under the pages shows the same document, scrolled and zoomed on its own, for looking at one place while writing in another. It does not take typing.
    write::PageView *splitView() const { return m_split; }
    void setSplit(bool on);
    // View > Window: windows of JeffWrite placed side by side with their scrolling tied together (the other window is `sideBySidePartner()`), and the windows placed in rows or a stack.
    void setSideBySide(bool on);
    WriteWindow *sideBySidePartner() const { return m_sidePartner; }
    static QList<WriteWindow *> writeWindows();
    static void arrangeAllWindows();
    static void cascadeWindows();

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

    // Copy Formatting: picked up (Once, or Locked by a double click on its button) it holds the look of the selection, and paints it on each selection made with the mouse until it
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
    void addLinkFieldNoteContextMenu(QMenu &menu);
    void createEditingActions();
    void createHeaderFooterActions();
    void createNoteActions();
    void createFieldActions();
    void createRefsActions();
    void reloadDocument();
    void updateFieldsNow();
    void createTableActions();
    void createPictureActions();
    void createShapeActions();
    void createPictureAdjustActions();
    void insertEquationDialog();
    void createChartActions();
    void chartRibbonParts(RibbonParts &parts);
    void refreshChartUi();
    void editChartData();
    void editChartTitle();
    void editChartAxisTitle(bool values);
    void cropSelectedPicture();
    void changeSelectedPicture();
    void shapeRibbonParts(RibbonParts &parts);
    void refreshShapeUi();
    void editShapeText();
    void turnSelectedShape(double degrees, bool relative);
    void alignSelectedObject(int align);
    void gradientSelectedShape(double angle, bool twoColors);
    void createInsertActions();
    void createLayoutExtraActions();
    void createViewExtraActions();
    void createOutlineActions();
    void outlineEdited();
    void refreshViewUi();
    void layoutRibbonParts(RibbonParts &parts);
    void refreshLayoutUi();
    void insertRibbonParts(RibbonParts &parts);
    void refreshInsertUi();
    void createStyleActions();
    void createHomeActions();
    void createProtectActions();
    void restrictEditing(bool lockTracking);
    void applyProtection();
    bool protectionAllows(const QString &id) const;
    void updateLanguage();
    void createMailActions();
    void mailRibbonParts(RibbonParts &parts);
    void refreshMailUi();   // (the Mailings commands that need a list of recipients are dimmed without one)
    friend struct MailUi;
    std::shared_ptr<void> m_mailHolder;   // (the Mailings tab: app/writewindow_mail.cpp)
    MailUi *m_mail = nullptr;
    void showStyleDialog(bool create);
    void pictureRibbonParts(RibbonParts &parts);
    void headerRibbonParts(RibbonParts &parts);
    void removeEdited(bool footer);
    void insertPictureFromFile();
    void fillSelectedShapeWithPicture();
    void insertPictureFromClipboard();
    void insertIconsFromPicker();
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
    void watchCopyFormatting(QWidget *root);
    void ribbonParts(RibbonParts &parts);
    void buildStatusBar();
    void refreshUi();
    // The mini toolbar: the common character and paragraph commands in a small bar that floats above words selected with the mouse.
    void createMiniToolbar();
    void updateMiniToolbar(bool show);
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
    QDockWidget *m_accessDock = nullptr;
    write::AccessPane *m_accessPane = nullptr;
    write::SpellPane *m_spellPane = nullptr;
    WriteFindPane *m_find = nullptr;
    QDockWidget *m_findDock = nullptr;
    QToolButton *m_langLabel = nullptr;
    QLabel *m_sectionLabel = nullptr;
    QLabel *m_pageLabel = nullptr, *m_wordLabel = nullptr, *m_zoomLabel = nullptr;
    QSlider *m_zoomSlider = nullptr;
    bool m_noteLimitShown = false;   // the status bar has said that the document holds the most notes
    QList<FontCombo *> m_fontCombos;
    QList<SizeCombo *> m_sizeCombos;
    QList<ColorButton *> m_colorButtons;
    QList<ColorButton *> m_pageColorButtons;   // (Design > Page Color)
    QList<MeasureSpin *> m_indentFields;
    QWidget *m_styleGallery = nullptr;
    std::function<void(const QString &)> m_linkOpener;
    std::function<QString(const QString &)> m_fieldCodeAsker;
    std::function<bool(double &, int &)> m_hyphenationAsker;
    std::function<bool(QString &, QString &, QString &)> m_alignmentTabAsker;
    std::function<bool(const QString &)> m_linkAsker;
    std::function<QString()> m_textFileChooser;
    NavigationPane *m_nav = nullptr;
    QDockWidget *m_navDock = nullptr;
    bool m_focusMode = false;
    WriteOutlineView *m_outline = nullptr;
    QSplitter *m_splitter = nullptr;
    write::PageView *m_split = nullptr;
    bool m_outlineOn = false, m_outlineBusy = false;
    struct FocusWas {
        bool fullScreen = false, ribbon = true, ruler = true, status = true;
        QList<QPointer<QDockWidget>> docks;
    } m_focusWas;
    bool m_readMode = false;
    QPointer<WriteWindow> m_sidePartner;   // View > View Side by Side: the window this one's scrolling follows, and follows this one's
    QMetaObject::Connection m_sideScroll[2];
    struct {
        bool readOnly = false, sideToSide = false;
        int zoomMode = 0;
        double zoom = 1;
    } m_readWas;
    StylesPane *m_styles = nullptr;
    QDockWidget *m_stylesDock = nullptr;
    PainterMode m_painter = PainterMode::Off;
    qint64 m_painterPicked = 0;   // when the painter was picked up (milliseconds since the epoch)
    write::FormatBrush m_brush;
    QList<MeasureSpin *> m_pictureFields;   // the height and width of the Picture Format tab
    QList<MeasureSpin *> m_shapeFields;     // the height and width of the Shape Format tab
    write::Ruler *m_ruler = nullptr;
    write::VerticalRuler *m_vruler = nullptr;
    QList<MeasureSpin *> m_hfFields;        // the header and footer distances of the Header & Footer tab
    QList<MeasureSpin *> m_tableFields;     // the row height and column width of the Table Layout tab
    QWidget *m_tableStyleGallery = nullptr;
    QHash<QString, QAction *> m_actions;
    QString m_ribbonErrors;
    QString m_path;
    QString m_untitled;
    QString m_recoveryCopy, m_recoveredFrom;
    write::SortOptions m_sortOptions;
    QString m_protectKind;   // what the enforced protection of the document allows (readOnly, comments, trackedChanges, or forms), else empty
    bool m_readOnlyByProtection = false;
    QString m_titledKind;
    QTimer m_refreshTimer, m_recoverTimer, m_wordTimer;
    quint64 m_serial = 0;
    bool m_updatingFields = false;
    QFrame *m_mini = nullptr;
    std::function<QString(const QString &, const QString &)> m_pathChooser;
    std::function<QString()> m_pictureChooser;
    std::function<bool(const QUrl &)> m_urlOpener;
    std::function<void(const QString &, const QString &)> m_notifier;
    std::function<void(QPrinter &)> m_printerPreparer;
    std::function<FormatChoice(const QStringList &)> m_formatAsker;
    std::function<bool()> m_stackTurner;
    std::unique_ptr<write::WriteDoc> m_selectionDoc;   // the copy that the last count of the pages to print for the selection was made from
    QList<int> m_editPlaces;
    int m_placeStep = 0;
    void recordPlace(int position);
    void applyDisplayDefaults();
    void applyEditingDefaults();   // Options > Advanced: how a link is followed, and dragging text
    void returnToPlace(const QString &path);
    static QString placeKey(const QString &path);
    FormatChoice askFormatChoice(const QStringList &lost, const QString &kind = QStringLiteral(".odt"));
    QSet<QString> m_odtNoted;       // (what the person was told an .odt file can't hold: told again only of what is new)
    std::function<bool(write::TextForm *, const QByteArray *)> m_textFormChooser;
    write::TextForm m_textForm;     // how the plain text file this document came from (or was saved to) is written
    bool m_textFormKnown = false;   // (it was chosen: Save writes the same form without asking again)
    bool m_textNoted = false;       // (the person was told that a text file holds only the words)
    bool m_macrosNoted = false;     // (the person was told that the macros are not saved)
    QString m_filePassword;         // the password the file opened with or was given: the file is written protected with it (never shown, wiped when the window goes)
    bool m_fileProtected = false;
    bool m_recoveryNoted = false;       // (the person was told that no recovery copy is kept of a protected document)
    bool m_personalInfoRemoved = false; // (Inspect took the personal information out: saving does not put the name of the person back)
    QString m_protectedPath;        // the file a document that has had something left out (see WriteDoc::leftSomethingOut) was opened from: not written over
};

} // namespace jp
