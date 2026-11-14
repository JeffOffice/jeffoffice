#pragma once
// The form controls of a workbook's sheets, as the window shows and works them (see libs/office/sheet/formcontrol.h for what the file says of each). A sheet's controls are read when it is first
// shown; each is put in the layer over the cells with the other objects (grid/objectlayer.h), drawn by core/controlpaint.h, in the window and on the printed page.
//
// A control sets a cell and follows it, and runs no macro: a check box writes TRUE or FALSE, an option button its place among its group, a list its chosen line, a spin button or scroll bar its
// number (inside its limits); a button does nothing and says so. A click is turned into an Operation, which the window carries out (a cell typed, or, for a control with no cell, a change of the
// control's own state: one step of Undo either way). The cell is the truth: when it changes (typed, calculated, or put back by Undo) the control it is linked to takes its look from it; until it
// changes, the control shows what the file stored.
//
// Bounded: the controls of a sheet (formcontrol.h), the cells of a list (kMaxListItems), and the controls a person makes (kMaxMadeControls).

#include "grid/objectlayer.h"
#include "grid/valuestore.h"
#include "calc/value.h"
#include "sheet/formcontrol.h"
#include "sheet/sheetreader.h"

#include <QObject>
#include <QStringList>

#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

namespace jp::office::sheet {
struct SaveChanges;
}

namespace jp::crunch {

// The ids of controls in the layer start here (the ids of the other objects of a sheet are small numbers).
constexpr int kControlIdBase = 1 << 24;
constexpr int kMaxListItems = 1000;
constexpr int kMaxMadeControls = 500;

inline bool isControlId(int id) { return id >= kControlIdBase; }

// One control as a click or a person's choice changes it.
struct ControlChange {
    int id = 0;
    jp::office::sheet::FormControl before, after;
};

// What a click does: the controls whose own state changes, and the cell to write (typed as a person types it).
struct ControlOperation {
    int sheet = 0;
    std::vector<ControlChange> changes;
    bool setsCell = false;
    int cellSheet = 0;
    quint32 row = 0, col = 0;
    QString text;
};

class SheetControls : public QObject {
    Q_OBJECT
public:
    using ValueSource = std::function<jp::calc::Value(int sheet, quint32 row, quint32 col)>;

    explicit SheetControls(QObject *parent = nullptr);

    // The workbook whose controls are shown; everything is forgotten.
    void open(const std::shared_ptr<jp::office::sheet::Workbook> &book);
    // Where the value of a cell is asked (as the grid shows it).
    void setValueSource(ValueSource source) { m_values = std::move(source); }
    // Where the edits of rows and columns made since the file was read or saved are kept: a control moves with them (its anchor, and the cell and range it names), and one whose first cell is
    // deleted is gone until Undo brings the cells back.
    using EditsSource = std::function<const std::vector<jp::office::sheet::StructEdit> &()>;
    void setEditsSource(EditsSource edits) { m_edits = std::move(edits); }
    // Rows or columns were inserted or deleted (or that was undone): every listed control is where the edits put it.
    void cellsMoved();

    // The sheet's controls as the layer has them (the sheet's are read if they have not been); `forPrint` leaves out those that do not print and those that are hidden.
    std::vector<jp::grid::LayerObject> layerObjects(int sheet, bool forPrint = false);
    bool owns(int sheet, int id) const { return find(sheet, id) != nullptr; }
    // The layer's object for one control, if it is there to be shown (not taken away, not hidden).
    std::optional<jp::grid::LayerObject> layerObjectOf(int sheet, int id);
    // A control as it is now (`found` false: there is none); the same as it is in the file; and the ones made here.
    jp::office::sheet::FormControl controlOf(int sheet, int id, bool *found = nullptr) const;
    int countOf(int sheet) const;
    // The cell a control sets (the sheet, row, and column), if it names one that can be found.
    bool linkOf(int sheet, int id, int *cellSheet, quint32 *row, quint32 *col) const;
    // Whether the text is a cell (a range, with `range`) of this workbook, as a control's link is written ("$A$3", "Sheet1!$A$3").
    bool validReference(int sheet, const QString &ref, bool range) const;
    // The lines a list shows (the texts of the cells of its range) and the place of the one chosen (from 1; 0 none).
    QStringList itemsOf(int sheet, int id) const;
    // The name a control has for a person ("Check Box 1"), and the words shown over it when it is a button.
    static QString kindName(jp::office::sheet::ControlKind kind);

