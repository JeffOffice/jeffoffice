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
#include <memory>

namespace jp::flow {

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
    // The rows of tables that are on the page, drawn after the text (Paginator::tables). Each has a place in the page's story: a paragraph of the
    // same number of characters as the rows have in the document's, one line as tall as they are, so that every other position in it is as it is in
    // the document's.
    QVector<std::shared_ptr<TableFragment>> tables;
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
        // A page that starts in the middle of a row of a table that goes over the break: which part of the row it starts with (the number of the part, from 1), and the room
        // the row's first part had, in twentieths of a point (the later parts are found from it).
        int piece = 0, pieceRoom = 0;
        bool operator==(const PageStart &o) const { return pos == o.pos && skip == o.skip && hard == o.hard && section == o.section && piece == o.piece && pieceRoom == o.pieceRoom; }
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
    // How many rows of tables were measured since it was made (what is measured again after an edit is the rows it touched): for tests of what edits cost.
    qint64 tableRowsMeasured() const { return m_tables ? m_tables->rowsMeasured() : 0; }
    qint64 tableCellsLaidOut() const { return m_tables ? m_tables->cellsLaidOut() : 0; }
    // How many pages the page numbers have been walked back over to find where their section began, for tests.
    qint64 numberStepsForTest() const { return m_numberSteps; }

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
    QVector<StoryLayout::LineBrief> linesWithTables(const QVector<FrameSpec> &frames, const StoryLayout::Span &span, int firstFrame, int piece, int pieceRoom, bool *windowShort);
    int m_roomOut = 0;   // the room the first part of the row that goes on to the next page had (twentieths of a point), as the last lines of a page left it
    std::vector<TableGroup> tableGroups(const QVector<StoryLayout::LineBrief> &lines, int kept, int firstFrame);
    bool tablePlaceholder(QTextCursor &dst, QTextBlock &b, bool first, const std::vector<TableGroup> &groups) const;
    QTextBlock lastBlockWithTables(const QVector<StoryLayout::LineBrief> &lines, int kept, const std::vector<TableGroup> &groups) const;
    void addTableFragments(MaterializedPage *page, const Section &sec, const std::vector<TableGroup> &groups);
    Run layoutPage(int index, const PageStart &start);
    int numberOf(int index) const;
    void changed(int position, int removed, int added);   // the story was edited
    void settle(int index);                               // lays out again what edits changed, until page `index` is right (-1: all)
    void settleStep();                                    // one page of that
    void extend(int index);                               // lays out forward until `index` is known
    std::unique_ptr<QTextDocument> sliceOf(const Run &run, int *storyFrom, bool *fromBefore) const;

    FlowDocument *m_doc;
    std::unique_ptr<TableFlow> m_tables;
    std::unique_ptr<QObject> m_watch;
    QVector<PageStart> m_starts;
    bool m_complete = false;
    // After edits, the pages up to m_valid are laid out afresh and those after are as they were, moved with the text. One laid out
    // afresh that starts where one of them did, past m_dirtyEnd (the end of the paragraphs the edits touched, as the text is now),
    // means the pages from there are the same, if they were laid out afresh too (those up to m_high were, after the first edit; the
    // ones after it are as they were before it).
    bool m_pending = false;
    int m_valid = 0, m_high = 0, m_dirtyEnd = 0;
    qint64 m_laidOut = 0;
    // The number of the page asked for last, so that the next one's is one more: a walk back to the section's first page is for a page that doesn't follow it.
    mutable int m_numberOf = -1, m_number = 0;
    mutable qint64 m_numberSteps = 0;
};

} // namespace jp::flow
