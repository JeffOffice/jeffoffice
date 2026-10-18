#pragma once
// Lays a FlowDocument out in pages, one page at a time. A page is a short run of libs/text's StoryLayout over the story, from where the
// page before ended, so a document of thousands of pages is never laid out all at once, and an edit lays out again only the pages it
// touches (it goes on until a page starts where it did before). The paginator keeps where each page starts and nothing more; the pages
// that are looked at (on the screen, or one at a time for a printer or a PDF) are made when they're asked for (materialize), as ordinary
// pages of text boxes that Renderer::paintPage draws.
//
// What crosses a page is settled here: widow and orphan control, keeping paragraphs with the next and their lines together, a page
// break before or after a paragraph, a section break (the next section's first page has its own size and margins), columns, and the
// space before a paragraph at the top of a page (kept after a break that was asked for, left out where the text only came to the top).

#include "flow/flowdocument.h"
#include "flow/tableflow.h"

#include "core/document.h"
#include "render/renderer.h"

#include <QObject>
#include <QTextBlock>
#include <QTextCursor>
#include <QVector>
#include <cmath>
#include <deque>
#include <map>
#include <memory>
#include <tuple>

namespace jp::flow {

// The most objects one page is given (a file with more of them has the rest left off its pages).
constexpr int kMostObjects = 500;
// The most objects the text of one page goes round (the first, in the order of their anchors: the rest are drawn, and the text goes through them). Each is a place
// that every line of the column it reaches into has to be kept out of.
constexpr int kMostWrapped = 24;
// The most rows a table has that is set at a place (a longer one is in the text).
constexpr int kMostFloatingRows = 60;
// The most paragraphs a frame has: one of more is paragraphs in the text, laid out as any are, and is no frame.
constexpr int kMostFrameParagraphs = 300;
// The most frames and tables set at places that one paragraph is the anchor of (those right before it, the nearest first): the frames that come before them are paragraphs in the
// text, and so are the tables, and nothing of a file is left off its pages.
constexpr int kMostFloatsAtAParagraph = 16;

// One page of the document made into the pages the page layer draws: a Document of a single page of the section's size, with a text box
// for each column that has lines, linked in order, over a story of the text on the page. Paint it with
// Renderer::paintPage(painter, context, 0), context.doc being doc.get(), as for any document.
struct MaterializedPage {
    std::unique_ptr<Document> doc;
    int index = 0;                  // the page's place in the document, from 0
    int section = 0;
    int number = 1;                 // the number it shows (its fields say so: the document's setup has it as the first page number)
    QSizeF size;                    // the page, in points
    QVector<QString> boxes;         // the text boxes' ids, a column each, in order
    // Positions in the page's story and the document's story differ by this: a position in the story of the page is `storyFrom` less than
    // the same place in the document's.
    int storyFrom = 0;
    // The first line of the page may be the rest of a paragraph that began on the page before; and the last, the beginning of one that
    // goes on onto the next.
    bool continuesFromBefore = false, continuesOnAfter = false;
    // Where the text of a section begins that has more columns than the one above it and begins below that text on this page (none: none): the empty paragraph that ends the
    // section above, which ends there, takes no room and has no line in the page's boxes.
    QVector<int> freeBreakEnds;
    // The tables in the page's header and footer, drawn with the others (they are not the document's text: no position of the document's is in them).
    QVector<std::shared_ptr<TableFragment>> furnitureTables;
    // The rows of tables that are on the page, drawn after the text (Paginator::tables). Each has a place in the page's story: a paragraph of the
    // same number of characters as the rows have in the document's, one line as tall as they are, so that every other position in it is as it is in
    // the document's.
    QVector<std::shared_ptr<TableFragment>> tables;
    // The page shows how many pages the document has (a NUMPAGES or SECTIONPAGES field in its text, header, or footer), which was `pageTotal` when it was made: it is
    // made again when the document has another.
    bool showsTotal = false;
    int pageTotal = 0;
    // The pictures in the text of the table cells on the page: the id each has in the flow ("flow-img:" and its place in the document's images) and in the page's document.
    QHash<QString, QString> cellImages;
    // The frames set on the page: the positions of the story their paragraphs take (`from` to `to`, just past the last paragraph's end), and the id of the text box in the page
    // that holds their text (each paragraph of the box is a paragraph of the frame, in order, so a position in the frame is the same place in the box's story from `from`).
    struct FrameBox {
        int from = 0, to = 0;
        QString itemId;
    };
    QVector<FrameBox> frames;
};

// Draws the rows of tables that a page has: the page layer's drawing of the page leaves them, as they are not among its items. (Under or over the text, as
// they hold none of it.)
void paintTables(QPainter *p, const MaterializedPage &page, const TablePaint &options);

class Paginator {
public:
    // Where a page starts, which is all the paginator keeps of it.
    struct PageStart {
        int pos = 0;            // the document position of the page's first line
        int skip = 0;           // the lines of that line's paragraph that the pages before hold
        // The page starts for a reason of its own (it is the document's first, or follows a page or section break), not for lack of room
        // on the one before: space before its first paragraph is kept.
        bool hard = true;
        int section = 0;
        // A page that starts in the middle of a row of a table that goes over the break: which part of the row it starts with (the number of the part, from 1), and where in
        // each of the row's cells (how far down its content) the part begins.
        int piece = 0;
        QVector<double> pieceAt;
        // The footnotes and endnotes before the page's first paragraph (the numbers its own go on from), and the footnotes cut at the foot of the page
        // before, each as "story|number|from".
        int foot = 0, end = 0;
        QStringList carried;
        // The page is the first of its section (the one before began in another): the section's first page can have headers and footers of its own, which a page
        // is laid out with.
        bool firstOfSection = false;
        // The paragraph the page begins in was set round objects on the page before: its lines there were not as wide as the column, so the lines before this page can't be laid
        // out again in a frame of that width, and the rest of the paragraph is laid out as a paragraph of its own.
        bool wrapped = false;
        // A blank page that an odd-page or even-page section break leaves: a page of the section before (`section`) with nothing on it, before the page that begins
        // the next section at `pos`.
        bool blank = false;
        bool operator==(const PageStart &o) const { return blank == o.blank && pos == o.pos && skip == o.skip && hard == o.hard && section == o.section && piece == o.piece && pieceAt == o.pieceAt && foot == o.foot && end == o.end && carried == o.carried && firstOfSection == o.firstOfSection && wrapped == o.wrapped; }
        bool operator!=(const PageStart &o) const { return !(*this == o); }
    };

