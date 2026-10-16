#pragma once
// Tables in the flow of a document: finding them in the story, laying them out, and drawing them. A table is the paragraphs of its cells (see
// tabledata.h); here those are grouped into rows and cells, the cells are laid out with the text engine, each in a frame as wide as the cell, and
// the rows are given their heights. The paginator treats a table as a block of rows that it places down the page like lines and breaks between,
// and a page that has some of a table's rows makes a TableFragment of them to draw.

#include "flow/flowdocument.h"
#include "flow/tabledata.h"

#include <QHash>
#include <QLineF>
#include <QPainter>
#include <QPointF>
#include <QRectF>
#include <memory>
#include <vector>

class QTextDocument;

namespace jp::flow {

// A paragraph of a table, where it is, and which cell it is in.
struct BlockRef {
    int pos = 0, end = 0;       // the paragraph's first position, and the one after it (the next paragraph's first)
    int key = 0;                // its cell's key
};

// The tables of a story at one moment: each outermost table as a stretch of paragraphs. (Built by looking at every paragraph's format once; the paginator
// builds it again when the story has been edited.)
class TableIndex {
public:
    struct Span {
        int from = 0, to = 0;               // positions: the table's first paragraph's first, and just after its last
        int first = 0, last = 0;            // its paragraphs: refs()[first, last)
        std::shared_ptr<const TableData> table;
    };
    void rebuild(const QTextDocument *story, const TableStore &store);
    const std::vector<Span> &spans() const { return m_spans; }
    const std::vector<BlockRef> &refs() const { return m_refs; }
    // The table whose paragraphs hold the position (an index into spans(), -1 for none).
    int spanAt(int pos) const;
    // The first table that starts at the position or after it (spans().size() for none).
    int firstFrom(int pos) const;
    // The cells a paragraph is in, the outermost first: the table's own cell, and for a nested table the cells it is in.
    const QVector<const CellRecord *> &chain(int key) const;

private:
    std::vector<Span> m_spans;
    std::vector<BlockRef> m_refs;
    const TableStore *m_store = nullptr;
    mutable QHash<int, QVector<const CellRecord *>> m_chains;
};

class TableFlow;
class TableLayout;
struct TableFragment;

// What is in a cell, laid out: stretches of paragraphs and tables, one under the other, from the top of the cell's text area.
struct CellContent {
    double height = 0;
    struct Part {
        double y = 0;
        std::shared_ptr<StoryLayout> text;      // a stretch of paragraphs (its lines are frame 0, from the top of the part)
        int from = 0, to = 0;                   // its document positions
        std::shared_ptr<TableFragment> table;   // or a table inside the cell, whole
    };
    std::vector<Part> parts;
};

// Where the layout of a table finds the story and the index, while the story is as it was when they were made; after an edit a layout keeps the
// heights it has and measures no more.
struct TableContext {
    struct Live { quint64 epoch = 0; };
    FlowDocument *doc = nullptr;
    const TableIndex *index = nullptr;
    std::shared_ptr<Live> live;
    quint64 at = 0;
    bool valid() const { return live && live->epoch == at; }
};

// The shape of a table: its columns and rows, and where each cell is. (The heights of rows are worked out as they are asked for, in order: a table of
// thousands of rows is only measured as far as the pages that are looked at need.)
class TableLayout {
public:
    struct Cell {
        const CellRecord *rec = nullptr;
        int row = 0, col = 0, span = 1;     // where it is in the grid
        int rows = 1;                       // the rows it covers (more than one for the first of cells joined down)
        bool covered = false;               // a cell that continues the one above: it is drawn as part of that one
        int r0 = 0, r1 = 0;                 // its paragraphs: the index's refs()[r0, r1)
        int from = 0, to = 0;               // their positions
        double x = 0, w = 0;                // from the table's left edge, as wide as the columns it covers
        double need = -1;                   // the height its content needs with its margins (-1: not measured)
        QMarginsF margins;
    };
    struct Row {
        const RowData *data = nullptr;
        int cell0 = 0, cells = 0;           // its cells: cells()[cell0, cell0 + cells)
        int from = 0, to = 0;               // positions
        double height = 0;
    };

