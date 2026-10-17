// Tables in DrawingML (drawingml_table.h).

#include "office/drawingml_table.h"

#include "office/drawingml_style.h"
#include "office/tablestyles.h"
#include "office/xml.h"

#include <QDomDocument>
#include <QHash>
#include <QRegularExpression>
#include <QTextDocument>
#include <QXmlStreamReader>
#include <algorithm>
#include <array>
#include <cmath>
#include <optional>
#include <utility>
#include <vector>

namespace jp::office {

namespace {

// A table is kept to what TableItem::fromJson reads back, and to a size whose cells are not too many stories.
constexpr int kMaxRowsOrColumns = 256;
constexpr int kMaxCells = 10000;
constexpr int kTableCells = 2;       // what a table costs a budget of cells besides its own: measured, a table of one cell is as heavy as three
constexpr double kMaxPoint = 1e6;   // points: the farthest a position or a size is taken to be

const QLatin1String kTableUri("http://schemas.openxmlformats.org/drawingml/2006/table");
const QLatin1String kStrictTableUri("http://purl.oclc.org/ooxml/drawingml/table");

double position(double v)
{
    return std::isfinite(v) ? std::clamp(v, -kMaxPoint, kMaxPoint) : 0.0;
}

QDomElement styleById(const QDomElement &list, const QString &id)
{
    for (QDomElement s = list.firstChildElement(); !s.isNull(); s = s.nextSiblingElement())
        if (s.localName() == QLatin1String("tblStyle") && attr(s, QStringLiteral("styleId")).compare(id, Qt::CaseInsensitive) == 0) return s;
    return QDomElement();
}

QVector<QDomElement> childrenNamed(const QDomElement &e, const char *local)
{
    QVector<QDomElement> out;
    for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement())
        if (c.localName() == QLatin1String(local)) out.append(c);
    return out;
}

// The parts of a table a style has something to say about, in the order the
// standard lists them: each takes over from the ones before.
enum Region { WholeTbl, Band1H, Band2H, Band1V, Band2V, LastCol, FirstCol, LastRow, SeCell, SwCell, FirstRow, NeCell, NwCell, RegionCount };
const char *const kRegionNames[RegionCount] = {"wholeTbl", "band1H", "band2H", "band1V", "band2V", "lastCol", "firstCol",
                                               "lastRow", "seCell", "swCell", "firstRow", "neCell", "nwCell"};

// The sides of a region's border: the table's edges, and between its cells.
enum Side { Left, Right, Top, Bottom, InsideH, InsideV, SideCount };
const char *const kSideNames[SideCount] = {"left", "right", "top", "bottom", "insideH", "insideV"};

// What one region of a table style says.
struct Look {
    bool hasFill = false;
    Fill fill;
    bool hasBorder[SideCount] = {};
    Stroke border[SideCount];
    std::optional<bool> bold, italic;
    QColor color;
};

struct Flags {
    bool firstRow = false, lastRow = false, firstCol = false, lastCol = false, bandRow = false, bandCol = false;
    int rows = 0, cols = 0;
};

bool inRegion(Region region, int r, int c, const Flags &f)
{
    const int rowStart = f.firstRow ? 1 : 0, colStart = f.firstCol ? 1 : 0;
    switch (region) {
    case WholeTbl: return true;
    case Band1H: return f.bandRow && r >= rowStart && (r - rowStart) % 2 == 0;
    case Band2H: return f.bandRow && r >= rowStart && (r - rowStart) % 2 == 1;
    case Band1V: return f.bandCol && c >= colStart && (c - colStart) % 2 == 0;
    case Band2V: return f.bandCol && c >= colStart && (c - colStart) % 2 == 1;
    case LastCol: return f.lastCol && c == f.cols - 1;
    case FirstCol: return f.firstCol && c == 0;
    case LastRow: return f.lastRow && r == f.rows - 1;
    case SeCell: return f.lastRow && f.lastCol && r == f.rows - 1 && c == f.cols - 1;
    case SwCell: return f.lastRow && f.firstCol && r == f.rows - 1 && c == 0;
    case FirstRow: return f.firstRow && r == 0;
    case NeCell: return f.firstRow && f.lastCol && r == 0 && c == f.cols - 1;
    case NwCell: return f.firstRow && f.firstCol && r == 0 && c == 0;
    case RegionCount: break;
    }
    return false;
}

Look readLook(const QDomElement &region, const DrawingContext &ctx)
{
    Look look;
    if (region.isNull()) return look;
    const QDomElement text = childNamed(region, "tcTxStyle");
    if (!text.isNull()) {
        const QString b = attr(text, QStringLiteral("b")), i = attr(text, QStringLiteral("i"));
        if (b == QLatin1String("on")) look.bold = true;
        else if (b == QLatin1String("off")) look.bold = false;
        if (i == QLatin1String("on")) look.italic = true;
        else if (i == QLatin1String("off")) look.italic = false;
        // The color beside the font reference is the text's; that of the reference is what it falls back on.
        QColor c = readColor(text, ctx.theme, ctx.colors);
        if (!c.isValid()) c = readColor(childNamed(text, "fontRef"), ctx.theme, ctx.colors);
        if (c.isValid()) look.color = QColor(c.red(), c.green(), c.blue());
    }
    const QDomElement cell = childNamed(region, "tcStyle");
    const QDomElement borders = childNamed(cell, "tcBdr");
    for (int k = 0; k < SideCount; ++k) {
        const QDomElement side = childNamed(borders, kSideNames[k]);
        if (side.firstChildElement().isNull()) continue;
        look.hasBorder[k] = true;
        look.border[k] = readLine(side, side, ctx);   // an a:ln, or a reference to one of the theme's
    }
    const QDomElement fill = childNamed(cell, "fill");
    if (!fill.isNull() && hasOwnFill(fill)) {
        look.hasFill = true;
        look.fill = readFill(fill, QDomElement(), ctx);
    } else if (!childNamed(cell, "fillRef").isNull() && !ctx.formats.isNull()) {
        look.hasFill = true;
        look.fill = readFill(QDomElement(), cell, ctx);
    }
    return look;
}

std::array<Look, RegionCount> looksOf(const QDomElement &style, const DrawingContext &ctx)
{
    std::array<Look, RegionCount> looks;
    for (int k = 0; k < RegionCount; ++k) looks[k] = readLook(childNamed(style, kRegionNames[k]), ctx);
    return looks;
}

// What a style gives every cell of a table with these parts: the borders first, each region over the ones before, then the fill and the text.
TableStyleEffect effectOf(const std::array<Look, RegionCount> &looks, const Flags &flags)
{
    const int rows = flags.rows, cols = flags.cols;
    TableStyleEffect fx;
    fx.rows = rows;
    fx.cols = cols;
    fx.across.resize(size_t(rows + 1) * cols);
    fx.down.resize(size_t(rows) * (cols + 1));
    fx.fill.resize(size_t(rows) * cols);
    fx.bold.resize(size_t(rows) * cols);
    fx.italic.resize(size_t(rows) * cols);
    fx.color.resize(size_t(rows) * cols);
    for (int k = 0; k < RegionCount; ++k) {
        const Look &look = looks[k];
        const bool any = std::any_of(std::begin(look.hasBorder), std::end(look.hasBorder), [](bool b) { return b; });
        if (!any) continue;
        const auto in = [&](int r, int c) { return r >= 0 && r < rows && c >= 0 && c < cols && inRegion(Region(k), r, c, flags); };
        for (int r = 0; r < rows; ++r)
            for (int c = 0; c < cols; ++c) {
                if (!in(r, c)) continue;
                const int left = in(r, c - 1) ? InsideV : Left, right = in(r, c + 1) ? InsideV : Right;
                const int top = in(r - 1, c) ? InsideH : Top, bottom = in(r + 1, c) ? InsideH : Bottom;
                if (look.hasBorder[left]) fx.leftOf(r, c) = look.border[left];
                if (look.hasBorder[right]) fx.leftOf(r, c + 1) = look.border[right];
                if (look.hasBorder[top]) fx.above(r, c) = look.border[top];
                if (look.hasBorder[bottom]) fx.above(r + 1, c) = look.border[bottom];
            }
    }
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c) {
            const size_t at = size_t(r) * cols + c;
            for (int k = 0; k < RegionCount; ++k) {
                if (!inRegion(Region(k), r, c, flags)) continue;
                if (looks[k].hasFill) fx.fill[at] = looks[k].fill;
                if (looks[k].bold) fx.bold[at] = looks[k].bold;
                if (looks[k].italic) fx.italic[at] = looks[k].italic;
                if (looks[k].color.isValid()) fx.color[at] = looks[k].color;
            }
        }
    return fx;
}

