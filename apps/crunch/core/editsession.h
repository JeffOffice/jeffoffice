#pragma once
// Editing a workbook that is open: what a person types into a cell, Delete, and Undo and Redo, on the window's thread; and what saving needs to know about it. The cells and the
// formulas are the worker's while it works (WorkbookLoader), so a change is made while the window holds the worker back for it (WorkbookLoader::hold), and it is the cell and
// its place in the cells that are changed there; finding the formulas that read the cell, and working them out, is the worker's, and the results come back in batches as ever.
//
// Every edit is a command on a QUndoStack that holds the cells as they were and as they are, whole (content, formula, format) and nothing else: no copy of the workbook, no values
// of the formulas that depend (they are worked out again), so the memory it takes is what the edits changed. A cell typed into is one step. The session also keeps what saving needs: the
// cells that changed, and the formulas whose result is not what the file has (known from the batches the worker sends).

#include "core/clipdata.h"
#include "core/pagesettings.h"
#include "core/loader.h"
#include "core/nametable.h"
#include "core/sortspec.h"
#include "core/objectstate.h"
#include "grid/valuestore.h"
#include "sheet/entry.h"
#include "calc/numfmt.h"
#include "sheet/textmatch.h"
#include "sheet/xlsxsave.h"

#include <QLocale>
#include <QObject>
#include <QUndoStack>

#include <atomic>
#include <functional>
#include <map>
#include <optional>
#include <memory>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace jp::crunch {

// The most cells a Delete (or any one edit) changes: past it the edit is refused, with a message, so that it never holds the worker, or the window, for long.
constexpr qint64 kMaxCellsPerEdit = 100000;
// The most cells a formatting command changes the format of at once (a whole column of a sheet that is full of cells, or a block), a step of them being 20 bytes each.
constexpr qint64 kMaxStyledCells = 2000000;
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

// A table made, taken out, or changed by a step: its place in the workbook's tables, and the table as it was and as it is. A table taken out or renamed has the formulas that named it written
// again (the cells' are in the step; the defined names' are made when the step is first done).
struct TableEdit {
    enum class Kind : quint8 { Replace, Insert, Erase } kind = Kind::Replace;
    enum class Rewrite : quint8 { None, Rename, ToRange } rewrite = Rewrite::None;
    int index = 0;
    jp::office::sheet::Table before, after;
    std::vector<std::pair<QString, QString>> names;   // the defined names that named the table, as they were (made when the step is first done)
};

// The width of columns or the height of rows changed by a step: the runs of columns (the specs of rows) in a range as they were and as they are.
struct SizeEdit {
    int sheet = 0;
    bool columns = true;
    quint32 first = 0, last = 0;   // the columns or rows
    std::vector<jp::office::sheet::ColumnSpan> columnsBefore, columnsAfter;
    std::vector<jp::office::sheet::RowSpec> rowsBefore, rowsAfter;
};

// A sheet added, copied, renamed, deleted, moved, shown or hidden, or given a tab color by a step. The sheets are never renumbered (see Calculation::addSheet): `sheet` is the sheet's place for
// good, and what a step changes of the order of the tabs is kept whole.
struct SheetEdit {
    enum class Kind : quint8 { Insert, Copy, Delete, Rename, Move, State, TabColor } kind = Kind::Insert;
    int sheet = 0;
    QString nameBefore, nameAfter;
    QVector<int> orderBefore, orderAfter;
    jp::office::sheet::SheetState stateBefore = jp::office::sheet::SheetState::Visible, stateAfter = jp::office::sheet::SheetState::Visible;
    jp::office::sheet::ColorSpec colorBefore, colorAfter;
    int copyOf = -1;                                       // Copy: the sheet it is a copy of
    jp::office::sheet::Calculation::SheetRewrite rewrite;  // Rename and Delete: what the formulas were (made when the step is done)
    // Whether the step changes what the file's parts name (and so can't be undone against a file that was saved after it).
    bool structural() const { return kind == Kind::Insert || kind == Kind::Copy || kind == Kind::Delete || kind == Kind::Rename || kind == Kind::Move; }
};

// A defined name made, changed, or taken away by a step: its place in the name table, the entry and what the name stands for (the text of its formula) as they were and as they are. A
// name given another name has the formulas that used it as they were (made when the step is first done).
struct NameEdit {
    int id = 0;
    NameEntry before, after;
    QString textBefore, textAfter;
    jp::office::sheet::Calculation::SheetRewrite rewrite;
    bool renamed() const { return before.alive && after.alive && before.name.compare(after.name, Qt::CaseInsensitive) != 0; }
};

