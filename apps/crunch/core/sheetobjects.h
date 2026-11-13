#pragma once
// The charts, pictures, and shapes of a workbook's sheets, as the window shows them (see libs/shell/grid/objectlayer.h for how they are put over the cells). A sheet's drawing is
// listed when the sheet is first shown (its anchors only: where each object is and what kind it is), and each object is made when a picture of it is first wanted: each chart through
// libs/office/office/chart.h, each picture, shape, text box, group, and connector through the drawing code the presentations use (libs/office/office/drawingml.h), so a drawing looks here
// as it does there; nothing is drawn a second way. A sheet of ten thousand shapes costs the shapes on its screen. A chart is drawn from the cells it reads, as they are now
// (libs/office/sheet/chartdata.h), and is drawn again when one of them changes: what it needs is taken on the window's thread, and the picture is made on a worker; before the workbook is
// whole, and for what it cannot resolve, the chart has the numbers its part stores.
//
// Nothing here changes the file: the drawing parts and chart parts stay as they were read, and saving writes them as they were (moved with the cells, the caches of a chart refreshed where
// its cells' values changed, and an object that was deleted taken out with what only it used: libs/office/sheet/xlsxsave.h and drawingsave.h).
//
// Bounded: the objects of all the workbook's drawings (kMaxWorkbookObjects), the anchors of a drawing, the shapes, texts, and paragraphs one workbook may keep (the budget of the drawing
// code), the points and series a chart keeps, the cells a chart is watched by, and the pictures and renders of the layer.

#include "core/document.h"
#include "core/objectclip.h"
#include "core/objectstate.h"
#include "grid/chartcells.h"
#include "grid/objectlayer.h"
#include "grid/valuestore.h"
#include "office/chart.h"
#include "office/diagram.h"
#include "office/drawingml.h"
#include "render/diagramdrawing.h"
#include "render/renderer.h"
#include "sheet/chartdata.h"
#include "sheet/drawing.h"
#include "sheet/sheetreader.h"
#include "sheet/xlsxsave.h"

#include <QJsonObject>
#include <QObject>
#include <QSet>
#include <QString>
#include <QStringList>

#include <functional>
#include <memory>
#include <vector>

namespace jp::crunch {

// The most diagrams of a workbook that are drawn from their drawings (a diagram that many frames show is drawn once for each size); the rest are boxes that say what they are.
constexpr int kMaxDiagramsDrawn = 200;
// And the most the pictures of their drawings come to, in bytes of SVG.
constexpr qint64 kMaxDiagramBytes = qint64(64) << 20;

// What a sheet's drawing holds and how much of it is shown, for the window's words and the yardsticks.
struct DrawingCounts {
    int anchors = 0;         // anchors in the drawing part
    int charts = 0, pictures = 0, shapes = 0, groups = 0, connectors = 0, diagrams = 0;   // objects read, by kind
    int shown = 0;           // objects that can be drawn
    int leftOut = 0;         // objects read that cannot be drawn (a chart that is not one, a picture that cannot be read, a group of nothing...)
    int hidden = 0;          // objects the file hides (they are not drawn)
    int skipped = 0;         // anchors that hold nothing that is read (tables, controls, ink)
    bool truncated = false;  // more anchors than may be read
    QStringList why;         // in words, each thing left out
    int total() const { return charts + pictures + shapes + groups + connectors + diagrams; }
};

// What the numbers, words, error-bar amounts, and field-button names a chart is drawn from come to (never 0): a chart is drawn again when this changes.
quint64 chartDataKey(const jp::office::Chart &chart);

class SheetObjects : public QObject {
    Q_OBJECT
public:
    // Where the structure edits that were made since the file was read or saved are kept (rows and columns inserted or deleted), in the order they were made.
    using EditsSource = std::function<const std::vector<jp::office::sheet::StructEdit> &()>;

