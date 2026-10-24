// Rows, columns, and cells inserted and deleted in an open workbook: the model does the work (Calculation::applyEdit), and this keeps what the session holds by cell in step with it: the
// values in the window's store, the marks of what changed since the file was read, the tables, and the list of edits the file will be made from. See EditSession::insertRows.
#include "core/editsession.h"

#include <QElapsedTimer>

#include <algorithm>

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
    n += qint64(record.ranges.size() + record.merges.size()) * qint64(sizeof(CellRange));
    n += qint64(record.columns.size()) * qint64(sizeof(ColumnSpan)) + qint64(record.rowSpecs.size()) * qint64(sizeof(RowSpec));
    for (const auto &name : record.names) n += qint64(name.first.size() + name.second.size()) * 2;
    n += qint64(record.rewritten.size()) * qint64(sizeof(Calculation::EditRecord::Place));
    n += qint64(touchedGone.size() + touchedAdded.size()) * 8 + qint64(tables.size()) * qint64(sizeof(TablePlace));
    n += qint64(resultsGone.size() + engineGone.size() + fileGone.size()) * qint64(sizeof(std::pair<quint64, Value>));
    return n;
}

bool EditSession::canMove(const StructEdit &edit, QString *why) const
{
    if (!canEdit(why) || !canEditSheet(edit.sheet, why)) return false;
    if (!m_book->calc->canApply(edit, why)) return false;
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
    Step step;
    step.structure = std::make_shared<StructChange>();
    step.structure->edit = edit;
    pushStep(std::move(step), text);
    return out;
}

EditSession::Outcome EditSession::insertRows(int sheet, quint32 first, quint32 count)
{
    return structure(StructEdit::insertRows(sheet, first, count), count == 1 ? tr("Inserting a row") : tr("Inserting rows"));
}

EditSession::Outcome EditSession::deleteRows(int sheet, quint32 first, quint32 count)
{
    return structure(StructEdit::deleteRows(sheet, first, count), count == 1 ? tr("Deleting a row") : tr("Deleting rows"));
}

EditSession::Outcome EditSession::insertColumns(int sheet, quint32 first, quint32 count)
{
    return structure(StructEdit::insertColumns(sheet, first, count), count == 1 ? tr("Inserting a column") : tr("Inserting columns"));
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
void followSet(std::set<quint64> *marks, const StructEdit &e, std::vector<quint64> *gone)
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

} // namespace

void EditSession::followEdit(const StructEdit &edit, StructChange *sc, bool forward)
{
    using K = StructEdit::Kind;
    if (forward) {
        sc->touchedGone.clear();
        sc->resultsGone.clear();
        sc->engineGone.clear();
        sc->fileGone.clear();
        sc->touchedAdded.clear();
        followSet(&m_touched, edit, &sc->touchedGone);
        followMap(&m_results, edit, &sc->resultsGone);
        followMap(&m_engineAtStart, edit, &sc->engineGone);
        followMap(&m_fileResults, edit, &sc->fileGone);
        auto mark = [&](quint64 key) {
            if (m_touched.insert(key).second) sc->touchedAdded.push_back(key);
        };
        // the formulas that were written again are written whole when the file is made (what they read moved)
        const Calculation *calc = m_book->calc.get();
        for (const Calculation::EditRecord::Place &p : sc->record.rewritten) {
            const Calculation::CellContent c = calc->content(p.sheet, p.row, p.col);
            if (c.dataTable) continue;   // (the file's own formula is moved with its cell)
            mark(keyOf(p.sheet, p.row, p.col));
        }
        // cells moved one by one, across the rows or columns of the file's sheet, are written whole where they are now
        if (edit.kind == K::InsertCellsDown || edit.kind == K::DeleteCellsUp || edit.kind == K::InsertCellsRight || edit.kind == K::DeleteCellsLeft) {
            const Sheet &s = calc->sheet(edit.sheet);
            const bool vertical = edit.kind == K::InsertCellsDown || edit.kind == K::DeleteCellsUp;
            const quint32 r1 = edit.row1, c1 = edit.col1;
            const quint32 r2 = vertical ? kMaxRows - 1 : edit.row2, c2 = vertical ? edit.col2 : kMaxColumns - 1;
            s.cells.forEachInRange(r1, c1, r2, c2, [&](const Cell &c) {
                mark(keyOf(edit.sheet, c.row, c.col));
                return true;
            });
        }
        m_edits.push_back(edit);
    } else {
        // (the state before the undo is the one after the edit was done: nothing was done to the cells since)
        for (const quint64 key : sc->touchedAdded) m_touched.erase(key);
        sc->touchedAdded.clear();
        const StructEdit back = edit.inverse();
        followSet(&m_touched, back, nullptr);
        followMap(&m_results, back, nullptr);
        followMap(&m_engineAtStart, back, nullptr);
        followMap(&m_fileResults, back, nullptr);
        for (const quint64 key : sc->touchedGone) m_touched.insert(key);
        for (auto &entry : sc->resultsGone) m_results[entry.first] = entry.second;
        for (auto &entry : sc->engineGone) m_engineAtStart[entry.first] = entry.second;
        for (auto &entry : sc->fileGone) m_fileResults[entry.first] = entry.second;
        if (!m_edits.empty() && m_edits.back().kind == edit.kind && m_edits.back().sheet == edit.sheet) m_edits.pop_back();
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
    if (forward) {
        sc.record = Calculation::EditRecord();
        ok = calc->applyEdit(sc.edit, &sc.record);
        if (ok) sc.tables = book.moveTables(sc.edit);
    } else {
        ok = calc->undoEdit(sc.record);
        if (ok) book.putTablesBack(sc.tables);
    }
    if (!ok) return false;
    sc.done = forward;
    const StructEdit now = forward ? sc.edit : sc.edit.inverse();   // what was done to the cells
    followEdit(sc.edit, &sc, forward);
    // the values the window holds follow the cells
    m_values->remap(now.sheet, [&](quint32 *row, quint32 *col) {
        int s = now.sheet;
        return adjustCell(now, &s, row, col) != RefFate::Gone;
    });
    m_log.push_back(now);
    hold->structureChanged();
    if (m_holdHook) m_holdHook(calc, true);
    hold.reset();
    m_tracking = true;
    Q_EMIT cellsMoved(now);
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
