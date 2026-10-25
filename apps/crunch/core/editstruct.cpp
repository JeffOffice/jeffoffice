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

qint64 StructChange::bytes() const
{
    qint64 n = qint64(sizeof(*this));
    for (const Calculation::EditRecord::GoneCell &g : record.gone) n += qint64(sizeof(g)) + contentBytes(g.content);
    for (const Calculation::EditRecord::FormulaWas &w : record.was) n += qint64(sizeof(w)) + qint64(w.text.size()) * 2;
    for (const Calculation::EditRecord::RewrittenGroup &g : record.groups) n += qint64(sizeof(g)) + qint64(g.oldText.size()) * 2 + qint64(g.cells.size()) * qint64(sizeof(std::pair<quint32, quint32>));
    n += qint64(record.ranges.size() + record.merges.size()) * qint64(sizeof(CellRange));
    n += qint64(record.columns.size()) * qint64(sizeof(ColumnSpan)) + qint64(record.rowSpecs.size()) * qint64(sizeof(RowSpec));
    for (const auto &name : record.names) n += qint64(name.first.size() + name.second.size()) * 2;
    n += qint64(record.rewritten.size()) * qint64(sizeof(Calculation::EditRecord::Place));
    n += qint64(touchedGone.size() + touchedAdded.size()) * 8 + qint64(tables.size()) * qint64(sizeof(TablePlace));
    for (const TablePlace &p : tables)
        if (p.whole) n += tableBytes(*p.whole);
    n += qint64(resultsGone.size() + engineGone.size() + fileGone.size()) * qint64(sizeof(std::pair<quint64, Value>));
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

// Where the key of a cell is after the edit: false if it is gone.
bool movedKey(const StructEdit &e, quint64 key, quint64 *now)
{
    int sheet = int(key >> 40);
    quint32 row = quint32((key >> 16) & 0xFFFFFF), col = quint32(key & 0xFFFF);
    const RefFate fate = adjustCell(e, &sheet, &row, &col);
    if (fate == RefFate::Gone) return false;
    *now = EditSession::keyOf(sheet, row, col);
    return true;
}

bool mayMove(const StructEdit &e, quint64 key)
{
    const int sheet = int(key >> 40);
    return sheet == e.sheet || (e.kind == StructEdit::Kind::Move && sheet == e.toSheet);
}

// The marks of cells (a set) after the edit; the keys of those that are gone are given back.
template <class Set>
void followSet(Set *marks, const StructEdit &e, std::vector<quint64> *gone)
{
    std::vector<quint64> moved;
    for (auto it = marks->begin(); it != marks->end();) {
        if (!mayMove(e, *it)) {
            ++it;
            continue;
        }
        quint64 now = 0;
        if (!movedKey(e, *it, &now)) {
            if (gone) gone->push_back(*it);
            it = marks->erase(it);
        } else if (now != *it) {
            moved.push_back(now);
            it = marks->erase(it);
        } else {
            ++it;
        }
    }
    marks->insert(moved.begin(), moved.end());
}

// And the results kept by cell.
void followMap(std::map<quint64, Value> *results, const StructEdit &e, std::vector<std::pair<quint64, Value>> *gone)
{
    std::vector<std::pair<quint64, Value>> moved;
    for (auto it = results->begin(); it != results->end();) {
        if (!mayMove(e, it->first)) {
            ++it;
            continue;
        }
        quint64 now = 0;
        if (!movedKey(e, it->first, &now)) {
            if (gone) gone->push_back(*it);
            it = results->erase(it);
        } else if (now != it->first) {
            moved.push_back({now, it->second});
            it = results->erase(it);
        } else {
            ++it;
        }
    }
    for (auto &entry : moved) (*results)[entry.first] = std::move(entry.second);
}

// The marks of resized columns (`columns`) or rows after the edit that inserts or deletes columns or rows: those deleted are given back.
void followLines(std::set<quint64> *marks, const StructEdit &e, bool columns, std::vector<quint64> *gone)
{
    const bool applies = columns ? (e.kind == K::InsertColumns || e.kind == K::DeleteColumns) : (e.kind == K::InsertRows || e.kind == K::DeleteRows);
    if (!applies) return;
    std::vector<quint64> moved;
    for (auto it = marks->begin(); it != marks->end();) {
        const quint64 key = *it;
        if (int(key >> 40) != e.sheet) {
            ++it;
            continue;
        }
        const quint32 at = columns ? quint32(key & 0xFFFF) : quint32((key >> 16) & 0xFFFFFF);
        quint32 now = at;
        bool alive = true;
        if (e.kind == K::InsertColumns || e.kind == K::InsertRows) {
            if (at >= e.first) now = at + e.count;
            alive = now < (columns ? kMaxColumns : kMaxRows);
        } else if (at >= e.first) {
            if (at < e.first + e.count) alive = false;
            else now = at - e.count;
        }
        if (!alive) {
            if (gone) gone->push_back(key);
            it = marks->erase(it);
        } else if (now != at) {
            moved.push_back(columns ? EditSession::keyOf(e.sheet, 0, now) : EditSession::keyOf(e.sheet, now, 0));
            it = marks->erase(it);
        } else {
            ++it;
        }
    }
    marks->insert(moved.begin(), moved.end());
}

} // namespace

