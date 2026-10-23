#pragma once
// Working a formula out: a program (calc/formula.h) run over a source of cells. The source is abstract: the workbook model plugs into it, and so can a test's grid.
// No GUI. The arithmetic follows what workbooks depend on: a text read as a number, an empty cell as 0 or "", errors passing through, 15 digits, a sum that
// should be zero coming out as zero. See calc/value.h for the rules of values.
//
// Work is bounded: one evaluation does at most a fixed number of steps (an operation, a function call, or a cell read from a range), builds no array of more than
// kMaxArrayCells cells and has no more than kMaxLiveArrayCells alive at once, nests no name or INDIRECT deeper than kMaxNameDepth, and makes no text of more than
// kMaxValueTextChars characters, nor in all kMaxMadeTextChars characters of them in arrays. Past a bound the evaluation stops and status() says which; the value is then an error.

#include "calc/area.h"
#include "calc/formula.h"
#include "calc/value.h"

#include <QString>

#include <functional>
#include <memory>
#include <vector>

namespace jp::calc {

constexpr qint64 kMaxEvalWork = 50000000;
constexpr int kMaxNameDepth = 16;
// The characters of text one evaluation may make as the elements of arrays (a text is two bytes a character): a column of long texts joined to itself is millions of them.
constexpr qint64 kMaxMadeTextChars = 64000000;

class CellSource {
public:
    virtual ~CellSource() = default;
    // The sheets, by place. A name is matched without regard to case; -1 for none.
    virtual int sheetCount() const = 0;
    virtual int sheetIndex(const QString &name) const = 0;
    // What the cell holds (an empty value for none). A cell with a formula gives its result.
    virtual Value cell(int sheet, quint32 row, quint32 col) const = 0;
    // Calls f(row, col, value) for each cell of the area that holds anything, in order of row and then column; f gives false to stop. A cell that is just
    // there (formatted, no value) may be called with an empty value, or left out.
    virtual void visit(int sheet, quint32 row1, quint32 col1, quint32 row2, quint32 col2, const std::function<bool(quint32, quint32, const Value &)> &f) const = 0;
    // The last row and column (counting from 0) that hold anything, plus one: 0, 0 for an empty sheet.
    virtual void usedExtent(int sheet, quint32 *rows, quint32 *cols) const = 0;
    // What a defined name stands for, as formula text ("Sheet1!$A$1:$B$9", "0.05", "OFFSET(...)"): the name of the sheet if it has one for that sheet, else the
    // workbook's. Empty if there is none.
    virtual QString definedName(int sheet, const QString &name) const { Q_UNUSED(sheet); Q_UNUSED(name); return {}; }
    // The cell holds a formula that is a SUBTOTAL, which SUBTOTAL leaves out of what it adds up.
    virtual bool isSubtotal(int sheet, quint32 row, quint32 col) const { Q_UNUSED(sheet); Q_UNUSED(row); Q_UNUSED(col); return false; }
    // Changes when the sheets or their names do, so a program re-reads which sheet each name is (a source that is replaced by another at the same address must
    // change it too).
    virtual qint64 generation() const { return 0; }
};

struct EvalContext {
    const CellSource *source = nullptr;
    int sheet = 0;                       // the sheet of the cell the formula is in, and the cell (counting from 0)
    quint32 row = 0, col = 0;
    int shiftRows = 0, shiftCols = 0;    // how far that cell is from the one the formula's text was written for (a shared formula)
    DateSystem dateSystem = DateSystem::Date1900;
    double now = 0;                      // the day number NOW() and TODAY() give; 0 reads the clock, once for the evaluation (see clockSerial)
    // If set, every block of cells the evaluation names is added to it: what the formula's own references name (as they were worked out), what a name or a text given
    // to INDIRECT stands for, and what OFFSET and the like give. What a formula that reads where its values say it reads, is told to the dependency graph with.
    std::vector<CellArea> *touched = nullptr;
};

// The clock as a day number with the time as its fraction, in the date system. A recalculation of many formulas reads it once and gives it to each as EvalContext::now,
// so every NOW in it is the same moment.
double clockSerial(DateSystem system);

enum class EvalStatus : quint8 { Ok, TooMuchWork, TooLarge, TooDeep };

// An Evaluator, and a Program while it is run (it remembers which sheet each name it uses is), belong to one thread at a time.
class Evaluator {
public:
    Evaluator();
    ~Evaluator();
    Evaluator(const Evaluator &) = delete;
    Evaluator &operator=(const Evaluator &) = delete;

    // The value of the program in the context. A reference in the end is read (the cell's value; an empty cell is 0; a range is its first cell, or the
    // cell the formula's own row and column cross, or the whole array for an array formula).
    Value evaluate(const Program &program, const EvalContext &context);
    // How the last evaluation ended.
    EvalStatus status() const;
    // What the last evaluation met that is not worked out, through a defined name or a text given to INDIRECT as well as in the formula: 1 a reference to another
    // workbook, 2 the columns of a table, 4 a function that is not in the registry. 0 when it met none; the value is then right as far as the engine can say.
    int unsupportedKinds() const;
    // The steps the last evaluation took.
    qint64 work() const;
    // The most cells of arrays the last evaluation had alive at once.
    qint64 peakArrayCells() const;
    // The characters of text the last evaluation made as the elements of arrays (counted before they are made; never more than kMaxMadeTextChars).
    qint64 madeTextChars() const;
    void setWorkLimit(qint64 steps);
    // The numbers RAND gives follow from this.
    void seedRandom(quint64 seed);

private:
    struct Impl;
    std::unique_ptr<Impl> d;
};

} // namespace jp::calc
