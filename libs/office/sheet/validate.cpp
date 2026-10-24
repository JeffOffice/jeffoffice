// The rules of validation: which rule is for a cell, whether an entry is allowed by it, and the choices of a list. A rule's formulas are worked out as the program works formulas out, with
// the entry in the cell it is for.
#include "sheet/calculation.h"

#include "calc/eval.h"
#include "calc/formula.h"
#include "sheet/cellref.h"

#include <algorithm>
#include <cmath>

namespace jp::office::sheet {

using jp::calc::CellArea;
using jp::calc::Value;

namespace {

// The cells of a source, except that one of them has another value: what the rule's formula is worked out over when the entry is not yet in the cell.
class Overlay : public jp::calc::CellSource {
public:
    Overlay(const jp::calc::CellSource &base, int sheet, quint32 row, quint32 col, const Value &value) : m_base(base), m_sheet(sheet), m_row(row), m_col(col), m_value(value) {}
    int sheetCount() const override { return m_base.sheetCount(); }
    int sheetIndex(const QString &name) const override { return m_base.sheetIndex(name); }
    Value cell(int sheet, quint32 row, quint32 col) const override { return is(sheet, row, col) ? m_value : m_base.cell(sheet, row, col); }
    void visit(int sheet, quint32 row1, quint32 col1, quint32 row2, quint32 col2, const std::function<bool(quint32, quint32, const Value &)> &f) const override
    {
        if (sheet != m_sheet || m_row < row1 || m_row > row2 || m_col < col1 || m_col > col2) {
            m_base.visit(sheet, row1, col1, row2, col2, f);
            return;
        }
        // (in order of row and column, with the cell's value in its place)
        bool given = false, go = true;
        m_base.visit(sheet, row1, col1, row2, col2, [&](quint32 r, quint32 c, const Value &v) {
            if (!given && (r > m_row || (r == m_row && c >= m_col))) {
                given = true;
                if (!f(m_row, m_col, m_value)) {
                    go = false;
                    return false;
                }
                if (r == m_row && c == m_col) return true;
            }
            return f(r, c, v);
        });
        if (!given && go) f(m_row, m_col, m_value);
    }
    void usedExtent(int sheet, quint32 *rows, quint32 *cols) const override
    {
        m_base.usedExtent(sheet, rows, cols);
        if (sheet == m_sheet) {
            *rows = std::max(*rows, m_row + 1);
            *cols = std::max(*cols, m_col + 1);
        }
    }
    QString definedName(int sheet, const QString &name) const override { return m_base.definedName(sheet, name); }
    bool isSubtotal(int sheet, quint32 row, quint32 col) const override { return m_base.isSubtotal(sheet, row, col); }
    qint64 generation() const override { return m_base.generation(); }

private:
    bool is(int sheet, quint32 row, quint32 col) const { return sheet == m_sheet && row == m_row && col == m_col; }
    const jp::calc::CellSource &m_base;
    int m_sheet;
    quint32 m_row, m_col;
    Value m_value;
};

bool numberOf(const Value &v, double *out)
{
    switch (v.kind()) {
    case Value::Kind::Number: *out = v.number(); return true;
    case Value::Kind::Bool: *out = v.boolean() ? 1 : 0; return true;
    case Value::Kind::Empty: *out = 0; return true;
    case Value::Kind::Text: return jp::calc::textToNumber(v.text(), jp::calc::DateSystem::Date1900, out);
    default: return false;
    }
}

// The text a value is entered as (what is compared with the choices of a list).
QString textOfValue(const Value &v)
{
    switch (v.kind()) {
    case Value::Kind::Number: return jp::calc::numberToText(v.number());
    case Value::Kind::Text: return v.text();
    case Value::Kind::Bool: return v.boolean() ? QStringLiteral("TRUE") : QStringLiteral("FALSE");
    case Value::Kind::Error: return jp::calc::errorKindText(v.errorKind());
    default: return QString();
    }
}

} // namespace

const Validation *Calculation::validationAt(int sheet, quint32 row, quint32 col) const
{
    if (sheet < 0 || sheet >= int(m_sheets.size())) return nullptr;
    for (const Validation &v : m_sheets[size_t(sheet)].validations)
        for (const CellRange &r : v.ranges)
            if (r.contains(row, col)) return &v;
    return nullptr;
}

Value Calculation::ruleFormula(const Validation &rule, const QString &text, int sheet, quint32 row, quint32 col, const jp::calc::CellSource &source, bool asArray) const
{
    if (text.trimmed().isEmpty()) return Value();
    // the formula is written for the first cell of the rule; for this cell its references are moved the same distance
    QString shifted = text;
    if (!rule.ranges.isEmpty()) shifted = offsetFormula(text, int(qint64(row) - rule.ranges.first().row1), int(qint64(col) - rule.ranges.first().col1));
    jp::calc::ParseOptions po;
    po.arrayFormula = asArray;
    po.definedName = asArray;
    const jp::calc::ParseResult parsed = jp::calc::parseFormula(shifted, po);
    if (!parsed.ok()) return Value::error(jp::calc::ErrorKind::Name);
    jp::calc::Evaluator evaluator;
    evaluator.setWorkLimit(2000000);
    jp::calc::EvalContext context;
    context.source = &source;
    context.sheet = sheet;
    context.row = row;
    context.col = col;
    context.dateSystem = m_book.info().dateSystem;
    return evaluator.evaluate(*parsed.program, context);
}

QStringList Calculation::listChoices(int sheet, quint32 row, quint32 col, const Validation &rule) const
{
    QStringList out;
    const QString f = rule.formula1.trimmed();
    if (f.isEmpty()) return out;
    if (f.startsWith(QLatin1Char('"'))) {
        // the items are in the formula: "Yes,No,Maybe"
        QString inner = f.mid(1);
        if (inner.endsWith(QLatin1Char('"'))) inner.chop(1);
        inner.replace(QStringLiteral("\"\""), QStringLiteral("\""));
        for (const QString &item : inner.split(QLatin1Char(','))) out << item.trimmed();
        return out;
    }
    const Value v = ruleFormula(rule, rule.formula1, sheet, row, col, *this, true);
    if (v.isArray()) {
        for (int r = 0; r < v.rows() && out.size() < 5000; ++r)
            for (int c = 0; c < v.cols() && out.size() < 5000; ++c) {
                const Value &item = v.at(r, c);
                if (!item.isEmpty() && !item.isError()) out << textOfValue(item);
            }
    } else if (!v.isEmpty() && !v.isError()) {
        out << textOfValue(v);
    }
    return out;
}

bool Calculation::entryAllowed(int sheet, quint32 row, quint32 col, const Validation &rule, const Value &entry, const QString &typed) const
{
    using T = Validation::Type;
    using C = Validation::Compare;
    if (rule.type == T::Any) return true;
    if (entry.isEmpty()) return rule.allowBlank;
    if (rule.type == T::List) {
        const QString f = rule.formula1.trimmed();
        const bool literal = f.startsWith(QLatin1Char('"'));
        const QString text = typed.isEmpty() ? textOfValue(entry) : typed;
        double wanted = 0, item = 0;
        const bool wantedNumber = entry.isNumber();
        if (wantedNumber) wanted = entry.number();
        for (const QString &choice : listChoices(sheet, row, col, rule)) {
            if (wantedNumber && jp::calc::textToNumber(choice, jp::calc::DateSystem::Date1900, &item) && jp::calc::sameTo15Digits(item, wanted)) return true;
            if (literal ? choice == text : choice.compare(text, Qt::CaseInsensitive) == 0) return true;
        }
        return false;
    }
    const jp::calc::CellSource &base = *this;
    Overlay overlay(base, sheet, row, col, entry);
    if (rule.type == T::Custom) {
        const Value v = ruleFormula(rule, rule.formula1, sheet, row, col, overlay, false);
        double n = 0;
        return !v.isError() && numberOf(v, &n) && n != 0;
    }
    // the limits are numbers; the entry is a number (or, for a length, its text)
    double x = 0;
    if (rule.type == T::TextLength) {
        x = double((typed.isEmpty() ? textOfValue(entry) : typed).size());
    } else {
        if (!entry.isNumber()) return false;
        x = entry.number();
        if (rule.type == T::Whole && x != std::floor(x)) return false;
    }
    double a = 0, b = 0;
    const Value v1 = ruleFormula(rule, rule.formula1, sheet, row, col, base, false), v2 = ruleFormula(rule, rule.formula2, sheet, row, col, base, false);
    const bool two = rule.compare == C::Between || rule.compare == C::NotBetween;
    if (!numberOf(v1, &a) || (two && !numberOf(v2, &b))) return true;   // (a limit that is no number: the rule can't be kept, so it lets the entry in)
    switch (rule.compare) {
    case C::Between: return x >= std::min(a, b) && x <= std::max(a, b);
    case C::NotBetween: return x < std::min(a, b) || x > std::max(a, b);
    case C::Equal: return x == a;
    case C::NotEqual: return x != a;
    case C::Greater: return x > a;
    case C::Less: return x < a;
    case C::GreaterEqual: return x >= a;
    case C::LessEqual: return x <= a;
    }
    return true;
}

} // namespace jp::office::sheet
