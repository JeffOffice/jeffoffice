#pragma once
// Editing a workbook that is open: what a person types into a cell, Delete, and Undo and Redo, on the window's thread; and what saving needs to know about it. The cells and the
// formulas are the worker's while it works (WorkbookLoader), so a change is made while the window holds the worker back for it (WorkbookLoader::hold), and it is the cell and
// its place in the cells that are changed there; finding the formulas that read the cell, and working them out, is the worker's, and the results come back in batches as ever.
//
// Every edit is a command on a QUndoStack that holds the cells as they were and as they are, whole (content, formula, format) and nothing else: no copy of the workbook, no values
// of the formulas that depend (they are worked out again), so the memory it takes is what the edits changed. A cell typed into is one step. The session also keeps what saving needs: the
// cells that changed, and the formulas whose result is not what the file has (known from the batches the worker sends).

#include "core/clipdata.h"
#include "core/loader.h"
#include "grid/valuestore.h"
#include "sheet/entry.h"
#include "sheet/xlsxsave.h"

#include <QLocale>
#include <QObject>
#include <QUndoStack>

#include <functional>
#include <map>
#include <memory>
#include <set>
#include <unordered_set>
#include <vector>

namespace jp::crunch {

// The most cells a Delete (or any one edit) changes: past it the edit is refused, with a message, so that it never holds the worker, or the window, for long.
constexpr qint64 kMaxCellsPerEdit = 100000;
// The steps Undo goes back (each a command of the stack).
constexpr int kUndoSteps = 1000;
// The most memory the steps hold, by an estimate of the bytes of the cells in them (as they were and as they are): past it the oldest steps are forgotten, the newest never.
constexpr qint64 kUndoBytes = 128 << 20;

struct CellChange {
    int sheet = 0;
    quint32 row = 0;
    quint16 col = 0;
    jp::office::sheet::Calculation::CellContent before, after;
    // What is to be settled in the workbook when the change is first made (it can't be before: the shared strings and the styles are changed with the worker held back): a text that
    // is to go in the shared strings, and a number format (a code) the cell's format is to be given.
    bool textToShare = false;
    QString formatCode;
    // Only the format of the cell changes (its value, formula, and the array it may be part of stay as they are).
    bool styleOnly = false;
};

// A table's column renamed by a step (its header cell is one of the step's cells): the table by its place in the workbook's tables, the column by its place in the table.
struct TableRename {
    int table = 0;
    int column = 0;
    QString before, after;
    std::vector<std::pair<QString, QString>> names;   // the defined names that named the column, as they were (made when the step is first done)
};

// The width of columns or the height of rows changed by a step: the runs of columns (the specs of rows) in a range as they were and as they are.
struct SizeEdit {
    int sheet = 0;
    bool columns = true;
    quint32 first = 0, last = 0;   // the columns or rows
    std::vector<jp::office::sheet::ColumnSpan> columnsBefore, columnsAfter;
    std::vector<jp::office::sheet::RowSpec> rowsBefore, rowsAfter;
};

// The merged ranges of a sheet changed by a step (a paste with merges, cells moved): as they were and as they are.
struct MergeEdit {
    int sheet = 0;
    QVector<jp::office::sheet::CellRange> before, after;
    qint64 countBefore = 0, countAfter = 0;
};

// Rows, columns, or cells inserted or deleted by a step: the edit, what the model needs to undo it (made when it is first done), and the marks the session keeps by cell that it moved.
struct StructChange {
    bool done = false;                                                   // the cells are as the edit leaves them
    jp::office::sheet::StructEdit edit;
    jp::office::sheet::Calculation::EditRecord record;
    std::vector<jp::office::sheet::TablePlace> tables;                   // the tables it moved, where they were
    std::vector<quint64> touchedGone, touchedAdded;                     // cells marked as changed that it deleted, and those it marked
    std::vector<std::pair<quint64, jp::calc::Value>> resultsGone, engineGone, fileGone;   // the results kept by cell that it deleted
    std::vector<quint64> columnsGone, rowsGone;                          // the columns and rows marked as resized that it deleted
    qint64 bytes() const;
    int savedGen = 0;                                                    // how many saves had been made when it was done (an Undo after a later save is made against the saved file)
};

// What one step of the undo stack changes: cells (as they were and as they are), and whatever goes with them.
struct Step {
    std::vector<CellChange> cells;
    std::vector<TableRename> renames;
    std::vector<SizeEdit> sizes;
    std::vector<MergeEdit> merges;
    std::shared_ptr<StructChange> structure;   // or rows, columns, or cells inserted or deleted
    bool empty() const { return cells.empty() && renames.empty() && sizes.empty() && merges.empty() && !structure; }
};

// About how many bytes a cell's content takes in a step: the structure, and the texts in it.
qint64 contentBytes(const jp::office::sheet::Calculation::CellContent &c);

class EditSession : public QObject {
    Q_OBJECT
public:
    EditSession(WorkbookLoader *loader, const std::shared_ptr<LoadedBook> &book, jp::grid::ValueStore *values, QObject *parent = nullptr);
    ~EditSession() override;