    // Gives the control a state (a step of Undo is done, undone, or done again); `deleted` takes it away (a made control that was undone). The layer is told through `changed`.
    void apply(int sheet, int id, const jp::office::sheet::FormControl &state, bool deleted);
    bool isDeleted(int sheet, int id) const;
    // Whether the control is not shown: taken away by a person, or its cells were deleted.
    bool isGone(int sheet, int id) const;
    // Makes a control of a kind at a place (the cells it is anchored to and where that is in points); it is there but taken away until a step gives it a state with `deleted` false. The id, or -1
    // with the reason.
    int make(int sheet, jp::office::sheet::ControlKind kind, const jp::office::sheet::DrawingAnchor &anchor, const QRectF &placePt, QString *why = nullptr);
    // The cells linked to were changed (or the sheet is shown): the controls look at them again. The ids of controls whose look changed, in the sheet.
    std::vector<int> refresh(int sheet);
    // The same for a batch of cells that changed (the places they are at now): only the controls linked to one of them look again (and those whose input range one is in are drawn again), so that
    // a batch that touches no control costs nothing of the controls'.
    std::vector<int> refreshCells(const std::vector<jp::grid::ValueStore::Item> &cells);
    // For tests: how many times a control looked at its cell, and how many controls the working out of option button groups passed, in all.
    qint64 looks() const { return m_looks; }
    qint64 groupVisits() const { return m_groupVisits; }
    // The controls that changed since the file was read or saved, for saving (each is the control as it is now; `was` is as the file has it, for a control that is not new).
    void addToSave(jp::office::sheet::SaveChanges *changes);
    // The file was saved: the controls are what it holds now.
    void saved();
    // Chooses a line of a list (a drop-down's popup): the operation a click on it makes.
    bool choose(int sheet, int id, int place, ControlOperation *out);
    // A control's change into the state `after` as a person asked in Format Control: the operation (a cell is not typed).
    bool formatted(int sheet, int id, const jp::office::sheet::FormControl &after, ControlOperation *out) const;

Q_SIGNALS:
    // A click on a control that does something: the window carries it out.
    void operated(const jp::crunch::ControlOperation &operation);
    // A click on a drop-down: the window opens the lines.
    void listRequested(int sheet, int id, const QStringList &items, int selected);
    // A click on a button (no macro runs).
    void macroSkipped(int sheet, int id);
    // The look of a control changed (a cell, an Undo): the layer draws it again.
    void changed(int sheet, int id);

private:
    struct Item {
        int id = 0;
        jp::office::sheet::FormControl now, was;
        bool deleted = false;          // taken away by a person
        bool dropped = false;          // the cell it is anchored at was deleted
        bool inFile = true;            // the file the program last read or wrote has its shape
        jp::office::sheet::DrawingAnchor baseAnchor;   // where it is as the file has it (the edits from `baseAt` on are worked in to give `now`)
        QString baseLink, baseRange;                   // the cell and range as they were when the edits from `refsAt` on began
        int baseAt = 0, refsAt = 0;
        bool seenKnown = false;
        QString seen;           // what its cell held when last looked at (a Value's debug text)
    };
    struct Sheet {
        bool listed = false;
        std::vector<std::shared_ptr<Item>> items;
        int nextId = kControlIdBase;
    };
    Sheet &sheetState(int sheet);
    void recompute(int sheet, Item &item);
    const Item *find(int sheet, int id) const;
    Item *find(int sheet, int id);
    void list(int sheet, Sheet &s);
    jp::grid::LayerObject layerObject(int sheet, const std::shared_ptr<Item> &item);
    // The place of a cell named by text ("$A$3", "Sheet1!$A$3") as seen from `sheet`.
    bool resolve(int sheet, const QString &ref, int *cellSheet, jp::office::sheet::CellRange *range) const;
    // The group an option button is in: the controls (their items, in order) from the one that begins it to the one before the next that does.
    std::vector<const Item *> groupOf(const Sheet &s, const Item &item) const;
    // The cell a link names, if it is one (as linkOf says it for the link a control has).
    bool cellOf(int sheet, const QString &link, int *cellSheet, quint32 *row, quint32 *col) const;
    // What is worked out of the controls of every sheet that is listed, once for as long as they stay as they are: each option button's group (the cell its group is linked by, its place in it), and which
    // controls are linked to which cell, and which list a range.
    struct GroupInfo {
        QString link;
        int place = 1;
    };
    struct Watched {
        int sheet = 0;
        Item *item = nullptr;
    };
    struct Ranged {
        int sheet = 0, cellSheet = 0;
        jp::office::sheet::CellRange range;
        Item *item = nullptr;
    };
    struct Index {
        bool valid = false;
        QStringList names;   // (the sheets' names it was worked out under: a link names a sheet by its name)
        std::unordered_map<quint64, std::vector<Watched>> byCell;
        std::vector<Ranged> ranged;
        std::unordered_map<const Item *, GroupInfo> groups;
    };
    void ensureIndex();
    // The control looks at its cell again; `changed`, if given, is told its id when its look changed.
    void look(int sheet, Item &item, std::vector<int> *changed);
    bool operate(int sheet, const std::shared_ptr<Item> &item, const QPointF &fraction, const QSizeF &units);
    // The state a value in the cell gives the control.
    bool takeValue(Item &item, const jp::calc::Value &v, int place);
    static QString valueText(const jp::calc::Value &v);

    std::shared_ptr<jp::office::sheet::Workbook> m_book;
    ValueSource m_values;
    EditsSource m_edits;
    std::map<int, std::shared_ptr<std::map<qint64, qint64>>> m_ids;   // by sheet: the shape numbers the last save gave the controls made here
    std::vector<Sheet> m_sheets;
    Index m_index;
    mutable qint64 m_looks = 0, m_groupVisits = 0;
};

} // namespace jp::crunch

Q_DECLARE_METATYPE(jp::crunch::ControlOperation)
