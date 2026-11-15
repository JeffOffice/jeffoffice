// The file's sort state of a filter or a table, made from a sort and read back: see sortstate.h.
#include "core/sortstate.h"

#include "grid/condformat.h"
#include "sheet/cellref.h"
#include "sheet/sheetreader.h"

#include <QCoreApplication>
#include <algorithm>
#include <QRegularExpression>
#include <QXmlStreamReader>

namespace jp::crunch {

using namespace jp::office::sheet;

namespace {
using jp::grid::iconSetCount;

// The word for a color, and for a shape, as the cells draw them; each one a literal the translator sees.
QString colorWord(const QString &set, int place, int count)
{
    if (set == QLatin1String("4RedToBlack")) return place == 0 ? QCoreApplication::translate("jp::crunch::SortState", "Black") : place == 1 ? QCoreApplication::translate("jp::crunch::SortState", "Gray") : place == 2 ? QCoreApplication::translate("jp::crunch::SortState", "Pink") : QCoreApplication::translate("jp::crunch::SortState", "Red");
    if (set == QLatin1String("4TrafficLights")) return place == 0 ? QCoreApplication::translate("jp::crunch::SortState", "Black") : place == 1 ? QCoreApplication::translate("jp::crunch::SortState", "Red") : place == 2 ? QCoreApplication::translate("jp::crunch::SortState", "Yellow") : QCoreApplication::translate("jp::crunch::SortState", "Green");
    if (set.contains(QLatin1String("Gray"))) return QCoreApplication::translate("jp::crunch::SortState", "Gray");
    if (set.contains(QLatin1String("Stars"))) return QCoreApplication::translate("jp::crunch::SortState", "Gold");
    if (set.contains(QLatin1String("Rating")) || set.contains(QLatin1String("Quarters"))) return QCoreApplication::translate("jp::crunch::SortState", "Black");
    if (set.contains(QLatin1String("Boxes"))) return QCoreApplication::translate("jp::crunch::SortState", "Gray");
    if (count == 3) return place == 0 ? QCoreApplication::translate("jp::crunch::SortState", "Red") : place == 1 ? QCoreApplication::translate("jp::crunch::SortState", "Yellow") : QCoreApplication::translate("jp::crunch::SortState", "Green");
    return place == 0 ? QCoreApplication::translate("jp::crunch::SortState", "Red") : place == count - 1 ? QCoreApplication::translate("jp::crunch::SortState", "Green") : QCoreApplication::translate("jp::crunch::SortState", "Yellow");
}
}

QStringList sortedIconKeys(const QStringList &keys)
{
    QStringList out = keys;
    std::stable_sort(out.begin(), out.end(), [](const QString &a, const QString &b) {
        auto order = [](const QString &k) {
            const qsizetype colon = k.lastIndexOf(QLatin1Char(':'));
            const int set = jp::grid::iconSetNames().indexOf(colon < 0 ? k : k.left(colon));
            return std::pair<int, int>(set < 0 ? 1000 : set, colon < 0 ? 0 : k.mid(colon + 1).toInt());
        };
        return order(a) < order(b);
    });
    return out;
}

QString iconLabel(const QString &key)
{
    const qsizetype colon = key.lastIndexOf(QLatin1Char(':'));
    if (colon < 0) return QCoreApplication::translate("jp::crunch::SortState", "No icon");
    const QString set = key.left(colon);
    const int place = key.mid(colon + 1).toInt(), count = iconSetCount(set);
    if (count == 0 || place < 0 || place >= count) return QCoreApplication::translate("jp::crunch::SortState", "%1, icon %2").arg(set).arg(place + 1);
    const QString color = colorWord(set, place, count);
    auto join = [&](const QString &shape) { return QCoreApplication::translate("jp::crunch::SortState", "%1 %2").arg(color, shape); };
    if (set.contains(QLatin1String("Arrows"))) {
        const double turn = count == 3 ? (place - 1) * 90.0 : count == 4 ? (place - 1.5) * 60.0 : (place - 2) * 45.0;
        if (turn <= -89) return join(QCoreApplication::translate("jp::crunch::SortState", "down arrow"));
        if (turn >= 89) return join(QCoreApplication::translate("jp::crunch::SortState", "up arrow"));
        if (turn > -1 && turn < 1) return join(QCoreApplication::translate("jp::crunch::SortState", "sideways arrow"));
        return join(turn < 0 ? QCoreApplication::translate("jp::crunch::SortState", "angled down arrow") : QCoreApplication::translate("jp::crunch::SortState", "angled up arrow"));
    }
    if (set.contains(QLatin1String("Triangles"))) return place == 1 ? join(QCoreApplication::translate("jp::crunch::SortState", "dash")) : join(place == 2 ? QCoreApplication::translate("jp::crunch::SortState", "up triangle") : QCoreApplication::translate("jp::crunch::SortState", "down triangle"));
    if (set.contains(QLatin1String("Flags"))) return join(QCoreApplication::translate("jp::crunch::SortState", "flag"));
    if (set.contains(QLatin1String("Signs"))) return join(place == 0 ? QCoreApplication::translate("jp::crunch::SortState", "diamond") : place == 1 ? QCoreApplication::translate("jp::crunch::SortState", "triangle") : QCoreApplication::translate("jp::crunch::SortState", "circle"));
    if (set.contains(QLatin1String("Symbols"))) {
        const bool bare = set.contains(QLatin1String("Symbols2"));
        if (bare) return join(place == 0 ? QCoreApplication::translate("jp::crunch::SortState", "cross") : place == 1 ? QCoreApplication::translate("jp::crunch::SortState", "exclamation mark") : QCoreApplication::translate("jp::crunch::SortState", "check mark"));
        return join(place == 0 ? QCoreApplication::translate("jp::crunch::SortState", "circle with a cross") : place == 1 ? QCoreApplication::translate("jp::crunch::SortState", "circle with an exclamation mark") : QCoreApplication::translate("jp::crunch::SortState", "circle with a check mark"));
    }
    if (set.contains(QLatin1String("Stars"))) return place == 0 ? QCoreApplication::translate("jp::crunch::SortState", "Empty gold star") : place == 1 ? QCoreApplication::translate("jp::crunch::SortState", "Half gold star") : QCoreApplication::translate("jp::crunch::SortState", "Full gold star");
    if (set.contains(QLatin1String("Rating"))) return QCoreApplication::translate("jp::crunch::SortState", "Black bars, %1 of %2 filled").arg(place + 1).arg(count);
    if (set.contains(QLatin1String("Quarters"))) {
        if (place == 0) return QCoreApplication::translate("jp::crunch::SortState", "Empty black circle");
        if (place == count - 1) return QCoreApplication::translate("jp::crunch::SortState", "Full black circle");
        return QCoreApplication::translate("jp::crunch::SortState", "Black circle, %1 of %2 filled").arg(place).arg(count - 1);
    }
    if (set.contains(QLatin1String("Boxes"))) {
        if (place == 0) return QCoreApplication::translate("jp::crunch::SortState", "Empty gray box");
        if (place == count - 1) return QCoreApplication::translate("jp::crunch::SortState", "Full gray box");
        return QCoreApplication::translate("jp::crunch::SortState", "Gray box, %1 of %2 filled").arg(place).arg(count - 1);
    }
    return join(set.contains(QLatin1String("TrafficLights2")) ? QCoreApplication::translate("jp::crunch::SortState", "circle on a dark square") : QCoreApplication::translate("jp::crunch::SortState", "circle"));
}

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
        if (level.by == SortLevel::By::Icon) {
            // (cells with no icon are the set "NoIcons")
            const qsizetype colon = level.icon.lastIndexOf(QLatin1Char(':'));
            const QString set = colon < 0 ? QStringLiteral("NoIcons") : level.icon.left(colon), place = colon < 0 ? QStringLiteral("0") : level.icon.mid(colon + 1);
            out += QStringLiteral(" sortBy=\"icon\" iconSet=\"%1\" iconId=\"%2\"").arg(set.toHtmlEscaped(), place);
            descending = !level.onTop;
        } else if (level.by == SortLevel::By::Value) {
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
            } else if (by == QLatin1String("icon")) {
                const QString set = r.attributes().value(QLatin1String("iconSet")).toString();
                bool ok = false;
                const int place = r.attributes().value(QLatin1String("iconId")).toInt(&ok);
                if (set.isEmpty() || !ok || place < 0) return false;
                level.by = SortLevel::By::Icon;
                level.icon = set == QLatin1String("NoIcons") ? QString() : QStringLiteral("%1:%2").arg(set).arg(place);
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
