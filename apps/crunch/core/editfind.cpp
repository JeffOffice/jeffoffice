// Find and Replace: the text a cell shows, and Replace as one step. See EditSession::replaceInCells; the search itself is FindRun.
#include "core/editsession.h"

#include "calc/numfmt.h"
#include "grid/text.h"
#include "sheet/cellref.h"

#include <set>

namespace jp::crunch {

using namespace jp::office::sheet;
using jp::calc::Value;

QString EditSession::displayText(int sheet, quint32 row, quint32 col) const
{
    if (!m_book || !m_book->calc || !m_book->book || sheet < 0 || sheet >= m_book->calc->sheetCount()) return {};
    const Cell *c = m_book->calc->sheet(sheet).cells.find(row, col);
    if (!c) return {};
    const Value v = valueAt(sheet, row, col);
    switch (v.kind()) {
    case Value::Kind::Empty: return {};
    case Value::Kind::Text: return v.text();
    case Value::Kind::Bool: return v.boolean() ? QStringLiteral("TRUE") : QStringLiteral("FALSE");
    case Value::Kind::Error: return jp::calc::errorKindText(v.errorKind());
    case Value::Kind::Array: return {};
    case Value::Kind::Number: break;
    }
    const jp::calc::NumberFormat format(m_book->book->styles().formatCode(c->style, QLocale::c()));
    const jp::calc::NumberSymbols symbols = jp::calc::NumberSymbols::of(m_locale);
    const jp::calc::Formatted f = format.format(v.number(), m_book->book->info().dateSystem, 0, &symbols);
    return f.failed ? jp::calc::numberToText(v.number()) : f.text;
}

QString EditSession::searchText(int sheet, quint32 row, quint32 col) const
{
    if (!m_book || !m_book->calc || !m_book->book || sheet < 0 || sheet >= m_book->calc->sheetCount()) return {};
    const Sheet &s = m_book->calc->sheet(sheet);
    const Cell *c = s.cells.find(row, col);
    if (!c) return {};
    if (c->hasFormula() && !(c->flags & (ArrayMember))) {
        const Calculation::CellContent content = m_book->calc->content(sheet, row, col);
        return content.isFormula ? QLatin1Char('=') + formulaToLocale(jp::grid::displayFormula(content.formula), m_locale) : QString();
    }
    switch (c->type) {
    case CellType::Empty: return {};
    case CellType::SharedString: return m_book->book->sharedStrings().at(c->textIndex());
    case CellType::InlineString:
    case CellType::FormulaString: return s.texts.value(c->textIndex());
    default: break;
    }
    const Calculation::CellContent content = m_book->calc->content(sheet, row, col);
    const QString code = m_book->book->styles().formatCode(content.style, m_locale);
    return editTextOfValue(content.value, code, entryOptions(sheet, code == QLatin1String("@")));
}

struct EditSession::ReplacePlan {
    std::vector<CellSpot> cells;
    TextPattern pattern;
    QString with;
    std::unique_ptr<TextMatcher> matcher;
    size_t next = 0;
    Step step;
    int occurrences = 0;
    std::set<int> checked;
    QString problem;            // a cell cannot be changed: why (nothing is changed then)
    bool done = false;
    qint64 changesAtPlan = 0;
    int undoIndexAtPlan = 0;
};

std::shared_ptr<EditSession::ReplacePlan> EditSession::planReplace(const std::vector<CellSpot> &cells, const TextPattern &pattern, const QString &with, QString *why)
{
    auto refuse = [&](const QString &problem) {
        if (why) *why = problem;
        return std::shared_ptr<ReplacePlan>();
    };
    QString reason;
    if (!canEdit(&reason)) return refuse(reason);
    auto plan = std::make_shared<ReplacePlan>();
    plan->matcher = std::make_unique<TextMatcher>(pattern);
    if (!plan->matcher->valid()) return refuse(tr("Type the text to look for."));
    if (qint64(cells.size()) > kMaxCellsPerEdit)
        return refuse(tr("That is more cells than JeffCrunch changes at once (%1). Narrow the search, and replace again.").arg(QLocale().toString(kMaxCellsPerEdit)));
    plan->cells = cells;
    plan->pattern = pattern;
    plan->with = with;
    plan->changesAtPlan = m_loader->changesMade();
    plan->undoIndexAtPlan = m_undo.index();
    return plan;
}

bool EditSession::prepareReplace(ReplacePlan &plan, qint64 budget)
{
    if (plan.done) return true;
    const Calculation *calc = m_book->calc.get();
    qint64 spent = 0;
    for (; plan.next < plan.cells.size(); ++plan.next) {
        if (budget > 0 && spent >= budget) return false;
        ++spent;
        const CellSpot &spot = plan.cells[plan.next];
        QString why;
        if (plan.checked.insert(spot.sheet).second && !canEditSheet(spot.sheet, &why)) {
            plan.problem = why;
            break;
        }
        const QString where = cellName(spot.row, spot.col);
        const Cell *cell = calc->sheet(spot.sheet).cells.find(spot.row, spot.col);
        if (!cell) continue;
        // (the text a cell is typed as: a text constant is what it holds, anything else as the formula bar shows it)
        const bool plainText = !cell->hasFormula() && (cell->type == CellType::SharedString || cell->type == CellType::InlineString || cell->type == CellType::FormulaString);
        const QString text = plainText ? searchText(spot.sheet, spot.row, spot.col) : editText(spot.sheet, spot.row, spot.col);
        int n = 0;
        QString now = plan.matcher->replaced(text, plan.with, &n);
        if (n == 0) continue;
        // (a text constant stays a text where the new text begins with a mark that would make it a formula or an apostrophe-led text when typed)
        if (plainText && (now.startsWith(QLatin1Char('=')) || now.startsWith(QLatin1Char('\''))) ) now.prepend(QLatin1Char('\''));
        const Calculation::CellContent before = calc->content(spot.sheet, spot.row, spot.col);
        if (!before.exists) continue;
        if (!before.editable) {
            plan.problem = tr("%1 is part of an array or a data table, so it can't be changed one cell at a time. Nothing was replaced.").arg(where);
            break;
        }
        if (isLocked(spot.sheet, before.style)) {
            plan.problem = tr("%1 is locked, and its sheet is protected, so it can't be changed. Nothing was replaced.").arg(where);
            break;
        }
        if (m_book->book->tableHeaderAt(spot.sheet, spot.row, spot.col, nullptr) >= 0) {
            plan.problem = tr("%1 is in the first row of a table, which holds the names of its columns. Nothing was replaced.").arg(where);
            break;
        }
        CellChange ch;
        bool none = false;
        Outcome problem;
        if (!typedChange(spot.sheet, spot.row, spot.col, now, &ch, &none, &problem)) {
            plan.problem = tr("Replacing in %1 would leave \"%2\", which is not something a cell can hold: %3 Nothing was replaced.").arg(where, now.left(60), problem.problem);
            break;
        }
        plan.occurrences += n;
        if (none) continue;
        plan.step.cells.push_back(std::move(ch));
    }
    plan.done = true;
    return true;
}

EditSession::Outcome EditSession::applyReplace(ReplacePlan &plan, ReplaceResult *result)
{
    Outcome out;
    if (result) *result = ReplaceResult();
    auto refuse = [&](const QString &problem) {
        out.ok = false;
        out.problem = problem;
        return out;
    };
    QString why;
    if (!canEdit(&why)) return refuse(why);
    if (!plan.done) return refuse(tr("The replace could not be finished. Try it again."));
    if (!plan.problem.isEmpty()) return refuse(plan.problem);
    if (m_loader->changesMade() != plan.changesAtPlan || m_undo.index() != plan.undoIndexAtPlan) return refuse(tr("The cells were changed while the replace was being worked out, so it was not made. Replace again."));
    if (result) {
        result->cells = int(plan.step.cells.size());
        result->occurrences = plan.occurrences;
    }
    if (plan.step.cells.empty()) return out;
    pushStep(std::move(plan.step), tr("Replacing"));
    plan.step = Step();
    return out;
}

EditSession::Outcome EditSession::replaceInCells(const std::vector<CellSpot> &cells, const TextPattern &pattern, const QString &with, ReplaceResult *result)
{
    Outcome out;
    if (result) *result = ReplaceResult();
    QString why;
    const std::shared_ptr<ReplacePlan> plan = planReplace(cells, pattern, with, &why);
    if (!plan) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    prepareReplace(*plan, 0);
    return applyReplace(*plan, result);
}

} // namespace jp::crunch
