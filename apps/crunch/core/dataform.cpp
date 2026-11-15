#include "core/dataform.h"

#include "core/gotospecial.h"
#include "calc/formula.h"
#include "calc/value.h"
#include "sheet/cellref.h"
#include "sheet/tables.h"

#include <QCoreApplication>
#include <QLocale>

namespace jp::crunch {

using namespace jp::office::sheet;
using jp::calc::Value;

namespace {

QString tr(const char *text) { return QCoreApplication::translate("jp::crunch::DataForm", text); }

EditSession::Outcome refuse(const QString &why)
{
    EditSession::Outcome o;
    o.ok = false;
    o.problem = why;
    return o;
}

// What a condition typed in the form is as a value: a number when it reads as one (so that "5" means equal to 5), else the text.
Value typedValue(const QString &typed)
{
    bool ok = false;
    double v = QLocale().toDouble(typed.trimmed(), &ok);
    if (!ok) v = QLocale::c().toDouble(typed.trimmed(), &ok);
    return ok ? Value::number(v) : Value::text(typed);
}

} // namespace

bool DataForm::open(EditSession *session, int sheet, quint32 row, quint32 col, QString *why)
{
    *this = DataForm();
    auto fail = [&](const QString &text) {
        if (why) *why = text;
        return false;
    };
    if (!session || !session->workbook()) return fail(tr("There is no list here."));
    m_session = session;
    m_sheet = sheet;
    m_table = session->tableAt(sheet, row, col);
    if (m_table >= 0) {
        const Table &t = session->workbook()->tables()[m_table];
        for (int i = 0; i < t.columns.size() && i < kMaxFormFields; ++i) m_fields.push_back({t.columns[i], t.col1 + quint32(i)});
    } else {
        bool truncated = false;
        const CellRange region = currentRegion(*session, sheet, row, col, &truncated);
        if (!region.valid || region.cellCount() < 2 || region.row2 <= region.row1) {
            m_session = nullptr;
            return fail(tr("The form needs a list: a row of names for the fields, and at least one row of data under it."));
        }
        if (session->displayText(sheet, region.row1, region.col1).isEmpty() && region.col1 == region.col2) {
            m_session = nullptr;
            return fail(tr("The form needs a list: a row of names for the fields, and at least one row of data under it."));
        }
        m_list = region;
        for (quint32 c = region.col1; c <= region.col2 && int(m_fields.size()) < kMaxFormFields; ++c) {
            QString name = session->displayText(sheet, region.row1, c).trimmed();
            if (name.isEmpty()) name = tr("Column %1").arg(jp::calc::columnLetters(c));
            m_fields.push_back({name, c});
        }
    }
    refresh();
    if (recordCount() < 1) {
        m_session = nullptr;
        m_fields.clear();
        return fail(tr("The list has no data under its names yet. Type a row of data under the names first."));
    }
    m_current = 0;
    // the record with the cell is shown first
    if (row >= rowOf(0) && qint64(row) <= qint64(rowOf(0)) + recordCount() - 1) m_current = qint64(row) - qint64(rowOf(0));
    return true;
}

void DataForm::refresh()
{
    if (m_table >= 0 && m_session && m_session->workbook() && m_table < m_session->workbook()->tables().size()) {
        const Table &t = m_session->workbook()->tables()[m_table];
        m_list.row1 = t.row1;
        m_list.row2 = quint32(std::max<qint64>(t.row1, t.dataLast()));
        m_list.col1 = t.col1;
        m_list.col2 = t.col2;
        m_list.valid = true;
        if (t.headerRows == 0) m_list.row1 = t.dataFirst() > 0 ? t.dataFirst() - 1 : 0;   // (the row above the data stands for the names)
    }
}

CellRange DataForm::list() const { return m_list; }

qint64 DataForm::recordCount() const
{
    if (!m_session || !m_list.valid) return 0;
    if (m_table >= 0) {
        const Table &t = m_session->workbook()->tables()[m_table];
        return t.hasData() ? qint64(t.dataLast()) - qint64(t.dataFirst()) + 1 : 0;
    }
    return qint64(m_list.row2) - qint64(m_list.row1);
}

quint32 DataForm::rowOf(qint64 record) const
{
    if (m_table >= 0) return m_session->workbook()->tables()[m_table].dataFirst() + quint32(record);
    return m_list.row1 + 1 + quint32(record);
}

bool DataForm::setCurrent(qint64 record)
{
    if (record < 0 || record >= recordCount()) return false;
    m_current = record;
    return true;
}

QString DataForm::editText(qint64 record, int field) const
{
    if (field < 0 || field >= fieldCount()) return QString();
    return m_session->editText(m_sheet, rowOf(record), m_fields[size_t(field)].col);
}

QString DataForm::shownText(qint64 record, int field) const
{
    if (field < 0 || field >= fieldCount()) return QString();
    return m_session->displayText(m_sheet, rowOf(record), m_fields[size_t(field)].col);
}

bool DataForm::isReadOnly(qint64 record, int field) const
{
    if (field < 0 || field >= fieldCount()) return false;
    return editText(record, field).startsWith(QLatin1Char('='));
}

bool DataForm::isCalculatedColumn(int field) const
{
    if (m_table < 0 || field < 0 || field >= fieldCount()) return false;
    const Table &t = m_session->workbook()->tables()[m_table];
    return field < t.info.size() && !t.info[field].calcFormula.isEmpty();
}

QStringList DataForm::values(qint64 record) const
{
    QStringList out;
    for (int f = 0; f < fieldCount(); ++f) out << (isReadOnly(record, f) ? shownText(record, f) : editText(record, f));
    return out;
}

void DataForm::setCriteria(const QStringList &typed)
{
    m_criteria.clear();
    m_conditions.clear();
    m_active.clear();
    bool any = false;
    for (int f = 0; f < fieldCount(); ++f) {
        const QString t = f < typed.size() ? typed[f].trimmed() : QString();
        m_criteria << t;
        m_conditions.push_back(t.isEmpty() ? Condition() : parseCondition(t, typedValue(t)));
        m_active.push_back(!t.isEmpty());
        any = any || !t.isEmpty();
    }
    if (!any) clearCriteria();
}

void DataForm::clearCriteria()
{
    m_criteria.clear();
    m_conditions.clear();
    m_active.clear();
}

bool DataForm::matches(qint64 record) const
{
    if (!hasCriteria()) return true;
    for (int f = 0; f < fieldCount(); ++f)
        if (m_active[size_t(f)] && !conditionMet(m_conditions[size_t(f)], m_session->valueAt(m_sheet, rowOf(record), m_fields[size_t(f)].col))) return false;
    return true;
}

qint64 DataForm::findNext(qint64 from) const
{
    const qint64 n = recordCount();
    qint64 looked = 0;
    for (qint64 r = from + 1; r < n && looked < kMaxFormSearch; ++r, ++looked)
        if (matches(r)) return r;
    return -1;
}

qint64 DataForm::findPrevious(qint64 from) const
{
    qint64 looked = 0;
    for (qint64 r = std::min(from, recordCount()) - 1; r >= 0 && looked < kMaxFormSearch; --r, ++looked)
        if (matches(r)) return r;
    return -1;
}

EditSession::Outcome DataForm::change(qint64 record, const QStringList &typed)
{
    if (!m_session || record < 0 || record >= recordCount()) return refuse(tr("There is no such record."));
    QVector<int> changed;
    for (int f = 0; f < fieldCount() && f < typed.size(); ++f)
        if (!isReadOnly(record, f) && typed[f] != editText(record, f)) changed << f;
    EditSession::Outcome ok;
    if (changed.isEmpty()) return ok;
    QUndoStack *stack = m_session->undoStack();
    const int before = stack->index();
    stack->beginMacro(tr("Edit Record"));
    for (int f : changed) {
        const EditSession::Outcome o = m_session->type(m_sheet, rowOf(record), m_fields[size_t(f)].col, typed[f]);
        if (!o.ok) {
            stack->endMacro();
            if (stack->index() > before) stack->undo();
            return o;
        }
    }
    stack->endMacro();
    refresh();
    return ok;
}

EditSession::Outcome DataForm::add(const QStringList &typed, bool *added)
{
    if (added) *added = false;
    EditSession::Outcome ok;
    if (!m_session) return refuse(tr("There is no list."));
    bool any = false;
    for (int f = 0; f < fieldCount() && f < typed.size(); ++f) any = any || !typed[f].isEmpty();
    if (!any) return ok;
    const qint64 last = m_table >= 0 ? qint64(m_session->workbook()->tables()[m_table].dataLast()) : qint64(m_list.row2);
    // (a table that has a row of totals gets the new row above it, as the table's own typing does not: the form says it can't)
    if (m_table >= 0 && m_session->workbook()->tables()[m_table].totalsRows > 0) return refuse(tr("The table has a row of totals under its data. Turn the totals off to add a record here, or type the new row in the sheet."));
    const qint64 row = last + 1;
    if (row >= qint64(1048576)) return refuse(tr("The list is at the last row of the sheet."));
    for (int f = 0; f < fieldCount(); ++f)
        if (!m_session->editText(m_sheet, quint32(row), m_fields[size_t(f)].col).isEmpty()) return refuse(tr("The list can't be extended: there is something under it. Move it away, or insert a row."));
    QUndoStack *stack = m_session->undoStack();
    const int before = stack->index();
    stack->beginMacro(tr("New Record"));
    for (int f = 0; f < fieldCount() && f < typed.size(); ++f) {
        if (typed[f].isEmpty()) continue;
        if (typed[f].startsWith(QLatin1Char('=')) && isCalculatedColumn(f)) continue;
        const EditSession::Outcome o = m_session->type(m_sheet, quint32(row), m_fields[size_t(f)].col, typed[f]);
        if (!o.ok) {
            stack->endMacro();
            if (stack->index() > before) stack->undo();
            return o;
        }
    }
    stack->endMacro();
    if (m_table < 0) m_list.row2 = quint32(row);
    refresh();
    if (added) *added = true;
    m_current = recordCount() - 1;
    return ok;
}

EditSession::Outcome DataForm::remove(qint64 record)
{
    if (!m_session || record < 0 || record >= recordCount()) return refuse(tr("There is no such record."));
    if (recordCount() < 2 && m_table >= 0) return refuse(tr("A table keeps at least one row of data."));
    CellRange block;
    block.row1 = block.row2 = rowOf(record);
    block.col1 = m_list.col1;
    block.col2 = m_list.col2;
    block.valid = true;
    const EditSession::Outcome o = m_session->deleteCells(m_sheet, block, true);
    if (!o.ok) return o;
    if (m_table < 0) {
        if (m_list.row2 > m_list.row1) --m_list.row2;
    }
    refresh();
    m_current = std::min(record, recordCount() - 1);
    if (recordCount() < 1) m_current = 0;
    return o;
}

} // namespace jp::crunch
