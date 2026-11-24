// JeffDeck's views: Normal, Outline, and the Slide Sorter, one at a time, from
// the View tab and the status bar.

#include "app/deckcanvas.h"
#include "app/deckwindow.h"
#include "app/editor.h"
#include "app/masterpane.h"
#include "app/notespageview.h"
#include "app/notespane.h"
#include "app/outlineview.h"
#include "app/ribbon.h"
#include "app/slidesorter.h"
#include "app/slidespane.h"
#include "core/deck.h"
#include "core/showsettings.h"
#include "app/slideshow.h"

#include <QActionGroup>
#include <QCoreApplication>
#include <QDockWidget>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMenu>
#include <QStatusBar>
#include <QToolButton>
#include <QSplitter>
#include <QStackedWidget>
#include <cmath>
#include "app/plaintext.h"

namespace jp {

// The commands that choose the view, and the views themselves. The central
// area becomes a stack of the split (Normal and Outline, where the outline
// takes the thumbnails' place beside the slide and its notes) and the sorter.
void DeckWindow::createViewModeActions()
{
    takeCentralWidget();
    m_views = new QStackedWidget(this);
    m_views->addWidget(m_split);
    setCentralWidget(m_views);
    m_sorter = new SlideSorter(m_ed, this);
    m_views->addWidget(m_sorter);
    m_outline = new OutlineView(m_ed, this);
    m_outline->setMinimumWidth(260);
    m_split->insertWidget(1, m_outline);
    m_split->setCollapsible(1, false);
    m_outline->hide();
    m_masterPane = new MasterPane(m_ed, this);
    m_split->insertWidget(2, m_masterPane);
    m_split->setCollapsible(2, false);
    m_masterPane->hide();
    m_notesPage = new NotesPageView(m_ed, this);
    m_split->insertWidget(4, m_notesPage);
    m_split->setCollapsible(4, false);
    m_notesPage->hide();

    // One view at a time.
    auto *views = new QActionGroup(this);
    auto view = [&](const char *id, const QString &text, const char *iconName, View v) {
        QAction *a = mk(QString::fromLatin1(id), text, QString::fromLatin1(iconName), QKeySequence(), [this, v] { setView(v); }, true);
        views->addAction(a);
        return a;
    };
    view("view.normal", tr("Normal"), "layout-panel-top", View::Normal)->setChecked(true);
    view("view.outline", tr("Outline View"), "list-tree", View::Outline);
    view("view.sorter", tr("Slide Sorter"), "layout-grid", View::Sorter);
    view("view.notesPage", tr("Notes Page"), "notebook-pen", View::NotesPage);
    {
        QAction *reading = mk(QStringLiteral("view.reading"), tr("Reading View"), QStringLiteral("book-open"), QKeySequence(), [this] { startReadingView(); }, true);
        views->addAction(reading);
    }
    view("view.master", tr("Slide Master"), "layout-template", View::Master);
    view("view.notesMaster", tr("Notes Master"), "notebook-tabs", View::NotesMaster);
    view("view.handoutMaster", tr("Handout Master"), "files", View::HandoutMaster);

    connect(m_sorter, &SlideSorter::openRequested, this, [this](int slide) {
        m_ed->setCurrentPage(slide);
        setView(View::Normal);
    });
    connect(m_sorter, &SlideSorter::newSlideWanted, this, [this] { newSlide(); });
    // The sorter and the notes and handout views use the zoom controls (the slider, its buttons, the zoom menu) for their own zoom: they zoom the canvas, which the view follows.
    connect(m_sorter, &SlideSorter::zoomChanged, this, [this](double zoom) {
        if (m_view != View::Sorter || std::abs(m_canvas->zoom() - zoom) < 1e-9) return;
        m_canvas->zoomToFit(Canvas::Fit::None);
        m_canvas->setZoom(zoom);
    });
    connect(m_notesPage, &NotesPageView::zoomChanged, this, [this](double zoom) {
        if (!borrowsZoom(m_view) || m_view == View::Sorter || std::abs(m_canvas->zoom() - zoom) < 1e-9) return;
        m_canvas->zoomToFit(Canvas::Fit::None);
        m_canvas->setZoom(zoom);
    });
    connect(m_canvas, &Canvas::zoomChanged, this, [this](double zoom) {
        if (m_view == View::Sorter) {
            m_sorter->setZoom(zoom);
            if (std::abs(m_sorter->zoom() - zoom) > 1e-9) m_canvas->setZoom(m_sorter->zoom());   // the thumbnails' smallest or largest
        } else if (borrowsZoom(m_view)) {
            m_notesPage->setZoom(zoom);
            if (std::abs(m_notesPage->zoom() - zoom) > 1e-9) m_canvas->setZoom(m_notesPage->zoom());
        }
    });
}

// The views that take the zoom controls from the canvas.
bool DeckWindow::borrowsZoom(View view) { return view == View::Sorter || view == View::NotesPage || view == View::NotesMaster || view == View::HandoutMaster; }

static const char *viewAction(DeckWindow::View view)
{
    switch (view) {
    case DeckWindow::View::Outline: return "view.outline";
    case DeckWindow::View::Sorter: return "view.sorter";
    case DeckWindow::View::Master: return "view.master";
    case DeckWindow::View::NotesPage: return "view.notesPage";
    case DeckWindow::View::NotesMaster: return "view.notesMaster";
    case DeckWindow::View::HandoutMaster: return "view.handoutMaster";
    default: return "view.normal";
    }
}

void DeckWindow::setView(View view)
{
    if (m_reading) leaveReadingView();
    m_notes->commit();
    if (m_outline) m_outline->commit();
    if (m_notesPage) m_notesPage->commit();
    const View was = m_view;
    const bool toMaster = view == View::Master && was != View::Master, fromMaster = view != View::Master && was == View::Master;
    const bool notesMasters = view == View::NotesMaster || view == View::HandoutMaster, wasNotesMasters = was == View::NotesMaster || was == View::HandoutMaster;
    m_switchingView = true;
    const bool opened = !toMaster || enterMasterView();
    if (fromMaster) leaveMasterView();
    m_switchingView = false;
    if (!opened) {
        // Nothing to edit: the view stays as it was.
        act(viewAction(was))->setChecked(true);
        return;
    }
    // Close Master View returns to the view the Notes Master or Handout Master was opened from.
    if (notesMasters && !wasNotesMasters) m_viewBeforeMasters = was == View::Master ? View::Normal : was;
    m_view = view;
    if (view != was) {
        m_ed->stopHighlighter();   // (a changed view ends the highlighter)
        if (borrowsZoom(view) && !borrowsZoom(was)) {
            m_ed->endTextEdit();
            m_normalFit = int(m_canvas->fitMode());
            m_normalZoom = m_canvas->zoom();
        }
        const bool notesViews = view == View::NotesPage || notesMasters;
        m_views->setCurrentWidget(view == View::Sorter ? static_cast<QWidget *>(m_sorter) : m_split);
        m_slides->setVisible(view == View::Normal || view == View::NotesPage);
        m_outline->setVisible(view == View::Outline);
        m_masterPane->setVisible(view == View::Master);
        m_rightSplit->setVisible(!notesViews);
        m_notesPage->setVisible(notesViews);
        if (notesViews) m_notesPage->setMode(view == View::NotesPage ? NotesPageView::Mode::NotesPage : view == View::NotesMaster ? NotesPageView::Mode::NotesMaster : NotesPageView::Mode::HandoutMaster);
        const int width = std::max(m_split->width(), 600), outline = std::min(440, width * 2 / 5), masters = std::min(300, width / 3);
        m_split->setSizes(view == View::Outline ? QList<int>{0, outline, 0, width - outline, 0}
                          : view == View::Master ? QList<int>{0, 0, masters, width - masters, 0}
                          : view == View::NotesPage ? QList<int>{250, 0, 0, 0, width - 250}
                          : notesMasters ? QList<int>{0, 0, 0, 0, width}
                                         : QList<int>{250, 0, 0, width - 250, 0});
        if (borrowsZoom(view)) {
            m_canvas->zoomToFit(Canvas::Fit::None);
            if (view == View::Sorter) {
                m_canvas->setZoom(m_sorter->zoom());
            } else {
                // The page fits the view when it opens (as soon as the view has its size).
                m_notesPage->fitToView();
            }
        } else if (borrowsZoom(was)) {
            m_canvas->zoomToFit(Canvas::Fit::None);
            m_canvas->setZoom(m_normalZoom);
            if (Canvas::Fit(m_normalFit) != Canvas::Fit::None) m_canvas->zoomToFit(Canvas::Fit(m_normalFit));
        }
        act("view.fit")->setEnabled(!borrowsZoom(view));
        if (notesMasters != wasNotesMasters || view != was) {
            // The tabs of the Notes Master and the Handout Master show while one is open, and the tabs that work on a slide wait.
            m_ribbon->setContextVisible(QStringLiteral("Notes Master"), view == View::NotesMaster, view == View::NotesMaster);
            m_ribbon->setContextVisible(QStringLiteral("Handout Master"), view == View::HandoutMaster, view == View::HandoutMaster);
            if (notesMasters != wasNotesMasters && view != View::Master)
                for (const char *name : {"Transitions", "Animations", "Slide Show", "Review"}) m_ribbon->setTabVisible(QString::fromLatin1(name), !notesMasters);
        }
    }
    act(viewAction(view))->setChecked(true);
    if (view == View::Sorter) m_sorter->setFocus();
    else if (view == View::Outline) m_outline->setFocus();
    else if (view == View::NotesPage || notesMasters) m_notesPage->setFocus();
    else if (was != View::Normal) m_canvas->setFocus(Qt::OtherFocusReason);
    refreshNotesMasterActions();
    refreshUi();
}

// Reading View: the show is played in the place of the editor, by an individual, with the ribbon and the panes put away and the status bar kept. The status bar
// gets Previous, Next, and a Menu button (the show's own menu); the view the person came from is where Esc, End Show, and the other views go back to.
void DeckWindow::startReadingView()
{
    if (m_reading) return;
    const bool masters = m_view == View::Master || m_view == View::NotesMaster || m_view == View::HandoutMaster;
    if (masters || m_show) {
        // Nothing to read in a master, and one show at a time.
        act(viewAction(m_view))->setChecked(true);
        return;
    }
    m_notes->commit();
    if (m_outline) m_outline->commit();
    if (m_notesPage) m_notesPage->commit();
    m_ed->endTextEdit();
    m_readingFocus = focusWidget();
    m_ed->setReadOnly(true);   // (a change from anywhere is taken back, as in a window open for reading only)
    deck::ShowSettings settings = deck::showSettingsOf(*m_ed->doc());
    settings.type = deck::ShowSettings::Type::Individual;
    settings.loop = false;
    auto *show = new SlideShow(m_ed->doc(), &m_ed->cache(), m_ed->currentPage());
    show->setSettings(settings);
    if (!m_ed->filePath().isEmpty()) show->setFilePath(m_ed->filePath());
    show->embedIn(m_views);
    m_reading = show;
    watchShow(show, false);
    connect(show, &SlideShow::ended, this, [this, show] {
        if (m_reading == show) leaveReadingView();
    });
    m_views->addWidget(show);
    m_views->setCurrentWidget(show);
    m_ribbon->hide();
    m_readingDocks.clear();
    for (QDockWidget *dock : findChildren<QDockWidget *>())
        if (dock->isVisible()) {
            m_readingDocks << dock;
            dock->hide();
        }
    // The controls, after the slide's place in the status bar.
    auto *bar = new QWidget;
    bar->setObjectName(QStringLiteral("readingBar"));
    auto *row = new QHBoxLayout(bar);
    row->setContentsMargins(0, 0, 0, 0);
    auto *place = new QLabel(bar);
    place->setObjectName(QStringLiteral("readingPlace"));
    auto button = [&](const char *id, const QString &text, const QString &tip) {
        auto *b = new QToolButton(bar);
        b->setObjectName(QString::fromLatin1(id));
        b->setText(text);
        b->setToolTip(jp::plainToolTip(tip));
        b->setAccessibleName(text);
        b->setAutoRaise(true);
        row->addWidget(b);
        return b;
    };
    QToolButton *previous = button("readingPrevious", tr("Previous"), tr("Go back one step or slide"));
    row->addWidget(place);
    QToolButton *next = button("readingNext", tr("Next"), tr("Go forward one step or slide"));
    QToolButton *menu = button("readingMenu", tr("Show Menu"), tr("Open the menu for going to a slide, the pointer, the screen, and ending the show"));
    connect(previous, &QToolButton::clicked, show, [show] { show->previous(); });
    connect(next, &QToolButton::clicked, show, [show] { show->next(); });
    connect(menu, &QToolButton::clicked, show, [show, menu] {
        QMenu *m = show->buildMenu(show);   // (the show's own, so that it goes if the show ends while it is open)
        m->setAttribute(Qt::WA_DeleteOnClose);
        m->popup(menu->mapToGlobal(QPoint(0, 0)) - QPoint(0, m->sizeHint().height()));
    });
    auto update = [show, place, this] {
        const int slide = show->targetSlide();
        const int n = m_ed->doc()->pages.size();
        place->setText(slide < 0 ? tr("End of the show") : tr("Slide %1 of %2").arg(slide + 1).arg(n));
    };
    connect(show, &SlideShow::changed, bar, update);
    update();
    statusBar()->addPermanentWidget(bar);
    m_readingBar = bar;
    // What zooms the editor, and the notes pane's button, have nothing to do while reading.
    for (QWidget *w : readingHiddenStatusWidgets()) w->hide();
    m_slideLabel->hide();
    act("view.reading")->setChecked(true);
    show->show();
    show->setFocus();
    refreshUi();
}

// Esc is the show's whatever in the window has the keyboard (a status bar control that Tab reached ignores it, and it comes here).
void DeckWindow::keyPressEvent(QKeyEvent *e)
{
    if (m_reading && e->key() == Qt::Key_Escape && e->modifiers() == Qt::NoModifier) {
        QCoreApplication::sendEvent(m_reading, e);
        return;
    }
    AppWindow::keyPressEvent(e);
}

void DeckWindow::endReadingView()
{
    if (!m_reading) return;
    m_reading->close();
    // The question about the ink comes before whatever starts next.
    QCoreApplication::sendPostedEvents(this, QEvent::MetaCall);
}

// Everything Reading View put away comes back, on the view it started from. The show is closed (and goes with its timers) when this is called from a view change.
void DeckWindow::leaveReadingView()
{
    SlideShow *show = m_reading;
    m_reading.clear();
    if (show) {
        show->hide();
        show->close();
        m_views->removeWidget(show);
    }
    if (m_readingBar) {
        statusBar()->removeWidget(m_readingBar);
        m_readingBar->deleteLater();
    }
    m_ribbon->show();
    for (const QPointer<QDockWidget> &dock : std::as_const(m_readingDocks))
        if (dock) dock->show();
    m_readingDocks.clear();
    m_views->setCurrentWidget(m_view == View::Sorter ? static_cast<QWidget *>(m_sorter) : m_split);
    m_slideLabel->show();
    for (QWidget *w : readingHiddenStatusWidgets()) w->show();
    act(viewAction(m_view))->setChecked(true);
    m_ed->setReadOnly(isReadOnly());
    // The keyboard goes back to what had it.
    if (m_readingFocus && m_readingFocus->isVisible()) m_readingFocus->setFocus(Qt::OtherFocusReason);
    m_readingFocus.clear();
    refreshUi();
}

// The status bar's buttons and sliders that belong to the editor: the notes button and the zoom controls.
QList<QWidget *> DeckWindow::readingHiddenStatusWidgets() const
{
    QList<QWidget *> out{m_zoomSlider, m_zoomLabel};
    for (QToolButton *b : statusBar()->findChildren<QToolButton *>())
        if (b->defaultAction() && (b->defaultAction() == act("view.notes") || b->defaultAction() == act("zoom.out") || b->defaultAction() == act("zoom.in") || b->defaultAction() == act("view.fit")))
            out << b;
    return out;
}

} // namespace jp