    // The locale a person types in, and the date a date without a year is in (the system's and today by default; a test sets them).
    void setLocale(const QLocale &locale) { m_locale = locale; }
    void setToday(const QDate &today) { m_today = today; }
    QLocale locale() const { return m_locale; }

    QUndoStack *undoStack() { return &m_undo; }
    // Whether there is a step to undo that is still held (the oldest steps may have been forgotten: see kUndoBytes).
    bool canUndo() const;
    // How many steps the last save made Undo forget (those up to a step that deleted rows, columns, or cells).
    int stepsForgottenBySave() const { return m_forgotBySave; }
    // The bytes the steps hold, and the most they may (a test sets it).
    qint64 undoBytes() const { return *m_undoBytes; }
    qint64 undoByteLimit() const { return m_undoByteLimit; }
    void setUndoByteLimit(qint64 bytes);

    // Whether the workbook can be changed now: it is read whole (the worker is ready), no sheet was left in part, and no save is going on; else why not.
    bool canEdit(QString *why = nullptr) const;
    // Whether a sheet has cells to change (a chart sheet has none).
    bool canEditSheet(int sheet, QString *why = nullptr) const;
    // Whether a cell can be: it is on a sheet of cells, and not part of an array or a data table.
    bool canEditCell(int sheet, quint32 row, quint32 col, QString *why = nullptr) const;

    // What a cell is edited as (see editTextOfValue); a formula with its "=".
    QString editText(int sheet, quint32 row, quint32 col) const;
    // The value a cell shows (what the grid shows: the calculation's result for a formula, else what it holds); empty for no cell.
    jp::calc::Value valueAt(int sheet, quint32 row, quint32 col) const;

    // What the formula bar shows for a cell: the text the grid gives for it (`gridText`), written as the person types in the locale (a formula with its separators, a number with its
    // decimal mark); the grid's text itself where the locale needs no change.
    QString barText(int sheet, quint32 row, quint32 col, const QString &gridText) const;
    // The number format code of a cell as its format gives it ("General" for none).
    QString formatCodeOf(int sheet, quint32 row, quint32 col) const;

