// Rows, columns, and cells inserted and deleted in a calculation: the cells move, and every formula that reads cells that moved is written again with its references following them.
// See Calculation::applyEdit.
#include "sheet/calculation.h"
#include "sheet/tables.h"

#include <QCoreApplication>

#include <algorithm>
#include <unordered_map>

namespace jp::office::sheet {

using jp::calc::CellArea;
using jp::calc::kGridCols;
using jp::calc::kGridRows;
using jp::calc::Value;

namespace {

QString say(const char *text) { return QCoreApplication::translate("jp::office::sheet::Calculation", text); }

std::atomic<bool> g_graphShifts{true};

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
    if (e.sheet < 0 || e.sheet >= int(m_sheets.size())) return refuse(say(QT_TRANSLATE_NOOP("jp::office::sheet::Calculation", "There is no such sheet.")));
    if (e.kind == K::Move) return canMoveCells(e, why);
    if (e.kind == K::Offset) return refuse(say(QT_TRANSLATE_NOOP("jp::office::sheet::Calculation", "That is not done by inserting or deleting.")));
    const Sheet &s = m_sheets[size_t(e.sheet)];
    if (m_untracked > 0) return refuse(say(QT_TRANSLATE_NOOP("jp::office::sheet::Calculation", "This workbook has too many formulas for JeffCrunch to follow when rows, columns, or cells are inserted or deleted.")));
    if (s.isProtected) return refuse(say(QT_TRANSLATE_NOOP("jp::office::sheet::Calculation", "This sheet is protected, so rows, columns, and cells can't be inserted or deleted. Take the protection off in the program that put it on.")));
    const quint32 count = shiftBy(e);
    if (count == 0) return refuse(say(QT_TRANSLATE_NOOP("jp::office::sheet::Calculation", "There is nothing to insert or delete.")));
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
        if (hit) return refuse(say(QT_TRANSLATE_NOOP("jp::office::sheet::Calculation", "That would change part of an array formula or a data table, which can't be done.")));
    }
    // a data table reads its input cells (its text names them, TABLE(A1) or TABLE(A1,B1)): the file has no way to say that one was deleted, so that is not done
    if (deletes(e)) {
        QStringList names;
        for (const quint32 id : m_special) {
            const Node &n = m_nodes[id];
            if ((n.flags & Dead) || n.sheet != e.sheet || n.kept != 'D') continue;
            const Cell *c = s.cells.find(n.row, n.col);
            if (!c || !c->hasFormula()) continue;
            if (names.isEmpty())
                for (const SheetEntry &se : m_book.info().sheets) names << se.name;
            const QString was = s.formulaText(*c);
            bool changed = false;
            const QString now = adjustFormula(was, n.sheet, names, e, &changed);
            if (changed && now.contains(QLatin1String("#REF!")) && !was.contains(QLatin1String("#REF!")))
                return refuse(say(QT_TRANSLATE_NOOP("jp::office::sheet::Calculation", "That would delete a cell that a data table reads, which can't be done.")));
        }
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
        if (pushed) return refuse(say(QT_TRANSLATE_NOOP("jp::office::sheet::Calculation", "That would push cells off the edge of the sheet. Delete the cells at the end first.")));
    }
    return true;
}