    // Where the renames and deletes of sheets made since the file was read or saved are kept, in the order they were made (a chart's references follow them).
    using SheetOpsSource = std::function<const std::vector<jp::office::sheet::SheetOp> &()>;

    explicit SheetObjects(QObject *parent = nullptr);
    ~SheetObjects() override;

    // The workbook whose drawings are shown (none: nothing is). Everything is forgotten.
    void open(const std::shared_ptr<jp::office::sheet::Workbook> &book);
    // The workbook is whole: charts read its cells (`source` has the sheets, `values` the results worked out). Until then, and for what cannot be resolved, a chart has its stored numbers.
    void setCells(const jp::office::sheet::WorkbookSource *source, const jp::grid::ValueStore *values);
    void setEditsSource(EditsSource edits) { m_edits = std::move(edits); }
    void setSheetOpsSource(SheetOpsSource ops) { m_sheetOps = std::move(ops); }
    // Where the text of a cell is asked (as a barcode linked to the cell holds it).
    using CellTextSource = std::function<QString(int sheet, quint32 row, quint32 col)>;
    void setCellTextSource(CellTextSource text) { m_cellText = std::move(text); }

    jp::grid::ObjectLayer *layer() { return &m_layer; }
    // The cells charts read (null until the workbook is whole), and the theme the workbook's charts are set in.
    const jp::office::sheet::ChartCells *cells() const { return m_cells.get(); }
    const jp::office::Theme &theme();
    // Puts the sheet's objects in the layer (listing the sheet's drawing if that has not been done); the objects are made as their pictures are wanted. A drawing of more than
    // kParseApartBytes is parsed on a worker thread (a drawing of ten thousand shapes takes the better part of a second to parse): the layer has no objects until then, and `shownSheet`
    // is the sheet already.
    void show(int sheet);
    static constexpr qint64 kParseApartBytes = 256 * 1024;
    int shownSheet() const { return m_shown; }
    // The rows and columns of the sheet in view are as the file has them, whole (the first rows were swapped for the sheet read whole): an object that keeps its size when cells are resized
    // (editAs "oneCell" or "absolute") is measured now, so that it keeps what the file gave it.
    void geometryKnown(int sheet);
    // What the sheet's drawing holds (every object is made for it).
    DrawingCounts counts(int sheet);
    // The charts, pictures, and shapes of a sheet as a person would name them (those taken away are left out): what they are, their name, the words that tell what they show (alt text), and
    // the cell the top left of each is at. The sheet's drawing is listed if it has not been (an object of a drawing that is being read apart is not given).
    struct ObjectInfo {
        int id = 0;
        jp::office::sheet::ObjectKind kind = jp::office::sheet::ObjectKind::Other;
        QString name, description;
        quint32 row = 0, col = 0;
    };
    std::vector<ObjectInfo> describe(int sheet);
    // The objects of a sheet that print (not hidden, not taken away, and not set to stay off the paper), in the order they are drawn, each ready to be drawn when asked (see
    // core/printobjects.h). The sheet's drawing is listed if it has not been.
    std::vector<jp::grid::LayerObject> printObjects(int sheet);
    bool isChartSheet(int sheet) const;
    // Whether the object can be taken away: not when the file stores it with another (in one element), which would go with it; if not, why.
    bool canDelete(int sheet, int id, QString *why = nullptr);

