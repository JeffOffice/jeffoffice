#include "core/scenarios.h"

#include "sheet/cellref.h"

#include <QCoreApplication>
#include <QRegularExpression>

namespace jp::crunch {

bool parseScenarioCells(const QString &text, QStringList *cells, QString *problem)
{
    using namespace jp::office::sheet;
    auto refuse = [&](const QString &why) {
        if (problem) *problem = why;
        return false;
    };
    QStringList out;
    const QStringList parts = text.split(QRegularExpression(QStringLiteral("[,;\\s]+")), Qt::SkipEmptyParts);
    if (parts.isEmpty()) return refuse(QCoreApplication::translate("jp::crunch::Scenarios", "Write the cells the scenario changes, such as B1:B3, D5."));
    for (const QString &part : parts) {
        CellRange range;
        QString clean = part;
        clean.remove(QLatin1Char('$'));
        if (!parseRangeRef(clean, &range) || !range.valid) return refuse(QCoreApplication::translate("jp::crunch::Scenarios", "\"%1\" is not a cell or a range of cells.").arg(part.left(40)));
        if (qint64(range.row2 - range.row1 + 1) * qint64(range.col2 - range.col1 + 1) > kMaxScenarioCells)
            return refuse(QCoreApplication::translate("jp::crunch::Scenarios", "A scenario can change at most %1 cells.").arg(kMaxScenarioCells));
        for (quint32 r = range.row1; r <= range.row2; ++r)
            for (quint32 c = range.col1; c <= range.col2; ++c) {
                const QString name = cellName(r, c);
                if (out.contains(name)) continue;
                if (out.size() >= kMaxScenarioCells) return refuse(QCoreApplication::translate("jp::crunch::Scenarios", "A scenario can change at most %1 cells.").arg(kMaxScenarioCells));
                out << name;
            }
    }
    if (cells) *cells = out;
    return true;
}

} // namespace jp::crunch
