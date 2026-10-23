#pragma once
// What the evaluator and the functions share (internal to calc/): the stack machine's items, and Call, which is what a function sees of its arguments.

#include "calc/area.h"
#include "calc/budget.h"
#include "calc/eval.h"
#include "calc/functions.h"

#include <QHash>

#include <algorithm>
#include <cmath>
#include <memory>
#include <random>
#include <vector>

namespace jp::calc {

// A block of cells of one sheet (or of several, for a 3-D reference: sheets sheet1 through sheet2 by their place).
using RefArea = CellArea;

// What is on the machine's stack: a value, a reference (one area, or several for a union), or an argument that was left out.
struct Item {
    enum class Kind : quint8 { Value, Ref, Missing };
    Kind kind = Kind::Value;
    Value value;
    RefArea area;
    std::shared_ptr<std::vector<RefArea>> more;   // the areas after the first one of a union

    static Item ofValue(const Value &v)
    {
        Item i;
        i.value = v;
        return i;
    }
    static Item ofError(ErrorKind e) { return ofValue(Value::error(e)); }
    static Item ofRef(const RefArea &a)
    {
        Item i;
        i.kind = Kind::Ref;
        i.area = a;
        return i;
    }
    static Item missing()
    {
        Item i;
        i.kind = Kind::Missing;
        return i;
    }
    bool isRef() const { return kind == Kind::Ref; }
    bool isMissing() const { return kind == Kind::Missing; }
    int areaCount() const { return kind == Kind::Ref ? 1 + (more ? int(more->size()) : 0) : 0; }
    const RefArea &areaAt(int i) const { return i == 0 ? area : (*more)[size_t(i - 1)]; }
};

class Machine;

// Values read as the operators read them. A number: a number; true and false as 1 and 0; empty as 0; a text read as a number. A text: a number as General writes it;
// true and false as TRUE and FALSE; empty as "". A boolean: a boolean; a number that is not 0; the text TRUE or FALSE in any case; empty as false. False, with the error to
// give, when the value is an error or can't be read so.
bool coerceNumber(const Value &v, DateSystem system, double *out, ErrorKind *error);
bool coerceText(const Value &v, QString *out, ErrorKind *error);
bool coerceBool(const Value &v, bool *out, ErrorKind *error);

// Does the text match the pattern, from character ti of the text and pi of the pattern? Any case; "?" is one character, "*" any number, "~" makes the next one plain.
// With `prefix` the pattern need only match the start of what is left of the text (SEARCH); else all of it.
// (The steps the matching takes are added to the work of the evaluation with the next charge: see calc/budget.h.)
bool wildcardMatch(const QString &text, int ti, const QString &pattern, int pi, bool prefix = false);
bool hasWildcards(const QString &s);

// The numbers of the arguments of a call from `first` on, as functions of many numbers take them: in a reference or an array the numbers; typed, a text read as a number
// and true and false as 1 and 0. False, with `*error` the error to give, if one is met. With skipSubtotals the cells of references that hold a SUBTOTAL are left out.
bool gatherNumbers(Call &c, int first, std::vector<double> *out, Value *error, bool skipSubtotals = false);

// x to the power y once the cases with no answer are out (0^0, 0 to a negative power, a negative number to a fraction), as the original works it out: a whole
// exponent by squaring and multiplying, any other as exp(y ln x) in double arithmetic. (A correctly rounded pow gives other last digits in one cell of a few hundred, and
// the files kept the digits of the original: with these two rules 21 cells in 3.1 million of the practice workbooks, instead of 1,900, differ by more than a few units of the last place.)
double powerOf(double x, double y);

// A sum or difference of numbers that cancel is zero where what is left is no more than the rounding error of the terms. The limits were found by comparing with the
// results of the files (a wider one makes a zero of differences the files keep, a narrower one keeps the rounding error of differences they make zero): the last
// operation of a formula is zero within 2^-49 of the larger term (9450 - 9449.999999999989 is zero; 0.01 - 0.00999999999999997 is not), and SUM, at each step, within 2^-50.
constexpr double kOperatorSnap = 1.7763568394002505e-15;
constexpr double kSumSnap = 8.881784197001252e-16;
inline double snappedSum(double a, double b)
{
    const double r = a + b;
    if (r != 0 && a != 0 && b != 0 && (a < 0) != (b < 0) && std::fabs(r) <= std::max(std::fabs(a), std::fabs(b)) * kSumSnap) return 0;
    return r;
}

inline Value numberResult(double r)
{
    return std::isfinite(r) ? Value::number(r) : Value::error(ErrorKind::Num);
}

// A rectangular view of an area of cells or of an array, for the functions that look things up: element (row, col) counted from the top left corner.
class Table {
public:
    // The rows and columns that hold anything or may (a reference to whole columns is cut to the sheet's last row), and all of those the reference names.
    qint64 rows() const { return m_rows; }
    qint64 cols() const { return m_cols; }
    qint64 fullRows() const { return m_fullRows; }
    qint64 fullCols() const { return m_fullCols; }
    Value at(qint64 row, qint64 col) const;
    // Along column 0 (`column`) or row 0 of the table: the first place from i to `last` that holds something, or last + 1 if there is none; and the last place from i
    // back to `first` that does, or first - 1. For cells these are found among those the sheet stores, not by looking at each place.
    qint64 nextFilled(bool column, qint64 i, qint64 last) const;
    qint64 previousFilled(bool column, qint64 i, qint64 first) const;
    // Where the table starts on its sheet (for a reference).
    bool isRef() const { return m_isRef; }
    const RefArea &area() const { return m_area; }

private:
    friend class Call;
    const Machine *m_machine = nullptr;
    bool m_isRef = false;
    RefArea m_area;
    Value m_array;
    qint64 m_rows = 0, m_cols = 0, m_fullRows = 0, m_fullCols = 0;
};

class Call {
public:
    Call(Machine &machine, std::vector<Item> &args, bool arrayMode, const FunctionInfo &function) : m(machine), m_args(args), m_array(arrayMode), m_function(function) {}

