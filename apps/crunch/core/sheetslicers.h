#pragma once
// The slicers of a workbook's sheets, as the window shows and works them: a panel of buttons over the cells where the file puts it (the anchor of its frame in the sheet's drawing), drawn by
// core/slicerpaint.h in the layer with the other objects (grid/objectlayer.h), in the window and on the printed page. What a slicer filters, and which items it shows chosen, is the table's:
// EditSession::slicerFace says it, and a click is told to the window, which makes it one step of Undo through the same path the Slicers pane uses (EditSession::clickSlicerItem). The slicer is
// never kept here: it has no state of its own to save but where it is and whether it is there (its anchor is the drawing's: moved or sized, it is written to the frame and to its stand-in, and
// deleted, the slicer, its frame, and what only it used leave the file; see SaveChanges::removedSlicers). It is picked, moved, sized, and deleted like another object (a press on a button or the
// scroll bar is its own, a press on its caption or frame picks it), scrolls when the buttons do not fit, and chooses a run of items with Shift or by the keyboard.
//
// Not yet: making one, a timeline, or putting it in another place in the order the file draws in.
//
// Bounded: the slicers of a sheet are those of its drawing (at most kMaxDrawingAnchors with the other objects), and each draws and answers for at most kMaxSlicerButtons buttons.

#include "core/sheetcontrols.h"
#include "core/objectstate.h"
#include "core/slicerface.h"
#include "grid/objectlayer.h"
#include "sheet/drawing.h"
#include "sheet/sheetreader.h"

#include <QKeyEvent>
#include <QObject>
#include <QVector>

#include <functional>
#include <memory>
#include <optional>
#include <vector>

namespace jp::office::sheet {
struct SaveChanges;
}

namespace jp::crunch {

class SheetSlicers : public QObject {
    Q_OBJECT
public:
    // What a slicer looks like now (EditSession::slicerFace), and which slicer the drawing's name is (EditSession::slicerNamed).
    using FaceSource = std::function<bool(int slicer, const QString &name, int sheet, qint64 first, int most, SlicerFace *out)>;
    using NameSource = std::function<int(const QString &name, int sheet)>;
    using EditsSource = std::function<const std::vector<jp::office::sheet::StructEdit> &()>;

    explicit SheetSlicers(QObject *parent = nullptr);

    // The workbook whose slicers are shown; everything is forgotten.
    void open(const std::shared_ptr<jp::office::sheet::Workbook> &book);
    void setSources(FaceSource faces, NameSource names);
    // Where the edits of rows and columns made since the file was read or saved are kept: a slicer moves with them, and one whose first cell is deleted is not shown.
    void setEditsSource(EditsSource edits) { m_edits = std::move(edits); }

