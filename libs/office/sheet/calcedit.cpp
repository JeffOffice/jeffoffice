// Rows, columns, and cells inserted and deleted in a calculation: the cells move, and every formula that reads cells that moved is written again with its references following them.
// See Calculation::applyEdit.
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

// The cells whose reading is changed by the edit: those of the block the cells of it move in, and the block's reach (to the edge of the sheet).
CellArea affectedArea(const StructEdit &e)
{
    using K = StructEdit::Kind;
    CellArea a;
    a.sheet1 = a.sheet2 = e.sheet;
    a.row1 = 0;
    a.col1 = 0;
    a.row2 = kGridRows - 1;
    a.col2 = kGridCols - 1;
    switch (e.kind) {
    case K::InsertRows:
    case K::DeleteRows: a.row1 = e.first; break;
    case K::InsertColumns:
    case K::DeleteColumns: a.col1 = e.first; break;
    case K::InsertCellsDown:
    case K::DeleteCellsUp:
        a.row1 = e.row1;
        a.col1 = e.col1;
        a.col2 = e.col2;
        break;
    case K::InsertCellsRight:
    case K::DeleteCellsLeft:
        a.row1 = e.row1;
        a.col1 = e.col1;
        a.row2 = e.row2;
        break;
    case K::Move:
    case K::Offset: break;
    }
    return a;
}

// How many rows (columns) an edit of cells makes room for, or closes up.
quint32 shiftBy(const StructEdit &e)
{
    using K = StructEdit::Kind;
    switch (e.kind) {
    case K::InsertCellsDown:
    case K::DeleteCellsUp: return e.row2 - e.row1 + 1;
    case K::InsertCellsRight:
    case K::DeleteCellsLeft: return e.col2 - e.col1 + 1;
    default: return e.count;
    }
}

bool deletes(const StructEdit &e)
{
    using K = StructEdit::Kind;
    return e.kind == K::DeleteRows || e.kind == K::DeleteColumns || e.kind == K::DeleteCellsUp || e.kind == K::DeleteCellsLeft;
}

} // namespace

bool Calculation::canApply(const StructEdit &e, QString *why) const
{
    using K = StructEdit::Kind;
    auto refuse = [&](const QString &text) {
        if (why) *why = text;
        return false;
    };
    if (e.sheet < 0 || e.sheet >= int(m_sheets.size())) return refuse(say("There is no such sheet."));
    if (e.kind == K::Move || e.kind == K::Offset) return refuse(say("That is not done by inserting or deleting."));
    const Sheet &s = m_sheets[size_t(e.sheet)];
    if (s.isProtected) return refuse(say("This sheet is protected, so rows, columns, and cells can't be inserted or deleted. Take the protection off in the program that put it on."));
    const quint32 count = shiftBy(e);
    if (count == 0) return refuse(say("There is nothing to insert or delete."));
    const CellArea region = affectedArea(e);
    // the cells of an array formula or a data table are not cut through, nor are they deleted
    for (const CellRange &r : s.ranges) {
        if (!r.valid || r.cellCount() <= 1) continue;
        bool hit = false;
        switch (e.kind) {
        case K::InsertRows: hit = r.row1 < e.first && r.row2 >= e.first; break;
        case K::DeleteRows: hit = r.row2 >= e.first && r.row1 < e.first + count; break;
        case K::InsertColumns: hit = r.col1 < e.first && r.col2 >= e.first; break;
        case K::DeleteColumns: hit = r.col2 >= e.first && r.col1 < e.first + count; break;
        default: hit = r.row2 >= region.row1 && r.row1 <= region.row2 && r.col2 >= region.col1 && r.col1 <= region.col2; break;
        }
        if (hit) return refuse(say("That would change part of an array formula or a data table, which can't be done."));
    }
    // cells would be pushed off the edge of the sheet
    if (!deletes(e)) {
        const bool rows = e.kind == K::InsertRows || e.kind == K::InsertCellsDown;
        const quint32 limit = rows ? kGridRows : kGridCols;
        bool pushed = false;
        const quint32 firstGone = limit - count;
        if (e.kind == K::InsertRows) {
            if (s.extentRows > firstGone) pushed = s.cells.find(firstGone, 0) != nullptr || s.extentRows > firstGone;
        } else if (e.kind == K::InsertColumns) {
            if (s.extentCols > firstGone) pushed = true;
        } else if (e.kind == K::InsertCellsDown) {
            s.cells.forEachInRange(firstGone, e.col1, kGridRows - 1, e.col2, [&](const Cell &c) {
                if (c.type != CellType::Empty || c.hasFormula()) pushed = true;
                return !pushed;
            });
        } else {
            s.cells.forEachInRange(e.row1, firstGone, e.row2, kGridCols - 1, [&](const Cell &c) {
                if (c.type != CellType::Empty || c.hasFormula()) pushed = true;
                return !pushed;
            });
        }
        if (e.kind == K::InsertRows && pushed) {
            // (the extent says there are cells at the end: look for one that holds something)
            pushed = false;
            s.cells.forEachInRange(firstGone, 0, kGridRows - 1, kGridCols - 1, [&](const Cell &c) {
                if (c.type != CellType::Empty || c.hasFormula()) pushed = true;
                return !pushed;
            });
        } else if (e.kind == K::InsertColumns && pushed) {
            pushed = false;
            s.cells.forEach([&](const Cell &c) {
                if (c.col >= firstGone && (c.type != CellType::Empty || c.hasFormula())) pushed = true;
            });
        }
        if (pushed) return refuse(say("That would push cells off the edge of the sheet. Delete the cells at the end first."));
    }
    return true;
}

