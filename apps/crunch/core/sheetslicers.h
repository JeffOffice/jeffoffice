#pragma once
// The slicers of a workbook's sheets, as the window shows and works them: a panel of buttons over the cells where the file puts it (the anchor of its frame in the sheet's drawing), drawn by
// core/slicerpaint.h in the layer with the other objects (grid/objectlayer.h), in the window and on the printed page. What a slicer filters, and which items it shows chosen, is the table's:
// EditSession::slicerFace says it, and a click is told to the window, which makes it one step of Undo through the same path the Slicers pane uses (EditSession::clickSlicerItem). The slicer is
// never kept here: it has no state of its own to save (its anchor is the drawing's, and the file's drawing part is left as it is).
//
// Not yet: moving, sizing, or deleting one, scrolling the buttons that do not fit (the last row says how many there are, and opens the Slicers pane), choosing several with Shift, or making one.
//
// Bounded: the slicers of a sheet are those of its drawing (at most kMaxDrawingAnchors with the other objects), and each draws and answers for at most kMaxSlicerButtons buttons.

#include "core/sheetcontrols.h"
#include "core/slicerface.h"
#include "grid/objectlayer.h"
#include "sheet/drawing.h"
#include "sheet/sheetreader.h"

#include <QObject>

#include <functional>
#include <memory>
#include <optional>
#include <vector>

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
    std::optional<jp::grid::LayerObject> layerObjectOf(int sheet, int id);
    bool owns(int sheet, int id) const { return find(sheet, id) != nullptr; }
    int countOf(int sheet) const;
    // The place of the slicer's element among the children of the drawing's root (the order the file draws in: an object whose own place is later is above the slicer).
    int ordinalOf(int sheet, int id) const;
    // Whether the slicer is not shown (its cell was deleted).
    bool isGone(int sheet, int id) const;
    // Rows or columns were inserted or deleted (or that was undone): every listed slicer is where the edits put it.
    void cellsMoved();
    // The tables a slicer reads changed: the slicers of the sheet are drawn again (and ask for their look when they are).
    void looksChanged(int sheet);
    // The file was saved: the anchors the edits gave are the file's.
    void saved();
    // The anchor a slicer has now.
    jp::office::sheet::DrawingAnchor anchorOf(int sheet, int id) const;
    // What a press on a slicer does, as a click at `at` (units from its top left) on one `size` big: the item or the clear button, told by the signals. False if it is on nothing.
    bool press(int sheet, int id, const QPointF &at, const QSizeF &size, bool extend);
    // For tests: how many looks (faces) were asked of the engine.
    qint64 looks() const { return m_looks; }

Q_SIGNALS:
    // A click on the button of the item at `place`, with Ctrl held (`extend`) or not.
    void itemClicked(int slicer, int place, bool extend);
    // A click on the line that says buttons are left out: the window shows the Slicers pane, which lists every item.
    void moreClicked(int slicer);
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
        int ordinal = 0;
        bool hidden = false, prints = true, dropped = false;
        int slicer = -2;   // which slicer it is (-1 none, -2 not looked for yet)
    };
    struct Sheet {
        bool listed = false;
        std::vector<std::shared_ptr<Item>> items;
    };
    const Item *find(int sheet, int id) const;
    Item *find(int sheet, int id) { return const_cast<Item *>(static_cast<const SheetSlicers *>(this)->find(sheet, id)); }
    int slicerOf(int sheet, Item &item);
    bool faceOf(int sheet, Item &item, SlicerFace *face);
    jp::grid::LayerObject layerObject(int sheet, const std::shared_ptr<Item> &item);
    void recompute(int sheet, Item &item);

    std::shared_ptr<jp::office::sheet::Workbook> m_book;
    FaceSource m_faces;
    NameSource m_names;
    EditsSource m_edits;
    std::vector<Sheet> m_sheets;
    mutable qint64 m_looks = 0;
};

} // namespace jp::crunch