    // ---- changes
    // Cells were changed (typed, calculated, or put back by Undo): a chart that reads one is drawn again.
    void cellsChanged(const std::vector<jp::grid::ValueStore::Item> &cells);
    // Rows or columns were inserted or deleted (or that was undone): the objects, and the charts' references, are where those edits put them. Only the edit that is new is worked in,
    // unless the list of edits is not the one before with one more at its end (an Undo): the objects are then worked out again from what they were.
    void cellsMoved();
    // Sheets were added, copied, renamed, or deleted (or that was undone): there is room for the sheets there are, and the charts' references name the sheets as they are called now (a
    // sheet that was deleted is `#REF!` in them, and a sheet given its name later is not the one they read).
    void sheetsChanged();
    // The sizes of rows or columns of the sheet changed (a hidden row is left out of a chart): charts are drawn again.
    void sizesChanged(int sheet);
    // An object is taken away (a person deleted it) or brought back (undone); the layer is told if its sheet is shown.
    void setDeleted(int sheet, int id, bool deleted);
    bool isDeleted(int sheet, int id);
    // What saving needs of the objects, put in the changes the file is made from: the objects taken away, the anchors of those that keep their size when cells are resized, and the caches
    // of the charts whose cells changed or whose references moved (the cells that changed are those the changes already hold). Charts are found in the parts of the file, so a sheet
    // that was never shown is not read for it.
    void addToSave(jp::office::sheet::SaveChanges *changes);
    // The file was saved: what it holds is what the objects are now (the edits made so far are in it, and the objects taken away are not).
    void saved();