// How the workbook calculates: whether by itself, when it is told, or by itself except for tables; and whether a formula that reads itself is worked out by going round it, how many
// rounds it goes at the most, and the change that is small enough to stop at.
struct CalcSettings {
    jp::office::sheet::CalcMode mode = jp::office::sheet::CalcMode::Auto;
    bool iterate = false;
    int count = 100;
    double delta = 0.001;
    bool operator==(const CalcSettings &o) const { return mode == o.mode && iterate == o.iterate && count == o.count && delta == o.delta; }
    bool operator!=(const CalcSettings &o) const { return !(*this == o); }
};
struct CalcEdit {
    CalcSettings before, after;
};

// The default sizes of a sheet (the height of rows and the width of columns that have none of their own) changed by a step: as they were and as they are.
struct FormatEdit {
    int sheet = 0;
    jp::office::sheet::SheetFormat before, after;
};

// The merged ranges of a sheet changed by a step (a paste with merges, cells moved): as they were and as they are.
struct MergeEdit {
    int sheet = 0;
    QVector<jp::office::sheet::CellRange> before, after;
    qint64 countBefore = 0, countAfter = 0;
};

// The filter of a sheet changed by a step (its conditions, or turned on or off): as it was and as it is. The rows it shows and hides are in the step's `sizes`.
struct FilterEdit {
    int sheet = 0;
    jp::office::sheet::AutoFilter before, after;
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

// A cell whose cell format a step changed, and nothing else of it (a formatting command's: it is what a format given to a million cells holds, so it is small): where it is, and the
// formats it had and has.
// The specs of the rows of a sheet (their heights, which are hidden, and why) as they were and as they are, whole: what a filter that shows and hides very many rows changes, put in and taken out
// at once. The rows from `first` to `last` are those it looked at (what is saved of them).
struct RowSpecsEdit {
    int sheet = 0;
    QVector<jp::office::sheet::RowSpec> before, after;
    quint32 first = 0, last = 0;
};

struct StyleCell {
    qint32 sheet = 0;
    quint32 row = 0, col = 0;
    quint32 before = 0, after = 0;
};

// What one step of the undo stack changes: cells (as they were and as they are), and whatever goes with them.
struct Step {
    std::vector<CellChange> cells;
    std::vector<StyleCell> styles;
    std::vector<TableRename> renames;
    std::vector<TableEdit> tables;
    std::vector<SizeEdit> sizes;
    std::vector<MergeEdit> merges;
    std::vector<FormatEdit> formats;
    std::vector<FilterEdit> filters;
    std::vector<RowSpecsEdit> rowSpecs;
    std::vector<std::shared_ptr<jp::office::sheet::CellPermute>> permutes;   // cells put in another order (what is tied to a cell goes with it when the file is written)
    std::vector<SheetEdit> sheets;
    std::shared_ptr<StructChange> structure;   // or rows, columns, or cells inserted or deleted
    std::vector<ObjectEdit> objects;           // or objects over the cells taken away (done: they are gone; undone: they are back)
    std::vector<PageEdit> pages;               // or how sheets print (the paper, margins, header and footer, breaks, print area, titles)
    std::vector<NameEdit> names;               // or defined names made, changed, or taken away
    std::vector<CalcEdit> calcs;               // or how the workbook calculates
    bool empty() const { return cells.empty() && styles.empty() && renames.empty() && tables.empty() && sizes.empty() && merges.empty() && formats.empty() && filters.empty() && rowSpecs.empty() && permutes.empty() && sheets.empty() && !structure && objects.empty() && pages.empty() && names.empty() && calcs.empty(); }
};

// About how many bytes a cell's content takes in a step: the structure, and the texts in it.
qint64 contentBytes(const jp::office::sheet::Calculation::CellContent &c);

class EditSession : public QObject {
    Q_OBJECT
public:
    EditSession(WorkbookLoader *loader, const std::shared_ptr<LoadedBook> &book, jp::grid::ValueStore *values, QObject *parent = nullptr);
    ~EditSession() override;

    // The locale a person types in, and the date a date without a year is in (the system's and today by default; a test sets them).
    void setLocale(const QLocale &locale)
    {
        m_locale = locale;
        m_symbolsReady = false;
    }
    void setToday(const QDate &today) { m_today = today; }
    QLocale locale() const { return m_locale; }

    QUndoStack *undoStack() { return &m_undo; }
    // The workbook and its calculation, to be read (on the window's thread, and not while an edit is being made).
    const jp::office::sheet::Calculation *calculation() const { return m_book ? m_book->calc.get() : nullptr; }
    const jp::office::sheet::Workbook *workbook() const { return m_book ? m_book->book.get() : nullptr; }
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
        // A step made a slice at a time is not over: call again (ok and `problem` empty so far).
        bool pending = false;
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



