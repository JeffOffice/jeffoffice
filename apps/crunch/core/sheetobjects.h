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
#include "grid/chartcells.h"
#include "grid/objectlayer.h"
#include "grid/valuestore.h"
#include "office/chart.h"
#include "office/drawingml.h"
#include "render/renderer.h"
#include "sheet/chartdata.h"
#include "sheet/drawing.h"
#include "sheet/sheetreader.h"
#include "sheet/xlsxsave.h"

#include <QObject>
#include <QString>
#include <QStringList>

#include <functional>
#include <memory>
#include <vector>

namespace jp::crunch {

// What a sheet's drawing holds and how much of it is shown, for the window's words and the yardsticks.
struct DrawingCounts {
    int anchors = 0;         // anchors in the drawing part
    int charts = 0, pictures = 0, shapes = 0, groups = 0, connectors = 0;   // objects read, by kind
    int shown = 0;           // objects that can be drawn
    int leftOut = 0;         // objects read that cannot be drawn (a chart that is not one, a picture that cannot be read, a group of nothing...)
    int hidden = 0;          // objects the file hides (they are not drawn)
    int skipped = 0;         // anchors that hold nothing that is read (tables, controls, ink)
    bool truncated = false;  // more anchors than may be read
    QStringList why;         // in words, each thing left out
    int total() const { return charts + pictures + shapes + groups + connectors; }
};

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

    jp::grid::ObjectLayer *layer() { return &m_layer; }
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
    int chartPartsRead() const { return m_chartPartsRead; }
    // How many drawings were parsed on a worker thread (see show).
    int drawingsParsedApart() const { return m_parsedApart; }
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
        jp::ItemPtr item;                              // a picture, shape, group, or connector
        std::shared_ptr<const jp::office::Chart> chart;   // a chart as read from its part, shared by the objects of that part
        QStringList baseRefs, movedRefs, refs;         // its references as the file has them, as the edits to rows and columns leave them (still by the file's names of the sheets), and now (by the names the sheets have)
        jp::office::sheet::ChartReadIndex reads;       // the cells it read when last given to the worker
        quint64 drawnKey = 0;                          // what the numbers and words it was last given to draw come to (0: it has not been given)
        bool deleted = false;                          // taken away by a person (not yet saved)
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
        DrawingCounts counts;
    };
    Sheet &sheetState(int sheet);
    void list(int sheet, Sheet &s);
    // The sheet's objects into the layer, its drawing listed.
    void showNow(int sheet);
    // A large drawing is parsed on a worker (true): `parsed` is told when it is.
    bool parseApart(int sheet, Sheet &s);
    void parsed(int sheet, quint64 generation, const std::shared_ptr<QDomDocument> &doc, const QString &why);
    void ensureBuilt(int sheet, Object &o);
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

    std::shared_ptr<jp::office::sheet::Workbook> m_book;
    const jp::office::sheet::WorkbookSource *m_source = nullptr;
    const jp::grid::ValueStore *m_values = nullptr;
    std::unique_ptr<jp::grid::WorkbookChartCells> m_cells;
    EditsSource m_edits;
    SheetOpsSource m_sheetOps;
    std::vector<jp::office::sheet::StructEdit> m_applied;   // the edits the objects have had worked in
    std::vector<Sheet> m_sheets;
    QHash<QString, int> m_chartSheet;                        // the sheet each chart part is on (by the drawing of the sheet), lower case
    QHash<QString, QStringList> m_partRefs;
    jp::grid::ObjectLayer m_layer;
    std::unique_ptr<jp::Document> m_doc;
    jp::LayoutCache m_layoutCache;
    QHash<QString, QString> m_images;
    jp::office::DrawingBudget m_budget;
    jp::office::Theme m_theme;
    QDomDocument m_themeDoc;
    QDomElement m_formats;
    bool m_contextRead = false;
    int m_shown = -1;
    int m_objectsMade = 0;
    int m_chartPartsRead = 0;
    int m_parsedApart = 0;
    quint64 m_generation = 0;                                // of the workbook open (a worker's answer for another is let go)
    qint64 m_chartsDrawn = 0;
    qint64 m_editSteps = 0;
};

} // namespace jp::crunch
