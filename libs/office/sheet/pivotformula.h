#pragma once
// The formula of a calculated field of a pivot table: a sum of other fields worked out cell by cell. A field's name stands for the total of that field in the cell (in single quotes when it has
// spaces); the numbers, + - * / ^ %, comparisons, & for text, and a few functions are known. Read once, then worked out for each cell.

#include <QString>
#include <QStringList>
#include <QVector>

#include <functional>
#include <memory>

namespace jp::office::sheet {

constexpr int kPivotFormulaMaxChars = 8192;
constexpr int kPivotFormulaMaxNodes = 2000;
constexpr int kPivotFormulaMaxDepth = 64;

struct PivotFormulaValue {
    enum Kind : quint8 { Number, Text, Bool, Error } kind = Number;
    double number = 0;   // a number, or a bool as 0 or 1
    QString text;        // text, or the error as it is spelled (#DIV/0!)
    static PivotFormulaValue num(double d) { PivotFormulaValue v; v.number = d; return v; }
    static PivotFormulaValue err(const QString &e) { PivotFormulaValue v; v.kind = Error; v.text = e; return v; }
};

class PivotFormula {
public:
    struct Node;
    PivotFormula();
    ~PivotFormula();
    PivotFormula(const PivotFormula &) = delete;
    PivotFormula &operator=(const PivotFormula &) = delete;
    // False, with `why`, for a formula that is not well formed or is past a bound.
    static std::shared_ptr<const PivotFormula> parse(const QString &text, QString *why);
    // The names of the fields the formula uses, each once.
    QStringList names() const { return m_names; }
    // The value for a cell: `field` gives a field's total in it (a number, or an error).
    PivotFormulaValue evaluate(const std::function<PivotFormulaValue(const QString &name)> &field) const;

private:
    friend struct Parser;
    QVector<Node> *m_nodes = nullptr;
    int m_root = -1;
    QStringList m_names;
};

}   // namespace jp::office::sheet