    // ---- tables (edittables.cpp). A table is named by its place among the workbook's tables (Workbook::tables); each of these is one step of Undo, and nothing is changed when it is not ok.
    // The table whose cells include the cell, or -1; and the one whose header row, data, or totals row it is in is the same table.
    int tableAt(int sheet, quint32 row, quint32 col) const;
    // Insert > Table: the cells of `range` become a table. With `hasHeaders` the first row holds the names of the columns (a cell that is not text becomes the text it shows, an empty one is
    // named, and two of one name are numbered); without, the table has no header row and its columns are named Column1, Column2, and so on. A table needs a row of data: a single row
    // with headers is given the empty row under it. `made` is the table's place.
    Outcome makeTable(int sheet, const jp::office::sheet::CellRange &range, bool hasHeaders, const QString &style, int *made = nullptr);
    // Why a name can't be a table's (not `except`'s own), in a sentence; empty when it can: not a valid name, or another table's or a defined name, in any case.
    QString tableNameProblem(const QString &name, int except) const;
    // Turns the table back into plain cells: the cells keep what they hold and how they look (the style is drawn no more), and the formulas that named the table's columns name the cells.
    Outcome tableToRange(int table);
    Outcome renameTable(int table, const QString &name);
    // The marks of a table's look and the filter buttons.
    struct TableMarks {
        QString style;
        bool firstColumn = false, lastColumn = false, rowStripes = true, columnStripes = false, filterButtons = true;
    };
    Outcome setTableMarks(int table, const TableMarks &marks);
    // The row of column names on or off. Off: the names' cells are cleared and the table begins at the row under them. On: the row above the table (which must be empty) is the table's, with
    // the columns' names in it.
    Outcome setTableHeaderRow(int table, bool on);
    // The row of totals on or off. On: the row under the table (empty) holds a total for the last column and the word "Total" in the first; off: that row is cleared.
    Outcome setTableTotalsRow(int table, bool on);
    // What a column's total shows: one of the file's words (sum, average, count, countNums, max, min, stdDev, var), "custom" with its formula (no "="), or empty for none; the cell is written.
    Outcome setTableTotal(int table, int column, const QString &function, const QString &custom = QString());
    // A table's cells changed to `range` (it keeps its first cell and header row; it may grow or shrink down and right): the new rows get the calculated columns' formulas, new columns are
    // named, the totals row moves with the last row.
    Outcome resizeTable(int table, const jp::office::sheet::CellRange &range);
    // What a table's cells would be if a cell was typed into (sheet, row, col): the table it would grow, and how, or -1: typing in the row under a table, or the column beside it, adds it.
    struct TableGrowth {
        int table = -1;
        bool row = false, column = false;
    };
    TableGrowth growthByTyping(int sheet, quint32 row, quint32 col) const;
    // Whether a formula typed in the table's cell would make its column a calculated column (the column has no formula, and the cells of its other rows are empty), and the choice not to: the
    // last calculated column made by typing is undone (the cell keeps its formula).
    bool lastEntryMadeCalculatedColumn() const { return m_lastCalculated; }
    Outcome undoCalculatedColumn();
    // The columns' calculated formulas of a table, for a row: the formula (as the cell holds it, with no "=") of the column for that row, or empty for none.
    QString calculatedFormulaFor(const jp::office::sheet::Table &table, int column, quint32 row) const;
    // The table names for a list to offer, and a table's column names; at most `limit`.
    QStringList tableNames(int limit = 200) const;

    // ---- steps put in a slice at a time
    // A step of very many cells is put in a slice at a time (no turn of the window does more than a few thousand cells) and then lies on the undo stack as one step. While it is being put in, the
    // workbook is not changed by anything else (canEdit says so), and it cannot be saved.
    struct SlicedPush {
        Step step;
        QString text;
        size_t next = 0;
        bool began = false;
    };
    std::shared_ptr<SlicedPush> beginSlicedPush(Step step, const QString &text);
    // Puts in about `budget` more cells (0: all); true when the step is all in and is on the undo stack.
    bool continueSlicedPush(SlicedPush &push, qint64 budget);
    bool isSlicing() const { return m_slicing; }
    // What the slices did (a test counts them): how many cells the biggest slice put in, and how many slices there have been.
    qint64 longestSlice() const { return m_longestSlice; }
    int slicesPushed() const { return m_slicesPushed; }
    // How many number formats have been made for the cells' formats (one for each format, not one for each cell that is shown through it).
    qint64 formatsMade() const { return m_formatsMade; }

