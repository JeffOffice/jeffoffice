#pragma once
// The data entry form: one record of a list at a time. The list is the table the cell is in, else the block of cells round it (its first row names the fields, as a table's header row does).
// This is the model under the form's window: which record is shown, the fields with their names, which of them are read-only (a field that holds a formula), moving between records
// (all of them, or those that meet the criteria typed), and the changes: a record changed, added at the end, or deleted, each one step of Undo. No GUI.

#include "core/advancedfilter.h"
#include "core/editsession.h"
#include "core/gotospecial.h"

#include <QString>
#include <QStringList>
#include <vector>

namespace jp::crunch {

// The most fields a form shows (the first columns of a wider list), and the most records looked at in one search for a record that meets the criteria.
constexpr int kMaxFormFields = 256;
constexpr qint64 kMaxFormSearch = 2000000;
// The most records a form holds (the rows of a sheet).
constexpr qint64 kMaxFormRecords = 1048576;

class DataForm {
public:
    struct Field {
        QString name;
        quint32 col = 0;
    };

    // The form for the list round the cell (`row`, `col`); false, with `why`, when there is none: the cell is empty, or the list has a header row and no data under it and is not a table.
    // `regionWork` is how much work the search for the block of cells may do (a test makes it small).
    bool open(EditSession *session, int sheet, quint32 row, quint32 col, QString *why = nullptr, qint64 regionWork = kMaxSpecialWork);
    // The block of cells was too large to be found whole: the form has the first rows of it.
    bool truncated() const { return m_truncated; }
    bool isOpen() const { return m_session != nullptr; }
    int sheet() const { return m_sheet; }
    const std::vector<Field> &fields() const { return m_fields; }
    int fieldCount() const { return int(m_fields.size()); }
    bool isTable() const { return m_table >= 0; }
    // The list as it is now (with its header row), and the number of records: the rows of data that are shown (a row hidden by hand or by a filter is not one; a table's row of totals is not either).
    jp::office::sheet::CellRange list() const;
    qint64 recordCount() const;
    // The record shown (from 0); the row of the sheet it is in.
    qint64 current() const { return m_current; }
    // How many times a text of a field was read from the sheet (for a test of the window's work).
    qint64 reads() const { return m_reads; }
    quint32 rowOf(qint64 record) const;
    bool setCurrent(qint64 record);

    // The text of a field of a record to edit: what a cell is typed with (a formula with its "="). `shown` is what the cell shows (a formula's result).
    QString editText(qint64 record, int field) const;
    QString shownText(qint64 record, int field) const;
    QStringList values(qint64 record) const;   // for each field: the text of a field to edit, the shown text of a formula's
    // A field that is not changed here: the cell holds a formula (or is a calculated column of a table).
    bool isReadOnly(qint64 record, int field) const;
    // For a record that is not made yet (New): a field that a table's calculated column fills.
    bool isCalculatedColumn(int field) const;

    // ---- criteria: what is typed for each field (empty: no condition), as a value, or a value after =, <>, <, <=, >, or >=; text is matched at its start, with ? and * for characters.
    void setCriteria(const QStringList &typed);
    void clearCriteria();
    bool hasCriteria() const { return !m_criteria.isEmpty(); }
    const QStringList &criteria() const { return m_criteria; }
    bool matches(qint64 record) const;   // true for every record when there are no criteria
    // The next (or the previous) record, from the one given, that meets the criteria; -1 when there is none (also when the search stopped at kMaxFormSearch records).
    qint64 findNext(qint64 from) const;
    qint64 findPrevious(qint64 from) const;

    // ---- changes, each one step of Undo with the cells and the list as they are after it; not ok, and nothing changed, with `problem` as a sentence.
    // The fields' texts as they are now (one for each field) written to a record: only those that differ and are not read-only. No step when nothing differs.
    EditSession::Outcome change(qint64 record, const QStringList &typed);
    // A record added under the last one (a table grows by one row), from the texts; it is shown (made current). A record with every text empty is not added (ok, and `added` false).
    EditSession::Outcome add(const QStringList &typed, bool *added = nullptr);
    // The record taken out: the rows under it move up (a table's row of the table, a list's cells in its columns). The record after it is current, or the one before when it was the last.
    EditSession::Outcome remove(qint64 record);

private:
    void refresh();
    void rollBack(int before);
    mutable qint64 m_reads = 0;
    bool m_truncated = false;
    std::vector<quint32> m_rows;   // the sheet row of each record, in order
    qint64 m_dataFirst = 0, m_dataLast = -1;
    EditSession *m_session = nullptr;
    int m_sheet = 0;
    int m_table = -1;
    jp::office::sheet::CellRange m_list;   // as the list is (a table's, again after a change)
    std::vector<Field> m_fields;
    QStringList m_criteria;
    std::vector<Condition> m_conditions;   // by field (an empty `text` and no `numeric` are no condition: see m_active)
    std::vector<bool> m_active;
    qint64 m_current = 0;
};

} // namespace jp::crunch