Flags flagsOf(const TableItem &t)
{
    Flags f;
    f.rows = t.rows;
    f.cols = t.cols;
    f.firstRow = t.header;
    f.lastRow = t.totalRow;
    f.firstCol = t.firstCol;
    f.lastCol = t.lastCol;
    f.bandRow = t.banded;
    f.bandCol = t.bandedCols;
    return f;
}

// A cell's border line (a:lnL and its kin are lines themselves, as a:ln is).
Stroke cellLine(const QDomElement &e, const DrawingContext &ctx, QDomDocument *scratch)
{
    const QString a(ns::A);
    QDomElement props = scratch->createElementNS(a, QStringLiteral("a:spPr"));
    QDomElement ln = scratch->createElementNS(a, QStringLiteral("a:ln"));
    const QDomNamedNodeMap attrs = e.attributes();
    for (int i = 0; i < attrs.length(); ++i) {
        const QDomAttr at = attrs.item(i).toAttr();
        ln.setAttributeNS(QString(), at.localName(), at.value());
    }
    for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) ln.appendChild(scratch->importNode(c, true));
    props.appendChild(ln);
    return readLine(props, QDomElement(), ctx);
}

// Whether a cell's line says what it is (a fill, or none) and so replaces the style's.
bool saysLine(const QDomElement &e)
{
    for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) {
        const QString n = c.localName();
        if (n == QLatin1String("noFill") || n == QLatin1String("solidFill") || n == QLatin1String("gradFill") || n == QLatin1String("pattFill")) return true;
    }
    return false;
}