bool Calculation::applyEdit(const StructEdit &e, EditRecord *rec)
{
    using K = StructEdit::Kind;
    if (!canApply(e, nullptr)) return false;
    abandonNow();
    *rec = EditRecord();
    rec->edit = e;
    rec->sheet = e.sheet;
    Sheet &S = m_sheets[size_t(e.sheet)];
    rec->ranges = S.ranges;
    rec->merges = S.merges;
    rec->mergeCount = S.mergeCount;
    rec->columns = S.columns;
    rec->rowSpecs = S.rowSpecs;
    rec->extentRows = S.extentRows;
    rec->extentCols = S.extentCols;
    rec->rowsSeen = m_rows[size_t(e.sheet)];
    rec->colsSeen = m_cols[size_t(e.sheet)];
    QStringList names;
    for (const SheetEntry &se : m_book.info().sheets) names << se.name;

    // ---- what reads the cells that move: the graph says (before anything moves), and the formulas the graph does not follow (those kept apart, those that read where they are told)
    const CellArea region = affectedArea(e);
    std::vector<quint32> candidates;
    m_graph->dependentsOfArea(region, &candidates);
    for (quint32 id = 0; id < quint32(m_nodes.size()); ++id) {
        const Node &n = m_nodes[id];
        if (n.flags & Dead) continue;
        if ((n.flags & Told) || ((n.flags & Kept) && n.kept != 'U')) candidates.push_back(id);
    }
    std::sort(candidates.begin(), candidates.end());
    candidates.erase(std::unique(candidates.begin(), candidates.end()), candidates.end());
    struct Rewrite {
        quint32 id;
        QString was, now;
    };
    std::vector<Rewrite> rewrites;
    rewrites.reserve(candidates.size());
    for (const quint32 id : candidates) {
        const Node &n = m_nodes[id];
        if (n.flags & Dead) continue;
        const Sheet &sn = m_sheets[size_t(n.sheet)];
        const Cell *c = sn.cells.find(n.row, n.col);
        if (!c || !c->hasFormula()) continue;
        Rewrite r;
        r.id = id;
        r.was = sn.formulaText(*c);
        bool changed = false;
        r.now = adjustFormula(r.was, n.sheet, names, e, &changed);
        if (!changed) r.now.clear();
        rewrites.push_back(std::move(r));
    }
    // the places the formulas are at now, to say where they were
    struct Place {
        quint32 id;
        int sheet;
        quint32 row, col;
    };
    std::vector<Place> places;
    places.reserve(rewrites.size());
    for (const Rewrite &r : rewrites) places.push_back({r.id, m_nodes[r.id].sheet, m_nodes[r.id].row, m_nodes[r.id].col});

    // ---- the cells that are deleted, as they are
    const quint32 count = shiftBy(e);
    if (deletes(e)) {
        quint32 r1 = 0, c1 = 0, r2 = kGridRows - 1, c2 = kGridCols - 1;
        switch (e.kind) {
        case K::DeleteRows: r1 = e.first; r2 = e.first + count - 1; break;
        case K::DeleteColumns: c1 = e.first; c2 = e.first + count - 1; break;
        case K::DeleteCellsUp: r1 = e.row1; r2 = e.row2; c1 = e.col1; c2 = e.col2; break;
        default: r1 = e.row1; r2 = e.row2; c1 = e.col1; c2 = e.col2; break;
        }
        S.cells.forEachInRange(r1, c1, r2, c2, [&](const Cell &c) {
            rec->gone.push_back({e.sheet, c.row, c.col, content(e.sheet, c.row, c.col)});
            return true;
        });
    }

    // ---- the cells move; the nodes of the formulas among them go with them
    std::vector<Cell> removed;
    SheetState &st = m_state[size_t(e.sheet)];
    auto onMoved = [&](const Cell &, const Cell &after) {
        ++rec->cellsMoved;
        if (!after.hasFormula() || after.formula < 0 || size_t(after.formula) >= st.nodeOf.size()) return;
        const quint32 id = st.nodeOf[size_t(after.formula)];
        if (id == kNone) return;
        m_nodes[id].row = after.row;
        m_nodes[id].col = after.col;
    };
    switch (e.kind) {
    case K::InsertRows:
        S.cells.reshuffle(e.first, [&](const Cell &c) { return c.row >= e.first; }, [&](Cell *c) { c->row += count; return c->row < kGridRows; }, onMoved, &removed);
        break;
    case K::DeleteRows:
        S.cells.reshuffle(e.first, [&](const Cell &c) { return c.row >= e.first; }, [&](Cell *c) {
            if (c->row < e.first + count) return false;
            c->row -= count;
            return true;
        }, onMoved, &removed);
        break;
    case K::InsertColumns:
        S.cells.reshuffle(0, [&](const Cell &c) { return c.col >= e.first; }, [&](Cell *c) {
            const quint32 col = quint32(c->col) + count;
            c->col = quint16(std::min<quint32>(col, 0xFFFF));
            return col < kGridCols;
        }, onMoved, &removed);
        break;
    case K::DeleteColumns:
        S.cells.reshuffle(0, [&](const Cell &c) { return c.col >= e.first; }, [&](Cell *c) {
            if (c->col < e.first + count) return false;
            c->col = quint16(c->col - count);
            return true;
        }, onMoved, &removed);
        break;
    case K::InsertCellsDown:
        S.cells.reshuffle(e.row1, [&](const Cell &c) { return c.row >= e.row1 && c.col >= e.col1 && c.col <= e.col2; }, [&](Cell *c) { c->row += count; return c->row < kGridRows; }, onMoved, &removed);
        break;
    case K::DeleteCellsUp:
        S.cells.reshuffle(e.row1, [&](const Cell &c) { return c.row >= e.row1 && c.col >= e.col1 && c.col <= e.col2; }, [&](Cell *c) {
            if (c->row < e.row1 + count) return false;
            c->row -= count;
            return true;
        }, onMoved, &removed);
        break;
    case K::InsertCellsRight:
        S.cells.reshuffle(e.row1, [&](const Cell &c) { return c.row >= e.row1 && c.row <= e.row2 && c.col >= e.col1; }, [&](Cell *c) {
            const quint32 col = quint32(c->col) + count;
            c->col = quint16(std::min<quint32>(col, 0xFFFF));
            return col < kGridCols;
        }, onMoved, &removed);
        break;
    case K::DeleteCellsLeft:
        S.cells.reshuffle(e.row1, [&](const Cell &c) { return c.row >= e.row1 && c.row <= e.row2 && c.col >= e.col1; }, [&](Cell *c) {
            if (c->col < e.col1 + count) return false;
            c->col = quint16(c->col - count);
            return true;
        }, onMoved, &removed);
        break;
    default: break;
    }
    // the formulas that were in cells that are gone
    for (const Cell &c : removed) {
        if (!c.hasFormula() || c.formula < 0 || size_t(c.formula) >= st.nodeOf.size()) continue;
        const quint32 id = st.nodeOf[size_t(c.formula)];
        if (id != kNone) dropNode(id);
    }

    // ---- the sheet's other structures
    for (CellRange &r : S.ranges) adjustRange(e, e.sheet, &r);
    {
        QVector<CellRange> kept;
        for (CellRange r : S.merges)
            if (adjustRange(e, e.sheet, &r)) kept.append(r);
            else --S.mergeCount;
        S.merges = kept;
    }
    if (e.kind == K::InsertRows || e.kind == K::DeleteRows) {
        QVector<RowSpec> specs;
        for (RowSpec r : S.rowSpecs) {
            if (e.kind == K::InsertRows) {
                if (r.row >= e.first) r.row += count;
            } else {
                if (r.row >= e.first && r.row < e.first + count) continue;
                if (r.row >= e.first + count) r.row -= count;
            }
            specs.append(r);
        }
        S.rowSpecs = specs;
    }
    if (e.kind == K::InsertColumns || e.kind == K::DeleteColumns) {
        QVector<ColumnSpan> spans;
        for (ColumnSpan c : S.columns) {
            if (e.kind == K::InsertColumns) {
                if (c.first >= e.first) {
                    c.first += count;
                    c.last += count;
                } else if (c.last >= e.first) {
                    c.last += count;   // (a run with the new columns in it widens)
                }
            } else {
                const quint32 end = e.first + count;
                if (c.last < e.first) {
                    // before: as it is
                } else if (c.first >= end) {
                    c.first -= count;
                    c.last -= count;
                } else {
                    // overlapped: what is left of it
                    const quint32 before = c.first < e.first ? e.first - c.first : 0, after = c.last >= end ? c.last - end + 1 : 0;
                    if (before + after == 0) continue;
                    const quint32 first = c.first < e.first ? c.first : e.first;
                    c.first = first;
                    c.last = first + before + after - 1;
                }
            }
            if (c.first < kGridCols) spans.append(c);
        }
        S.columns = spans;
    }
    auto shiftExtent = [&](quint32 *extent, quint32 at, quint32 limit) {
        if (*extent <= at) return;
        if (deletes(e)) *extent = std::max(at, *extent > count ? *extent - count : 0u);
        else *extent = std::min(limit, *extent + count);
    };
    const bool rowEdit = e.kind == K::InsertRows || e.kind == K::DeleteRows || e.kind == K::InsertCellsDown || e.kind == K::DeleteCellsUp;
    const quint32 at = (e.kind == K::InsertRows || e.kind == K::DeleteRows || e.kind == K::InsertColumns || e.kind == K::DeleteColumns) ? e.first : (rowEdit ? e.row1 : e.col1);
    if (rowEdit) {
        shiftExtent(&S.extentRows, at, kGridRows);
        quint32 seen = m_rows[size_t(e.sheet)];
        shiftExtent(&seen, at, kGridRows);
        m_rows[size_t(e.sheet)] = seen;
    } else {
        shiftExtent(&S.extentCols, at, kGridCols);
        quint32 seen = m_cols[size_t(e.sheet)];
        shiftExtent(&seen, at, kGridCols);
        m_cols[size_t(e.sheet)] = seen;
    }

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

    // ---- the formulas that read what moved are written again, with what they read followed; those that read where they are told are made again too
    for (size_t i = 0; i < rewrites.size(); ++i) {
        const Rewrite &r = rewrites[i];
        Node &n0 = m_nodes[r.id];
        if (n0.flags & Dead) continue;
        const bool told = n0.flags & Told;
        if (r.now.isEmpty() && !told && !namesChanged) continue;
        const int sheet = n0.sheet;
        Sheet &sn = m_sheets[size_t(sheet)];
        Cell *c = sn.cells.findMutable(n0.row, n0.col);
        if (!c || !c->hasFormula()) continue;
        if (!r.now.isEmpty()) {
            EditRecord::FormulaWas was;
            was.sheet = places[i].sheet;
            was.row = places[i].row;
            was.col = places[i].col;
            was.text = r.was;
            was.hasStored = false;
            if (n0.flags & Kept) {
                was.hasStored = true;
                was.stored = nodeValue(n0, r.id);
            }
            rec->was.push_back(std::move(was));
            sn.formulaTexts.append(r.now);
            if (c->flags & (ArrayFormula | DataTable)) {
                sn.formulas[c->formula].text = qint32(sn.formulaTexts.size() - 1);   // (the cells of its range point to it: it keeps its place)
            } else {
                Formula f = sn.formulas[c->formula];
                f.text = qint32(sn.formulaTexts.size() - 1);
                f.dRow = f.dCol = 0;
                sn.formulas.append(f);
                c->formula = qint32(sn.formulas.size() - 1);
            }
            ++rec->formulasRewritten;
        }
        dropNode(r.id);
        const quint32 id = addNode(sheet, *c, c->formula);
        queueDependents(areaOf(m_nodes[id]));
    }
    // what reads cells that moved in or out of a range it reads (a text that is as it was, a whole column, or a block shifted inside a range) has its value again
    queueDependents(region);
    if (!m_running) beginRecalculation();
    return true;
}