void EditSession::followEdit(const StructEdit &edit, StructChange *sc, bool forward)
{
    using K = StructEdit::Kind;
    // cells moved one by one, across the rows or columns of the file's sheet, are written whole where they are now
    auto markShifted = [&](const StructEdit &e, const std::function<void(quint64)> &mark) {
        if (e.kind != K::InsertCellsDown && e.kind != K::DeleteCellsUp && e.kind != K::InsertCellsRight && e.kind != K::DeleteCellsLeft) return;
        const Sheet &s = m_book->calc->sheet(e.sheet);
        const bool vertical = e.kind == K::InsertCellsDown || e.kind == K::DeleteCellsUp;
        const quint32 r1 = e.row1, c1 = e.col1;
        const quint32 r2 = vertical ? kMaxRows - 1 : e.row2, c2 = vertical ? e.col2 : kMaxColumns - 1;
        s.cells.forEachInRange(r1, c1, r2, c2, [&](const Cell &c) {
            mark(keyOf(e.sheet, c.row, c.col));
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
        // (the marks of a Move stay where they are: they say what differs from the file at a place, and the file's cells do not move with the cells)
        if (edit.kind != K::Move) {
            followSet(&m_touched, edit, &sc->touchedGone);
            followMap(&m_results, edit, &sc->resultsGone);
            followMap(&m_engineAtStart, edit, &sc->engineGone);
            followMap(&m_fileResults, edit, &sc->fileGone);
        }
        sc->columnsGone.clear();
        sc->rowsGone.clear();
        followLines(&m_columnsResized, edit, true, &sc->columnsGone);
        followLines(&m_rowsResized, edit, false, &sc->rowsGone);
        auto mark = [&](quint64 key) {
            if (m_touched.insert(key).second) sc->touchedAdded.push_back(key);
        };
        // the formulas that were written again are written whole when the file is made (what they read moved)
        const Calculation *calc = m_book->calc.get();
        m_touched.reserve(m_touched.size() + sc->record.rewritten.size());
        for (const Calculation::EditRecord::Place &p : sc->record.rewritten) {
            if (p.dataTable) continue;   // (the file's own formula is moved with its cell)
            mark(keyOf(p.sheet, p.row, p.col));
        }
        markShifted(edit, mark);
        if (edit.kind == K::Move) {
            // the places of the block and of what it went over are written whole: those that held cells, and the cells now in the new place
            for (const Calculation::EditRecord::GoneCell &g : sc->record.gone) mark(keyOf(g.sheet, g.row, g.col));
            const quint32 toRow2 = edit.toRow + (edit.row2 - edit.row1), toCol2 = edit.toCol + (edit.col2 - edit.col1);
            calc->sheet(edit.toSheet).cells.forEachInRange(edit.toRow, edit.toCol, toRow2, toCol2, [&](const Cell &c) {
                mark(keyOf(edit.toSheet, c.row, c.col));
                return true;
            });
            m_mergesChanged.insert(edit.sheet);
            m_mergesChanged.insert(edit.toSheet);
        }
        m_edits.push_back(edit);
        forgetNotes();
    } else if (sc->savedGen != m_saveGen) {
        // The file was saved since the edit was done, so it has the edit: the edit that undoes it is made against the saved file (it goes in the list for the file), and the cells that
        // come back, and those that were written over, are written whole. (A step that deleted cells is not undone after a save: see forgetDeletesInFile.)
        const StructEdit back = edit.inverse();
        if (edit.kind != K::Move) {
            followSet(&m_touched, back, nullptr);
            followMap(&m_results, back, nullptr);
            followMap(&m_engineAtStart, back, nullptr);
            followMap(&m_fileResults, back, nullptr);
            followLines(&m_columnsResized, back, true, nullptr);
            followLines(&m_rowsResized, back, false, nullptr);
        }
        auto mark = [&](quint64 key) { m_touched.insert(key); };
        for (const Calculation::EditRecord::GoneCell &g : sc->record.gone) mark(keyOf(g.sheet, g.row, g.col));
        for (const Calculation::EditRecord::Place &p : sc->record.rewritten) {
            // (the formulas that were written again have the text they had: where they are now, after the edit is undone)
            int sheet = p.sheet;
            quint32 row = p.row, col = p.col;
            if (edit.kind == K::Move || adjustCell(back, &sheet, &row, &col) != RefFate::Gone) mark(keyOf(sheet, row, col));
        }
        markShifted(back, mark);
        if (edit.kind == K::Move) {
            // the cells that went to the new place are not there any more
            const qint64 dRow = qint64(edit.toRow) - edit.row1, dCol = qint64(edit.toCol) - edit.col1;
            for (const Calculation::EditRecord::GoneCell &g : sc->record.gone)
                if (g.sheet == edit.sheet && g.row >= edit.row1 && g.row <= edit.row2 && g.col >= edit.col1 && g.col <= edit.col2) mark(keyOf(edit.toSheet, quint32(qint64(g.row) + dRow), quint32(qint64(g.col) + dCol)));
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
        if (edit.kind != K::Move) {
            followSet(&m_touched, back, nullptr);
            followMap(&m_results, back, nullptr);
            followMap(&m_engineAtStart, back, nullptr);
            followMap(&m_fileResults, back, nullptr);
        }
        followLines(&m_columnsResized, back, true, nullptr);
        followLines(&m_rowsResized, back, false, nullptr);
        for (const quint64 key : sc->columnsGone) m_columnsResized.insert(key);
        for (const quint64 key : sc->rowsGone) m_rowsResized.insert(key);
        for (const quint64 key : sc->touchedGone) m_touched.insert(key);
        for (auto &entry : sc->resultsGone) m_results[entry.first] = entry.second;
        for (auto &entry : sc->engineGone) m_engineAtStart[entry.first] = entry.second;
        for (auto &entry : sc->fileGone) m_fileResults[entry.first] = entry.second;
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
        sc.record = Calculation::EditRecord();
        ok = calc->applyEdit(sc.edit, &sc.record);
        if (ok) sc.tables = book.moveTables(sc.edit);
    } else {
        ok = calc->undoEdit(sc.record, &undone);
        if (ok) book.putTablesBack(sc.tables);
    }
    if (!ok) return false;
    if (!sc.tables.empty()) calc->tablesChanged();   // (the formulas that name a table's columns read the table where it is now)
    sc.done = forward;
    const StructEdit now = forward ? sc.edit : sc.edit.inverse();   // what was done to the cells
    followEdit(sc.edit, &sc, forward);
    {
        // the print area, the titles, and the breaks follow the cells (and come back with them)
        QStringList names;
        for (const jp::office::sheet::SheetEntry &se : book.info().sheets) names << se.name;
        if (forward) m_pages.followEdit(sc.edit, names, &sc);
        else m_pages.undoEdit(&sc);
        std::set<int> sheets = {sc.edit.sheet};
        if (sc.edit.kind == StructEdit::Kind::Move) sheets.insert(sc.edit.toSheet);
        for (const int s : sheets) Q_EMIT pageSetupChanged(s);
    }
    // the values the window holds follow the cells: those of cells that are gone are let go of, and those of formulas that moved (with their cells) go with them
    {
        const Calculation::EditRecord &done = forward ? sc.record : undone;
        std::map<int, std::vector<std::pair<quint32, quint32>>> gone;
        for (const Calculation::EditRecord::GoneCell &g : done.gone) gone[g.sheet].emplace_back(g.row, g.col);
        for (const auto &entry : gone) m_values->erase(entry.first, entry.second);
        if (now.kind != StructEdit::Kind::Move && !done.movedFormulas.empty()) {
            std::vector<ValueStore::Move> moves;
            moves.reserve(done.movedFormulas.size());
            for (const Calculation::EditRecord::MovedFormula &m : done.movedFormulas) moves.push_back({m.fromRow, m.fromCol, m.toRow, m.toCol});
            m_values->relocate(now.sheet, moves);
        }
    }
    m_log.push_back(now);
    hold->structureChanged();
    if (m_holdHook) m_holdHook(calc, true);
    hold.reset();
    m_tracking = true;
    Q_EMIT cellsMoved(now);
    if (!sc.tables.empty()) Q_EMIT tablesChanged(sc.edit.sheet);
    return true;
}

void EditSession::moveToNow(std::vector<ValueStore::Item> *items, int generation) const
{
    for (size_t g = size_t(std::max(0, generation)); g < m_log.size(); ++g) {
        const StructEdit &e = m_log[g];
        auto last = std::remove_if(items->begin(), items->end(), [&](ValueStore::Item &it) {
            int sheet = it.sheet;
            const RefFate fate = adjustCell(e, &sheet, &it.row, &it.col);
            if (fate == RefFate::Gone) return true;
            it.sheet = sheet;
            return false;
        });
        items->erase(last, items->end());
    }
}

} // namespace jp::crunch