qint64 Calculation::estimateUndoBytes(const StructEdit &e, qint64 stopAt) const
{
    using K = StructEdit::Kind;
    constexpr qint64 kPerCell = 320, kPerFormula = 320;   // (what a cell of the record and a formula written again take: see StructChange::bytes)
    if (e.sheet < 0 || e.sheet >= int(m_sheets.size())) return 0;
    qint64 bytes = 0;
    auto countCells = [&](int sheet, quint32 r1, quint32 c1, quint32 r2, quint32 c2) {
        if (sheet < 0 || sheet >= int(m_sheets.size())) return;
        m_sheets[size_t(sheet)].cells.forEachInRange(r1, c1, r2, c2, [&](const Cell &) {
            bytes += kPerCell;
            return bytes < stopAt;
        });
    };
    CellArea region = affectedArea(e);
    if (e.kind == K::Move) {
        const quint32 toRow2 = e.toRow + (e.row2 - e.row1), toCol2 = e.toCol + (e.col2 - e.col1);
        countCells(e.sheet, e.row1, e.col1, e.row2, e.col2);
        if (bytes < stopAt) countCells(e.toSheet, e.toRow, e.toCol, toRow2, toCol2);
        region.row1 = e.row1;
        region.row2 = e.row2;
        region.col1 = e.col1;
        region.col2 = e.col2;
    } else if (deletes(e)) {
        switch (e.kind) {
        case K::DeleteRows: countCells(e.sheet, e.first, 0, e.first + e.count - 1, kGridCols - 1); break;
        case K::DeleteColumns: countCells(e.sheet, 0, e.first, kGridRows - 1, e.first + e.count - 1); break;
        default: countCells(e.sheet, e.row1, e.col1, e.row2, e.col2); break;
        }
    }
    if (bytes >= stopAt) return bytes;
    // the formulas that read what moves are written again: the graph says how many
    std::vector<quint32> readers;
    m_graph->dependentsOfArea(region, &readers);
    bytes += (qint64(readers.size()) + qint64(m_special.size())) * kPerFormula;
    return bytes;
}

// The formulas that are told where they read or are kept apart (not those with a function the engine lacks, which the graph holds), added to `out`.
void Calculation::specialNodes(std::vector<quint32> *out)
{
    // (the list is made smaller when most of it is gone or repeated)
    if (m_special.size() > size_t(m_formulaCount) * 2 + 4096) {
        std::sort(m_special.begin(), m_special.end());
        m_special.erase(std::unique(m_special.begin(), m_special.end()), m_special.end());
        size_t keep = 0;
        for (const quint32 id : m_special) {
            const Node &n = m_nodes[id];
            if (!(n.flags & Dead) && ((n.flags & Told) || ((n.flags & Kept) && n.kept != 'U'))) m_special[keep++] = id;
        }
        m_special.resize(keep);
    }
    for (const quint32 id : m_special) {
        const Node &n = m_nodes[id];
        if (!(n.flags & Dead) && ((n.flags & Told) || ((n.flags & Kept) && n.kept != 'U'))) out->push_back(id);
    }
}

void Calculation::setGraphShifts(bool on) { g_graphShifts = on; }

