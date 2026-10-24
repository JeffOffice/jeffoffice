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
    if (e.sheet < 0 || e.sheet >= int(m_sheets.size()) || e.toSheet < 0 || e.toSheet >= int(m_sheets.size())) return refuse(say("There is no such sheet."));
    if (e.row2 < e.row1 || e.col2 < e.col1 || e.row2 >= kGridRows || e.col2 >= kGridCols) return refuse(say("There is nothing to move."));
    const quint64 rows = quint64(e.row2) - e.row1 + 1, cols = quint64(e.col2) - e.col1 + 1;
    if (quint64(e.toRow) + rows > kGridRows || quint64(e.toCol) + cols > kGridCols) return refuse(say("That would push cells off the edge of the sheet."));
    if (e.sheet == e.toSheet && e.toRow == e.row1 && e.toCol == e.col1) return refuse(say("The cells are there already."));
    if (m_untracked > 0) return refuse(say("This workbook has too many formulas for JeffCrunch to follow when cells are moved."));
    const Sheet &from = m_sheets[size_t(e.sheet)], &to = m_sheets[size_t(e.toSheet)];
    if (from.isProtected || to.isProtected) return refuse(say("This sheet is protected, so cells can't be moved on it. Take the protection off in the program that put it on."));
    const quint32 toRow2 = e.toRow + quint32(rows) - 1, toCol2 = e.toCol + quint32(cols) - 1;
    const QString array = say("Cells of an array formula or a data table can't be moved, or written over.");
    for (const CellRange &r : from.ranges)
        if (r.cellCount() > 1 && meets(r, e.row1, e.col1, e.row2, e.col2)) return refuse(array);
    for (const CellRange &r : to.ranges)
        if (r.cellCount() > 1 && meets(r, e.toRow, e.toCol, toRow2, toCol2)) return refuse(array);
    return true;
}

bool Calculation::applyMove(const StructEdit &e, EditRecord *rec)
{
    abandonNow();
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
    m_graph->dependentsOfArea(area, &candidates);
    for (quint32 id = 0; id < quint32(m_nodes.size()); ++id) {
        const Node &n = m_nodes[id];
        if (n.flags & Dead) continue;
        if ((n.flags & Told) || ((n.flags & Kept) && n.kept != 'U')) candidates.push_back(id);
    }
    std::sort(candidates.begin(), candidates.end());
    candidates.erase(std::unique(candidates.begin(), candidates.end()), candidates.end());

    // ---- the cells of the block, with the formulas among them written to read where the cells will be
    struct Moving {
        quint32 row, col;
        CellContent content;
    };
    std::vector<Moving> moving;
    std::vector<std::pair<quint32, quint32>> overwritten;
    S.cells.forEachInRange(e.row1, e.col1, e.row2, e.col2, [&](const Cell &c) {
        Moving m;
        m.row = c.row;
        m.col = c.col;
        m.content = content(e.sheet, c.row, c.col);
        rec->gone.push_back({e.sheet, c.row, c.col, m.content});
        if (m.content.isFormula) {
            bool changed = false;
            const QString now = adjustFormula(m.content.formula, e.sheet, names, e, &changed);
            if (changed) m.content.formula = now;
        }
        moving.push_back(std::move(m));
        return true;
    });
    D.cells.forEachInRange(e.toRow, e.toCol, toRow2, toCol2, [&](const Cell &c) {
        if (inSource(e.toSheet, c.row, c.col)) return true;   // (already counted: it is one of the block's)
        rec->gone.push_back({e.toSheet, c.row, c.col, content(e.toSheet, c.row, c.col)});
        overwritten.push_back({c.row, c.col});
        return true;
    });
    rec->cellsMoved = qint64(moving.size());

    // ---- the formulas outside the block that read it are written again (before the cells move, while the graph still knows who reads what)
    struct Rewrite {
        quint32 id;
        QString was, now;
    };
    std::vector<Rewrite> rewrites;
    for (const quint32 id : candidates) {
        const Node &n = m_nodes[id];
        if (n.flags & Dead) continue;
        if (inSource(n.sheet, n.row, n.col) || inDestination(n.sheet, n.row, n.col)) continue;
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

    // ---- both places are emptied, and the cells put where they go
    auto clear = [&](int sheet, quint32 row, quint32 col) {
        setValue(sheet, row, col, Value(), Marking::Later);
        setStyle(sheet, row, col, 0);
    };
    for (const Moving &m : moving) clear(e.sheet, m.row, m.col);
    for (const auto &p : overwritten) clear(e.toSheet, p.first, p.second);
    for (const Moving &m : moving) put(e.toSheet, quint32(qint64(m.row) + dRow), quint32(qint64(m.col) + dCol), m.content, Marking::Later);

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
        Cell *c = sn.cells.findMutable(n0.row, n0.col);
        if (!c || !c->hasFormula()) continue;
        if (!r.now.isEmpty()) {
            EditRecord::FormulaWas was;
            was.sheet = sheet;
            was.row = c->row;
            was.col = c->col;
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
            rec->rewritten.push_back({sheet, c->row, c->col});
        }
        dropNode(r.id);
        const quint32 id = addNode(sheet, *c, c->formula);
        queueDependents(areaOf(m_nodes[id]));
    }
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

bool Calculation::undoMove(const EditRecord &rec)
{
    const StructEdit &e = rec.edit;
    abandonNow();
    const quint32 rows = e.row2 - e.row1 + 1, cols = e.col2 - e.col1 + 1;
    const quint32 toRow2 = e.toRow + rows - 1, toCol2 = e.toCol + cols - 1;
    auto clearRange = [&](int sheet, quint32 row1, quint32 col1, quint32 row2, quint32 col2) {
        std::vector<std::pair<quint32, quint32>> there;
        m_sheets[size_t(sheet)].cells.forEachInRange(row1, col1, row2, col2, [&](const Cell &c) {
            there.push_back({c.row, c.col});
            return true;
        });
        for (const auto &p : there) {
            setValue(sheet, p.first, p.second, Value(), Marking::Later);
            setStyle(sheet, p.first, p.second, 0);
        }
    };
    clearRange(e.toSheet, e.toRow, e.toCol, toRow2, toCol2);
    clearRange(e.sheet, e.row1, e.col1, e.row2, e.col2);
    for (const EditRecord::GoneCell &g : rec.gone) put(g.sheet, g.row, g.col, g.content, Marking::Later);
    for (const EditRecord::FormulaWas &w : rec.was) {
        const Value *stored = w.hasStored ? &w.stored : nullptr;
        if (rewriteArrayText(w.sheet, w.row, w.col, w.text)) continue;
        setFormula(w.sheet, w.row, w.col, w.text, Marking::Later, stored);
    }
    for (const auto &entry : rec.validations)
        if (entry.first >= 0 && entry.first < int(m_sheets.size())) m_sheets[size_t(entry.first)].validations = entry.second;
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
