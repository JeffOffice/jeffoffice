#pragma once
// What a formula works with: a value is a number, a text, true or false, an error, nothing (an empty cell), or an array of values. No GUI.
//
// The rules of the spreadsheet that workbooks depend on live here, in one place:
//   - a text read as a number ("12", " 1,234.5 ", "$5", "5%", "(3)", "1e3", a date "3/15/2024", a time "10:30") and a number written as text (General, 15 digits);
//   - the order of values for the comparison operators: numbers before text before true/false, text compared without regard to case, an empty cell equal to
//     0, "", or false, whichever the other side is;
//   - 15 significant digits: two numbers that agree to 15 digits are equal for a comparison, and are the same result for the yardstick.

#include "calc/dates.h"

#include <QString>
#include <QStringView>

#include <memory>
#include <vector>

namespace jp::calc {

// The errors a cell can hold, in the order the readers number them (the file spells them "#DIV/0!" and so on).
enum class ErrorKind : quint8 { Null, Div0, Value, Ref, Name, Num, NA, GettingData, Spill, Calc, Field, Blocked, Connect, External, Unknown };
QString errorKindText(ErrorKind error);
// The error a text spells (any case); Unknown, and `known` false, for any other.
ErrorKind errorKindFromText(QStringView text, bool *known = nullptr);

// Characters a text value may hold: a cell holds 32,767, and a longer text is the error #VALUE!.
constexpr int kMaxValueTextChars = 32767;
// Cells in one array value (a range read into an array, an array constant, or the result of an operation on arrays); past it the work is refused.
constexpr qint64 kMaxArrayCells = 2000000;
// Cells of arrays one evaluation may have alive at once (an array takes 56 bytes a cell): more arrays than that are refused as they are asked for, not when they are all built.
constexpr qint64 kMaxLiveArrayCells = 6000000;

struct ArrayData;

// A value. Cheap to copy: a text is shared, and so is an array (which is never changed once another has a copy: set() makes a copy first).
class Value {
public:
    enum class Kind : quint8 { Empty, Number, Text, Bool, Error, Array };

    Value() = default;
    static Value number(double v)
    {
        Value r;
        r.m_kind = Kind::Number;
        r.m_num = v == 0 ? 0.0 : v;   // no negative zero
        return r;
    }
    static Value text(const QString &s)
    {
        Value r;
        r.m_kind = Kind::Text;
        r.m_text = s;
        return r;
    }
    static Value boolean(bool b)
    {
        Value r;
        r.m_kind = Kind::Bool;
        r.m_num = b ? 1 : 0;
        return r;
    }
    static Value error(ErrorKind e)
    {
        Value r;
        r.m_kind = Kind::Error;
        r.m_error = e;
        return r;
    }
    // An array of rows by columns, every element empty. At least 1 by 1; empty (and a null array) if the cells would pass kMaxArrayCells, or the cells an evaluation may
    // have alive at once (kMaxLiveArrayCells: the evaluation is then stopped).
    static Value array(int rows, int cols);

    Kind kind() const { return m_kind; }
    bool isEmpty() const { return m_kind == Kind::Empty; }
    bool isNumber() const { return m_kind == Kind::Number; }
    bool isText() const { return m_kind == Kind::Text; }
    bool isBool() const { return m_kind == Kind::Bool; }
    bool isError() const { return m_kind == Kind::Error; }
    bool isArray() const { return m_kind == Kind::Array; }

    // The number of a Number, 0 or 1 for a Bool; 0 for any other.
    double number() const { return m_num; }
    bool boolean() const { return m_num != 0; }
    const QString &text() const { return m_text; }
    ErrorKind errorKind() const { return m_error; }

    // An array's size; every other value is 1 by 1.
    int rows() const;
    int cols() const;
    // Element (row, col) of an array, counting from 0; for any other value the value itself. An index past the array gives the error #N/A.
    const Value &at(int row, int col) const;
    // Sets an element of an array (a copy is made first if the array is shared). Nothing happens for another value or an index outside.
    void set(int row, int col, const Value &v);

    // One line that says what the value is, for tests and tools: "n:1.5", "s:text", "b:1", "e:#DIV/0!", "-" for empty, and "a:2x2[n:1,n:2;n:3,n:4]" for an array.
    QString debug() const;

    bool operator==(const Value &o) const;
    bool operator!=(const Value &o) const { return !(*this == o); }

private:
    Kind m_kind = Kind::Empty;
    ErrorKind m_error = ErrorKind::Null;
    double m_num = 0;
    QString m_text;
    std::shared_ptr<ArrayData> m_array;
};

struct ArrayData {
    ArrayData() = default;
    ArrayData(const ArrayData &other);   // (the cells of an array count while it lives: see calc/budget.h)
    ArrayData &operator=(const ArrayData &) = delete;
    ~ArrayData();
    int rows = 0, cols = 0;
    std::vector<Value> cells;   // by row
};

// ---- the rules

// The text a number makes when it is joined to other text or compared as text: General format, 15 significant digits ("1.5", "1E+15", "0.1", "-0.0000123").
QString numberToText(double v);

// A text as VALUE() reads it. Spaces around it are ignored. A number may have a sign, a "$", groups of three with ",", a point, an exponent, a "%" (divides by
// 100), or be in parentheses (negative). Else it may be a date ("3/15/2024", "2024-03-15", "15-Mar-2024"), a time ("10:30", "10:30:15", "10:30 PM"), or a date and a
// time. False for anything else, and for the empty text.
bool textToNumber(const QString &text, DateSystem system, double *out);

// Case-insensitive order of two texts: -1, 0, or 1. Symbols come before digits and digits before letters. A letter with an accent is a difference after the letters
// of the whole text: the order is that of the letters without accents (e, é, f), and of the accents only where the letters are the same (cote, coté, côte, côté);
// a letter and the accent that follows it are the letter with the accent. (Texts of the ASCII characters are compared one character at a time.)
int compareText(const QString &a, const QString &b);

// The order of two values that are not errors and not arrays, as the comparison operators see them: -1, 0, or 1. A number is less than a text, and a text less
// than true or false. An empty value is 0 against a number, "" against a text, and false against true or false. Two numbers that are the same to 15 digits are equal.
int compareValues(const Value &a, const Value &b);

// The same number to 15 significant digits (what the yardstick and the comparison operators call equal).
bool sameTo15Digits(double a, double b);
// The number rounded to 15 significant digits.
double roundTo15Digits(double v);

} // namespace jp::calc
