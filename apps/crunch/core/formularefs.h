#pragma once
// The cell references written in a formula's text, where each is, and what they are of, for pointing at cells while a formula is typed: the references are colored in the text and
// outlined on the sheet, and F4 changes which parts of one are fixed ($). No GUI.

#include <QString>
#include <QVector>

#include <vector>

namespace jp::crunch {

struct FormulaRef {
    enum class Kind : quint8 { Cell, Range, Columns, Rows } kind = Kind::Cell;
    int start = 0, length = 0;   // in the text, with the sheet's name in front when there is one
    int bodyStart = 0;           // where the cells' part begins (after "Sheet2!")
    QString sheet;               // the sheet's name as it names it (no quotes); empty for none
    quint32 row1 = 0, col1 = 0, row2 = 0, col2 = 0;   // from 0, the first end the smaller
    int end() const { return start + length; }
};

// The references of a formula (with or without its "="), in order. Not read as references: text in quotes, anything in square brackets (a table's columns), the name of a function
// (LOG10()), and a word that goes on as a name (A1B, Sales.Q1).
std::vector<FormulaRef> findReferences(const QString &text);

// The reference with the parts that are fixed changed, as F4 does: A1, $A$1, A$1, $A1, and back (a column or row reference has one part to fix). `body` is the cells' part, as
// "B2:C3" or "$D:$D", with no sheet.
QString nextFixedForm(const QString &body);

// The text of a reference to cells (rows and columns from 0), with no "$": B3, B2:D5, A:A, B:D, 3:3, 2:5. A range of one cell is that cell.
QString referenceText(FormulaRef::Kind kind, quint32 row1, quint32 col1, quint32 row2, quint32 col2);

// The colors the references of a formula are given, in order (they begin again after the last), as ARGB.
QVector<quint32> referenceColors();

} // namespace jp::crunch