    // What a page is like.
    struct Page {
        PageStart start;
        int number = 1;         // the number it shows
        QSizeF size;
        QRectF content;
        int columns = 1;
        double columnGap = 0;
    };

    // The paginator watches the document's story for edits (the document must outlast it, and so must its story). Like the text engine it
    // drives, it is for one thread, and the story is not to be edited while one of its calls runs.
    explicit Paginator(FlowDocument *doc);
    ~Paginator();
    Paginator(const Paginator &) = delete;
    Paginator &operator=(const Paginator &) = delete;

    // How many pages are known, and whether those are all the document has. A page is known once the page before it is laid out. (After
    // an edit the pages after it are counted as they were until they are laid out again; `settled` says when they have been.)
    int pagesKnown();
    bool complete();
    // Whether every page is as the text makes it: no edit is waiting to be laid out.
    bool settled() const { return !m_pending; }
    // Lays out pages until page `index` is known and as the text makes it (or the document ends); all of them with -1. Returns how many
    // are known. After an edit, this lays out the pages from the one before the edit to `index`: for the pages on the screen.
    int paginate(int index = -1);
    // Lays out until the time is up; whether the document is done. For the pages after the ones on the screen, in the background.
    bool paginateFor(double milliseconds);
    // The number of pages in all, laid out if need be.
    int pageCount() { paginate(-1); return pagesKnown(); }
    // Everything again (the sections or the settings changed).
    void invalidate();
    // The pages from where section `section` begins again (that section, or one after it, changed, or sections came or went from it on): the pages
    // of the sections before it are as they were.
    void invalidateFrom(int section);

    Page page(int index);                       // a page that is known
    int pageOfPosition(int position);           // the page whose lines hold a position; the last known one past them
    std::unique_ptr<MaterializedPage> materialize(int index);   // null for a page that is not known
    // The lines of a page in order, each with the column it is in as its `frame`, and its place in that column (for tests and tools).
    QVector<StoryLayout::LineBrief> lines(int index);

