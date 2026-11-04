#include "core/subtotal.h"

#include "calc/value.h"
#include "core/formulaspot.h"
#include "core/outline.h"
#include "sheet/cellref.h"

#include <QCoreApplication>
#include <QUndoStack>

#include <algorithm>

namespace jp::crunch {

using namespace jp::office::sheet;
using jp::calc::Value;

namespace {

constexpr int kMaxSubtotalRuns = 2000;

EditSession::Outcome refuse(const QString &why)
{
    EditSession::Outcome out;
    out.ok = false;
    out.problem = why;
    return out;
}

QString keyOf(const EditSession &s, int sheet, quint32 row, quint32 col)
{
    const Value v = s.valueAt(sheet, row, col);
    if (v.isEmpty()) return QString();
    if (v.isText()) return v.text().toCaseFolded();
    if (v.isNumber()) return jp::calc::numberToText(v.number());
    return s.editText(sheet, row, col).toCaseFolded();
}

QString labelOf(const EditSession &s, int sheet, quint32 row, quint32 col)
{
    const Value v = s.valueAt(sheet, row, col);
    if (v.isText()) return v.text();
    if (v.isNumber()) return jp::calc::numberToText(v.number());
    return s.editText(sheet, row, col);
}

// A row of the list that is a subtotal: text and SUBTOTAL formulas only, with a formula at least.
bool isSubtotalRow(const EditSession &s, int sheet, quint32 row, const CellRange &region)
{
    bool formula = false;
    for (quint32 col = region.col1; col <= region.col2; ++col) {
        const Value v = s.valueAt(sheet, row, col);
        const QString held = s.editText(sheet, row, col);
        if (held.startsWith(QLatin1String("=SUBTOTAL("), Qt::CaseInsensitive)) {
            formula = true;
            continue;
        }
        if (v.isEmpty() || (v.isText() && !held.startsWith(QLatin1Char('=')))) continue;
        return false;
    }
    return formula;
}

} // namespace

const std::vector<SubtotalFunction> &subtotalFunctions()
{
    static const std::vector<SubtotalFunction> all = {
        {9, QT_TRANSLATE_NOOP("jp::crunch::Subtotal", "Sum")},
        {3, QT_TRANSLATE_NOOP("jp::crunch::Subtotal", "Count")},
        {1, QT_TRANSLATE_NOOP("jp::crunch::Subtotal", "Average")},
        {4, QT_TRANSLATE_NOOP("jp::crunch::Subtotal", "Max")},
        {5, QT_TRANSLATE_NOOP("jp::crunch::Subtotal", "Min")},
        {6, QT_TRANSLATE_NOOP("jp::crunch::Subtotal", "Product")},
        {2, QT_TRANSLATE_NOOP("jp::crunch::Subtotal", "Count Numbers")},
        {7, QT_TRANSLATE_NOOP("jp::crunch::Subtotal", "StdDev")},
        {8, QT_TRANSLATE_NOOP("jp::crunch::Subtotal", "StdDevp")},
        {10, QT_TRANSLATE_NOOP("jp::crunch::Subtotal", "Var")},
        {11, QT_TRANSLATE_NOOP("jp::crunch::Subtotal", "Varp")},
    };
    return all;
}

EditSession::Outcome removeSubtotals(EditSession &session, int sheet, CellRange region, int *removed)
{
    if (removed) *removed = 0;
    if (!region.valid) return refuse(QCoreApplication::translate("jp::crunch::Subtotal", "Select a cell in the list first."));
    QString why;
    if (!session.canEdit(&why) || !session.canEditSheet(sheet, &why)) return refuse(why);
    std::vector<quint32> rows;
    for (quint32 row = region.row1 + 1; row <= region.row2; ++row)
        if (isSubtotalRow(session, sheet, row, region)) rows.push_back(row);
    if (rows.empty()) return EditSession::Outcome();
    QUndoStack *stack = session.undoStack();
    stack->beginMacro(QCoreApplication::translate("jp::crunch::Subtotal", "Remove All Subtotals"));
    EditSession::Outcome out;
    // (from the bottom up, a run of rows at a time, so that the rows still to go keep their places)
    for (size_t i = rows.size(); i > 0 && out.ok;) {
        size_t first = i - 1;
        while (first > 0 && rows[first - 1] + 1 == rows[first]) --first;
        out = session.deleteRows(sheet, rows[first], quint32(i - first));
        i = first;
    }
    if (out.ok) {
        // the rows left lose the level the subtotals gave them
        const quint32 last = region.row2 - quint32(rows.size());
        std::vector<OutlineChange> changes;
        const Outline outline = session.outlineOf(sheet, false);
        for (quint32 row = region.row1 + 1; row <= last; ++row)
            if (outline.levelOf(row) > 0) changes.push_back({row, 0, 0, 0});
        if (!changes.empty()) out = session.changeOutline(sheet, false, changes, QCoreApplication::translate("jp::crunch::Subtotal", "Remove All Subtotals"));
    }
    stack->endMacro();
    if (!out.ok) {
        stack->undo();
        return out;
    }
    if (removed) *removed = int(rows.size());
    return EditSession::Outcome();
}

EditSession::Outcome applySubtotals(EditSession &session, int sheet, CellRange region, const SubtotalSpec &spec, int *groups)
{
    if (groups) *groups = 0;
    if (!region.valid || region.row2 <= region.row1) return refuse(QCoreApplication::translate("jp::crunch::Subtotal", "Select a cell in a list that has a header row and rows of data."));
    if (spec.column < region.col1 || spec.column > region.col2) return refuse(QCoreApplication::translate("jp::crunch::Subtotal", "Choose a column of the list to add subtotals at."));
    if (spec.columns.empty()) return refuse(QCoreApplication::translate("jp::crunch::Subtotal", "Choose at least one column to add subtotals to."));
    QString why;
    if (!session.canEdit(&why) || !session.canEditSheet(sheet, &why)) return refuse(why);
    bool known = false;
    for (const SubtotalFunction &f : subtotalFunctions()) known = known || f.code == spec.function;
    if (!known) return refuse(QCoreApplication::translate("jp::crunch::Subtotal", "Choose a function to use."));
    QUndoStack *stack = session.undoStack();
    stack->beginMacro(QCoreApplication::translate("jp::crunch::Subtotal", "Subtotal"));
    EditSession::Outcome out;
    if (spec.replace) {
        int removed = 0;
        out = removeSubtotals(session, sheet, region, &removed);
        region.row2 -= quint32(removed);
    }
    // the runs of equal entries
    struct Run {
        quint32 first, last;
    };
    std::vector<Run> runs;
    if (out.ok) {
        for (quint32 row = region.row1 + 1; row <= region.row2; ++row) {
            if (!runs.empty() && keyOf(session, sheet, row, spec.column) == keyOf(session, sheet, runs.back().last, spec.column)) runs.back().last = row;
            else runs.push_back({row, row});
            if (runs.size() > size_t(kMaxSubtotalRuns)) {
                out = refuse(QCoreApplication::translate("jp::crunch::Subtotal", "The list has too many runs of equal entries to total at once. Sort it so that fewer entries change, or total part of it."));
                break;
            }
        }
    }
    // the rows go in from the bottom up (the grand total first), so that the runs above keep their places
    const quint32 lastData = region.row2;
    if (out.ok) out = session.insertRows(sheet, lastData + 1, 1);
    for (size_t i = runs.size(); i > 0 && out.ok; --i) out = session.insertRows(sheet, runs[i - 1].last + 1, 1);
    const QChar separator = argumentSeparator(session.locale());
    auto formula = [&](quint32 col, quint32 first, quint32 last) {
        return QStringLiteral("=SUBTOTAL(%1%2%3%4:%5%6)").arg(spec.function).arg(separator, columnName(col)).arg(first + 1).arg(columnName(col)).arg(last + 1);
    };
    // (the rows are now where the runs were, each moved down by the subtotal rows above it)
    for (size_t i = 0; i < runs.size() && out.ok; ++i) {
        const quint32 shift = quint32(i);
        const quint32 first = runs[i].first + shift, last = runs[i].last + shift, at = last + 1;
        const QString label = labelOf(session, sheet, first, spec.column);
        out = session.type(sheet, at, spec.column, QCoreApplication::translate("jp::crunch::Subtotal", "%1 Total").arg(label), false);
        for (size_t c = 0; c < spec.columns.size() && out.ok; ++c)
            if (spec.columns[c] != spec.column) out = session.type(sheet, at, spec.columns[c], formula(spec.columns[c], first, last), false);
    }
    if (out.ok) {
        const quint32 at = lastData + quint32(runs.size()) + 1, first = region.row1 + 1;
        out = session.type(sheet, at, spec.column, QCoreApplication::translate("jp::crunch::Subtotal", "Grand Total"), false);
        for (size_t c = 0; c < spec.columns.size() && out.ok; ++c)
            if (spec.columns[c] != spec.column) out = session.type(sheet, at, spec.columns[c], formula(spec.columns[c], first, at - 1), false);
    }
    // each run is a group of the outline, its subtotal the summary below
    for (size_t i = 0; i < runs.size() && out.ok; ++i) {
        const quint32 shift = quint32(i);
        const Outline outline = session.outlineOf(sheet, false);
        const auto changes = outline.group(runs[i].first + shift, runs[i].last + shift);
        if (!changes.empty()) out = session.changeOutline(sheet, false, changes, QCoreApplication::translate("jp::crunch::Subtotal", "Subtotal"));
    }
    stack->endMacro();
    if (!out.ok) {
        stack->undo();
        return out;
    }
    if (groups) *groups = int(runs.size());
    return EditSession::Outcome();
}

} // namespace jp::crunch