    int argc() const { return int(m_args.size()); }
    bool missing(int i) const { return i >= argc() || m_args[size_t(i)].isMissing(); }
    const Item &item(int i) const { return m_args[size_t(i)]; }
    // The formula is worked out as an array formula (or this call is in the argument of a function that takes arrays).
    bool arrayMode() const { return m_array; }
    Machine &machine() const { return m; }
    const EvalContext &context() const;

    // The value of argument i as one value: a value as it is, a reference by the cell it names (a block by implicit intersection), an array by its first element. An
    // argument left out is an empty value.
    Value scalar(int i);
    // The argument as a number (a text is read as one; true is 1, false 0; an empty value is 0). False, with the error to give kept for failure(), if it can't be.
    bool number(int i, double *out);
    // The same, whole: cut toward zero. False too if it is past +-2^53.
    bool integer(int i, qint64 *out);
    bool text(int i, QString *out);
    bool boolean(int i, bool *out);
    // The same for an argument that may be left out: `def` then.
    bool numberOr(int i, double def, double *out) { return missing(i) ? (*out = def, true) : number(i, out); }
    bool integerOr(int i, qint64 def, qint64 *out) { return missing(i) ? (*out = def, true) : integer(i, out); }
    bool booleanOr(int i, bool def, bool *out) { return missing(i) ? (*out = def, true) : boolean(i, out); }

    // The error to give after a helper above returned false (or one set by error()).
    Value failure() const { return m_failure; }
    Value error(ErrorKind e)
    {
        m_failure = Value::error(e);
        return m_failure;
    }
    // Gives a reference instead of a value (OFFSET, INDEX, INDIRECT, IF, CHOOSE): the function then returns anything.
    Value returnRef(const Item &ref)
    {
        m_result = ref;
        m_hasResult = true;
        return Value();
    }
    bool hasResult() const { return m_hasResult; }
    const Item &result() const { return m_result; }

    // Calls f(value, typed) for each value in argument i that a function of many values takes: for a reference, each cell that holds something (typed is false); for an
    // array, each element that holds something (false); for a typed value, it once (true); an argument left out once, as an empty value (true). f gives false to stop.
    // False if f stopped, or the work gave out.
    bool each(int i, const std::function<bool(const Value &, bool typed)> &f);
    // The same, with the place of each: the row and column of a cell on its sheet, or of an element in its array (from 0).
    bool eachAt(int i, const std::function<bool(const Value &, quint32 row, quint32 col)> &f);
    // The same, with the sheet of each cell as well (-1 for an element of an array).
    bool eachCell(int i, const std::function<bool(const Value &, int sheet, quint32 row, quint32 col)> &f);

    // Argument i as a rectangle of values; false (and failure() the error) if it is several blocks, 3-D, or too large.
    bool table(int i, Table *out, bool clip = true);
    // Argument i as an array of values (a reference is read into one).
    bool array(int i, Value *out);

private:
    friend class Machine;
    Machine &m;
    std::vector<Item> &m_args;
    bool m_array;
    const FunctionInfo &m_function;
    Value m_failure = Value::error(ErrorKind::Value);
    Item m_result;
    bool m_hasResult = false;
};

class Machine {
public:
    Machine();
    Value run(const Program &program, const EvalContext &context);

