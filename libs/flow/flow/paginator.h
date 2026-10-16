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

#include "core/document.h"
#include "render/renderer.h"

#include <QObject>
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
};

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
        bool operator==(const PageStart &o) const { return pos == o.pos && skip == o.skip && hard == o.hard && section == o.section; }
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

    Page page(int index);                       // a page that is known
    int pageOfPosition(int position);           // the page whose lines hold a position; the last known one past them
    std::unique_ptr<MaterializedPage> materialize(int index);   // null for a page that is not known
    // The lines of a page in order, each with the column it is in as its `frame`, and its place in that column (for tests and tools).
    QVector<StoryLayout::LineBrief> lines(int index);

    // How many pages have been laid out since this was made, to see how much an edit costs.
    qint64 pagesLaidOut() const { return m_laidOut; }

private:
    struct Run;
    struct BlockFlags;
    Run layoutPage(int index, const PageStart &start);
    int numberOf(int index) const;
    void changed(int position, int removed, int added);   // the story was edited
    void settle(int index);                               // lays out again what edits changed, until page `index` is right (-1: all)
    void settleStep();                                    // one page of that
    void extend(int index);                               // lays out forward until `index` is known
    std::unique_ptr<QTextDocument> sliceOf(const Run &run, int index, int *storyFrom, bool *fromBefore) const;

    FlowDocument *m_doc;
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
};

} // namespace jp::flow