// The list style that gives a cell's text what its style says: bold, italic, and color, at every level.
QDomElement textLayer(QDomDocument *doc, std::optional<bool> bold, std::optional<bool> italic, const QColor &color)
{
    const QString a(ns::A);
    QDomElement list = doc->createElementNS(a, QStringLiteral("a:lstStyle"));
    for (int level = 1; level <= 9; ++level) {
        QDomElement lvl = doc->createElementNS(a, QStringLiteral("a:lvl%1pPr").arg(level));
        QDomElement run = doc->createElementNS(a, QStringLiteral("a:defRPr"));
        if (bold) run.setAttributeNS(QString(), QStringLiteral("b"), *bold ? QStringLiteral("1") : QStringLiteral("0"));
        if (italic) run.setAttributeNS(QString(), QStringLiteral("i"), *italic ? QStringLiteral("1") : QStringLiteral("0"));
        if (color.isValid()) {
            QDomElement fill = doc->createElementNS(a, QStringLiteral("a:solidFill"));
            QDomElement rgb = doc->createElementNS(a, QStringLiteral("a:srgbClr"));
            rgb.setAttributeNS(QString(), QStringLiteral("val"), color.name(QColor::HexRgb).mid(1).toUpper());
            fill.appendChild(rgb);
            run.appendChild(fill);
        }
        lvl.appendChild(run);
        list.appendChild(lvl);
    }
    return list;
}

int clampSpan(const QDomElement &tc, const char *name, int room)
{
    return int(std::clamp<qint64>(parseInt(attr(tc, QLatin1String(name)), 1), 1, std::max(1, room)));
}

} // namespace

bool isTableFrame(const QDomElement &graphicFrame)
{
    if (graphicFrame.isNull() || graphicFrame.localName() != QLatin1String("graphicFrame")) return false;
    const QString uri = attr(childNamed(childNamed(graphicFrame, "graphic"), "graphicData"), QStringLiteral("uri"));
    return uri == kTableUri || uri == kStrictTableUri;
}

