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

class QLabel;
class QSlider;
class QSplitter;

namespace jp {

class ColorButton;
class DeckCanvas;
class Editor;
class FontCombo;
class NotesPane;
class Ribbon;
struct RibbonParts;
class SizeCombo;
class SlidesPane;

namespace deck {
struct FooterOptions;
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
    SlideShow *slideShow() const { return m_show; }
    // What the ribbon file named that the window doesn't have (empty when
    // the ribbon built cleanly).
    QString ribbonErrors() const { return m_ribbonErrors; }

    // A new slide after the current one, with the layout of that type ("" for
    // the current slide's layout, or Title and Content after a title slide).
    void newSlide(const QString &layoutType = QString());
    void duplicateSlide();
    void deleteSlide();
    void toggleHidden();
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
    // Starts the slide show at `slide` (0 is the first).
    void startShow(int slide);
    // Opens a presentation file (.pptx, .potx, .ppsx) in this window if it
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

protected:
    void closeEvent(QCloseEvent *e) override;

private:
    QAction *mk(const QString &id, const QString &text, const QString &iconName, const QKeySequence &key, const std::function<void()> &fn,
                bool checkable = false);
    void createActions();
    void createSaveActions();     // deckwindow_save.cpp
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
    void showFileMenu();
    bool maybeSave();
    void dropRecoveryCopy();      // deckwindow_save.cpp

    Editor *m_ed = nullptr;
    DeckCanvas *m_canvas = nullptr;
    Ribbon *m_ribbon = nullptr;
    SlidesPane *m_slides = nullptr;
    NotesPane *m_notes = nullptr;
    QSplitter *m_split = nullptr, *m_rightSplit = nullptr;
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
    QString m_recoveryCopy;      // this window's AutoRecover copy, if written
    QString m_recoveredFrom;     // a recovered presentation's original file, for Save As
    QString m_templatePath;      // a template this window wrote: saving again writes over it
    std::function<QString(const QString &, const QString &)> m_pathChooser;
    quint64 m_serial = 0;
};

} // namespace jp