    // The table whose paragraphs are the index's refs()[i0, i1), its cells `depth` cells deep (0 for a table that is not in a cell).
    static std::shared_ptr<TableLayout> make(const TableContext &ctx, int i0, int i1, int depth);

    int rowCount() const { return int(m_rows.size()); }
    const Row &row(int r) const { return m_rows[size_t(r)]; }
    const Cell &cell(int i) const { return m_cells[size_t(i)]; }
    int cellCount() const { return int(m_cells.size()); }
    const TableData &data() const { return *m_data; }
    double width() const { return m_width; }                // all the columns
    int depth() const { return m_depth; }
    // The row that has the position (the last, for one after them all; -1 for a table with none).
    int rowOfPosition(int pos) const;
    // The cell that has the position as one of its own paragraphs' (an index into the cells; -1 for none).
    int cellOfPosition(int pos) const;
    // The heights of the rows up to and including `r`, measured if they have not been (each row is measured after the ones before it).
    void measure(int r);
    double rowHeight(int r) { measure(r); return m_rows[size_t(r)].height; }
    // How far down the top of row `r` is from the top of row `from`.
    double rowOffset(int from, int r);
    // How many rows at the top repeat on the pages the table goes on to, and how tall they are.
    int headerRows() const { return m_headers; }
    double headerHeight();
    // The place of the table's left edge from the left of a text area of the width it is set in.
    double leftEdgeIn(double area) const;
    // The text area a cell's content is laid out in, from the table's left edge: the cell without its margins.
    double innerWidth(const Cell &c) const;
    // The cell's content, laid out.
    std::shared_ptr<CellContent> contentOf(const Cell &c) const;

    // A row that goes over a page break is shown a part at a time: each cell's lines that fit the page, and the rest on the next, as a word processor sets a row that
    // may be split. A part is told by its state: 0 for the row whole or the part at its top, else the index of the part (1 and up) with the room the first part
    // had (in twentieths of a point, above 10 bits), from which the parts before it are found again.
    struct Piece {
        double height = 0;                  // as tall as its room when more of the row comes after it, else as the cells that are left need
        QVector<double> offsets;            // for each cell of the row: how far down its content the part begins (from the top of the cell's text area)
        bool progress = false;              // some cell has a line in the part (without, the row is not split there: it goes to the next page whole)
        int next = 0;                       // the state of the part after it (0: the row ends in this one)
    };
    static int pieceState(int index, double firstRoom) { return (index & 0x3FF) | (int(std::lround(firstRoom * 20)) << 10); }
    static int pieceIndex(int state) { return state & 0x3FF; }
    static double pieceFirstRoom(int state) { return (state >> 10) / 20.0; }
    // Whether row `r` can go over a page break: not one that says it can't, one of an exact height, or one with cells joined down in it (or covered by those).
    bool splittable(int r) const;
    // The part of row `r` that begins at `state`, when `room` is the most it may take (state 0) and `fullRoom` what each later part may (a page less the
    // header rows). A part at the top of a page is given a line of each cell even if none fits (`force`), so that a row taller than a page ends.
    Piece pieceOf(int r, int state, double room, double fullRoom, bool force);

private:
    friend class TableFlow;
    TableLayout() = default;
    double measureCell(Cell &c);
    std::shared_ptr<const TableData> m_data;
    std::vector<Row> m_rows;
    std::vector<Cell> m_cells;
    QVector<double> m_xs;
    double m_width = 0;
    int m_headers = 0;
    int m_measured = 0;                     // the rows before this one have their heights
    int m_depth = 0;
    int m_maxRows = 1;                      // the most rows a cell covers
    TableContext m_ctx;
    struct Units {
        std::vector<std::pair<double, double>> lines;   // the top and bottom of each line of a cell's content (a table in it as one), from the top of its text area
        double height = 0;
    };
    std::shared_ptr<const Units> unitsOf(int cell);
    QHash<int, std::shared_ptr<const Units>> m_units;
};

// The cells of paragraphs [i0, i1) of the index laid out as the content of a cell `direct` cells deep, at a width (the paragraphs of the cell itself and
// the tables in it, one under the other).
std::shared_ptr<CellContent> layoutCellContent(const TableContext &ctx, int i0, int i1, int direct, double width);

// How a table is drawn.
struct TablePaint {
    TextOutput output = TextOutput::Picture;
    bool marks = false;                     // paragraph marks and the like
    int selFrom = -1, selTo = -1;           // the selected text, as document positions
};

// Some rows of a table, with what is in their cells laid out, ready to draw at a place on a page.
struct TableFragment {
    std::shared_ptr<TableLayout> layout;
    int firstRow = 0, endRow = 0;           // the rows [firstRow, endRow) it shows
    bool header = false;                    // with the table's header rows above them
    QPointF origin;                         // where the table's left edge and the top of the fragment are (page coordinates)
    double height = 0;                      // all of it, the header rows too
    struct Item {
        int cell = 0;                       // an index into the layout's cells
        std::shared_ptr<CellContent> content;
    };
    std::vector<Item> items;                // the cells that show
    // Rows that are shown in part: the row, and the part (see TableLayout::Piece).
    struct Part {
        int row = 0;
        TableLayout::Piece piece;
    };
    std::vector<Part> parts;