ItemPtr readTable(const QDomElement &frame, const DrawingContext &ctx, const TextStyleStack &styles, const QDomElement &tableStyles)
{
    if (!isTableFrame(frame)) return nullptr;
    const QDomElement tbl = childNamed(childNamed(childNamed(frame, "graphic"), "graphicData"), "tbl");
    const QVector<QDomElement> gridCols = childrenNamed(childNamed(tbl, "tblGrid"), "gridCol");
    const QVector<QDomElement> trs = childrenNamed(tbl, "tr");
    const int cols = int(gridCols.size()), rows = int(trs.size());
    if (tbl.isNull() || cols == 0 || rows == 0 || cols > kMaxRowsOrColumns || rows > kMaxRowsOrColumns || cols * rows > kMaxCells) return nullptr;
    const QDomElement cNvPr = childNamed(childNamed(frame, "nvGraphicFramePr"), "cNvPr");
    if (parseBool(attr(cNvPr, QStringLiteral("hidden")))) return nullptr;
    // Each cell is a story of the import's budget, and a table is as heavy as two more cells (as much to read and to keep); one that would pass the
    // budget is left to the caller, as one that can't be read.
    if (ctx.newStory && ctx.budget && !ctx.budget->take(DrawingBudget::Cells, qint64(rows) * cols + kTableCells)) return nullptr;

    auto t = std::make_shared<TableItem>();
    t->rows = rows;
    t->cols = cols;
    t->name = attr(cNvPr, QStringLiteral("name"));
    t->altText = attr(cNvPr, QStringLiteral("descr"));
    t->altTitle = attr(cNvPr, QStringLiteral("title"));
    t->wrap.mode = Wrap::None;
    for (const QDomElement &g : gridCols) t->colW.append(std::clamp(parseLength(attr(g, QStringLiteral("w")), 0), 0.0, kMaxPoint));
    for (const QDomElement &r : trs) t->rowH.append(std::clamp(parseLength(attr(r, QStringLiteral("h")), 0), 0.0, kMaxPoint));
    t->cells.resize(rows * cols);
    // The grid, not the frame, says how big the table is.
    const QDomElement off = childNamed(childNamed(frame, "xfrm"), "off");
    t->rect = QRectF(QPointF(position(parseLength(attr(off, QStringLiteral("x")))), position(parseLength(attr(off, QStringLiteral("y"))))), QSizeF());
    t->syncRect();

    // The table's properties: what its parts are, its own fill, and its style.
    const QDomElement tblPr = childNamed(tbl, "tblPr");
    Flags flags;
    flags.rows = rows;
    flags.cols = cols;
    flags.firstRow = parseBool(attr(tblPr, QStringLiteral("firstRow")));
    flags.lastRow = parseBool(attr(tblPr, QStringLiteral("lastRow")));
    flags.firstCol = parseBool(attr(tblPr, QStringLiteral("firstCol")));
    flags.lastCol = parseBool(attr(tblPr, QStringLiteral("lastCol")));
    flags.bandRow = parseBool(attr(tblPr, QStringLiteral("bandRow")));
    flags.bandCol = parseBool(attr(tblPr, QStringLiteral("bandCol")));
    t->header = flags.firstRow;
    t->banded = flags.bandRow;
    t->totalRow = flags.lastRow;
    t->firstCol = flags.firstCol;
    t->lastCol = flags.lastCol;
    t->bandedCols = flags.bandCol;
    t->format = childNamed(tblPr, "tableStyleId").text().trimmed();
    if (hasOwnFill(tblPr)) t->fill = readFill(tblPr, QDomElement(), ctx);

    // The style: the file's definition, else the one we know by that id, else the usual style.
    std::array<Look, RegionCount> looks;
    QDomDocument fileStyles, builtinStyles;
    if (!t->format.isEmpty()) {
        QDomElement list = tableStyles;
        if (list.isNull() && ctx.package) {
            const QVector<Relationship> rels = ctx.package->relationshipsOfType(ctx.package->mainPart(), QStringLiteral("/tableStyles"));
            if (!rels.isEmpty() && !rels[0].external && parseXml(ctx.package->part(rels[0].target), &fileStyles)) list = fileStyles.documentElement();
        }
        QDomElement style = styleById(list, t->format);
        if (style.isNull()) {
            const QByteArray known = tableStyleXml(t->format);
            const QString id = known.isEmpty() ? defaultTableStyleId() : t->format;   // the usual style stands in for one we don't know: an approximation
            if (parseXml(known.isEmpty() ? tableStyleXml(id) : known, &builtinStyles)) style = styleById(builtinStyles.documentElement(), id);
        }
        looks = looksOf(style, ctx);
    }
    const TableStyleEffect fx = effectOf(looks, flags);

    // The cells' own spans first, so that what a merged cell covers is known.
    QDomDocument scratch;
    std::vector<QDomElement> tcs(size_t(rows) * cols);
    for (int r = 0; r < rows; ++r) {
        const QVector<QDomElement> row = childrenNamed(trs[r], "tc");
        for (int c = 0; c < cols && c < row.size(); ++c) {
            tcs[size_t(r) * cols + c] = row[c];
            TableCell &cell = t->cell(r, c);
            cell.colSpan = clampSpan(row[c], "gridSpan", cols - c);
            cell.rowSpan = clampSpan(row[c], "rowSpan", rows - r);
        }
    }
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c) {
            const TableCell origin = t->cell(r, c);
            if (origin.covered) continue;
            for (int rr = r; rr < r + origin.rowSpan; ++rr)
                for (int cc = c; cc < c + origin.colSpan; ++cc) {
                    if (rr == r && cc == c) continue;
                    TableCell &under = t->cell(rr, cc);
                    under.covered = true;
                    under.rowSpan = under.colSpan = 1;
                }
        }

    // The borders: the style's, then those the cells name.
    std::vector<Stroke> across = fx.across, down = fx.down;   // the lines above each row's cells; left of each column's
    const auto edgeAbove = [&](int r, int c) -> Stroke & { return across[size_t(r) * cols + c]; };
    const auto edgeLeft = [&](int r, int c) -> Stroke & { return down[size_t(r) * (cols + 1) + c]; };
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c) {
            const QDomElement pr = childNamed(tcs[size_t(r) * cols + c], "tcPr");
            TableCell &cell = t->cell(r, c);
            // A merged cell's far sides are the sides of what it covers.
            const struct { const char *name; Stroke *edge; } named[4] = {
                {"lnL", &edgeLeft(r, c)}, {"lnR", &edgeLeft(r, c + cell.colSpan)}, {"lnT", &edgeAbove(r, c)}, {"lnB", &edgeAbove(r + cell.rowSpan, c)}};
            for (const auto &n : named) {
                const QDomElement line = childNamed(pr, n.name);
                if (!line.isNull() && saysLine(line)) *n.edge = cellLine(line, ctx, &scratch);
            }
            if (cell.covered) continue;
            for (const auto &d : {std::make_pair("lnTlToBr", 1), std::make_pair("lnBlToTr", 2)}) {
                const QDomElement line = childNamed(pr, d.first);
                if (cell.diagonal || line.isNull() || !saysLine(line)) continue;
                const Stroke s = cellLine(line, ctx, &scratch);
                if (s.isNone()) continue;
                cell.diagonal = d.second;
                cell.diagonalLine = s;
            }
        }

    // Each cell: its text, then fill, borders, margins, and anchor.
    QHash<QString, QDomElement> layers;
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c) {
            const QDomElement tc = tcs[size_t(r) * cols + c];
            const QDomElement pr = childNamed(tc, "tcPr");
            TableCell &cell = t->cell(r, c);
            cell.hyphenate = false;

            Fill fill = fx.fill[size_t(r) * cols + c];
            const std::optional<bool> bold = fx.bold[size_t(r) * cols + c], italic = fx.italic[size_t(r) * cols + c];
            const QColor color = fx.color[size_t(r) * cols + c];
            TextStyleStack stack = styles;
            if (bold || italic || color.isValid()) {
                const QString key = QStringLiteral("%1/%2/%3").arg(bold ? int(*bold) : -1).arg(italic ? int(*italic) : -1).arg(color.isValid() ? color.name() : QString());
                if (!layers.contains(key)) layers.insert(key, textLayer(&scratch, bold, italic, color));
                stack.append(layers.value(key));
            }
            if (ctx.newStory) {
                cell.storyId = ctx.newStory();
                QTextDocument *doc = ctx.storyDoc ? ctx.storyDoc(cell.storyId) : nullptr;
                const QDomElement body = childNamed(tc, "txBody");
                if (doc && !body.isNull()) readTextBody(body, doc, ctx, stack);
            }
            if (cell.covered) continue;

            if (hasOwnFill(pr)) fill = readFill(pr, QDomElement(), ctx);
            cell.fill = fill;
            const auto inset = [&](const char *name, double fallback) {
                return hasAttr(pr, QLatin1String(name)) ? std::clamp(parseLength(attr(pr, QLatin1String(name)), fallback), 0.0, 10000.0) : fallback;
            };
            cell.margins = QMarginsF(inset("marL", 7.2), inset("marT", 3.6), inset("marR", 7.2), inset("marB", 3.6));
            const QString anchor = attr(pr, QStringLiteral("anchor"));
            cell.valign = anchor == QLatin1String("ctr") ? VAlign::Middle : anchor == QLatin1String("b") ? VAlign::Bottom : VAlign::Top;
            cell.vertical = attr(pr, QStringLiteral("vert")) == QLatin1String("vert");
            cell.border.left = edgeLeft(r, c);
            cell.border.right = edgeLeft(r, c + cell.colSpan);
            cell.border.top = edgeAbove(r, c);
            cell.border.bottom = edgeAbove(r + cell.rowSpan, c);
        }
    return t;
}

