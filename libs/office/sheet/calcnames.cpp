// The defined names of a calculation: read, given a text, taken away, and given another name (every formula that uses the name follows). See Calculation::renameName.
#include "sheet/calculation.h"
#include "sheet/namerefs.h"

#include <algorithm>

namespace jp::office::sheet {

namespace {

QString keyOf(int scope, const QString &name) { return QString::number(scope) + QLatin1Char('|') + name.toUpper(); }

} // namespace

bool Calculation::nameText(int scope, const QString &name, QString *text) const
{
    const auto at = m_names.constFind(keyOf(scope, name));
    if (at == m_names.constEnd()) return false;
    if (text) *text = namePlaces(at.key(), at.value());
    return true;
}

void Calculation::setNameText(int scope, const QString &name, const QString &text)
{
    abandonNow();
    const QString key = keyOf(scope, name);
    if (text.isEmpty()) {
        m_names.remove(key);
        m_idNames.remove(key);
    } else {
        bool inIds = false;
        m_names[key] = heldNameText(scope, text, &inIds);   // (the text a person writes is in places; the workbook holds what it can in ids)
        if (inIds) m_idNames.insert(key);
        else m_idNames.remove(key);
    }
    nameTextsChangedFor({name});
}

void Calculation::renameName(int scope, const QString &oldName, const QString &newName, SheetRewrite *record)
{
    if (oldName.toUpper() == newName.toUpper()) return;   // (only the capitals differ: formulas read it the same)
    abandonNow();
    BulkEdit bulk(this);
    const QString oldUpper = oldName.toUpper();
    // a use of the name is of this one if it is written with its sheet, or when no sheet is written, if the formula is where the name is read: the whole workbook's, except on a sheet that has
    // a name of the same text of its own
    auto appliesOn = [&](int formulaSheet) {
        return [this, scope, formulaSheet, &oldUpper](const QString &written) {
            if (written.isEmpty()) return scope >= 0 ? formulaSheet == scope : formulaSheet < 0 || !m_names.contains(keyOf(formulaSheet, oldUpper));
            if (scope < 0) return false;
            const auto at = m_byName.constFind(written.toCaseFolded());
            return at != m_byName.constEnd() && at.value() == scope;
        };
    };
    for (int si = 0; si < int(m_sheets.size()); ++si) {
        Sheet &s = m_sheets[size_t(si)];
        SheetState &st = m_state[size_t(si)];
        const auto applies = appliesOn(si);
        std::vector<char> changedSlot(size_t(s.formulaTexts.size()), 0);
        bool any = false;
        for (qint32 slot = 0; slot < s.formulaTexts.size(); ++slot) {
            const QString text = s.formulaTexts[slot];
            if (text.isEmpty() || !text.contains(oldName, Qt::CaseInsensitive)) continue;
            bool changed = false;
            // (a text with ids past the grid in it has words that are cells there and no names: a name that could be read as a cell is never held in ids)
            const QString now = renameNameInFormula(text, oldName, newName, applies, &changed, s.wideTexts.contains(slot));
            if (!changed) continue;
            record->texts.push_back({si, slot, text});
            setSlotText(si, slot, now);
            changedSlot[size_t(slot)] = 1;
            any = true;
        }
        if (any) {
            for (qint32 fi = 0; fi < s.formulas.size(); ++fi) {
                const Formula &f = s.formulas[fi];
                if (f.text < 0 || size_t(f.text) >= changedSlot.size() || !changedSlot[size_t(f.text)]) continue;
                if (size_t(fi) >= st.nodeOf.size() || st.nodeOf[size_t(fi)] == kNone) continue;
                const quint32 id = st.nodeOf[size_t(fi)];
                const Cell *c = s.cells.findById(m_nodes[id].rowId, m_nodes[id].colId);
                record->rewritten.push_back({si, rowOf(m_nodes[id]), colOf(m_nodes[id]), m_nodes[id].rowId, m_nodes[id].colId, c && (c->flags & DataTable)});
            }
        }
        // the rules of validation and of conditional formatting
        bool rulesChanged = false;
        QVector<Validation> validations = s.validations;
        for (Validation &v : validations) {
            bool c1 = false, c2 = false;
            v.formula1 = renameNameInFormula(v.formula1, oldName, newName, applies, &c1);
            v.formula2 = renameNameInFormula(v.formula2, oldName, newName, applies, &c2);
            rulesChanged = rulesChanged || c1 || c2;
        }
        if (rulesChanged) {
            record->validations.push_back({si, s.validations});
            s.validations = validations;
            forgetRuleIndex();
        }
        bool cfChanged = false;
        QVector<CondRule> rules = s.conditionalFormats;
        for (CondRule &r : rules) {
            bool c1 = false, c2 = false;
            r.formula1 = renameNameInFormula(r.formula1, oldName, newName, applies, &c1);
            r.formula2 = renameNameInFormula(r.formula2, oldName, newName, applies, &c2);
            cfChanged = cfChanged || c1 || c2;
            for (CondValue &v : r.values)
                if (v.type == CondValue::Type::Formula) {
                    bool cv = false;
                    v.value = renameNameInFormula(v.value, oldName, newName, applies, &cv);
                    cfChanged = cfChanged || cv;
                }
        }
        if (cfChanged) {
            record->conditionalFormats.push_back({si, s.conditionalFormats});
            s.conditionalFormats = rules;
        }
    }
    // the other names' texts (a name belongs to the sheet its key says, or to the workbook)
    for (auto it = m_names.begin(); it != m_names.end(); ++it) {
        const int bar = int(it.key().indexOf(QLatin1Char('|')));
        const int own = bar > 0 ? it.key().left(bar).toInt() : -1;
        bool changed = false;
        const QString now = renameNameInFormula(it.value(), oldName, newName, appliesOn(own), &changed, m_idNames.contains(it.key()));
        if (!changed) continue;
        record->names.push_back({it.key(), taggedName(it.key())});
        it.value() = now;
    }
    // the name itself
    const QString oldKey = keyOf(scope, oldName), newKey = keyOf(scope, newName);
    const auto at = m_names.find(oldKey);
    if (at != m_names.end()) {
        const QString text = at.value();
        m_names.erase(at);
        m_names[newKey] = text;
        if (m_idNames.remove(oldKey)) m_idNames.insert(newKey);
    }
    nameTextsChangedFor({oldName, newName});
}

void Calculation::undoRenameName(int scope, const QString &oldName, const QString &newName, const SheetRewrite &record)
{
    if (oldName.toUpper() == newName.toUpper()) return;
    abandonNow();
    BulkEdit bulk(this);
    const QString oldKey = keyOf(scope, oldName), newKey = keyOf(scope, newName);
    const auto at = m_names.find(newKey);
    if (at != m_names.end()) {
        const QString text = at.value();
        m_names.erase(at);
        m_names[oldKey] = text;
        if (m_idNames.remove(newKey)) m_idNames.insert(oldKey);
    }
    undoSheetRewrite(record);
    nameTextsChangedFor({oldName, newName});
}

void Calculation::nameTextsChangedFor(const QStringList &given)
{
    // the names that are to be looked for: these, and every name whose text mentions one of them, and so on
    QStringList names = given;
    bool grew = true;
    for (int pass = 0; grew && pass < 16; ++pass) {
        grew = false;
        for (auto it = m_names.constBegin(); it != m_names.constEnd(); ++it) {
            const int bar = int(it.key().indexOf(QLatin1Char('|')));
            const QString own = it.key().mid(bar + 1);
            bool known = false;
            for (const QString &n : names) known = known || n.compare(own, Qt::CaseInsensitive) == 0;
            if (known) continue;
            for (const QString &n : names)
                if (it.value().contains(n, Qt::CaseInsensitive) && formulaUsesName(it.value(), n)) {
                    names << own;
                    grew = true;
                    break;
                }
        }
    }
    m_nameTraits.clear();
    qint64 rebuilt = 0;
    for (int si = 0; si < int(m_sheets.size()); ++si) {
        Sheet &s = m_sheets[size_t(si)];
        SheetState &st = m_state[size_t(si)];
        std::vector<char> mention(size_t(s.formulaTexts.size()), 0);
        bool any = false;
        for (qint32 slot = 0; slot < s.formulaTexts.size(); ++slot) {
            const QString &text = s.formulaTexts[slot];
            if (text.isEmpty()) continue;
            for (const QString &n : names)
                if (text.contains(n, Qt::CaseInsensitive)) {
                    mention[size_t(slot)] = 1;
                    any = true;
                    if (size_t(slot) < st.plain.size()) st.plain[size_t(slot)].tried = false;
                    if (size_t(slot) < st.array.size()) st.array[size_t(slot)].tried = false;
                    break;
                }
        }
        if (!any) continue;
        for (qint32 fi = 0; fi < s.formulas.size(); ++fi) {
            const Formula &f = s.formulas[fi];
            if (f.text < 0 || size_t(f.text) >= mention.size() || !mention[size_t(f.text)]) continue;
            if (size_t(fi) >= st.nodeOf.size() || st.nodeOf[size_t(fi)] == kNone) continue;
            const quint32 id = st.nodeOf[size_t(fi)];
            Cell *c = s.cells.findMutableById(m_nodes[id].rowId, m_nodes[id].colId);
            if (!c || !c->hasFormula()) continue;
            const qint32 formula = c->formula;
            const quint32 rowId = m_nodes[id].rowId, colId = m_nodes[id].colId;
            dropNode(id);
            const quint32 now = addNodeById(si, rowId, colId, *c, formula);
            queueDependents(areaOf(m_nodes[now]));
            ++m_nameRebuilds;
            ++m_editStats.namesRegisteredAgain;
            ++rebuilt;
        }
    }
    // (a change of a name that nothing mentions leaves nothing to work out, and no recalculation is begun for it)
    if (rebuilt > 0 && !m_running) beginRecalculation();
}

} // namespace jp::office::sheet