    // How many pages have been laid out since this was made, to see how much an edit costs.
    // The page that shows a position that is in a cell of a table: for a row that goes over page breaks, the page that has the part of the row with that line (which may not
    // be the one that has the paragraph's place in the story: -1 when the position is not in a table).
    int pageOfTablePosition(int position);
    qint64 pagesLaidOut() const { return m_laidOut; }
    // How many times a page has been laid out, the passes that find where its walls are included: for tests of what asking for a page costs.
    qint64 pagePassesForTest() const { return m_passes; }
    // How many times the frames of a header or footer were made, and how far down the page its wrapped shapes reach was worked out (each once for a story as it is, not for each page):
    // for tests of what a page costs.
    qint64 headerFramesMadeForTest() const { return m_headerFramesMade; }
    qint64 headerReachesWorkedOutForTest() const { return m_headerReachesMade; }
    // How many times the tables of a header or footer were laid out, and how many cells of them are held: for tests of what a page costs.
    qint64 headerTablesLaidOutForTest() const { return m_headerTablesMade; }
    qint64 headerTableCellsHeldForTest() const { return m_headerTableCells; }
    // How many rows of tables were measured since it was made (what is measured again after an edit is the rows it touched): for tests of what edits cost.
    qint64 tableRowsMeasured() const { return m_tables ? m_tables->rowsMeasured() : 0; }
    // How many rows were looked at to see whether a row kept with the next has room with those after it (a count for the tests of cost).
    qint64 keepChecks() const { return m_keepChecks; }
    qint64 tableCellsLaidOut() const { return m_tables ? m_tables->cellsLaidOut() : 0; }
    // How many pages the page numbers have been walked back over to find where their section began, for tests.
    qint64 numberStepsForTest() const { return m_numberSteps; }
    // How many characters the copies of the text on the pages have held since it was made (the copy a page is drawn from holds that page's text and a line more): for tests of what a page costs.
    qint64 charsCopiedForTest() const { return m_charsCopied; }

private:
    struct Run;
    struct BlockFlags;
    // The rows of one table on a page, as lines (paginator_tables.cpp).
    struct TableGroup {
        int span = 0;                       // the table, in the index of the tables
        int firstRow = 0, endRow = 0;
        int frame = 0;                      // the column (from 0) it is in
        QRectF rect;                        // its place in the column: the rows' lines together
        int from = 0, to = 0;               // the positions it takes in the story
        std::vector<TableFragment::Part> parts;   // its rows that are only in part on the page
    };
    // The parts of rows that the last lines were made with (a row's part is a line: its row's first position, and its state as the line's number).
    struct PieceRecord {
        int pos = 0, state = 0;
        TableLayout::Piece piece;
    };
    std::vector<PieceRecord> m_pieces;
    // What laying out a page with tables gives besides the lines: the text engine's own notes of the page (where the notes are, the rules above them, and the notes cut at the foot).
    struct TablesRun {
        int pageIndex = 0, pageNumber = 1;                // (given:) the page, and the number it shows
        bool windowShort = false;
        QVector<StoryLayout::NotePlace> notes;
        QVector<StoryLayout::Rule> rules;
        QVector<StoryLayout::CarriedNote> carried;
        QVector<double> reserve;                          // the room the notes take at the foot of each frame, the rule above them included
    };
    // The notes of a page that has tables, as they are set down: the room each frame has taken at its foot (the rule above them included), and where each note is.
    struct Ledger;
    QVector<StoryLayout::LineBrief> linesWithTables(const QVector<FrameSpec> &frames, const StoryLayout::Span &span, int firstFrame, const PageStart &st, TablesRun *out);
    QVector<StoryLayout::LineBrief> tableLines(const QVector<FrameSpec> &frames, const StoryLayout::Span &span, int firstFrame, const PageStart &st, TablesRun *out, Ledger *ledger);
    // The notes in the order of their numbers at the foot of each frame, under one rule, as a page sets them.
    void restackNotes(const QVector<FrameSpec> &frames, Ledger *ledger, TablesRun *out);
    // How tall the note is when set in a column `width` wide (its story is the document's `fields`').
    double noteHeight(const QString &id, double width);
    // A note cut to fit `room` points at the foot of a column `width` wide: how tall its first piece is (0: none fits), and where in its story the rest begins (0: it is whole).
    struct NoteCut {
        double height = 0;
        int from = 0;
    };
    NoteCut cutNote(const QString &id, double width, double room);
    std::map<std::tuple<QString, int, int, int, uint>, double> m_noteHeights;   // by note, the width in hundredths of a point, its story's revision and length (a revision comes again after an undo and another edit), and the environment the text is set in
    qint64 m_keepChecks = 0;   // see keepChecks()
    uint m_envHash = 0;   // what the environment (the fonts, their scale, the line rules) was when the notes were measured
    double m_ruleGap = 0;   // the gap above a page's notes (the last the text engine said)
    QVector<double> m_offsOut;   // where in the row's cells the part that begins the next page starts, as the last lines of a page left it
    // The parts of row `row` of the table `span` that page `index` shows (the page laid out again).
    std::vector<TableLayout::Piece> partsOnPage(int index, int span, int row);
    std::vector<TableGroup> tableGroups(const QVector<StoryLayout::LineBrief> &lines, int kept, int firstFrame);
    bool tablePlaceholder(QTextCursor &dst, QTextBlock &b, bool first, const std::vector<TableGroup> &groups) const;
    QTextBlock lastBlockWithTables(const QVector<StoryLayout::LineBrief> &lines, int kept, const std::vector<TableGroup> &groups) const;
    // The last of the paragraphs that a word processor's hidden marks join from `b` (the one whose properties they have): `b` where it is joined to none, and for a paragraph of a table.
    QTextBlock joinedEndOf(const QTextBlock &b) const;
    // Where the paragraph that has a position begins: where the first of those joined to it does.
    int paragraphStartOf(int position) const;
    // The tables of a text box's story, laid out at the room the box has: their rows are drawn in the page's furniture tables, each at its place in the box, and in the copy of the
    // story that the box holds the paragraphs of each are one, as tall as the rows are.
    void tablesInBox(const QTextDocument *source, QTextDocument *copy, const QRectF &box, const QMarginsF &insets, int number, Document *d, MaterializedPage *page);
    void addTableFragments(MaterializedPage *page, const QVector<QRectF> &cols, const QRectF &body, const std::vector<TableGroup> &groups);
    // An object that the text of a page goes round, as the page was laid out with it: which object (counted from 1) at which anchor character (a position in the story), and
    // the rectangle it is in on the page (page coordinates).
    struct Wrapped {
        int object = 0, at = 0;
        QRectF rect;
        int paragraph = -1;   // the position of the paragraph its anchor is in (not a part of what makes two walls the same)
        std::shared_ptr<const FloatingObject> frame;   // (a frame's object: the paginator's cache of them may let it go while the page is laid out, so the page holds it)
        double below = 0;     // room kept clear under it that is not the object's: the space before of the paragraph that goes below a table as wide as the column
        double anchorTop = -1e18;   // where the paragraph or the line it is set from begins (page coordinates; none for an object set from the margin or the page): the room above it goes no higher
        bool operator==(const Wrapped &o) const {
            return object == o.object && at == o.at && std::abs(below - o.below) < 0.01 && std::abs(rect.left() - o.rect.left()) < 0.01 && std::abs(rect.top() - o.rect.top()) < 0.01 && std::abs(rect.width() - o.rect.width()) < 0.01 && std::abs(rect.height() - o.rect.height()) < 0.01;
        }
    };
    // The page: laid out with the objects its lines anchor and the text goes round, until the objects and the lines agree about where each is (a few times at most: where
    // they don't, the last layout stands, with the objects it was made with); and, in that order, once (`layoutPageWith`) for the objects given.
    Run layoutPage(int index, const PageStart &start);
    // (`cutAt`, a paragraph's position, ends the page before that paragraph, if the page has lines before it: a table set at a place that would go over the foot of the page is
    // moved to the next page with its paragraph.)
    Run layoutPageWith(int index, const PageStart &start, const QVector<Wrapped> &wrapped, int cutAt = -1);
    // The objects that the lines of a laid-out page anchor, with the rectangle each is in, for those the text goes round (at most kMostWrapped of them, in the order of their anchors).
    // (`moveBefore`: the position of the first paragraph that a table set at a place from it would go over the foot of the text area from, if any.)
    QVector<Wrapped> wrappersOf(const Run &run, int index, const PageStart &start, int *moveBefore = nullptr);
    // The frame-local polygons that a column on the page (its rectangle, page coordinates) has the objects in as places the text keeps out of (as the page layer finds them for the
    // text box that is made of the column: Renderer::wrapObstacles).
    QVector<QPolygonF> obstaclesFor(const QVector<Wrapped> &wrapped, const QRectF &column) const;
    // The room a wrapped object takes: its rectangle and the distances round it (and what is kept clear under it). The distance above an object that is set from its paragraph or its
    // line does not reach up into the lines before them: they are laid out first, and the object is set from where the paragraph begins.
    static QRectF roomOf(const FloatingObject &o, const Wrapped &w);
    bool breakContinuesAt(const StoryLayout::LineBrief &last, PageStart *next) const;
    int numberOf(int index) const;
    bool beginsPageOf(int section, const PageStart &page) const;   // the page is the first of the section (not the page after one that a continuous section began in the middle of)
    // The place each section begins at in the story (its first paragraph, the one after the break that ends the section before), as the story is.
    const QVector<int> &sectionStarts() const;
    void changed(int position, int removed, int added);   // the story was edited
    void settle(int index);                               // lays out again what edits changed, until page `index` is right (-1: all)
    void settleStep();                                    // one page of that
    void extend(int index);                               // lays out forward until `index` is known
    std::unique_ptr<QTextDocument> sliceOf(const Run &run, const PageStart &st, int *storyFrom, bool *fromBefore);

