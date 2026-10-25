#include "core/namesfromcells.h"

#include "sheet/namerefs.h"

#include <QSet>

namespace jp::crunch {

using jp::office::sheet::CellRange;

QString nameFromLabel(const QString &label)
{
    QString name;
    bool anyWord = false;
    for (const QChar c : label.trimmed()) {
        if (c.isLetterOrNumber()) {
            name += c;
            anyWord = true;
        } else if (c == QLatin1Char('_') || c == QLatin1Char('.')) {
            name += c;
        } else {
            name += QLatin1Char('_');   // (a space, or a mark a name can't have)
        }
    }
    if (!anyWord) return QString();
    if (!name[0].isLetter() && name[0] != QLatin1Char('_')) name.prepend(QLatin1Char('_'));
    if (!jp::office::sheet::nameProblem(name.left(255)).isEmpty()) name.prepend(QLatin1Char('_'));   // (a cell's address, such as Q1)
    name = name.left(255);
    return jp::office::sheet::nameProblem(name).isEmpty() ? name : QString();
}

std::vector<LabelName> namesFromLabels(const CellRange &block, int edges, const std::function<QString(quint32, quint32)> &labelAt, int *skipped)
{
    std::vector<LabelName> out;
    if (skipped) *skipped = 0;
    if (!block.valid) return out;
    QSet<QString> made;
    auto add = [&](const QString &label, quint32 r1, quint32 c1, quint32 r2, quint32 c2) {
        const QString name = nameFromLabel(label);
        if (name.isEmpty() || made.contains(name.toUpper())) {
            if (skipped) ++*skipped;
            return;
        }
        made.insert(name.toUpper());
        LabelName n;
        n.name = name;
        n.cells.valid = true;
        n.cells.row1 = r1;
        n.cells.col1 = c1;
        n.cells.row2 = r2;
        n.cells.col2 = c2;
        out.push_back(n);
    };
    const bool tall = block.row2 > block.row1, wide = block.col2 > block.col1;
    if ((edges & TopEdge) && tall)
        for (quint32 c = block.col1; c <= block.col2; ++c) add(labelAt(block.row1, c), block.row1 + 1, c, block.row2, c);
    if ((edges & LeftEdge) && wide)
        for (quint32 r = block.row1; r <= block.row2; ++r) add(labelAt(r, block.col1), r, block.col1 + 1, r, block.col2);
    if ((edges & BottomEdge) && tall)
        for (quint32 c = block.col1; c <= block.col2; ++c) add(labelAt(block.row2, c), block.row1, c, block.row2 - 1, c);
    if ((edges & RightEdge) && wide)
        for (quint32 r = block.row1; r <= block.row2; ++r) add(labelAt(r, block.col2), r, block.col1, r, block.col2 - 1);
    return out;
}

} // namespace jp::crunch
