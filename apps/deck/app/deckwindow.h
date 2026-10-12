#pragma once
// JeffDeck's main window: the ribbon, the slide thumbnails, the slide being
// edited with its notes below, and the status bar.

#include "app/appwindow.h"
#include "app/slideshow.h"

#include <QHash>
#include <QKeySequence>
#include <QPointer>
#include <QTimer>
#include <functional>

class QDockWidget;
class QLabel;
class QSlider;
class QSplitter;
class QStackedWidget;

namespace jp {

class ColorButton;
class CommentsPane;
class DeckCanvas;
class DeckFileScreen;
class Editor;
class FontCombo;
class HelpView;
class HelpWindow;
class NotesPane;
class OutlineView;
class Ribbon;
class ReusePane;
struct RibbonParts;
class SizeCombo;
class SlideSorter;
class SlidesPane;

namespace deck {
struct FooterOptions;
struct MediaPlay;
struct OutputOptions;
}
namespace recovery {
struct Recovered;
}

class DeckWindow : public AppWindow {
    Q_OBJECT
public:
    explicit DeckWindow(QWidget *parent = nullptr);
    ~DeckWindow() override;

    QAction *act(const QString &id) const override;
    QStringList actionIds() const override;
    Ribbon *ribbon() const override { return m_ribbon; }
    Editor *editor() const { return m_ed; }
    DeckCanvas *canvas() const { return m_canvas; }
    SlidesPane *slidesPane() const { return m_slides; }
    NotesPane *notesPane() const { return m_notes; }
    CommentsPane *commentsPane() const { return m_comments; }
    SlideShow *slideShow() const { return m_show; }
    SlideSorter *slideSorter() const { return m_sorter; }
    OutlineView *outlineView() const { return m_outline; }
    // The Reuse Slides pane on the right, once it has been opened (Reuse
    // Slides on the New Slide menu): a click on one of its slides puts a copy
    // after the slide being edited, as one step that undoes.
    ReusePane *reusePane() const { return m_reuse; }
    void showReusePane();
    void reuseSlide(int slide, bool keepFormatting);
    // The ways to see the slides: Normal (the thumbnails, the slide, and its
    // notes), Outline (the text in the thumbnails' place), and the Slide Sorter.
    enum class View { Normal, Outline, Sorter };
    View view() const { return m_view; }
    void setView(View view);
    // What the ribbon file named that the window doesn't have (empty when
    // the ribbon built cleanly).
    QString ribbonErrors() const { return m_ribbonErrors; }

    // A new slide after the current one, with the layout of that type ("" for
    // the current slide's layout, or Title and Content after a title slide).
    void newSlide(const QString &layoutType = QString());
    void duplicateSlide();
    void deleteSlide();
    void toggleHidden();
    // Comments: the pane beside the slide and the markers on it, together.
    void showComments(bool on);
    // A new comment on the slide: asks who is writing the first time, and opens the box to type it in.
    void newComment();
    // Sections (deckwindow_sections.cpp), each one step that undoes. A section
    // is named by its index, -1 for the one the slide being edited is in.
    // Add Section starts one at that slide (-1: the slide being edited);
    // Rename asks for the name; moving takes the section's slides along (by -1
    // up, 1 down); Remove Section & Slides keeps at least one slide.
    void addSection(int before = -1);
    void renameSection(int section = -1);
    void removeSection(int section = -1);
    void removeSectionAndSlides(int section = -1);
    void moveSection(int section, int by);
    void removeAllSections();
    void setLayout(const QString &layoutType);
    void resetSlide();
    // Gives the slides another size, everything on them scaled to fit (as
    // one step that undoes).
    void setSlideSize(const QSizeF &size);
    // A solid background for the slide being edited, or for all of them.
    void setBackground(const QColor &color, bool all);
    // Header & Footer for the slide being edited, or for all of them.
    void applyFooters(const deck::FooterOptions &options, bool all);
    // A picture from a file, centered on the slide (or with its corner at `at`
    // when that isn't negative) and selected. False when the file can't be
    // read as a picture.
    bool insertPicture(const QString &path, const QPointF &at = QPointF(-1, -1));
    // A table of that size, centered on the slide, with its first cell ready to type in.
    void insertTable(int rows, int columns);
    // A movie ("video") or a sound ("audio") from a file, put on the slide as the presentation program does: kept in the
    // presentation, with its first picture as the poster and the timing that plays it in the click sequence. False, with
    // the reason shown, when the file can't be put on the slide. (deckwindow_media.cpp)
    bool insertMedia(const QString &path, const QString &kind = QString());
    // Changes how the selected movie or sound plays, as one step that undoes (the Playback tab's boxes).
    void editMedia(const QString &label, const std::function<void(deck::MediaPlay &)> &change);
    // Plays the selected movie or sound on the slide, or pauses it.
    void playMedia();
    // Starts the slide show at `slide` (0 is the first).
    void startShow(int slide);
    // Opens a presentation file (.pptx, .potx, .ppsx, .ppt, .pps, .pot, .odp, .otp) in this window if it
    // holds a new presentation nobody has touched, else in a window of its
    // own. False, with the reason shown, if the file can't be read.
    bool openFile(const QString &path);
    // Saves to the file the presentation came from, else asks where (Save As).
    // A presentation opened from a template always asks, so the template is
    // not written over. False if nothing was written.
    bool save();
    // Asks for a name and writes the presentation there (.pptx, .potx, or .ppsx).
    bool saveAs();
    // Writes the file, and the presentation is saved and takes the file's name.
    // On failure, says why and leaves the presentation unsaved.
    bool saveTo(const QString &path);
    // What Save As asks for a name with: the file dialog, unless this stands
    // in for it (tests). Given the name to suggest and the filter to start on;
    // returns the path chosen, empty for none.
    void setSavePathChooser(const std::function<QString(const QString &suggested, const QString &filter)> &chooser) { m_pathChooser = chooser; }
    // AutoRecover: writes a .pptx copy of unsaved work (the timer does, at the
    // interval Options sets), and offers the copies a run that crashed left.
    void autoRecover();
    int autoRecoverInterval() const { return m_recoverTimer.interval(); }   // ms
    QString recoveryCopy() const { return m_recoveryCopy; }
    void offerRecovery(bool askIfNone = false);
    // Opens a recovered copy as unsaved work, in this window while it holds
    // nothing, else in a window of its own. Null, with the reason, if it can't be read.
    DeckWindow *openRecovered(const recovery::Recovered &r, QString *error = nullptr);