    // ---- notes
    // The notes of a sheet as the cells hold them now: the file's, moved with the cells by the edits made since (a note on a cell that was deleted is gone). Made once and kept until a change.
    std::shared_ptr<const std::map<quint64, QString>> notesOf(int sheet, bool *cannot = nullptr, QString *why = nullptr) const;

    // ---- Find and Replace (editfind.cpp)
    // What a cell shows, as text: its value through its number format (the person's marks for the decimal and the digits), a text as it is, TRUE and FALSE, an error's name; empty for no cell.
    QString displayText(int sheet, quint32 row, quint32 col) const;
    // The same as a file writes it, with no language in it ("1,234.50", "1/15/2024"): what the lists of a filter are kept in, so that a file filtered in one language is filtered in another.
    QString neutralText(int sheet, quint32 row, quint32 col) const;
    QString shownText(int sheet, quint32 row, quint32 col, bool neutral) const;
    // The number format of a cell format, made once for each (not for each cell that has it).
    const jp::calc::NumberFormat &formatOfStyle(quint32 style) const;
    // The color as the number it comes to (0xRRGGBB), or -1 for none: a theme color and an RGB that look alike are the same.
    qint64 colorKey(const jp::office::sheet::ColorSpec &color) const;
    struct CellSpot {
        int sheet = 0;
        quint32 row = 0, col = 0;
    };
    struct ReplaceResult {
        int cells = 0;          // cells that changed
        int occurrences = 0;    // and the places in them
    };
    // Puts `with` in place of what matches the pattern in the text each of these cells holds (a formula with its "=", a constant as it is typed), and enters the new text as if typed: one
    // step. Nothing is changed, and the reason is told, if a cell is part of an array, is locked on a protected sheet, is in a table's header row, would not be a formula any more, or
    // there are more than kMaxCellsPerEdit. (A large one is made a slice at a time: planReplace, prepareReplace, and applyReplace; replaceInCells is all of them at once.)
    Outcome replaceInCells(const std::vector<CellSpot> &cells, const jp::office::sheet::TextPattern &pattern, const QString &with, ReplaceResult *result = nullptr);
    struct ReplacePlan;
    std::shared_ptr<ReplacePlan> planReplace(const std::vector<CellSpot> &cells, const jp::office::sheet::TextPattern &pattern, const QString &with, QString *why);
    // Works out the new text of about `budget` more cells (0: all); true when done (or when a cell cannot be changed: applyReplace says why).
    bool prepareReplace(ReplacePlan &plan, qint64 budget);
    Outcome applyReplace(ReplacePlan &plan, ReplaceResult *result = nullptr, qint64 budget = 0);
    // The text a search looks in for a cell: its formula with the "=", else what it holds (a text as it is; a number, true or false, or an error as it is typed). Faster than editText, which makes
    // a text that could be read as something else safe to type.
    QString searchText(int sheet, quint32 row, quint32 col) const;

    // ---- Sort (editsort.cpp)
    // A sort being made: what it reads of the cells (the keys, read a slice at a time by `prepareSort`), the order it comes to (`orderSort`, which reads only the keys and so may be done on another
    // thread while the window goes on), and the one step it is (`applySort`).
    struct SortPlan;
    // The plan for sorting the range; null, with the reason in `why`, if it cannot be: a cell in it is locked or part of an array, merged cells of different sizes (or that span the rows
    // sorted), a header row of a table, more cells than kMaxCellsPerEdit, no level, or a level that is not in the range.
    std::shared_ptr<SortPlan> planSort(int sheet, const jp::office::sheet::CellRange &range, const SortSpec &spec, QString *why);
    // Reads the keys of about `budget` more of the lines to sort (0: all of them); true when every key is read.
    bool prepareSort(SortPlan &plan, qint64 budget);
    // Works out the order from the keys; false if `cancel` was set (nothing then comes of the plan). Reads nothing of the workbook.
    static bool orderSort(SortPlan &plan, const std::atomic<bool> *cancel = nullptr);
    // Puts the cells in the order: one step. Not ok, and nothing changed, if the plan is not ready or the workbook has changed since it was made.
    // With a budget the step is made and put in a slice at a time (`pending` is set while there is more).
    Outcome applySort(SortPlan &plan, qint64 budget = 0);
    // All of it at once (for the tools and the tests): a plan, its keys, its order, and the step.
    Outcome sortRange(int sheet, const jp::office::sheet::CellRange &range, const SortSpec &spec);
    // Whether the first row of the range looks like names of columns: all text, with something that is not text in one of the cells under it.
    bool looksLikeHeader(int sheet, const jp::office::sheet::CellRange &range) const;
    // The distinct colors of the cells (cell colors, or font colors) of a column of the range, in the order they first appear, at most `limit`; with `none` set if some cell has no color.
    std::vector<jp::office::sheet::ColorSpec> colorsIn(int sheet, const jp::office::sheet::CellRange &range, bool font, bool *none, int limit = 64) const;