    // ---- what functions and operators use
    const EvalContext &context() const { return ctx; }
    const CellSource &source() const { return *ctx.source; }
    // Counts n steps; false when the evaluation has gone past its limit (then stopped() is true).
    bool charge(qint64 n)
    {
        work += n + budget::takeWork();
        if (work > limit && !over) {
            over = true;
            if (status == EvalStatus::Ok) status = EvalStatus::TooMuchWork;
        }
        if (!over && budget::refused()) tooLarge();   // an array was refused for the cells alive at once
        return !over;
    }
    bool stopped() const { return over; }
    void tooLarge()
    {
        over = true;
        status = EvalStatus::TooLarge;
    }
    // Counts the characters of a text about to be made as an element of an array; false when that would be more than the evaluation may make (then stopped() is true).
    // Called before the text is made: one that would pass the budget is not made, and is not counted.
    bool chargeText(qint64 chars)
    {
        if (textMade + chars > kMaxMadeTextChars) {
            if (!over) tooLarge();
            return false;
        }
        textMade += chars;
        return charge(chars / 64);   // (making it takes time too)
    }
    // An array of rows by columns, empty: or, if it may not be made, the error #VALUE! and the evaluation is stopped.
    Value newArray(int rows, int cols)
    {
        Value a = Value::array(rows, cols);
        if (!a.isArray()) {
            tooLarge();
            return Value::error(ErrorKind::Value);
        }
        return a;
    }
    Value cell(int sheet, quint32 row, quint32 col) const { return ctx.source->cell(sheet, row, col); }
    // The value of an item as one value (see Call::scalar).
    Value scalar(const Item &item, bool arrayMode);
    // An item read into an array (a reference into its cells, a value into itself); an error value if it is several blocks or too large.
    Value toArray(const Item &item);
    // The implicit intersection of an area with the cell of the formula: the cell's value, or #VALUE!.
    Value intersect(const RefArea &area) const;
    // A text as a program for a defined name or INDIRECT: the item it comes to (a reference, a value, or an error).
    // `relativeToA1`: the references in it that move are those of a defined name, written for the cell A1 and moved to the cell of the formula.
    // `identity` says what is being worked out for a circle to be seen (a name is its scope and its name; a text given to INDIRECT is the text, which is what none says).
    Item evaluateText(const QString &formula, bool r1c1, bool relativeToA1, int sheetForNames, const QString &identity = QString());
    // The sheets a reference through several covers, as places; -1 if one is not there.
    int sheetCount() const { return ctx.source->sheetCount(); }
    // The last row and column that hold anything, plus one.
    void usedExtent(int sheet, quint32 *rows, quint32 *cols) const { ctx.source->usedExtent(sheet, rows, cols); }
    // The moment NOW and TODAY give: the context's, else the clock read once for the evaluation.
    double now() const;
    std::mt19937_64 &random() { return rng; }

    EvalContext ctx;
    qint64 work = 0, limit = kMaxEvalWork;
    qint64 peakCells = 0;   // the most cells of arrays alive at once in the last evaluation
    qint64 textMade = 0;    // the characters of text made as elements of arrays, in this one
    bool over = false;
    EvalStatus status = EvalStatus::Ok;
    std::mt19937_64 rng;
    int nameDepth = 0;
    int unsupported = 0;
    mutable double m_clock = 0;
    mutable bool m_clockRead = false;
    bool wrapRefs = false;   // the references that move are those of a defined name: past the edge of the sheet they come round to the other side

private:
    friend class Evaluator;
    struct Frame;
    Item runProgram(const Program &program, bool arrayFormula);
    Item callFunction(const FunctionInfo &f, std::vector<Item> &args, bool arrayMode);
    Item binary(Op op, quint8 flags, const Item &a, const Item &b);
    Item unaryOp(Op op, quint8 flags, const Item &a);
    Item referenceOp(Op op, const Item &a, const Item &b);
    void noteRefs(const Item &item);
    bool resolveRef(const Program &p, const RefLit &r, RefArea *area, ErrorKind *error);
    bool bindSheets(const Program &p);

    std::vector<Item> stack;
    struct Cached {
        std::shared_ptr<const Program> program;
    };
    QHash<QString, Cached> m_texts;
    qint64 m_textsBytes = 0;   // about what the texts above, their programs, and their keys take
    std::vector<QString> m_active;   // what is being worked out (a name, or a text), the one inside the other
};

} // namespace jp::calc
