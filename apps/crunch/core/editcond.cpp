// The rules of conditional formatting a person makes or clears: Highlight Cells Rules and Top/Bottom Rules from the Home tab, and Clear Rules. A rule goes first (it has the highest
// priority); each change is one step of Undo; the file gets the sheet's rules written anew (core of the sheet's patch: sheetpatch.cpp) when the sheet is saved.
#include "core/editsession.h"

#include <QUndoCommand>

#include <algorithm>

namespace jp::crunch {

using jp::office::sheet::CellRange;
using jp::office::sheet::CondRule;
using jp::office::sheet::DifferentialFormat;

namespace {

// The most rules a sheet may hold before one more is refused (a file's own are kept whatever their number).
constexpr int kMaxRulesOnASheet = 5000;

class CondChange : public QUndoCommand {
public:
    CondChange(EditSession *session, int sheet, const QVector<CondRule> &before, const QVector<CondRule> &after, const QString &text)
        : QUndoCommand(text), m_session(session), m_sheet(sheet), m_before(before), m_after(after)
    {
    }
    void redo() override { m_session->setCondRules(m_sheet, m_after); }
    void undo() override { m_session->setCondRules(m_sheet, m_before); }

private:
    EditSession *m_session;
    int m_sheet;
    QVector<CondRule> m_before, m_after;
};

bool meets(const CellRange &a, const CellRange &b)
{
    return a.valid && b.valid && a.row1 <= b.row2 && a.row2 >= b.row1 && a.col1 <= b.col2 && a.col2 >= b.col1;
}

} // namespace

QVector<CondRule> EditSession::condRules(int sheet) const
{
    const jp::office::sheet::Calculation *calc = calculation();
    return calc && sheet >= 0 && sheet < calc->sheetCount() ? calc->sheet(sheet).conditionalFormats : QVector<CondRule>();
}

void EditSession::setCondRules(int sheet, const QVector<CondRule> &rules)
{
    if (std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold()) hold->calculation()->replaceConditionalFormats(sheet, rules);
    m_condTouched.insert(sheet);
    Q_EMIT conditionalFormatsChanged(sheet);
}

EditSession::Outcome EditSession::addCondRule(int sheet, CondRule rule, const DifferentialFormat &format)
{
    Outcome out;
    QString why;
    auto refuse = [&](const QString &text) {
        out.ok = false;
        out.problem = text;
        return out;
    };
    if (!canEdit(&why) || !canEditSheet(sheet, &why)) return refuse(why);
    if (m_book->book->isConverted()) return refuse(tr("Conditional formatting of a workbook opened from this kind of file is kept when it is saved as an .xlsx workbook. Save it as one, and then add rules."));
    if (rule.ranges.isEmpty()) return refuse(tr("Select the cells the rule is for."));
    QVector<CondRule> now = condRules(sheet);
    for (const CondRule &r : now)
        if (r.type == CondRule::Type::DataBar) return refuse(tr("This sheet has data bars in the form of a newer file. JeffCrunch keeps them as they are, and doesn't change the rules of the sheet."));
    if (now.size() >= kMaxRulesOnASheet) return refuse(tr("This sheet has as many rules as JeffCrunch changes (%1).").arg(kMaxRulesOnASheet));
    qint32 dxf = -1;
    {
        std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold();
        if (!hold) return refuse(tr("The workbook is busy. Try again in a moment."));
        dxf = m_book->book->dxfFor(format);
    }
    if (dxf < 0 && !format.isEmpty()) return refuse(tr("This workbook has no place to keep the look of a rule."));
    rule.dxf = dxf;
    // (it goes first: the others' priorities are one more)
    for (CondRule &r : now) r.priority += 1;
    rule.priority = 1;
    QVector<CondRule> after = now;
    after.prepend(rule);
    m_undo.push(new CondChange(this, sheet, condRules(sheet), after, tr("Conditional Formatting")));
    return out;
}

EditSession::Outcome EditSession::clearCondRules(int sheet, const CellRange &range)
{
    Outcome out;
    QString why;
    if (!canEdit(&why) || !canEditSheet(sheet, &why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    const QVector<CondRule> before = condRules(sheet);
    QVector<CondRule> after;
    for (const CondRule &r : before) {
        bool hit = !range.valid;
        for (const CellRange &c : r.ranges) hit = hit || meets(c, range);
        if (!hit) after.append(r);
    }
    if (after.size() == before.size()) return out;   // (nothing to clear)
    for (const CondRule &r : before)
        if (r.type == CondRule::Type::DataBar) {
            out.ok = false;
            out.problem = tr("This sheet has data bars in the form of a newer file. JeffCrunch keeps them as they are, and doesn't change the rules of the sheet.");
            return out;
        }
    m_undo.push(new CondChange(this, sheet, before, after, range.valid ? tr("Clear Rules from Selected Cells") : tr("Clear Rules from Sheet")));
    return out;
}

} // namespace jp::crunch
