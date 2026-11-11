#pragma once
// The JeffPub window: ribbon, pages pane, workspace, task panes, status
// bar and the File backstage. Commands live in mainwindow_actions.cpp, the
// ribbon's layout in resources/ribbon.json (see ribbonbuilder.h) and the
// controls it names in mainwindow_ribbon.cpp.

#include "app/appwindow.h"
#include "app/filetasks.h"
#include "app/editor.h"
#include "app/infopage.h"
#include "app/pageaccesspane.h"
#include "app/pageselectionpane.h"
#include "app/versionstore.h"
#include "office/packageprops.h"
#include "io/pdfcrypt.h"

#include <QHash>
#include <QMainWindow>
#include <functional>
#include <QPointer>
#include <QTimer>

class QLabel;
class QSlider;
class QStackedWidget;
class QSplitter;
class QToolButton;
class QComboBox;

namespace jp {
namespace recovery {
struct Recovered;
}

class Canvas;
class Ribbon;
class KeyTips;
class RibbonTab;
class PagesPane;
class TaskPane;
class Backstage;
class HelpView;
class HelpWindow;
class FontCombo;
class SizeCombo;
class ColorButton;
class MeasureSpin;
class Gallery;
class ChartActions;
struct RibbonParts;

class MainWindow : public AppWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    Editor *editor() const { return m_ed; }
    Canvas *canvas() const { return m_canvas; }
    Ribbon *ribbon() const override { return m_ribbon; }
    QAction *act(const QString &id) const override;
    // What went wrong building the ribbon from ribbon.json (empty when nothing did).
    QString ribbonError() const { return m_ribbonError; }
    QStringList actionIds() const override { QStringList ids = m_actions.keys(); ids.sort(); return ids; }

