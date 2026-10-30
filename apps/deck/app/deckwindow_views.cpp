// JeffDeck's views: Normal, Outline, and the Slide Sorter, one at a time, from
// the View tab and the status bar.

#include "app/deckcanvas.h"
#include "app/deckwindow.h"
#include "app/editor.h"
#include "app/masterpane.h"
#include "app/notespane.h"
#include "app/outlineview.h"
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

    // One view at a time; Notes Page and Reading View wait.
    auto *views = new QActionGroup(this);
    auto view = [&](const char *id, const QString &text, const char *iconName, View v) {
        QAction *a = mk(QString::fromLatin1(id), text, QString::fromLatin1(iconName), QKeySequence(), [this, v] { setView(v); }, true);
        views->addAction(a);
        return a;
    };
    view("view.normal", tr("Normal"), "layout-panel-top", View::Normal)->setChecked(true);
    view("view.outline", tr("Outline View"), "list-tree", View::Outline);
    view("view.sorter", tr("Slide Sorter"), "layout-grid", View::Sorter);
    view("view.master", tr("Slide Master"), "layout-template", View::Master);

    connect(m_sorter, &SlideSorter::openRequested, this, [this](int slide) {
        m_ed->setCurrentPage(slide);
        setView(View::Normal);
    });
    connect(m_sorter, &SlideSorter::newSlideWanted, this, [this] { newSlide(); });
    // The sorter uses the zoom controls (the slider, its buttons, the zoom menu) for its thumbnails' size: they zoom the canvas, which the sorter follows.
    connect(m_sorter, &SlideSorter::zoomChanged, this, [this](double zoom) {
        if (m_view != View::Sorter || std::abs(m_canvas->zoom() - zoom) < 1e-9) return;
        m_canvas->zoomToFit(Canvas::Fit::None);
        m_canvas->setZoom(zoom);
    });
    connect(m_canvas, &Canvas::zoomChanged, this, [this](double zoom) {
        if (m_view != View::Sorter) return;
        m_sorter->setZoom(zoom);
        if (std::abs(m_sorter->zoom() - zoom) > 1e-9) m_canvas->setZoom(m_sorter->zoom());   // the thumbnails' smallest or largest
    });
}

void DeckWindow::setView(View view)
{
    m_notes->commit();
    if (m_outline) m_outline->commit();
    const View was = m_view;
    const bool toMaster = view == View::Master && was != View::Master, fromMaster = view != View::Master && was == View::Master;
    m_switchingView = true;
    const bool opened = !toMaster || enterMasterView();
    if (fromMaster) leaveMasterView();
    m_switchingView = false;
    if (!opened) {
        // Nothing to edit: the view stays as it was.
        act(was == View::Normal ? "view.normal" : was == View::Outline ? "view.outline" : "view.sorter")->setChecked(true);
        return;
    }
    m_view = view;
    if (view != was) {
        if (view == View::Sorter) {
            m_ed->endTextEdit();
            m_normalFit = int(m_canvas->fitMode());
            m_normalZoom = m_canvas->zoom();
        }
        m_views->setCurrentWidget(view == View::Sorter ? static_cast<QWidget *>(m_sorter) : m_split);
        m_slides->setVisible(view == View::Normal);
        m_outline->setVisible(view == View::Outline);
        m_masterPane->setVisible(view == View::Master);
        const int width = std::max(m_split->width(), 600), outline = std::min(440, width * 2 / 5), masters = std::min(300, width / 3);
        m_split->setSizes(view == View::Outline ? QList<int>{0, outline, 0, width - outline}
                          : view == View::Master ? QList<int>{0, 0, masters, width - masters}
                                                 : QList<int>{250, 0, 0, width - 250});
        if (view == View::Sorter) {
            m_canvas->zoomToFit(Canvas::Fit::None);
            m_canvas->setZoom(m_sorter->zoom());
        } else if (was == View::Sorter) {
            m_canvas->zoomToFit(Canvas::Fit::None);
            m_canvas->setZoom(m_normalZoom);
            if (Canvas::Fit(m_normalFit) != Canvas::Fit::None) m_canvas->zoomToFit(Canvas::Fit(m_normalFit));
        }
        act("view.fit")->setEnabled(view != View::Sorter);
    }
    act(view == View::Normal ? "view.normal" : view == View::Outline ? "view.outline" : view == View::Master ? "view.master" : "view.sorter")->setChecked(true);
    if (view == View::Sorter) m_sorter->setFocus();
    else if (view == View::Outline) m_outline->setFocus();
    else if (was != View::Normal) m_canvas->setFocus(Qt::OtherFocusReason);
    refreshUi();
}

} // namespace jp
