#pragma once
// The yardstick for formulas (docs/jeffcrunch.md): a workbook stores the last result of every formula, so each formula cell can be worked out once, reading every
// cell it refers to from the values the file stored, and the answer compared with the formula's own stored result. This checks each formula on its own; recalculating
// a whole workbook from its inputs is the dependency graph's business.
//
// A number matches to 15 significant digits (or when they are a few units of the last place apart: two math libraries differ there); a text, true or false, and the kind
// of an error match exactly. Kept apart, as no mismatch: a formula that calls a function the
// registry lacks, one that calls a function whose value changes with no cell changing (NOW, TODAY, RAND, RANDBETWEEN, OFFSET, INDIRECT), one that refers to another
// workbook or to the columns of a table, a data table, a formula the parser does not take, a formula the file stored no result for, and one the work bound or the time given
// stopped.

#include "calc/eval.h"
#include "sheet/sheetreader.h"

#include <QMap>
#include <QString>
#include <QStringList>

#include <deque>
#include <unordered_map>
#include <functional>
#include <memory>
#include <vector>

namespace jp::office::sheet {

// What all the sheets of a workbook may take to hold, in bytes as the reader counts them (kMaxSheetBytes for one sheet; 207 MB is the most counted for a sheet of the
// practice workbooks, and 414 MB for a workbook): past this, the sheets not yet read are left out.
constexpr qint64 kMaxWorkbookBytes = 2000000000;

// The cells of a whole workbook, read into memory, as the formula engine asks for them.
class WorkbookSource : public jp::calc::CellSource {
public:
    explicit WorkbookSource(const Workbook &book, qint64 maxBytes = kMaxWorkbookBytes);
    // Reads every sheet. False if one could not be read whole (what was read stays, and notWhole() says which).
    bool loadAll();
    // Called before each sheet is read (with 0 cells) and as the read of it goes on (every few thousand cells, and every so many bytes), with the sheet's place and the cells
    // read of it; false stops the load, and the sheet that was being read, and those after it, are not whole.
    void setProgress(const std::function<bool(int sheet, qint64 cells)> &progress) { m_progress = progress; }
    QStringList notWhole() const { return m_notWhole; }
    int sheetCount() const override { return int(m_sheets.size()); }
    int sheetIndex(const QString &name) const override;
    jp::calc::Value cell(int sheet, quint32 row, quint32 col) const override;
    void visit(int sheet, quint32 row1, quint32 col1, quint32 row2, quint32 col2, const std::function<bool(quint32, quint32, const jp::calc::Value &)> &f) const override;
    void usedExtent(int sheet, quint32 *rows, quint32 *cols) const override;
    QString definedName(int sheet, const QString &name) const override;
    bool isSubtotal(int sheet, quint32 row, quint32 col) const override;
    int rowHiddenKind(int sheet, quint32 row) const override;
    // The rows hidden on the sheet changed (hidden, shown, filtered): they are looked up again.
    void hiddenRowsChanged(int sheet) const;
    jp::calc::StructuredAnswer structuredRef(const QString &text, int sheet, quint32 row, quint32 col, jp::calc::CellArea *area, jp::calc::ErrorKind *error) const override;
    // The tables changed (made, taken out, renamed, moved, resized): the next reference by a table's name looks them up again. Call with the worker held back.
    void tablesChanged() const { m_tableIndexReady = false; }