bool tableStyleEffect(const QString &styleId, const TableItem &table, const Theme &theme, TableStyleEffect *out, const TableStyleDefs *defs)
{
    if (table.rows <= 0 || table.cols <= 0) return false;
    QDomDocument doc;
    QDomElement style;
    const QByteArray defined = defs ? defs->value(styleId.toUpper()) : QByteArray();
    if (!defined.isEmpty()) {
        if (parseXml(defined, &doc)) style = doc.documentElement();
    } else if (const QByteArray xml = tableStyleXml(styleId); !xml.isEmpty() && parseXml(xml, &doc)) {
        style = styleById(doc.documentElement(), styleId);
    }
    if (style.isNull() || style.localName() != QLatin1String("tblStyle")) return false;
    DrawingContext ctx;
    ctx.theme = theme;
    *out = effectOf(looksOf(style, ctx), flagsOf(table));
    return true;
}

namespace {

QString emu(double points) { return QString::number(pointsToEmu(points)); }

// A plain color the way a shape's would be, until the drawing writer writes
// fills and lines itself: nothing but a color that is its own is written here.
void writeColor(QXmlStreamWriter &w, const ColorRef &color, double transparency)
{
    w.writeStartElement(QStringLiteral("a:solidFill"));
    w.writeStartElement(QStringLiteral("a:srgbClr"));
    w.writeAttribute(QStringLiteral("val"), color.rgbValue().name(QColor::HexRgb).mid(1).toUpper());
    if (transparency > 0) {
        w.writeEmptyElement(QStringLiteral("a:alpha"));
        w.writeAttribute(QStringLiteral("val"), QString::number(qint64(std::lround((1.0 - std::min(transparency, 1.0)) * 100000))));
    }
    w.writeEndElement();
    w.writeEndElement();
}

void writeCellFill(QXmlStreamWriter &w, const Fill &fill, DrawingWriteContext &ctx)
{
    if (fill.isNone()) w.writeEmptyElement(QStringLiteral("a:noFill"));
    else if (fill.type == Fill::Solid && fill.color.kind() == ColorRef::Rgb) writeColor(w, fill.color, fill.transparency);
    else writeFill(w, fill, ctx);
}

// What `xml` holds, with its outermost element called `name`.
void writeRenamed(QXmlStreamWriter &w, const QByteArray &xml, const QString &name)
{
    QXmlStreamReader r(xml);
    r.setNamespaceProcessing(false);
    int depth = 0;
    while (!r.atEnd()) {
        r.readNext();
        if (r.isStartElement()) {
            // With namespace processing off the names keep their prefixes, but writeCurrentToken would drop them.
            w.writeStartElement(depth++ == 0 ? name : r.qualifiedName().toString());
            w.writeAttributes(r.attributes());
        } else if (r.isEndElement()) {
            --depth;
            w.writeEndElement();
        } else if (depth > 0 && !r.isComment()) {
            w.writeCurrentToken(r);
        }
    }
}

// One of a cell's lines (a:lnL ...): a line of its own, or, where it has
// none, one that says so, as the style would otherwise give the edge one.
void writeCellLine(QXmlStreamWriter &w, const QString &name, const Stroke &s, DrawingWriteContext &ctx)
{
    if (s.isNone() || s.color.kind() != ColorRef::Rgb) {
        if (!s.isNone()) {
            QByteArray bytes;
            QXmlStreamWriter inner(&bytes);
            writeLine(inner, s, ctx);
            if (bytes.startsWith("<a:ln")) {
                writeRenamed(w, bytes, name);
                return;
            }
        }
        w.writeStartElement(name);
        w.writeEmptyElement(QStringLiteral("a:noFill"));
        w.writeEndElement();
        return;
    }
    static const char *const dashes[] = {"solid", "dot", "sysDot", "dash", "dashDot", "lgDash", "lgDashDot", "lgDashDotDot"};
    static const char *const compounds[] = {"sng", "dbl", "thickThin", "thinThick", "tri"};
    w.writeStartElement(name);
    w.writeAttribute(QStringLiteral("w"), emu(s.width));
    w.writeAttribute(QStringLiteral("cap"), s.cap == Qt::RoundCap ? QStringLiteral("rnd") : s.cap == Qt::SquareCap ? QStringLiteral("sq") : QStringLiteral("flat"));
    w.writeAttribute(QStringLiteral("cmpd"), QLatin1String(compounds[std::clamp(int(s.compound), 0, 4)]));
    w.writeAttribute(QStringLiteral("algn"), s.inset ? QStringLiteral("in") : QStringLiteral("ctr"));
    writeColor(w, s.color, s.transparency);
    w.writeEmptyElement(QStringLiteral("a:prstDash"));
    w.writeAttribute(QStringLiteral("val"), QLatin1String(dashes[std::clamp(int(s.dash), 0, 7)]));
    if (s.join == Qt::RoundJoin) w.writeEmptyElement(QStringLiteral("a:round"));
    else if (s.join == Qt::BevelJoin) w.writeEmptyElement(QStringLiteral("a:bevel"));
    w.writeEndElement();
}

} // namespace

