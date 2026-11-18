#pragma once
// The embedded objects of a workbook's sheets (libs/office/ole/embedobj.h): a range, a chart, or a page of one of our apps, or an object made by another program, each kept as the package it is
// with the picture that shows it. They are not pictures: an object keeps its package, so the program that made it opens it again, and its own picture, which is what the sheet shows (a
// re-encoded picture never loses the object). A sheet's objects are read when it is first shown (libs/office/sheet/drawing.h, readSheetOleObjects); each is put in the layer over the cells
// with the other objects (grid/objectlayer.h) under ids that start at kEmbedIdBase, drawn from its picture, in the window and on the printed page.
//
// An object can be placed, moved, sized, deleted, and copied (the copy is an object of its own, and so is that of a copied sheet); one of ours can be changed too (the program that made it saves
// it again), and its picture is drawn again when it is sized. All are written as embedded objects of the file (an .xlsx holds one in the sheet's `oleObjects`; an .ods as a draw:object with its
// picture). An object of another program is kept as the file has it, byte for byte (its package, its picture whatever kind it is, and its element): moving or sizing it changes only the place
// the file says, and it follows rows and columns inserted or deleted as ours does.
//
// Bounded: the objects of a workbook are held to ole::limits() (their count, package bytes, and pictures), the sheets are read when the workbook is opened, in order, against one budget, a
// sheet that has a hostile number of objects is read only as far as the budget reaches, a sheet whose part is larger than 64 MB is not read through (its objects stay in the file untouched),
// and a picture is drawn only if it is no larger than kMaxPreviewBytes once unpacked.

#include "core/objectstate.h"
#include "core/sheetcontrols.h"
#include "grid/objectlayer.h"
#include "sheet/drawing.h"
#include "sheet/sheetreader.h"
#include "sheet/xlsxsave.h"

#include <QObject>

#include <functional>
#include <memory>
#include <optional>
#include <vector>

namespace jp::crunch {

// The most a picture may take once unpacked (what the layer's pictures are held to as well).
constexpr qint64 kMaxPreviewBytes = qint64(64) << 20;

class SheetEmbeds : public QObject {
    Q_OBJECT
public:
    using EditsSource = std::function<const std::vector<jp::office::sheet::StructEdit> &()>;
    // Where an object is on the screen now, in the grid's units (false: not known, the object's own size is used).
    using BoxSource = std::function<bool(int sheet, int id, double *w, double *h)>;

    explicit SheetEmbeds(QObject *parent = nullptr);

    // The largest sheet part that is looked through for objects (64 MB; a test lowers it, 0 puts the real one back).
    static constexpr qint64 kScanBytes = qint64(64) << 20;
    static qint64 scanLimit();
    static void setScanLimit(qint64 bytes);

    // The picture of an object as an image, unpacked (null for one that is not a PNG, or that would unpack to more than kMaxPreviewBytes: that is looked at in the header, before any of it is
    // unpacked).
    static QImage previewImage(const QByteArray &png);
    // How many pictures have been unpacked (for a test to count that a picture is unpacked once, not at every drawing).
    static qint64 previewReads();

    // The workbook whose objects are shown; everything is forgotten, and every sheet is read (once, in order: the budget is the workbook's).
    void open(const std::shared_ptr<jp::office::sheet::Workbook> &book);
    // Where the sheet a sheet was copied from is asked (-1: it is no copy).
    using CopySource = std::function<int(int sheet)>;
    void setCopySource(CopySource source) { m_copySource = std::move(source); }
    // Sheets were added, copied, or taken away (or that was undone): a copy has objects of its own, those of the sheet it is a copy of, and a sheet that is gone has none.
    void sheetsChanged();
    void setEditsSource(EditsSource edits) { m_edits = std::move(edits); }
    void setBoxSource(BoxSource box) { m_box = std::move(box); }
    // Rows or columns were inserted or deleted (or that was undone): an object of ours is where the edits put it.
    void cellsMoved();