    // ---- Filter (editfilter.cpp)
    // A filter being put on a sheet: the conditions it will have, the rows they hide (worked out a slice at a time by `prepareFilter`), and the one step it is (`applyFilter`). The rows hidden
    // by a filter are kept apart from the rows a person hid.
    struct FilterPlan;
    // The most a filter may be asked to do: its rows times its conditions (a filter with more says so, and is not made). A filter with no condition (Clear, Filter off) has none of it.
    static constexpr qint64 kMaxFilterWork = 40000000;
    // The plan for the filter `after` (its range, and the conditions of its columns); null, with the reason in `why`, if it cannot be: the work (rows times conditions) is past kMaxFilterWork, the range is in a table, a column's condition
    // is none that can be worked out, or the sheet cannot be changed. An `after` that is not valid takes the sheet's filter away (the rows it hid are shown).
    std::shared_ptr<FilterPlan> planFilter(int sheet, const jp::office::sheet::AutoFilter &after, QString *why);
    // Works out which rows the conditions hide, a slice (about `budget` cells; 0 for all) at a time; true when it is done.
    bool prepareFilter(FilterPlan &plan, qint64 budget);
    // Puts the filter in, and shows and hides the rows: one step. Not ok if the plan is not done or the cells have been changed since it was made.
    Outcome applyFilter(FilterPlan &plan, const QString &text);
    // How many things the plan does in all (rows and cells looked at): what a window decides by whether to work it out a slice at a time.
    static qint64 filterWork(const FilterPlan &plan);
    // All of it at once: the sheet's filter is `after`.
    Outcome setFilter(int sheet, const jp::office::sheet::AutoFilter &after, const QString &text);
    // The sheet's filter with the condition of one column (from the first column of its range) put in (a condition of kind None takes the column's away).
    jp::office::sheet::AutoFilter filterWith(int sheet, quint32 column, const jp::office::sheet::FilterColumn &condition) const;
    // The same with every column's condition taken away (the buttons stay).
    jp::office::sheet::AutoFilter filterCleared(int sheet) const;
    // Whether a row is hidden by the sheet's filter (and not by hand).
    bool rowFilteredOut(int sheet, quint32 row) const;
    // The color a color condition asks for, as a color: the one it names, or the one its format in the file has.
    jp::office::sheet::ColorSpec colorOfCondition(const jp::office::sheet::FilterColumn &condition) const;

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
    // Takes out of a batch of results those for a sheet that is not there any more (a sheet added and taken away by Undo while the worker was still working out its formulas).
    void dropRemovedSheets(std::vector<jp::grid::ValueStore::Item> *items) const;

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
    // The default width of the columns of a sheet and the default height of its rows (those that have no size of their own): one step. The width is as a SizeChange has it (characters
    // of the widest digit, with the padding), the height in points; one that is not given is left as it is.
    Outcome setDefaultSizes(int sheet, std::optional<double> columnWidth, std::optional<double> rowHeight, const QString &text);
    // ---- the sheets (one step each, undone, redone, and saved: the formulas that name a sheet follow it when it is renamed, and have #REF! for it when it is deleted)
    // The places of the sheets in the order of the tabs (those deleted are not among them, and hidden ones are).
    QVector<int> sheetOrder() const;
    // Whether a name can be a sheet's (not `except`'s own): 1 to 31 characters, none of : \ / ? * [ ], not beginning or ending with an apostrophe, not "History", and not another sheet's, in any
    // case; else why.
    bool validSheetName(const QString &name, int except, QString *why = nullptr) const;
    // The name a new sheet is given ("Sheet4"), and a copy of `sheet` ("Data (2)").
    QString newSheetName() const;
    QString copyName(int sheet) const;
    // A new sheet, or a copy of one, put before the sheet `before` in the order (-1: at the end); `made` is its place. `name` empty is the name given by newSheetName (copyName for a copy).
    // A copy has the cells, formats, sizes, merged ranges, and rules of the sheet, and what its file says of its protection, filter, links, page setup, page breaks, outline, and the look of
    // its data bars; a sheet with a table can't be copied. What is left out (pictures, charts, notes, and the like) is said by the outcome's `problem`, with `ok` true.
    Outcome insertSheet(int before, const QString &name, int *made = nullptr);
    Outcome copySheet(int sheet, int before, const QString &name, int *made = nullptr);
    // Deletes a sheet (the last that is shown, or one that has a table, can't be); the sheet and everything in it are kept for Undo.
    Outcome deleteSheet(int sheet);
    Outcome renameSheet(int sheet, const QString &name);
    // Puts a sheet before another in the order of the tabs (-1: at the end).
    Outcome moveSheet(int sheet, int before);
    Outcome setSheetHidden(int sheet, bool hidden);
    Outcome setTabColor(int sheet, const jp::office::sheet::ColorSpec &color);
    // Whether a sheet holds any cells (what a person is asked about before it is deleted).
    bool sheetHasCells(int sheet) const;
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
    // The same by the style's name: a style the workbook has, or a built-in one it is given now. Not ok if there is no such style, or the file has no place to keep one.
    Outcome applyStyleNamed(int sheet, const jp::office::sheet::CellRange &range, const QString &name, const QString &text);

