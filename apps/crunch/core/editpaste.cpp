// Copy, paste, and cut and paste: the cells of a range as the clipboard holds them, and the one step a paste is. See EditSession::paste.
#include "core/editsession.h"

#include "sheet/cellref.h"

#include <QCoreApplication>

#include <algorithm>
#include <unordered_map>

namespace jp::crunch {

using namespace jp::office;
using namespace jp::office::sheet;
using jp::calc::Value;

namespace {

quint64 placeKey(quint32 row, quint32 col) { return (quint64(row) << 32) | col; }

bool sameMeaning(const Calculation::CellContent &a, const Calculation::CellContent &b)
{
    return a.exists == b.exists && a.isFormula == b.isFormula && a.formula == b.formula && a.value == b.value && a.style == b.style;
}

// The result of an operation made with a number on a number; an error for a division by nothing.
Value operate(EditSession::PasteOptions::Operation op, double destination, double given)
{
    using Op = EditSession::PasteOptions::Operation;
    switch (op) {
    case Op::Add: return Value::number(destination + given);
    case Op::Subtract: return Value::number(destination - given);
    case Op::Multiply: return Value::number(destination * given);
    case Op::Divide: return given == 0 ? Value::error(jp::calc::ErrorKind::Div0) : Value::number(destination / given);
    case Op::None: break;
    }
    return Value::number(destination);
}

const char *operatorOf(EditSession::PasteOptions::Operation op)
{
    using Op = EditSession::PasteOptions::Operation;
    switch (op) {
    case Op::Add: return "+";
    case Op::Subtract: return "-";
    case Op::Multiply: return "*";
    case Op::Divide: return "/";
    case Op::None: break;
    }
    return "+";
}

} // namespace

ClipData EditSession::copyCells(int sheet, const CellRange &range, bool cut) const
{
    ClipData d;
    d.token = m_token;
    d.sheet = sheet;
    d.source = range;
    d.cut = cut;
    if (!m_book || !m_book->calc || !m_book->book || !range.valid || sheet < 0 || sheet >= m_book->calc->sheetCount()) return d;
    const Calculation *calc = m_book->calc.get();
    const Sheet &s = calc->sheet(sheet);
    // (the rows and columns of the selection past the last the sheet has cells in hold nothing: a whole column is its used rows)
    const quint32 lastRow = std::min(range.row2, std::max(range.row1, s.extentRows > 0 ? s.extentRows - 1 : 0));
    const quint32 lastCol = std::min(range.col2, std::max(range.col1, s.extentCols > 0 ? s.extentCols - 1 : 0));
    const quint64 rows = quint64(lastRow) - range.row1 + 1, cols = quint64(lastCol) - range.col1 + 1;
    if (rows * cols > quint64(kMaxCellsPerEdit)) return d;
    d.rows = quint32(rows);
    d.cols = quint32(cols);
    const QLocale usual(QLocale::English, QLocale::UnitedStates);
    s.cells.forEachInRange(range.row1, range.col1, lastRow, lastCol, [&](quint32 row, quint32 col, const Cell &c) {
        ClipCell cc;
        cc.dRow = row - range.row1;
        cc.dCol = col - range.col1;
        cc.content = calc->content(sheet, row, col);
        if (!cc.content.exists) return true;
        cc.shown = valueAt(sheet, row, col);
        if (!cc.content.editable && !cc.content.isFormula) cc.content.value = cc.shown;   // (a cell of an array holds the result)
        // a formula the format hides on a protected sheet is not given away by copying: what is copied is what it is worth
        if (cc.content.isFormula && formulaHidden(sheet, cc.content.style)) {
            cc.content.isFormula = false;
            cc.content.formula.clear();
            cc.content.value = cc.shown;
        }
        if (cc.content.style != 0) cc.formatCode = m_book->book->styles().formatCode(cc.content.style, usual);
        cc.content.arrayRange = CellRange();
        d.cells.push_back(std::move(cc));
        return true;
    });
    for (const CellRange &m : s.merges)
        if (m.row1 >= range.row1 && m.row2 <= lastRow && m.col1 >= range.col1 && m.col2 <= lastCol) {
            CellRange r = m;
            r.row1 -= range.row1;
            r.row2 -= range.row1;
            r.col1 -= range.col1;
            r.col2 -= range.col1;
            d.merges.push_back(r);
        }
    return d;
}

EditSession::Outcome EditSession::pasteChanges(int sheet, const CellRange &target, quint32 blockRows, quint32 blockCols, const PasteMaker &make, const std::vector<CellRange> &merges, bool withMerges,
                                               const QString &text)
{
    Outcome out;
    QString why;
    auto refuse = [&](const QString &problem) {
        out.ok = false;
        out.problem = problem;
        return out;
    };
    if (!canEdit(&why) || !canEditSheet(sheet, &why)) return refuse(why);
    if (blockRows == 0 || blockCols == 0 || !target.valid) return out;
    const quint32 targetRows = target.row2 - target.row1 + 1, targetCols = target.col2 - target.col1 + 1;
    quint32 tilesRows = 1, tilesCols = 1;
    if (targetRows >= blockRows && targetCols >= blockCols && targetRows % blockRows == 0 && targetCols % blockCols == 0) {
        tilesRows = targetRows / blockRows;
        tilesCols = targetCols / blockCols;
    }
    const quint64 totalRows = quint64(blockRows) * tilesRows, totalCols = quint64(blockCols) * tilesCols;
    if (target.row1 + totalRows > kMaxRows || target.col1 + totalCols > kMaxColumns) return refuse(tr("That would go past the edge of the sheet. Select a cell nearer its start."));
    if (totalRows * totalCols > quint64(kMaxCellsPerEdit))
        return refuse(tr("That is more cells than JeffCrunch pastes at once (%1). Paste into fewer.").arg(QLocale().toString(kMaxCellsPerEdit)));
    const Calculation *calc = m_book->calc.get();
    Step step;
    // (a pasted value is not held to the rules of validation, as in the programs people know: only what is typed is)
    for (quint32 tr_ = 0; tr_ < tilesRows; ++tr_)
        for (quint32 tc = 0; tc < tilesCols; ++tc)
            for (quint32 dr = 0; dr < blockRows; ++dr)
                for (quint32 dc = 0; dc < blockCols; ++dc) {
                    PasteSpot spot;
                    spot.row = target.row1 + tr_ * blockRows + dr;
                    spot.col = target.col1 + tc * blockCols + dc;
                    spot.dRow = dr;
                    spot.dCol = dc;
                    CellChange ch;
                    ch.sheet = sheet;
                    ch.row = spot.row;
                    ch.col = quint16(spot.col);
                    ch.before = calc->content(sheet, spot.row, spot.col);
                    ch.after = ch.before;
                    Outcome problem;
                    if (!make(spot, &ch, &problem)) return problem;
                    if (sameMeaning(ch.before, ch.after) && ch.formatCode.isEmpty()) continue;
                    if (!ch.before.editable) return refuse(tr("You can't change part of an array or a data table."));
                    if (isLocked(sheet, ch.before.exists ? ch.before.style : implicitStyle(sheet, spot.row, spot.col))) return refuse(lockedMessage());
                    if (m_book->book->tableHeaderAt(sheet, spot.row, spot.col, nullptr) >= 0)
                        return refuse(tr("The first row of a table holds the names of its columns, so cells can't be pasted over it. Paste below it, or change the names one by one."));
                    step.cells.push_back(std::move(ch));
                }
    if (withMerges && qint64(merges.size()) * qint64(tilesRows) * qint64(tilesCols) > jp::office::sheet::kMaxMerges)
        return refuse(tr("That would make more merged cells than JeffCrunch makes at once. Paste into fewer cells."));
    if (withMerges && calc->mergeCountOf(sheet) == qint64(calc->mergesOf(sheet).size())) {
        MergeEdit m;
        m.sheet = sheet;
        m.before = calc->mergesOf(sheet);
        m.countBefore = calc->mergeCountOf(sheet);
        const quint32 row2 = target.row1 + quint32(totalRows) - 1, col2 = target.col1 + quint32(totalCols) - 1;
        for (const CellRange &r : m.before)
            if (!(r.row2 < target.row1 || r.row1 > row2 || r.col2 < target.col1 || r.col1 > col2)) continue;   // (written over)
            else m.after.append(r);
        for (quint32 tr_ = 0; tr_ < tilesRows; ++tr_)
            for (quint32 tc = 0; tc < tilesCols; ++tc)
                for (const CellRange &r : merges) {
                    CellRange placed = r;
                    placed.row1 += target.row1 + tr_ * blockRows;
                    placed.row2 += target.row1 + tr_ * blockRows;
                    placed.col1 += target.col1 + tc * blockCols;
                    placed.col2 += target.col1 + tc * blockCols;
                    m.after.append(placed);
                }
        m.countAfter = m.after.size();
        if (!(m.after == m.before)) step.merges.push_back(m);
    }
    out.range.valid = true;
    out.range.row1 = target.row1;
    out.range.col1 = target.col1;
    out.range.row2 = target.row1 + quint32(totalRows) - 1;
    out.range.col2 = target.col1 + quint32(totalCols) - 1;
    if (step.cells.empty() && step.merges.empty()) return out;
    pushStep(std::move(step), text);
    return out;
}

EditSession::Outcome EditSession::paste(int sheet, const CellRange &target, const ClipData &clip, const PasteOptions &o)
{
    Outcome out;
    if (!clip.valid()) {
        out.ok = false;
        out.problem = tr("There is nothing to paste.");
        return out;
    }
    if (!m_book || !m_book->book || !m_book->calc) {
        out.ok = false;
        out.problem = tr("The workbook is still being opened. It can be changed when it is open.");
        return out;
    }
    const bool same = clip.token == m_token && clip.token != 0;
    using What = PasteOptions::What;
    using Op = PasteOptions::Operation;
    std::unordered_map<quint64, const ClipCell *> at;
    for (const ClipCell &c : clip.cells) at.emplace(placeKey(c.dRow, c.dCol), &c);
    const bool hasShared = !m_book->book->sharedStringsPart().isEmpty();
    const quint32 blockRows = o.transpose ? clip.cols : clip.rows, blockCols = o.transpose ? clip.rows : clip.cols;
    PasteMaker make = [&](const PasteSpot &spot, CellChange *ch, Outcome *) {
        const quint32 sr = o.transpose ? spot.dCol : spot.dRow, sc = o.transpose ? spot.dRow : spot.dCol;   // the place in the copy
        const auto found = at.find(placeKey(sr, sc));
        const ClipCell *src = found == at.end() ? nullptr : found->second;
        const bool hasValue = src && src->content.exists && (src->content.isFormula || !src->content.value.isEmpty());
        if (o.skipBlanks && !hasValue) return true;
        const Calculation::CellContent &before = ch->before;
        Calculation::CellContent &after = ch->after;
        const qint64 dRow = qint64(spot.row) - (qint64(clip.source.row1) + sr), dCol = qint64(spot.col) - (qint64(clip.source.col1) + sc);
        auto setStyleFrom = [&] {
            if (same) {
                after.style = src ? src->content.style : 0;
            } else {
                after.style = 0;
                if (src && !src->formatCode.isEmpty() && src->formatCode != QLatin1String("General")) ch->formatCode = src->formatCode;
            }
        };
        auto clearContent = [&] {
            after.isFormula = false;
            after.formula.clear();
            after.value = Value();
            after.sharedString = -1;
            after.hasStored = false;
        };
        if (o.operation != Op::None && o.what != What::Formats) {
            // the copy's number is made with what is there
            const Value given = src ? (src->content.isFormula ? src->shown : src->content.value) : Value();
            if (!given.isNumber()) return true;
            if (before.isFormula) {
                after.formula = QLatin1Char('(') + before.formula + QLatin1Char(')') + QLatin1String(operatorOf(o.operation)) + jp::calc::numberToText(given.number());
                after.hasStored = false;
            } else if (before.value.isNumber() || before.value.isEmpty()) {
                after.value = operate(o.operation, before.value.isNumber() ? before.value.number() : 0, given.number());
                after.sharedString = -1;
                after.exists = true;
            }
            return true;
        }
        switch (o.what) {
        case What::All:
        case What::Formulas:
        case What::Values:
            if (!hasValue) {
                clearContent();
            } else if (o.what == What::Values) {
                after.isFormula = false;
                after.formula.clear();
                after.value = src->content.isFormula ? src->shown : src->content.value;
                after.sharedString = -1;
                after.hasStored = false;
            } else {
                after.isFormula = src->content.isFormula;
                after.formula = src->content.isFormula ? offsetFormula(src->content.formula, int(dRow), int(dCol)) : QString();
                // (a copy made in this workbook carries the formula as it is held, for put to take with the distance changed: only if the text was offset by the distance from the cell it was read from)
                if (same && src->content.isFormula) after.copyOf(src->content, sheet, spot.row, spot.col, dRow, dCol);
                after.value = src->content.isFormula ? Value() : src->content.value;
                after.sharedString = -1;
                after.hasStored = false;
            }
            if (o.what == What::All) setStyleFrom();
            break;
        case What::Formats: setStyleFrom(); break;
        }
        if (after.value.isText() && !after.isFormula) ch->textToShare = hasShared;
        after.exists = after.isFormula || !after.value.isEmpty() || after.style != 0 || !ch->formatCode.isEmpty();
        return true;
    };
    std::vector<CellRange> merges;
    for (CellRange m : clip.merges) {
        if (o.transpose) {
            std::swap(m.row1, m.col1);
            std::swap(m.row2, m.col2);
        }
        merges.push_back(m);
    }
    return pasteChanges(sheet, target, blockRows, blockCols, make, merges, o.what == What::All || o.what == What::Formats, tr("Pasting"));
}

EditSession::Outcome EditSession::pasteText(int sheet, const CellRange &target, const QVector<QStringList> &table, const PasteOptions &o)
{
    Outcome out;
    if (!m_book || !m_book->book || !m_book->calc || table.isEmpty()) {
        out.ok = !table.isEmpty() ? false : true;
        if (!out.ok) out.problem = tr("The workbook is still being opened. It can be changed when it is open.");
        return out;
    }
    if (o.what == PasteOptions::What::Formats) return out;   // (text has no formats)
    quint32 width = 0;
    for (const QStringList &row : table) width = std::max<quint32>(width, quint32(row.size()));
    const quint32 blockRows = o.transpose ? width : quint32(table.size()), blockCols = o.transpose ? quint32(table.size()) : width;
    using Op = PasteOptions::Operation;
    PasteMaker make = [&](const PasteSpot &spot, CellChange *ch, Outcome *) {
        const quint32 sr = o.transpose ? spot.dCol : spot.dRow, sc = o.transpose ? spot.dRow : spot.dCol;
        const QString text = sr < quint32(table.size()) && sc < quint32(table[int(sr)].size()) ? table[int(sr)][int(sc)] : QString();
        if (o.skipBlanks && text.isEmpty()) return true;
        CellChange typed;
        bool none = false;
        Outcome problem;
        if (!typedChange(ch->sheet, spot.row, spot.col, text, &typed, &none, &problem)) {
            // (a text that begins like a formula and is none, or is not a number: it is text)
            typed = *ch;
            typed.after.exists = true;
            typed.after.isFormula = false;
            typed.after.formula.clear();
            typed.after.value = Value::text(text);
            typed.after.sharedString = -1;
            typed.textToShare = !m_book->book->sharedStringsPart().isEmpty();
        }
        if (o.operation != Op::None) {
            // (the number pasted is made with what is there)
            if (!typed.after.value.isNumber() || typed.after.isFormula) return true;
            const double given = typed.after.value.number();
            CellChange made = *ch;
            if (ch->before.isFormula) {
                made.after.formula = QLatin1Char('(') + ch->before.formula + QLatin1Char(')') + QLatin1String(operatorOf(o.operation)) + jp::calc::numberToText(given);
                made.after.hasStored = false;
            } else if (ch->before.value.isNumber() || ch->before.value.isEmpty()) {
                made.after.value = operate(o.operation, ch->before.value.isNumber() ? ch->before.value.number() : 0, given);
                made.after.sharedString = -1;
                made.after.exists = true;
            }
            *ch = made;
            return true;
        }
        *ch = typed;
        return true;
    };
    return pasteChanges(sheet, target, blockRows, blockCols, make, {}, false, tr("Pasting"));
}

EditSession::Outcome EditSession::moveCells(int sheet, const CellRange &source, int toSheet, quint32 toRow, quint32 toCol)
{
    return structure(StructEdit::move(sheet, source, toSheet, toRow, toCol), tr("Moving cells"));
}

} // namespace jp::crunch