    // ---- making and changing objects (the window makes each change one step of Undo: core/editsession.h, objectstate.h)
    // The state of an object as it is now (its chart is made if it has not been), to be the "before" of a step, or to be changed into its "after".
    ObjectState stateOf(int sheet, int id);
    // Gives the object a state (a step is done, undone, or done again); the layer is told if the sheet is shown.
    void applyState(int sheet, int id, const ObjectState &state);
    // What a workbook is let hold of what a person put there and has not saved: the objects (at most what one save writes) and the points of the charts made or changed (a chart's numbers are
    // kept as the model holds them, and Undo holds copies). A refusal says so in words; the limits are the same wherever an object is made.
    struct Limits {
        int maxAdded = jp::office::sheet::kMaxAddedObjects;
        qint64 maxPoints = 400000;
        qint64 maxBytes = jp::office::sheet::kMaxAddedBytes;   // the pictures pasted since the last save
    };
    // How a pivot chart made here is named in the file now (`[file]Sheet!Table`, from the table's place the chart keeps): asked at each save, so that a rename or a Save As is followed.
    // Fills in the names of the fields a pivot chart's buttons stand for, from the table the chart follows.
    void setPivotFielder(const std::function<void(jp::office::Chart *, int)> &fielder) { m_pivotFielder = fielder; }
    void setPivotNamer(const std::function<QString(const jp::office::Chart &)> &namer) { m_pivotNamer = namer; }
    void setLimits(const Limits &limits) { m_limits = limits; }
    const Limits &limits() const { return m_limits; }
    // The objects put there since the file was read or saved that are not taken away, and the points of the charts that are new or changed, in the whole workbook.
    int addedCount() const;
    qint64 chartPointsHeld() const;
    qint64 pastedBytesHeld() const;
    // The points a chart reads or holds (the cells its references name, up to what a series may read, and the numbers it holds for what has no reference).
    qint64 pointsOf(int sheet, const jp::office::Chart &chart) const;
    // Whether a chart of this size may be held in the place of the object `replacing` (-1: as one more); if not, why.
    bool chartRoom(int sheet, const jp::office::Chart &chart, int replacing, QString *why = nullptr) const;
    // Makes a chart object on a worksheet from the model (its references inside it, in the names the sheets have now). It is there but taken away (hidden) until a step gives it the state with
    // `deleted` false; discardMade() lets it go if the step is not made. The id, or -1 with the reason in `why`.
    // Whether a chart like `chart` can be put on a sheet now (the limits makeChart keeps), judged from `sheet`, one that is there; else why.
    bool canMakeChart(int sheet, const jp::office::Chart &chart, QString *why = nullptr);
    int makeChart(int sheet, const jp::office::Chart &chart, const jp::office::sheet::DrawingAnchor &anchor, QString *why = nullptr);
    void discardMade(int sheet, int id);
    // Copies a picture, shape, group, or connector into a clip (with the pictures it uses); false with the reason if it uses something that is not in the workbook. And puts one from a clip
    // on a sheet, as makeChart does a chart: hidden until a step gives it the state with `deleted` false.
    bool copyElement(int sheet, int id, jp::crunch::ObjectClip *clip, QString *why = nullptr);
    int makeElement(int sheet, const jp::crunch::ObjectClip &clip, const jp::office::sheet::DrawingAnchor &anchor, QString *why = nullptr);
    // The state with a changed chart (its references as they are now), for the step that changes it; and with the object put in another place.
    ObjectState stateWithChart(int sheet, int id, const jp::office::Chart &chart);
    // What it takes to write an object of the saved file back after a save has dropped it (null for an object that is not in the file, or when it holds too much); the object is told it.
    std::shared_ptr<const jp::crunch::ObjectRevive> captureRevive(int sheet, int id, QString *why = nullptr);
    // An object put on a sheet (a chart made from a copy) is written from the bytes of another's record, so the parts it had come with it.
    // An anchor given as a place and a size in points (an absolute one, with what it does when cells change as `editAs`) as two cells of this sheet: its own row heights and column widths say which.
    jp::office::sheet::DrawingAnchor anchorKeepingSize(int sheet, const jp::office::sheet::DrawingAnchor &absolute);
    void adoptRevive(int sheet, int id, const std::shared_ptr<const jp::crunch::ObjectRevive> &revive);
    ObjectState stateMoved(int sheet, int id, const jp::office::sheet::DrawingAnchor &anchor);
    // The state with the object on the printed pages or off them; and whether it is on them now.
    ObjectState stateWithPrints(int sheet, int id, bool prints);
    bool printsOf(int sheet, int id);
    // Whether the object's chart can be changed (it is a chart, the file stores it by itself, and it holds nothing a chart saved from the model would lose); if not, why.
    bool canEditChart(int sheet, int id, QString *why = nullptr, bool onChartSheet = false);   // (onChartSheet: a chart on a chart sheet counts, as for Move Chart)
    // An object's place now, as the layer has it (the grid's units), and the sheet's object under that name; false if there is none.
    bool exists(int sheet, int id);
    // The most bytes of the pictures that stand for equations (beside their math) one save writes. Past it, an equation is saved as its math alone, and `note` (if set) says so.
    static constexpr qint64 kMaxFallbackBytes = qint64(64) << 20;
    void setSaveNote(const std::function<void(const QString &)> &note) { m_saveNote = note; }
    // For a test: how many bytes of such pictures the last save made.
    qint64 fallbackBytesForTest() const { return m_fallbackBytes; }
    void setFallbackLimitForTest(qint64 bytes) { m_fallbackLimit = bytes; }
    // Whether the object is an equation (a text box whose text is math markup), and if so the line it was typed as (empty for markup that no line makes) and the markup (m:oMath or m:oMathPara).
    bool equationOf(int sheet, int id, QString *linear, QByteArray *markup);
    jp::office::sheet::ObjectKind kindOf(int sheet, int id);
    QString nameOf(int sheet, int id);
    QString descriptionOf(int sheet, int id);
    // The settings of an object that is a barcode of Insert > Barcode (a picture of the drawing that holds the marker, office/barcodemarker.h); empty for any other object.
    QJsonObject barcodeOf(int sheet, int id);
    // The barcodes of the sheets shown so far that are linked to a cell (the settings' `cell`): the sheet and id, the cell now (the sheet and place; `found` is false once the edits to rows and
    // columns have taken the cell away), the reference as it is written for the cell now, and the settings.
    struct LinkedBarcode {
        int sheet = 0, id = 0;
        bool found = false;
        int cellSheet = 0;
        quint32 row = 0, col = 0;
        QString ref;
        QJsonObject settings;
    };
    std::vector<LinkedBarcode> linkedBarcodes();
    // The same for one object (false if it is not there, taken away, or not linked).
    bool linkedBarcodeOf(int sheet, int id, LinkedBarcode *out);
    // The reference of the cell a barcode is linked to as it is now (empty for none).
    QString barcodeLinkOf(int sheet, int id);
    // What the program keeps of an object that is taken away so that Undo can bring it back (a barcode made here: its element, its pictures, and the drawing of it), in bytes; 0 for other objects.
    qint64 heldBytesOf(int sheet, int id);
    // The step that took the object away is forgotten: what was kept of it goes (the object stays in the list as a mark of its place, which other steps' places count).
    void releaseDead(int sheet, int id);
    static constexpr qint64 kHeldBytesOfAnObject = 400 * 1024;
    // The reference a barcode links to a cell by: the sheet's name quoted where it needs it, and the cell as $B$3.
    QString barcodeCellRef(int sheet, quint32 row, quint32 col) const;
    // The state of the object with a new name (empty: as it is) and alternative text.
    ObjectState stateWithNames(int sheet, int id, const QString &name, const QString &description);
    // Whether another object of the sheet has the name.
    bool nameTaken(int sheet, int id, const QString &name);
    // Where an object goes when it is brought forward (+1) or sent backward (-1), or to the front (2) or the back (-2): its place among the sheet's objects, or -1 for none (it is there already).
    int zTarget(int sheet, int id, int how);
    // Whether the object can be put in another place in the drawing (the file does not store it in one element with another).
    bool canReorder(int sheet, int id, QString *why = nullptr);
    // The ids of the sheet's objects that are not taken away, from the back.
    std::vector<int> idsOf(int sheet);
    // A name for a new object of a kind that no object of the sheet has ("Chart 3").
    QString newName(int sheet, jp::office::sheet::ObjectKind kind);
    // The state of an object that a pasted copy begins as: a new object like it (a chart of its model; another kind is not copied here) with the anchor given. -1 if it can't be.
    int duplicate(int sheet, int fromSheet, int fromId, const jp::office::sheet::DrawingAnchor &anchor, QString *why = nullptr);

