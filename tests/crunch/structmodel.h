#pragma once
// The model check of rows, columns, and cells inserted and deleted (and of the edits that move cells the same way: fill, copy and paste, cut and paste, sort, Undo, Redo, and a save that is
// read again), the harness that the redesign of how JeffCrunch holds its rows and columns is checked against, round by round (docs/jeffcrunch-rowids.md, "Round 0").
//
// A seed makes a list of steps, the same on every machine (the generator is its own, not Qt's). The steps are run on a small workbook of three sheets that has something of everything in it
// (see baseWorkbook), and after every step:
//   - the window's values for the formulas are what the calculation has;
//   - the workbook is saved to memory, opened again in a window of its own, and every formula is worked out again from nothing: the values, the text of every formula, and the state of everything
//     that follows the cells (conditional formats, validations, defined names, tables, charts, notes, links, merged cells, print areas, page breaks, filters, row and column sizes) must be equal
//     to what the session that was edited holds;
//   - the graph of what reads what holds every cell that each formula actually reads (a formula is worked out again over a source that writes down what it is asked for);
//   - a step that was refused changed nothing.
// A failure says the seed and the fewest steps that still fail (steps are dropped while it does). The state after every step is also reduced to 64 bits (`digest`): the digests of a fixed set of
// seeds are kept in the repository (tests/crunch/structmodel_golden.txt), so that a change that keeps what the engine means can be compared with today's engine step by step.
// The same code runs in the tests (a few seeds) and in `jcrunchtool model-check` (as many as asked for).

#include <QMap>
#include <QByteArray>
#include <QString>
#include <QStringList>

#include <cstdint>
#include <vector>

namespace jp::structmodel {

struct Step {
    enum Kind : int {
        InsertRows, DeleteRows, InsertColumns, DeleteColumns, InsertCellsDown, InsertCellsRight, DeleteCellsUp, DeleteCellsLeft,
        Type, Fill, Paste, Cut, Sort, Undo, Redo, Reopen, Save,
        Merge, ResizeRows, ResizeColumns, SetNote, SetLink, DefineName, SetPage,
        KindCount
    };
    Kind kind = Undo;
    int sheet = 0;
    int row1 = 0, col1 = 0, row2 = 0, col2 = 0;   // the cell or block (rows and columns from 0); rows and columns edits use row1 or col1 as the first
    int count = 1;                                // rows or columns, or the cells a fill makes
    int toSheet = 0, toRow = 0, toCol = 0;        // a cut and paste, and the cell a paste goes to
    int option = 0;                               // what a paste puts, a fill's direction, a sort's order, what a merge, a resize, a link, or a page setup does (see the .cpp)
    QString text;                                 // what is typed
    // In words, for a failure to show; and as one line a file can hold (fromLine reads it back).
    QString words() const;
    QString line() const;
    static bool fromLine(const QString &line, Step *out);
};

// The workbook every run begins with: three sheets ("Data", "Calc", "Extra") of about 40 rows and 12 columns, with values and formulas of every kind of reference, shared formulas, a table, a
// chart, notes, links, merged cells, conditional formats, validations, defined names, a print area and titles, page breaks, and a filter.
QByteArray baseWorkbook();

// The steps a seed makes (the same everywhere).
// `extra`: the typed formulas are also of kinds the golden seeds do not have (a range back to front, ATAN2, a word that could be a cell past XFD); the same seed is another list of steps.
std::vector<Step> generate(quint32 seed, int count, bool extra = false);

struct Options {
    bool check = true;        // the checks after every step (else the steps are run and only the digests are made)
    bool graph = true;        // and the check of the graph
    int checkEvery = 1;       // the checks are made after every nth step (and after the last)
    bool forceOff = false;    // the formulas are held in places whatever the process says
    bool idFrame = false;     // the workbooks hold their formulas in the id frame (CalcOptions::idFrame: docs/jeffcrunch-rowids.md, round 5); else as the process says (JC_ID_FRAME)
    int fault = 0;            // for the tests of the harness itself: 1 to 16 change what the file read again holds of one thing (see the .cpp), 100 makes the graph check ask for a cell nothing reads, 101 changes what the model says
};

struct Failure {
    bool failed = false;
    int step = 0;             // the steps that had been run when it was found (0: the workbook before any step)
    QString kind;             // what kind of failure: "store", "graph", "refused", "open", or the part of the state that differed ("cell", "val", "cf", ...)
    QString what;
    bool ok() const { return !failed; }
};

struct Result {
    Failure failure;
    std::vector<quint64> digests;    // after each step run (the state of the session that was edited, as 64 bits)
    std::vector<QString> stepsText;  // each step in words
    int refused = 0;                 // steps the workbook did not take
    int undone = 0, redone = 0;      // Undo and Redo that did something
    int circles = 0;                 // steps after which a formula was in a circle (the values were then not compared)
    int checks = 0;
    qint64 msSteps = 0, msOpen = 0, msDescribe = 0, msGraph = 0;   // where the time went: the steps, opening the saved workbook, describing the states, and the graph check
    int modelChecks = 0;             // edits of whole rows or columns that were held to the second opinion
    int lineEdits = 0;               // rows or columns inserted or deleted that the workbook took, and the cells the engine moved to make them (none: a cell is held by the id of its row and column)
    qint64 lineEditCellsMoved = 0;
    qint64 graphFormulas = 0, graphReads = 0;   // formulas the graph check worked out again, and what they read
    QMap<QString, QByteArray> smallParts;   // the bytes of the parts under 4 KB, to show in a report when one differs
    quint64 partsDigest = 0;         // the parts of the file a save would write after the last step (their bytes), as 64 bits
    QStringList parts;               // and each part's name and digest
    QString finalState;              // the state after the last step, one line for each thing (what `describe` makes; for a look at it)
};

// Runs the steps from the base workbook, with the checks the options ask for; stops at the first failure.
Result run(const std::vector<Step> &steps, const Options &options);

// Drops steps while the same kind of failure remains (those after the failing one first); returns what is left.
std::vector<Step> shrink(const std::vector<Step> &steps, const Options &options, const Failure &failure);

// The text a failure is told in: the seed, the failure, and the steps one to a line (each as `line()` and in words).
QString report(quint32 seed, const std::vector<Step> &steps, const Failure &failure);

// The second opinion's rewriting of a formula's text (no "=" in front) written on `formulaSheet`, for a step that inserts or deletes whole rows or columns (the text is returned as it is for any
// other step): what the model of the edit says the formula becomes. For the tests of the model.
QString modelRewrite(const QString &formula, int formulaSheet, const Step &edit);

// Digests: the golden file's contents for the seeds (`steps` each), made by running without the checks.
QString goldenText(const std::vector<quint32> &seeds, int steps);

// `jcrunchtool model-check <seeds> <steps> [first seed] [idframe] [wide]`, `model-golden`, `model-replay <file>`, and `model-dump <seed> <steps>` (see tools/jcrunchtool_edit.cpp).
int command(const QStringList &args);

} // namespace jp::structmodel