    // File operations (used by the backstage too).
    void newPublication(std::unique_ptr<Document> doc);
    // Page Design > Change Template: `design`, built from the gallery's choices,
    // takes this publication's text and pictures, in this window as one undo
    // step, or, with `toNewWindow`, in a new publication in a window of its
    // own while this one stays as it is. Returns the window with the result.
    MainWindow *changeTemplate(std::unique_ptr<Document> design, bool toNewWindow);
    // `how`: as it is (the default), read-only, as a copy (an unsaved publication made from the file; the file is never written), or in a window of its own.
    bool openFile(const QString &path, OpenHow how = OpenHow::Normal);
    // Shows the file dialog and opens what is chosen the way `how` says (asking about unsaved changes first, unless it opens in a window of its own).
    void browseAndOpen(OpenHow how = OpenHow::Normal);
    void setReadOnly(bool on) override;
    // What a sentence for the person (what Open and Repair left out) is shown with: a message box, unless this stands in for it (tests).
    void setNotifier(const std::function<void(const QString &title, const QString &text)> &notifier) { m_notifier = notifier; }
    // Copies of unsaved work left by a run of JeffPub that didn't close
    // normally: offered for opening or deleting (askIfNone: say so when
    // there are none, from File > Open).
    void offerRecovery(bool askIfNone = false);
    // Opens one recovered copy, in this window while it holds nothing, else
    // in a new one, which is returned (null if the copy can't be read).
    MainWindow *openRecovered(const recovery::Recovered &r, QString *error = nullptr);
    // The automatic recovery copy this window keeps of its document, if any.
    QString recoveryCopy() const { return m_recoveryCopy; }
    KeyTips *keyTips() const { return m_keyTips; }
    void autoRecover();
    // After File > Options: settings this window keeps its own copy of (the
    // automatic recovery interval).
    void settingsChanged();
    int autoRecoverInterval() const { return m_recoverTimer.interval(); }   // ms
    bool save();
    bool saveAs(const QString &format = QString());
    // Save a Copy: asks for a name (or takes `path`), writes the publication there, and goes on with the original (its name and unsaved changes).
    bool saveCopy(const QString &path = QString());
    // `asCopy`: only writes the file; the publication keeps its name and stays as saved or unsaved as it was.
    bool saveTo(const QString &path, bool asCopy = false);
    bool maybeSave();
    // The Info page of the File screen (mainwindow_info.cpp): the publication's properties, Mark as Final, a password (for a .jpub file only), Inspect Publication, and Version History.
    // The properties are kept in the publication, and undone with it.
    QWidget *buildInfoPage(QWidget *parent);
    office::PackageProps properties() const;
    void applyProperties(const office::PackageProps &edited);
    // Takes the personal information out of the properties, and keeps it out (a later change of a property does not name the person again). How many names were there.
    int removePersonalInformation();
    bool markedAsFinal() const;
    QString markFinalBlocked() const;
    // Whether the file asks for a password when it is opened (it was opened with one, or Encrypt with Password was used); the password is put on when the publication is saved. Only a .jpub file
    // can have one.
    bool isPasswordProtected() const { return m_fileProtected; }
    void encryptWithPassword();
    QString filePassword() const { return m_fileProtected ? m_filePassword : QString(); }   // what a copy written for sharing is protected with ("": none)
    QString encryptBlocked() const;   // why the file can't have a password ("" when it can)
    void setMarkedFinal(bool on);
    QVector<InspectItem> inspectItems();
    void inspectPublication();
    QVector<write::DocVersion> versions() const;
    bool saveVersion(const QString &name);
    bool restoreVersion(const QString &file);
    void versionHistory();
    // archival: PDF/A-1b (ISO 19005-1) for long-term storage.
    void exportPdf(const QString &path = QString(), bool merged = false, bool archival = false);
    // Create PDF with choices: quality preset, page range, document properties.
    struct PdfSettings {
        enum Preset { Minimum, Standard, HighQuality, CommercialPress };
        Preset preset = HighQuality;
        int from = 0, to = -1;          // page indexes; to < 0 means the last page
        bool properties = true;         // title, author, subject and keywords
        bool archival = false;          // PDF/A-1b
        bool pdfx = false;              // PDF/X (1a:2001 or 4) for a commercial printer
        int pdfxCondition = 0;          // its version and printing condition (pdfXConditions())
        QString pdfxProfile;            // the printer's own profile (.icc), for a condition that takes one
        bool merged = false;
        bool bookmarks = true;          // a bookmark for each page (its name, or its number)
        bool tags = true;               // the document's structure for screen readers (tagged PDF); left out of PDF/X
        // A password to open the PDF with (the standards PDF/A and PDF/X forbid one, so they are off when it is set); wiped after the export.
        PdfProtection protect;
        // A booklet's PDF holds its printed sheets: two pages to a side, in
        // folding order (as Publisher makes it). Off: one page per PDF page.
        bool booklet = true;
    };
    void exportPdfWithOptions();
    bool exportPdfTo(const QString &path, const PdfSettings &s);
    // Whether a PDF opens in the system's viewer once it is saved (on by
    // default, as the other program's "open file after publishing"), and how
    // it is opened (replaceable for tests; does nothing without a screen).
    static bool openPdfAfterSaving();
    static void setOpenPdfAfterSaving(bool on);
    static std::function<bool(const QString &path)> openFileHook;
    void exportImages();
    // Each page as an SVG drawing: `path` for one page, name-1.svg, name-2.svg... for several.
    bool exportSvgTo(const QString &path, QString *error = nullptr);
    void exportHtml();
    // An EPUB e-book; `cover` makes the first page its cover.
    void exportEpub();
    bool exportEpubTo(const QString &path, const QString &title, const QString &author, bool cover, QString *error = nullptr);
    // Page for page: each page as designed (a fixed-layout EPUB).
    bool exportFixedEpubTo(const QString &path, const QString &title, const QString &author, QString *error = nullptr);
    bool exportHtmlTo(const QString &path);
    void printPublication();
    void showBackstage(const QString &page = QString());
    // The start screen for a start that shows the window before the fonts are added: put up before the window is shown, so the first picture of the window is it (never the empty
    // publication), its frame first (the side panel, the page's place empty) and the page from fillStartScreen(), with the template thumbnails held back (the blank page stands in)
    // until endStartScreen(), which is called once the fonts are added.
    void beginStartScreen(const QString &page);
    void fillStartScreen();   // makes its page (once the first picture of the window is drawn)
    void endStartScreen();
    int backstageThumbnailCount() const;
    int backstageHeldThumbnailTicks() const;
    void hideBackstage();
    void showTaskPane(const QString &name);
    // Review > Check Accessibility: the list of what is hard for some readers to use, which goes to a problem when it is clicked.
    void showAccessibilityPane();
    PageAccessibilityPane *accessibilityPane() const { return m_accessPane; }
    void hideTaskPane();
    // Help on a topic ("index", the contents, when empty): in the Help pane,
    // or in a window of its own over the File page, which covers the pane.
    void showHelp(const QString &topic = QString());
    // The topic for what is being done now: the ribbon tab, task pane or
    // File page the keyboard is in, else the selected object (empty: none).
    QString helpContext() const;
    HelpView *helpView() const;
    // Help over a dialog, which blocks the window and so its Help pane.
    HelpWindow *showHelpOver(QWidget *owner, const QString &topic);
    QString currentTaskPane() const;
    QImage pageThumbnail(int page, int maxSide);
    void insertPictureFromFile(const QString &replaceItemId = QString(), const QPointF &at = QPointF(-1, -1));
    void insertFiles(const QStringList &paths, const QPointF &at, PictureInsert how = PictureInsert::Embed);
    void changePictureLink(const QString &pictureId);
    void editTextArt(const QString &id);
    void insertEquationDialog();
    void editEquation(const QString &id);
    void refreshUi();
    void screenshotTo(const QString &path);

protected:
    void closeEvent(QCloseEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
    bool eventFilter(QObject *o, QEvent *e) override;

private:
    void createActions();
    void createChartActions();    // mainwindow_chart.cpp
    void addChartParts(RibbonParts &parts);
    void refreshChart();
    ChartActions *m_chartActions = nullptr;
    void buildRibbon();
    // The controls and menus ribbon.json names (mainwindow_ribbon.cpp).
    void ribbonControlParts(RibbonParts &parts);
    void ribbonGalleryParts(RibbonParts &parts);
    void ribbonMenuParts(RibbonParts &parts);
    void buildStatusBar();
    void updateTitle();
    void updateContextTabs();
    void contextMenu(const QPoint &global);
    void dropRecoveryCopy();
    QAction *mk(const QString &id, const QString &text, const QString &iconName, const QKeySequence &key, const std::function<void()> &fn,
                bool checkable = false);

    Editor *m_ed;
    QPointer<PageAccessibilityPane> m_accessPane;
    QPointer<SelectionPane> m_selectionPane;
    Canvas *m_canvas;
    Ribbon *m_ribbon;
    KeyTips *m_keyTips = nullptr;
    PagesPane *m_pages;
    TaskPane *m_task;
    Backstage *m_backstage;
    QSplitter *m_split;
    QHash<QString, QAction *> m_actions;
    QString m_ribbonError;

    // Ribbon controls that mirror the selection.
    QVector<FontCombo *> m_fontCombos;
    QVector<SizeCombo *> m_sizeCombos;
    QVector<ColorButton *> m_colorButtons;
    QVector<MeasureSpin *> m_widthSpins, m_heightSpins;
    QVector<MeasureSpin *> m_tableWidthSpins, m_tableHeightSpins;   // Table Layout > Size: the table's own
    Gallery *m_styleGallery = nullptr;
    Gallery *m_schemeGallery = nullptr;

    // Status bar.
    QLabel *m_pageLabel, *m_posLabel, *m_sizeLabel, *m_zoomLabel;
    QString positionText(const QPointF &page) const;   // a place on the page as the rulers count it
    QSlider *m_zoomSlider;
    QTimer m_refreshTimer, m_recoverTimer;
    quint64 m_serial;               // this window, among this run's (names an unsaved document's copy)
    QString m_recoveryCopy;         // this window's automatic recovery copy, if written
    QString m_recoveredFrom;        // a recovered document's original file, for Save As
    std::function<void(const QString &, const QString &)> m_notifier;
    bool m_fileProtected = false;   // the file is saved with a password (a .jpub file's package inside the encrypted container)
    QString m_filePassword;
    write::VersionKey m_versions;   // what the versions kept for the file are protected with (they follow a new password when the file is saved)
    bool m_protectionNoted = false;
    bool m_recovered = false;       // the document is recovered work, not yet saved
    bool m_personalInfoRemoved = false;   // Inspect took the names out of the properties: they stay out until the publication is closed
    void notice(const QString &title, const QString &text, bool warning = false);
    QPointer<QWidget> m_measurement;
};

// Opens documents the system hands the running program: macOS sends a
// FileOpen event for a file double-clicked in the Finder or dropped on the
// Dock icon, not a command-line argument. Installed on the application.
void installFileOpenHandler(QObject *app);

} // namespace jp