    // The defined names by "<sheet>|<NAME>" (the sheet counted from 0, or -1), and what each stands for (for the yardstick of saving).
    const QHash<QString, QString> &definedNameTexts() const { return m_names; }
    const Sheet &sheet(int i) const { return m_sheets[size_t(i)]; }
    const SharedStrings &shared() const { return m_book.sharedStrings(); }
    jp::calc::Value valueOf(const Sheet &sheet, const Cell &cell) const;

protected:
    const Workbook &m_book;
    std::deque<Sheet> m_sheets;   // (a deque: a sheet added later does not move those there are, which the window holds pointers to)
    std::vector<quint32> m_rows, m_cols;
    mutable std::vector<std::vector<char>> m_subtotalTexts;   // for each sheet, for each text of a formula: 1 if it calls SUBTOTAL, 0 if not, -1 not yet asked
    mutable std::vector<std::unordered_map<quint32, quint8>> m_hiddenRows;   // for each sheet, the rows that are hidden and why (1 by hand, 2 by a filter), made when first asked
    mutable std::vector<char> m_hiddenRowsReady;
    mutable bool m_tableIndexReady = false;
    mutable QHash<QString, int> m_tableByName;                   // by the lower-case name, the table's place among the workbook's
    mutable std::vector<int> m_lastTableOfSheet;                 // the table a reference with no name was last found in, by the sheet (formulas of one table are worked out one after another)
    mutable std::vector<std::vector<int>> m_tablesOfSheet;       // by the sheet, the places of its tables
    QHash<QString, int> m_byName;
    QHash<QString, QString> m_names;   // "<sheet>|<NAME>" -> what it stands for; the sheet counted from 0, or -1
    QStringList m_notWhole;
    qint64 m_maxBytes;
    std::function<bool(int, qint64)> m_progress;
};

enum class RecalcClass : char {
    Match = 'M',
    Mismatch = 'X',
    Unsupported = 'U',   // calls a function that is not in the registry
    Volatile = 'V',
    External = 'E',
    Structured = 'T',
    ParseError = 'P',
    NoResult = 'N',      // the file stored no result for it, or the formula element holds no formula
    DataTable = 'D',
    Limit = 'L',
    Skipped = 'S',       // not looked at: the time given ran out
    Tainted = 'Z',       // (full recalculation) worked out from a formula that was kept apart: not counted, but told whether it matches
    Iterative = 'I',     // (full recalculation) in a circle of a workbook that asks for iterative calculation
    Circular = 'C',      // (full recalculation) in a circle: the file's value is kept (what the last calculation left), and marks what reads it
    Iterated = 'R',      // (full recalculation) calls a function found by trying numbers (IRR, XIRR): the file's digits are those of one program's iteration, so it is kept apart
};

struct RecalcCell {
    QString sheet, cell, formula, stored, ours;
    RecalcClass cls = RecalcClass::Match;
    QStringList functions;   // the functions it calls, upper case, once each
    bool cascaded = false;   // (full recalculation) a mismatch that reads a formula that is one
};

struct RecalcOptions {
    double seconds = 0;          // 0: no limit
    qint64 workLimit = 0;        // steps for one formula; 0: the engine's own
    int keepMatches = 0;         // how many matching cells to hand to `each`, besides all the others
    bool volatileToo = true;     // work out the volatile formulas that always give the same (OFFSET, INDIRECT) and count them apart
    qint64 workbookBytes = kMaxWorkbookBytes;   // what the sheets may take together (the ones not read before it is passed are left out)
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

// ---- the full recalculation (docs/jeffcrunch.md): every formula worked out from the constants, in dependency order, not from the results the file stored.

struct FullOptions {
    double seconds = 0;          // 0: no limit; what the time did not reach is kept apart
    qint64 workLimit = 0;        // steps for one formula; 0: the engine's own
    int keepMatches = 0;         // how many matching cells to hand to `each`, besides all the others
    qint64 workbookBytes = kMaxWorkbookBytes;
    double now = 45366.5;        // the time NOW and TODAY give (the formulas that change by themselves are kept apart, so this reaches only those that read the clock otherwise)
};

struct FullReport {
    qint64 formulaCells = 0;
    QMap<char, qint64> byClass;                   // M X U V E T P N D L I C R Z S
    QMap<QString, FunctionTally> functions;       // for the formulas that were checked
    QMap<QString, qint64> unsupportedFunctions;
    QMap<QString, qint64> patterns;
    qint64 volatileChecked = 0, volatileMatched = 0;   // OFFSET and INDIRECT, worked out and counted apart
    qint64 taintedMatched = 0;                    // of the formulas worked out from one that was kept apart, those that match what the file stored
    qint64 circular = 0;                          // formulas in a circle (kept apart: the engine's own result is 0), and of them those the file stored 0 for
    qint64 circularStoredZero = 0;
    qint64 rootMismatches = 0;                    // of the mismatches, those that read no formula that is a mismatch: wrong of their own, where the others follow a wrong one
    qint64 evaluations = 0;                       // times the engine worked a formula out (one that waited for another is worked out again)
    qint64 graphEntries = 0, graphBytes = 0, untracked = 0;
    qint64 sheetXmlBytes = 0;                     // the most any one sheet's part had
    bool manualCalc = false;
    bool whole = true;
    double loadSeconds = 0, calcSeconds = 0, seconds = 0;
    qint64 checked() const { return byClass.value('M') + byClass.value('X'); }
};

// Reads the workbook, works out every formula, and compares each with the result the file stored. `each` is called for every cell that is not a match (and for the first
// keepMatches that are). False, with `error`, only if the workbook can't be read at all.
bool recalcFull(const Workbook &book, const FullOptions &options, const std::function<void(const RecalcCell &)> &each, FullReport *report, QString *error = nullptr);

} // namespace jp::office::sheet
