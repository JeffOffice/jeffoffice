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
#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <vector>

class QTextDocument;

namespace jp::flow {

// A paragraph of a table, where it is, and which cell it is in.
struct BlockRef {
    int pos = 0, end = 0;       // the paragraph's first position, and the one after it (the next paragraph's first)
    int key = 0;                // its cell's key
    int foot = 0, endNotes = 0; // the footnotes and the endnotes in the story before the paragraph (counted only where the document has notes)
    int notes = 0;              // the footnotes the paragraph cites
};

// A footnote cited in a table's cell: where its reference is, which note, and the number it shows.
struct NoteRef {
    int pos = 0;
    QString id;
    int number = 0;
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
    // `doc` has the notes' numbers: where it has notes (FlowDocument::fields), each paragraph is given how many are before it.
    void rebuild(const QTextDocument *story, const TableStore &store, const FlowDocument *doc = nullptr);
    const std::vector<Span> &spans() const { return m_spans; }
    const std::vector<BlockRef> &refs() const { return m_refs; }
    // The table whose paragraphs hold the position (an index into spans(), -1 for none).
    int spanAt(int pos) const;
    // The first table that starts at the position or after it (spans().size() for none).
    int firstFrom(int pos) const;
    // The cells a paragraph is in, the outermost first: the table's own cell, and for a nested table the cells it is in.
    const QVector<const CellRecord *> &chain(int key) const;
    // An edit in the story at `position` that took out `removed` characters and put in `added`, where it is not in a table or beside one: the tables' places move
    // with the text. False (nothing done) when it touches a table, which is to be found again.
    bool shift(int position, int removed, int added);

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
        int foot = 0, end = 0;                  // the footnotes and endnotes before it, in the story
        QVector<NoteRef> notes;                 // the footnotes its text cites, in order
    };
    std::vector<Part> parts;
};

// The cells that were laid out lately, so that a cell is not laid out again for each use of it (to measure its row, to find where a row may be cut, to draw it): the most
// lately used are kept, as many as hold so many paragraphs, and all are dropped when the story is edited (their places in it have moved).
class CellContentCache {
public:
    // The content of the cell with that key at that width, or none.
    std::shared_ptr<CellContent> find(int key, double width);
    // Keeps a content (`paragraphs` is what it costs): the ones used least lately go when there are too many.
    void insert(int key, double width, const std::shared_ptr<CellContent> &content, int paragraphs);
    // Keeps a content among the last few that were only measured: the row that is measured last may be asked for again at once (to see where it can be cut), the others are not.
    void insertJustMeasured(int key, double width, const std::shared_ptr<CellContent> &content);
    void clear();
    int paragraphs() const { return m_paragraphs; }

private:
    struct Entry {
        std::shared_ptr<CellContent> content;
        std::list<qint64>::iterator at;
        int paragraphs = 0;
    };
    static qint64 keyOf(int key, double width) { return (qint64(key) << 24) | qint64(std::clamp(int(std::lround(width * 16)), 0, (1 << 24) - 1)); }
    std::list<qint64> m_order;      // most lately used first
    QHash<qint64, Entry> m_entries;
    int m_paragraphs = 0;
    static constexpr int kJustMeasured = 32;
    struct Recent {
        qint64 key = -1;
        std::shared_ptr<CellContent> content;
    };
    Recent m_recent[kJustMeasured];
    int m_recentAt = 0;
};

// Where the layout of a table finds the story and the index, while the story is as it was when they were made; after an edit a layout keeps the
// heights it has and measures no more.
struct TableContext {
    struct Live { quint64 epoch = 0; };
    // The heights rows were measured to, kept through edits that do not touch them: by the row and the table (a table that has other columns is another).
    using Heights = QHash<QPair<const RowData *, const TableData *>, double>;
    FlowDocument *doc = nullptr;
    Heights *heights = nullptr;
    qint64 *measured = nullptr;     // counts the rows that were measured (not found in `heights`)
    qint64 *laidOut = nullptr;      // counts the cells that were laid out (not found among those laid out lately)
    CellContentCache *contents = nullptr;   // cells laid out lately
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
    // A line of a cell's content: where it is, and the notes it cites.
    struct UnitLine {
        double top = 0, bottom = 0;
        QVector<NoteRef> notes;
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
    // The most rows a cell covers (1 when none is joined down).
    int mostRowsJoined() const { return m_maxRows; }
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
    std::shared_ptr<CellContent> contentOf(const Cell &c) const { return contentFor(c, true); }

