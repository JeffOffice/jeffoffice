#pragma once
// The yardstick for formulas (docs/jeffcrunch.md): a workbook stores the last result of every formula, so each formula cell can be worked out once, reading every
// cell it refers to from the values the file stored, and the answer compared with the formula's own stored result. This checks each formula on its own; recalculating
// a whole workbook from its inputs is the dependency graph's business.
//
// A number matches to 15 significant digits; a text, true or false, and the kind of an error match exactly. Kept apart, as no mismatch: a formula that calls a function the
// registry lacks, one that calls a function whose value changes with no cell changing (NOW, TODAY, RAND, RANDBETWEEN, OFFSET, INDIRECT), one that refers to another
// workbook or to the columns of a table, a data table, a formula the parser does not take, a formula the file stored no result for, and one the work bound or the time given
// stopped.

#include "calc/eval.h"
#include "sheet/sheetreader.h"

#include <QMap>
#include <QString>
#include <QStringList>

#include <functional>
#include <memory>
#include <vector>

namespace jp::office::sheet {

// The cells of a whole workbook, read into memory, as the formula engine asks for them.
class WorkbookSource : public jp::calc::CellSource {
public:
    explicit WorkbookSource(const Workbook &book);
    // Reads every sheet. False if one could not be read whole (what was read stays, and notWhole() says which).
    bool loadAll();
    QStringList notWhole() const { return m_notWhole; }
    int sheetCount() const override { return int(m_sheets.size()); }
    int sheetIndex(const QString &name) const override;
    jp::calc::Value cell(int sheet, quint32 row, quint32 col) const override;
    void visit(int sheet, quint32 row1, quint32 col1, quint32 row2, quint32 col2, const std::function<bool(quint32, quint32, const jp::calc::Value &)> &f) const override;
    void usedExtent(int sheet, quint32 *rows, quint32 *cols) const override;
    QString definedName(int sheet, const QString &name) const override;

    const Sheet &sheet(int i) const { return m_sheets[size_t(i)]; }
    const SharedStrings &shared() const { return m_book.sharedStrings(); }
    jp::calc::Value valueOf(const Sheet &sheet, const Cell &cell) const;

private:
    const Workbook &m_book;
    std::vector<Sheet> m_sheets;
    std::vector<quint32> m_rows, m_cols;
    QHash<QString, int> m_byName;
    QHash<QString, QString> m_names;   // "<sheet>|<NAME>" -> what it stands for; the sheet counted from 0, or -1
    QStringList m_notWhole;
};

enum class RecalcClass : char {
    Match = 'M',
    Mismatch = 'X',
    Unsupported = 'U',   // calls a function that is not in the registry
    Volatile = 'V',
    External = 'E',
    Structured = 'T',
    ParseError = 'P',
    NoResult = 'N',
    DataTable = 'D',
    Limit = 'L',
    Skipped = 'S',       // not looked at: the time given ran out
};

struct RecalcCell {
    QString sheet, cell, formula, stored, ours;
    RecalcClass cls = RecalcClass::Match;
    QStringList functions;   // the functions it calls, upper case, once each
};

struct RecalcOptions {
    double seconds = 0;          // 0: no limit
    qint64 workLimit = 0;        // steps for one formula; 0: the engine's own
    int keepMatches = 0;         // how many matching cells to hand to `each`, besides all the others
    bool volatileToo = true;     // work out the volatile formulas that always give the same (OFFSET, INDIRECT) and count them apart
};

struct FunctionTally {
    qint64 calls = 0, matchedCalls = 0;   // in the formulas that were checked
    qint64 cells = 0, matchedCells = 0;
};

struct RecalcReport {
    qint64 formulaCells = 0;
    QMap<char, qint64> byClass;
    QMap<QString, FunctionTally> functions;
    QMap<QString, qint64> unsupportedFunctions;   // unknown function -> cells that call it
    QMap<QString, qint64> patterns;               // how mismatches look: "stored -> ours: functions" -> count
    qint64 volatileChecked = 0, volatileMatched = 0;   // volatile formulas of functions that don't change by themselves, worked out anyway
    bool manualCalc = false;
    bool whole = true;                            // every sheet was read whole
    double seconds = 0;
    qint64 checked() const { return byClass.value('M') + byClass.value('X'); }
};

// Works out every formula cell of the workbook and compares it with the result stored. `each` is called for every cell that is not a match (and for the first
// keepMatches that are). False, with `error`, only if the workbook can't be read at all.
bool recalcWorkbook(const Workbook &book, const RecalcOptions &options, const std::function<void(const RecalcCell &)> &each, RecalcReport *report, QString *error = nullptr);

} // namespace jp::office::sheet