    // The room a page's text has: the section's text area, less what the header and footer ask for beyond the margins (a header or a footer
    // taller than the margin leaves room for pushes the text away from it, as it does in a word processor), and the stories to draw above and below.
    struct Furniture {
        QRectF body;                                  // the text area, in page coordinates, all columns across
        const QTextDocument *header = nullptr, *footer = nullptr;
        double headerHeight = 0, footerHeight = 0;
    };
    Furniture furnitureOf(int index, const PageStart &st, int number);
    // (A header's or footer's layout is kept by its story, its width and the room beside it to the page's edge in hundredths of a point, and the story's revision.)
    using StoryKey = std::tuple<const QTextDocument *, int, int, int>;
    static StoryKey storyKey(const QTextDocument *story, double width, double rightRoom) { return {story, int(std::lround(width * 100)), story->revision(), int(std::lround(rightRoom * 100))}; }
    double storyHeight(const QTextDocument *story, double width, int number, bool withSpaceAfter = false, double rightRoom = 0);
    // Puts the lines of the section's page borders on a page made for painting (the page's first items, under the text), where the section's borders say a page of it has them.
    void addPageBorders(Document *d, const Section &sec, bool firstOfSection, const Furniture &f);
    // Puts the page's header and footer (a copy of each story, in a text box) on a page made for painting, and the counts its fields show.
    // (`from` and `to`: the story's positions the page holds, for the fields of the header and footer that show the text of the page.)
    void addFurniture(Document *d, const Furniture &f, const Section &sec, int section, int number, MaterializedPage *page = nullptr, int from = -1, int to = -1);
    // The fields of a header's or footer's story that show a paragraph style's text (STYLEREF) show the page's paragraph of the style: the story is a copy made for the page.
    void substituteStyleRefs(QTextDocument *story, int from, int to) const;
    // A header or footer with tables in it: its text before, between, and after them, and each table, laid out from the top at a width for the page that shows number `number`
    // (the page's number is what the fields in its cells show). The tables are laid out in a private copy of the story, with the table machinery of the document's own.
    struct HeaderPart {
        double y = 0, height = 0;
        int from = 0, to = 0;                        // a stretch of text: its positions in the story
        std::shared_ptr<TableFragment> table;        // or a table
    };
    struct HeaderTables {
        double height = 0;
        bool showsTotal = false;                     // a field in it shows how many pages the document has
        qint64 cells = 0;                            // the cells of its tables (what keeping it costs)
        std::vector<HeaderPart> parts;
        QVector<StoryLayout::LineInfo> lines;        // its lines, text and cells', from the story's own top and left (for the objects its paragraphs anchor)
        std::shared_ptr<FlowDocument> doc;
        std::shared_ptr<FieldSource> fields;         // (what its fields show of the document's pages)
        std::shared_ptr<TableFlow> flow;
    };
    // What a header or footer story holds that decides how it is laid out and for which pages: tables, the page's number, the count of pages, the count of the section's pages.
    struct StoryFacts {
        bool tables = false, page = false, pages = false, sectionPages = false;
    };
    StoryFacts storyFacts(const QTextDocument *story);
    bool storyHasTables(const QTextDocument *story) { return storyFacts(story).tables; }
    // The story laid out for a page that shows `number`, in a document of `pages` pages of which `sectionPages` are the page's section's. It is laid out once for the pages it is the same
    // for: with no field of the page's in it, for all.
    std::shared_ptr<const HeaderTables> headerTablesOf(const QTextDocument *story, double width, int number, double rightRoom, bool withSpaceAfter, int pages, int sectionPages, bool ofABox = false);
    // The places the objects of a page are anchored: the anchor's object, its column's left edge, and where its paragraph and its line are (the page's y), and
    // the anchor's place across (x). The page's header and footer have theirs.
    // (`pos` is where the anchor character is in its story; the paragraph's top and the line's are where the text would have set them without the objects it goes round
    // moving them down: an object is set from where its paragraph would be, not from where its own wrap leaves it.)
    // (An anchor of a frame is at the paragraph after the frame's paragraphs, `frameFrom` to `frameTo` of the story, and `frame` is the object made of them.)
    struct Anchor {
        int object = 0, pos = 0;
        double columnLeft = 0, paragraphTop = 0, lineTop = 0, x = 0;
        int frameFrom = 0, frameTo = 0;
        std::shared_ptr<const FloatingObject> frame;
        int table = -1;     // a floating table (its place in the index of tables): `frame` is the object that stands for it
        int paragraph = -1; // the position of the paragraph it is set from (frames and tables)
    };
    // The object an anchor has as `id` (its own, or a frame's made of the paragraphs).
    const FloatingObject &objectAt(const Anchor &a, int id) const { return a.frame ? *a.frame : m_doc->objects[size_t(id) - 1]; }
    // A line of a story as the page holds it (page coordinates), for finding where an anchor is.
    struct AnchorLine {
        int block = 0, pos = 0, end = 0;
        QRectF rect;
        double columnLeft = 0;
        double pushed = 0;    // how far wrapping moved it down
        bool first = false;   // it is the first line of its paragraph (frames are set where their paragraph's first line is)
        int reach = 0;        // where its anchors end: `end`, or for the last line of a paragraph the paragraph's end (characters that show nothing belong to the line after them, and a
                              // paragraph's last have none: an object anchored at the end of a paragraph is on its last line)
        double above = 0;     // of a paragraph's first line, the space before the paragraph that the engine applied above it
        int upTo() const { return std::max(end, reach); }
    };
    // A line of a page's text as the anchors are found in it.
    AnchorLine anchorLineOf(const StoryLayout::LineBrief &ln, const QRectF &rect, double columnLeft) const;
    // The frames and the tables set at places of the story, and the paragraph each is set from. A frame is the paragraphs next to one another that have its key (fp::Frame) and
    // are set out of the text (tp::OutOfLayout); a table is one that has a place (TableData::place). All of those next to one another (and the paragraphs of the text that show
    // nothing, which are not lines to be set from) are set from the paragraph in the text after them: that paragraph's first line, or the first row of the table that comes after
    // them, if it is one that is in the text. Those that are not set from any paragraph are in the text: the frames and tables at the end of the story, those past the most that
    // one paragraph is the anchor of (`kMostFloatsAtAParagraph`, the nearest to it), and the frames of more paragraphs than a frame has. Found once for a story as it is.
    struct FloatRef {
        int from = 0, to = 0;       // the positions of a frame's paragraphs, or of a table's cells: just past the last one ends it
        int key = 0;                // a frame's key (from 1); 0 for a table
        int span = -1;              // a table's place in the index of tables
        int anchor = -1;            // the position of the paragraph it is set from; none (-1): it is in the text
        int section = 0;            // the section that paragraph is in (a table floats if it fits the pages of its own section)
    };
    struct FloatIndex {
        bool made = false;
        bool anyTable = false;                      // a table has a place
        std::vector<FloatRef> refs;                 // in the story's order
        QHash<int, QVector<int>> at;                // by the position of an anchor paragraph: the refs set from it, in the story's order (the nearest to it last)
        QHash<int, int> tableRef;                   // a table's place in the index of tables: its ref
        QVector<int> inText;                        // in order: the positions of the paragraphs of frames that are lines of the text (the text engine lays those out, set apart as they are)
    };
    const FloatIndex &floatIndex();
    void forgetFloats();   // the index and the frames and tables made are of the story as it was (an edit, or the settings changed)
    // The position of the paragraph that a position in a frame, or in a table that is set at a place, is set from (the page that has that paragraph's first line has it); the position
    // itself for one in the text.
    int anchorOfPosition(int position);
    // The anchors of the frames and tables that are set from the paragraphs whose first lines are among `lines` (a column `columnWidth` wide holds them), and of the objects anchored in
    // the frames' text.
    QVector<Anchor> floatAnchors(const QVector<AnchorLine> &lines, double columnWidth);
    // The lines of a page's text that objects are anchored by, in page coordinates (`body` is the text area): `held`, those of the text itself (the rows of tables are not: what a cell
    // anchors does not move the text round it), and `firstLines`, those that frames and tables are set from (the first row of a table in the text stands for the table).
    void pageAnchorLines(const Run &run, const QRectF &body, QVector<AnchorLine> *held, QVector<AnchorLine> *firstLines) const;
    // The anchors of a page: those of the objects anchored in the text of `held`, and of the frames and tables set from the first lines among `firstLines`.
    QVector<Anchor> anchorsOfLines(const QVector<AnchorLine> &held, const QVector<AnchorLine> &firstLines, double columnWidth);
    // The frame of paragraphs `from` to `to` (the story's positions) as an object: its text copied for the page, and its size from the frame's and the text's.
    // (The text of a frame is from the story's paragraphs `from` to `to`: the document's own, or `source`, a header's or a footer's, whose frames are not cached.)
    std::shared_ptr<const FloatingObject> frameObject(int key, int from, int to, double columnWidth, const QTextDocument *source = nullptr);
    std::unique_ptr<QTextDocument> frameStory(int from, int to, const QTextDocument *source = nullptr) const;
    // The frames of a header's or a footer's story (paragraphs set at a place on the page, which the story's own lines leave out), each an anchor for the page's objects.
    QVector<Anchor> storyFrameAnchors(const QTextDocument *story, double columnLeft, double columnWidth, double top);
    // A table that is set at a place (TableData::place) and that can be: of few enough rows, and not taller than a page's text area. Its rows are no lines of the text, which
    // goes round it: it is an object (`tableObject`), anchored at the paragraph after it, that has its rows drawn where it is.
    bool floatsTable(int span);
    // Whether the page has anything that is set at a place and that the text goes round: the document's objects, its frames (which are objects), or a table set at a place.
    bool hasFloaters();
    FloatIndex m_floats;
    // The walls a page ended with when it was laid out, for the page asked for again (its lines, the page made): the objects it was laid out round, found by laying it out more than
    // once. By the page, with what it was laid out as (where it starts, how many pages there were: a page count in its text) so that a page that is not that is found again.
    struct WallMemo {
        PageStart start;
        int pages = 0;
        QVector<Wrapped> walls;
        int cutAt = -1;
    };
    std::map<int, WallMemo> m_wallMemo;
    std::shared_ptr<const FloatingObject> tableObject(int span, double columnWidth);
    QVector<Anchor> tableAnchors(const QVector<AnchorLine> &lines, double columnWidth);
    std::map<std::tuple<int, int, int, uint>, std::shared_ptr<const FloatingObject>> m_tableObjects;   // by where the table is in the story, the column's width, and the environment (edits empty it)
    std::map<std::tuple<int, int, int, int, uint>, std::shared_ptr<const FloatingObject>> m_frames;   // by key, the paragraphs' places, the width the column has, and the environment (edits empty it)
    // The anchors in a story from `from` to `to` (document positions), given the lines that hold it.
    QVector<Anchor> anchorsIn(const QTextDocument *story, int from, int to, const QVector<AnchorLine> &lines, const QHash<int, QVector<int>> *byBlock = nullptr);
    // How far down the page the shapes that the text goes round, set in a header, reach (0: none do): the page's text begins under them.
    double headerObjectsBottom(const QTextDocument *story, const Section &sec, int number);
    // The characters of a paragraph that anchor an object (a position and the object's number each), found once for a story as it is.
    const QVector<std::pair<int, int>> &anchorCharsOf(const QTextBlock &b);
    QHash<int, QVector<std::pair<int, int>>> m_anchorChars;   // by the paragraph's position
    // Where an object is on the page (page coordinates) for the anchor it has: from the page, the margin, a strip of it, the column, the paragraph, or the line, as it says, and
    // kept on the page (down from the paragraph or the line).
    // (A table set at a place from its paragraph that would go over `limit`, the foot of the text area, is slid up to end at it, if it still begins at or below its paragraph: the
    // table is on the page, and goes to the next only if it would not fit there; none: -1.)
    QRectF placeOf(const FloatingObject &o, const Anchor &a, const Section &sec, bool clamp = true, double limit = -1) const;
    // Puts the objects of the anchors on a page made for painting: pictures (their data in the page's document), rules, boxes, and text boxes, behind the
    // text or over it. Those in `wrapped` (the objects the page was laid out with) are where it was laid out with them and ask the text to go round; any other asks nothing.
    // (An object with a wrap that leaves no text beside it is a wall as wide as the columns that it reaches into, which `columns` (their rectangles on the page) say: the page layer's
    // wall of that kind is as wide as the page, and would put the text of the other columns aside too.)
    // (`page` is given for the tables that are set at places: their rows are drawn from it. The objects behind the text go after the first `front` items of the page, which are
    // those of its header and footer. `boxTables` is the page that the tables of the text boxes are drawn on, where `page` (the body's, the frames of its text) is not given: for the objects of a header
    // or a footer.)
    void addObjects(Document *d, const QVector<Anchor> &anchors, const Section &sec, int number, const QVector<Wrapped> &wrapped = {}, const QVector<QRectF> &columns = {}, MaterializedPage *page = nullptr, size_t front = 0,
                    MaterializedPage *boxTables = nullptr);
    // Changes the pictures of the inline objects in a story to the Ids they have in the page's document.
    void resolveImages(QTextDocument *story, Document *d);
    QString imageIdOf(Document *d, int image);
    // Adds the pictures set in the text of the cells of a page's tables to the page's document (the ids they have there are the page's `cellImages`).
    void resolveCellImages(MaterializedPage *page, Document *d);
    QHash<int, QString> m_imageIds;   // the Ids of the pictures in the page being made
    // How many footnote and endnote references the story has from `from` to `to`, document positions.
    // (Where numbers restart, in each section, they are those after the last place they do: `restarted` says whether one is in the stretch.)
    void countNotes(int from, int to, int *footnotes, int *endnotes, bool *restarted = nullptr) const;
    // Whether the numbers of the notes begin again at a paragraph (the one before it ends a section, and the notes restart in each).
    bool restartsNotesAt(const QTextBlock &block) const;
    // The places in the story from a position on, for as many characters, where the numbers of the notes begin again.
    QVector<int> notesRestartIn(int from, int characters) const;
    // A page made for painting has the footnotes its lines refer to, each in a text box at the place the layout gave it, with the rule above them: the
    // places are the layout's own (`places`, `rules`: its notes, and the frames the columns are in) and `ids` the notes the page's lines cite.
    // (Over the part of a note that goes on from the page before, the file's own separator for it; under the last note, when `notesGoOn`, the file's continuation notice.)
    void addNotes(Document *d, const QVector<StoryLayout::NotePlace> &places, const QVector<StoryLayout::Rule> &rules, int firstFrame, const QStringList &ids,
                  const QVector<QRectF> &cols, const QRectF &body, bool notesGoOn);
    std::map<StoryKey, double> m_storyHeights;
    std::map<std::pair<const QTextDocument *, int>, StoryFacts> m_storyFacts;   // what each header or footer story (as of its revision) holds
    // The layouts of the tables of headers and footers kept, by story, width, revision, room beside, whether the space after counts, and the page number, count of pages, and count of
    // the section's pages that the story shows (0 for those it doesn't); the oldest go when the cells they hold are many.
    using HeaderTablesKey = std::tuple<const QTextDocument *, int, int, int, bool, int, int, int>;
    std::map<HeaderTablesKey, std::shared_ptr<HeaderTables>> m_headerTables;
    std::deque<HeaderTablesKey> m_headerTablesOrder;
    // (The tables of text boxes are kept apart, with a bound of their own: they would otherwise take the room of a header's, which is laid out again on every page that is past the bound.)
    std::map<HeaderTablesKey, std::shared_ptr<HeaderTables>> m_boxTables;
    std::deque<HeaderTablesKey> m_boxTablesOrder;
    qint64 m_boxTableCells = 0;
    std::map<std::tuple<const QTextDocument *, int, int, int, int, uint>, QVector<Anchor>> m_storyFrameAnchors;   // the frames of each header or footer story, by its width, revision, places, and the environment
    std::map<std::tuple<const QTextDocument *, int, QString>, double> m_headerBottoms;   // how far down the page the wrapped shapes of each header reach, by story, revision, and the section's places
    std::map<StoryKey, QVector<StoryLayout::LineInfo>> m_storyLines;   // the lines of each, from the same layout   // by story, its width in hundredths of a point, and its revision