    // The File screen over the window, on a page ("new", "open", "print",
    // "export", "about"; the Open page when none is named), and back to the slides.
    void showFileScreen(const QString &page = QString());
    void hideFileScreen();
    // What the File screen's Print and Export pages do: ask where, then write
    // or print. False when nothing was written or sent.
    bool printOutput(const deck::OutputOptions &options);
    bool exportPdf();
    bool exportPicture();

    // JeffDeck's Help (F1 and the ribbon's "?"): the Help pane on the right,
    // on `topic` ("" for the topic of what is being done, else the contents);
    // over the File screen, a window of its own.
    void showHelp(const QString &topic = QString());
    HelpView *helpView() const { return m_help; }

protected:
    void closeEvent(QCloseEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
    bool eventFilter(QObject *o, QEvent *e) override;

private:
    QAction *mk(const QString &id, const QString &text, const QString &iconName, const QKeySequence &key, const std::function<void()> &fn,
                bool checkable = false);
    void createShowActions();
    void createViewModeActions();   // deckwindow_views.cpp
    void addShowParts(struct RibbonParts &parts);
    void createActions();
    void createSaveActions();     // deckwindow_save.cpp
    void createDialogActions();   // deckwindow_dialogs.cpp
    void createCommentActions();  // deckwindow_comments.cpp
    void createSectionActions();  // deckwindow_sections.cpp
    void createReuseActions();    // deckwindow_reuse.cpp, from createSectionActions()
    void createOutputActions();   // deckwindow_output.cpp
    void createFormatActions();   // deckwindow_actions.cpp
    void createInsertActions();
    void createDesignActions();
    void createViewActions();
    void buildRibbon();           // deckwindow_ribbon.cpp
    void ribbonControlParts(RibbonParts &parts);
    void ribbonGalleryParts(RibbonParts &parts);
    void ribbonMenuParts(RibbonParts &parts);
    void buildStatusBar();
    void refreshUi();
    void showHeaderFooter();
    void showFormatBackground();
    void askSlideSize(const QSizeF &size);
    void insertPictureFromFile();
    void updateTitle();
    bool maybeSave();
    void dropRecoveryCopy();      // deckwindow_save.cpp
    void createHelpActions();     // deckwindow_help.cpp
    void createMediaActions();    // deckwindow_media.cpp
    void addMediaParts(struct RibbonParts &parts);
    void insertMediaFromFile(const QString &kind);
    void trimMedia();
    void refreshMedia();
    QString helpContext() const;
    HelpWindow *showHelpOver(const QString &topic);

    Editor *m_ed = nullptr;
    DeckCanvas *m_canvas = nullptr;
    Ribbon *m_ribbon = nullptr;
    SlidesPane *m_slides = nullptr;
    NotesPane *m_notes = nullptr;
    CommentsPane *m_comments = nullptr;
    DeckFileScreen *m_fileScreen = nullptr;
    QSplitter *m_split = nullptr, *m_rightSplit = nullptr, *m_commentSplit = nullptr;   // the last: the slide and the Comments pane
    QStackedWidget *m_views = nullptr;   // the split, and the Slide Sorter
    SlideSorter *m_sorter = nullptr;
    ReusePane *m_reuse = nullptr;
    QDockWidget *m_reuseDock = nullptr;
    HelpView *m_help = nullptr;
    QDockWidget *m_helpDock = nullptr;
    OutlineView *m_outline = nullptr;    // in the split, where the thumbnails are in Normal view
    View m_view = View::Normal;
    // Normal view's zoom while the Slide Sorter borrows the zoom controls: its Canvas::Fit and the zoom.
    int m_normalFit = 0;
    double m_normalZoom = 1;
    QLabel *m_slideLabel = nullptr, *m_zoomLabel = nullptr;
    QSlider *m_zoomSlider = nullptr;
    QList<FontCombo *> m_fontCombos;
    QList<SizeCombo *> m_sizeCombos;
    QList<ColorButton *> m_colorButtons;
    QSizeF m_shownSize;   // the slide size the view was last fitted to
    QPointer<SlideShow> m_show;
    QHash<QString, QAction *> m_actions;
    QString m_ribbonErrors;
    QTimer m_refreshTimer, m_recoverTimer;
    QPair<QString, int> m_fitWatch;   // the text box being edited, and its story's revision when that began
    QString m_recoveryCopy;      // this window's AutoRecover copy, if written
    QString m_recoveredFrom;     // a recovered presentation's original file, for Save As
    QString m_templatePath;      // a template this window wrote: saving again writes over it
    std::function<QString(const QString &, const QString &)> m_pathChooser;
    quint64 m_serial = 0;
};

} // namespace jp