bool Calculation::applyEdit(const StructEdit &e, EditRecord *rec)
{
    using K = StructEdit::Kind;
    if (!canApply(e, nullptr)) return false;
    if (e.kind == K::Move) return applyMove(e, rec);
    abandonNow();
    *rec = EditRecord();
    rec->edit = e;
    rec->sheet = e.sheet;
    Sheet &S = m_sheets[size_t(e.sheet)];
    rec->ranges = S.ranges;
    rec->merges = S.merges;
    rec->autoFilter = S.autoFilter;
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
    specialNodes(&candidates);
    std::sort(candidates.begin(), candidates.end());
    candidates.erase(std::unique(candidates.begin(), candidates.end()), candidates.end());
    m_editWork += qint64(candidates.size()) + qint64(m_special.size());
    struct Rewrite {
        quint32 id;
        QString was, now;   // the text of the formula for its cell, and what it is to be (empty: as it was)
        int group = -1;     // or the group of formulas of a shared family that are written again alike, and keep their share of one text
    };
    struct Group {
        int sheet;
        QString newText;
        qint32 newIndex = -1;   // its place, once it is made
        int record = -1;        // and where the group is in the record
        bool failed = false;    // the text could not be made: the cells are written one by one
        bool hasGone = false;   // a reference of it is to cells that are deleted (it is #REF! now)
    };
    std::vector<Rewrite> rewrites;
    rewrites.reserve(candidates.size());
    std::vector<Group> groups;
    // (the formulas of a shared family are told apart by what the edit does to the references each has, and the new text is made once for those it does alike)
    struct GroupKey {
        int sheet;
        qint32 text;
        TextRewriter::Key key;
    };
    std::vector<GroupKey> groupKeys;
    std::unordered_multimap<quint64, int> groupsByHash;
    std::unordered_map<quint64, TextRewriter> rewriters;
    const bool rowsOrColumns = e.kind == K::InsertRows || e.kind == K::DeleteRows || e.kind == K::InsertColumns || e.kind == K::DeleteColumns;
    for (const quint32 id : candidates) {
        const Node &n = m_nodes[id];
        if (n.flags & Dead) continue;
        const Sheet &sn = m_sheets[size_t(n.sheet)];
        const Cell *c = sn.cells.find(n.row, n.col);
        if (!c || !c->hasFormula()) continue;
        Rewrite r;
        r.id = id;
        if (rowsOrColumns && !(c->flags & (ArrayFormula | DataTable)) && !(n.flags & Kept) && c->formula >= 0 && size_t(c->formula) < size_t(sn.formulas.size())) {
            const Formula &f = sn.formulas[c->formula];
            countTexts(n.sheet);
            if (f.text >= 0 && (f.dRow != 0 || f.dCol != 0 || m_state[size_t(n.sheet)].textRefs[size_t(f.text)] > 1)) {
                const quint64 textKey = (quint64(quint32(n.sheet)) << 32) | quint32(f.text);
                auto rw = rewriters.find(textKey);
                if (rw == rewriters.end()) rw = rewriters.emplace(textKey, TextRewriter(sn.formulaTexts[f.text], n.sheet, names)).first;
                TextRewriter::Key key;
                const TextRewriter::Fate fate = rw->second.fate(e, f.dRow, f.dCol, &key);
                if (fate == TextRewriter::Fate::Same) {
                    rewrites.push_back(std::move(r));
                    continue;
                }
                if (fate == TextRewriter::Fate::Changed) {
                    const quint64 hash = key.hash() ^ (textKey * 0x9E3779B97F4A7C15ull);
                    int found = -1;
                    for (auto range = groupsByHash.equal_range(hash); range.first != range.second && found < 0; ++range.first) {
                        const GroupKey &k = groupKeys[size_t(range.first->second)];
                        if (k.sheet == n.sheet && k.text == f.text && k.key == key) found = range.first->second;
                    }
                    if (found < 0) {
                        Group g;
                        g.sheet = n.sheet;
                        g.newText = rw->second.apply(key);
                        g.failed = g.newText.isEmpty();
                        for (int k = 0; k < TextRewriter::kMaxRefs; ++k) g.hasGone = g.hasGone || key.v[k * 5] == 2;
                        groups.push_back(std::move(g));
                        groupKeys.push_back({n.sheet, f.text, key});
                        found = int(groups.size()) - 1;
                        groupsByHash.emplace(hash, found);
                    }
                    if (!groups[size_t(found)].failed) {
                        r.group = found;
                        rewrites.push_back(std::move(r));
                        continue;
                    }
                }
            }
        }
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
            rec->gone.push_back(goneCell(e.sheet, c));
            return true;
        });
    }

    // ---- the cells move; the nodes of the formulas among them go with them
    std::vector<Cell> removed;
    SheetState &st = m_state[size_t(e.sheet)];
    // (a formula that moves has its node told where it is, and the window's value for it is moved)
    auto onFormulaMoved = [&](const Cell &before, const Cell &after) {
        rec->movedFormulas.push_back({before.row, quint32(before.col), after.row, quint32(after.col)});
        if (!after.hasFormula() || after.formula < 0 || size_t(after.formula) >= st.nodeOf.size()) return;
        const quint32 id = st.nodeOf[size_t(after.formula)];
        if (id == kNone) return;
        m_nodes[id].row = after.row;
        m_nodes[id].col = after.col;
    };
    auto onMoved = [&](const Cell &before, const Cell &after) {
        ++rec->cellsMoved;
        if (after.hasFormula() || (after.flags & ArrayMember)) onFormulaMoved(before, after);
    };
    switch (e.kind) {
    case K::InsertRows: rec->cellsMoved = S.cells.shiftRows(e.first, qint64(count), kGridRows, onFormulaMoved, &removed); break;
    case K::DeleteRows: rec->cellsMoved = S.cells.shiftRows(e.first, -qint64(count), kGridRows, onFormulaMoved, &removed); break;
    case K::InsertColumns: rec->cellsMoved = S.cells.shiftColumns(e.first, qint64(count), kGridCols, onFormulaMoved, &removed); break;
    case K::DeleteColumns: rec->cellsMoved = S.cells.shiftColumns(e.first, -qint64(count), kGridCols, onFormulaMoved, &removed); break;
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
        releaseFormula(e.sheet, c.formula);   // (nothing has the formula of a cell that is gone; Undo makes it again from the record)
    }
    // Rows inserted or deleted: what every formula reads on the sheet is where its cells are now, told to the graph at once; so a formula whose text is only moved with what it reads need
    // not tell the graph again (it is worked out again, as every formula that was written again is).
    const bool shiftedGraph = g_graphShifts.load(std::memory_order_relaxed) && (e.kind == K::InsertRows || e.kind == K::DeleteRows);
    if (shiftedGraph) {
        m_graph->shiftRows(e.sheet, e.first, e.kind == K::InsertRows ? qint64(count) : -qint64(count), kGridRows);
        // (a formula that reads a block of several sheets is not rewritten for a change of one of them: it is made again, and tells the graph what it reads)
        const std::vector<quint32> again(m_threeD.begin(), m_threeD.end());   // (a copy: each is dropped and made again, which changes the set)
        for (const quint32 id : again) {
            const Node &n = m_nodes[id];
            Sheet &sn = m_sheets[size_t(n.sheet)];
            Cell *c = sn.cells.findMutable(n.row, n.col);
            if (!c || !c->hasFormula()) continue;
            const int sheet = n.sheet;
            const qint32 formula = c->formula;
            dropNode(id);
            const quint32 now = addNode(sheet, *c, formula);
            queueDependents(areaOf(m_nodes[now]));
        }
    }

    // ---- the sheet's other structures
    for (CellRange &r : S.ranges)
        if (!adjustRange(e, e.sheet, &r)) r.valid = false;   // (the range of an array formula or a data table of one cell that was deleted: its place in the list stays, for Undo)
    {
        QVector<CellRange> kept;
        for (CellRange r : S.merges)
            if (adjustRange(e, e.sheet, &r) && r.cellCount() > 1) kept.append(r);
            else --S.mergeCount;
        S.merges = kept;
    }
    // the filter goes with its cells: its range follows them, and the conditions of its columns follow the columns (a filter whose range is deleted is gone)
    if (S.autoFilter.ref.valid) {
        AutoFilter moved = S.autoFilter;
        CellRange ref = moved.ref;
        if (!adjustRange(e, e.sheet, &ref)) {
            moved = AutoFilter();
        } else {
            QVector<FilterColumn> columns;
            for (FilterColumn c : moved.columns) {
                CellRange at;
                at.valid = true;
                at.row1 = at.row2 = S.autoFilter.ref.row1;
                at.col1 = at.col2 = S.autoFilter.ref.col1 + c.column;
                if (!adjustRange(e, e.sheet, &at) || at.col1 < ref.col1) continue;
                c.column = at.col1 - ref.col1;
                columns.append(c);
            }
            moved.ref = ref;
            moved.columns = columns;
        }
        S.autoFilter = moved;
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
        for (ColumnSpan c : S.columns)
            if (adjustColumnRun(e, &c.first, &c.last)) spans.append(c);
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

    m_editWork += rec->cellsMoved + qint64(rec->gone.size());

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

    // ---- the formulas that read what moved are written again, with what they read followed; those that read where they are told are made again too
    for (size_t i = 0; i < rewrites.size(); ++i) {
        const Rewrite &r = rewrites[i];
        Node &n0 = m_nodes[r.id];
        if (n0.flags & Dead) continue;
        const bool told = n0.flags & Told;
        if (r.now.isEmpty() && r.group < 0 && !told && !namesChanged) continue;
        const int sheet = n0.sheet;
        Sheet &sn = m_sheets[size_t(sheet)];
        Cell *c = sn.cells.findMutable(n0.row, n0.col);
        if (!c || !c->hasFormula()) continue;
        if (r.group >= 0) {
            // a formula of a shared family: it keeps its share of the family's text, which is made once, and it is as far from the cell it was written for as it was
            Group &g = groups[size_t(r.group)];
            Formula &f = sn.formulas[c->formula];
            if (g.newIndex < 0) g.newIndex = internText(sheet, g.newText);
            else holdText(sheet, g.newIndex);
            if (g.record < 0) {
                rec->groups.push_back({sheet, sn.formulaTexts[f.text], {}});
                g.record = int(rec->groups.size()) - 1;
            }
            rec->groups[size_t(g.record)].cells.push_back({places[i].row, places[i].col});
            const qint32 old = f.text;
            f.text = g.newIndex;
            releaseText(sheet, old);
            ++rec->formulasRewritten;
            rec->rewritten.push_back({sheet, c->row, c->col, false});
        } else if (!r.now.isEmpty()) {
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
            retext(sheet, c, r.now);   // (in place: the formula keeps its place, and a cell of a shared family has a text of its own from now)
            ++rec->formulasRewritten;
            rec->rewritten.push_back({sheet, c->row, c->col, bool(c->flags & DataTable)});
        }
        // A formula whose text only moved with what it reads (the graph has what it reads already) is worked out again in its place: its node stays, with its place in the graph. One that
        // reads where it is told, is kept apart, is an array, reads several sheets, or has a reference that is gone is made again.
        if (shiftedGraph && (r.group >= 0 || !r.now.isEmpty()) && !told && !(n0.flags & (Kept | Array | ThreeD)) && !(c->flags & (ArrayFormula | DataTable))
            && (r.group >= 0 ? !groups[size_t(r.group)].hasGone : !(r.now.contains(QLatin1String("#REF!")) && !r.was.contains(QLatin1String("#REF!"))))) {
            if (n0.state == Done) {
                n0.state = Dirty;
                ++m_notDone;
                m_dirty.push_back(r.id);
            }
            queueDependents(areaOf(n0));
            continue;
        }
        const bool wasKeptForItsFunction = (n0.flags & Kept) && n0.kept == 'U';
        const Value resultItHad = wasKeptForItsFunction ? nodeValue(n0, r.id) : Value();
        dropNode(r.id);
        const quint32 id = addNode(sheet, *c, c->formula);
        queueDependents(areaOf(m_nodes[id]));
        if (wasKeptForItsFunction && (m_nodes[id].flags & Kept) && m_nodes[id].kept == 'U') {
            // (a formula kept apart for a function the engine lacks has the result it had, #NAME? if something it read changed; and #NAME? when a cell it read is deleted)
            storeResult(m_nodes[id], id, resultItHad, false);
            if (r.now.contains(QLatin1String("#REF!")) && !r.was.contains(QLatin1String("#REF!"))) {
                std::vector<CellArea> given;
                giveName(id, &given);
            }
        }
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
    {
        Formula &f = s.formulas[c->formula];
        const qint32 old = f.text;
        f.text = internText(sheet, text);
        releaseText(sheet, old);
    }
    const quint32 id = addNode(sheet, *c, c->formula);
    queueDependents(areaOf(m_nodes[id]));
    return true;
}

Calculation::EditRecord::GoneCell Calculation::goneCell(int sheet, const Cell &c) const
{
    EditRecord::GoneCell g{sheet, c.row, c.col, content(sheet, c.row, c.col)};
    if ((c.flags & (ArrayFormula | DataTable)) && c.hasFormula() && c.formula >= 0) {
        const Sheet &s = m_sheets[size_t(sheet)];
        if (size_t(c.formula) < size_t(s.formulas.size())) {
            g.kind = quint32(c.flags & (ArrayFormula | DataTable));
            g.range = s.formulas[c.formula].range;
        }
    }
    return g;
}

// The cells that were put back as formulas are made what they were: an array formula or a data table (of one cell: those of more cells are not deleted or written over), with the range
// it had. The ranges of the sheet are as they were when this is called.
void Calculation::putBackKinds(const EditRecord &rec)
{
    for (const EditRecord::GoneCell &g : rec.gone) {
        if (!g.kind || g.range < 0 || g.sheet < 0 || g.sheet >= int(m_sheets.size())) continue;
        Sheet &s = m_sheets[size_t(g.sheet)];
        Cell *c = s.cells.findMutable(g.row, g.col);
        if (!c || !c->hasFormula() || c->formula < 0 || g.range >= s.ranges.size()) continue;
        c->flags |= quint8(g.kind);
        s.formulas[c->formula].range = g.range;
        const quint32 id = nodeAt(g.sheet, g.row, g.col);
        if (id != kNone) dropNode(id);
        const quint32 now = addNode(g.sheet, *c, c->formula);
        queueDependents(areaOf(m_nodes[now]));
        if (g.kind & ArrayFormula) markArrayMembers(g.sheet, g.row, g.col);   // (an array of more cells has its members back)
    }
}

bool Calculation::undoEdit(const EditRecord &rec, EditRecord *done)
{
    if (rec.edit.kind == StructEdit::Kind::Move) return undoMove(rec, done);
    EditRecord local;
    EditRecord &throwaway = done ? *done : local;
    if (!applyEdit(rec.edit.inverse(), &throwaway)) return false;
    BulkEdit bulk(this);
    Sheet &S = m_sheets[size_t(rec.sheet)];
    // the cells that were deleted are put back, and the formulas that were written again have the text they had
    for (const EditRecord::GoneCell &g : rec.gone) put(g.sheet, g.row, g.col, g.content, Marking::Later);
    for (const EditRecord::FormulaWas &w : rec.was) {
        const jp::calc::Value *stored = w.hasStored ? &w.stored : nullptr;
        if (!w.hasStored && w.sheet >= 0 && w.sheet < int(m_sheets.size())) {
            // (the inverse edit has most of them as they were: those that do are left alone)
            const Sheet &sw = m_sheets[size_t(w.sheet)];
            const Cell *c = sw.cells.find(w.row, w.col);
            if (c && c->hasFormula() && !(c->flags & (ArrayFormula | DataTable)) && sw.formulaText(*c) == w.text) continue;
        }
        if (rewriteArrayText(w.sheet, w.row, w.col, w.text)) continue;
        setFormula(w.sheet, w.row, w.col, w.text, Marking::Later, stored);
    }
    // the formulas of shared families have the text they had (the inverse edit has most of them so already: those that have it are left as they are)
    for (const EditRecord::RewrittenGroup &g : rec.groups) {
        if (g.sheet < 0 || g.sheet >= int(m_sheets.size())) continue;
        Sheet &sg = m_sheets[size_t(g.sheet)];
        qint32 index = -1;
        for (const auto &place : g.cells) {
            Cell *c = sg.cells.findMutable(place.first, place.second);
            if (!c || !c->hasFormula() || c->formula < 0 || size_t(c->formula) >= size_t(sg.formulas.size())) continue;
            Formula &f = sg.formulas[c->formula];
            const qint32 old = f.text;
            const bool already = old >= 0 && sg.formulaTexts[old] == g.oldText;
            if (already) continue;
            if (index < 0) index = internText(g.sheet, g.oldText);
            else holdText(g.sheet, index);
            f.text = index;
            releaseText(g.sheet, old);
            const quint32 id = nodeAt(g.sheet, place.first, place.second);
            const qint32 formula = c->formula;
            if (id != kNone) dropNode(id);
            const quint32 now = addNode(g.sheet, *c, formula);
            queueDependents(areaOf(m_nodes[now]));
        }
    }
    for (const auto &entry : rec.validations)
        if (entry.first >= 0 && entry.first < int(m_sheets.size())) {
            m_sheets[size_t(entry.first)].validations = entry.second;
            forgetRuleIndex();
        }
    for (const auto &entry : rec.conditionalFormats)
        if (entry.first >= 0 && entry.first < int(m_sheets.size())) m_sheets[size_t(entry.first)].conditionalFormats = entry.second;
    S.ranges = rec.ranges;
    putBackKinds(rec);
    S.merges = rec.merges;
    S.autoFilter = rec.autoFilter;
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

// What uses a defined name reads it again, and is worked out again.
void Calculation::nameTextsChanged()
{
    m_nameTraits.clear();
    for (SheetState &state : m_state) {
        for (Compiled &slot : state.plain) slot.tried = false;
        for (Compiled &slot : state.array) slot.tried = false;
    }
    for (quint32 id = 0; id < quint32(m_nodes.size()); ++id) {
        const Node &n = m_nodes[id];
        if ((n.flags & Dead) || !(n.flags & Told)) continue;
        Sheet &sn = m_sheets[size_t(n.sheet)];
        Cell *c = sn.cells.findMutable(n.row, n.col);
        if (!c || !c->hasFormula()) continue;
        const int sheet = n.sheet;
        const qint32 formula = c->formula;
        dropNode(id);
        addNode(sheet, *c, formula);
    }
    if (!m_running) beginRecalculation();
}

std::vector<std::pair<QString, QString>> Calculation::renameColumnInNames(const QString &table, const QString &oldName, const QString &newName)
{
    std::vector<std::pair<QString, QString>> was;
    abandonNow();
    for (auto it = m_names.begin(); it != m_names.end(); ++it) {
        if (!it.value().contains(QLatin1Char('['))) continue;
        const QString now = renameColumnInFormula(it.value(), table, oldName, newName, false);
        if (now == it.value()) continue;
        was.push_back({it.key(), it.value()});
        it.value() = now;
    }
    if (!was.empty()) nameTextsChanged();
    return was;
}

std::vector<std::pair<QString, QString>> Calculation::rewriteNames(const std::function<QString(const QString &)> &rewrite)
{
    std::vector<std::pair<QString, QString>> was;
    abandonNow();
    for (auto it = m_names.begin(); it != m_names.end(); ++it) {
        if (!it.value().contains(QLatin1Char('['))) continue;
        const QString now = rewrite(it.value());
        if (now == it.value()) continue;
        was.push_back({it.key(), it.value()});
        it.value() = now;
    }
    if (!was.empty()) nameTextsChanged();
    return was;
}

void Calculation::restoreNames(const std::vector<std::pair<QString, QString>> &names)
{
    if (names.empty()) return;
    abandonNow();
    for (const auto &entry : names) m_names[entry.first] = entry.second;
    nameTextsChanged();
}

// The rules of validation follow the edit: the cells they are for, and the cells their formulas name. The sheets whose rules changed are recorded as they were.
void Calculation::adjustValidations(const StructEdit &e, const QStringList &names, EditRecord *rec)
{
    for (int si = 0; si < int(m_sheets.size()); ++si) {
        Sheet &sheet = m_sheets[size_t(si)];
        if (sheet.validations.isEmpty()) continue;
        QVector<Validation> now;
        bool changed = false;
        for (Validation v : sheet.validations) {
            Validation was = v;
            QVector<CellRange> ranges;
            for (CellRange r : v.ranges)
                if (adjustRange(e, si, &r)) ranges.append(r);
            v.ranges = ranges;
            bool a = false, b = false;
            const CellRange first = was.ranges.isEmpty() ? CellRange() : was.ranges.first();   // (the formulas are written for its first cell)
            v.formula1 = adjustRuleFormula(v.formula1, si, names, e, first, &a);
            v.formula2 = adjustRuleFormula(v.formula2, si, names, e, first, &b);
            if (a || b || !(ranges == was.ranges)) changed = true;
            if (!v.ranges.isEmpty()) now.append(v);
        }
        if (!changed) continue;
        rec->validations.push_back({si, sheet.validations});
        sheet.validations = now;
        forgetRuleIndex();
    }
}

// The rules of conditional formatting follow the cells like the rules of validation: their ranges, and their formulas, which are written for the first cell of the first range.
void Calculation::adjustConditionalFormats(const StructEdit &e, const QStringList &names, EditRecord *rec)
{
    for (int si = 0; si < int(m_sheets.size()); ++si) {
        Sheet &sheet = m_sheets[size_t(si)];
        if (sheet.conditionalFormats.isEmpty()) continue;
        QVector<CondRule> now;
        now.reserve(sheet.conditionalFormats.size());
        bool changed = false;
        for (const CondRule &was : sheet.conditionalFormats) {
            CondRule v = was;
            QVector<CellRange> ranges;
            for (CellRange r : v.ranges)
                if (adjustRange(e, si, &r)) ranges.append(r);
            v.ranges = ranges;
            const CellRange first = was.ranges.isEmpty() ? CellRange() : was.ranges.first();
            bool any = false, one = false;
            v.formula1 = adjustRuleFormula(v.formula1, si, names, e, first, &one);
            any = any || one;
            v.formula2 = adjustRuleFormula(v.formula2, si, names, e, first, &one);
            any = any || one;
            for (CondValue &value : v.values)
                if (value.type == CondValue::Type::Formula) {
                    value.value = adjustRuleFormula(value.value, si, names, e, first, &one);
                    any = any || one;
                }
            if (any || !(ranges == was.ranges)) changed = true;
            if (!v.ranges.isEmpty()) now.append(std::move(v));
            else changed = true;
        }
        if (!changed) continue;
        rec->conditionalFormats.push_back({si, sheet.conditionalFormats});
        sheet.conditionalFormats = std::move(now);
    }
}

std::vector<ColumnSpan> Calculation::columnRuns(int sheet, quint32 first, quint32 last) const
{
    std::vector<ColumnSpan> out;
    if (sheet < 0 || sheet >= int(m_sheets.size())) return out;
    for (const ColumnSpan &c : m_sheets[size_t(sheet)].columns) {
        if (c.last < first || c.first > last) continue;
        ColumnSpan cut = c;
        cut.first = std::max(c.first, first);
        cut.last = std::min(c.last, last);
        out.push_back(cut);
    }
    std::sort(out.begin(), out.end(), [](const ColumnSpan &a, const ColumnSpan &b) { return a.first < b.first; });
    return out;
}

std::vector<RowSpec> Calculation::rowSpecsIn(int sheet, quint32 first, quint32 last) const
{
    std::vector<RowSpec> out;
    if (sheet < 0 || sheet >= int(m_sheets.size())) return out;
    for (const RowSpec &r : m_sheets[size_t(sheet)].rowSpecs)
        if (r.row >= first && r.row <= last) out.push_back(r);
    std::sort(out.begin(), out.end(), [](const RowSpec &a, const RowSpec &b) { return a.row < b.row; });
    return out;
}

void Calculation::setColumnRuns(int sheet, quint32 first, quint32 last, const std::vector<ColumnSpan> &runs)
{
    if (sheet < 0 || sheet >= int(m_sheets.size())) return;
    Sheet &S = m_sheets[size_t(sheet)];
    QVector<ColumnSpan> now;
    now.reserve(S.columns.size() + int(runs.size()) + 2);
    for (const ColumnSpan &c : S.columns) {
        if (c.last < first || c.first > last) {
            now.append(c);
            continue;
        }
        if (c.first < first) {
            ColumnSpan left = c;
            left.last = first - 1;
            now.append(left);
        }
        if (c.last > last) {
            ColumnSpan right = c;
            right.first = last + 1;
            now.append(right);
        }
    }
    for (const ColumnSpan &c : runs) now.append(c);
    std::stable_sort(now.begin(), now.end(), [](const ColumnSpan &a, const ColumnSpan &b) { return a.first < b.first; });
    S.columns = now;
}

void Calculation::setRowSpecs(int sheet, quint32 first, quint32 last, const std::vector<RowSpec> &specs)
{
    if (sheet < 0 || sheet >= int(m_sheets.size())) return;
    Sheet &S = m_sheets[size_t(sheet)];
    QVector<RowSpec> now;
    now.reserve(S.rowSpecs.size() + int(specs.size()));
    for (const RowSpec &r : S.rowSpecs)
        if (r.row < first || r.row > last) now.append(r);
    for (const RowSpec &r : specs) now.append(r);
    std::stable_sort(now.begin(), now.end(), [](const RowSpec &a, const RowSpec &b) { return a.row < b.row; });
    S.rowSpecs = now;
}

} // namespace jp::office::sheet