    // A row that goes over a page break is shown a part at a time: each cell's lines that fit the page, and the rest on the next, as a word processor sets a row that
    // may be split. A part is told by its number: 0 for the row whole or the part at its top, else the number of the part (1 and up), which is found again from the
    // room the first part had.
    struct Piece {
        double height = 0;                  // as tall as its room when more of the row comes after it, else as the cells that are left need
        QVector<double> offsets;            // for each cell of the row: how far down its content the part begins (from the top of the cell's text area)
        QVector<double> ends;               // for each cell: how far down its content the part goes (where the lines left for the next part begin; very large when none are)
        QVector<NoteRef> notes;             // the notes cited by the lines in the part, which the page sets at its foot
        double noteHeight = 0;              // their height, with the rule above them where it is theirs
        bool progress = false;              // some cell has a line in the part (without, the row is not split there: it goes to the next page whole)
        int next = 0;                       // the number of the part after it (0: the row ends in this one)
    };
    // The most parts a row is shown in: what a row has left after that many is in the last part, whole (so that a row, however much it holds, ends).
    static constexpr int kMostParts = 50000;
    // How far down the cell's text area the line of the position is (the top of the line; -1 when the cell has no line with it).
    double lineTopIn(int cell, int position) const;
    // Whether row `r` can go over a page break: not one that says it can't, one of an exact height, or one with cells joined down in it (or covered by those).
    bool splittable(int r) const;
    // Part number `index` of row `r`: when it is the first, `room` is the most it may take; else `first` is what the first part had, and `fullRoom` is what each later part may
    // (a page less the header rows). A part at the top of a page is given a line of each cell even if none fits (`force`), so that a row taller than a page ends.
    // What the pages that a row is in parts on have for it: the room of the first part, that of each later part (by whether the page's number is odd or even, as a
    // section's headers of other heights for each make them other), the room of the part asked for (the page it is on now), and what the notes cited in the row take: the
    // rule above them, which the first part's page may have already.
    struct Rooms {
        double first = 0;
        double full[2] = {0, 0};
        int firstParity = 0;                // the first part's page's number, odd or even (1 or 0)
        double ruleGap = 0;
        bool ruleAbove = false;             // the first part's page has notes above it already: no rule is needed for these
    };
    Piece pieceOf(int r, int index, const Rooms &rooms, double roomHere, bool force);
    // How tall a note is when set at the column's width (set by the paginator, which has the notes' stories), for the room a part's notes take.
    void setNoteHeights(std::function<double(const QString &)> height) { m_noteHeight = std::move(height); }
    // The notes cited by the lines of the row's cells (the cells that begin in it), in order.
    QVector<NoteRef> rowNotes(int r);
    // Whether any paragraph of the row's cells cites a footnote (without laying anything out).
    bool rowHasNotes(int r) const;

private:
    friend class TableFlow;
    TableLayout() = default;
    double measureCell(Cell &c);
    // `keep`: the content is kept with the cells laid out lately (what is only measured is not, if it is small: it is not the one that will be asked for again).
    std::shared_ptr<CellContent> contentFor(const Cell &c, bool keep) const;
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
        std::vector<UnitLine> lines;        // each line of a cell's content (a table in it as one), from the top of its text area
        double height = 0;
    };
    std::shared_ptr<const Units> unitsOf(int cell);
    QHash<int, std::shared_ptr<const Units>> m_units;
    // Where each part of a row begins, by the rows' rooms (a part is found from the one before it: parts are asked for in order).
    std::map<std::array<int, 7>, std::vector<QVector<double>>> m_partStarts;
    std::function<double(const QString &)> m_noteHeight;
};

// The cells of paragraphs [i0, i1) of the index laid out as the content of a cell `direct` cells deep, at a width (the paragraphs of the cell itself and
// the tables in it, one under the other).
std::shared_ptr<CellContent> layoutCellContent(const TableContext &ctx, int i0, int i1, int direct, double width);

