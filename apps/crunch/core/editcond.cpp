// The rules of conditional formatting a person makes or clears: Highlight Cells Rules and Top/Bottom Rules from the Home tab, and Clear Rules. A rule goes first (it has the highest
// priority); each change is one step of Undo; the file gets the sheet's rules written anew (core of the sheet's patch: sheetpatch.cpp) when the sheet is saved.
#include "core/editsession.h"

#include <QCoreApplication>
#include <QUndoCommand>

#include <memory>
#include <set>

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
    CondChange(EditSession *session, int sheet, const QVector<CondRule> &before, const QVector<CondRule> &after, const QString &text, const std::shared_ptr<qint64> &total,
               const DifferentialFormat &format = DifferentialFormat(), qint32 dxf = -1, bool dxfAdded = false)
        : QUndoCommand(text), m_session(session), m_sheet(sheet), m_before(before), m_after(after), m_total(total), m_format(format), m_dxf(dxf), m_dxfAdded(dxfAdded)
    {
        // (the two lists are held for Undo and Redo: what they take is counted with the other steps', so that the steps kept stay within the limit)
        for (const QVector<CondRule> *list : {&m_before, &m_after})
            for (const CondRule &r : *list) m_bytes += qint64(sizeof(r)) + qint64(r.ranges.size()) * qint64(sizeof(CellRange)) + qint64(r.formula1.size() + r.formula2.size() + r.text.size()) * 2;
        *m_total += m_bytes;
    }
    ~CondChange() override { *m_total -= m_bytes; }
    void redo() override
    {
        if (m_dead) return m_session->condStepRefused(m_why);
        if (m_dxfAdded && !m_first) m_session->rememberDxf(m_format);   // (put back where it was: the rules name it by its place)
        m_first = false;
        m_session->setCondRules(m_sheet, m_after);
    }
    void undo() override
    {
        if (m_dead) return m_session->condStepRefused(m_why);
        m_session->setCondRules(m_sheet, m_before);
        if (m_dxfAdded) m_session->forgetDxf(m_dxf);   // (a look made for the rule goes with it)
    }
    int sheet() const { return m_sheet; }
    // The file was saved: its rules are the ones of `fileOf` (by uid, the priority each has in the file). The copies this step holds are put in step with that: a rule that is in the file is its own
    // element there (named by that priority), one that is not is made again when it comes back. A rule that has settings the file's elements hold (a 2010 entry) can't be made again from what is
    // kept, so a step that would bring one back is given up, and says so.
    void rebase(const std::map<quint32, int> &fileOf)
    {
        m_dxfAdded = false;   // (the look is in the file's styles now)
        for (QVector<CondRule> *list : {&m_before, &m_after})
            for (CondRule &r : *list) {
                const auto in = fileOf.find(r.uid);
                if (in != fileOf.end()) {
                    r.origPriority = r.priority = in->second;
                } else {
                    r.origPriority = 0;
                    if (!r.extId.trimmed().isEmpty() && !m_dead) {
                        m_dead = true;
                        m_why = QCoreApplication::translate("jp::crunch::EditSession", "This change to conditional formatting can't be undone or redone now: it was made before the file was saved, and it brings back a rule with extra settings that the saved file no longer has.");
                    }
                }
            }
    }

private:
    EditSession *m_session;
    int m_sheet;
    QVector<CondRule> m_before, m_after;
    std::shared_ptr<qint64> m_total;
    DifferentialFormat m_format;
    qint32 m_dxf;
    bool m_dxfAdded, m_first = true, m_dead = false;
    QString m_why;
    qint64 m_bytes = 0;
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
    if (!m_condBase.count(sheet)) {
        // (the rules the file has, as they are before the first change since it was read or saved)
        std::set<int> base;
        for (const CondRule &r : condRules(sheet))
            if (r.origPriority > 0) base.insert(r.origPriority);
        m_condBase[sheet] = base;
    }
    if (std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold()) hold->calculation()->replaceConditionalFormats(sheet, rules);
    m_condTouched.insert(sheet);
    Q_EMIT conditionalFormatsChanged(sheet);
}

