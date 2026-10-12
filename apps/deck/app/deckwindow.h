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
class QSplitter;

namespace jp {

class Canvas;
class Editor;
class NotesPane;
class Ribbon;
class SlidesPane;

class DeckWindow : public AppWindow {
    Q_OBJECT
public:
    explicit DeckWindow(QWidget *parent = nullptr);
    ~DeckWindow() override;

    QAction *act(const QString &id) const override;
    QStringList actionIds() const override;
    Ribbon *ribbon() const override { return m_ribbon; }
    Editor *editor() const { return m_ed; }
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
    // Starts the slide show at `slide` (0 is the first).
    void startShow(int slide);

protected:
    void closeEvent(QCloseEvent *e) override;

private:
    QAction *mk(const QString &id, const QString &text, const QString &iconName, const QKeySequence &key, const std::function<void()> &fn,
                bool checkable = false);
    void createShowActions();
    void addShowParts(struct RibbonParts &parts);
    void createActions();
    void buildRibbon();
    void buildStatusBar();
    void refreshUi();
    void updateTitle();
    void showFileMenu();
    bool maybeSave();

    Editor *m_ed = nullptr;
    Canvas *m_canvas = nullptr;
    Ribbon *m_ribbon = nullptr;
    SlidesPane *m_slides = nullptr;
    NotesPane *m_notes = nullptr;
    QSplitter *m_split = nullptr, *m_rightSplit = nullptr;
    QLabel *m_slideLabel = nullptr;
    QPointer<SlideShow> m_show;
    QHash<QString, QAction *> m_actions;
    QString m_ribbonErrors;
    QTimer m_refreshTimer;
};

} // namespace jp
