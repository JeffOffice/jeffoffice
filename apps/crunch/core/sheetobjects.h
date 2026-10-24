#pragma once
// The charts, pictures, and shapes of a workbook's sheets, as the window shows them (see libs/shell/grid/objectlayer.h for how they are put over the cells). A sheet's drawing is
// read when the sheet is first shown: each chart through libs/office/office/chart.h, each picture, shape, text box, group, and connector through the drawing code the presentations
// use (libs/office/office/drawingml.h), so a drawing looks here as it does there; nothing is drawn a second way. A chart is drawn from the cells it reads, as they are now
// (libs/office/sheet/chartdata.h), and is drawn again when one of them changes; before the workbook is whole, and for what it cannot resolve, from the numbers the chart part stores.
//
// Nothing here changes the file: the drawing parts and chart parts stay as they were read, and saving writes them as they were (moved with the cells, and with the caches of a chart
// refreshed where its cells' values changed: libs/office/sheet/xlsxsave.h).
//
// Bounded: the objects of all the workbook's drawings (kMaxWorkbookObjects), the anchors of a drawing, the shapes, texts, and paragraphs one workbook may keep (the budget of the drawing
// code), the points and series a chart keeps, and the pictures and renders of the layer.

#include "core/document.h"
#include "grid/chartcells.h"
#include "grid/objectlayer.h"
#include "grid/valuestore.h"
#include "office/chart.h"
#include "office/drawingml.h"
#include "render/renderer.h"
#include "sheet/chartdata.h"
#include "sheet/drawing.h"
#include "sheet/xlsxsave.h"
#include "sheet/sheetreader.h"

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
    explicit SheetObjects(QObject *parent = nullptr);
    ~SheetObjects() override;

    // The workbook whose drawings are shown. Everything is forgotten.
    void open(const std::shared_ptr<jp::office::sheet::Workbook> &book);
    // The workbook is whole: charts read its cells (`source` has the sheets, `values` the results worked out). Until then, and for what cannot be resolved, a chart has its stored numbers.
    void setCells(const jp::office::sheet::WorkbookSource *source, const jp::grid::ValueStore *values);
    // Where the structure edits that were made since the file was read or saved are kept (rows and columns inserted or deleted), asked for when objects are made or moved.
    void setEditsSource(std::function<std::vector<jp::office::sheet::StructEdit>()> edits) { m_edits = std::move(edits); }

    jp::grid::ObjectLayer *layer() { return &m_layer; }
    // Puts the sheet's objects in the layer (reading the sheet's drawing if that has not been done).
    void show(int sheet);
    int shownSheet() const { return m_shown; }
    // What the sheet's drawing holds (reads it if need be).
    DrawingCounts counts(int sheet);
    bool isChartSheet(int sheet) const;

    // ---- changes
    // Cells were changed (typed, calculated, or put back by Undo): a chart that reads one is drawn again.
    void cellsChanged(const std::vector<jp::grid::ValueStore::Item> &cells);
    // Rows or columns were inserted or deleted (or that was undone): the objects, and the charts' references, are where those edits put them.
    void cellsMoved();
    // The sizes of rows or columns of the sheet changed (a hidden row is left out of a chart): charts are drawn again.
    void sizesChanged(int sheet);
    // An object is taken away (a person deleted it) or brought back (undone); the layer is told if its sheet is shown.
    void setDeleted(int sheet, int id, bool deleted);
    bool isDeleted(int sheet, int id);
    // What saving needs of the objects, put in the changes the file is made from: the objects taken away, and the caches of the charts whose cells changed or whose references moved
    // (those the changes already hold are the cells that changed). Every sheet's drawing is read for it.
    void addToSave(jp::office::sheet::SaveChanges *changes);
    // The file was saved: what it holds is what the objects are now (the edits made so far are in it, and the objects taken away are not).
    void saved();

    // For tests and tools: the chart of an object as it is drawn now (the numbers the cells give), or an empty chart.
    jp::office::Chart chartNow(int sheet, int objectId);
    // The cells a chart reads now (by the references it has now, after the edits made).
    QVector<jp::office::sheet::ChartBlock> chartReads(int sheet, int objectId);
    // How many charts have been drawn from the cells, and how many objects were made (counted for tests).
    qint64 chartsDrawn() const { return m_chartsDrawn; }

private:
    struct Object {
        int id = 0;
        jp::office::sheet::DrawingObject meta;
        jp::office::sheet::DrawingAnchor base;     // as the file has it (now)
        jp::office::sheet::DrawingAnchor anchor;   // with the edits made since
        jp::ItemPtr item;                          // a picture, shape, group, or connector
        std::shared_ptr<jp::office::Chart> baseChart;   // a chart as read from the file (now)
        std::shared_ptr<jp::office::Chart> chart;       // with the edits made since
        QVector<jp::office::sheet::ChartBlock> reads;   // what it read when it was last drawn
        QHash<int, jp::office::sheet::CellRange> hull;  // and the smallest block of each sheet that holds those
        bool made = false;                         // can be drawn
        bool deleted = false;                      // taken away by a person (not yet saved)
    };
    struct Sheet {
        bool built = false;
        bool chartSheet = false;
        QString drawingPart;
        jp::office::sheet::SheetDrawing drawing;
        std::vector<std::shared_ptr<Object>> objects;
        DrawingCounts counts;
    };
    Sheet &sheetState(int sheet);
    void build(int sheet, Sheet &s);
    std::shared_ptr<Object> buildObject(Sheet &s, const jp::office::sheet::DrawingObject &d, const jp::office::DrawingContext &ctx);
    void readContext();
    QImage renderChart(int sheet, Object &o, const jp::grid::RenderRequest &q);
    QImage renderItem(Object &o, const jp::grid::RenderRequest &q);
    void remember(Object &o, const jp::office::Chart &drawn);
    jp::grid::LayerObject layerObject(int sheet, const std::shared_ptr<Object> &o);
    void applyEdits(int sheet, Sheet &s);
    QStringList sheetNames() const;

    std::shared_ptr<jp::office::sheet::Workbook> m_book;
    const jp::office::sheet::WorkbookSource *m_source = nullptr;
    const jp::grid::ValueStore *m_values = nullptr;
    std::unique_ptr<jp::grid::WorkbookChartCells> m_cells;
    std::function<std::vector<jp::office::sheet::StructEdit>()> m_edits;
    std::vector<Sheet> m_sheets;
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
    qint64 m_chartsDrawn = 0;
};

} // namespace jp::crunch
