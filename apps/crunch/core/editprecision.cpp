// Precision as displayed: the numbers of a workbook are kept as its cells show them. Turning it on rounds every number constant to what its cell's number format shows, as one step of Undo with
// the setting; after that a number put in a cell (EditSession::apply) and a number a formula gives (Calculation::storeResult) are rounded the same way, and the formulas are left as they are.
#include "core/editsession.h"

#include "calc/numfmt.h"

#include <QCoreApplication>
#include <QLocale>

#include <algorithm>
#include <limits>

namespace jp::crunch {

using namespace jp::office::sheet;

double ShownRounder::round(quint32 style, double value)
{
    auto at = m_formats.find(style);
    if (at == m_formats.end()) at = m_formats.emplace(style, jp::calc::NumberFormat(m_styles.formatCode(style, QLocale::c()))).first;
    double shown = value;
    if (!at->second.shownValue(value, m_system, &shown)) return value;
    return shown;
}

struct EditSession::PrecisionPlan {
    Step step;
    int sheet = 0;                       // the sheet being looked at
    CellStore::RowTailCursor cursor;
    qint64 looked = 0, total = 0;        // the cells looked at, and all there are
    bool done = false;
    QString problem;                     // a number cannot be changed: why (nothing is changed then)
    qint64 wouldChange = 0;              // the numbers that would change (counted past the most a step holds, to say how many)
    qint64 holds = 0;                    // the most numbers one step holds
    qint64 changesAtPlan = 0;
    int undoIndexAtPlan = 0;
    std::unique_ptr<ShownRounder> rounder;
    std::shared_ptr<SlicedPush> push;    // the step being put in a slice at a time
    size_t changed = 0;
};

qint64 EditSession::precisionLooked(const PrecisionPlan &plan) { return plan.looked; }
qint64 EditSession::precisionTotal(const PrecisionPlan &plan) { return plan.total; }
qint64 EditSession::precisionChanges(const PrecisionPlan &plan) { return qint64(plan.push ? plan.changed : plan.step.numbers.size()); }

// What Undo can keep of one step: half of what it holds (a step after it must not push it out), and never more than kMaxPrecisionNumbers.
qint64 EditSession::precisionHolds() const
{
    const qint64 room = std::max<qint64>(0, m_undoByteLimit / 2 - qint64(sizeof(CalcEdit)));
    return std::min<qint64>(kMaxPrecisionNumbers, room / qint64(sizeof(NumberCell)));
}

std::shared_ptr<EditSession::PrecisionPlan> EditSession::planPrecisionAsShown(QString *why)
{
    auto refuse = [&](const QString &problem) {
        if (why) *why = problem;
        return std::shared_ptr<PrecisionPlan>();
    };
    QString reason;
    if (!canEdit(&reason)) return refuse(reason);
    if (m_calcNow.precisionAsShown) return refuse(tr("The values are kept as the cells show them already."));
    auto plan = std::make_shared<PrecisionPlan>();
    const Calculation *calc = m_book->calc.get();
    for (int i = 0; i < calc->sheetCount(); ++i) plan->total += calc->sheet(i).cells.cellCount();
    plan->holds = precisionHolds();
    plan->rounder = std::make_unique<ShownRounder>(m_book->book->styles(), m_book->book->info().dateSystem);
    plan->changesAtPlan = m_loader->changesMade();
    plan->undoIndexAtPlan = m_undo.index();
    return plan;
}

bool EditSession::preparePrecision(PrecisionPlan &plan, qint64 budget)
{
    if (plan.done) return true;
    const Calculation *calc = m_book->calc.get();
    qint64 steps = 0;
    const qint64 limit = budget > 0 ? budget : std::numeric_limits<qint64>::max();
    while (plan.sheet < calc->sheetCount()) {
        if (budget > 0 && steps >= limit) return false;
        const int sheet = plan.sheet;
        if (m_book->book->info().sheets[sheet].deleted || !canEditSheet(sheet)) {
            ++plan.sheet;
            plan.cursor = CellStore::RowTailCursor();
            continue;
        }
        const bool finished = calc->sheet(sheet).cells.forEachRowTailSome(&plan.cursor, limit, &steps, [&](quint32 row, quint32 col, const Cell &c) {
            ++plan.looked;
            if (c.flags != 0 || (c.type != CellType::Number && c.type != CellType::Date) || !plan.problem.isEmpty()) return false;
            const double shown = plan.rounder->round(c.style, c.number);
            if (shown == c.number) return false;
            if (isLocked(sheet, c.style, row, col)) {
                plan.problem = tr("%1 is protected, and some of its numbers are locked, so they can't be rounded. Unprotect it, and try again.").arg(m_book->book->info().sheets[sheet].name);
                return true;
            }
            ++plan.wouldChange;
            if (qint64(plan.step.numbers.size()) >= plan.holds) return false;   // (counted, not kept: the step that would hold them is refused when the count is known)
            NumberCell n;
            n.sheet = sheet;
            n.row = row;
            n.col = col;
            n.before = c.number;
            n.after = shown;
            if (plan.step.numbers.size() == plan.step.numbers.capacity()) plan.step.numbers.reserve(std::min<size_t>(size_t(plan.holds), std::max<size_t>(1024, plan.step.numbers.size() + plan.step.numbers.size() / 4)));   // (steady growth, not doubling: the step is large)
            plan.step.numbers.push_back(n);
            return false;
        });
        if (!plan.problem.isEmpty()) break;
        if (!finished) return false;
        ++plan.sheet;
        plan.cursor = CellStore::RowTailCursor();
    }
    if (plan.problem.isEmpty() && plan.wouldChange > plan.holds)
        plan.problem = tr("That would round %1 numbers, and JeffCrunch rounds at most %2 at once, so that Undo can bring them back.").arg(QLocale().toString(plan.wouldChange), QLocale().toString(plan.holds));
    plan.done = true;
    return true;
}

EditSession::Outcome EditSession::applyPrecision(PrecisionPlan &plan, qint64 budget)
{
    Outcome out;
    auto refuse = [&](const QString &problem) {
        out.ok = false;
        out.problem = problem;
        return out;
    };
    const QString text = tr("Set Precision as Displayed");
    if (!plan.push) {
        QString why;
        if (!canEdit(&why)) return refuse(why);
        if (!plan.done) return refuse(tr("The rounding could not be finished. Try it again."));
        if (!plan.problem.isEmpty()) return refuse(plan.problem);
        if (m_loader->changesMade() != plan.changesAtPlan || m_undo.index() != plan.undoIndexAtPlan) return refuse(tr("The cells were changed while the rounding was being worked out, so it was not made. Try it again."));
        CalcEdit e;
        e.before = m_calcNow;
        e.after = m_calcNow;
        e.after.precisionAsShown = true;
        plan.step.calcs.push_back(e);
        plan.changed = plan.step.numbers.size();
        if (budget <= 0) {
            pushStep(std::move(plan.step), text);
            plan.step = Step();
            return out;
        }
        plan.push = beginSlicedPush(std::move(plan.step), text);
        plan.step = Step();
    }
    if (!continueSlicedPush(*plan.push, budget)) out.pending = true;
    return out;
}

EditSession::Outcome EditSession::setPrecisionAsShown(bool on)
{
    Outcome out;
    if (on) {
        QString why;
        const std::shared_ptr<PrecisionPlan> plan = planPrecisionAsShown(&why);
        if (!plan) {
            out.ok = false;
            out.problem = why;
            return out;
        }
        preparePrecision(*plan, 0);
        return applyPrecision(*plan);
    }
    QString why;
    if (!canEdit(&why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    if (!m_calcNow.precisionAsShown) return out;   // (nothing to change)
    CalcEdit e;
    e.before = m_calcNow;
    e.after = m_calcNow;
    e.after.precisionAsShown = false;
    Step step;
    step.calcs.push_back(e);
    pushStep(std::move(step), tr("Set Precision as Displayed"));
    return out;
}

} // namespace jp::crunch
