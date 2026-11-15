#pragma once
// Advanced Filter: the rows of a list that meet conditions written on the sheet. The conditions are a range whose first row repeats column headings of the list and whose other rows are the
// conditions: the cells of a row must all hold (And), and a row of them is enough (Or). A condition is a value (text is matched at the start, `?` and `*` stand for characters), or a value
// after `=`, `<>`, `<`, `<=`, `>`, or `>=`; `=` alone means an empty cell. A column heading not found in the list is left out; a row with no conditions in it lets every row through.

#include "core/editsession.h"

#include <QLocale>

#include <vector>

namespace jp::crunch {

struct AdvancedFilterSpec {
    jp::office::sheet::CellRange list;       // with its heading row
    jp::office::sheet::CellRange criteria;   // with its heading row
    bool unique = false;                     // a row equal to an earlier row that met the conditions is left out
};

struct AdvancedFilterResult {
    bool ok = true;
    QString problem;
    std::vector<quint32> rows;               // the rows of the list (not its heading row) that meet the conditions, in order
};

// One condition as it is typed (a value, or a value after =, <>, <, <=, >, or >=), and whether a cell's value meets it: the Advanced Filter's own, shared with the data form's criteria.
struct Condition {
    enum class Op { Begins, Equal, NotEqual, Less, LessEqual, Greater, GreaterEqual } op = Op::Begins;
    bool numeric = false;
    double number = 0;
    QString text;      // folded
    bool blank = false;
    bool wild = false;
};
// `value` is what the typed text is as a value (a number when it is one and has no sign before it: it then means "equal to").
// The language a number is read in is `locale` (the workbook's). A pattern with ? and * in it is matched without backtracking, a run of * is one, and a pattern is at most kMaxPatternChars long.
constexpr int kMaxPatternChars = 255;
Condition parseCondition(const QString &typed, const jp::calc::Value &value, const QLocale &locale = QLocale());
bool conditionMet(const Condition &c, const jp::calc::Value &v);

AdvancedFilterResult advancedFilterRows(const EditSession &session, int sheet, const AdvancedFilterSpec &spec);

} // namespace jp::crunch