    // For tests and tools: the chart of an object as it is drawn now (the numbers the cells give), or an empty chart.
    jp::office::Chart chartNow(int sheet, int objectId);
    // The cells a chart reads now (by the references it has now, after the edits made).
    QVector<jp::office::sheet::ChartBlock> chartReads(int sheet, int objectId);
    // How many references of a chart (its series' names, categories, and values, and its linked titles) there are, and how many of them name cells that can be found (the rest are drawn
    // from the numbers the part stores). Needs the cells.
    void chartRefCounts(int sheet, int objectId, int *refs, int *found, QStringList *missed = nullptr);
    // The anchor an object has now (as the screen places it).
    jp::office::sheet::DrawingAnchor anchorOf(int sheet, int objectId);
    // How many charts have been given to the worker, objects have been made, and parts of charts have been read (counted for tests).
    qint64 chartsDrawn() const { return m_chartsDrawn; }
    int objectsMade() const { return m_objectsMade; }
    // The objects that are there (made, and not taken away): what the limit on a workbook's objects counts.
    int liveObjects() const { return m_objectsMade - m_deadMade; }
    int chartPartsRead() const { return m_chartPartsRead; }
    // How many drawings were parsed on a worker thread (see show).
    int drawingsParsedApart() const { return m_parsedApart; }
    // How many diagrams were drawn from their drawings so far (at most kMaxDiagramsDrawn: the rest are boxes), and the SVG picture a diagram object was made as (empty for none).
    int diagramsDrawn() const { return m_diagramsDrawn; }
    // How many data parts of diagrams were inflated to find their drawings (at most kMaxDiagramsDrawn).
    int diagramPartsRead() const { return m_diagramPartsRead; }
    QByteArray diagramPicture(int sheet, int id);
    // How many edits of rows and columns have been worked into objects (one for each object and edit; an Undo works the edits since the file's again, for each object).
    qint64 editSteps() const { return m_editSteps; }
    // Whether the sheet's drawing has been read (listed), and how many pictures the drawing code left out for their size or kept (what it holds of them is the file's bytes, never decoded here).
    bool listed(int sheet) const { return sheet >= 0 && size_t(sheet) < m_sheets.size() && m_sheets[size_t(sheet)].listed; }
    int imagesLeftOut() const { return m_doc->imagesLeftOut; }
    int imagesKept() const { return int(m_doc->images.size()); }
    // The bytes of the pictures decoded and held now (a picture is decoded when it is drawn, and never past what the image limits allow).
    qint64 imageBytesDecoded() const
    {
        qint64 n = 0;
        for (auto it = m_doc->images.cbegin(); it != m_doc->images.cend(); ++it) n += it->cache.sizeInBytes();
        return n;
    }

private:
    struct Object {
        int id = 0;
        jp::office::sheet::DrawingObject meta;
        jp::office::sheet::DrawingAnchor fileAnchor;   // as the file has it (as it was read, or last saved)
        jp::office::sheet::DrawingAnchor base;         // as drawn before the edits since (an object that keeps its size, as one-cell or absolute)
        jp::office::sheet::DrawingAnchor anchor;       // with the edits made since
        jp::office::sheet::DrawingAnchor savedAnchor;  // what the file being written has
        bool frozen = false;                           // `base` is the size-keeping form
        bool built = false, made = false;
        bool released = false;                         // what was kept of it for Undo is let go (see releaseDead)
        bool provisional = false;                      // a barcode made with a stand-in PNG: the save makes the real one
        bool counted = false;                          // it is counted in m_objectsMade (and, while taken away, in m_deadMade)
        int linkState = -1;                            // a picture that holds a barcode marker with a cell: -1 not looked at, 0 no, 1 yes (found without making the picture)
        quint64 linkEpoch = 0;                         // the m_linkEpoch the cell below was found at
        bool linkFound = false;
        int linkSheet = 0;
        quint32 linkRow = 0, linkCol = 0;
        jp::ItemPtr item;                              // a picture, shape, group, or connector
        std::shared_ptr<const jp::office::Chart> chart;   // a chart as read from its part, shared by the objects of that part
        QStringList baseRefs, movedRefs, refs;         // its references as the file has them, as the edits to rows and columns leave them (still by the file's names of the sheets), and now (by the names the sheets have)
        jp::office::sheet::ChartReadIndex reads;       // the cells it read when last given to the worker
        quint64 drawnKey = 0;                          // what the numbers and words it was last given to draw come to (0: it has not been given)
        bool deleted = false;                          // taken away by a person (not yet saved)
        bool added = false;                            // put there by a person: the file does not have it yet
        std::vector<jp::crunch::AnchorPlacement> placements;   // where a person put it (`base` is the file's; `anchor` is what they come to with the edits)
        std::vector<jp::crunch::RefPlacement> refPlacements;   // what a person made the chart read
        std::shared_ptr<const jp::office::Chart> fileChart;   // the chart as the file's part gave it (a chart that is another was changed by a person)
        QString savedPart;                             // what saving makes for it: the chart part's name
        QString diagramKey;                            // for a diagram: the picture made of it (m_diagramPictures)
        qint64 points = 0;                             // what a chart that is new or changed holds, for the limits
        std::shared_ptr<QDomDocument> ownDoc;          // for a picture or shape put here from a copy: its element's own document, the package that has the pictures it names, how it is read, and its XML to be written
        std::shared_ptr<jp::office::Package> ownPackage;
        std::shared_ptr<jp::office::DrawingContext> ownContext;
        QByteArray pasteXml, pasteNamespaces;
        QList<jp::crunch::ClipMedia> pasteMedia;
        QString savedPivotName;                        // the name the chart's pivot source has in the file being written
        QString fileDescription, savedDescription;     // its alternative text as the file has it, and as the save in flight writes it
        QString fileName, savedName;                   // its name as the file has it, and as the save in flight writes it
        bool madeHere = false;                         // a person made it here (a chart, a pasted picture or shape): after a save has dropped it, it is written again from what is held of it
        std::weak_ptr<const jp::crunch::ObjectRevive> revive;   // what writes an object of the file back (held by the step that took it away)
        std::shared_ptr<const jp::crunch::ObjectRevive> reviveOwn;   // held while the object is back and not yet saved (the steps that hold the record may be given up by a new edit)
        bool filePrints = true, savedPrints = true;    // whether the file has it printing, and whether the save in flight writes it so
        int savedOrdinal = -1;                         // where it will be among the children of the drawing's root when what addToSave gave is saved
        int editable = -1;                             // whether the chart can be changed (-1 not looked at, 0 no, 1 yes), and why not: found once, for a part of the file is read to say
        QString editWhy;
        bool gone = false;                             // taken away and the file written without it (it keeps its place in the list, so that the places Undo holds stay)
    };
    struct Sheet {
        bool listed = false;
        bool parsing = false;                          // its drawing part is being parsed on a worker
        bool showPending = false;                      // and the sheet is to be shown when it is
        std::shared_ptr<QDomDocument> parsedDoc;       // what the worker made, until the drawing is listed
        bool chartSheet = false;
        QString drawingPart;
        jp::office::sheet::SheetDrawing drawing;
        std::vector<std::shared_ptr<Object>> objects;  // those that are not hidden, in the order of the file
        std::shared_ptr<jp::office::DrawingContext> context;
        QHash<QString, std::shared_ptr<const jp::office::Chart>> parsed;   // the charts read, by part (the part is read once, whatever number of anchors name it)
        QHash<QString, QString> failedParts;           // chart parts that could not be read, and why
        QHash<QString, QString> diagramParts;          // the drawing part each diagram data link leads to (empty: the file keeps none)
        DrawingCounts counts;
        int nextId = 0;                                // the id of the next object put on the sheet
        qint64 nextShapeId = 2;                        // and the number the next one has in the drawing
        bool shapeIdsKnown = false;                    // `nextShapeId` is past every id the drawing part has (found by looking through the whole part)
        QString savedDrawing;                          // the drawing part saving makes for a sheet that has none
    };
    Sheet &sheetState(int sheet);
    void list(int sheet, Sheet &s);
    // The sheet's objects into the layer, its drawing listed.
    void showNow(int sheet);
    // A large drawing is parsed on a worker (true): `parsed` is told when it is.
    bool parseApart(int sheet, Sheet &s);
    void parsed(int sheet, quint64 generation, const std::shared_ptr<QDomDocument> &doc, const QString &why);
    void ensureBuilt(int sheet, Object &o);
    jp::ItemPtr diagramItem(Sheet &s, Object &o);
    void readContext();
    std::function<QImage()> prepareChart(int sheet, Object &o, const jp::grid::RenderRequest &q);
    QImage renderItem(Object &o, const jp::grid::RenderRequest &q);
    jp::grid::LayerObject layerObject(int sheet, const std::shared_ptr<Object> &o);
    void applyEdit(int sheet, Object &o, const jp::office::sheet::StructEdit &edit);
    void replay(int sheet, Object &o);
    // The names the file has for the sheets (what the edits to rows and columns, and the references of the file, name them by); a sheet that was added has none.
    QStringList sheetNames() const;
    // A reference after the renames and deletes of sheets made since the file was read or saved.
    QString followSheetOps(const QString &ref) const;
    void refreshRefs(Object &o) const;
    // The references the chart parts of the package read, by part (read once; a save makes them stale).
    const QStringList &partRefs(const QString &part);
    // Whether the chart, with the references it has now, would be drawn from other numbers and words than it was last given (`budget` is what is left to read for the answer: past it, yes).
    bool chartDataChanged(int sheet, Object &o, qint64 *budget);
    jp::office::sheet::DrawingAnchor twoCellFor(const Object &o, const jp::grid::Axis &rows, const jp::grid::Axis &cols) const;
    Object *find(int sheet, int id);
    void knowShapeIds(int sheet, Sheet &s);
    std::shared_ptr<jp::office::DrawingContext> baseContext();
    std::vector<jp::office::sheet::StructEdit> editsFrom(int at, int to = -1) const;
    // A barcode that holds the cell it was made from (the settings' `cell`) reads it as a chart reads its cells: the reference follows the edits to rows and columns.
    void linkBarcode(int sheet, Object &o, bool made);
    // Whether the object is a picture that holds a barcode marker with a `cell` (read from its element: the picture is not made to find out).
    bool mayBeLinked(Object &o);
    // Writes the barcode of the object again, with the value of its cell and the reference to it as they are, in what the save is made of (see addToSave).
    void saveBarcode(int sheet, Sheet &s, Object &o, jp::office::sheet::SaveChanges *changes, QSet<QString> *claimed);
    QStringList movedRefList(const QStringList &refs, int sheet, int at, int to) const;
    // A reference as the file's names for the sheets would have it (the renames and deletes of sheets since the file was read or saved taken back).
    QString unfollowSheetOps(const QString &ref) const;