void EditSession::condStepRefused(const QString &why) { Q_EMIT conditionalUndoRefused(why); }

// The sheet's rules get names for the steps of Undo to tell them by (kept by copies, so every copy has them).
void EditSession::ensureCondUids(int sheet)
{
    QVector<CondRule> rules = condRules(sheet);
    bool any = false;
    for (CondRule &r : rules)
        if (!r.uid) {
            r.uid = ++m_condUid;
            any = true;
        }
    if (any)
        if (std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold()) hold->calculation()->replaceConditionalFormats(sheet, rules);
}

// What the save puts in the file for a sheet's rules, worked out from the rules as they are: the rules made here are numbered from 1 in the order they have, those the file has keep their priority
// plus that many, and the file's rules that are not there any more are the ones taken away. (`fileOf` gets the priority every rule will have in the file, by uid.)
void EditSession::condForSave(int sheet, jp::office::sheet::SheetChanges *sc, std::map<quint32, int> *fileOf, QVector<CondRule> *after) const
{
    QVector<CondRule> rules = condRules(sheet);
    QVector<CondRule> made;
    std::set<int> present;
    for (const CondRule &r : rules) {
        if (r.origPriority > 0) {
            present.insert(r.origPriority);
            if (sc) sc->condRanges[r.origPriority] = r.ranges;
        } else {
            made.append(r);
        }
    }
    std::stable_sort(made.begin(), made.end(), [](const CondRule &a, const CondRule &b) { return a.priority < b.priority; });
    for (int i = 0; i < made.size(); ++i) made[i].priority = i + 1;
    const int shift = int(made.size());
    if (sc) {
        sc->condEdit = true;
        sc->condShift = shift;
        sc->condNew = made;
        const auto base = m_condBase.find(sheet);
        if (base != m_condBase.end())
            for (const int p : base->second)
                if (!present.count(p)) sc->condRemoved.insert(p);
    }
    if (fileOf || after) {
        // (a rule made here is told by its uid)
        std::map<quint32, int> rank;
        for (const CondRule &m : made) rank[m.uid] = m.priority;
        for (CondRule &r : rules) {
            const int p = r.origPriority > 0 ? r.origPriority + shift : rank[r.uid];
            if (fileOf) (*fileOf)[r.uid] = p;
            r.origPriority = r.priority = p;
        }
        if (after) *after = rules;
    }
}

// The file was saved with these rules: they are the file's own from here on, and the steps of Undo are put in step with it.
void EditSession::condSaved()
{
    std::map<int, std::map<quint32, int>> files;
    for (const int sheet : m_condTouched) {
        std::map<quint32, int> fileOf;
        QVector<CondRule> after;
        condForSave(sheet, nullptr, &fileOf, &after);
        if (std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold()) hold->calculation()->replaceConditionalFormats(sheet, after);
        files[sheet] = fileOf;
        m_condBase.erase(sheet);
    }
    for (int i = 0; i < m_undo.count(); ++i)
        if (auto *step = const_cast<CondChange *>(dynamic_cast<const CondChange *>(m_undo.command(i)))) {
            const auto file = files.find(step->sheet());
            if (file != files.end()) step->rebase(file->second);
        }
}

void EditSession::rememberDxf(const DifferentialFormat &format)
{
    if (std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold()) m_book->book->dxfFor(format);
}

void EditSession::forgetDxf(qint32 place)
{
    if (std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold()) m_book->book->removeAddedDxf(place);
}

