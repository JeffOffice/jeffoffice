#pragma once
// What the slides pane and the slide sorter share for sections: how a
// section's heading is drawn, and the menu a heading opens.

#include <QRect>
#include <QString>
#include <functional>

class QMenu;
class QPainter;

namespace jp {

// The height of a heading, which the slide sorter lays out with.
constexpr int kSectionHeaderHeight = 26;

// A section's heading in `r`: a triangle that points down when the section is
// open and right when it is folded away, its name, and how many slides it has.
void paintSectionHeader(QPainter *p, const QRect &r, const QString &name, int count, bool collapsed, bool hover);

// What the commands of a heading's menu do.
struct SectionMenu {
    std::function<void()> rename, remove, removeWithSlides, moveUp, moveDown, collapseAll, expandAll;
    bool canMoveUp = true, canMoveDown = true;
};
void fillSectionMenu(QMenu &menu, const SectionMenu &commands);

} // namespace jp