    struct Outcome {
        bool ok = true;
        QString problem;      // a sentence for the person, when not ok
        int problemAt = -1;   // and the character of what was typed it is about, or -1
        jp::office::sheet::CellRange range;   // for a paste: the cells it covered
        // A rule of validation does not allow what was entered (`problem` is the rule's message, or one of ours): its style (Validation::Style: 0 stop, 1 warning, 2 information) and title. A
        // stop is refused; for the others the person is asked, and the same is done again with the rule left out (`checkRule` false). Only what is typed is held to the rules: a paste is not.
        bool rule = false;
        int ruleStyle = 0;
        // The step is bigger than Undo can hold: it is not done, and `problem` asks whether to go on (the same call after allowNextBigStep is made, and then neither it nor the steps before it
        // are undone).
        bool confirm = false;
        QString ruleTitle;
    };
    // The next step that moves cells is made though it is bigger than Undo holds (the person was asked, and said to go on).
    void allowNextBigStep() { m_bigStepAllowed = true; }
    // Types text into a cell, as the person typed it: a number, a percent, a date, TRUE, a formula, or text; the cell takes the format the entry asks for if it has none of its own. The
    // change is one step on the undo stack. Not ok, and nothing changed, if the text is a formula that is not one, or is too long, or the cell cannot be changed.
    Outcome type(int sheet, quint32 row, quint32 col, const QString &typed, bool checkRule = true);
    // Clears the contents of the cells in the range (their formats stay): one step. Not ok, and nothing changed, if a cell in it is part of an array, or there are more cells than
    // kMaxCellsPerEdit.
    Outcome clear(int sheet, qint64 row1, qint64 col1, qint64 row2, qint64 col2);

    // AutoSum and the functions of its menu (`function` is SUM, AVERAGE, COUNT, MAX, or MIN) for what is selected. With one cell: the numbers above it, or if there are none left of it,
    // are the range, and the formula is to be edited in the cell (`text`, with the cursor `cursorBack` characters before its end: inside the brackets of one with no range). With a
    // block of cells: a formula for each column under it, or for each row to the right of one row, in the empty cells at its end if its last row (column) is empty; one step.
    struct AutoSumResult {
        enum class Kind { Nothing, Edit, Done } kind = Kind::Nothing;
        quint32 row = 0, col = 0;
        QString text;
        int cursorBack = 0;
        Outcome outcome;
    };
    AutoSumResult autoSum(int sheet, const jp::office::sheet::CellRange &selection, const QString &function);

    // Fill: the cells past a block, in a direction, given what the block holds (the fill handle and Fill Series), or a copy of it (Ctrl+D, Ctrl+R). Each line of the block (a column
    // for down and up, a row for right and left) is taken by itself: numbers go on by their step (a trend through them when there are several), dates by days, weekdays, months, or
    // years, the names of weekdays and months (the locale's or English) in turn, and a text that ends in a number counts up; anything else is copied over and over. A formula is
    // copied with its references moved; every cell takes the format of the cell it follows. With `Auto` a single number is copied, and with `Linear` it counts up.
    enum class FillDirection { Down, Right, Up, Left };
    enum class FillKind { Auto, Copy, Linear, Growth, Date };
    enum class DateUnit { Day, Weekday, Month, Year };
    struct FillOptions {
        FillKind kind = FillKind::Auto;
        DateUnit unit = DateUnit::Day;
        bool hasStep = false;
        double step = 1;
        bool hasStop = false;
        double stop = 0;
    };
    // Fills `count` cells past `source` in the direction (one step); nothing changed and the problem told if a cell there can't be changed.
    Outcome fill(int sheet, const jp::office::sheet::CellRange &source, FillDirection direction, quint32 count, const FillOptions &options);
    Outcome fill(int sheet, const jp::office::sheet::CellRange &source, FillDirection direction, quint32 count);   // (as the fill handle: Auto)



    // Inserts or deletes rows, columns, or cells: the cells after them move over (down, right, left, or up) and everything that refers to them follows (formulas on every sheet, defined
    // names, merged cells, tables, and, in the file, conditional formats, validations, hyperlinks, charts, print areas, and the rest); a reference to cells that are deleted becomes #REF!.
    // One step. Not ok, and nothing changed, if it would cut through an array formula or a table, push cells off the sheet, or the sheet is protected. Rows, columns, and cells are counted
    // from 0; `block` is the cells that are made room for or closed up.
    Outcome insertRows(int sheet, quint32 first, quint32 count);
    Outcome deleteRows(int sheet, quint32 first, quint32 count);
    Outcome insertColumns(int sheet, quint32 first, quint32 count);
    Outcome deleteColumns(int sheet, quint32 first, quint32 count);
    Outcome insertCells(int sheet, const jp::office::sheet::CellRange &block, bool shiftDown);
    Outcome deleteCells(int sheet, const jp::office::sheet::CellRange &block, bool shiftUp);
    // Whether the edit can be made now, and if not, why (a sentence).
    bool canMove(const jp::office::sheet::StructEdit &edit, QString *why = nullptr) const;
    // The generation of the cells' places: how many times cells have been moved by an edit (Undo and Redo too). A batch of results made at an earlier generation names the cells by where they
    // were; `moveToNow` puts the places in it where the cells are now.
    int generation() const { return int(m_log.size()); }
    void moveToNow(std::vector<jp::grid::ValueStore::Item> *items, int generation) const;

