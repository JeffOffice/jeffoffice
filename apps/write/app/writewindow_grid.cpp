// JeffWrite's drawing grid and alignment guides (View tab): dots over the page, and what a picture, shape, or text box set at a place on the page goes to when the mouse carries or sizes it.
// They are the person's choices, kept for every document and not saved in one; the engine is libs/draw/core/snapguides.h and the settings dialog is the one of libs/page that JeffDeck uses.

#include "app/writewindow.h"

#include "app/guidesdialog.h"
#include "app/pageview.h"
#include "app/settings.h"

#include <QAction>
#include <QSignalBlocker>
#include <QStatusBar>
#include <cmath>

namespace jp {

namespace {
const QString kShowKey = QStringLiteral("write/drawingGrid");
const QString kSpacingKey = QStringLiteral("write/gridSpacing");
const QString kSnapGridKey = QStringLiteral("write/snapToGrid");
const QString kGuidesKey = QStringLiteral("write/alignmentGuides");
}

// The view takes the person's choices as they are kept.
void WriteWindow::applyGridView()
{
    SnapOptions o;
    o.showGrid = Settings::get().value(kShowKey, false).toBool();
    o.snapGrid = Settings::get().value(kSnapGridKey, false).toBool();
    o.snapObjects = Settings::get().value(kGuidesKey, false).toBool();
    const double saved = Settings::get().value(kSpacingKey, kDefaultSnapSpacing).toDouble();
    o.spacing = std::isfinite(saved) ? std::clamp(saved, kMinGridSpacing, kMostSnapSpacing) : kDefaultSnapSpacing;
    for (const auto &c : {std::pair<const char *, bool>{"view.drawingGrid", o.showGrid}, {"view.snapGrid", o.snapGrid}, {"view.alignmentGuides", o.snapObjects}}) {
        QAction *a = act(QString::fromLatin1(c.first));
        const QSignalBlocker block(a);
        a->setChecked(c.second);
    }
    m_view->setObjectSnap(o);
}

// A choice of a command or of the dialog is kept, and every window hears it.
void WriteWindow::keepGridChoices(const SnapOptions &o)
{
    Settings::get().setValue(kShowKey, o.showGrid);
    Settings::get().setValue(kSnapGridKey, o.snapGrid);
    Settings::get().setValue(kGuidesKey, o.snapObjects);
    Settings::get().setValue(kSpacingKey, o.spacing);
    Q_EMIT ObjectSnapNotifier::get().changed();
}

void WriteWindow::createGridActions()
{
    auto toggle = [this](const char *id, auto set, const QString &on, const QString &off) {
        return [this, id, set, on, off] {
            SnapOptions o = m_view->objectSnap();
            const bool checked = act(QString::fromLatin1(id))->isChecked();
            set(o, checked);
            keepGridChoices(o);
            statusBar()->showMessage(checked ? on : off, 6000);
        };
    };
    mk("view.drawingGrid", tr("Drawing Grid"), "grid-2x2", QKeySequence(),
       toggle("view.drawingGrid", [](SnapOptions &o, bool v) { o.showGrid = v; }, tr("The drawing grid is shown on the page."), tr("The drawing grid is hidden.")), true);
    mk("view.snapGrid", tr("Snap to Grid"), "magnet", QKeySequence(),
       toggle("view.snapGrid", [](SnapOptions &o, bool v) { o.snapGrid = v; },
              tr("Snap to Grid is on: pictures, shapes, and text boxes go to the grid as you drag them."), tr("Snap to Grid is off.")), true);
    mk("view.alignmentGuides", tr("Alignment Guides"), "layout-grid", QKeySequence(),
       toggle("view.alignmentGuides", [](SnapOptions &o, bool v) { o.snapObjects = v; },
              tr("Alignment Guides are on: lines show when an object lines up with another object, the page, or the margins, and it snaps to them."), tr("Alignment Guides are off.")), true);
    mk("view.gridGuides", tr("Grid and Guides…"), "grid-3x3", QKeySequence(), [this] {
        GridSettings s;
        s.forPage = true;
        const SnapOptions o = m_view->objectSnap();
        s.spacing = o.spacing;
        s.showGrid = o.showGrid;
        s.snapGrid = o.snapGrid;
        s.snapObjects = o.snapObjects;
        GridSettings now = s;
        if (!gridSettingsDialog(this, now, QSizeF()) || now == s) return;
        SnapOptions n;
        n.spacing = now.spacing;
        n.showGrid = now.showGrid;
        n.snapGrid = now.snapGrid;
        n.snapObjects = now.snapObjects;
        keepGridChoices(n);
    });
    // (a choice made in any window is the choice of every window at once)
    connect(&ObjectSnapNotifier::get(), &ObjectSnapNotifier::changed, this, [this] { applyGridView(); });
}

} // namespace jp
