// The file's sort state of a filter or a table, made from a sort and read back: see sortstate.h.
#include "core/sortstate.h"

#include "sheet/cellref.h"
#include "sheet/sheetreader.h"

#include <QCoreApplication>
#include <QRegularExpression>
#include <QXmlStreamReader>

namespace jp::crunch {

using namespace jp::office::sheet;

QString sortStateXml(const CellRange &data, const SortSpec &spec, Workbook &book)
{
    if (!data.valid || spec.levels.isEmpty()) return QString();
    QString out = QStringLiteral("<sortState ref=\"%1\"").arg(rangeName(data));
    if (spec.leftToRight) out += QStringLiteral(" columnSort=\"1\"");
    if (spec.caseSensitive) out += QStringLiteral(" caseSensitive=\"1\"");
    out += QLatin1Char('>');
    for (const SortLevel &level : spec.levels) {
        CellRange line = data;
        if (spec.leftToRight) line.row1 = line.row2 = data.row1 + quint32(level.key);
        else line.col1 = line.col2 = data.col1 + quint32(level.key);
        out += QStringLiteral("<sortCondition ref=\"%1\"").arg(rangeName(line));
        bool descending = level.descending;
        if (level.by == SortLevel::By::Value) {
            if (!level.list.isEmpty()) {
                QStringList items;
                for (const QString &item : level.list) {
                    if (item.contains(QLatin1Char(','))) return QString();
                    items << item.toHtmlEscaped();
                }
                out += QStringLiteral(" customList=\"%1\"").arg(items.join(QLatin1Char(',')));
            }
        } else {
            const bool font = level.by == SortLevel::By::FontColor;
            const qint32 dxf = level.color.isSet() ? book.dxfForColor(level.color, font) : -1;
            if (dxf < 0) return QString();
            out += QStringLiteral(" sortBy=\"%1\" dxfId=\"%2\"").arg(font ? QStringLiteral("fontColor") : QStringLiteral("cellColor")).arg(dxf);
            descending = !level.onTop;
        }
        if (descending) out += QStringLiteral(" descending=\"1\"");
        out += QStringLiteral("/>");
    }
    return out + QStringLiteral("</sortState>");
}

bool parseSortState(const QString &xml, const Styles &styles, CellRange *data, SortSpec *spec)
{
    QXmlStreamReader r(xml);
    SortSpec out;
    CellRange range;
    bool any = false;
    while (!r.atEnd()) {
        r.readNext();
        if (!r.isStartElement()) continue;
        const QStringView name = r.name();
        if (name == QLatin1String("sortState")) {
            if (!parseRangeRef(r.attributes().value(QLatin1String("ref")), &range)) return false;
            range.valid = true;
            const QString column = r.attributes().value(QLatin1String("columnSort")).toString(), match = r.attributes().value(QLatin1String("caseSensitive")).toString();
            out.leftToRight = column == QLatin1String("1") || column == QLatin1String("true");
            out.caseSensitive = match == QLatin1String("1") || match == QLatin1String("true");
        } else if (name == QLatin1String("sortCondition") && range.valid) {
            CellRange line;
            if (!parseRangeRef(r.attributes().value(QLatin1String("ref")), &line)) return false;
            SortLevel level;
            const qint64 key = out.leftToRight ? qint64(line.row1) - range.row1 : qint64(line.col1) - range.col1;
            if (key < 0 || key >= (out.leftToRight ? qint64(range.row2) - range.row1 + 1 : qint64(range.col2) - range.col1 + 1) || out.levels.size() >= kMaxSortLevels) return false;
            level.key = int(key);
            const QString descending = r.attributes().value(QLatin1String("descending")).toString(), by = r.attributes().value(QLatin1String("sortBy")).toString();
            level.descending = descending == QLatin1String("1") || descending == QLatin1String("true");
            if (by == QLatin1String("cellColor") || by == QLatin1String("fontColor")) {
                const bool font = by == QLatin1String("fontColor");
                bool ok = false;
                const int dxf = r.attributes().value(QLatin1String("dxfId")).toInt(&ok);
                if (!ok || dxf < 0 || dxf >= styles.dxfs.size()) return false;
                const DifferentialFormat &d = styles.dxfs[dxf];
                level.by = font ? SortLevel::By::FontColor : SortLevel::By::CellColor;
                level.color = font ? d.fontColor : (d.fill.bg.isSet() ? d.fill.bg : d.fill.fg);
                level.onTop = !level.descending;
                level.descending = false;
            } else if (by.isEmpty() || by == QLatin1String("value")) {
                const QString list = r.attributes().value(QLatin1String("customList")).toString();
                if (!list.isEmpty()) level.list = list.split(QLatin1Char(','));
            } else {
                return false;
            }
            out.levels.append(level);
            any = true;
        }
    }
    if (r.hasError() || !any || !range.valid) return false;
    *data = range;
    *spec = out;
    return true;
}

} // namespace jp::crunch