    FlowDocument *m_doc;
    std::unique_ptr<TableFlow> m_tables;
    std::unique_ptr<QObject> m_watch;
    QVector<PageStart> m_starts;
    bool m_complete = false;
    // After edits, the pages up to m_valid are laid out afresh and those after are as they were, moved with the text. One laid out
    // afresh that starts where one of them did, past m_dirtyEnd (the end of the paragraphs the edits touched, as the text is now),
    // means the pages from there are the same, if they were laid out afresh too (those up to m_high were, after the first edit; the
    // ones after it are as they were before it).
    int m_shift = 0;   // how many more pages than there were are before the ones an edit has not been settled to yet
    bool m_pending = false;
    int m_valid = 0, m_high = 0, m_dirtyEnd = 0;
    qint64 m_laidOut = 0;
    qint64 m_passes = 0;
    qint64 m_headerFramesMade = 0, m_headerReachesMade = 0;
    qint64 m_charsCopied = 0;
    qint64 m_headerTablesMade = 0, m_headerTableCells = 0;   // layouts of the tables made, and the cells held
    // The number of the page asked for last, so that the next one's is one more: a walk back to the section's first page is for a page that doesn't follow it.
    mutable int m_numberOf = -1, m_number = 0;
    mutable qint64 m_numberSteps = 0;
    mutable QVector<int> m_sectionStarts;
    mutable int m_sectionStartsRevision = -1;
};

} // namespace jp::flow
