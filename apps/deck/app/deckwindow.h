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
#include <memory>

class QDockWidget;
class QLabel;
class QSlider;
class QSplitter;
class QStackedWidget;

namespace jp {

class AnimationPane;
class ColorButton;
class Document;
class CommentsPane;
class DeckCanvas;
class DeckFileScreen;
class Editor;
class FontCombo;
class HelpView;
class HelpWindow;
class NotesPane;
class OnlinePicturesPane;
class OutlineView;
class Ribbon;
class ReusePane;
struct RibbonParts;
class SizeCombo;
class SlideSorter;
class SlidesPane;

namespace deck {
struct ColorSet;
struct FontSet;
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
    // The Animation Pane on the right (Animations tab > Animation Pane), with the effects of the slide being edited; the
    // numbers by the shapes show while it, or the tab, does (deckwindow_animations.cpp).
    AnimationPane *animationPane() const { return m_animPane; }
    void showAnimationPane(bool on = true);
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
    // Design > Themes and Variants (deckwindow_design.cpp), each one step that undoes. A theme is one of the built-in ones (by number) or
    // the one a presentation file has, for every slide or for the selected ones (the slides selected in the Slide Sorter, else the slide
    // being edited, who get a slide master of their own). The file is any the program opens; false, with the reason shown, if it can't
    // be used. A variant (by number, in variantsOf), colors, fonts, and background styles apply to the whole presentation.
    void applyTheme(int theme, bool allSlides);
    bool applyThemeFile(const QString &path, bool allSlides);
    void browseForTheme(bool allSlides = true);
    void chooseVariant(int variant);
    void chooseColors(const deck::ColorSet &colors);
    void chooseFonts(const deck::FontSet &fonts);
    void chooseBackgroundStyle(int style);
    // A picture from a file, centered on the slide (or with its corner at `at`
    // when that isn't negative) and selected. False when the file can't be
    // read as a picture. With `altText` the picture has it for screen readers;
    // with `credit` a line of small text under it names its author (all one step).
    bool insertPicture(const QString &path, const QPointF &at = QPointF(-1, -1), const QString &altText = QString(), const QString &credit = QString());
    // Icons by name (the bundled ones), each put on the slide as an SVG picture an inch square in the
    // theme's text color, side by side across the middle, selected, as one step that undoes. A name that is
    // no icon is left out. (deckwindow_insert.cpp)
    void insertIcons(const QStringList &names);
    // A Text Art in a style of the Text Art gallery, in the middle of the slide and selected, and the box that
    // asks for its words, with the old ones selected so that typing replaces them. (deckwindow_insert.cpp)
    void insertTextArt(const QString &styleId);
    // The Online Pictures pane on the right (Insert > Pictures > Online Pictures), once it has been opened: a
    // picture chosen in it is put on the slide, with its credit under it when asked.
    OnlinePicturesPane *onlinePictures() const { return m_online; }
    void showOnlinePictures();
    // A table of that size, centered on the slide, with its first cell ready to type in.
    void insertTable(int rows, int columns);
    // A movie ("video") or a sound ("audio") from a file, put on the slide as the presentation program does: kept in the
    // presentation, with its first picture as the poster and the timing that plays it in the click sequence. False, with
    // the reason shown, when the file can't be put on the slide. (deckwindow_media.cpp)
    bool insertMedia(const QString &path, const QString &kind = QString());
    // Changes how the selected movie or sound plays, as one step that undoes (the Playback tab's boxes).
    // `fileLength` is how long the whole file lasts, in milliseconds, when the change knows it (a trim does).
    void editMedia(const QString &label, const std::function<void(deck::MediaPlay &)> &change, qint64 fileLength = 0);
    // Plays the selected movie or sound on the slide, or pauses it.
    void playMedia();
    // Starts the slide show at `slide` (0 is the first), with every slide in
    // play from there; the rest of how it goes is what the presentation is set
    // up for (Set Up Slide Show).
    void startShow(int slide);
    // The Slide Show tab (deckwindow_show.cpp). From Beginning plays the slides
    // the show is set up for. Set Up Slide Show and Custom Shows ask in dialogs
    // and are each one step that undoes; a custom show plays by its id.
    // Rehearse Timings plays the show with the timing bar. What a show leaves
    // behind is asked about when it ends: ink (kept as freeform lines on the
    // slides) and the new times (kept as each slide's After, with Use Timings on).
    void startShowFromBeginning();
    void startCustomShow(qint64 id);
    void setUpShow();
    void editCustomShows();
    void rehearseTimings();
    // What those questions are asked with, and the notices the show gives (a range
    // of slides past the end): the message box, unless this stands in for it
    // (tests). Given the question, it says whether to keep; a notice's answer is ignored.
    void setShowQuestion(const std::function<bool(const QString &)> &ask) { m_showQuestion = ask; }
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
    // The ending of a presentation file as the usage statistics name its kind: one JeffDeck reads or writes ("pptx", "odp", ...),
    // else "other". Never any part of the name.
    static QString fileKind(const QString &path);
    QAction *mk(const QString &id, const QString &text, const QString &iconName, const QKeySequence &key, const std::function<void()> &fn,
                bool checkable = false);
    void createShowActions();
    void playShow(int slide, const deck::ShowPlan &plan, bool recording, bool rehearsal = false);
    void watchShow(SlideShow *show, bool recording);
    void afterShow(const QHash<QString, QVector<deck::InkStroke>> &ink, const QMap<int, double> &seconds, double total, bool recording);
    bool askAboutShow(const QString &question, const QString &keep, const QString &discard);
    void tellAboutShow(const QString &text);
    void createViewModeActions();   // deckwindow_views.cpp
    void addShowParts(struct RibbonParts &parts);
    void addDesignParts(struct RibbonParts &parts);   // deckwindow_design.cpp
    bool applyThemeFrom(const Document &source, bool allSlides);
    void changeDesign(const QString &label, const std::function<void(Document &)> &fn);
    void createActions();
    void createSaveActions();     // deckwindow_save.cpp
    void createDialogActions();   // deckwindow_dialogs.cpp
    void createCommentActions();  // deckwindow_comments.cpp
    void createSectionActions();  // deckwindow_sections.cpp
    void createReuseActions();    // deckwindow_reuse.cpp, from createSectionActions()
    void createAnimationActions();   // deckwindow_animations.cpp
    void addAnimationParts(struct RibbonParts &parts);
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
    void createPictureActions();  // deckwindow_insert.cpp: Online Pictures, Icons, This Device
    void addInsertParts(struct RibbonParts &parts);
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
    OnlinePicturesPane *m_online = nullptr;
    QDockWidget *m_onlineDock = nullptr;
    HelpView *m_help = nullptr;
    QDockWidget *m_helpDock = nullptr;
    AnimationPane *m_animPane = nullptr;
    QDockWidget *m_animDock = nullptr;
    std::shared_ptr<void> m_animations;   // what the animation controls do (deckwindow_animations.cpp)
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
    const Document *m_fontsOf = nullptr;   // the presentation whose embedded fonts this window holds open (core/embeddedfonts.h)
    QHash<QString, QAction *> m_actions;
    QString m_ribbonErrors;
    QTimer m_refreshTimer, m_recoverTimer;
    QPair<QString, int> m_fitWatch;   // the text box being edited, and its story's revision when that began
    QString m_recoveryCopy;      // this window's AutoRecover copy, if written
    QString m_recoveredFrom;     // a recovered presentation's original file, for Save As
    QString m_templatePath;      // a template this window wrote: saving again writes over it
    std::function<QString(const QString &, const QString &)> m_pathChooser;
    std::function<bool(const QString &)> m_showQuestion;
    quint64 m_serial = 0;
};

} // namespace jp