    // The sheet's objects as the layer has them (the sheet's are read if they have not been); `forPrint` is the same list (every object prints).
    std::vector<jp::grid::LayerObject> layerObjects(int sheet);
    std::optional<jp::grid::LayerObject> layerObjectOf(int sheet, int id);
    bool owns(int sheet, int id) const { return find(sheet, id) != nullptr; }
    bool isDeleted(int sheet, int id) const;
    // Not shown: taken away, or its cells were deleted.
    bool isGone(int sheet, int id) const;
    // The ids of the sheet's objects that are there, in the order they are drawn; and how many.
    std::vector<int> idsOf(int sheet);
    int countOf(int sheet);
    // The object as it is now (null: there is none), its anchor, and its name.
    std::shared_ptr<const jp::office::ole::EmbeddedObject> objectOf(int sheet, int id) const;
    jp::office::sheet::DrawingAnchor anchorOf(int sheet, int id) const;
    QString nameOf(int sheet, int id) const;
    // Its size in EMU as the screen has it now (its own when the screen does not say).
    void sizeOf(int sheet, int id, qint64 *widthEmu, qint64 *heightEmu) const;
    // Whether the object can be moved and sized (any that is there).
    bool canMove(int sheet, int id) const;

    // The state of an object as it is now (for the "before" of a step), the state with another anchor (a move or a size; `unitsNow` is the size it has there, in the grid's units, when known: an
    // object of ours given another size has its picture drawn again at it), and with other bytes and picture (an edit saved).
    ObjectState stateOf(int sheet, int id, int epoch) const;
    ObjectState stateMoved(int sheet, int id, const jp::office::sheet::DrawingAnchor &anchor, int epoch, const QSizeF &unitsNow = QSizeF()) const;
    ObjectState stateChanged(int sheet, int id, const jp::office::ole::EmbeddedObject &object, int epoch) const;
    // Gives the object a state (a step is done, undone, or done again). `placed`: the state's place is of the rows and columns now (not of before a save that followed edits to them).
    void apply(int sheet, int id, const EmbedState &state, bool deleted, bool placed);
    // Puts an object on a sheet. It is there but taken away until a step gives it the state with `deleted` false; discard() lets it go if the step is not made. The id, or -1 with the reason.
    int make(int sheet, const jp::office::ole::EmbeddedObject &object, const jp::office::sheet::DrawingAnchor &anchor, QString *why = nullptr);
    void discard(int sheet, int id);
    // A copy of the object, as another object of its own (its package and picture are copied, so that a change to one is no change to the other).
    std::optional<jp::office::ole::EmbeddedObject> copyOf(int sheet, int id) const;
    // How many objects the workbook has that are not taken away.
    int liveCount() const;
    // Whether the sheet's objects are held (false: it is over the size that is read through, and its objects stay in the file as they are).
    bool holdsObjectsOf(int sheet) const;

    // What saving needs: the objects taken away, moved, changed, or new, put in the changes the file is made from. The file was saved: what it holds is what the objects are now.
    void addToSave(jp::office::sheet::SaveChanges *changes);
    void saved();

Q_SIGNALS:
    // An object changed (a step, an edit saved): the layer draws it again.
    void changed(int sheet, int id);

private:
    struct Item {
        int id = 0;
        // (an object that is taken away and is not in the file is held by the steps of Undo that have it alone: `object` is let go of, and a step that gives it back gives it again)
        std::shared_ptr<const jp::office::ole::EmbeddedObject> object, fileObject;
        jp::office::sheet::DrawingAnchor baseAnchor, anchor, fileAnchor;
        int baseAt = 0;
        int fileIndex = -1;      // its place among the sheet's `oleObjects` in the file (-1: not in the file)
        QString objectRel, partName;   // what the file names its bytes by (the sheet's relationship, and the part), to find it again after a save
        QImage picture;          // its picture, unpacked once (on the window's thread)
        bool pictureRead = false;
        bool inFile = false;
        bool deleted = false;    // taken away by a person
        bool dropped = false;    // its cells were deleted
        QString name;
    };
    struct Sheet {
        bool listed = false;
        bool tooBig = false;     // its part is past what is read through: its objects are the file's
        std::vector<std::shared_ptr<Item>> items;
        int nextId = kEmbedIdBase;
    };
    Sheet &sheetState(int sheet);
    void list(int sheet, Sheet &s);
    void recompute(int sheet, Item &item);
    const Item *find(int sheet, int id) const;
    Item *find(int sheet, int id);
    jp::grid::LayerObject layerObject(int sheet, const std::shared_ptr<Item> &item);
    int editCount() const;
    void listAll();
    void copyFrom(int sheet, int source);
    void totals(int *live, qint64 *bytes) const;

    std::shared_ptr<jp::office::sheet::Workbook> m_book;
    EditsSource m_edits;
    BoxSource m_box;
    CopySource m_copySource;
    std::vector<Sheet> m_sheets;
    jp::office::sheet::OleReadState m_read;   // the objects of the sheets read so far, counted against one budget
};

} // namespace jp::crunch
