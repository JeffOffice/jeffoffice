// Rows, columns, and cells inserted and deleted in an open workbook: the model does the work (Calculation::applyEdit), and this keeps what the session holds by cell in step with it: the
// values in the window's store, the marks of what changed since the file was read, the tables, and the list of edits the file will be made from. See EditSession::insertRows.
#include "core/editsession.h"

#include <QElapsedTimer>

#include <algorithm>
#include <map>

namespace jp::crunch {

using namespace jp::office;
using namespace jp::office::sheet;
using jp::calc::Value;
using jp::grid::ValueStore;

StructChange::~StructChange()
{
    if (!drops || record.lines.edit.runs.empty()) return;
    Calculation::EditRecord kept;
    kept.edit = edit;
    kept.sheet = edit.sheet;
    kept.lines = std::move(record.lines);
    std::lock_guard<std::mutex> guard(drops->lock);
    drops->records.push_back(std::move(kept));
}

void EditSession::letGoOfLines(Calculation *calc)
{
    std::vector<Calculation::EditRecord> list;
    {
        std::lock_guard<std::mutex> guard(m_lineDrops->lock);
        list.swap(m_lineDrops->records);
    }
    // (the newest first: the ids an insert took are given back when it is the last to have taken any)
    std::sort(list.begin(), list.end(), [](const Calculation::EditRecord &a, const Calculation::EditRecord &b) { return a.lines.edit.after > b.lines.edit.after; });
    // (the cells those steps could bring back are gone, and the ids they took may be given again: a mark on an id that nothing can name is let go of first; a record that holds no id has nothing to sweep)
    struct Released {
        int sheet;
        bool rows;
        std::vector<jp::calc::IdRun> runs;
    };
    std::vector<Released> released;
    for (Calculation::EditRecord &r : list) {
        const bool rows = r.edit.kind == StructEdit::Kind::InsertRows || r.edit.kind == StructEdit::Kind::DeleteRows;
        Released got{r.edit.sheet, rows, r.lines.edit.runs};
        got.runs.insert(got.runs.end(), r.lines.edit.pushed.begin(), r.lines.edit.pushed.end());
        got.runs.insert(got.runs.end(), r.lines.refill.runs.begin(), r.lines.refill.runs.end());
        calc->forgetLines(&r);
        if (!got.runs.empty()) released.push_back(std::move(got));
    }
    for (const Released &g : released) sweepMarks(g.sheet, g.rows, g.runs);
}

void EditSession::setLineLimitsForTest(int sheet, const jp::calc::LineLimits &rows, const jp::calc::LineLimits &cols)
{
    if (std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold()) hold->calculation()->setLineLimitsForTest(sheet, rows, cols);
}

void EditSession::compactLinesFor(const StructEdit &edit)
{
    using K = StructEdit::Kind;
    const bool rows = edit.kind == K::InsertRows || edit.kind == K::DeleteRows;
    if (!rows && edit.kind != K::InsertColumns && edit.kind != K::DeleteColumns) return;
    {
        std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold();
        if (!hold) return;
        letGoOfLines(hold->calculation());
        if (!hold->calculation()->linesNeedCompaction(edit)) return;
    }
    forgetLineSteps(edit.sheet, rows);
    std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold();
    if (!hold) return;
    letGoOfLines(hold->calculation());
    std::vector<jp::calc::LineMapCore::Renumber> table;
    if (!hold->calculation()->compactLines(edit.sheet, rows, &table) || table.empty()) return;
    m_fileOrderLost = true;   // (the ids are numbered by the places they are at: the text of a formula is the file's no more, as far as the ids can tell)
    // (what is held by the ids of the order's lines is renumbered with them: the window's values, and the marks)
    std::vector<ValueStore::Renumbering> renumbering;
    for (const jp::calc::LineMapCore::Renumber &r : table) renumbering.push_back({r.oldFirst, r.count, r.newFirst});
    m_values->renumber(edit.sheet, rows, renumbering);
    renumberMarks(edit.sheet, rows, table);
    hold->changed();   // (the formulas are worked out again, and told by the new ids)
}

qint64 StructChange::bytes() const
{
    qint64 n = qint64(sizeof(*this));
    for (const Calculation::EditRecord::GoneCell &g : record.gone) n += qint64(sizeof(g)) + contentBytes(g.content);
    for (const Calculation::EditRecord::FormulaWas &w : record.was) n += qint64(sizeof(w)) + qint64(w.text.size()) * 2;
    // (the formulas as they were held, in ids: the texts of a family are shared, but each entry is counted whole, as the record's other texts are)
    for (const Calculation::EditRecord::HeldText &h : record.goneHeld) n += qint64(sizeof(h)) + qint64(h.text.size()) * 2;
    for (const Calculation::EditRecord::HeldText &h : record.wasHeld) n += qint64(sizeof(h)) + qint64(h.text.size()) * 2;
    n += qint64(record.renumberEpochs.size()) * qint64(sizeof(quint32));
    for (const Calculation::EditRecord::RewrittenGroup &g : record.groups) n += qint64(sizeof(g)) + qint64(g.oldText.size()) * 2 + qint64(g.cells.size()) * qint64(sizeof(std::pair<quint32, quint32>));
    n += qint64(record.ranges.size() + record.merges.size()) * qint64(sizeof(CellRange));
    n += qint64(record.columns.size()) * qint64(sizeof(ColumnSpan)) + qint64(record.rowSpecs.size()) * qint64(sizeof(RowSpec));
    for (const auto &name : record.names) n += qint64(name.first.size() + name.second.size()) * 2;
    n += qint64(record.rewritten.size()) * qint64(sizeof(Calculation::EditRecord::Place));
    n += qint64(touchedGone.size() + touchedAdded.size()) * 8 + qint64(tables.size()) * qint64(sizeof(TablePlace));
    for (const TablePlace &p : tables)
        if (p.whole) n += tableBytes(*p.whole);
    n += qint64(resultsGone.size() + engineGone.size() + fileGone.size()) * qint64(sizeof(std::pair<quint64, Value>));
    if (anchors) n += anchors->bytes();   // (the notes and links it moved or took away)
    return n;
}

bool EditSession::canMove(const StructEdit &edit, QString *why) const
{
    if (!canEdit(why) || !canEditSheet(edit.sheet, why)) return false;
    if (edit.kind == StructEdit::Kind::Move && !canEditSheet(edit.toSheet, why)) return false;
    if (!m_book->calc->canApply(edit, why)) return false;
    // a pivot table sits on the cells of its sheet, where the file says: what would move it or cut through it is not done (the file would have it where it was); the rest of the sheet is
    // edited as ever
    using K = StructEdit::Kind;
    auto touches = [&](int sheet, const CellRange &pivot) {
        const quint32 f = edit.first;
        switch (edit.kind) {
        case K::InsertRows:
        case K::DeleteRows: return sheet == edit.sheet && pivot.row2 >= f;
        case K::InsertColumns:
        case K::DeleteColumns: return sheet == edit.sheet && pivot.col2 >= f;
        case K::InsertCellsDown:
        case K::DeleteCellsUp: return sheet == edit.sheet && pivot.col2 >= edit.col1 && pivot.col1 <= edit.col2 && pivot.row2 >= edit.row1;
        case K::InsertCellsRight:
        case K::DeleteCellsLeft: return sheet == edit.sheet && pivot.row2 >= edit.row1 && pivot.row1 <= edit.row2 && pivot.col2 >= edit.col1;
        case K::Move: {
            const quint32 toRow2 = edit.toRow + (edit.row2 - edit.row1), toCol2 = edit.toCol + (edit.col2 - edit.col1);
            const bool fromHere = sheet == edit.sheet && !(pivot.row2 < edit.row1 || pivot.row1 > edit.row2 || pivot.col2 < edit.col1 || pivot.col1 > edit.col2);
            const bool toHere = sheet == edit.toSheet && !(pivot.row2 < edit.toRow || pivot.row1 > toRow2 || pivot.col2 < edit.toCol || pivot.col1 > toCol2);
            return fromHere || toHere;
        }
        default: return false;
        }
    };
    for (const int sheet : {edit.sheet, edit.kind == K::Move ? edit.toSheet : edit.sheet}) {
        for (const CellRange &pivot : m_book->book->pivotTablesOn(sheet)) {
            if (!touches(sheet, pivot)) continue;
            if (why) *why = tr("That would move or change a pivot table, which JeffCrunch can't do yet. Change cells that are not part of it.");
            return false;
        }
    }
    return m_book->book->tablesAllow(edit, why);
}

EditSession::Outcome EditSession::structure(const StructEdit &edit, const QString &text)
{
    Outcome out;
    QString why;
    compactLinesFor(edit);
    if (!canMove(edit, &why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    // a step that would hold more than Undo can is asked about first (what it deletes or writes over, and what it writes again, are kept to undo it)
    bool big = false;
    {
        // (the graph is asked how many formulas read what moves, with the worker held back: it works in the graph)
        std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold();
        if (!hold) {
            out.ok = false;
            out.problem = tr("The workbook is busy. Try again in a moment.");
            return out;
        }
        big = hold->calculation()->estimateUndoBytes(edit, m_undoByteLimit + 1) > m_undoByteLimit;
    }
    if (big && !m_bigStepAllowed) {
        out.ok = false;
        out.confirm = true;
        out.problem = tr("This can't be undone, and neither can what you did before it. Do you want to go on?");
        return out;
    }
    m_bigStepAllowed = false;
    Step step;
    step.structure = std::make_shared<StructChange>();
    step.structure->drops = m_lineDrops;
    step.structure->edit = edit;
    pushStep(std::move(step), text);
    if (big) {
        // (it is done, and what it holds is let go of with the steps before it: it is not kept, and Undo does not go back over it)
        forgetStepsDone();
    }
    return out;
}

EditSession::Outcome EditSession::insertRows(int sheet, quint32 first, quint32 count)
{
    const Outcome out = structure(StructEdit::insertRows(sheet, first, count), count == 1 ? tr("Inserting a row") : tr("Inserting rows"));
    if (out.ok && !out.confirm) fillTableAfterInsert(sheet, true, first, count);
    return out;
}

EditSession::Outcome EditSession::deleteRows(int sheet, quint32 first, quint32 count)
{
    return structure(StructEdit::deleteRows(sheet, first, count), count == 1 ? tr("Deleting a row") : tr("Deleting rows"));
}

EditSession::Outcome EditSession::insertColumns(int sheet, quint32 first, quint32 count)
{
    const Outcome out = structure(StructEdit::insertColumns(sheet, first, count), count == 1 ? tr("Inserting a column") : tr("Inserting columns"));
    if (out.ok && !out.confirm) fillTableAfterInsert(sheet, false, first, count);
    return out;
}

EditSession::Outcome EditSession::deleteColumns(int sheet, quint32 first, quint32 count)
{
    return structure(StructEdit::deleteColumns(sheet, first, count), count == 1 ? tr("Deleting a column") : tr("Deleting columns"));
}

EditSession::Outcome EditSession::insertCells(int sheet, const CellRange &block, bool shiftDown)
{
    return structure(shiftDown ? StructEdit::insertCellsDown(sheet, block) : StructEdit::insertCellsRight(sheet, block), tr("Inserting cells"));
}

EditSession::Outcome EditSession::deleteCells(int sheet, const CellRange &block, bool shiftUp)
{
    return structure(shiftUp ? StructEdit::deleteCellsUp(sheet, block) : StructEdit::deleteCellsLeft(sheet, block), tr("Deleting cells"));
}

namespace {

using K = StructEdit::Kind;

// Whole rows or whole columns: the edits that move no cell (the order of the sheet's rows or columns changes, and what is held by the ids of its rows and columns stays where it is).
bool wholeLines(const StructEdit &e) { return e.kind == K::InsertRows || e.kind == K::DeleteRows || e.kind == K::InsertColumns || e.kind == K::DeleteColumns; }

// Where the key of a cell is after the edit of cells (shifted one by one: they are given new ids as they go): false if it is gone.
bool movedKey(const Calculation &calc, const StructEdit &e, quint64 key, quint64 *now)
{
    int sheet = int(key >> 40);
    const CellStore &was = calc.sheet(sheet).cells;
    quint32 row = was.rowPlaceOf(quint32((key >> 16) & 0xFFFFFF)), col = was.colPlaceOf(quint32(key & 0xFFFF));
    if (row == CellStore::kNoLine || col == CellStore::kNoLine) {
        *now = key;   // (a cell in a row or column that is deleted is not moved by cells shifted)
        return true;
    }
    const RefFate fate = adjustCell(e, &sheet, &row, &col);
    if (fate == RefFate::Gone) return false;
    const CellStore &after = calc.sheet(sheet).cells;
    const quint32 rowId = after.rowIdAt(row), colId = after.colIdAt(col);
    if (rowId == CellStore::kNoLine || colId == CellStore::kNoLine) return false;
    *now = EditSession::keyOf(sheet, rowId, colId);
    return true;
}

bool mayMove(const StructEdit &e, quint64 key)
{
    const int sheet = int(key >> 40);
    return sheet == e.sheet || (e.kind == StructEdit::Kind::Move && sheet == e.toSheet);
}

// The marks of cells (a set) after the edit; the keys of those that are gone are given back.
void followSet(const Calculation &calc, KeySet *marks, const StructEdit &e, std::vector<quint64> *gone)
{
    std::vector<quint64> moved;
    for (const quint64 key : marks->keys()) {
        if (!mayMove(e, key)) continue;
        quint64 now = 0;
        if (!movedKey(calc, e, key, &now)) {
            if (gone) gone->push_back(key);
            marks->erase(key);
        } else if (now != key) {
            moved.push_back(now);
            marks->erase(key);
        }
    }
    for (const quint64 key : moved) marks->insert(key);
}

// And the results kept by cell.
void followMap(const Calculation &calc, ResultMap *results, const StructEdit &e, std::vector<std::pair<quint64, Value>> *gone)
{
    std::vector<std::pair<quint64, Value>> moved;
    for (auto it = results->begin(); it != results->end();) {
        if (!mayMove(e, it->first)) {
            ++it;
            continue;
        }
        quint64 now = 0;
        if (!movedKey(calc, e, it->first, &now)) {
            if (gone) gone->push_back(*it);
            it = results->erase(it);
        } else if (now != it->first) {
            moved.push_back({now, it->second});
            it = results->erase(it);
        } else {
            ++it;
        }
    }
    for (auto &entry : moved) results->set(entry.first, entry.second);
}

} // namespace

quint64 EditSession::markOf(int sheet, quint32 row, quint32 col) const
{
    if (!m_book || !m_book->calc || sheet < 0 || sheet >= m_book->calc->sheetCount()) return kNoKey;
    const CellStore &cells = m_book->calc->sheet(sheet).cells;
    const quint32 rowId = cells.rowIdAt(row), colId = cells.colIdAt(col);
    if (rowId == CellStore::kNoLine || colId == CellStore::kNoLine || rowId >= (quint32(1) << 24) || colId >= (quint32(1) << 16)) return kNoKey;
    return keyOf(sheet, rowId, colId);
}

bool EditSession::placeOfMark(quint64 key, int *sheet, quint32 *row, quint32 *col) const
{
    const int s = int(key >> 40);
    if (!m_book || !m_book->calc || s < 0 || s >= m_book->calc->sheetCount()) return false;
    const CellStore &cells = m_book->calc->sheet(s).cells;
    const quint32 r = cells.rowPlaceOf(quint32((key >> 16) & 0xFFFFFF)), c = cells.colPlaceOf(quint32(key & 0xFFFF));
    if (r == CellStore::kNoLine || c == CellStore::kNoLine) return false;
    *sheet = s;
    *row = r;
    *col = c;
    return true;
}

void EditSession::markTouched(int sheet, quint32 row, quint32 col)
{
    const quint64 key = markOf(sheet, row, col);
    if (key != kNoKey) m_touched.insert(key);
}

void EditSession::sweepMarks(int sheet, bool rows, const std::vector<jp::calc::IdRun> &released)
{
    if (!m_book || !m_book->calc || sheet < 0 || sheet >= m_book->calc->sheetCount() || released.empty()) return;
    const CellStore &cells = m_book->calc->sheet(sheet).cells;
    // (only a mark on an id of a line the records let go of can have lost its line: the others are passed by with one look at the runs, and the id is asked about only then)
    auto gone = [&](quint64 key, bool useRow, bool useCol) {
        ++m_marksSwept;
        if (int(key >> 40) != sheet) return false;
        const bool byRow = rows ? useRow : false, byCol = !rows ? useCol : false;
        if (!byRow && !byCol) return false;
        const quint32 id = byRow ? quint32((key >> 16) & 0xFFFFFF) : quint32(key & 0xFFFF);
        bool inRuns = false;
        for (const jp::calc::IdRun &r : released) inRuns = inRuns || (id >= r.first && id - r.first < r.count);
        return inRuns && !(byRow ? cells.rowIdKnown(id) : cells.colIdKnown(id));
    };
    std::vector<quint64> dead;
    m_touched.forEach([&](quint64 key) {
        if (gone(key, true, true)) dead.push_back(key);
    });
    for (const quint64 key : dead) m_touched.erase(key);
    for (auto *map : {&m_results, &m_engineAtStart, &m_fileResults})
        for (auto it = map->begin(); it != map->end();) it = gone(it->first, true, true) ? map->erase(it) : std::next(it);
    for (auto it = m_columnsResized.begin(); it != m_columnsResized.end();) it = gone(*it, false, true) ? m_columnsResized.erase(it) : std::next(it);
    for (auto it = m_rowsResized.begin(); it != m_rowsResized.end();) it = gone(*it, true, false) ? m_rowsResized.erase(it) : std::next(it);
}

void EditSession::renumberMarks(int sheet, bool rows, const std::vector<jp::calc::LineMapCore::Renumber> &table)
{
    // (every key of the sheet has the id of its rows or columns made what the cells were given; a key whose line is not in the table is of a line that is gone)
    auto renumbered = [&](quint64 key, bool useRow, bool useCol, quint64 *now) {
        if (int(key >> 40) != sheet) {
            *now = key;
            return true;
        }
        quint32 row = quint32((key >> 16) & 0xFFFFFF), col = quint32(key & 0xFFFF);
        if (rows && useRow) {
            row = CellStore::renumberLine(table, row);
            if (row == CellStore::kNoLine) return false;
        } else if (!rows && useCol) {
            col = CellStore::renumberLine(table, col);
            if (col == CellStore::kNoLine) return false;
        }
        *now = keyOf(sheet, row, col);
        return true;
    };
    {
        KeySet kept;
        kept.reserve(m_touched.size());
        for (const quint64 key : m_touched.keys()) {
            quint64 now;
            if (renumbered(key, true, true, &now)) kept.insert(now);
        }
        m_touched.swap(kept);
    }
    for (auto *map : {&m_results, &m_engineAtStart, &m_fileResults}) {
        ResultMap kept;
        for (const auto entry : *map) {
            quint64 now;
            if (renumbered(entry.first, true, true, &now)) kept.set(now, entry.second);
        }
        map->swap(kept);
    }
    for (auto *set : {&m_columnsResized, &m_rowsResized}) {
        std::set<quint64> kept;
        for (const quint64 key : *set) {
            quint64 now;
            if (renumbered(key, set == &m_rowsResized, set == &m_columnsResized, &now)) kept.insert(now);
        }
        set->swap(kept);
    }
    // the ids that the steps' records hold: the cells they rewrote and deleted (`Place`, `GoneCell`, `ShiftedFormula`), for a step that is kept (an edit of the other axis, of cells, or of names) is
    // undone after this and follows its marks by them
    auto renumberId = [&](quint32 *id) {
        const quint32 now = CellStore::renumberLine(table, *id);
        *id = now;   // (kNoLine for a line that is gone: no cell is there to be found by it)
    };
    auto renumberPlaces = [&](std::vector<Calculation::EditRecord::Place> *places) {
        for (Calculation::EditRecord::Place &p : *places) {
            if (p.sheet != sheet) continue;
            renumberId(rows ? &p.rowId : &p.colId);
        }
    };
    eachStepData([&](Step *step) {
        for (SheetEdit &e : step->sheets) renumberPlaces(&e.rewrite.rewritten);
        for (NameEdit &e : step->names) renumberPlaces(&e.rewrite.rewritten);
    });
    // the steps' own keys (cells shifted by one step: what it deleted and marked, to put back if it is undone)
    eachStructChange([&](StructChange *sc) {
            renumberPlaces(&sc->record.rewritten);
            for (Calculation::EditRecord::GoneCell &g : sc->record.gone)
                if (g.sheet == sheet) renumberId(rows ? &g.rowId : &g.colId);
            if (sc->record.edit.sheet == sheet)
                for (Calculation::EditRecord::ShiftedFormula &m : sc->record.shifted) {
                    renumberId(rows ? &m.fromRowId : &m.fromColId);
                    renumberId(rows ? &m.toRowId : &m.toColId);
                }
            for (std::vector<quint64> *list : {&sc->touchedGone, &sc->touchedAdded}) {
                std::vector<quint64> kept;
                for (const quint64 key : *list) {
                    quint64 now;
                    if (renumbered(key, true, true, &now)) kept.push_back(now);
                }
                list->swap(kept);
            }
            for (std::vector<std::pair<quint64, Value>> *list : {&sc->resultsGone, &sc->engineGone, &sc->fileGone}) {
                std::vector<std::pair<quint64, Value>> kept;
                for (auto &entry : *list) {
                    quint64 now;
                    if (renumbered(entry.first, true, true, &now)) kept.push_back({now, std::move(entry.second)});
                }
                list->swap(kept);
            }
        });
}

void EditSession::dropStale(std::vector<ValueStore::Item> *items)
{
    if (!m_book || !m_book->calc) return;
    Calculation &calc = *m_book->calc;
    struct Redo {
        int sheet;
        Calculation::Target at;
    };
    std::vector<Redo> redo;
    items->erase(std::remove_if(items->begin(), items->end(), [&](const ValueStore::Item &it) {
        Calculation::Target again;
        if (!calc.staleResult(it.sheet, it.row, it.col, it.epoch, &again)) return false;
        if (again.valid) redo.push_back({it.sheet, again});
        return true;
    }), items->end());
    if (redo.empty()) return;
    // the formulas that moved and whose value was let go of are worked out again (the worker is held for it: a rare thing, and only what was in flight)
    if (std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold()) {
        for (const Redo &r : redo) hold->calculation()->redirtyCell(r.sheet, r.at.rowId, r.at.colId);
        hold->changed();
    }
}

void EditSession::pruneStamps(const std::vector<quint32> &epochs)
{
    if (m_book && m_book->calc) m_book->calc->pruneStamps(epochs);
}

std::vector<ValueStore::Item> EditSession::placed(const std::vector<ValueStore::Item> &items) const
{
    std::vector<ValueStore::Item> out;
    if (!m_book || !m_book->calc) return out;
    out.reserve(items.size());
    const int sheets = m_book->calc->sheetCount();
    for (const ValueStore::Item &it : items) {
        if (it.sheet < 0 || it.sheet >= sheets) continue;
        const CellStore &cells = m_book->calc->sheet(it.sheet).cells;
        const quint32 row = cells.rowPlaceOf(it.row), col = cells.colPlaceOf(it.col);
        if (row == CellStore::kNoLine || col == CellStore::kNoLine) continue;
        ValueStore::Item p;
        p.sheet = it.sheet;
        p.row = row;
        p.col = col;
        out.push_back(std::move(p));
    }
    return out;
}

void EditSession::followEdit(const StructEdit &edit, StructChange *sc, bool forward)
{
    // cells moved one by one, across the rows or columns of the file's sheet, are written whole where they are now
    const Calculation &calc = *m_book->calc;
    const bool lines = wholeLines(edit);
    auto markShifted = [&](const StructEdit &e, const std::function<void(quint64)> &mark) {
        if (e.kind != K::InsertCellsDown && e.kind != K::DeleteCellsUp && e.kind != K::InsertCellsRight && e.kind != K::DeleteCellsLeft) return;
        const Sheet &s = calc.sheet(e.sheet);
        const bool vertical = e.kind == K::InsertCellsDown || e.kind == K::DeleteCellsUp;
        const quint32 r1 = e.row1, c1 = e.col1;
        const quint32 r2 = vertical ? kMaxRows - 1 : e.row2, c2 = vertical ? e.col2 : kMaxColumns - 1;
        s.cells.forEachInRange(r1, c1, r2, c2, [&](quint32, quint32, const Cell &c) {
            mark(keyOf(e.sheet, c.rowId, c.colId));
            return true;
        });
    };
    if (forward) {
        sc->savedGen = m_saveGen;
        sc->touchedGone.clear();
        sc->resultsGone.clear();
        sc->engineGone.clear();
        sc->fileGone.clear();
        sc->touchedAdded.clear();
        // (whole rows and columns move no mark: they are by the ids of rows and columns, which an edit of them does not change; the marks of cells shifted or moved are followed)
        if (!lines) {
            m_marksFollowed += qint64(m_touched.size() + m_results.size() + m_engineAtStart.size() + m_fileResults.size());
            // (the marks of a Move stay where they are: they say what differs from the file at a place, and the file's cells do not move with the cells)
            if (edit.kind != K::Move) {
                followSet(calc, &m_touched, edit, &sc->touchedGone);
                followMap(calc, &m_results, edit, &sc->resultsGone);
                followMap(calc, &m_engineAtStart, edit, &sc->engineGone);
                followMap(calc, &m_fileResults, edit, &sc->fileGone);
            }
        }
        auto mark = [&](quint64 key) {
            if (m_touched.insert(key)) sc->touchedAdded.push_back(key);
        };
        // the formulas that were written again are written whole when the file is made (what they read moved)
        m_touched.reserve(m_touched.size() + sc->record.rewritten.size());
        for (const Calculation::EditRecord::Place &p : sc->record.rewritten) {
            if (p.dataTable) continue;   // (the file's own formula is moved with its cell)
            mark(keyOf(p.sheet, p.rowId, p.colId));
        }
        markShifted(edit, mark);
        if (edit.kind == K::Move) {
            // the places of the block and of what it went over are written whole: those that held cells, and the cells now in the new place
            for (const Calculation::EditRecord::GoneCell &g : sc->record.gone) mark(keyOf(g.sheet, g.rowId, g.colId));
            const quint32 toRow2 = edit.toRow + (edit.row2 - edit.row1), toCol2 = edit.toCol + (edit.col2 - edit.col1);
            calc.sheet(edit.toSheet).cells.forEachInRange(edit.toRow, edit.toCol, toRow2, toCol2, [&](quint32, quint32, const Cell &c) {
                mark(keyOf(edit.toSheet, c.rowId, c.colId));
                return true;
            });
            m_mergesChanged.insert(edit.sheet);
            m_mergesChanged.insert(edit.toSheet);
        }
        m_edits.push_back(edit);
        forgetNotes();
    } else if (sc->savedGen != m_saveGen) {
        // The file was saved since the edit was done, so it has the edit: the edit that undoes it is made against the saved file (it goes in the list for the file), and the cells that come back, and
        // those that were written over, are written whole. (A step that deleted cells is not undone after a save: see forgetDeletesInFile.)
        const StructEdit back = edit.inverse();
        if (!lines) {
            m_marksFollowed += qint64(m_touched.size() + m_results.size() + m_engineAtStart.size() + m_fileResults.size());
            if (edit.kind != K::Move) {
                followSet(calc, &m_touched, back, nullptr);
                followMap(calc, &m_results, back, nullptr);
                followMap(calc, &m_engineAtStart, back, nullptr);
                followMap(calc, &m_fileResults, back, nullptr);
            }
        }
        // (the places of the record are those of the edit done: the order is as it was then once the edit is undone, so the places and their cells are the same)
        auto mark = [&](quint64 key) {
            if (key != kNoKey) m_touched.insert(key);
        };
        for (const Calculation::EditRecord::GoneCell &g : sc->record.gone) mark(markOf(g.sheet, g.row, g.col));
        for (const Calculation::EditRecord::Place &p : sc->record.rewritten) {
            // (the formulas that were written again have the text they had: where they are now, after the edit is undone)
            if (lines) {
                if (p.rowId != CellStore::kNoLine && p.colId != CellStore::kNoLine) mark(keyOf(p.sheet, p.rowId, p.colId));
                continue;
            }
            int sheet = p.sheet;
            quint32 row = p.row, col = p.col;
            if (edit.kind == K::Move || adjustCell(back, &sheet, &row, &col) != RefFate::Gone) mark(markOf(sheet, row, col));
        }
        markShifted(back, [&](quint64 key) { mark(key); });
        if (edit.kind == K::Move) {
            // the cells that went to the new place are not there any more
            const qint64 dRow = qint64(edit.toRow) - edit.row1, dCol = qint64(edit.toCol) - edit.col1;
            for (const Calculation::EditRecord::GoneCell &g : sc->record.gone)
                if (g.sheet == edit.sheet && g.row >= edit.row1 && g.row <= edit.row2 && g.col >= edit.col1 && g.col <= edit.col2) mark(markOf(edit.toSheet, quint32(qint64(g.row) + dRow), quint32(qint64(g.col) + dCol)));
            m_mergesChanged.insert(edit.sheet);
            m_mergesChanged.insert(edit.toSheet);
        } else {
            m_mergesChanged.insert(edit.sheet);
        }
        m_edits.push_back(back);
        forgetNotes();
    } else {
        // (the state before the undo is the one after the edit was done: nothing was done to the cells since)
        for (const quint64 key : sc->touchedAdded) m_touched.erase(key);
        sc->touchedAdded.clear();
        const StructEdit back = edit.inverse();
        if (!lines && edit.kind != K::Move) {
            followSet(calc, &m_touched, back, nullptr);
            followMap(calc, &m_results, back, nullptr);
            followMap(calc, &m_engineAtStart, back, nullptr);
            followMap(calc, &m_fileResults, back, nullptr);
        }
        for (const quint64 key : sc->touchedGone) m_touched.insert(key);
        for (auto &entry : sc->resultsGone) m_results.set(entry.first, entry.second);
        for (auto &entry : sc->engineGone) m_engineAtStart.set(entry.first, entry.second);
        for (auto &entry : sc->fileGone) m_fileResults.set(entry.first, entry.second);
        if (!m_edits.empty() && m_edits.back().kind == edit.kind && m_edits.back().sheet == edit.sheet) m_edits.pop_back();
        forgetNotes();
    }
}

bool EditSession::applyStructure(Step *step, bool forward)
{
    StructChange &sc = *step->structure;
    if (forward == sc.done) return true;
    QElapsedTimer waited;
    waited.start();
    std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold();
    m_lastHoldWait = waited.nsecsElapsed() / 1000;
    m_longestHoldWait = std::max(m_longestHoldWait, m_lastHoldWait);
    if (!hold) return false;
    Calculation *calc = hold->calculation();
    Workbook &book = *m_book->book;
    if (m_holdHook) m_holdHook(calc, false);
    bool ok;
    Calculation::EditRecord undone;   // what undoing did to the cells (the record of the edit it made)
    if (forward) {
        // (an edit done again after Undo brings back the rows or columns it made, not new ones)
        Calculation::EditRecord previous = std::move(sc.record);
        sc.record = Calculation::EditRecord();
        sc.record.lines = std::move(previous.lines);
        sc.record.edit = previous.edit;
        sc.record.sheet = previous.sheet;
        ok = calc->applyEdit(sc.edit, &sc.record);
        if (ok) sc.tables = book.moveTables(sc.edit);
    } else {
        ok = calc->undoEdit(sc.record, &undone);
        if (ok) book.putTablesBack(sc.tables);
    }
    if (!ok) return false;
    if (!sc.tables.empty()) calc->tablesMovedByEdit();   // (the formulas that name a table whose ids changed read it where it is now)
    sc.done = forward;
    const StructEdit now = forward ? sc.edit : sc.edit.inverse();   // what was done to the cells
    followEdit(sc.edit, &sc, forward);
    {
        // the print area, the titles, and the breaks follow the cells (and come back with them)
        QStringList names;
        for (const jp::office::sheet::SheetEntry &se : book.info().sheets) names << se.name;
        if (forward) m_pages.followEdit(sc.edit, names, &sc);
        else m_pages.undoEdit(&sc);
        if (forward) {
            sc.anchors = m_anchors.followEdit(sc.edit, names);
        } else {
            m_anchors.undoEdit(sc.anchors, sc.edit);
            sc.anchors.reset();
        }
        std::set<int> sheets = {sc.edit.sheet};
        if (sc.edit.kind == StructEdit::Kind::Move) sheets.insert(sc.edit.toSheet);
        for (const int s : sheets) {
            Q_EMIT pageSetupChanged(s);
            Q_EMIT anchorsChanged(s);
        }
    }
    // the values the window holds follow the cells: those of cells that are gone are let go of, and, for cells shifted one by one, those of formulas that moved (with their cells) go with them. Whole
    // rows and columns move no value: they are held by the ids of the cell's row and column.
    {
        const Calculation::EditRecord &done = forward ? sc.record : undone;
        std::map<int, std::vector<std::pair<quint32, quint32>>> gone;
        for (const Calculation::EditRecord::GoneCell &g : done.gone) gone[g.sheet].emplace_back(g.rowId, g.colId);
        for (const auto &entry : gone) m_values->erase(entry.first, entry.second);
        if (now.kind != StructEdit::Kind::Move && !done.shifted.empty()) {
            std::vector<ValueStore::Move> moves;
            moves.reserve(done.shifted.size());
            for (const Calculation::EditRecord::ShiftedFormula &m : done.shifted) moves.push_back({m.fromRowId, m.fromColId, m.toRowId, m.toColId});
            m_values->relocate(now.sheet, moves);
        }
    }
    hold->changed();
    if (m_holdHook) m_holdHook(calc, true);
    hold.reset();
    m_tracking = true;
    Q_EMIT cellsMoved(now);
    if (!sc.tables.empty()) Q_EMIT tablesChanged(sc.edit.sheet);
    return true;
}

} // namespace jp::crunch