    // An object over the cells is taken away (Delete with it picked), put there, moved, or changed: one step, which Undo gives back. The hook (setObjectHook) is told the change when the step is
    // done, undone, and done again (`forward`: the object is to have the state after). A step that took an object away is like one that deleted rows: Undo goes back as far as the last such
    // step saved, no further; a step that put one there cannot be done again once the object was taken away and the file saved.
    using ObjectHook = std::function<void(const ObjectEdit &edit, bool forward)>;
    void setObjectHook(ObjectHook hook) { m_objectHook = std::move(hook); }
    Outcome deleteObject(int sheet, int id, const QString &text, const ObjectState &before, const ObjectState &after);
    // The same for any change of objects (all in one step).
    Outcome changeObjects(std::vector<ObjectEdit> edits, const QString &text);

    // ---- how the sheets print (editpage.cpp)
    // How a sheet prints now; and a change to it, one step of Undo (nothing if it is as it is). The print area, titles, and breaks follow rows, columns, and cells inserted and deleted.
    const PageSettings &pages() const { return m_pages; }
    Outcome setPageSetup(int sheet, const jp::office::sheet::PageSetup &setup, const QString &text);
    // How a sheet is shown (0 the cells, 1 the page breaks, 2 the pages) and at what zoom in percent: kept for saving, not a step of Undo.
    void setSheetView(int sheet, int kind, int zoom) { m_pages.setView(sheet, kind, zoom); }
    bool viewsChanged() const { return m_pages.viewsChanged(); }

    // The edits that insert or delete rows, columns, or cells since the file was read or saved that are done (not undone), in order: what the objects over the cells are moved by.
    const std::vector<jp::office::sheet::StructEdit> &structureEdits() const { return m_edits; }
    // The renames and deletes of sheets since the file was read or saved that are done (not undone), in order: what the charts' references follow.
    const std::vector<jp::office::sheet::SheetOp> &sheetOperations() const { return m_sheetOps; }

    // The worker's results, as they reach the window (before the window's store has them: the store's value is the engine's before it): kept for saving, those that the engine changed and that
    // differ from what the file holds.
    void noteResults(const std::vector<jp::grid::ValueStore::Item> &items);
    // The worker has told every result there is to tell (the calculation is finished, with no change after it unworked).
    void calculationFinished();

    // ---- defined names (see core/nametable.h). Each of these is one step of Undo.
    struct NameDraft {
        QString name;
        int scope = -1;       // the place of the sheet it belongs to; -1 for the workbook
        QString text;         // what it stands for, as typed in the person's locale ("=" in front is allowed)
        QString comment;
        bool hidden = false;
    };
    const NameTable &names() const { return m_nameTable; }
    // Whether the name is one to show: alive, and not of a sheet that is deleted.
    bool nameListed(int id) const;
    // What the name stands for as the person types it ("=Sheet1!$A$1"), and the value it has (a block of cells is "{…}"); empty for none.
    QString nameTextOf(int id) const;
    QString nameValueOf(int id) const;
    // Why a name can't be made or changed to this, in a sentence (empty when it can); `id` is the name being changed, -1 for a new one.
    QString nameProblem(const NameDraft &draft, int id) const;
    Outcome defineName(const NameDraft &draft, int *made = nullptr);
    Outcome changeName(int id, const NameDraft &draft);
    Outcome removeName(int id);
    // Names made from the labels round a block of cells, as one step (core/namesfromcells.h): how many were made, and how many were left out (a label that made no name, a repeat, and a name
    // that is there already is not made again).
    Outcome defineNamesFromLabels(int sheet, const jp::office::sheet::CellRange &block, int edges, int *made = nullptr, int *skipped = nullptr);
    // ---- how the workbook calculates
    CalcSettings calcSettings() const { return m_calcNow; }
    Outcome setCalcSettings(const CalcSettings &settings);