    QRectF rect() const { return QRectF(origin, QSizeF(layout->width(), height)); }
    // The part of a row this shows (null for a row whole), and how tall the row is here.
    const Part *partOf(int row) const;
    double heightOf(int row) const;
    void paint(QPainter *p, const TablePaint &o) const;
    // The text caret for a position in one of its cells: the line, in page coordinates. False when none of its cells has the position.
    bool caretLine(int pos, QLineF *line) const;
    // The position nearest a page point, in the cell the point is in or the nearest one (-1: the fragment has no cell with text).
    int positionAt(const QPointF &point) const;
    bool contains(int pos) const;
    // The page rectangle of the cell that has the position (empty for none).
    QRectF cellRect(int pos) const;

private:
    struct Placed {
        int item = 0;
        QRectF rect;                        // page coordinates
        QRectF clip;                        // the part of it on the page (a cell joined down may reach past)
        QPointF text;                       // where the cell's text area begins
    };
    std::vector<Placed> place() const;
};

class TableFlow {
public:
    explicit TableFlow(FlowDocument *doc);
    ~TableFlow();
    // Whether the story has tables at all (what the paginator asks before anything else, so a document without them is laid out as before).
    bool any() const { return !m_doc->tables.isEmpty(); }
    // The story was edited: what was found of it, and the tables laid out, are gone.
    void edited();
    const TableIndex &index();
    TableContext context();
    // The table at index `span` of the index, laid out (the same one again while the story is as it was).
    std::shared_ptr<TableLayout> layout(int span);
    // A fragment of rows [first, end) of a table, with the cells it shows laid out; `parts` are those of its rows that it shows only a part of.
    std::shared_ptr<TableFragment> fragment(const std::shared_ptr<TableLayout> &layout, int firstRow, int endRow, const std::vector<TableFragment::Part> &parts = {});
    // Whether a page that starts with row `first` of the table has the header rows above it (it does when the table has gone on from earlier pages).
    static bool repeatsHeader(const TableLayout &t, int first) { return t.headerRows() > 0 && first >= t.headerRows(); }

private:
    FlowDocument *m_doc;
    TableIndex m_index;
    bool m_valid = false;
    std::shared_ptr<TableContext::Live> m_live;
    QHash<int, std::shared_ptr<TableLayout>> m_layouts;
};

} // namespace jp::flow
