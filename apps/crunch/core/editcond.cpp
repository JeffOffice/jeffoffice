// The rules of conditional formatting a person makes or clears: Highlight Cells Rules and Top/Bottom Rules from the Home tab, and Clear Rules. A rule goes first (it has the highest
// priority); each change is one step of Undo; the file gets the sheet's rules written anew (core of the sheet's patch: sheetpatch.cpp) when the sheet is saved.
#include "core/editsession.h"

#include <QUndoCommand>

#include <functional>
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
// The most blocks of cells all the rules of a sheet may have after a clear splits them (what the save would write is bounded by it).
constexpr qint64 kMaxRangesAfterAClear = 200000;

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
        if (m_dxfAdded && !m_first) m_session->rememberDxf(m_format);   // (put back where it was: the rules name it by its place)
        m_first = false;
        m_session->setCondRules(m_sheet, m_after);
    }
    void undo() override
    {
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
                if (in != fileOf.end()) r.origPriority = r.priority = in->second;
                else r.origPriority = 0;   // (not in the file: brought back as the bytes kept of it, or made again)
            }
    }

private:
    EditSession *m_session;
    int m_sheet;
    QVector<CondRule> m_before, m_after;
    std::shared_ptr<qint64> m_total;
    DifferentialFormat m_format;
    qint32 m_dxf;
    bool m_dxfAdded, m_first = true;
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
        std::map<int, quint32> baseUid;
        for (const CondRule &r : condRules(sheet))
            if (r.origPriority > 0) {
                base.insert(r.origPriority);
                baseUid[r.origPriority] = r.uid;
            }
        m_condBase[sheet] = base;
        m_condBaseUid[sheet] = baseUid;
    }
    if (m_condSnap.count(sheet)) m_condChangedDuringSave.insert(sheet);
    if (std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold()) hold->calculation()->replaceConditionalFormats(sheet, rules);
    m_condTouched.insert(sheet);
    Q_EMIT conditionalFormatsChanged(sheet);
}

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

// Rules a step of structure (rows put in, cells deleted) brought back carry the priorities they had before the file was last saved. A rule the last save wrote is given its priority in the file;
// one that was made before that and is not in the file has none (it is brought back from the bytes kept of it, or made again).
void EditSession::condFixStale(int sheet)
{
    QVector<CondRule> rules = condRules(sheet);
    const auto file = m_condFileOf.find(sheet);
    bool any = false;
    for (CondRule &r : rules) {
        if (!r.uid) continue;
        if (file != m_condFileOf.end() && file->second.count(r.uid)) {
            const int p = file->second.at(r.uid);
            if (r.origPriority != p) {
                r.origPriority = p;
                any = true;
            }
        } else if (r.uid <= m_condUidMark && r.origPriority > 0) {
            r.origPriority = 0;
            any = true;
        }
    }
    if (any)
        if (std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold()) hold->calculation()->replaceConditionalFormats(sheet, rules);
}

namespace {
qint64 rawBytes(const jp::office::sheet::CondRaw &r) { return qint64(r.open.size() + r.rule.size() + r.x14Open.size() + r.x14Rule.size() + r.x14Sqref.size()); }
} // namespace

// What a save puts in the file for a sheet's rules, worked out from the rules as they are: the rules made here are numbered from 1 in the order they have, those the file has keep their priority plus
// that many, the file's rules that are not there any more are taken away (their bytes are kept, for Undo), and the rules brought back from those bytes go in with the priority they had. The rules as the
// save will have them are kept (`m_condSnap`) for when it is done.
void EditSession::condForSave(int sheet, jp::office::sheet::SheetChanges *sc, bool writtenWhole)
{
    CondSnap snap;
    if (writtenWhole) {
        // (a sheet made here is written from its rules as they are, with nothing of the file's to match)
        for (const CondRule &r : condRules(sheet)) snap.fileOf[r.uid] = r.priority;
        snap.after = condRules(sheet);
        m_condSnap[sheet] = snap;
        m_condChangedDuringSave.erase(sheet);
        return;
    }
    condFixStale(sheet);
    QVector<CondRule> rules = condRules(sheet);
    auto &stored = m_condRaw[sheet];
    QVector<CondRule> made;
    std::set<int> present;
    for (const CondRule &r : rules) {
        if (r.origPriority > 0) {
            present.insert(r.origPriority);
            if (sc) sc->condRanges[r.origPriority] = r.ranges;
        } else if (stored.count(r.uid)) {
            snap.restored.insert(r.uid);
            if (sc) sc->condRestore.push_back({r.ranges, stored.at(r.uid)});
        } else {
            made.append(r);
        }
    }
    std::stable_sort(made.begin(), made.end(), [](const CondRule &a, const CondRule &b) { return a.priority < b.priority; });
    for (int i = 0; i < made.size(); ++i) made[i].priority = i + 1;
    snap.shift = int(made.size());
    std::map<quint32, int> rank;
    for (const CondRule &m : made) rank[m.uid] = m.priority;
    for (CondRule &r : rules) {
        const int p = r.origPriority > 0 ? r.origPriority + snap.shift : snap.restored.count(r.uid) ? stored.at(r.uid).priority + snap.shift : rank[r.uid];
        snap.fileOf[r.uid] = p;
        r.origPriority = r.priority = p;
    }
    snap.after = rules;
    const auto base = m_condBase.find(sheet);
    if (base != m_condBase.end())
        for (const int p : base->second)
            if (!present.count(p)) snap.removed.insert(p);
    if (sc) {
        sc->condEdit = true;
        sc->condShift = snap.shift;
        sc->condNew = made;
        sc->condRemoved = snap.removed;
        snap.raw = std::make_shared<std::map<int, jp::office::sheet::CondRaw>>();
        sc->condRawOut = snap.raw;
    }
    m_condSnap[sheet] = snap;
    m_condChangedDuringSave.erase(sheet);
}