    // ---- the clipboard
    // What a paste puts: everything the cells have (All), their formulas or values, their values (a formula's result), or their formats; an operation made with the cells already there (the
    // numbers of the copy added to them, ...); the cells that are empty in the copy left alone; the block turned over (rows become columns).
    struct PasteOptions {
        enum class What { All, Formulas, Values, Formats } what = What::All;
        enum class Operation { None, Add, Subtract, Multiply, Divide } operation = Operation::None;
        bool skipBlanks = false;
        bool transpose = false;
    };
    // This workbook's name for the clipboard: a copy made here carries it.
    qint64 token() const { return m_token; }
    // The cells of a range as the clipboard holds them (the block less the rows and columns past the last the sheet has cells in; invalid, with no cells, past kMaxCellsPerEdit). Reads the
    // sheet, holds nothing.
    ClipData copyCells(int sheet, const jp::office::sheet::CellRange &range, bool cut) const;
    // Pastes at the first cell of `target` (or, if `target` is a whole number of times the block, over all of it): one step. Not ok, and nothing changed, if a cell it would change is part of
    // an array or a table's header or locked, or there are too many.
    Outcome paste(int sheet, const jp::office::sheet::CellRange &target, const ClipData &clip, const PasteOptions &options);
    // The same for texts in a table (from another program), entered as if typed.
    Outcome pasteText(int sheet, const jp::office::sheet::CellRange &target, const QVector<QStringList> &table, const PasteOptions &options);
    // Cut and paste: the cells of the block go to the other place over what is there, and what refers to them follows. One step.
    Outcome moveCells(int sheet, const jp::office::sheet::CellRange &source, int toSheet, quint32 toRow, quint32 toCol);

    // The width of columns and the height of rows: one step. A `SizeChange` sets the size (the width in characters of the widest digit, with the padding; the height in points), hides or
    // shows, or both; `custom` says that a person set the size (a fit to the contents is not custom, and a column that is not custom is widened to fit a number typed in it).
    struct SizeChange {
        bool setSize = false;
        double size = 0;
        bool custom = true;
        bool setHidden = false;
        bool hidden = false;
    };
    Outcome resizeColumns(int sheet, quint32 first, quint32 last, const SizeChange &change, const QString &text);
    Outcome resizeRows(int sheet, quint32 first, quint32 last, const SizeChange &change, const QString &text);
    // Columns each given its width in one step (a fit to the contents of several): the column and the width.
    Outcome fitColumns(int sheet, const std::vector<std::pair<quint32, double>> &widths);
    Outcome fitRows(int sheet, const std::vector<std::pair<quint32, double>> &heights);
    // A number typed in a column that has the width it began with widens it to hold it: the width is added to the step of the entry (the last on the stack, which must be an entry in
    // that column); false if it was not.
    bool widenColumnWithEntry(int sheet, quint32 col, double width);
    // The most rows one resize changes.
    static constexpr qint64 kMaxRowsPerResize = 100000;

