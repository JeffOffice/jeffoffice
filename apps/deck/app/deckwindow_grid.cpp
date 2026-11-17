// JeffDeck's Grid and Guides: the dialog of the View tab, and the presentation's grid settings and guides on the canvas. The settings and the guides
// live in the presentation (core/gridview.h) and a .pptx keeps them; the program's settings hold what the last dialog chose, for a presentation that has none.

#include "app/deckwindow.h"

#include "app/deckcanvas.h"
#include "app/editor.h"
#include "app/guidesdialog.h"
#include "app/settings.h"
#include "core/gridview.h"

#include <QAction>
#include <algorithm>
#include <cmath>

namespace jp {

namespace {
const QString kSpacingKey = QStringLiteral("view/gridSpacing");
const QString kSnapGridKey = QStringLiteral("view/snapGrid");
const QString kSnapObjectsKey = QStringLiteral("view/snapObjects");
}

// The view takes the grid settings of the presentation, else the program's own; with `showGuides` also whether the guides show.
void DeckWindow::applyGridView(bool showGuides)
{
    const Document *d = m_ed->doc();
    if (!d) return;
    const deck::GridView g = deck::gridViewOf(*d);
    m_gridSeen = g;
    double saved = Settings::get().value(kSpacingKey, deck::kDefaultGridSpacing).toDouble();
    if (!std::isfinite(saved)) saved = deck::kDefaultGridSpacing;
    const double spacing = g.spacing.value_or(std::clamp(saved, kMinGridSpacing, 720.0));
    const bool snapGrid = g.snapGrid.value_or(Settings::get().value(kSnapGridKey, false).toBool());
    const bool snapObjects = g.snapObjects.value_or(Settings::get().value(kSnapObjectsKey, true).toBool());
    ViewOptions &v = m_ed->view;
    if (v.gridSpacing != spacing || v.snapGrid != snapGrid || v.snapObjects != snapObjects || !v.sharedGuides) {
        m_ed->setView([&](ViewOptions &o) {
            o.sharedGuides = true;
            o.gridSpacing = spacing;
            o.snapGrid = snapGrid;
            o.snapObjects = snapObjects;
        });
        m_canvas->viewport()->update();
    }
    if (showGuides) m_canvas->setGuides(g.showGuides.value_or(false));
}

void DeckWindow::createGridActions()
{
    mk("view.gridGuides", tr("Grid and Guides…"), "grid-3x3", QKeySequence(), [this] {
        Document *d = m_ed->doc();
        const deck::GridView was = deck::gridViewOf(*d);
        GridSettings s;
        s.spacing = m_ed->view.gridSpacing;
        s.showGrid = m_canvas->gridlines();
        s.snapGrid = m_ed->view.snapGrid;
        s.snapObjects = m_ed->view.snapObjects;
        s.showGuides = m_canvas->guides();
        s.guides = d->viewGuides;
        GridSettings now = s;
        if (!gridSettingsDialog(this, now, d->pageSize()) || now == s) return;
        deck::GridView g = was;
        g.spacing = now.spacing;
        g.snapGrid = now.snapGrid;
        g.snapObjects = now.snapObjects;
        g.showGuides = now.showGuides;
        RulerGuides guides = now.guides;
        deck::fitGuides(guides, d->pageSize());
        if (!(g == was) || guides.h != d->viewGuides.h || guides.v != d->viewGuides.v)
            m_ed->change(tr("Grid and Guides"), [&] {
                deck::setGridView(*d, g);
                d->viewGuides = guides;
            });
        Settings::get().setValue(kSpacingKey, now.spacing);
        Settings::get().setValue(kSnapGridKey, now.snapGrid);
        Settings::get().setValue(kSnapObjectsKey, now.snapObjects);
        m_canvas->setGridlines(now.showGrid);
        applyGridView(true);
        refreshUi();
    });
}

} // namespace jp
