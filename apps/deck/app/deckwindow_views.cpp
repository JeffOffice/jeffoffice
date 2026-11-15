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

#include <QActionGroup>
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

} // namespace jp
