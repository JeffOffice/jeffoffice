#pragma once
// The dialogs for a page's guides and grid. gridGuidesDialog is JeffPub's Layout Guides (margins, columns and rows, baselines); gridSettingsDialog is a
// presentation's Grid and Guides: the grid's spacing, what snaps to what, and the guides of the whole document.

#include "core/document.h"

#include <QSizeF>

class QWidget;

namespace jp {

class Editor;

// Layout Guides: Margin Guides (tab 0), Grid Guides (1), Baseline Guides (2).
void gridGuidesDialog(QWidget *p, Editor *ed, int startTab = 1);

// What the Grid and Guides dialog sets. The spacing is in points.
struct GridSettings {
    double spacing = 6;
    bool showGrid = false;
    bool snapGrid = false;
    bool snapObjects = true;
    bool showGuides = false;
    RulerGuides guides;
    bool operator==(const GridSettings &o) const
    {
        return spacing == o.spacing && showGrid == o.showGrid && snapGrid == o.snapGrid && snapObjects == o.snapObjects && showGuides == o.showGuides && guides.h == o.guides.h && guides.v == o.guides.v;
    }
};

// The least spacing of a grid, and the most guides one way (a file can say more; they are left out).
constexpr double kMinGridSpacing = 1;
constexpr int kMaxGuidesPerAxis = 64;

// Grid and Guides for a page of `page` points. Returns whether it was accepted; `s` is changed only then.
bool gridSettingsDialog(QWidget *p, GridSettings &s, const QSizeF &page);

} // namespace jp
