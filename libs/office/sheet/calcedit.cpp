// Rows, columns, and cells inserted and deleted in a calculation: the cells move, and every formula that reads cells that moved is written again with its references following them.
// See Calculation::applyEdit.
#include "sheet/calculation.h"

#include "calc/idframe.h"
#include "sheet/tables.h"

#include <QCoreApplication>

#include <algorithm>
#include <chrono>
#include <set>
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
    if (s.isProtected) {
        // a protected sheet lets rows or columns be inserted or deleted if its protection allows it (and deleted only if none of the cells in them is locked)
        const SheetProtection &p = s.protection;
        bool allowed = false;
        switch (e.kind) {
        case K::InsertRows: allowed = p.allowInsertRows; break;
        case K::InsertColumns: allowed = p.allowInsertColumns; break;
        case K::DeleteRows: allowed = p.allowDeleteRows; break;
        case K::DeleteColumns: allowed = p.allowDeleteColumns; break;
        default: break;
        }
        if (!allowed) return refuse(say(QT_TRANSLATE_NOOP("jp::office::sheet::Calculation", "This sheet is protected, and its protection doesn't allow rows, columns, or cells to be inserted or deleted this way. Take the protection off first.")));
        if (e.kind == K::DeleteRows || e.kind == K::DeleteColumns) {
            const CellArea gone = affectedArea(e);
            bool locked = false;
            s.cells.forEachInRange(gone.row1, gone.col1, gone.row2, gone.col2, [&](quint32, quint32, const Cell &c) {
                locked = m_book.styles().format(c.style).locked;
                return !locked;
            });
            if (locked) return refuse(say(QT_TRANSLATE_NOOP("jp::office::sheet::Calculation", "This sheet is protected, and some of the cells in what you are deleting are locked, so it can't be deleted. Take the protection off first.")));
        }
    }
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
            const Cell *c = cellOf(n);
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
            s.cells.forEachInRange(firstGone, e.col1, kGridRows - 1, e.col2, [&](quint32, quint32, const Cell &c) {
                if (c.type != CellType::Empty || c.hasFormula()) pushed = true;
                return !pushed;
            });
        } else {
            s.cells.forEachInRange(e.row1, firstGone, e.row2, kGridCols - 1, [&](quint32, quint32, const Cell &c) {
                if (c.type != CellType::Empty || c.hasFormula()) pushed = true;
                return !pushed;
            });
        }
        if (e.kind == K::InsertRows && pushed) {
            // (the extent says there are cells at the end: look for one that holds something)
            pushed = false;
            s.cells.forEachInRange(firstGone, 0, kGridRows - 1, kGridCols - 1, [&](quint32, quint32, const Cell &c) {
                if (c.type != CellType::Empty || c.hasFormula()) pushed = true;
                return !pushed;
            });
        } else if (e.kind == K::InsertColumns && pushed) {
            pushed = false;
            s.cells.forEach([&](quint32, quint32 col, const Cell &c) {
                if (col >= firstGone && (c.type != CellType::Empty || c.hasFormula())) pushed = true;
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
    // (id frame) An edit of whole rows or columns holds the formulas it writes again, which are those held in places and those told where they read (the rest are not written again), and for a delete
    // that keeps its cells (no array, data table, or formula held in places in its lines) the cells it leaves dead, at the figure the step counts them at.
    const bool wholeRows = e.kind == K::InsertRows || e.kind == K::DeleteRows;
    if (m_options.idFrame && (wholeRows || e.kind == K::InsertColumns || e.kind == K::DeleteColumns)) {
        bytes = (qint64(m_positionalNodes.size()) + qint64(m_special.size())) * kPerFormula;
        if (e.kind == K::InsertRows || e.kind == K::InsertColumns) return bytes;
        if (!deadLinesHoldSpecials(e, e.count)) return bytes + m_sheets[size_t(e.sheet)].cells.cellsInLines(wholeRows, e.first, e.count) * kUndoBytesPerDeadCell;
        bytes = 0;   // (it copies the cells, as before ids: counted below)
    }
    auto countCells = [&](int sheet, quint32 r1, quint32 c1, quint32 r2, quint32 c2) {
        if (sheet < 0 || sheet >= int(m_sheets.size())) return;
        m_sheets[size_t(sheet)].cells.forEachInRange(r1, c1, r2, c2, [&](quint32, quint32, const Cell &) {
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
    readersOf(region, &readers);
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
            if (!(n.flags & Dead) && (((n.flags & Told) && !(n.traits & IdTold)) || ((n.flags & Kept) && n.kept != 'U'))) m_special[keep++] = id;
        }
        m_special.resize(keep);
    }
    for (const quint32 id : m_special) {
        const Node &n = m_nodes[id];
        if (!(n.flags & Dead) && (((n.flags & Told) && !(n.traits & IdTold)) || ((n.flags & Kept) && n.kept != 'U'))) out->push_back(id);
    }
}

void Calculation::setGraphShifts(bool on) { g_graphShifts = on; }

namespace {
bool sameColumns(const QVector<ColumnSpan> &a, const QVector<ColumnSpan> &b)
{
    if (a.size() != b.size()) return false;
    for (int i = 0; i < a.size(); ++i) {
        const ColumnSpan &x = a[i], &y = b[i];
        if (x.first != y.first || x.last != y.last || x.width != y.width || x.hidden != y.hidden || x.customWidth != y.customWidth || x.style != y.style || x.outline != y.outline || x.collapsed != y.collapsed
            || x.bestFit != y.bestFit || x.phonetic != y.phonetic)
            return false;
    }
    return true;
}

bool wholeLines(const StructEdit &e, bool *rows)
{
    using K = StructEdit::Kind;
    *rows = e.kind == K::InsertRows || e.kind == K::DeleteRows;
    return *rows || e.kind == K::InsertColumns || e.kind == K::DeleteColumns;
}
} // namespace

bool Calculation::linesNeedCompaction(const StructEdit &e) const
{
    bool rows = false;
    if (!wholeLines(e, &rows) || e.sheet < 0 || e.sheet >= int(m_sheets.size())) return false;
    return m_sheets[size_t(e.sheet)].cells.needsCompaction(rows, e.count);
}

bool Calculation::compactLines(int sheet, bool rows, std::vector<jp::calc::LineMapCore::Renumber> *out)
{
    ++m_structStamp;
    if (sheet < 0 || sheet >= int(m_sheets.size())) return false;
    finishPurges(sheet);   // (the dead cells that are waiting to be let go of, and their nodes, are numbered by the ids that are renumbered below)
    abandonNow();
    // The lines that are dead now go with the renumbering (every step that could bring them back is stale after it: Undo of it is refused), and so do the cells in them. The nodes of the formulas in those
    // cells, which a delete that kept them left registered (a caller that has not forgotten the step, as the session does, may not have let them go of), are finished as a purge finishes them: dropped, with
    // their formulas; else they would be left with a line that has no number.
    if (m_dormantLines > 0) {
        const CellStore &cs = m_sheets[size_t(sheet)].cells;
        std::vector<quint32> gone;
        for (quint32 id = 0; id < quint32(m_nodes.size()); ++id) {
            const Node &n = m_nodes[id];
            if ((n.flags & Dead) || n.sheet != sheet) continue;
            if ((rows ? cs.rowPlaceOf(n.rowId) : cs.colPlaceOf(n.colId)) == CellStore::kNoLine) gone.push_back(id);
        }
        SheetState &st = m_state[size_t(sheet)];
        for (const quint32 id : gone) {
            const qint32 formula = m_nodes[id].formula;
            dropNode(id);
            if (formula >= 0 && size_t(formula) < st.nodeOf.size()) releaseFormula(sheet, formula);
        }
    }
    // what is waiting for the worker names ids that are renumbered below: it is done now, with the ids it has (the formulas it names are marked to be worked out, and what reads them)
    const bool drained = drainBeforeRenumbering(sheet);
    std::vector<jp::calc::LineMapCore::Renumber> table;
    // (the order as it is, with the records of the lines that were deleted: formulas held in ids may name them)
    const jp::calc::LineMapCore before = rows ? m_sheets[size_t(sheet)].cells.rows().core() : m_sheets[size_t(sheet)].cells.cols().core();
    if (m_sheets[size_t(sheet)].cells.rebaseLines(rows, &table) != jp::calc::LineStatus::Ok) return false;
    if (out) *out = table;
    if (table.empty()) {
        if (drained && !m_running) beginRecalculation();
        return true;   // (the order was one run: the ids are the places they were)
    }
    // the ids the nodes hold are renumbered with the cells, and what they worked out and told before is named by the old ids: every value of an epoch before this one is let go of (staleResult: the
    // cells of an array are named too, and no stamp is made for each), and, if any is in flight, the formulas are worked out again
    ++m_state[size_t(sheet)].epoch;
    m_state[size_t(sheet)].renumberEpoch = m_state[size_t(sheet)].epoch;
    const bool again = resultsInFlight();
    {
        std::lock_guard<std::mutex> guard(m_stampLock);
        for (auto it = m_stamps.begin(); it != m_stamps.end();)
            if (int(it->first >> 40) == sheet) it = m_stamps.erase(it);
            else ++it;
        m_stampCount.store(m_stamps.size(), std::memory_order_relaxed);
    }
    for (quint32 id = 0; id < quint32(m_nodes.size()); ++id) {
        Node &n = m_nodes[id];
        if ((n.flags & Dead) || n.sheet != sheet) continue;
        const quint32 now = CellStore::renumberLine(table, rows ? n.rowId : n.colId);
        Q_ASSERT(now != CellStore::kNoLine);
        if (rows) n.rowId = now;
        else n.colId = quint16(now);
        if (again && !(n.flags & Kept)) redirty(id);
    }
    // the specs of rows are held by the ids of the rows: they are renumbered with them (those of rows that are gone go), and what the worker knows of the hidden rows is by the old ids
    if (rows && m_sheets[size_t(sheet)].rowSpecsAreIds) {
        QVector<RowSpec> kept;
        kept.reserve(m_sheets[size_t(sheet)].rowSpecs.size());
        for (RowSpec r : std::as_const(m_sheets[size_t(sheet)].rowSpecs)) {
            r.row = CellStore::renumberLine(table, r.row);
            if (r.row != CellStore::kNoLine) kept.append(r);
        }
        std::stable_sort(kept.begin(), kept.end(), [](const RowSpec &a, const RowSpec &b) { return a.row < b.row; });
        m_sheets[size_t(sheet)].rowSpecs = kept;
        hiddenRowsChanged(sheet);
    }
    // the texts in ids that name the lines are written again with the new ids
    renumberTexts(sheet, rows, table, before);
    // the graph holds the old ids too, and so do the lists of the formulas that have an inserted line at an end (rebuildGraph makes them again: no line is inserted now)
    m_readersOfInserted.clear();
    m_insertedEndsOf.clear();
    const bool told = rebuildGraph();
    if ((again || told || drained) && !m_running) beginRecalculation();
    return true;
}

void Calculation::renumberTexts(int target, bool rows, const std::vector<jp::calc::LineMapCore::Renumber> &table, const jp::calc::LineMapCore &before, int onlySheet)
{
    if (!m_options.idFrame || table.empty()) return;
    using jp::calc::ParseLimits;
    // One reference as the new ids say it: the sheet whose ids changed has its lines renumbered, and a line that was deleted is the line next to where it was in the order before.
    const auto mapLine = [&](int sheet, jp::calc::RefLit *lit) {
        MapResult result = MapResult::Same;
        if (sheet == target) {
            const bool whole = lit->flags & (rows ? jp::calc::kRefWholeCols : jp::calc::kRefWholeRows);
            if (!whole) {
                // (the ends as the text names them; one that was deleted is the line next to where it was, by the order before the renumbering, and nothing may be left of the reference)
                quint32 a, b;
                bool aAbs, bAbs;
                jp::calc::refEnds(*lit, rows, &a, &b, &aAbs, &bAbs);
                quint32 ia = a, ib = b;
                if (before.posOf(a) == CellStore::kNoLine) ia = before.resolveStart(a);
                if (before.posOf(b) == CellStore::kNoLine) ib = before.resolveEnd(b);
                if (ia == CellStore::kNoLine || ib == CellStore::kNoLine) return MapResult::Gone;
                if ((ia != a || ib != b) && before.posOf(ia) > before.posOf(ib)) return MapResult::Gone;
                const quint32 x = CellStore::renumberLine(table, ia), y = CellStore::renumberLine(table, ib);
                if (x == CellStore::kNoLine || y == CellStore::kNoLine) return MapResult::Gone;
                if (x != a || y != b || (lit->written & (rows ? 1 : 2))) result = MapResult::Changed;
                jp::calc::setRefEnds(lit, rows, x, y, aAbs, bAbs);
            }
            // (the other kind of line keeps the order its text names the ends in, which the parser put in the order of the numbers)
            quint32 s0, e0;
            bool sa, ea;
            jp::calc::refEnds(*lit, !rows, &s0, &e0, &sa, &ea);
            jp::calc::setRefEnds(lit, !rows, s0, e0, sa, ea);
        }
        return result;
    };
    for (int si = 0; si < int(m_sheets.size()); ++si) {
        if (onlySheet >= 0 && si != onlySheet) continue;
        Sheet &s = m_sheets[size_t(si)];
        if (!s.frame || s.formulaTexts.isEmpty() || s.formulas.isEmpty()) continue;
        countTexts(si);
        // the formulas that have each text, and, for the sheet whose ids changed, the ids of the cell each is in (now)
        std::vector<std::vector<qint32>> members(size_t(s.formulaTexts.size()));
        for (qint32 fi = 0; fi < s.formulas.size(); ++fi) {
            const qint32 t = s.formulas[fi].text;
            if (t >= 0 && t < s.formulaTexts.size() && !s.positionalTexts.contains(t)) members[size_t(t)].push_back(fi);
        }
        std::vector<quint32> cellLine;
        if (si == target) {
            cellLine.assign(size_t(s.formulas.size()), CellStore::kNoLine);
            s.cells.forEach([&](quint32, quint32, const Cell &c) {
                if (c.hasFormula() && c.formula >= 0 && c.formula < s.formulas.size()) cellLine[size_t(c.formula)] = rows ? c.rowId : c.colId;
            });
        }
        for (qint32 t = 0; t < qint32(members.size()); ++t) {   // (the texts made below are not looked at)
            const std::vector<qint32> &users = members[size_t(t)];
            if (users.empty()) continue;
            const QString text = s.formulaTexts[t];
            const ParseLimits was = limitsOf(s, t);
            // the text of one formula in ids, as the new ids say it; whether it has an id past the grid
            auto renumbered = [&](qint32 fi, QString *out, bool *wide) {
                const Formula &f = s.formulas[fi];
                const QString own = f.dRow == 0 && f.dCol == 0 ? text : offsetFormula(text, f.dRow, f.dCol, nullptr, was);
                *wide = false;
                const auto map = [&](int sheet, jp::calc::RefLit *lit) {
                    const MapResult result = mapLine(sheet, lit);
                    if (result == MapResult::Gone) return result;
                    *wide = *wide || lit->row1 >= jp::calc::kGridRows || lit->row2 >= jp::calc::kGridRows || lit->col1 >= jp::calc::kGridCols || lit->col2 >= jp::calc::kGridCols;
                    return result;
                };
                return mapReferences(*this, si, own, was, map, out);
            };
            QString first;
            bool firstWide = false;
            qint32 origin = users.front();
            if (!renumbered(origin, &first, &firstWide)) continue;
            const ParseLimits now = firstWide ? ParseLimits::ids() : ParseLimits::grid();
            const Formula f0 = s.formulas[origin];
            for (const qint32 fi : users) {
                const Formula f = s.formulas[fi];
                // how far the formula is from the origin's cell, in the new ids
                qint32 dRow = f.dRow - f0.dRow, dCol = f.dCol - f0.dCol;
                if (si == target && cellLine[size_t(fi)] != CellStore::kNoLine && cellLine[size_t(origin)] != CellStore::kNoLine) {
                    const qint32 d = qint32(qint64(cellLine[size_t(fi)]) - qint64(cellLine[size_t(origin)]));
                    (rows ? dRow : dCol) = d;
                }
                QString mine;
                bool mineWide = false;
                const bool ok = renumbered(fi, &mine, &mineWide);
                if (!ok) continue;
                const QString family = dRow == 0 && dCol == 0 ? first : offsetFormula(first, dRow, dCol, nullptr, now);
                const bool shared = family == mine && mineWide == firstWide;
                const TextClass cls = (shared ? firstWide : mineWide) ? TextClass::Wide : TextClass::Plain;
                const qint32 old = s.formulas[fi].text;
                s.formulas[fi].text = internText(si, shared ? first : mine, cls);
                s.formulas[fi].dRow = shared ? dRow : 0;
                s.formulas[fi].dCol = shared ? dCol : 0;
                releaseText(si, old);
            }
        }
    }
    // the defined names held in ids
    if (onlySheet < 0) {
        for (auto it = m_names.begin(); it != m_names.end(); ++it) {
            if (!m_idNames.contains(it.key())) continue;
            const int bar = int(it.key().indexOf(QLatin1Char('|')));
            const int scope = bar > 0 ? it.key().left(bar).toInt() : -1;
            QString now;
            if (mapReferences(*this, scope >= 0 ? scope : 0, it.value(), ParseLimits::ids(), mapLine, &now)) it.value() = now;
        }
    }
}

// The graph holds ids, and the ids of a sheet's lines were renumbered: every formula tells it again what it reads, by the places its text names. What a formula that reads where it is told
// touched as it was worked out is told again when it is worked out, which it is made to be.
// Questions, queued changes, and blocks left for another sheet's turn name ids; before the ids of a sheet are renumbered they are answered: the formulas they name are marked to be worked out, and
// what reads those, through the graph as it is now. True if there was anything.
bool Calculation::drainBeforeRenumbering(int sheet)
{
    bool any = false;
    std::vector<CellArea> changed;
    for (size_t i = 0; i < m_questions.size();) {
        const Question &q = m_questions[i];
        if (q.sheet != sheet) {
            ++i;
            continue;
        }
        any = true;
        std::vector<quint32> asked;
        buildAsked(q, &asked, nullptr);
        for (const quint32 node : asked) markAsked(node, q, &changed);
        if (q.observers)
            for (const auto &entry : m_seen) markAsked(entry.first, q, &changed);
        if (q.ownOnly)
            for (quint32 id = 0; id < quint32(m_nodes.size()); ++id) {
                const Node &n = m_nodes[id];
                if ((n.traits & OwnOnly) && !(n.flags & Dead) && n.sheet == q.sheet && (q.rows ? rowOf(n) : colOf(n)) >= q.ownFirst) markAsked(id, q, &changed);
            }
        m_questions.erase(m_questions.begin() + std::ptrdiff_t(i));
        m_begin.built = false;
        m_begin.asked.clear();
        m_begin.observers.clear();
    }
    if (!m_begin.changed.empty()) {
        changed.insert(changed.end(), m_begin.changed.begin(), m_begin.changed.end());
        m_begin.changed.clear();
        any = true;
    }
    if (!m_deferredAreas.empty()) {
        changed.insert(changed.end(), m_deferredAreas.begin(), m_deferredAreas.end());
        m_deferredAreas.clear();
        any = true;
    }
    if (!changed.empty()) markDependentsOfIds(&changed);
    return any;
}

bool Calculation::rebuildGraph()
{
    bool worked = false;
    for (quint32 id = 0; id < quint32(m_nodes.size()); ++id) {
        Node &n = m_nodes[id];
        if (n.flags & Dead) continue;
        // (a formula kept apart after it was worked out, a circle or one that read what the engine lacks, has entries the others kept apart never made: all are told again)
        const Sheet &s = m_sheets[size_t(n.sheet)];
        if (n.formula < 0 || n.formula >= s.formulas.size() || s.formulas[n.formula].text < 0 || s.formulas[n.formula].text >= s.formulaTexts.size()) continue;
        m_toldHash[id] = 0;
        registerNode(id, nullptr);
        if ((n.flags & Told) && !(n.flags & Kept)) {
            redirty(id);
            worked = true;
        }
    }
    return worked;
}

void Calculation::setLineLimitsForTest(int sheet, const jp::calc::LineLimits &rows, const jp::calc::LineLimits &cols)
{
    if (sheet >= 0 && sheet < int(m_sheets.size())) m_sheets[size_t(sheet)].cells.setLineLimits(rows, cols);
}

void Calculation::forgetLines(EditRecord *rec)
{
    bool rows = false;
    std::vector<EditRecord::GoneCell>().swap(rec->gone);   // (the copy of the cells it deleted: nothing can bring them back now)
    if (!wholeLines(rec->edit, &rows)) return;
    if (rec->dormant && rec->lines.made) {
        // (the cells of the lines that were deleted are still there, with their formulas and the nodes of those: they are let go of one slice after another, now)
        forgetLinesLater(rec);
        finishPurges();
        return;
    }
    // (the sheet that has the order the record names: the sheets may have been moved since)
    for (Sheet &s : m_sheets)
        if (s.cells.serial() == rec->lines.owner) {
            s.cells.forgetLines(rows, &rec->lines, m_options.idFrame);
            return;
        }
    rec->lines = CellStore::LineEdits();
}

// Whether the lines (places `first` to `first + count - 1`) a delete of whole rows or columns takes out hold a formula whose text is held in places (what an edit writes again is not written for a
// formula in a dead line, which would then come back with Undo as it was before the edits after it), or a cell of an array or a data table.
bool Calculation::deadLinesHoldSpecials(const StructEdit &e, quint32 count) const
{
    const bool rows = e.kind == StructEdit::Kind::DeleteRows;
    const quint32 first = e.first, last = e.first + count - 1;
    for (const quint32 id : m_positionalNodes) {
        const Node &n = m_nodes[id];
        if ((n.flags & Dead) || n.sheet != e.sheet) continue;
        const quint32 place = rows ? rowOf(n) : colOf(n);
        if (place != CellStore::kNoLine && place >= first && place <= last) return true;
    }
    for (const CellRange &r : m_sheets[size_t(e.sheet)].ranges)
        if (r.valid && (rows ? !(r.row2 < first || r.row1 > last) : !(r.col2 < first || r.col1 > last))) return true;
    return false;
}

bool Calculation::applyEdit(const StructEdit &e, EditRecord *rec) { return applyEditImpl(e, rec, nullptr); }

// ---- letting go of the dead cells (and the nodes of their formulas) of a delete that kept them: when its step is forgotten, a few blocks at a time
void Calculation::forgetLinesLater(EditRecord *rec)
{
    bool rows = false;
    if (!rec->dormant || !wholeLines(rec->edit, &rows) || !rec->lines.made) {
        forgetLines(rec);
        return;
    }
    // (the cells of the dead lines are the ones to let go of, in slices; what the order holds of them is let go of when the last is gone)
    std::vector<EditRecord::GoneCell>().swap(rec->gone);
    PurgeJob job;
    job.sheet = -1;
    for (int i = 0; i < int(m_sheets.size()); ++i)
        if (m_sheets[size_t(i)].cells.serial() == rec->lines.owner) job.sheet = i;
    if (job.sheet < 0) {
        --m_dormantLines;   // (its sheet is gone: so are its cells)
        rec->lines = CellStore::LineEdits();
        return;
    }
    job.rows = rows;
    job.runs = rec->lines.edit.runs;
    job.lines = std::move(rec->lines);
    job.keepDead = m_options.idFrame;
    rec->lines = CellStore::LineEdits();
    m_purges.push_back(std::move(job));
}

qint64 Calculation::purgeWaiting() const
{
    qint64 n = 0;
    std::set<int> seen;
    for (const PurgeJob &j : m_purges)
        if (seen.insert(j.sheet).second) n += m_sheets[size_t(j.sheet)].cells.deadCount();
    return n;
}

bool Calculation::purgeSome(qint64 budget, PurgeStep *step)
{
    *step = PurgeStep();
    if (m_purges.empty()) return false;
    PurgeJob &job = m_purges.front();
    // (the sheet is found by the order it has: the sheets may have been moved or taken away since)
    int sheet = -1;
    for (int i = 0; i < int(m_sheets.size()); ++i)
        if (m_sheets[size_t(i)].cells.serial() == job.lines.owner) sheet = i;
    if (sheet < 0) {
        --m_dormantLines;
        m_purges.pop_front();
        step->finished = true;
        return true;
    }
    Sheet &S = m_sheets[size_t(sheet)];
    SheetState &st = m_state[size_t(sheet)];
    step->sheet = sheet;
    bool abandoned = false;   // (the worker's lists are put right once, by the first formula that is dropped: a slice of cells with none does not touch them)
    qint64 dropped = 0;
    const bool done = S.cells.purgeSome(job.rows, job.runs, &job.cursor, budget, &step->work, [&](const Cell &c) {
        if (!c.hasFormula() || c.formula < 0) return;
        step->formulaCells.push_back({c.rowId, quint32(c.colId)});
        if (size_t(c.formula) < st.nodeOf.size() && st.nodeOf[size_t(c.formula)] != kNone) {
            if (!abandoned) {
                abandonNow();
                abandoned = true;
            }
            dropNode(st.nodeOf[size_t(c.formula)]);
            ++dropped;
        }
        releaseFormula(sheet, c.formula);   // (nothing has the formula of a cell that is gone)
    });
    m_editWork += step->work;
    if (!done) return true;
    // the cells are gone: the order lets go of the lines (their ids are given back if they were the last taken), and the specs of the rows that went with them
    S.cells.finishForgetLines(job.rows, &job.lines, job.keepDead);
    if (job.rows && S.rowSpecsAreIds) {
        QVector<RowSpec> kept;
        kept.reserve(S.rowSpecs.size());
        for (const RowSpec &r : std::as_const(S.rowSpecs))
            if (S.cells.rowIdKnown(r.row)) kept.append(r);
        if (kept.size() != S.rowSpecs.size()) S.rowSpecs = kept;
    }
    step->finished = true;
    step->rows = job.rows;
    step->runs = job.runs;
    --m_dormantLines;
    m_purges.pop_front();
    return true;
}

void Calculation::finishPurges(int sheet)
{
    PurgeStep step;
    while (!m_purges.empty()) {
        if (sheet >= 0 && std::none_of(m_purges.begin(), m_purges.end(), [&](const PurgeJob &j) { return j.sheet == sheet; })) return;
        purgeSome(qint64(1) << 40, &step);
    }
}

bool Calculation::applyEditImpl(const StructEdit &edit, EditRecord *rec, const EditRecord *undoOf)
{
    ++m_structStamp;
    using K = StructEdit::Kind;
    const StructEdit e = edit;   // (a copy: it may be the edit of the record, which is made again below)
    if (!canApply(e, nullptr)) return false;
    if (e.kind == K::Move) return applyMove(e, rec);
    abandonNow();
    // (the ids the tables stand for now, to see after the edit which tables it changed)
    bool linesEdit = false;
    if (m_options.idFrame && wholeLines(e, &linesEdit) && !m_book.tables().isEmpty()) snapshotTableIds();
    auto lapAt = std::chrono::steady_clock::now();
    auto lap = [&](int phase) {
        const auto now = std::chrono::steady_clock::now();
        m_editStats.phaseMicros[phase] += std::chrono::duration_cast<std::chrono::microseconds>(now - lapAt).count();
        lapAt = now;
    };
    const bool wholeLine = e.kind == K::InsertRows || e.kind == K::DeleteRows || e.kind == K::InsertColumns || e.kind == K::DeleteColumns;
    // (an edit of cells makes the epoch one more; an edit of whole lines does if it is not rigid for some formula, which is known when it is made)
    if (!wholeLine) ++m_state[size_t(e.sheet)].epoch;
    // (a record that is given with the lines of an edit that was undone is for that edit done again: its rows or columns come back as they were, and that needs no ids)
    CellStore::LineEdits again;
    bool redoing = false;
    if (!undoOf && rec->lines.made == false && rec->lines.edit.runs.size() && rec->edit.kind == e.kind && rec->edit.sheet == e.sheet && rec->edit.first == e.first && rec->edit.count == e.count) {
        again = rec->lines;
        redoing = true;
    }
    *rec = EditRecord();
    rec->edit = e;
    rec->sheet = e.sheet;
    if (m_options.idFrame) rec->renumberEpochs = renumberEpochs();
    Sheet &S = m_sheets[size_t(e.sheet)];
    // (what the edit changes of the sheet's other structures is kept as it was: the lists are shared with these copies, which cost nothing, and a list that the edit does not change is not made again)
    const QVector<CellRange> rangesBefore = S.ranges, mergesBefore = S.merges;
    const AutoFilter filterBefore = S.autoFilter;
    const qint64 mergeCountBefore = S.mergeCount;
    const QVector<ColumnSpan> columnsBefore = S.columns;
    const QVector<RowSpec> rowSpecsBefore = S.rowSpecs;
    rec->extentRows = S.extentRows;
    rec->extentCols = S.extentCols;
    rec->rowsSeen = m_rows[size_t(e.sheet)];
    rec->colsSeen = m_cols[size_t(e.sheet)];
    QStringList names;
    for (const SheetEntry &se : m_book.info().sheets) names << se.name;

    lap(6);
    // ---- what reads the cells that move: the graph says (before anything moves), and the formulas the graph does not follow (those kept apart, those that read where they are told)
    const CellArea region = affectedArea(e);
    std::vector<quint32> candidates, specials, oracleReaders;
    // With the id frame an edit of whole lines writes no text again for a formula that is held in ids: what it names is the same ids, which are where they are now, and one with an end in a line that
    // died reads the line next to it (docs/jeffcrunch-rowids.md, 4.4). The formulas it looks at are those held in places and those the graph does not follow (kept apart, told where they read).
    const bool idLines = m_options.idFrame && wholeLine;
    if (idLines) {
        // (a formula kept apart for a function the engine lacks can't be told what the function makes of a text it reads: it has #NAME? when the text of a formula it reads is written again. So while the
        // workbook has one, every formula that reads what moves is looked at, and the ones whose text as shown changed are as good as written again)
        if (m_hasUnknown) readersOf(region, &candidates);
        // (the oracle, an edit that asks no question of the worker: every formula that reads what moves or dies is worked out again, which the questions do otherwise)
        if (!g_graphShifts.load(std::memory_order_relaxed)) readersOf(region, &oracleReaders);
        for (const quint32 id : m_positionalNodes) candidates.push_back(id);
    } else {
        readersOf(region, &candidates);
    }
    specialNodes(&specials);
    candidates.insert(candidates.end(), specials.begin(), specials.end());
    std::sort(candidates.begin(), candidates.end());
    candidates.erase(std::unique(candidates.begin(), candidates.end()), candidates.end());
    lap(0);
    m_editWork += qint64(candidates.size()) + qint64(m_special.size());
    const qint64 graphWorkBefore = m_graph->buildWork() + m_graph->queryWork();
    ++m_editStats.edits;
    m_editStats.formulasVisited += qint64(candidates.size());
    struct Rewrite {
        quint32 id;
        QString was, now;   // the text of the formula for its cell, and what it is to be (empty: as it was)
        bool shownOnly = false;   // (id frame) its text is held as it was; what it is shown as is looked at again when the edit is made, and `now` is that if it changed
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
        if ((n.flags & Dead) || dormant(n)) continue;   // (a formula in a line that is deleted is as it was when the line went: nothing is written for it, and Undo brings the line back)
        const Sheet &sn = m_sheets[size_t(n.sheet)];
        const Cell *c = cellOf(n);
        if (!c || !c->hasFormula()) continue;
        Rewrite r;
        r.id = id;
        if (idLines && c->formula >= 0 && size_t(c->formula) < size_t(sn.formulas.size()) && inIds(sn, sn.formulas[c->formula].text)) {
            if (m_hasUnknown) {
                r.shownOnly = true;
                r.was = sn.formulaText(*c);
            }
            rewrites.push_back(std::move(r));   // (its text is as it is: the ones among these that are told where they read are made again below)
            continue;
        }
        if (rowsOrColumns && !m_options.idFrame && !(c->flags & (ArrayFormula | DataTable)) && !(n.flags & Kept) && c->formula >= 0 && size_t(c->formula) < size_t(sn.formulas.size())) {
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
                        if (!g.newText.isEmpty()) ++m_editStats.textsMade;
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
        else ++m_editStats.textsMade;
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
    for (const Rewrite &r : rewrites) places.push_back({r.id, m_nodes[r.id].sheet, rowOf(m_nodes[r.id]), colOf(m_nodes[r.id])});

    lap(1);
    // ---- the cells that are deleted, as they are
    const quint32 count = shiftBy(e);
    // (id frame) Whole rows or columns deleted keep their cells where they are, dead, with their formulas and the nodes of those (docs/jeffcrunch-rowids.md, 4.7): no cell is copied, and Undo brings them back
    // as they are. A formula whose text is in places, and an array or a data table, can't be left so (what is written again for the edits after would not be written for them), and the edit that has
    // one in its lines copies the cells, as it did before.
    const bool dormantDelete = idLines && !undoOf && (e.kind == K::DeleteRows || e.kind == K::DeleteColumns) && !deadLinesHoldSpecials(e, count);
    if (deletes(e) && !dormantDelete) {
        quint32 r1 = 0, c1 = 0, r2 = kGridRows - 1, c2 = kGridCols - 1;
        switch (e.kind) {
        case K::DeleteRows: r1 = e.first; r2 = e.first + count - 1; break;
        case K::DeleteColumns: c1 = e.first; c2 = e.first + count - 1; break;
        case K::DeleteCellsUp: r1 = e.row1; r2 = e.row2; c1 = e.col1; c2 = e.col2; break;
        default: r1 = e.row1; r2 = e.row2; c1 = e.col1; c2 = e.col2; break;
        }
        S.cells.forEachInRange(r1, c1, r2, c2, [&](quint32 row, quint32 col, const Cell &c) {
            pushGone(rec, e.sheet, row, col, c);
            return true;
        });
    } else if (e.kind == K::InsertRows || e.kind == K::InsertColumns) {
        // the rows (columns) that are pushed off the end of the sheet are deleted: what is in them is kept as a deleted cell's is, to be put back
        const bool rowsEdit = e.kind == K::InsertRows;
        const quint32 limit = rowsEdit ? kGridRows : kGridCols;
        const quint32 from = std::max(e.first, limit - std::min(count, limit));
        S.cells.forEachInRange(rowsEdit ? from : 0, rowsEdit ? 0 : from, kGridRows - 1, kGridCols - 1, [&](quint32 row, quint32 col, const Cell &c) {
            pushGone(rec, e.sheet, row, col, c);
            return true;
        });
    }

    // ---- the cells move; the nodes of the formulas among them go with them
    std::vector<PlacedCell> removed;
    SheetState &st = m_state[size_t(e.sheet)];
    // (for an edit of whole lines: the ids of the lines it put in, killed, and brought back, for what the worker is asked below)
    struct LineChange {
        bool rows = true;
        std::vector<jp::calc::IdRun> inserted, killed, revived;
    } lc;
    const qint64 deadBefore = S.cells.deadCount();
    // (cells shifted one by one get new ids: a formula among them has its node told which cell it is in, the window's value for it is moved, and a value told for it before, if one is in flight, is
    // let go of and the formula worked out again then; nothing is worked out again for a formula that has no value on its way)
    const bool stamping = resultsInFlight();   // (the worker is held: what is in flight does not change while the cells move)
    bool ownMoved = false;   // a formula that sees only its own cell was moved by an edit of cells
    auto onFormulaMoved = [&](const PlacedCell &before, const PlacedCell &after) {
        rec->shifted.push_back({before.cell.rowId, quint32(before.cell.colId), after.cell.rowId, quint32(after.cell.colId)});
        if (stamping) {
            const Target redo{after.cell.rowId, quint32(after.cell.colId), true};
            stampCell(e.sheet, before.cell.rowId, before.cell.colId, redo);
            stampCell(e.sheet, after.cell.rowId, after.cell.colId, redo);
        }
        if (!after.cell.hasFormula() || after.cell.formula < 0 || size_t(after.cell.formula) >= st.nodeOf.size()) return;
        const quint32 id = st.nodeOf[size_t(after.cell.formula)];
        if (id == kNone) return;
        m_nodes[id].rowId = after.cell.rowId;
        m_nodes[id].colId = after.cell.colId;
        // (a formula that sees where cells are, for which the places it kept were of the ids it had: it is looked at again)
        if (m_nodes[id].traits & OwnOnly) {
            ownMoved = true;
            noteSeenPlace(e.sheet, after.row, after.col);
        }
        if (m_nodes[id].traits & Observes) {
            const auto seen = m_seen.find(id);
            if (seen != m_seen.end()) setSeenAlways(seen->second, true);
        }
    };
    auto onMoved = [&](const PlacedCell &before, const PlacedCell &after) {
        ++rec->cellsMoved;
        if (after.cell.hasFormula() || (after.cell.flags & ArrayMember)) onFormulaMoved(before, after);
    };
    switch (e.kind) {
    case K::InsertRows:
    case K::DeleteRows:
    case K::InsertColumns:
    case K::DeleteColumns: {
        // The order of the rows (columns) changes and no cell moves: a node is of its cell by the ids of its row and column, which are the same, so nothing is told to the nodes; those of formulas that
        // were in rows or columns that died are let go of below.
        const bool rows = e.kind == K::InsertRows || e.kind == K::DeleteRows;
        const bool inserts = e.kind == K::InsertRows || e.kind == K::InsertColumns;
        CellStore::LineShift shift;
        std::vector<Cell> dead;
        jp::calc::LineStatus made = jp::calc::LineStatus::Ok;
        if (undoOf) {
            made = S.cells.undoLines(rows, undoOf->lines, &shift, &dead);
        } else if (redoing) {
            made = S.cells.redoLines(rows, again, &shift, &dead, dormantDelete);
            if (made == jp::calc::LineStatus::Ok) rec->lines = again;
        } else {
            made = S.cells.canEditLines(rows, count);
            if (made == jp::calc::LineStatus::Ok) made = inserts ? S.cells.insertLines(rows, e.first, count, &rec->lines, &shift, &dead) : S.cells.deleteLines(rows, e.first, count, &rec->lines, &shift, &dead, dormantDelete);
        }
        if (made != jp::calc::LineStatus::Ok) {
            // (an Undo or a Redo that the order does not take is not made with other rows: it fails, and says so)
            if (undoOf || redoing) qWarning("Calculation::applyEdit: the order of the rows or columns does not take %s (status %d)", undoOf ? "Undo" : "Redo", int(made));
            // (a record that was to be done again and is not is let go of: the ids and the dead cells it held are not held for nothing)
            if (redoing) S.cells.forgetLines(rows, &again);
            Q_ASSERT_X(!(undoOf || redoing), "Calculation::applyEdit", "the order of the rows or columns does not take Undo or Redo");
            return false;
        }
        for (const Cell &c : dead) removed.push_back(PlacedCell{0, 0, c});
        if (dormantDelete) {
            rec->dormant = true;
            rec->deadCells = S.cells.deadCount() - deadBefore;
            ++m_dormantLines;
        } else if (undoOf && undoOf->dormant) {
            --m_dormantLines;   // (the lines are back: the formulas in them are asked about below)
        }
        // (what the order did: an insert puts its lines in and kills those it pushed off the end; a delete kills its lines; Undo of an insert kills the lines and brings the pushed ones back; Undo of
        // a delete brings the lines back)
        const CellStore::LineEdits &done = undoOf ? undoOf->lines : rec->lines;
        lc.rows = rows;
        if (!undoOf) {
            if (inserts) {
                lc.inserted = done.edit.runs;
                lc.killed = done.edit.pushed;
            } else {
                lc.killed = done.edit.runs;
            }
        } else if (inserts) {
            lc.revived = done.edit.runs;
        } else {
            lc.killed = done.edit.runs;
            lc.revived = done.edit.pushed;
        }
        break;
    }
    case K::InsertCellsDown:
        S.cells.reshuffle(e.row1, [&](quint32 row, quint32 col, const Cell &) { return row >= e.row1 && col >= e.col1 && col <= e.col2; }, [&](PlacedCell *c) { c->row += count; return c->row < kGridRows; }, onMoved, &removed);
        break;
    case K::DeleteCellsUp:
        S.cells.reshuffle(e.row1, [&](quint32 row, quint32 col, const Cell &) { return row >= e.row1 && col >= e.col1 && col <= e.col2; }, [&](PlacedCell *c) {
            if (c->row < e.row1 + count) return false;
            c->row -= count;
            return true;
        }, onMoved, &removed);
        break;
    case K::InsertCellsRight:
        S.cells.reshuffle(e.row1, [&](quint32 row, quint32 col, const Cell &) { return row >= e.row1 && row <= e.row2 && col >= e.col1; }, [&](PlacedCell *c) {
            c->col += count;
            return c->col < kGridCols;
        }, onMoved, &removed);
        break;
    case K::DeleteCellsLeft:
        S.cells.reshuffle(e.row1, [&](quint32 row, quint32 col, const Cell &) { return row >= e.row1 && row <= e.row2 && col >= e.col1; }, [&](PlacedCell *c) {
            if (c->col < e.col1 + count) return false;
            c->col -= count;
            return true;
        }, onMoved, &removed);
        break;
    default: break;
    }
    // the formulas that were in cells that are gone
    for (const PlacedCell &p : removed) {
        const Cell &c = p.cell;
        if (!c.hasFormula() || c.formula < 0 || size_t(c.formula) >= st.nodeOf.size()) continue;
        const quint32 id = st.nodeOf[size_t(c.formula)];
        if (id != kNone) dropNode(id);
        releaseFormula(e.sheet, c.formula);   // (nothing has the formula of a cell that is gone; Undo makes it again from the record)
    }
    lap(2);
    // ---- what the worker is asked (4.6): of the formulas the edit leaves it to find, in ids, once. An edit that is rigid for every formula (what each reads and where it is, as far as it can see, are as
    // they were) asks nothing and leaves the epoch as it is.
    const bool useQuestions = wholeLine && g_graphShifts.load(std::memory_order_relaxed);
    if (wholeLine && (e.kind == K::InsertRows || e.kind == K::InsertColumns) && size_t(e.sheet) < m_seenMaxRow.size()) {
        // (the places formulas saw at or past the lines put in are further on by as many: the largest is kept as large)
        quint32 &largest = (e.kind == K::InsertRows ? m_seenMaxRow : m_seenMaxCol)[size_t(e.sheet)];
        if (largest > e.first) largest += count;
    }
    if (wholeLine) {
        if (useQuestions) {
            const jp::calc::LineMapCore &map = lc.rows ? S.cells.rows().core() : S.cells.cols().core();
            const quint32 firstNew = map.limits().firstNew;
            Question q;
            q.sheet = e.sheet;
            q.rows = lc.rows;
            // the lines whose readers are asked: those that died and those that came back, the original ones as runs of ids and the inserted ones (no block holds their ids) as the original lines either side
            bool insertedLines = false, killedInserted = false;
            auto touch = [&](const std::vector<jp::calc::IdRun> &runs, bool *inserted) {
                for (const jp::calc::IdRun &r : runs) {
                    if (r.first < firstNew) q.dead.push_back({r.first, std::min(r.count, firstNew - r.first)});
                    if (r.first + r.count > firstNew) {
                        *inserted = true;
                        insertedLines = true;
                    }
                }
            };
            // (the lines an insert pushed off the end of the sheet change nothing a formula holds if no cell was in them: a block to the end of the sheet gains the new lines and loses those)
            if (lc.inserted.empty() || S.cells.deadCount() != deadBefore) {
                touch(lc.killed, &killedInserted);
            } else {
                for (const jp::calc::IdRun &r : lc.killed)
                    if (r.first < firstNew) q.deadBlank.push_back({r.first, std::min(r.count, firstNew - r.first)});
            }
            bool revivedInserted = false;
            touch(lc.revived, &revivedInserted);
            // set 1: the gap of lines put in (or brought back), or of inserted lines that died: a block that holds the original lines either side holds it
            // (lines that come back by Undo of a delete are in their place; those an Undo of an insert brings back from the end of the sheet are not, and the inserted lines it kills were at the place of the edit)
            // The gap is asked of the blocks that hold the original lines either side of it. Where the lines between those two (deleted ones) or a side with no original line at all can be an end of a reference
            // (a reference whose end died stays as wide as it was while the line it reads is one that was put in, and one that starts or ends on lines put in at the front or the back has no original line
            // beyond them), the blocks that meet the ids between the two are asked: they hold the references that may have the gap inside.
            auto gapAt = [&](quint32 place, bool linesBothSides) {
                constexpr quint32 kNone = jp::calc::LineMapCore::kNone;
                const jp::calc::IdGap g = jp::calc::idGap(map, place);
                q.gapBefore = g.before;
                q.gapAfter = g.after;
                // (lines put in at the top of the sheet, or at its end, are the start or the end of no reference that was there: a gap with lines on only one side is asked of nobody)
                if (!linesBothSides || (g.before != kNone && g.after != kNone && g.after == g.before + 1)) return;
                const quint32 maxId = lc.rows ? jp::calc::kMaxRowId : jp::calc::kMaxColId;
                q.gapLo = g.before == kNone ? 0 : g.before + 1;
                q.gapHi = g.after == kNone ? maxId : (g.after > 0 ? g.after - 1 : 0);
                if (q.gapHi < q.gapLo) q.gapHi = q.gapLo;
                q.gapWide = true;
            };
            if (!lc.inserted.empty() || (undoOf && !deletes(e))) gapAt(e.first + count, e.first > 0 && e.first + count < map.size());
            else if (killedInserted) gapAt(e.first, true);   // (lines that were there and are gone: a reference that started or ended on one of them has another end now, even at the top)
            // set 3: inserted lines that held cells are gone or back
            q.deadPair = insertedLines && S.cells.deadCount() != deadBefore;
            // The inserted lines that died by a delete (or were pushed off the end), or came back by Undo of a delete (the lines an Undo of an insert kills are named by no formula: what named them was
            // undone first). No block holds an inserted line's id, so what reads it is asked by other means: the formulas that have it at an end of a reference (a list of its own), and the
            // formulas that have an end that died and is the line next to where it was now or before (a record of a dead line points at the live line next to it when it died): those dead lines are
            // asked as the lines that died or came back, and so on through the lines they point at.
            std::vector<jp::calc::IdRun> via;
            auto addInserted = [&](const std::vector<jp::calc::IdRun> &runs) {
                for (const jp::calc::IdRun &r : runs)
                    if (r.first + r.count > firstNew) {
                        const quint32 from = std::max(r.first, firstNew);
                        via.push_back({from, r.first + r.count - from});
                    }
            };
            if (!undoOf) addInserted(lc.killed);
            addInserted(lc.revived);
            if (!via.empty()) {
                const std::vector<jp::calc::LineDead> records = map.deadRecords();
                std::vector<char> taken(records.size(), 0);
                auto inVia = [&](quint32 id) {
                    if (id == jp::calc::LineMapCore::kNone) return false;
                    for (const jp::calc::IdRun &r : via)
                        if (id >= r.first && id - r.first < r.count) return true;
                    return false;
                };
                for (bool grew = true; grew;) {
                    grew = false;
                    for (size_t k = 0; k < records.size(); ++k) {
                        const jp::calc::LineDead &d = records[k];
                        if (taken[k] || (!inVia(d.after) && !inVia(d.before))) continue;
                        taken[k] = 1;
                        via.push_back({d.first, d.count});
                        grew = true;
                    }
                }
                for (size_t k = 0; k < records.size(); ++k)
                    if (taken[k] && records[k].first < firstNew) q.dead.push_back({records[k].first, std::min(records[k].count, firstNew - records[k].first)});
                if (!m_readersOfInserted.empty()) {
                    for (const jp::calc::IdRun &r : via) {
                        if (r.first + r.count <= firstNew) continue;
                        const quint32 from = std::max(r.first, firstNew);
                        const quint32 count = r.first + r.count - from;
                        if (count <= m_readersOfInserted.size()) {
                            for (quint32 k = 0; k < count; ++k) {
                                const auto at = m_readersOfInserted.find(insertedKey(e.sheet, lc.rows, from + k));
                                if (at != m_readersOfInserted.end()) q.nodes.insert(q.nodes.end(), at->second.begin(), at->second.end());
                            }
                        } else {
                            for (const auto &entry : m_readersOfInserted)
                                if (int(entry.first >> 40) == e.sheet && bool((entry.first >> 39) & 1) == lc.rows && quint32(entry.first & 0x7FFFFFFFFFull) >= from && quint32(entry.first & 0x7FFFFFFFFFull) < from + count)
                                    q.nodes.insert(q.nodes.end(), entry.second.begin(), entry.second.end());
                        }
                    }
                    std::sort(q.nodes.begin(), q.nodes.end());
                    q.nodes.erase(std::unique(q.nodes.begin(), q.nodes.end()), q.nodes.end());
                }
            }
            // set 2
            bool own = false;
            q.observers = (!m_seen.empty() || m_ownOnly > 0) && anObserverSawAnotherPlace(e.sheet, lc.rows, e.first, &own);
            q.ownOnly = own;
            q.ownFirst = e.first;
            const bool ask = questionHasAnAnswer(q);
            if (ask || !specials.empty()) ++m_state[size_t(e.sheet)].epoch;
            q.epoch = m_state[size_t(e.sheet)].epoch;
            if (ask) {
                m_questions.push_back(q);
                if (!m_running) beginRecalculation();
            }
        } else {
            // (the oracle: everything is done as for an edit of cells, the epoch grows, and what was written again is made again and worked out; the formulas that see where cells are are looked at too)
            ++m_state[size_t(e.sheet)].epoch;
            bool own = false;
            const bool seen = (!m_seen.empty() || m_ownOnly > 0) && anObserverSawAnotherPlace(e.sheet, lc.rows, e.first, &own);
            if (seen || own) {
                Question q;
                q.sheet = e.sheet;
                q.rows = lc.rows;
                q.observers = seen;
                q.ownOnly = own;
                q.ownFirst = e.first;
                q.epoch = m_state[size_t(e.sheet)].epoch;
                m_questions.push_back(q);
                if (!m_running) beginRecalculation();
            }
        }
    }
    // Lines that came back with their formulas (Undo of a delete that kept them): the formulas in them are worked out again, one job for the worker (what they read may have changed while they were dead;
    // the values they had are what the sheet shows until then)
    if (undoOf && undoOf->dormant && !lc.revived.empty()) {
        // The formulas that tell the graph what they read by what a table, a name, a hidden row, or a sheet stands for were passed by the edits made while their lines were dead (those that were undone since
        // were undone in order, but the table, name, or row they stand for was not asked about for them): the ones in the lines that came back are registered again, as those edits do for the live.
        {
            std::vector<jp::calc::IdRun> sorted = lc.revived;
            std::sort(sorted.begin(), sorted.end(), [](const jp::calc::IdRun &a, const jp::calc::IdRun &b) { return a.first < b.first; });
            auto inRevived = [&](quint32 id) {
                auto it = std::upper_bound(sorted.begin(), sorted.end(), id, [](quint32 v, const jp::calc::IdRun &r) { return v < r.first; });
                return it != sorted.begin() && id - (it - 1)->first < (it - 1)->count;
            };
            std::vector<quint32> again;
            auto consider = [&](quint32 id) {
                if (id >= m_nodes.size()) return;
                const Node &n = m_nodes[id];
                if ((n.flags & Dead) || n.sheet != e.sheet || !inRevived(lc.rows ? n.rowId : n.colId)) return;
                again.push_back(id);
            };
            // (bounded by the smaller of the two: the nodes registered in the lines that came back, found by their cells, or the lists of those that are told, structured, or subtotals, looked at for the
            // lines: a one-row Undo in a workbook of 100,000 table formulas looks at the row)
            const size_t listed = m_structuredNodes.size() + m_subtotalNodes.size() + m_idToldNodes.size() + m_special.size();
            qint64 lines = 0;
            for (const jp::calc::IdRun &r : lc.revived) lines += r.count;
            if (listed == 0) {
                // (none)
            } else if (qint64(listed) > 64 * lines) {
                const SheetState &stateOfSheet = m_state[size_t(e.sheet)];
                S.cells.forEachInIdRuns(lc.rows, lc.revived, [&](const Cell &c) {
                    if (c.hasFormula() && c.formula >= 0 && size_t(c.formula) < stateOfSheet.nodeOf.size() && stateOfSheet.nodeOf[size_t(c.formula)] != kNone) consider(stateOfSheet.nodeOf[size_t(c.formula)]);
                    return true;
                });
            } else {
                for (const quint32 id : m_structuredNodes) consider(id);
                for (const quint32 id : m_subtotalNodes) consider(id);
                for (const quint32 id : m_idToldNodes) consider(id);
                for (const quint32 id : m_special) consider(id);
            }
            std::sort(again.begin(), again.end());
            again.erase(std::unique(again.begin(), again.end()), again.end());
            // (a formula kept apart for what the engine lacks, and not told, is not registered again: it has the graph entries of what it reads, which a table, a name, or a hidden row does not change, and keeps the
            // result it had, as one made again from the record did; a kept formula that is told is registered again with the rest)
            for (const quint32 id : again) {
                if ((m_nodes[id].flags & Told) || m_structuredNodes.count(id) || m_subtotalNodes.count(id) || m_idToldNodes.count(id)) readdNode(id);
            }
        }
        Question q;
        q.sheet = e.sheet;
        q.rows = lc.rows;
        q.revive = lc.revived;
        q.epoch = m_state[size_t(e.sheet)].epoch;
        m_questions.push_back(std::move(q));
        if (!m_running) beginRecalculation();
    }
    // (an edit of cells moves cells one by one: the formulas that see where cells are are looked at too)
    if (!wholeLine && ((!m_seen.empty() && anObserverSawAnotherPlace()) || ownMoved)) {
        Question q;
        q.sheet = e.sheet;
        q.observers = !m_seen.empty();
        q.ownOnly = ownMoved;
        q.epoch = m_state[size_t(e.sheet)].epoch;
        m_questions.push_back(q);
        if (!m_running) beginRecalculation();
    }
    // A formula that uses names and tables tells the graph what it touched as it was worked out, which is places: where an end of a name or of a table was a line that died, what it touched were the lines
    // next to it, and the line that comes back is in no interval of its. So when lines come back every such formula is worked out again (a formula that reads where it is told is made again by every edit).
    if (idLines && !lc.revived.empty() && !m_idToldNodes.empty()) {
        for (const quint32 id : m_idToldNodes) {
            if ((m_nodes[id].flags & Dead) || dormant(m_nodes[id])) continue;
            redirty(id);
            queueDependents(areaOf(m_nodes[id]));
        }
        m_editStats.formulasVisited += qint64(m_idToldNodes.size());
    }
    // Rows or columns inserted or deleted: the graph holds the ids of the lines, which an edit does not change, so what every formula reads is where its cells are with nothing told to the graph; a
    // formula whose text is only moved with what it reads need not be made again (it is worked out again, as every formula that was written again is). A formula that reads a block of several
    // sheets is not rewritten for a change of one of them, and has the ids of each sheet's lines: it is made again, and tells the graph what it reads.
    if (wholeLine) {
        std::vector<quint32> again;   // (a copy: each is dropped and made again, which changes the set; only those whose sheets include the edited one)
        for (const auto &entry : m_threeDSpan)
            if (e.sheet >= entry.second.first && e.sheet <= entry.second.second) again.push_back(entry.first);
        for (const quint32 id : again) {
            const Node &n = m_nodes[id];
            if (dormant(n)) continue;
            Cell *c = cellOfMutable(n);
            if (!c || !c->hasFormula()) continue;
            const int sheet = n.sheet;
            const qint32 formula = c->formula;
            const quint32 rowId = n.rowId, colId = n.colId;
            dropNode(id);
            const quint32 now = addNodeById(sheet, rowId, colId, *c, formula);
            ++m_editStats.nodesRemade;
            queueDependents(areaOf(m_nodes[now]));
        }
    }

    lap(3);
    // ---- the sheet's other structures (each is kept as it was only if the edit changes it)
    {
        auto adjusted = [&](const CellRange &r0) {
            CellRange r = r0;
            if (!adjustRange(e, e.sheet, &r)) r.valid = false;   // (the range of an array formula or a data table of one cell that was deleted: its place in the list stays, for Undo)
            return r;
        };
        bool changed = false;
        for (const CellRange &r : std::as_const(S.ranges))
            if (!(adjusted(r) == r)) {
                changed = true;
                break;
            }
        if (changed) {
            rec->ranges = rangesBefore;
            rec->rangesSaved = true;
            for (CellRange &r : S.ranges) r = adjusted(r);
        }
    }
    {
        QVector<CellRange> kept;
        qint64 dropped = 0;
        bool changed = false;
        for (const CellRange &r0 : std::as_const(S.merges)) {
            CellRange r = r0;
            if (adjustRange(e, e.sheet, &r) && r.cellCount() > 1) {
                if (!(r == r0)) changed = true;
                kept.append(r);
            } else {
                ++dropped;
                changed = true;
            }
        }
        if (changed) {
            rec->merges = mergesBefore;
            rec->mergeCount = mergeCountBefore;
            rec->mergesSaved = true;
            S.mergeCount -= dropped;
            S.merges = kept;
        }
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
        if (!(moved == S.autoFilter) || !(moved.ref == S.autoFilter.ref)) {
            rec->autoFilter = filterBefore;
            rec->filterSaved = true;
            S.autoFilter = moved;
        }
    }
    // (the specs of rows are held by the id of the row in the id frame: no edit of rows moves one, and none has one to put back)
    if ((e.kind == K::InsertRows || e.kind == K::DeleteRows) && !S.rowSpecsAreIds) {
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
        if (specs.size() != rowSpecsBefore.size() || !std::equal(specs.begin(), specs.end(), rowSpecsBefore.begin(), [](const RowSpec &a, const RowSpec &b) { return a.row == b.row; })) {
            rec->rowSpecs = rowSpecsBefore;
            rec->rowSpecsSaved = true;
            S.rowSpecs = specs;
            hiddenRowsChanged(rec->sheet >= 0 ? rec->sheet : e.sheet);
        }
    }
    if (e.kind == K::InsertColumns || e.kind == K::DeleteColumns) {
        QVector<ColumnSpan> spans;
        for (ColumnSpan c : S.columns)
            if (adjustColumnRun(e, &c.first, &c.last)) spans.append(c);
        if (!sameColumns(spans, columnsBefore)) {
            rec->columns = columnsBefore;
            rec->columnsSaved = true;
            S.columns = spans;
        }
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
    m_editStats.cellsMoved += rec->cellsMoved;
    m_editStats.cellsGone += qint64(rec->gone.size());

    // ---- the rules of validation
    adjustValidations(e, names, rec);
    adjustConditionalFormats(e, names, rec);

    // ---- the defined names (one held in ids is as it was for an edit of whole lines: the same ids; for an edit of cells it is looked at by its places)
    bool namesChanged = false;
    QStringList idNamesChanged;
    for (auto it = m_names.begin(); it != m_names.end(); ++it) {
        const bool inIds = m_idNames.contains(it.key());
        if (inIds && wholeLine) continue;
        const int bar = int(it.key().indexOf(QLatin1Char('|')));
        const int scope = bar > 0 ? it.key().left(bar).toInt() : -1;
        bool changed = false;
        const QString places = inIds ? namePlaces(it.key(), it.value()) : it.value();
        const QString now = adjustFormula(places, scope >= 0 ? scope : 0, names, e, &changed, true);
        if (!changed) continue;
        rec->names.push_back({it.key(), taggedName(it.key())});
        if (inIds) {
            bool stillInIds = false;
            it.value() = heldNameText(scope, now, &stillInIds);
            if (!stillInIds) m_idNames.remove(it.key());
            idNamesChanged << it.key().mid(bar + 1);
        } else {
            it.value() = now;
        }
        namesChanged = true;
    }
    if (namesChanged) {
        m_nameTraits.clear();
        for (SheetState &state : m_state) {
            for (Compiled &slot : state.plain) slot.tried = false;
            for (Compiled &slot : state.array) slot.tried = false;
        }
    }

    lap(4);
    // ---- the formulas that read what moved are written again, with what they read followed; those that read where they are told are made again too
    for (size_t i = 0; i < rewrites.size(); ++i) {
        Rewrite &r = rewrites[i];
        Node &n0 = m_nodes[r.id];
        if ((n0.flags & Dead) || dormant(n0)) continue;
        const bool told = (n0.flags & Told) && !(n0.traits & IdTold);   // (with the id frame, a formula that uses names and tables only is not made again by an edit of whole lines)
        if (r.shownOnly) {
            // (the text it is shown as is the one it was written again to before ids: a formula kept apart for its function has #NAME? when the text of what it reads changed)
            const Cell *shownCell = cellOf(n0);
            if (shownCell && shownCell->hasFormula()) {
                const QString shown = m_sheets[size_t(n0.sheet)].formulaText(*shownCell);
                if (shown != r.was) r.now = shown;
            }
        }
        if (r.now.isEmpty() && r.group < 0 && !told && !namesChanged) continue;
        const int sheet = n0.sheet;
        Sheet &sn = m_sheets[size_t(sheet)];
        Cell *c = cellOfMutable(n0);
        if (!c || !c->hasFormula()) continue;
        if (namesChanged && !told && r.now.isEmpty() && r.group < 0) ++m_editStats.namesRegisteredAgain;
        bool classChanged = false;
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
            rec->groups[size_t(g.record)].offsets.push_back({f.dRow, f.dCol});
            const qint32 old = f.text;
            f.text = g.newIndex;
            releaseText(sheet, old);
            ++rec->formulasRewritten;
            rec->rewritten.push_back({sheet, rowOf(n0), colOf(n0), n0.rowId, n0.colId, false});
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
            EditRecord::HeldText held;
            if (m_options.idFrame) held.has = !(c->flags & (ArrayFormula | DataTable)) && heldOf(sheet, *c, &held.text, &held.dRow, &held.dCol, &held.cls);
            rec->was.push_back(std::move(was));
            if (m_options.idFrame) {
                rec->wasHeld.resize(rec->was.size());
                rec->wasHeld.back() = std::move(held);
            }
            const bool wasPositional = m_options.idFrame && m_positionalNodes.count(r.id);
            retext(sheet, c, r.now);   // (in place: the formula keeps its place, and a cell of a shared family has a text of its own from now)
            // (a text that was in places can be held in ids once it is written again, and the other way: its node tells the graph and the lists again)
            if (m_options.idFrame && c->formula >= 0 && c->formula < sn.formulas.size()) classChanged = wasPositional != !inIds(sn, sn.formulas[c->formula].text);
            if (m_options.idFrame) ++m_editStats.eagerFormulas;
            ++rec->formulasRewritten;
            rec->rewritten.push_back({sheet, rowOf(n0), colOf(n0), n0.rowId, n0.colId, bool(c->flags & DataTable)});
        }
        // A formula whose text only moved with what it reads (the graph holds the ids of the lines it reads, which are the same) keeps its node and its entries; the worker is asked about it if the edit
        // could change its value (a gap in a block it reads, a place it saw, a line it read that died), and it is not worked out again for being written again. One that reads where it is told, is kept
        // apart, is an array, reads several sheets, or has a reference that is gone is made again.
        if (useQuestions && !classChanged && (r.group >= 0 || !r.now.isEmpty()) && !told && !(n0.flags & (Kept | Array | ThreeD)) && !(c->flags & (ArrayFormula | DataTable))
            && (r.group >= 0 ? !groups[size_t(r.group)].hasGone : !(r.now.contains(QLatin1String("#REF!")) && !r.was.contains(QLatin1String("#REF!"))))) {
            if (m_hasUnknown) queueDependents(areaOf(n0));   // (what reads its text, FORMULATEXT, is not known to be rigid)
            continue;
        }
        const bool wasKeptForItsFunction = (n0.flags & Kept) && n0.kept == 'U';
        const Value resultItHad = wasKeptForItsFunction ? nodeValue(n0, r.id) : Value();
        const quint32 nodeRow = n0.rowId, nodeCol = n0.colId;
        dropNode(r.id);
        const quint32 id = addNodeById(sheet, nodeRow, nodeCol, *c, c->formula);
        ++m_editStats.nodesRemade;
        queueDependents(areaOf(m_nodes[id]));
        if (wasKeptForItsFunction && (m_nodes[id].flags & Kept) && m_nodes[id].kept == 'U') {
            // (a formula kept apart for a function the engine lacks has the result it had, #NAME? if something it read changed; and #NAME? when a cell it read is deleted)
            storeResult(m_nodes[id], id, resultItHad, false);
            if (r.group >= 0 || !r.now.isEmpty()) {   // (its text changed: it read cells that moved or went, and nothing can be said of what the function makes of them)
                std::vector<CellArea> given;
                giveName(id, &given);
            }
        }
    }
    // what reads cells that moved in or out of a range it reads (a text that is as it was, a whole column, or a block shifted inside a range) has its value again (an edit of whole lines asked the worker)
    if (!useQuestions) queueDependents(region);
    for (const quint32 id : oracleReaders) {
        if (m_nodes[id].flags & Dead) continue;
        redirty(id);
        queueDependents(areaOf(m_nodes[id]));
    }
    // (a name held in ids that an edit of cells wrote again stands for other cells: what uses it is made again)
    if (!idNamesChanged.isEmpty()) nameTextsChangedFor(idNamesChanged);
    lap(5);
    m_editStats.formulasRewritten += rec->formulasRewritten;
    m_editStats.namesRewritten += qint64(rec->names.size());
    m_editStats.rulesRewritten += qint64(rec->validations.size() + rec->conditionalFormats.size());
    m_editStats.graphWork += m_graph->buildWork() + m_graph->queryWork() - graphWorkBefore;
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
        const Held held = heldText(sheet, text);
        Formula &f = s.formulas[c->formula];
        const qint32 old = f.text;
        f.text = internText(sheet, held.text, held.cls);
        releaseText(sheet, old);
    }
    const quint32 id = addNode(sheet, row, col, *c, c->formula);
    queueDependents(areaOf(m_nodes[id]));
    return true;
}

Calculation::EditRecord::GoneCell Calculation::placeGone(int sheet, quint32 row, quint32 col) const
{
    EditRecord::GoneCell g{sheet, row, col, CellContent()};
    const CellStore &cells = m_sheets[size_t(sheet)].cells;
    g.rowId = cells.rowIdAt(row);
    g.colId = cells.colIdAt(col);
    return g;
}

Calculation::EditRecord::GoneCell Calculation::goneCell(int sheet, quint32 row, quint32 col, const Cell &c) const
{
    EditRecord::GoneCell g{sheet, row, col, content(sheet, row, col)};
    g.rowId = c.rowId;
    g.colId = c.colId;
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
        const quint32 now = addNode(g.sheet, g.row, g.col, *c, c->formula);
        queueDependents(areaOf(m_nodes[now]));
        if (g.kind & ArrayFormula) markArrayMembers(g.sheet, g.row, g.col);   // (an array of more cells has its members back)
    }
}

std::vector<quint32> Calculation::renumberEpochs() const
{
    std::vector<quint32> out;
    out.reserve(m_state.size());
    for (const SheetState &st : m_state) out.push_back(st.renumberEpoch);
    return out;
}

bool Calculation::renumberEpochsAre(const std::vector<quint32> &epochs) const
{
    if (epochs.size() != m_state.size()) return false;
    for (size_t i = 0; i < epochs.size(); ++i)
        if (epochs[i] != m_state[i].renumberEpoch) return false;
    return true;
}

void Calculation::pushGone(EditRecord *rec, int sheet, quint32 row, quint32 col, const Cell &c) const
{
    rec->gone.push_back(goneCell(sheet, row, col, c));
    if (!m_options.idFrame) return;
    rec->goneHeld.resize(rec->gone.size());
    EditRecord::HeldText &h = rec->goneHeld.back();
    if (!(c.flags & (ArrayFormula | DataTable))) h.has = heldOf(sheet, c, &h.text, &h.dRow, &h.dCol, &h.cls);
}

// The formula of the cell as the workbook holds it (false if it has none, or its text is held in places): the text in ids, how far the cell is from the one it was written for, and its class.
bool Calculation::heldOf(int sheet, const Cell &c, QString *held, qint32 *dRow, qint32 *dCol, quint8 *cls) const
{
    if (!c.hasFormula() || c.formula < 0) return false;
    const Sheet &s = m_sheets[size_t(sheet)];
    if (size_t(c.formula) >= size_t(s.formulas.size())) return false;
    const Formula &f = s.formulas[c.formula];
    if (f.text < 0 || f.text >= s.formulaTexts.size() || !inIds(s, f.text)) return false;
    *held = s.formulaTexts[f.text];
    *dRow = f.dRow;
    *dCol = f.dCol;
    *cls = quint8(textClass(s, f.text));
    return true;
}

// A formula put back in a cell it was in, as it was held: the text in ids and how far the cell is from the one it was written for.
void Calculation::restoreHeldFormula(int sheet, quint32 row, quint32 col, const QString &held, qint32 dRow, qint32 dCol, quint8 cls)
{
    if (sheet < 0 || sheet >= int(m_sheets.size())) return;
    Sheet &s = m_sheets[size_t(sheet)];
    Cell *c = s.cells.findMutable(row, col);
    if (!c || !c->hasFormula() || c->formula < 0 || c->formula >= s.formulas.size() || (c->flags & (ArrayFormula | DataTable))) return;
    Formula &f = s.formulas[c->formula];
    const qint32 text = internText(sheet, held, TextClass(cls));
    if (text == f.text && f.dRow == dRow && f.dCol == dCol) {
        releaseText(sheet, text);   // (it is what it was: the one more hold on it that making it took is let go of)
        return;
    }
    SheetState &st = m_state[size_t(sheet)];
    if (size_t(c->formula) < st.nodeOf.size() && st.nodeOf[size_t(c->formula)] != kNone) dropNode(st.nodeOf[size_t(c->formula)]);
    const qint32 old = f.text;
    f.text = text;
    f.dRow = dRow;
    f.dCol = dCol;
    releaseText(sheet, old);
    const quint32 id = addNode(sheet, row, col, *c, c->formula);
    queueDependents(areaOf(m_nodes[id]));
}

bool Calculation::undoEdit(const EditRecord &rec, EditRecord *done)
{
    if (rec.edit.kind == StructEdit::Kind::Move) return undoMove(rec, done);
    EditRecord local;
    EditRecord &throwaway = done ? *done : local;
    if (!applyEditImpl(rec.edit.inverse(), &throwaway, &rec)) return false;
    BulkEdit bulk(this);
    Sheet &S = m_sheets[size_t(rec.sheet)];
    // the cells that were deleted are put back, and the formulas that were written again have the text they had
    for (const EditRecord::GoneCell &g : rec.gone) put(g.sheet, g.row, g.col, g.content, Marking::Later);
    // (the formulas that were in them are the ones they were, in ids, unless the ids were renumbered since: the text a person read is not enough for a reference to a line that had died before)
    const bool heldCurrent = recordIdsAreCurrent(rec);
    if (heldCurrent)
        for (size_t k = 0; k < rec.gone.size() && k < rec.goneHeld.size(); ++k)
            if (rec.goneHeld[k].has) restoreHeldFormula(rec.gone[k].sheet, rec.gone[k].row, rec.gone[k].col, rec.goneHeld[k].text, rec.goneHeld[k].dRow, rec.goneHeld[k].dCol, rec.goneHeld[k].cls);
    for (size_t wk = 0; wk < rec.was.size(); ++wk) {
        const EditRecord::FormulaWas &w = rec.was[wk];
        const jp::calc::Value *stored = w.hasStored ? &w.stored : nullptr;
        const EditRecord::HeldText *held = heldCurrent && wk < rec.wasHeld.size() && rec.wasHeld[wk].has ? &rec.wasHeld[wk] : nullptr;
        if (held && !w.hasStored) {
            restoreHeldFormula(w.sheet, w.row, w.col, held->text, held->dRow, held->dCol, held->cls);
            continue;
        }
        if (!w.hasStored && w.sheet >= 0 && w.sheet < int(m_sheets.size())) {
            // (the inverse edit has most of them as they were: those that do are left alone)
            const Sheet &sw = m_sheets[size_t(w.sheet)];
            const Cell *c = sw.cells.find(w.row, w.col);
            if (c && c->hasFormula() && !(c->flags & (ArrayFormula | DataTable)) && sw.formulaText(*c) == w.text) continue;
        }
        if (rewriteArrayText(w.sheet, w.row, w.col, w.text)) continue;
        setFormula(w.sheet, w.row, w.col, w.text, Marking::Later, stored);
        // (a formula kept apart has the result it had; its text is the one it was held as, a line that had died before still being that line)
        if (held) restoreHeldFormula(w.sheet, w.row, w.col, held->text, held->dRow, held->dCol, held->cls);
    }
    // the formulas of shared families have the text they had (the inverse edit has most of them so already: those that have it are left as they are)
    for (const EditRecord::RewrittenGroup &g : rec.groups) {
        if (g.sheet < 0 || g.sheet >= int(m_sheets.size())) continue;
        Sheet &sg = m_sheets[size_t(g.sheet)];
        qint32 index = -1;
        for (size_t k = 0; k < g.cells.size(); ++k) {
            const auto &place = g.cells[k];
            Cell *c = sg.cells.findMutable(place.first, place.second);
            if (!c || !c->hasFormula() || c->formula < 0 || size_t(c->formula) >= size_t(sg.formulas.size())) continue;
            Formula &f = sg.formulas[c->formula];
            const qint32 old = f.text;
            // (the cell is as far from the cell the text was written for as it was: a cell that has been deleted and put back since is a formula of its own, and is put as far as it was)
            const qint32 dRow = k < g.offsets.size() ? g.offsets[k].first : f.dRow, dCol = k < g.offsets.size() ? g.offsets[k].second : f.dCol;
            const bool already = old >= 0 && sg.formulaTexts[old] == g.oldText && f.dRow == dRow && f.dCol == dCol;
            if (already) continue;
            if (index < 0) index = internText(g.sheet, g.oldText);
            else holdText(g.sheet, index);
            f.text = index;
            f.dRow = dRow;
            f.dCol = dCol;
            releaseText(g.sheet, old);
            ++m_editStats.formulasRewritten;
            const quint32 id = nodeAt(g.sheet, place.first, place.second);
            const qint32 formula = c->formula;
            if (id != kNone) dropNode(id);
            const quint32 now = addNode(g.sheet, place.first, place.second, *c, formula);
            ++m_editStats.nodesRemade;
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
    // (a list the edit did not change is as it was; if the edit that undoes it changed one, that one is put back as it was before that: what the edit left)
    if (rec.rangesSaved) S.ranges = rec.ranges;
    else if (throwaway.rangesSaved) S.ranges = throwaway.ranges;
    putBackKinds(rec);
    if (rec.mergesSaved) {
        S.merges = rec.merges;
        S.mergeCount = rec.mergeCount;
    } else if (throwaway.mergesSaved) {
        S.merges = throwaway.merges;
        S.mergeCount = throwaway.mergeCount;
    }
    if (rec.filterSaved) S.autoFilter = rec.autoFilter;
    else if (throwaway.filterSaved) S.autoFilter = throwaway.autoFilter;
    if (rec.columnsSaved) S.columns = rec.columns;
    else if (throwaway.columnsSaved) S.columns = throwaway.columns;
    if (rec.rowSpecsSaved || throwaway.rowSpecsSaved) {
        S.rowSpecs = rec.rowSpecsSaved ? rec.rowSpecs : throwaway.rowSpecs;
        hiddenRowsChanged(rec.sheet);
    }
    S.extentRows = std::max(S.extentRows, rec.extentRows);
    S.extentCols = std::max(S.extentCols, rec.extentCols);
    m_rows[size_t(rec.sheet)] = std::max(m_rows[size_t(rec.sheet)], rec.rowsSeen);
    m_cols[size_t(rec.sheet)] = std::max(m_cols[size_t(rec.sheet)], rec.colsSeen);
    // the names (by the key they are kept by)
    bool changed = false;
    for (const auto &entry : rec.names) {
        restoreTaggedName(entry.first, entry.second);
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
            if ((n.flags & Dead) || dormant(n)) continue;
            if (!(n.flags & Told)) continue;
            Cell *c = cellOfMutable(n);
            if (!c || !c->hasFormula()) continue;
            const int sheet = n.sheet;
            const qint32 formula = c->formula;
            const quint32 rowId = n.rowId, colId = n.colId;
            dropNode(id);
            addNodeById(sheet, rowId, colId, *c, formula);
            ++m_editStats.namesRegisteredAgain;
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
        if ((n.flags & Dead) || !(n.flags & Told) || dormant(n)) continue;
        Cell *c = cellOfMutable(n);
        if (!c || !c->hasFormula()) continue;
        const int sheet = n.sheet;
        const qint32 formula = c->formula;
        const quint32 rowId = n.rowId, colId = n.colId;
        dropNode(id);
        addNodeById(sheet, rowId, colId, *c, formula);
        ++m_editStats.namesRegisteredAgain;
    }
    if (!m_running) beginRecalculation();
}

std::vector<std::pair<QString, QString>> Calculation::renameColumnInNames(const QString &table, const QString &oldName, const QString &newName)
{
    return rewriteNames([&](const QString &text) { return renameColumnInFormula(text, table, oldName, newName, false); });
}

std::vector<std::pair<QString, QString>> Calculation::rewriteNames(const std::function<QString(const QString &)> &rewrite)
{
    std::vector<std::pair<QString, QString>> was;
    abandonNow();
    for (auto it = m_names.begin(); it != m_names.end(); ++it) {
        if (!it.value().contains(QLatin1Char('['))) continue;
        // (a name held in ids is given in places to what rewrites it, and held again as it comes back)
        const bool inIds = m_idNames.contains(it.key());
        const QString places = inIds ? namePlaces(it.key(), it.value()) : it.value();
        const QString now = rewrite(places);
        if (now == places) continue;
        was.push_back({it.key(), taggedName(it.key())});
        if (inIds) {
            const int bar = int(it.key().indexOf(QLatin1Char('|')));
            bool stillInIds = false;
            it.value() = heldNameText(bar > 0 ? it.key().left(bar).toInt() : -1, now, &stillInIds);
            if (!stillInIds) m_idNames.remove(it.key());
        } else {
            it.value() = now;
        }
    }
    if (!was.empty()) nameTextsChanged();
    return was;
}

void Calculation::restoreNames(const std::vector<std::pair<QString, QString>> &names)
{
    if (names.empty()) return;
    abandonNow();
    for (const auto &entry : names) restoreTaggedName(entry.first, entry.second);
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
    const Sheet &s = m_sheets[size_t(sheet)];
    const bool ids = s.rowSpecsAreIds && !s.cells.rows().isIdentity();
    for (RowSpec r : s.rowSpecs) {
        if (ids) {
            r.row = s.cells.rowPlaceOf(r.row);   // (a row that is deleted has no place)
            if (r.row == CellStore::kNoLine) continue;
        }
        if (r.row >= first && r.row <= last) out.push_back(r);
    }
    std::stable_sort(out.begin(), out.end(), [](const RowSpec &a, const RowSpec &b) { return a.row < b.row; });
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
    if (S.rowSpecsAreIds && !S.cells.rows().isIdentity()) {
        // (held by id: those of the rows between `first` and `last` (by place) are taken out, the new ones are put in by the ids of the rows they are for; the specs of rows that are deleted stay)
        for (const RowSpec &r : S.rowSpecs) {
            const quint32 place = S.cells.rowPlaceOf(r.row);
            if (place == CellStore::kNoLine || place < first || place > last) now.append(r);
        }
        for (RowSpec r : specs) {
            r.row = S.cells.rowIdAt(r.row);
            if (r.row != CellStore::kNoLine) now.append(r);
        }
        std::stable_sort(now.begin(), now.end(), [](const RowSpec &a, const RowSpec &b) { return a.row < b.row; });
    } else {
        for (const RowSpec &r : S.rowSpecs)
            if (r.row < first || r.row > last) now.append(r);
        for (const RowSpec &r : specs) now.append(r);
        std::stable_sort(now.begin(), now.end(), [](const RowSpec &a, const RowSpec &b) { return a.row < b.row; });
    }
    S.rowSpecs = now;
    hiddenRowsChanged(sheet);
}

} // namespace jp::office::sheet