bool Calculation::rewriteArrayText(int sheet, quint32 row, quint32 col, const QString &text)
{
    Sheet &s = m_sheets[size_t(sheet)];
    Cell *c = s.cells.findMutable(row, col);
    if (!c || !(c->flags & (ArrayFormula | DataTable)) || c->formula < 0) return false;
    abandonNow();
    SheetState &st = m_state[size_t(sheet)];
    if (size_t(c->formula) < st.nodeOf.size() && st.nodeOf[size_t(c->formula)] != kNone) dropNode(st.nodeOf[size_t(c->formula)]);
    s.formulaTexts.append(text);
    s.formulas[c->formula].text = qint32(s.formulaTexts.size() - 1);
    const quint32 id = addNode(sheet, *c, c->formula);
    queueDependents(areaOf(m_nodes[id]));
    return true;
}

bool Calculation::undoEdit(const EditRecord &rec)
{
    EditRecord throwaway;
    if (!applyEdit(rec.edit.inverse(), &throwaway)) return false;
    Sheet &S = m_sheets[size_t(rec.sheet)];
    // the cells that were deleted are put back, and the formulas that were written again have the text they had
    for (const EditRecord::GoneCell &g : rec.gone) put(g.sheet, g.row, g.col, g.content, Marking::Later);
    for (const EditRecord::FormulaWas &w : rec.was) {
        const jp::calc::Value *stored = w.hasStored ? &w.stored : nullptr;
        if (rewriteArrayText(w.sheet, w.row, w.col, w.text)) continue;
        setFormula(w.sheet, w.row, w.col, w.text, Marking::Later, stored);
    }
    S.ranges = rec.ranges;
    S.merges = rec.merges;
    S.mergeCount = rec.mergeCount;
    S.columns = rec.columns;
    S.rowSpecs = rec.rowSpecs;
    S.extentRows = std::max(S.extentRows, rec.extentRows);
    S.extentCols = std::max(S.extentCols, rec.extentCols);
    m_rows[size_t(rec.sheet)] = std::max(m_rows[size_t(rec.sheet)], rec.rowsSeen);
    m_cols[size_t(rec.sheet)] = std::max(m_cols[size_t(rec.sheet)], rec.colsSeen);
    // the names (by the key they are kept by)
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
        // (what uses a name is made again)
        for (quint32 id = 0; id < quint32(m_nodes.size()); ++id) {
            const Node &n = m_nodes[id];
            if (n.flags & Dead) continue;
            if (!(n.flags & Told)) continue;
            Sheet &sn = m_sheets[size_t(n.sheet)];
            Cell *c = sn.cells.findMutable(n.row, n.col);
            if (!c || !c->hasFormula()) continue;
            const int sheet = n.sheet;
            const qint32 formula = c->formula;
            dropNode(id);
            addNode(sheet, *c, formula);
        }
    }
    if (!m_running) beginRecalculation();
    return true;
}

} // namespace jp::office::sheet