// The file was saved with the rules the save was made from (not the ones as they are now: a change made while it was written is not in the file): they are the file's own from here on, the rules
// it took away are kept as the bytes the file had, and the steps of Undo are put in step with it.
void EditSession::condSaved()
{
    std::map<int, std::map<quint32, int>> files;
    for (auto &entry : m_condSnap) {
        const int sheet = entry.first;
        CondSnap &snap = entry.second;
        QVector<CondRule> rules = condRules(sheet);
        for (CondRule &r : rules) {
            const auto in = snap.fileOf.find(r.uid);
            if (in != snap.fileOf.end()) r.origPriority = r.priority = in->second;
        }
        if (std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold()) hold->calculation()->replaceConditionalFormats(sheet, rules);
        // the bytes of what was taken away (by the uid the rule had), the others kept in step with the priorities
        auto &stored = m_condRaw[sheet];
        for (auto it = stored.begin(); it != stored.end();) {
            if (snap.restored.count(it->first)) {
                *m_undoBytes -= rawBytes(it->second);
                it = stored.erase(it);
            } else {
                it->second.priority += snap.shift;
                ++it;
            }
        }
        const auto uids = m_condBaseUid.find(sheet);
        if (snap.raw && uids != m_condBaseUid.end())
            for (const int p : snap.removed) {
                const auto raw = snap.raw->find(p);
                const auto who = uids->second.find(p);
                if (raw == snap.raw->end() || who == uids->second.end()) continue;
                jp::office::sheet::CondRaw kept = raw->second;
                kept.priority = p + snap.shift;
                *m_undoBytes += rawBytes(kept);
                stored[who->second] = kept;
            }
        // (the file's rules from here on, by priority, and who they are)
        std::set<int> base;
        std::map<int, quint32> baseUid;
        for (const CondRule &r : snap.after) {
            base.insert(r.origPriority);
            baseUid[r.origPriority] = r.uid;
        }
        m_condBase[sheet] = base;
        m_condBaseUid[sheet] = baseUid;
        m_condFileOf[sheet] = snap.fileOf;
        files[sheet] = snap.fileOf;
        if (!m_condChangedDuringSave.count(sheet)) m_condTouched.erase(sheet);
    }
    m_condSnap.clear();
    m_condChangedDuringSave.clear();
    m_condUidMark = m_condUid;
    // the steps, those inside a macro too
    std::function<void(const QUndoCommand *)> visit = [&](const QUndoCommand *c) {
        if (auto *step = const_cast<CondChange *>(dynamic_cast<const CondChange *>(c))) {
            const auto file = files.find(step->sheet());
            if (file != files.end()) step->rebase(file->second);
        }
        for (int i = 0; i < c->childCount(); ++i) visit(c->child(i));
    };
    for (int i = 0; i < m_undo.count(); ++i) visit(m_undo.command(i));
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
    qint64 cells = 0;
    for (const CondRule &r : after) cells += r.ranges.size();
    if (cells > kMaxRangesAfterAClear) {
        out.ok = false;
        out.problem = tr("Clearing there would split the rules into more than %1 blocks of cells. Clear a larger part of the sheet at once.").arg(kMaxRangesAfterAClear);
        return out;
    }
    m_undo.push(new CondChange(this, sheet, before, after, range.valid ? tr("Clear Rules from Selected Cells") : tr("Clear Rules from Sheet"), m_undoBytes));
    return out;
}

} // namespace jp::crunch