    std::shared_ptr<jp::office::sheet::Workbook> m_book;
    const jp::office::sheet::WorkbookSource *m_source = nullptr;
    const jp::grid::ValueStore *m_values = nullptr;
    std::unique_ptr<jp::grid::WorkbookChartCells> m_cells;
    EditsSource m_edits;
    SheetOpsSource m_sheetOps;
    CellTextSource m_cellText;
    std::vector<jp::office::sheet::StructEdit> m_applied;   // the edits the objects have had worked in
    std::vector<Sheet> m_sheets;
    QHash<QString, int> m_drawingOwner;                      // the first sheet that names each drawing part (a file whose sheets share a drawing shows it on the first), lower case
    QHash<QString, int> m_chartSheet;                        // the sheet each chart part is on (by the drawing of the sheet), lower case
    QHash<QString, QStringList> m_partRefs;
    jp::grid::ObjectLayer m_layer;
    std::unique_ptr<jp::Document> m_doc;
    jp::LayoutCache m_layoutCache;
    QHash<QString, QString> m_images;
    // The pictures made of diagrams, by drawing and size (a diagram that many frames show is drawn once), and what has been drawn.
    struct DiagramPicture {
        QString imageId;
        QByteArray svg;
        QSizeF size;
    };
    QHash<QString, DiagramPicture> m_diagramPictures;
    int m_diagramsDrawn = 0, m_diagramPartsRead = 0;
    qint64 m_diagramBytes = 0, m_diagramDataBytes = 0;
    qint64 m_fallbackBytes = 0, m_fallbackLimit = kMaxFallbackBytes;   // the bytes of the pictures that stand for equations in the save being made
    std::function<void(const QString &)> m_saveNote;
    jp::office::DrawingBudget m_budget;
    jp::office::Theme m_theme;
    QDomDocument m_themeDoc;
    QDomElement m_formats;
    bool m_contextRead = false;
    int m_shown = -1;
    int m_objectsMade = 0;
    int m_deadMade = 0;                                      // of those, the ones taken away (a barcode that was swapped for another gives its slot back)
    quint64 m_linkEpoch = 1;                                 // changes when rows, columns, or sheets move: the cells barcodes are linked to are found again
    int m_chartPartsRead = 0;
    int m_parsedApart = 0;
    quint64 m_generation = 0;                                // of the workbook open (a worker's answer for another is let go)
    qint64 m_chartsDrawn = 0;
    qint64 m_editSteps = 0;
    Limits m_limits;
    std::function<QString(const jp::office::Chart &)> m_pivotNamer;
    std::function<void(jp::office::Chart *, int)> m_pivotFielder;   // (the chart, and the sheet it is on)
    int m_epoch = 0;                                         // counts the saves that followed edits to rows and columns (see core/objectstate.h)
};

} // namespace jp::crunch
