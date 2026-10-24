#pragma once
// Editing a workbook that is open: what a person types into a cell, Delete, and Undo and Redo, on the window's thread; and what saving needs to know about it. The cells and the
// formulas are the worker's while it works (WorkbookLoader), so a change is made while the window holds the worker back for it (WorkbookLoader::hold), and it is the cell and
// its place in the cells that are changed there; finding the formulas that read the cell, and working them out, is the worker's, and the results come back in batches as ever.
//
// Every edit is a command on a QUndoStack that holds the cells as they were and as they are, whole (content, formula, format) and nothing else: no copy of the workbook, no values
// of the formulas that depend (they are worked out again), so the memory it takes is what the edits changed. A cell typed into is one step. The session also keeps what saving needs: the
// cells that changed, and the formulas whose result is not what the file has (known from the batches the worker sends).

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
#include <vector>

namespace jp::crunch {

// The most cells a Delete (or any one edit) changes: past it the edit is refused, with a message, so that it never holds the worker, or the window, for long.
constexpr qint64 kMaxCellsPerEdit = 100000;
// The steps Undo goes back (each a command of the stack).
constexpr int kUndoSteps = 1000;

struct CellChange {
    int sheet = 0;
    quint32 row = 0;
    quint16 col = 0;
    jp::office::sheet::Calculation::CellContent before, after;
    // What is to be settled in the workbook when the change is first made (it can't be before: the shared strings and the styles are changed with the worker held back): a text that
    // is to go in the shared strings, and a number format (a code) the cell's format is to be given.
    bool textToShare = false;
    QString formatCode;
};

// A table's column renamed by a step (its header cell is one of the step's cells): the table by its place in the workbook's tables, the column by its place in the table.
struct TableRename {
    int table = 0;
    int column = 0;
    QString before, after;
};

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

    // Whether the workbook can be changed now: it is read whole (the worker is ready), no sheet was left in part, and no save is going on; else why not.
    bool canEdit(QString *why = nullptr) const;
    // Whether a sheet has cells to change (a chart sheet has none).
    bool canEditSheet(int sheet, QString *why = nullptr) const;
    // Whether a cell can be: it is on a sheet of cells, and not part of an array or a data table.
    bool canEditCell(int sheet, quint32 row, quint32 col, QString *why = nullptr) const;

    // What a cell is edited as (see editTextOfValue); a formula with its "=".
    QString editText(int sheet, quint32 row, quint32 col) const;
    // What the formula bar shows for a cell: the text the grid gives for it (`gridText`), written as the person types in the locale (a formula with its separators, a number with its
    // decimal mark); the grid's text itself where the locale needs no change.
    QString barText(int sheet, quint32 row, quint32 col, const QString &gridText) const;
    // The number format code of a cell as its format gives it ("General" for none).
    QString formatCodeOf(int sheet, quint32 row, quint32 col) const;

    struct Outcome {
        bool ok = true;
        QString problem;      // a sentence for the person, when not ok
        int problemAt = -1;   // and the character of what was typed it is about, or -1
    };
    // Types text into a cell, as the person typed it: a number, a percent, a date, TRUE, a formula, or text; the cell takes the format the entry asks for if it has none of its own. The
    // change is one step on the undo stack. Not ok, and nothing changed, if the text is a formula that is not one, or is too long, or the cell cannot be changed.
    Outcome type(int sheet, quint32 row, quint32 col, const QString &typed);
    // Clears the contents of the cells in the range (their formats stay): one step. Not ok, and nothing changed, if a cell in it is part of an array, or there are more cells than
    // kMaxCellsPerEdit.
    Outcome clear(int sheet, qint64 row1, qint64 col1, qint64 row2, qint64 col2);

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
        int strings = 0, numberFormats = 0, cellFormats = 0;   // how many of the workbook's additions are in it
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

private:
    friend class ChangeCells;
    // Makes the changes (forward: to `after`; else back to `before`) with the worker held back. False if the worker could not be held.
    bool apply(std::vector<CellChange> *changes, bool forward, const std::vector<TableRename> *renames = nullptr);
    // A text typed in a cell of a table's header row: the column is renamed (the formulas that name it follow, and the table's part when the workbook is saved).
    Outcome typeInHeader(int sheet, quint32 row, quint32 col, int table, int column, const QString &typed);
    jp::office::sheet::EntryOptions entryOptions(int sheet, bool textFormat) const;
    // Whether a cell of that format on the sheet can't be changed: the sheet is protected and the cell is locked.
    bool isLocked(int sheet, quint32 style) const;
    QString lockedMessage() const;
    bool storedEquals(int sheet, quint32 row, quint32 col, const jp::calc::Value &value) const;
    void cleanChanged(bool clean);

    WorkbookLoader *m_loader;
    std::shared_ptr<LoadedBook> m_book;
    jp::grid::ValueStore *m_values;
    QUndoStack m_undo;
    QLocale m_locale;
    QDate m_today;
    bool m_busy = false;
    std::function<void(jp::office::sheet::Calculation *, bool)> m_holdHook;
    qint64 m_longestHoldWait = 0, m_lastHoldWait = 0;
    bool m_tracking = false;               // an edit has been made: results are compared with the file's
    std::set<quint64> m_touched;           // cells changed by the person (written whole), by sheet, row, column
    std::map<quint64, jp::calc::Value> m_results;      // formulas whose value is not the file's
    std::map<quint64, jp::calc::Value> m_engineAtStart;  // for a formula whose result has changed: the engine's value before its first change since the file was read or saved
    std::map<quint64, jp::calc::Value> m_fileResults;  // what the file holds for formulas whose value there is not the one the workbook was read with (after a save)
};

} // namespace jp::crunch