    // ---- formatting (one step each; undone, redone, and saved as styles of the file with no format made twice)
    // The cells of a range given a change of their formats: the font, the fill, the borders, the number format, the alignment, or protection (see FormatChange). Whole columns (or whole rows)
    // are given the format as the columns' (rows') own, and the cells in them that have formats of their own are changed too; any other block must be no more than kMaxCellsPerEdit cells.
    // Not ok, and nothing changed, if a cell in it is locked on a protected sheet. `text` is the name of the step in the Edit menu.
    Outcome formatCells(int sheet, const jp::office::sheet::CellRange &range, const jp::office::sheet::FormatChange &change, const QString &text);
    // The borders of a block: which of its edges (the sides on the outside of the block, the lines between its cells, a diagonal) are given the line; a line of no style takes them away. The
    // cells beside the block that share its outer edges are given the same line on the side that meets it, as in the programs people know.
    enum BorderEdge { EdgeLeft = 1, EdgeRight = 2, EdgeTop = 4, EdgeBottom = 8, EdgeInsideHorizontal = 16, EdgeInsideVertical = 32, EdgeDiagonalUp = 64, EdgeDiagonalDown = 128 };
    Outcome setBorders(int sheet, const jp::office::sheet::CellRange &range, int edges, const jp::office::sheet::BorderSide &line, const QString &text);
    // Clears the formats of the cells in the range (they show General in the first cell format, with no fill or border), the contents and the formats, or both of them.
    Outcome clearFormats(int sheet, const jp::office::sheet::CellRange &range);
    Outcome clearAll(int sheet, const jp::office::sheet::CellRange &range);
    // Merges the cells of the range into one (the cell at the top left keeps its value; the others are emptied, which is asked about first when they hold anything: `confirm` in the outcome,
    // and the same call again with `discard`), merges each row of the range alone (Across), or takes the merges of the range away. Merge and center also centers the cell.
    enum class MergeKind { Merge, MergeAndCenter, Across, Unmerge };
    Outcome mergeCells(int sheet, const jp::office::sheet::CellRange &range, MergeKind kind, bool discard = false);
    // Gives the cells a named style (its place among the style formats: see Workbook::styleNamed), which sets their fonts, fills, borders, number format, alignment, and protection.
    Outcome applyStyle(int sheet, const jp::office::sheet::CellRange &range, quint32 style, const QString &text);

    // The worker's results, as they reach the window (before the window's store has them: the store's value is the engine's before it): kept for saving, those that the engine changed and that
    // differ from what the file holds.
    void noteResults(const std::vector<jp::grid::ValueStore::Item> &items);
    // The worker has told every result there is to tell (the calculation is finished, with no change after it unworked).
    void calculationFinished();

    // ---- saving
    // Whether there is anything the file lacks: the undo stack is not at the state that was last saved.
    bool isModified() const { return !m_undo.isClean(); }
    // What the file needs to be made, taken now (nothing is held on to): false, with a message, if it can't be made.
    struct Snapshot {
        jp::office::sheet::SaveChanges changes;
        int strings = 0, numberFormats = 0, cellFormats = 0, fonts = 0, fills = 0, borders = 0, styles = 0;   // how many of the workbook's additions are in it
        std::map<quint64, jp::calc::Value> results;            // the results it holds (to be the file's, once it is written)
        std::vector<quint64> touchedFormulas;                  // the formula cells it holds whole, with the results they were given
        std::map<quint64, jp::calc::Value> touchedResults;
    };
    bool takeSnapshot(Snapshot *out, QString *why);
    // The file was written from this snapshot: it is the base now. `package` is the new file's.
    void saved(const Snapshot &snapshot, const jp::office::Package &package);

    // How many cells have been changed since the file was read or saved, and how many formulas' results differ from the file's (for tests and the status).
    qint64 changedCells() const { return qint64(m_touched.size()); }
    qint64 changedResults() const { return qint64(m_results.size()); }
    // The longest the window waited for the worker to let go (microseconds, by the wall), and the last wait: what an edit costs besides the work of putting the cell in.
    qint64 longestHoldWaitMicros() const { return m_longestHoldWait; }
    qint64 lastHoldWaitMicros() const { return m_lastHoldWait; }
    // For tests: called with the calculation while the worker is held, before the changes of an edit are put in and after (so that a test can count what the edit did to it).
    void setHoldHook(const std::function<void(jp::office::sheet::Calculation *calc, bool after)> &hook) { m_holdHook = hook; }
    // Edits are held off (a save is going on).
    void setBusy(bool busy) { m_busy = busy; }
    bool isBusy() const { return m_busy; }

