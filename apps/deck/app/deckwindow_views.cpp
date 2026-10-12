// JeffDeck's views: Normal, Outline, and the Slide Sorter, one at a time, from
// the View tab and the status bar.

#include "app/deckcanvas.h"
#include "app/deckwindow.h"
#include "app/editor.h"
#include "app/notespane.h"
#include "app/outlineview.h"
#include "app/slidesorter.h"
#include "app/slidespane.h"

#include <QActionGroup>
#include <QSplitter>
#include <QStackedWidget>
#include <cmath>

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
    for (QAction *a : {view("view.notesPage", tr("Notes Page"), "notebook-text", View::Normal), view("view.reading", tr("Reading View"), "book-open", View::Normal)}) {
        a->setEnabled(false);
        a->setToolTip(tr("%1 is coming.").arg(a->text()));
    }

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
        const int width = std::max(m_split->width(), 600), outline = std::min(440, width * 2 / 5);
        m_split->setSizes(view == View::Outline ? QList<int>{0, outline, width - outline} : QList<int>{250, 0, width - 250});
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
    act(view == View::Normal ? "view.normal" : view == View::Outline ? "view.outline" : "view.sorter")->setChecked(true);
    if (view == View::Sorter) m_sorter->setFocus();
    else if (view == View::Outline) m_outline->setFocus();
    else if (was != View::Normal) m_canvas->setFocus(Qt::OtherFocusReason);
    refreshUi();
}

} // namespace jp