    // The slicers the sheet's drawing holds (SheetObjects reads the drawing once and tells them here); only the first telling counts.
    void list(int sheet, const QVector<jp::office::sheet::DrawingSlicer> &found);
    // The sheet's slicers as the layer has them; `forPrint` leaves out those that do not print. A slicer marked hidden in the file is not shown.
    std::vector<jp::grid::LayerObject> layerObjects(int sheet, bool forPrint = false);
    // The ids of the sheet's slicers that are there, in the order the file draws them; the next one after `from` (-1: from the start; backwards: before it) or -1 if there is none.
    int neighbor(int sheet, int from, bool backwards) const;
    std::optional<jp::grid::LayerObject> layerObjectOf(int sheet, int id);
    bool owns(int sheet, int id) const { return find(sheet, id) != nullptr; }
    int countOf(int sheet) const;
    // The place of the slicer's element among the children of the drawing's root (the order the file draws in: an object whose own place is later is above the slicer).
    int ordinalOf(int sheet, int id) const;
    // Whether the slicer is not shown (a person deleted it, or its cells were deleted, or the file hides it); `isDeleted`: a person deleted it.
    bool isGone(int sheet, int id) const;
    bool isDeleted(int sheet, int id) const;
    QString nameOf(int sheet, int id) const;
    // For tests: how many rows the slicer is scrolled by, and where the keyboard is among its buttons (-1: nowhere).
    int scrollRowOf(int sheet, int id) const;
    qint64 focusOf(int sheet, int id) const;
    // The slicer as it is now for a step of Undo (its place and whether it is there), the same moved to another place, and the slicer given a state (a step done, undone, or done again). `placed`: the
    // place in the state is of the rows and columns now (not of before a save that followed edits to them).
    ObjectState stateOf(int sheet, int id, int epoch) const;
    ObjectState stateMoved(int sheet, int id, const jp::office::sheet::DrawingAnchor &anchor, int epoch) const;
    void apply(int sheet, int id, const ObjectState &state, bool placed);
    // What saving needs (see SaveChanges): the places of the elements of the sheet's drawing that were taken away, among the children of its root.
    std::vector<int> removedOrdinals(int sheet) const;
    // Puts what the person did to the sheet's slicers in the changes: the moved ones' anchors, the deleted ones' frames and parts. `removed` is removedOrdinals plus the other objects' (sorted).
    void addToSave(int sheet, const QString &drawingPart, jp::office::sheet::SaveChanges *changes, const std::vector<int> &removed, const std::function<jp::office::sheet::DrawingAnchor(const jp::office::sheet::DrawingAnchor &)> &twoCell);
    // Rows or columns were inserted or deleted (or that was undone): every listed slicer is where the edits put it.
    void cellsMoved();
    // The tables a slicer reads changed: the slicers of the sheet are drawn again (and ask for their look when they are).
    void looksChanged(int sheet);
    // The file was saved: the anchors the edits gave are the file's.
    void saved();
    // The anchor a slicer has now.
    jp::office::sheet::DrawingAnchor anchorOf(int sheet, int id) const;
    // What a press on a slicer does, as a click at `at` (units from its top left) on one `size` big, with these keys held: the item, the run of items, the clear button, or the scroll bar, told by the signals
    // or done. False if it is on none of them (then it picks the slicer).
    bool press(int sheet, int id, const QPointF &at, const QSizeF &size, Qt::KeyboardModifiers keys);
    // The thumb of the scroll bar dragged to `at` (a press on it took it), or let go; and the wheel, `steps` notches (positive: up). True if the slicer scrolled (or could).
    void dragged(int sheet, int id, const QPointF &at, const QSizeF &size, bool done);
    bool wheeled(int sheet, int id, double steps, const QSizeF &size);
    // The keyboard is in the slicer (its buttons have the arrow keys), and which button; `enter` puts it on the first.
    bool isInside(int sheet, int id) const;
    void enter(int sheet, int id);
    void leave(int sheet, int id);
    // A key while the slicer is picked; true if it was the slicer's.
    bool key(int sheet, int id, QKeyEvent *event);
    // For tests: how many looks (faces) were asked of the engine.
    qint64 looks() const { return m_looks; }

Q_SIGNALS:
    // A click on the button of the item at `place`, with Ctrl held (`extend`) or not.
    void itemClicked(int slicer, int place, bool extend);
    // A Shift-click: the items at these places (the run from the item last clicked, as shown) are chosen, with those already chosen kept when Ctrl was held (`extend`).
    void runClicked(int slicer, const QVector<int> &places, bool extend);
    // A click on the line that says buttons are left out: the window shows the Slicers pane, which lists every item.
    void moreClicked(int slicer);
    // F6 or Shift+F6 pressed with a slicer picked: the keyboard goes on to the next slicer (or the previous), or to the cells after the last.
    void neighborWanted(bool backwards);
    // A click on the clear-filter button.
    void clearClicked(int slicer);
    // A slicer came back or went (its cells were deleted, or that was undone), or moved.
    void changed(int sheet, int id);
    // A slicer's look changed.
    void lookChanged(int sheet, int id);

private:
    struct Item {
        int id = 0;
        QString name;
        jp::office::sheet::DrawingAnchor baseAnchor, anchor;
        int ordinal = 0, savedOrdinal = 0;
        bool hidden = false, prints = true, dropped = false;
        bool deleted = false;        // a person deleted it
        bool inFile = true;          // the file has its frame (a deleted one that was saved has not)
        bool moved = false;          // a person moved or sized it since the file was read or saved
        int baseAt = 0;              // how many of the edits to rows and columns had been made when baseAnchor was given
        jp::office::sheet::DrawingAnchor fileAnchor;
        int slicer = -2;   // which slicer it is (-1 none so far, -2 not looked for yet: both are looked for again)
        // how it is shown: scrolled by `scrollRow` rows, the keyboard on button `focus` (-1: none; `inside`: the arrow keys are the buttons'), the item a run of items goes from, and the size it was last drawn at
        int scrollRow = 0, columns = 1;
        qint64 focus = -1, anchorAt = -1;
        bool inside = false, thumbHeld = false;
        double thumbGrab = 0, wheelRest = 0;   // (`wheelRest`: the part of a row a small turn of the wheel has not yet moved)
        QSizeF size;
    };
    struct Sheet {
        bool listed = false;
        std::vector<std::shared_ptr<Item>> items;
    };
    const Item *find(int sheet, int id) const;
    Item *find(int sheet, int id) { return const_cast<Item *>(static_cast<const SheetSlicers *>(this)->find(sheet, id)); }
    int slicerOf(int sheet, Item &item);
    bool faceOf(int sheet, Item &item, SlicerFace *face, const QSizeF &size, bool scrolls);
    int editCount() const { return m_edits ? int(m_edits().size()) : 0; }
    void scrollTo(int sheet, Item &item, int row);
    jp::grid::LayerObject layerObject(int sheet, const std::shared_ptr<Item> &item, bool forPrint = false);
    void recompute(int sheet, Item &item);

    std::shared_ptr<jp::office::sheet::Workbook> m_book;
    FaceSource m_faces;
    NameSource m_names;
    EditsSource m_edits;
    std::vector<Sheet> m_sheets;
    mutable qint64 m_looks = 0;
};

} // namespace jp::crunch