    // ---- saving
    // Whether there is anything the file lacks: the undo stack is not at the state that was last saved.
    bool isModified() const { return !m_undo.isClean(); }
    // What the file needs to be made, taken now (nothing is held on to): false, with a message, if it can't be made.
    struct Snapshot {
        jp::office::sheet::SaveChanges changes;
        int strings = 0, numberFormats = 0, cellFormats = 0, fonts = 0, fills = 0, borders = 0, styles = 0, dxfs = 0;   // how many of the workbook's additions are in it
        std::map<quint64, jp::calc::Value> results;            // the results it holds (to be the file's, once it is written)
        std::vector<quint64> touchedFormulas;                  // the formula cells it holds whole, with the results they were given
        std::map<quint64, jp::calc::Value> touchedResults;
        std::map<int, QString> headerDrawings;                 // set by the save: the drawing of header and footer pictures each sheet names now (see SaveReport)
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
    // The sheets were changed (added, deleted, renamed, moved, shown, hidden, or given a color) by a step, Undo, or Redo: the window makes its tabs again.
    void sheetsChanged();
    // How the workbook calculates was changed (by a step, or by Undo of one).
    void calcSettingsChanged();
    // Cells were changed by an edit, Undo, or Redo (the window draws them again).
    void cellsChanged(int sheet, const std::vector<jp::grid::ValueStore::Item> &cells);
    // Rows, columns, or cells were inserted or deleted (or put back by Undo: `edit` is what was done to the cells then); the window makes its picture of the sheet again.
    void cellsMoved(const jp::office::sheet::StructEdit &edit);
    // The width of columns or the height of rows of a sheet changed (by a step, Undo, or Redo): the window makes its picture of the sheet again.
    void sizesChanged(int sheet);
    // A step of the undo stack could not be undone or done again (the workbook could not be held, or the edit was refused): Undo and Redo of every step are given up, and this says so.
    void stepFailed(const QString &what);
    // The tables of a sheet changed (made, taken out, resized, restyled, or renamed, by a step, Undo, or Redo): the window draws the sheet again.
    void tablesChanged(int sheet);
    // How a sheet prints changed (a command, Undo or Redo of one, or an edit of rows, columns, or cells that moved its print area, titles, or breaks).
    void pageSetupChanged(int sheet);

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
        const jp::office::sheet::FormatChange *change = nullptr;   // the change (the caller holds it until the call is over), or none to clear the formats
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
    // `alreadyDone`: the step was put in a slice at a time already (see SlicedPush): it is put on the stack and not done again.
    void pushStep(Step step, const QString &text, bool alreadyDone = false);
    // A text typed in a cell of a table's header row: the column is renamed (the formulas that name it follow, and the table's part when the workbook is saved).
    Outcome typeInHeader(int sheet, quint32 row, quint32 col, int table, int column, const QString &typed);
    // After a typed entry in a table's cell: the table grows, and a formula typed in an empty column becomes the column's (edittables.cpp).
    void growTableAfterEntry(int sheet, quint32 row, quint32 col, const CellChange &entry);
    void calculatedColumnAfterEntry(int sheet, quint32 row, quint32 col, const CellChange &entry);
    void applyTableEdits(Step *step, bool forward, jp::office::sheet::Calculation *calc, jp::office::sheet::Workbook &book, std::set<int> *sheets);
    Outcome pushTableStep(Step step, const QString &text);
    // A change that puts text (never read as anything else) or a formula (as the file writes it) in a cell, or clears it.
    CellChange textCell(int sheet, quint32 row, quint32 col, const QString &text);
    CellChange formulaCell(int sheet, quint32 row, quint32 col, const QString &formula);
    CellChange clearedCell(int sheet, quint32 row, quint32 col);
    bool tableFree(int sheet, const jp::office::sheet::CellRange &range, int except, QString *why) const;
    bool cellsEmpty(int sheet, const jp::office::sheet::CellRange &range) const;
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
    bool m_sheetRemoved = false;                     // a sheet added was taken away by Undo (so results may come for sheets that are not there)
    void forgetSheet(int sheet);                     // what is kept by cell for a sheet taken away is let go of
    std::unordered_set<quint64> m_touched;           // cells changed by the person (written whole), by sheet, row, column
    std::map<quint64, jp::calc::Value> m_results;      // formulas whose value is not the file's
    std::map<quint64, jp::calc::Value> m_engineAtStart;  // for a formula whose result has changed: the engine's value before its first change since the file was read or saved
    qint64 m_token;
    std::set<int> m_formatsChanged;  // the sheets whose default sizes a step changed since the file was read or saved
    bool m_sheetsListed = false;                          // a step changed the list of sheets since the file was read or saved
    std::set<int> m_sheetsRestyled;                       // the sheets a step gave the format of very many cells (drawn again whole)
    std::set<int> m_statesChanged;                        // the sheets a step hid or showed
    std::set<int> m_tabColorsChanged;                     // the sheets whose tab color a step changed
    std::vector<jp::office::sheet::SheetOp> m_sheetOps;   // renames and deletes of sheets since the file was read or saved that are done (not undone), for the file
    // The sheets copied since the file was read or saved that are done (not undone): the copy, the sheet it is a copy of, and how many edits to rows and columns (m_edits) there were then.
    // The copy's file begins as the file of the sheet, with the edits made to it before the copy (the copy's own cells come from the model).
    struct CopyPoint {
        int sheet = 0, source = 0;
        size_t edits = 0;
    };
    std::vector<CopyPoint> m_copies;
    const CopyPoint *copyPointOf(int sheet) const;
    QString notCopiedNote(int sheet) const;
    void applyPermute(jp::office::sheet::CellPermute &p, bool forward);
    void forgetNotes();
    bool applySheetEdits(Step *step, bool forward, jp::office::sheet::Calculation *calc, jp::office::sheet::Workbook &book);
    Outcome pushSheetEdit(SheetEdit edit, const QString &text);
    bool sheetStepsRefused(QString *why);
    bool applyNameEdits(Step *step, bool forward, jp::office::sheet::Calculation *calc);
    void applyCalcEdits(Step *step, bool forward, jp::office::sheet::Calculation *calc);
    void nameChangesForSave(std::vector<jp::office::sheet::NameChange> *out) const;
    std::vector<int> shadowingSheets(const QString &name) const;
    NameTable m_nameTable;
    std::vector<jp::office::sheet::NameRename> m_nameRenames;   // names given other names since the file was read or saved that are done (not undone), for the file
    CalcSettings m_calcNow, m_calcFile;                          // how it calculates, and how the file says
    std::set<int> m_mergesChanged;   // the sheets whose merged ranges a step changed since the file was read or saved
    std::set<int> m_filtersChanged;  // and those whose filter a step changed
    std::map<int, std::pair<quint32, quint32>> m_filterRows;   // and the rows (from the first to the last) of each whose rows a filter showed or hid
    bool m_slicing = false;          // a step is being put in a slice at a time
    qint64 m_longestSlice = 0;
    int m_slicesPushed = 0;
    mutable qint64 m_formatsMade = 0;
    mutable std::unordered_map<quint32, std::shared_ptr<jp::calc::NumberFormat>> m_formats;   // the number formats of the cell formats, made when first wanted
    mutable jp::calc::NumberSymbols m_symbols;   // the marks of the locale, made when first wanted
    mutable bool m_symbolsReady = false;
    std::vector<jp::office::sheet::CellPermute> m_permutes;   // cells put in another order since the file was read or saved, in order, for the file (and for where the notes are)
    struct NotesOfSheet {
        std::shared_ptr<const std::map<quint64, QString>> notes, raw;   // as moved by the edits, and as the file has them
        size_t edits = 0, permutes = 0;
        int saves = -1;
        bool cannot = false;
        QString why;
    };
    mutable std::map<int, NotesOfSheet> m_notes;
    std::set<quint64> m_columnsResized, m_rowsResized;   // the columns and rows (by sheet and place) whose size a step changed since the file was read or saved
    std::vector<jp::office::sheet::StructEdit> m_log;  // every edit that moved cells (the place of a batch's results in time: see generation)
    ObjectHook m_objectHook;
    std::vector<jp::office::sheet::StructEdit> m_edits;   // those since the file was read or saved that are done (not undone), for the file
    PageSettings m_pages;
    std::vector<jp::office::sheet::ColumnRename> m_renames;   // and the table columns renamed, for the names, validations, rules, and charts that name them
    std::vector<jp::office::sheet::TableRewrite> m_tableRewrites;   // and the tables renamed or made plain cells, for the same
    int m_opSeq = 0;                   // the order renames of columns and tables are made in
    bool m_lastCalculated = false;     // the last entry made its column a calculated column (see lastEntryMadeCalculatedColumn)
    int m_calculatedIndex = -1;        // the place on the undo stack of the step that made it
    bool m_bigStepAllowed = false;     // see allowNextBigStep
    int m_saveGen = 0;                 // how many times the workbook has been saved
    int m_forgotBySave = 0;            // the steps the last save made Undo forget (see forgetDeletesInFile)
    std::map<quint64, jp::calc::Value> m_fileResults;  // what the file holds for formulas whose value there is not the one the workbook was read with (after a save)
};

} // namespace jp::crunch