// How a table is drawn.
struct TablePaint {
    TextOutput output = TextOutput::Picture;
    bool marks = false;                     // paragraph marks and the like
    int selFrom = -1, selTo = -1;           // the selected text, as document positions
    // Draws the objects set in the text of a stretch of a cell (its inline pictures and shapes), the painter placed as the text's lines are; none are drawn when unset.
    std::function<void(QPainter *, const StoryLayout &)> objects;
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
    // The start or the end of the line that has a position in one of its cells (false when it is in none).
    bool lineEdge(int pos, bool end, int *result) const;
    // Up or Down from a position in one of its cells, along `wantedX` (page coordinates; a negative one is the caret's own, and is set): the line above or below in the
    // cell, else the cell above or below in the fragment; `pos` when there is none.
    int moveVertical(int pos, int direction, double *wantedX) const;
    bool contains(int pos) const;
    // The lines of text the fragment shows, with the rectangle each has on the page (the lines of the part of a row that the next page shows are not among them): for the
    // places that objects anchored in cells are set from.
    void forLines(const std::function<void(const StoryLayout::LineBrief &line, const QRectF &rect)> &visit) const;
    // The layouts of the stretches of text in its cells (and in the tables in them).
    void forTexts(const std::function<void(const StoryLayout &text)> &visit) const;
    // The page rectangle of the cell that has the position (empty for none).
    QRectF cellRect(int pos) const;

private:
    struct Placed {
        int item = 0;
        QRectF rect;                        // page coordinates
        QRectF clip;                        // the part of it on the page (a cell joined down may reach past)
        QPointF text;                       // where the cell's text area begins
        double cut = 1e18;                  // the page height below which its text is not drawn (a part of a row ends there, with lines left for the next)
    };
    std::vector<Placed> place() const;
};

class TableFlow {
public:
    explicit TableFlow(FlowDocument *doc);
    ~TableFlow();
    // Whether the story has tables at all (what the paginator asks before anything else, so a document without them is laid out as before).
    bool any() const { return !m_doc->tables.isEmpty(); }
    // The story was edited at `position` (`removed` characters taken out, `added` put in): the tables laid out are gone, and so are the heights of the rows it touched;
    // what was found of the tables is made again, unless the edit was not near one.
    void edited(int position, int removed, int added);
    // The edits made while the tables were not found, dealt with now (the tables are found): what `takeDirty` says is then said of them as well.
    void settleEdits();
    // The places [from, to) in the story, as they are now, that the edits since this was last asked touched in tables: the rows they touched and those that cells joined down tie
    // them to. A page that begins in them is laid out again, as a page that begins where it did is not the same as it was when the height of a row on it comes from a row on an
    // earlier page (a cell joined down is as tall as its text, and the rows it covers share that). False when there are none.
    bool takeDirty(int *from, int *to);
    // Everything about the tables is to be found afresh (the sections or the fonts changed).
    void reset();
    const TableIndex &index();
    TableContext context();
    // How many rows (of tables, and of the tables in their cells) have been measured, as they were not known: what edits cost.
    qint64 rowsMeasured() const { return m_measured; }
    // How many cells have been laid out (a cell that is asked for again soon is not: see CellContentCache).
    qint64 cellsLaidOut() const { return m_laidOut; }
    // The table at index `span` of the index, laid out (the same one again while the story is as it was).
    std::shared_ptr<TableLayout> layout(int span);
    // A fragment of rows [first, end) of a table, with the cells it shows laid out; `parts` are those of its rows that it shows only a part of.
    std::shared_ptr<TableFragment> fragment(const std::shared_ptr<TableLayout> &layout, int firstRow, int endRow, double columnHeight, const std::vector<TableFragment::Part> &parts = {});
    // Whether a page that starts with row `first` of the table has the header rows above it: it does when the table has gone on from earlier pages, unless the header is more
    // than a third of the column (a word processor does not repeat what leaves too little room for the rest).
    static bool repeatsHeader(TableLayout &t, int first, double columnHeight) { return t.headerRows() > 0 && first >= t.headerRows() && t.headerHeight() <= columnHeight / 3 + 0.01; }

private:
    FlowDocument *m_doc;
    TableIndex m_index;
    bool m_valid = false;
    std::shared_ptr<TableContext::Live> m_live;
    QHash<int, std::shared_ptr<TableLayout>> m_layouts;
    TableContext::Heights m_heights;
    CellContentCache m_contents;
    qint64 m_measured = 0, m_laidOut = 0;
    // Edits made while the tables were not found (the index is made when it is asked for): where they were, as places in the story now, with what is beside them. The
    // heights of the rows in them are dropped when the tables are found.
    std::vector<std::pair<int, int>> m_touched;
    int m_dirtyFrom = 0x7fffffff, m_dirtyTo = -1;   // (see takeDirty)
    // Drops the heights of the rows that the places [lo, hi] are in, in the table and in the tables in its cells; whether one of the tables was there.
    bool dropHeights(int lo, int hi);
    void dropHeightsIn(const std::shared_ptr<TableLayout> &t, int lo, int hi);
};

} // namespace jp::flow