void writeTable(QXmlStreamWriter &w, const TableItem &t, DrawingWriteContext &ctx)
{
    // A table needs a column and a row to be one; what has none, or lacks the sizes or cells it says it has, is nothing to write.
    if (t.rows <= 0 || t.cols <= 0 || t.colW.size() < t.cols || t.rowH.size() < t.rows || t.cells.size() < qsizetype(t.rows) * t.cols) return;
    const int id = shapeNumber(ctx, t);
    w.writeStartElement(QStringLiteral("p:graphicFrame"));
    w.writeStartElement(QStringLiteral("p:nvGraphicFramePr"));
    w.writeEmptyElement(QStringLiteral("p:cNvPr"));
    w.writeAttribute(QStringLiteral("id"), QString::number(id));
    w.writeAttribute(QStringLiteral("name"), t.name.isEmpty() ? QStringLiteral("Table %1").arg(id - 1) : t.name);
    if (!t.altText.isEmpty()) w.writeAttribute(QStringLiteral("descr"), t.altText);
    if (!t.altTitle.isEmpty()) w.writeAttribute(QStringLiteral("title"), t.altTitle);
    w.writeStartElement(QStringLiteral("p:cNvGraphicFramePr"));
    w.writeEmptyElement(QStringLiteral("a:graphicFrameLocks"));
    w.writeAttribute(QStringLiteral("noGrp"), QStringLiteral("1"));
    w.writeEndElement();
    w.writeStartElement(QStringLiteral("p:nvPr"));
    if (t.role.startsWith(QLatin1String("ph:"))) {
        const QStringList parts = t.role.split(QLatin1Char(':'));
        w.writeEmptyElement(QStringLiteral("p:ph"));
        if (parts.value(1) != QLatin1String("obj") && !parts.value(1).isEmpty()) w.writeAttribute(QStringLiteral("type"), parts.value(1));
        if (parts.value(2).toLongLong() > 0) w.writeAttribute(QStringLiteral("idx"), parts.value(2));
    }
    w.writeEndElement();
    w.writeEndElement();

    qint64 width = 0, height = 0;
    for (double v : t.colW) width += pointsToEmu(v);
    for (double v : t.rowH) height += pointsToEmu(v);
    w.writeStartElement(QStringLiteral("p:xfrm"));
    w.writeEmptyElement(QStringLiteral("a:off"));
    w.writeAttribute(QStringLiteral("x"), emu(t.rect.x()));
    w.writeAttribute(QStringLiteral("y"), emu(t.rect.y()));
    w.writeEmptyElement(QStringLiteral("a:ext"));
    w.writeAttribute(QStringLiteral("cx"), QString::number(width));
    w.writeAttribute(QStringLiteral("cy"), QString::number(height));
    w.writeEndElement();

    w.writeStartElement(QStringLiteral("a:graphic"));
    w.writeStartElement(QStringLiteral("a:graphicData"));
    w.writeAttribute(QStringLiteral("uri"), kTableUri);
    w.writeStartElement(QStringLiteral("a:tbl"));
    w.writeStartElement(QStringLiteral("a:tblPr"));
    if (t.header) w.writeAttribute(QStringLiteral("firstRow"), QStringLiteral("1"));
    if (t.totalRow) w.writeAttribute(QStringLiteral("lastRow"), QStringLiteral("1"));
    if (t.firstCol) w.writeAttribute(QStringLiteral("firstCol"), QStringLiteral("1"));
    if (t.lastCol) w.writeAttribute(QStringLiteral("lastCol"), QStringLiteral("1"));
    if (t.banded) w.writeAttribute(QStringLiteral("bandRow"), QStringLiteral("1"));
    if (t.bandedCols) w.writeAttribute(QStringLiteral("bandCol"), QStringLiteral("1"));
    if (!t.fill.isNone()) writeCellFill(w, t.fill, ctx);
    // Only a style the presentation program has by id (a GUID) can be named.
    static const QRegularExpression guid(QStringLiteral("^\\{[0-9A-Fa-f]{8}(-[0-9A-Fa-f]{4}){3}-[0-9A-Fa-f]{12}\\}$"));
    if (guid.match(t.format).hasMatch()) {
        w.writeStartElement(QStringLiteral("a:tableStyleId"));
        w.writeCharacters(t.format);
        w.writeEndElement();
    }
    w.writeEndElement();
    w.writeStartElement(QStringLiteral("a:tblGrid"));
    for (double v : t.colW) {
        w.writeEmptyElement(QStringLiteral("a:gridCol"));
        w.writeAttribute(QStringLiteral("w"), emu(v));
    }
    w.writeEndElement();

    // The cell a covered one is under, to tell merged across from merged down.
    std::vector<QPoint> origin(size_t(t.rows) * t.cols, QPoint(-1, -1));
    for (int r = 0; r < t.rows; ++r)
        for (int c = 0; c < t.cols; ++c) {
            const TableCell &cell = t.cell(r, c);
            if (cell.covered) continue;
            for (int rr = r; rr < std::min(t.rows, r + cell.rowSpan); ++rr)
                for (int cc = c; cc < std::min(t.cols, c + cell.colSpan); ++cc) origin[size_t(rr) * t.cols + cc] = QPoint(c, r);
        }

    for (int r = 0; r < t.rows; ++r) {
        w.writeStartElement(QStringLiteral("a:tr"));
        w.writeAttribute(QStringLiteral("h"), emu(t.rowH[r]));
        for (int c = 0; c < t.cols; ++c) {
            const TableCell &cell = t.cell(r, c);
            w.writeStartElement(QStringLiteral("a:tc"));
            if (!cell.covered) {
                if (cell.colSpan > 1) w.writeAttribute(QStringLiteral("gridSpan"), QString::number(cell.colSpan));
                if (cell.rowSpan > 1) w.writeAttribute(QStringLiteral("rowSpan"), QString::number(cell.rowSpan));
            } else {
                const QPoint from = origin[size_t(r) * t.cols + c];
                if (from.x() < c) w.writeAttribute(QStringLiteral("hMerge"), QStringLiteral("1"));
                if (from.y() < r) w.writeAttribute(QStringLiteral("vMerge"), QStringLiteral("1"));
            }
            w.writeStartElement(QStringLiteral("a:txBody"));
            w.writeEmptyElement(QStringLiteral("a:bodyPr"));
            w.writeEmptyElement(QStringLiteral("a:lstStyle"));
            const QTextDocument *doc = ctx.storyDoc && !cell.storyId.isEmpty() ? ctx.storyDoc(cell.storyId) : nullptr;
            if (doc) writeParagraphs(w, doc, ctx);
            else w.writeEmptyElement(QStringLiteral("a:p"));
            w.writeEndElement();
            w.writeStartElement(QStringLiteral("a:tcPr"));
            if (!cell.covered) {
                w.writeAttribute(QStringLiteral("marL"), emu(cell.margins.left()));
                w.writeAttribute(QStringLiteral("marR"), emu(cell.margins.right()));
                w.writeAttribute(QStringLiteral("marT"), emu(cell.margins.top()));
                w.writeAttribute(QStringLiteral("marB"), emu(cell.margins.bottom()));
                if (cell.vertical) w.writeAttribute(QStringLiteral("vert"), QStringLiteral("vert"));
                if (cell.valign != VAlign::Top) w.writeAttribute(QStringLiteral("anchor"), cell.valign == VAlign::Middle ? QStringLiteral("ctr") : QStringLiteral("b"));
                writeCellLine(w, QStringLiteral("a:lnL"), cell.border.left, ctx);
                writeCellLine(w, QStringLiteral("a:lnR"), cell.border.right, ctx);
                writeCellLine(w, QStringLiteral("a:lnT"), cell.border.top, ctx);
                writeCellLine(w, QStringLiteral("a:lnB"), cell.border.bottom, ctx);
                if (cell.diagonal == 1) writeCellLine(w, QStringLiteral("a:lnTlToBr"), cell.diagonalLine, ctx);
                if (cell.diagonal == 2) writeCellLine(w, QStringLiteral("a:lnBlToTr"), cell.diagonalLine, ctx);
                writeCellFill(w, cell.fill, ctx);
            }
            w.writeEndElement();
            w.writeEndElement();
        }
        w.writeEndElement();
    }
    w.writeEndElement();   // a:tbl
    w.writeEndElement();   // a:graphicData
    w.writeEndElement();   // a:graphic
    w.writeEndElement();   // p:graphicFrame
}

} // namespace jp::office