    static quint64 keyOf(int sheet, quint32 row, quint32 col) { return (quint64(sheet) << 40) | (quint64(row) << 16) | quint64(col); }

Q_SIGNALS:
    // Cells were changed by an edit, Undo, or Redo (the window draws them again).
    void cellsChanged(int sheet, const std::vector<jp::grid::ValueStore::Item> &cells);
    // Rows, columns, or cells were inserted or deleted (or put back by Undo: `edit` is what was done to the cells then); the window makes its picture of the sheet again.
    void cellsMoved(const jp::office::sheet::StructEdit &edit);
    // The width of columns or the height of rows of a sheet changed (by a step, Undo, or Redo): the window makes its picture of the sheet again.
    void sizesChanged(int sheet);
    // A step of the undo stack could not be undone or done again (the workbook could not be held, or the edit was refused): Undo and Redo of every step are given up, and this says so.
    void stepFailed(const QString &what);

private:
    friend class ChangeCells;
    // A step could not be undone or done again: every step is forgotten (the workbook is not as the stack says), and `stepFailed` says so.
    void giveUpUndo(const QString &stepText, bool redoing);
    // Forgets every step that is done (and what each holds): Undo does not go back over them.
    void forgetStepsDone();
    // Makes the changes (forward: to `after`; else back to `before`) with the worker held back. False if the worker could not be held.
    bool apply(Step *step, bool forward);
    bool applyStructure(Step *step, bool forward);
    void applySizes(Step *step, bool forward, jp::office::sheet::Calculation *calc);
    bool widenLastStep(int index, int sheet, quint32 col, double width);
    Outcome resize(const SizeEdit &edit, const QString &text);
    // The format a cell that holds nothing has: its row's when the row has one, else its column's, else the first.
    quint32 implicitStyle(int sheet, quint32 row, quint32 col) const;
    // The formats of cells changed: each target is a cell given a change (or its formats cleared); the formats are made with the worker held back, then the step is pushed. Whole columns
    // and rows of the range get their own formats.
    struct RestyleTarget {
        quint32 row = 0, col = 0;
        std::shared_ptr<const jp::office::sheet::FormatChange> change;   // the change, or none to clear the formats
    };
    using SizeBuilder = std::function<void(jp::office::sheet::Workbook &, const jp::office::sheet::Calculation &, Step *)>;
    Outcome restyle(int sheet, const std::vector<RestyleTarget> &targets, const QString &text, const SizeBuilder &sizes = nullptr);
    Outcome restyleRange(int sheet, const jp::office::sheet::CellRange &range, const std::shared_ptr<const jp::office::sheet::FormatChange> &change, const QString &text);
    Outcome structure(const jp::office::sheet::StructEdit &edit, const QString &text);
    // The changes a paste makes: the block (`blockRows` by `blockCols`) is put at the first cell of `target`, or over all of it when it is a whole number of times the block. `make` is
    // asked for the change at each place of the block, and says false, with the problem, if the paste can't be made. `merges` are those of the block (from its first cell), put with it.
    struct PasteSpot {
        quint32 row, col;      // the cell the change is for
        quint32 dRow, dCol;    // and its place in the block
    };
    using PasteMaker = std::function<bool(const PasteSpot &spot, CellChange *change, Outcome *problem)>;
    Outcome pasteChanges(int sheet, const jp::office::sheet::CellRange &target, quint32 blockRows, quint32 blockCols, const PasteMaker &make, const std::vector<jp::office::sheet::CellRange> &merges,
                         bool withMerges, const QString &text);
    // Whether the rule of validation for a cell allows a value (not ok, with the rule's message, if not).
    Outcome checkRule(int sheet, quint32 row, quint32 col, const jp::calc::Value &value, bool isFormula, const QString &typed) const;
    // The marks kept by cell follow an edit (forward) or its undoing: `edit` is what was done to the cells.
    void followEdit(const jp::office::sheet::StructEdit &edit, StructChange *change, bool forward);
    // After a save: forgets the steps up to the last one that deleted rows, columns, or cells (the saved file has lost what was in them, and Undo could not give it back to the file). Every
    // other step stays: Undo of it is made against the saved file (the edit that undoes it goes in the list for the file).
    void forgetDeletesInFile();
    // Puts a step on the undo stack (done, as pushed) and keeps the stack within its bytes.
    void pushStep(Step step, const QString &text);
    // A text typed in a cell of a table's header row: the column is renamed (the formulas that name it follow, and the table's part when the workbook is saved).
    Outcome typeInHeader(int sheet, quint32 row, quint32 col, int table, int column, const QString &typed);
    jp::office::sheet::EntryOptions entryOptions(int sheet, bool textFormat) const;
    // The change typing `typed` into a cell makes (not a header cell's, not checked against the cell's rule): false, with the problem, if it can't be put in; `none` if it changes nothing.
    bool typedChange(int sheet, quint32 row, quint32 col, const QString &typed, CellChange *change, bool *none, Outcome *problem);
    // Whether a cell of that format on the sheet can't be changed: the sheet is protected and the cell is locked.
    bool isLocked(int sheet, quint32 style) const;
    QString lockedMessage() const;
    bool storedEquals(int sheet, quint32 row, quint32 col, const jp::calc::Value &value) const;
    void cleanChanged(bool clean);
    // The steps past the byte limit, oldest first, are forgotten (never the newest that is done).
    void trimUndo();
    int firstHeldStep() const;