// Whether the rules of the sheet may be changed: not in a workbook opened from another kind of file (saved as .xlsx, the rules come over as they were), not when the file has rules this program does
// does not hold whole (kinds it does not know, extension lists, a table's mark, the 2010 block, more than it reads), and not when the sheet has more rules than a step of Undo should hold.
QString EditSession::condRulesRefusal(int sheet)
{
    if (m_book->book->isConverted()) return tr("Conditional formatting of a workbook opened from this kind of file is kept when it is saved as an .xlsx workbook. Save it as one, and then change rules.");
    auto found = m_condWhole.find(sheet);
    if (found == m_condWhole.end()) found = m_condWhole.emplace(sheet, m_book->book->conditionalFormatsAreWhole(sheet)).first;
    if (!found->second) return tr("This sheet has rules of conditional formatting in a form JeffCrunch keeps as they are (more than it reads, or with a priority another rule has). It doesn't change the rules of the sheet.");
    const QVector<CondRule> now = condRules(sheet);
    if (now.size() > kMaxRulesOnASheet) return tr("This sheet has more rules than JeffCrunch changes (%1).").arg(kMaxRulesOnASheet);
    return QString();
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
    if (rule.ranges.isEmpty()) return refuse(tr("Select the cells the rule is for."));
    if (const QString no = condRulesRefusal(sheet); !no.isEmpty()) return refuse(no);
    ensureCondUids(sheet);
    QVector<CondRule> now = condRules(sheet);
    rule.uid = ++m_condUid;
    qint32 dxf = -1;
    bool added = false;
    {
        std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold();
        if (!hold) return refuse(tr("The workbook is busy. Try again in a moment."));
        const qsizetype before = m_book->book->addedDxfs().size();
        dxf = m_book->book->dxfFor(format);
        added = m_book->book->addedDxfs().size() > before;
    }
    if (dxf < 0 && !format.isEmpty()) return refuse(tr("This workbook has no place to keep the look of a rule."));
    rule.dxf = dxf;
    // (it goes first: the others' priorities are one more)
    QVector<CondRule> after = now;
    for (CondRule &r : after) r.priority += 1;
    rule.priority = 1;
    after.prepend(rule);
    m_undo.push(new CondChange(this, sheet, now, after, tr("Conditional Formatting"), m_undoBytes, format, dxf, added));
    return out;
}

namespace {

// `range` less the cells of `cut`: up to four blocks (above, below, left, and right of what is cut).
void subtract(const CellRange &range, const CellRange &cut, QVector<CellRange> *out)
{
    if (!meets(range, cut)) {
        out->append(range);
        return;
    }
    auto block = [&](quint32 r1, quint32 c1, quint32 r2, quint32 c2) {
        CellRange b = range;
        b.row1 = r1;
        b.col1 = c1;
        b.row2 = r2;
        b.col2 = c2;
        out->append(b);
    };
    if (cut.row1 > range.row1) block(range.row1, range.col1, cut.row1 - 1, range.col2);
    if (cut.row2 < range.row2) block(cut.row2 + 1, range.col1, range.row2, range.col2);
    const quint32 top = std::max(range.row1, cut.row1), bottom = std::min(range.row2, cut.row2);
    if (cut.col1 > range.col1) block(top, range.col1, bottom, cut.col1 - 1);
    if (cut.col2 < range.col2) block(top, cut.col2 + 1, bottom, range.col2);
}

} // namespace

EditSession::Outcome EditSession::clearCondRules(int sheet, const CellRange &range)
{
    Outcome out;
    QString why;
    if (!canEdit(&why) || !canEditSheet(sheet, &why)) {
        out.ok = false;
        out.problem = why;
        return out;
    }
    if (const QString no = condRulesRefusal(sheet); !no.isEmpty()) {
        out.ok = false;
        out.problem = no;
        return out;
    }
    ensureCondUids(sheet);
    const QVector<CondRule> before = condRules(sheet);
    QVector<CondRule> after;
    bool changed = false;
    for (const CondRule &r : before) {
        if (!range.valid) {
            changed = true;
            continue;
        }
        QVector<CellRange> left;
        for (const CellRange &c : r.ranges) subtract(c, range, &left);   // (the cells outside the selection keep the rule)
        if (left == r.ranges) {
            after.append(r);
            continue;
        }
        changed = true;
        if (left.isEmpty()) continue;
        CondRule kept = r;
        kept.ranges = left;
        after.append(kept);
    }
    if (!changed) return out;   // (nothing to clear)
    m_undo.push(new CondChange(this, sheet, before, after, range.valid ? tr("Clear Rules from Selected Cells") : tr("Clear Rules from Sheet"), m_undoBytes));
    return out;
}

} // namespace jp::crunch
