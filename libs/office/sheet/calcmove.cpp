// A block of cells put in another place (cut and paste): the cells go over what is there, and every formula that reads them is written again to read them where they are. See
// Calculation::applyEdit.
#include "sheet/calculation.h"

#include <QCoreApplication>

#include <algorithm>

namespace jp::office::sheet {

using jp::calc::CellArea;
using jp::calc::kGridCols;
using jp::calc::kGridRows;
using jp::calc::Value;

namespace {

QString say(const char *text) { return QCoreApplication::translate("jp::office::sheet::Calculation", text); }

bool meets(const CellRange &r, quint32 row1, quint32 col1, quint32 row2, quint32 col2) { return r.valid && r.row2 >= row1 && r.row1 <= row2 && r.col2 >= col1 && r.col1 <= col2; }

} // namespace

bool Calculation::canMoveCells(const StructEdit &e, QString *why) const
{
    auto refuse = [&](const QString &text) {
        if (why) *why = text;
        return false;
    };
    if (e.sheet < 0 || e.sheet >= int(m_sheets.size()) || e.toSheet < 0 || e.toSheet >= int(m_sheets.size())) return refuse(say(QT_TRANSLATE_NOOP("jp::office::sheet::Calculation", "There is no such sheet.")));
    if (e.row2 < e.row1 || e.col2 < e.col1 || e.row2 >= kGridRows || e.col2 >= kGridCols) return refuse(say(QT_TRANSLATE_NOOP("jp::office::sheet::Calculation", "There is nothing to move.")));
    const quint64 rows = quint64(e.row2) - e.row1 + 1, cols = quint64(e.col2) - e.col1 + 1;
    if (quint64(e.toRow) + rows > kGridRows || quint64(e.toCol) + cols > kGridCols) return refuse(say(QT_TRANSLATE_NOOP("jp::office::sheet::Calculation", "That would push cells off the edge of the sheet.")));
    if (e.sheet == e.toSheet && e.toRow == e.row1 && e.toCol == e.col1) return refuse(say(QT_TRANSLATE_NOOP("jp::office::sheet::Calculation", "The cells are there already.")));
    if (m_untracked > 0) return refuse(say(QT_TRANSLATE_NOOP("jp::office::sheet::Calculation", "This workbook has too many formulas for JeffCrunch to follow when cells are moved.")));
    const Sheet &from = m_sheets[size_t(e.sheet)], &to = m_sheets[size_t(e.toSheet)];
    if (from.isProtected || to.isProtected) return refuse(say(QT_TRANSLATE_NOOP("jp::office::sheet::Calculation", "This sheet is protected, so cells can't be moved on it. To do that, choose Unprotect Sheet on the Review tab.")));
    const quint32 toRow2 = e.toRow + quint32(rows) - 1, toCol2 = e.toCol + quint32(cols) - 1;
    const QString array = say(QT_TRANSLATE_NOOP("jp::office::sheet::Calculation", "Part of an array formula can't be moved or written over. Select the whole array, or none of it."));
    const QString dataTable = say(QT_TRANSLATE_NOOP("jp::office::sheet::Calculation", "A data table can't be moved or written over."));
    auto insideBlock = [&](const CellRange &r) { return r.row1 >= e.row1 && r.row2 <= e.row2 && r.col1 >= e.col1 && r.col2 <= e.col2; };
    for (const CellRange &r : from.ranges) {
        if (!meets(r, e.row1, e.col1, e.row2, e.col2)) continue;
        // an array goes whole (an array of one cell too); part of one does not, and a data table does not go at all
        const Cell *first = from.cells.find(r.row1, r.col1);
        if (first && (first->flags & DataTable)) return refuse(dataTable);
        if (!insideBlock(r) || !first || !(first->flags & ArrayFormula)) return refuse(array);
    }
    for (const CellRange &r : to.ranges) {
        if (r.cellCount() <= 1 || !meets(r, e.toRow, e.toCol, toRow2, toCol2)) continue;
        if (e.sheet == e.toSheet && insideBlock(r)) continue;   // (it goes with the block: it is not written over)
        const Cell *first = to.cells.find(r.row1, r.col1);
        return refuse(first && (first->flags & DataTable) ? dataTable : array);
    }
    // merged cells go whole or not at all, and are not written over in part
    const QString merged = say(QT_TRANSLATE_NOOP("jp::office::sheet::Calculation", "Part of a merged cell can't be moved or written over. Select the whole merged cell, or none of it."));
    for (const CellRange &r : from.merges)
        if (meets(r, e.row1, e.col1, e.row2, e.col2) && !(r.row1 >= e.row1 && r.row2 <= e.row2 && r.col1 >= e.col1 && r.col2 <= e.col2)) return refuse(merged);
    for (const CellRange &r : to.merges)
        if (meets(r, e.toRow, e.toCol, toRow2, toCol2) && !(r.row1 >= e.toRow && r.row2 <= toRow2 && r.col1 >= e.toCol && r.col2 <= toCol2)) return refuse(merged);
    return true;
}

void Calculation::dissolveArray(int sheet, qint32 slot)
{
    Sheet &s = m_sheets[size_t(sheet)];
    if (slot < 0 || slot >= s.ranges.size()) return;
    const CellRange r = s.ranges[slot];
    s.ranges[slot].valid = false;
    if (!r.valid) return;
    // (an array larger than the cells that were made members of has its first cell only)
    const quint32 lastRow = r.cellCount() > kMaxArrayCellsMarked ? r.row1 : r.row2, lastCol = r.cellCount() > kMaxArrayCellsMarked ? r.col1 : r.col2;
    for (quint32 row = r.row1; row <= lastRow; ++row)
        for (quint32 col = r.col1; col <= lastCol; ++col) {
            Cell *c = s.cells.findMutable(row, col);
            if (!c) continue;
            if (c->flags & ArrayMember) {
                c->flags &= quint8(~ArrayMember);
                c->formula = -1;
            } else if (c->flags & ArrayFormula) {
                c->flags &= quint8(~ArrayFormula);
                if (c->formula >= 0 && c->formula < s.formulas.size()) s.formulas[c->formula].range = -1;
            }
        }
}

qint32 Calculation::makeArray(int sheet, quint32 row, quint32 col, const CellRange &range, qint32 slot, bool *added)
{
    Sheet &s = m_sheets[size_t(sheet)];
    Cell *c = s.cells.findMutable(row, col);
    if (!c || !c->hasFormula() || c->formula < 0 || c->formula >= s.formulas.size()) return -1;
    CellRange placed = range;
    placed.valid = true;
    *added = false;
    if (slot >= 0 && slot < s.ranges.size() && !s.ranges[slot].valid) {
        s.ranges[slot] = placed;
    } else {
        slot = qint32(s.ranges.size());
        s.ranges.append(placed);
        *added = true;
    }
    c->flags |= ArrayFormula;
    s.formulas[c->formula].range = slot;
    const quint32 id = nodeAt(sheet, row, col);
    if (id != kNone) dropNode(id);
    const quint32 now = addNode(sheet, row, col, *c, c->formula);
    queueDependents(areaOf(m_nodes[now]));
    markArrayMembers(sheet, row, col);
    return slot;
}

// The cells of an array's range, but its first, are its: they read its value (those of an array kept apart are marked, as the calculation's loading does).
void Calculation::markArrayMembers(int sheet, quint32 row, quint32 col)
{
    Sheet &s = m_sheets[size_t(sheet)];
    const Cell *first = s.cells.find(row, col);
    if (!first || !first->hasFormula() || first->formula < 0 || first->formula >= s.formulas.size()) return;
    const qint32 slot = s.formulas[first->formula].range;
    if (slot < 0 || slot >= s.ranges.size() || !s.ranges[slot].valid) return;
    const CellRange r = s.ranges[slot];
    if (r.cellCount() <= 1 || r.cellCount() > kMaxArrayCellsMarked) return;
    const qint32 formula = first->formula;
    const quint32 id = nodeAt(sheet, row, col);
    const bool kept = id != kNone && (m_nodes[id].flags & Kept);
    for (quint32 rr = r.row1; rr <= r.row2; ++rr)
        for (quint32 cc = r.col1; cc <= r.col2; ++cc) {
            if (rr == row && cc == col) continue;
            Cell member;
            if (const Cell *have = s.cells.find(rr, cc)) {
                member = *have;
            }
            if (member.hasFormula()) continue;
            if (kept) {
                member.flags |= Tainted;
            } else {
                member.flags |= ArrayMember;
                member.formula = formula;
            }
            s.cells.set(rr, cc, member);
            m_rows[size_t(sheet)] = std::max(m_rows[size_t(sheet)], rr + 1);
            m_cols[size_t(sheet)] = std::max(m_cols[size_t(sheet)], cc + 1);
        }
}

bool Calculation::applyMove(const StructEdit &e, EditRecord *rec)
{
    abandonNow();
    BulkEdit bulk(this);
    ++m_state[size_t(e.sheet)].epoch;
    if (e.toSheet != e.sheet) ++m_state[size_t(e.toSheet)].epoch;
    *rec = EditRecord();
    rec->edit = e;
    rec->sheet = e.sheet;
    Sheet &S = m_sheets[size_t(e.sheet)];
    Sheet &D = m_sheets[size_t(e.toSheet)];
    rec->merges = S.merges;
    rec->mergeCount = S.mergeCount;
    if (e.toSheet != e.sheet) {
        rec->toMerges = D.merges;
        rec->toMergeCount = D.mergeCount;
    }
    QStringList names;
    for (const SheetEntry &se : m_book.info().sheets) names << se.name;
    const quint32 rows = e.row2 - e.row1 + 1, cols = e.col2 - e.col1 + 1;
    const qint64 dRow = qint64(e.toRow) - e.row1, dCol = qint64(e.toCol) - e.col1;
    const quint32 toRow2 = e.toRow + rows - 1, toCol2 = e.toCol + cols - 1;
    auto inSource = [&](int sheet, quint32 row, quint32 col) { return sheet == e.sheet && row >= e.row1 && row <= e.row2 && col >= e.col1 && col <= e.col2; };
    auto inDestination = [&](int sheet, quint32 row, quint32 col) { return sheet == e.toSheet && row >= e.toRow && row <= toRow2 && col >= e.toCol && col <= toCol2; };

    // ---- what reads the cells that move (the graph says, before anything moves), and the formulas the graph does not follow
    CellArea area;
    area.sheet1 = area.sheet2 = e.sheet;
    area.row1 = e.row1;
    area.row2 = e.row2;
    area.col1 = e.col1;
    area.col2 = e.col2;
    std::vector<quint32> candidates;
    readersOf(area, &candidates);
    {
        // (and those that read the cells that are written over: a reference to them has no cell to name)
        CellArea written = area;
        written.sheet1 = written.sheet2 = e.toSheet;
        written.row1 = e.toRow;
        written.row2 = e.toRow + rows - 1;
        written.col1 = e.toCol;
        written.col2 = e.toCol + cols - 1;
        readersOf(written, &candidates);
    }
    specialNodes(&candidates);
    std::sort(candidates.begin(), candidates.end());
    candidates.erase(std::unique(candidates.begin(), candidates.end()), candidates.end());
    const qint64 graphWorkBefore = m_graph->buildWork() + m_graph->queryWork();
    ++m_editStats.edits;
    m_editStats.formulasVisited += qint64(candidates.size());

    // ---- the cells of the block, with the formulas among them written to read where the cells will be
    struct Moving {
        quint32 row, col;
        CellContent content;
    };
    std::vector<Moving> moving;
    std::vector<std::pair<quint32, quint32>> overwritten;
    S.cells.forEachInRange(e.row1, e.col1, e.row2, e.col2, [&](quint32 row, quint32 col, const Cell &c) {
        Moving m;
        m.row = row;
        m.col = col;
        m.content = content(e.sheet, row, col);
        rec->gone.push_back(goneCell(e.sheet, row, col, c));
        if (m.content.isFormula) {
            bool changed = false;
            const QString now = adjustMovedFormula(m.content.formula, names, e, &changed);
            if (changed) m.content.formula = now;
        }
        moving.push_back(std::move(m));
        return true;
    });
    D.cells.forEachInRange(e.toRow, e.toCol, toRow2, toCol2, [&](quint32 row, quint32 col, const Cell &c) {
        if (inSource(e.toSheet, row, col)) return true;   // (already counted: it is one of the block's)
        rec->gone.push_back(goneCell(e.toSheet, row, col, c));
        overwritten.push_back({row, col});
        return true;
    });
    rec->cellsMoved = qint64(moving.size());
    m_editStats.cellsMoved += rec->cellsMoved;
    m_editStats.cellsGone += qint64(rec->gone.size());

    // ---- the formulas outside the block that read it are written again (before the cells move, while the graph still knows who reads what)
    struct Rewrite {
        quint32 id;
        QString was, now;
    };
    std::vector<Rewrite> rewrites;
    for (const quint32 id : candidates) {
        const Node &n = m_nodes[id];
        if (n.flags & Dead) continue;
        if (inSource(n.sheet, rowOf(n), colOf(n)) || inDestination(n.sheet, rowOf(n), colOf(n))) continue;
        const Sheet &sn = m_sheets[size_t(n.sheet)];
        const Cell *c = cellOf(n);
        if (!c || !c->hasFormula()) continue;
        Rewrite r;
        r.id = id;
        r.was = sn.formulaText(*c);
        bool changed = false;
        r.now = adjustFormula(r.was, n.sheet, names, e, &changed);
        if (!changed) r.now.clear();
        else ++m_editStats.textsMade;
        rewrites.push_back(std::move(r));
    }

    // ---- arrays: those whole in the block go with it (their cells are made ordinary here, and an array again where they land); an array of one cell that is written over is replaced
    struct ArrayGoing {
        quint32 row, col;   // the first cell, where it lands
        CellRange range;    // and the range
    };
    std::vector<ArrayGoing> arrays;
    std::vector<std::pair<int, qint32>> freeSlots;   // (the places in the lists of ranges that taken arrays left)
    for (qint32 i = 0; i < S.ranges.size(); ++i) {
        const CellRange r = S.ranges[i];
        if (!r.valid || !meets(r, e.row1, e.col1, e.row2, e.col2)) continue;
        ArrayGoing going;
        going.row = quint32(qint64(r.row1) + dRow);
        going.col = quint32(qint64(r.col1) + dCol);
        going.range = r;
        going.range.row1 = going.row;
        going.range.col1 = going.col;
        going.range.row2 = quint32(qint64(r.row2) + dRow);
        going.range.col2 = quint32(qint64(r.col2) + dCol);
        arrays.push_back(going);
        rec->arraysTaken.push_back({e.sheet, i, r, false});
        freeSlots.push_back({e.sheet, i});
        dissolveArray(e.sheet, i);
    }
    for (qint32 i = 0; i < D.ranges.size(); ++i) {
        const CellRange r = D.ranges[i];
        if (!r.valid || !meets(r, e.toRow, e.toCol, toRow2, toCol2)) continue;
        rec->arraysTaken.push_back({e.toSheet, i, r, false});
        freeSlots.push_back({e.toSheet, i});
        dissolveArray(e.toSheet, i);
    }

    // ---- both places are emptied, and the cells put where they go
    auto clear = [&](int sheet, quint32 row, quint32 col) {
        setValue(sheet, row, col, Value(), Marking::Later);
        setStyle(sheet, row, col, 0);
    };
    // (a value told for a cell these places held, before the edit, is not for what they hold now: the cells are stamped, at both places)
    auto stampPlace = [&](int sheet, quint32 row, quint32 col) {
        const CellStore &cells = m_sheets[size_t(sheet)].cells;
        const quint32 rowId = cells.rowIdAt(row), colId = cells.colIdAt(col);
        if (rowId != CellStore::kNoLine && colId != CellStore::kNoLine) stampCell(sheet, rowId, colId);
    };
    for (const Moving &m : moving) {
        stampPlace(e.sheet, m.row, m.col);   // (every cell, not the formulas alone: the cells of an array are told too)
        stampPlace(e.toSheet, quint32(qint64(m.row) + dRow), quint32(qint64(m.col) + dCol));
    }
    for (const auto &p : overwritten) stampPlace(e.toSheet, p.first, p.second);
    for (const Moving &m : moving) clear(e.sheet, m.row, m.col);
    for (const auto &p : overwritten) clear(e.toSheet, p.first, p.second);
    for (const Moving &m : moving) {
        put(e.toSheet, quint32(qint64(m.row) + dRow), quint32(qint64(m.col) + dCol), m.content, Marking::Later);
        if (m.content.isFormula) rec->landed.push_back({quint32(qint64(m.row) + dRow), quint32(qint64(m.col) + dCol)});
    }
    for (const ArrayGoing &going : arrays) {
        qint32 slot = -1;
        for (size_t k = 0; k < freeSlots.size(); ++k)
            if (freeSlots[k].first == e.toSheet) {
                slot = freeSlots[k].second;
                freeSlots.erase(freeSlots.begin() + qint64(k));
                break;
            }
        bool added = false;
        const qint32 used = makeArray(e.toSheet, going.row, going.col, going.range, slot, &added);
        if (used >= 0) rec->arraysMade.push_back({e.toSheet, used, going.range, added});
    }

    // ---- merged ranges
    {
        QVector<CellRange> keptFrom, arrivedTo;
        for (CellRange r : S.merges) {
            if (r.row1 >= e.row1 && r.row2 <= e.row2 && r.col1 >= e.col1 && r.col2 <= e.col2) {
                r.row1 = quint32(qint64(r.row1) + dRow);
                r.row2 = quint32(qint64(r.row2) + dRow);
                r.col1 = quint32(qint64(r.col1) + dCol);
                r.col2 = quint32(qint64(r.col2) + dCol);
                arrivedTo.append(r);
                --S.mergeCount;
            } else {
                keptFrom.append(r);
            }
        }
        S.merges = keptFrom;
        QVector<CellRange> inDest;
        const QVector<CellRange> &current = e.toSheet == e.sheet ? S.merges : D.merges;
        for (const CellRange &r : current) {
            if (meets(r, e.toRow, e.toCol, toRow2, toCol2)) {
                --(e.toSheet == e.sheet ? S : D).mergeCount;
                continue;   // (written over)
            }
            inDest.append(r);
        }
        for (const CellRange &r : arrivedTo) {
            inDest.append(r);
            ++(e.toSheet == e.sheet ? S : D).mergeCount;
        }
        (e.toSheet == e.sheet ? S : D).merges = inDest;
    }

    // ---- the rules of validation
    adjustValidations(e, names, rec);
    adjustConditionalFormats(e, names, rec);

    // ---- the defined names
    bool namesChanged = false;
    for (auto it = m_names.begin(); it != m_names.end(); ++it) {
        const int bar = int(it.key().indexOf(QLatin1Char('|')));
        const int scope = bar > 0 ? it.key().left(bar).toInt() : -1;
        bool changed = false;
        const QString now = adjustFormula(it.value(), scope >= 0 ? scope : 0, names, e, &changed, true);
        if (!changed) continue;
        rec->names.push_back({it.key(), it.value()});
        it.value() = now;
        namesChanged = true;
    }
    if (namesChanged) {
        m_nameTraits.clear();
        for (SheetState &state : m_state) {
            for (Compiled &slot : state.plain) slot.tried = false;
            for (Compiled &slot : state.array) slot.tried = false;
        }
    }

    // ---- the formulas outside that read the block
    for (const Rewrite &r : rewrites) {
        Node &n0 = m_nodes[r.id];
        if (n0.flags & Dead) continue;
        const bool told = n0.flags & Told;
        if (r.now.isEmpty() && !told && !namesChanged) continue;
        const int sheet = n0.sheet;
        Sheet &sn = m_sheets[size_t(sheet)];
        Cell *c = sn.cells.findMutableById(n0.rowId, n0.colId);
        if (!c || !c->hasFormula()) continue;
        if (!r.now.isEmpty()) {
            EditRecord::FormulaWas was;
            was.sheet = sheet;
            was.row = rowOf(n0);
            was.col = colOf(n0);
            was.text = r.was;
            was.hasStored = false;
            if (n0.flags & Kept) {
                was.hasStored = true;
                was.stored = nodeValue(n0, r.id);
            }
            rec->was.push_back(std::move(was));
            retext(sheet, c, r.now);   // (in place: the formula keeps its place, and a cell of a shared family has a text of its own from now)
            ++rec->formulasRewritten;
            rec->rewritten.push_back({sheet, rowOf(n0), colOf(n0), n0.rowId, n0.colId, bool(c->flags & DataTable)});
        }
        const quint32 rowId = n0.rowId, colId = n0.colId;
        stampCell(sheet, rowId, colId);
        dropNode(r.id);
        const quint32 id = addNodeById(sheet, rowId, colId, *c, c->formula);
        ++m_editStats.nodesRemade;
        queueDependents(areaOf(m_nodes[id]));
    }
    m_editStats.formulasRewritten += rec->formulasRewritten;
    m_editStats.namesRewritten += qint64(rec->names.size());
    m_editStats.rulesRewritten += qint64(rec->validations.size() + rec->conditionalFormats.size());
    m_editStats.graphWork += m_graph->buildWork() + m_graph->queryWork() - graphWorkBefore;
    // what read the places is worked out again
    queueDependents(area);
    CellArea dest = area;
    dest.sheet1 = dest.sheet2 = e.toSheet;
    dest.row1 = e.toRow;
    dest.row2 = toRow2;
    dest.col1 = e.toCol;
    dest.col2 = toCol2;
    queueDependents(dest);
    if (!m_running) beginRecalculation();
    return true;
}

bool Calculation::undoMove(const EditRecord &rec, EditRecord *done)
{
    const StructEdit &e = rec.edit;
    ++m_state[size_t(e.sheet)].epoch;
    if (e.toSheet != e.sheet) ++m_state[size_t(e.toSheet)].epoch;
    auto stampPlace = [&](int sheet, quint32 row, quint32 col) {
        const CellStore &cells = m_sheets[size_t(sheet)].cells;
        const quint32 rowId = cells.rowIdAt(row), colId = cells.colIdAt(col);
        if (rowId != CellStore::kNoLine && colId != CellStore::kNoLine) stampCell(sheet, rowId, colId);
    };
    for (const EditRecord::GoneCell &g : rec.gone) stampPlace(g.sheet, g.row, g.col);
    for (const auto &p : rec.landed) stampPlace(e.toSheet, p.first, p.second);
    if (done) {
        // (what undoing took away: the cells in both places, and what was put back is worked out afresh)
        *done = EditRecord();
        done->edit = e.inverse();
        for (const EditRecord::GoneCell &g : rec.gone) done->gone.push_back(placeGone(g.sheet, g.row, g.col));
        for (const auto &p : rec.landed) done->gone.push_back(placeGone(e.toSheet, p.first, p.second));
    }
    abandonNow();
    BulkEdit bulk(this);
    const quint32 rows = e.row2 - e.row1 + 1, cols = e.col2 - e.col1 + 1;
    const quint32 toRow2 = e.toRow + rows - 1, toCol2 = e.toCol + cols - 1;
    auto clearRange = [&](int sheet, quint32 row1, quint32 col1, quint32 row2, quint32 col2) {
        std::vector<std::pair<quint32, quint32>> there;
        m_sheets[size_t(sheet)].cells.forEachInRange(row1, col1, row2, col2, [&](quint32 row, quint32 col, const Cell &) {
            there.push_back({row, col});
            return true;
        });
        for (const auto &p : there) {
            setValue(sheet, p.first, p.second, Value(), Marking::Later);
            setStyle(sheet, p.first, p.second, 0);
        }
    };
    // (the arrays that landed are taken apart so that their cells can be cleared, and their places in the lists go)
    for (size_t k = rec.arraysMade.size(); k-- > 0;) {
        const EditRecord::ArraySlot &made = rec.arraysMade[k];
        dissolveArray(made.sheet, made.slot);
        QVector<CellRange> &list = m_sheets[size_t(made.sheet)].ranges;
        if (made.added && made.slot == list.size() - 1) list.removeLast();
    }
    clearRange(e.toSheet, e.toRow, e.toCol, toRow2, toCol2);
    clearRange(e.sheet, e.row1, e.col1, e.row2, e.col2);
    for (const EditRecord::GoneCell &g : rec.gone) put(g.sheet, g.row, g.col, g.content, Marking::Later);
    for (const EditRecord::ArraySlot &taken : rec.arraysTaken) {
        QVector<CellRange> &list = m_sheets[size_t(taken.sheet)].ranges;
        if (taken.slot >= 0 && taken.slot < list.size()) list[taken.slot] = taken.range;
    }
    putBackKinds(rec);
    for (const EditRecord::FormulaWas &w : rec.was) {
        const Value *stored = w.hasStored ? &w.stored : nullptr;
        if (rewriteArrayText(w.sheet, w.row, w.col, w.text)) continue;
        setFormula(w.sheet, w.row, w.col, w.text, Marking::Later, stored);
    }
    for (const auto &entry : rec.validations)
        if (entry.first >= 0 && entry.first < int(m_sheets.size())) {
            m_sheets[size_t(entry.first)].validations = entry.second;
            forgetRuleIndex();
        }
    for (const auto &entry : rec.conditionalFormats)
        if (entry.first >= 0 && entry.first < int(m_sheets.size())) m_sheets[size_t(entry.first)].conditionalFormats = entry.second;
    Sheet &S = m_sheets[size_t(e.sheet)];
    S.merges = rec.merges;
    S.mergeCount = rec.mergeCount;
    if (e.toSheet != e.sheet) {
        Sheet &D = m_sheets[size_t(e.toSheet)];
        D.merges = rec.toMerges;
        D.mergeCount = rec.toMergeCount;
    }
    bool changed = false;
    for (const auto &entry : rec.names) {
        m_names[entry.first] = entry.second;
        changed = true;
    }
    if (changed) {
        m_nameTraits.clear();
        for (SheetState &state : m_state) {
            for (Compiled &slot : state.plain) slot.tried = false;
            for (Compiled &slot : state.array) slot.tried = false;
        }
        for (quint32 id = 0; id < quint32(m_nodes.size()); ++id) {
            const Node &n = m_nodes[id];
            if ((n.flags & Dead) || !(n.flags & Told)) continue;
            Cell *c = cellOfMutable(n);
            if (!c || !c->hasFormula()) continue;
            const int sheet = n.sheet;
            const qint32 formula = c->formula;
            const quint32 rowId = n.rowId, colId = n.colId;
            dropNode(id);
            addNodeById(sheet, rowId, colId, *c, formula);
        }
    }
    if (!m_running) beginRecalculation();
    return true;
}

} // namespace jp::office::sheet