    WorkbookLoader *m_loader;
    std::shared_ptr<LoadedBook> m_book;
    jp::grid::ValueStore *m_values;
    QUndoStack m_undo;
    std::shared_ptr<qint64> m_undoBytes = std::make_shared<qint64>(0);   // (shared with the steps, which count themselves off as they go, whenever that is)
    qint64 m_undoByteLimit = kUndoBytes;
    QLocale m_locale;
    QDate m_today;
    bool m_busy = false;
    std::function<void(jp::office::sheet::Calculation *, bool)> m_holdHook;
    qint64 m_longestHoldWait = 0, m_lastHoldWait = 0;
    bool m_tracking = false;               // an edit has been made: results are compared with the file's
    std::unordered_set<quint64> m_touched;           // cells changed by the person (written whole), by sheet, row, column
    std::map<quint64, jp::calc::Value> m_results;      // formulas whose value is not the file's
    std::map<quint64, jp::calc::Value> m_engineAtStart;  // for a formula whose result has changed: the engine's value before its first change since the file was read or saved
    qint64 m_token;
    std::set<int> m_mergesChanged;   // the sheets whose merged ranges a step changed since the file was read or saved
    std::set<quint64> m_columnsResized, m_rowsResized;   // the columns and rows (by sheet and place) whose size a step changed since the file was read or saved
    std::vector<jp::office::sheet::StructEdit> m_log;  // every edit that moved cells (the place of a batch's results in time: see generation)
    std::vector<jp::office::sheet::StructEdit> m_edits;   // those since the file was read or saved that are done (not undone), for the file
    std::vector<jp::office::sheet::ColumnRename> m_renames;   // and the table columns renamed, for the names, validations, rules, and charts that name them
    bool m_bigStepAllowed = false;     // see allowNextBigStep
    int m_saveGen = 0;                 // how many times the workbook has been saved
    int m_forgotBySave = 0;            // the steps the last save made Undo forget (see forgetDeletesInFile)
    std::map<quint64, jp::calc::Value> m_fileResults;  // what the file holds for formulas whose value there is not the one the workbook was read with (after a save)
};

} // namespace jp::crunch
