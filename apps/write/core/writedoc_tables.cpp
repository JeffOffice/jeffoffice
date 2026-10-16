// Tables between the file and the story: the records of cells made from the blocks of a table (TableBuilder), and the blocks made again from the story's
// paragraphs and their cells' records (TableExporter, used by WriteDoc::exportBody).

#include "core/tablemodel.h"

#include "core/writedoc.h"

#include <QColor>
#include <algorithm>
#include <cmath>

using namespace jp::office::wml;

namespace jp::write {

namespace {

bool tagIs(const Block &b, const char *tag) { return b.tag == QLatin1String(tag); }

QColor colorOfHex(const QString &hex)
{
    if (hex.size() != 6) return QColor();
    bool ok = false;
    const uint v = hex.toUInt(&ok, 16);
    return ok ? QColor((v >> 16) & 0xFF, (v >> 8) & 0xFF, v & 0xFF) : QColor();
}

// A border as the file states it: none when the file's says none (a line of no style), nothing when the file says nothing.
std::optional<flow::TableLine> lineOf(const TableBorder &b)
{
    if (!b.stated()) return std::nullopt;
    flow::TableLine line;
    if (b.val == QLatin1String("nil") || b.val == QLatin1String("none")) return line;
    line.width = std::max(0.25, std::min(12.0, b.size / 8.0));
    line.color = b.color == QLatin1String("auto") ? QColor() : colorOfHex(b.color);
    if (b.val == QLatin1String("double") || b.val == QLatin1String("triple") || b.val.startsWith(QLatin1String("thinThick")) || b.val.startsWith(QLatin1String("thickThin")))
        line.style = flow::TableLine::Style::Double;
    else if (b.val == QLatin1String("dotted"))
        line.style = flow::TableLine::Style::Dotted;
    else if (b.val.contains(QLatin1String("dash"), Qt::CaseInsensitive) || b.val.startsWith(QLatin1String("dot")))
        line.style = flow::TableLine::Style::Dashed;
    else
        line.style = flow::TableLine::Style::Single;
    return line;
}

// The shading a cell or table says: the color the cell is filled with (none for no shading).
std::optional<QColor> fillOf(const TableMark &m)
{
    if (m.shdVal.isEmpty() && m.shdFill.isEmpty()) return std::nullopt;
    const QColor back = m.shdFill.isEmpty() || m.shdFill == QLatin1String("auto") ? QColor() : colorOfHex(m.shdFill);
    const QColor front = m.shdColor.isEmpty() || m.shdColor == QLatin1String("auto") ? QColor(Qt::black) : colorOfHex(m.shdColor);
    const QString v = m.shdVal;
    if (v == QLatin1String("solid")) return front.isValid() ? front : QColor(Qt::black);
    if (v.startsWith(QLatin1String("pct")) && v.size() > 3) {
        const double share = v.mid(3).toDouble() / 100.0;
        const QColor under = back.isValid() ? back : QColor(Qt::white);
        auto mix = [&](int a, int b) { return int(std::lround(a * (1 - share) + b * share)); };
        return QColor(mix(under.red(), front.red()), mix(under.green(), front.green()), mix(under.blue(), front.blue()));
    }
    if (back.isValid()) return back;
    return QColor(0, 0, 0, 0);   // (no fill: said, so the table's is not shown through)
}

double points(const TableWidth &w) { return w.type == QLatin1String("dxa") || w.type.isEmpty() ? w.w / 20.0 : 0.0; }

} // namespace

TableBuilder::TableBuilder(flow::FlowDocument *flow, const StyleSheet *sheet, int compatMode, const QVector<Block> &body) : m_flow(flow), m_sheet(sheet), m_compat(compatMode)
{
    // The rows of each table, counted before the tables come.
    QVector<int> open;
    for (const Block &b : body) {
        if (!b.isRaw || b.role == Inline::Single) continue;
        if (tagIs(b, "tbl")) {
            if (b.role == Inline::Open) {
                open.push_back(int(m_rowCounts.size()));
                m_rowCounts.push_back(0);
            } else if (!open.isEmpty()) {
                open.removeLast();
            }
        } else if (tagIs(b, "tr") && b.role == Inline::Open && !open.isEmpty()) {
            ++m_rowCounts[open.last()];
        }
    }
}

bool TableBuilder::isStructural(const Block &blk) const
{
    if (!blk.isRaw) return false;
    if ((tagIs(blk, "tbl") || tagIs(blk, "tr") || tagIs(blk, "tc")) && blk.role != Inline::Single) return true;
    // (What a file keeps between a table's rows or a row's cells.)
    if (!m_levels.isEmpty() && m_levels.last().kind != Level::Cell && blk.display.isEmpty()) return true;
    return false;
}

const TableLayer *TableBuilder::layer() const
{
    for (int i = int(m_levels.size()) - 1; i >= 0; --i)
        if (m_levels[i].kind == Level::Cell) return m_levels[i].cellPart->layer.get();
    return nullptr;
}

void TableBuilder::addTail(const QVector<Block> &pieces)
{
    for (int i = int(m_levels.size()) - 1; i >= 0; --i)
        if (m_levels[i].kind == Level::Cell) {
            m_levels[i].cellPart->tail += pieces;
            return;
        }
}

namespace {

// The parts of a table's style that apply to the cell in row `row` of `rows` and column `col` of `columns`, from the one that gives way to the rest to the one that wins.
QStringList stylePartsFor(const TableStyleDef &def, int look, int row, int rows, int col, int columns, int rowBand, int colBand)
{
    QStringList out;
    const bool firstRow = look & 0x20, lastRow = look & 0x40, firstCol = look & 0x80, lastCol = look & 0x100, noHBand = look & 0x200, noVBand = look & 0x400;
    const bool isFirstRow = firstRow && row == 0, isLastRow = lastRow && row == rows - 1, isFirstCol = firstCol && col == 0, isLastCol = lastCol && col == columns - 1;
    // (The bands count from the rows and columns that the first and the last ones leave: a band of 2 is two rows.)
    if (!noVBand && !isFirstCol && !isLastCol) out << ((col - (firstCol ? 1 : 0)) / std::max(1, colBand) % 2 == 0 ? QStringLiteral("band1Vert") : QStringLiteral("band2Vert"));
    if (!noHBand && !isFirstRow && !isLastRow) out << ((row - (firstRow ? 1 : 0)) / std::max(1, rowBand) % 2 == 0 ? QStringLiteral("band1Horz") : QStringLiteral("band2Horz"));
    if (isLastCol) out << QStringLiteral("lastCol");
    if (isFirstCol) out << QStringLiteral("firstCol");
    if (isLastRow) out << QStringLiteral("lastRow");
    if (isFirstRow) out << QStringLiteral("firstRow");
    if (isFirstRow && isFirstCol) out << QStringLiteral("nwCell");
    if (isFirstRow && isLastCol) out << QStringLiteral("neCell");
    if (isLastRow && isFirstCol) out << QStringLiteral("swCell");
    if (isLastRow && isLastCol) out << QStringLiteral("seCell");
    QStringList present;
    for (const QString &p : std::as_const(out))
        if (def.parts.contains(p)) present << p;
    return present;
}

} // namespace

void TableBuilder::take(const Block &blk, QVector<Block> *pending)
{
    const bool open = blk.role == Inline::Open;
    if (tagIs(blk, "tbl") && blk.role != Inline::Single) {
        if (open) {
            Level l;
            l.kind = Level::Table;
            const TableMark &direct = *blk.table;
            // The table as its style makes it, and then as it says itself.
            l.styleId = direct.style.isEmpty() ? m_sheet->defaultId(QStringLiteral("table")) : direct.style;
            l.style = &m_sheet->tableStyle(l.styleId);
            TableMark m = l.style->base.tbl;
            overlayTableMark(&m, direct);
            l.look = direct.look;
            l.rowBand = l.style->rowBand.value_or(1);
            l.colBand = l.style->colBand.value_or(1);
            l.rowCount = m_tables < m_rowCounts.size() ? m_rowCounts[m_tables] : 0;
            ++m_tables;
            l.columns = int(direct.grid.size());
            l.table = std::make_shared<flow::TableData>();
            flow::TableData &t = *l.table;
            for (int w : direct.grid) t.columns << w / 20.0;
            // Margins inside a cell, unless a cell says: the table's own, else its style's, else the usual (a little at the sides).
            t.margins = QMarginsF(m.marLeft ? points(*m.marLeft) : 5.4, m.marTop ? points(*m.marTop) : 0, m.marRight ? points(*m.marRight) : 5.4, m.marBottom ? points(*m.marBottom) : 0);
            t.top = lineOf(m.top).value_or(flow::TableLine());
            t.left = lineOf(m.left).value_or(flow::TableLine());
            t.bottom = lineOf(m.bottom).value_or(flow::TableLine());
            t.right = lineOf(m.right).value_or(flow::TableLine());
            t.insideH = lineOf(m.insideH).value_or(flow::TableLine());
            t.insideV = lineOf(m.insideV).value_or(flow::TableLine());
            t.fill = fillOf(m);
            t.spacing = points(m.spacing);
            const QString jc = m.jc;
            t.align = jc == QLatin1String("center") ? flow::TableData::Align::Center : jc == QLatin1String("right") || jc == QLatin1String("end") ? flow::TableData::Align::Right : flow::TableData::Align::Left;
            // (Before the 2013 program, the indent is to the first cell's text, and the table's edge is left of it by the cell's margin.)
            t.indent = points(m.indent);
            t.indentToText = m_compat < 15;
            l.tablePart = std::make_shared<TablePart>();
            l.tablePart->open = blk.raw;
            l.tablePart->mark = blk.table;
            l.tablePart->original = direct.original;
            l.tablePart->fingerprint = direct.fingerprint;
            l.tablePart->before = *pending;
            pending->clear();
            t.app = l.tablePart;
            m_levels.push_back(l);
        } else {
            // The table ends: what was kept after its last row goes with it, and a grid the file left out is what its first row's cells say.
            for (int i = int(m_levels.size()) - 1; i >= 0; --i)
                if (m_levels[i].kind == Level::Table) {
                    Level &l = m_levels[i];
                    l.tablePart->after = m_between;
                    if (l.table->columns.isEmpty()) l.table->columns = l.firstRow;
                    m_levels.resize(i);
                    break;
                }
            m_between.clear();
        }
        return;
    }
    if (tagIs(blk, "tr") && blk.role != Inline::Single) {
        if (open) {
            Level l;
            l.kind = Level::Row;
            const TableMark &m = *blk.table;
            l.row = std::make_shared<flow::RowData>();
            l.row->height = m.height / 20.0;
            l.row->exact = m.heightRule == QLatin1String("exact");
            l.row->cantSplit = m.cantSplit;
            l.row->header = m.header;
            l.row->before = m.gridBefore;
            l.row->after = m.gridAfter;
            l.col = m.gridBefore;
            l.rowPart = std::make_shared<RowPart>();
            l.rowPart->open = blk.raw;
            l.rowPart->before = m_between;
            m_between.clear();
            l.row->app = l.rowPart;
            for (int i = int(m_levels.size()) - 1; i >= 0; --i)
                if (m_levels[i].kind == Level::Table) {
                    ++m_levels[i].rows;
                    break;
                }
            m_levels.push_back(l);
        } else {
            for (int i = int(m_levels.size()) - 1; i >= 0; --i)
                if (m_levels[i].kind == Level::Row) {
                    m_levels[i].rowPart->after = m_between;
                    m_levels.resize(i);
                    break;
                }
            m_between.clear();
        }
        return;
    }
    if (tagIs(blk, "tc") && blk.role != Inline::Single) {
        if (open) {
            Level *table = nullptr, *row = nullptr;
            for (int i = int(m_levels.size()) - 1; i >= 0; --i) {
                if (m_levels[i].kind == Level::Row && !row) row = &m_levels[i];
                if (m_levels[i].kind == Level::Table) {
                    table = &m_levels[i];
                    break;
                }
            }
            Level cell;
            cell.kind = Level::Cell;
            cell.cellPart = std::make_shared<CellPart>();
            cell.cellPart->open = blk.raw;
            cell.cellPart->before = m_between;
            m_between.clear();
            flow::CellRecord rec;
            rec.parent = key();
            QVector<double> firstRow;
            if (table && row) {
                const TableMark &direct = *blk.table;
                rec.table = table->table;
                rec.row = row->row;
                const int rowIndex = table->rows - 1, col = row->col;
                row->col += std::max(1, direct.span);
                // The parts of the table's style that apply to this cell, and the cell as they and it say.
                const QStringList parts = stylePartsFor(*table->style, table->look, rowIndex, table->rowCount, col, std::max(table->columns, row->col), table->rowBand, table->colBand);
                TableMark m = table->style->base.cell;
                for (const QString &p : parts) overlayTableMark(&m, table->style->parts[p].cell);
                overlayTableMark(&m, direct);
                const QString layerKey = table->styleId + QLatin1Char('|') + parts.join(QLatin1Char(','));
                auto layer = m_layers.value(layerKey);
                if (!layer) {
                    layer = std::make_shared<TableLayer>();
                    layer->key = layerKey;
                    if (table->style->base.pPr) layer->p.overlay(*table->style->base.pPr);
                    if (table->style->base.rPr) layer->r.overlay(*table->style->base.rPr);
                    for (const QString &p : parts) {
                        const TableStylePart &sp = table->style->parts[p];
                        if (sp.pPr) layer->p.overlay(*sp.pPr);
                        if (sp.rPr) layer->r.overlay(*sp.rPr);
                    }
                    m_layers.insert(layerKey, layer);
                }
                cell.cellPart->layer = layer;
                flow::CellData &c = rec.cell;
                c.span = std::max(1, direct.span);
                c.vmerge = direct.vMerge == QLatin1String("restart") ? flow::CellData::VMerge::Restart : direct.vMerge == QLatin1String("continue") ? flow::CellData::VMerge::Continue : flow::CellData::VMerge::None;
                if (m.cMarTop || m.cMarLeft || m.cMarBottom || m.cMarRight) {
                    const QMarginsF d = rec.table->margins;
                    c.margins = QMarginsF(m.cMarLeft ? points(*m.cMarLeft) : d.left(), m.cMarTop ? points(*m.cMarTop) : d.top(), m.cMarRight ? points(*m.cMarRight) : d.right(),
                                          m.cMarBottom ? points(*m.cMarBottom) : d.bottom());
                }
                c.top = lineOf(m.top);
                c.left = lineOf(m.left);
                c.bottom = lineOf(m.bottom);
                c.right = lineOf(m.right);
                c.fill = fillOf(m);
                c.valign = m.vAlign == QLatin1String("center") ? flow::CellAlign::Center : m.vAlign == QLatin1String("bottom") ? flow::CellAlign::Bottom : flow::CellAlign::Top;
                c.noWrap = m.noWrap;
                c.app = cell.cellPart;
                // The widths of the first row's cells, for a table whose grid the file left out.
                if (table->rows == 1) {
                    const double w = direct.cellWidth.type == QLatin1String("dxa") ? direct.cellWidth.w / 20.0 : 0;
                    for (int k = 0; k < c.span; ++k) firstRow << (w > 0 ? w / c.span : 0.0);
                }
            }
            if (table) table->firstRow += firstRow;
            const int key = m_flow->tables.add(rec);
            m_cells.push_back(key);
            m_levels.push_back(cell);
        } else {
            for (int i = int(m_levels.size()) - 1; i >= 0; --i)
                if (m_levels[i].kind == Level::Cell) {
                    m_levels.resize(i);
                    break;
                }
            if (!m_cells.isEmpty()) m_cells.removeLast();
            m_between.clear();
        }
        return;
    }
    // A kept piece between the parts of a table.
    m_between.push_back(blk);
}

// ---- the other way

namespace {

QVector<const flow::CellRecord *> chainOf(const flow::TableStore &store, int key)
{
    QVector<const flow::CellRecord *> up;
    for (const flow::CellRecord *r = store.get(key); r && up.size() < 64; r = store.get(r->parent)) up.push_back(r);
    std::reverse(up.begin(), up.end());
    return up;
}

Block structureBlock(const char *tag, Inline::Role role, const QByteArray &raw)
{
    Block b;
    b.isRaw = true;
    b.role = role;
    b.tag = QLatin1String(tag);
    b.raw = raw;
    return b;
}

} // namespace

void TableExporter::moveTo(int key, QVector<Block> *out)
{
    const QVector<const flow::CellRecord *> now = key ? chainOf(m_flow->tables, key) : QVector<const flow::CellRecord *>();
    // The first level where the cell is another one.
    int same = 0;
    while (same < m_open.size() && same < now.size() && m_open[same] == now[same]) ++same;
    // Closes what is open from the deepest level up to that one.
    for (int i = int(m_open.size()) - 1; i >= same; --i) {
        const flow::CellRecord *c = m_open[i];
        const CellPart *cp = static_cast<const CellPart *>(c->cell.app.get());
        if (cp) *out += cp->tail;
        out->push_back(structureBlock("tc", Inline::Close, "</w:tc>"));
        // The row ends if the cell that comes next at this level is in another row (or there is none); and the table with it, if that is in another table.
        const flow::CellRecord *next = i < now.size() ? now[i] : nullptr;
        const bool rowEnds = !next || next->row != c->row || i > same;
        const bool tableEnds = !next || next->table != c->table || i > same;
        if (rowEnds) {
            const RowPart *rp = static_cast<const RowPart *>(c->row->app.get());
            if (rp) *out += rp->after;
            out->push_back(structureBlock("tr", Inline::Close, "</w:tr>"));
        }
        if (tableEnds) {
            const TablePart *tp = static_cast<const TablePart *>(c->table->app.get());
            if (tp) *out += tp->after;
            out->push_back(structureBlock("tbl", Inline::Close, "</w:tbl>"));
        }
    }
    // Opens what is new, from the level where it differs down to the paragraph's own cell.
    for (int i = same; i < now.size(); ++i) {
        const flow::CellRecord *c = now[i];
        const flow::CellRecord *was = i < m_open.size() ? m_open[i] : nullptr;
        // (Levels below the first that changed are all new; at the first, the table or the row may be the one that was already open.)
        const bool newTable = !was || was->table != c->table || i > same;
        const bool newRow = newTable || !was || was->row != c->row || i > same;
        if (newTable) {
            const TablePart *tp = static_cast<const TablePart *>(c->table->app.get());
            if (tp) *out += tp->before;
            Block b = structureBlock("tbl", Inline::Open, tp ? tp->open : QByteArray("<w:tbl>"));
            if (tp && tp->mark) {
                auto mark = std::make_shared<TableMark>(*tp->mark);
                mark->original = tp->original;
                mark->fingerprint = tp->fingerprint;
                b.table = mark;
            }
            m_opened.push_back({out->size(), const_cast<TablePart *>(tp)});
            out->push_back(b);
        }
        if (newRow) {
            const RowPart *rp = static_cast<const RowPart *>(c->row->app.get());
            if (rp) *out += rp->before;
            out->push_back(structureBlock("tr", Inline::Open, rp ? rp->open : QByteArray("<w:tr>")));
        }
        const CellPart *cp = static_cast<const CellPart *>(c->cell.app.get());
        if (cp) *out += cp->before;
        out->push_back(structureBlock("tc", Inline::Open, cp ? cp->open : QByteArray("<w:tc>")));
    }
    m_open = now;
}

// What a table written from the story comes to as the file writer would write it, when the document has just been opened: a table is written as it was
// read for as long as its blocks come to that again (nothing in it changed).
void WriteDoc::settleTables()
{
    if (m_flow->tables.isEmpty()) return;
    QVector<Block> out;
    QVector<TableExporter::Opened> opened;
    exportBody(&out, &opened);
    for (const TableExporter::Opened &o : std::as_const(opened)) {
        qsizetype depth = 0, end = o.at;
        for (; end < out.size(); ++end) {
            const Block &x = out[end];
            if (!x.isRaw || x.tag != QLatin1String("tbl")) continue;
            depth += x.role == Inline::Open ? 1 : x.role == Inline::Close ? -1 : 0;
            if (depth == 0) break;
        }
        if (end < out.size() && o.part) o.part->fingerprint = blocksFingerprint(out, o.at, end + 1);
    }
}

const TableLayer *WriteDoc::tableLayerOf(int cellKey) const
{
    const flow::CellRecord *rec = m_flow->tables.get(cellKey);
    const CellPart *part = rec ? static_cast<const CellPart *>(rec->cell.app.get()) : nullptr;
    return part ? part->layer.get() : nullptr;
}

} // namespace jp::write
