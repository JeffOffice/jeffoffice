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
#include <map>
#include <memory>
#include <tuple>

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
    // The page shows how many pages the document has (a NUMPAGES or SECTIONPAGES field in its text, header, or footer), which was `pageTotal` when it was made: it is
    // made again when the document has another.
    bool showsTotal = false;
    int pageTotal = 0;
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
        // The footnotes and endnotes before the page's first paragraph (the numbers its own go on from), and the footnotes cut at the foot of the page
        // before, each as "story|number|from".
        int foot = 0, end = 0;
        QStringList carried;
        // The text area the page has (its top and bottom), which the page before found: a header or footer the page's number or place gives it can be taller
        // than another's. A page that an edit moves has another number, so is laid out again if its area isn't the one it was laid out with.
        double areaTop = 0, areaBottom = 0;
        // Where the page begins, whatever its area is.
        bool sameStartAs(const PageStart &o) const { return pos == o.pos && skip == o.skip && hard == o.hard && section == o.section && foot == o.foot && end == o.end && carried == o.carried; }
        bool operator==(const PageStart &o) const { return sameStartAs(o) && areaTop == o.areaTop && areaBottom == o.areaBottom; }
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
    qint64 pagesLaidOut() const { return m_laidOut; }
    // How many pages the page numbers have been walked back over to find where their section began, for tests.
    qint64 numberStepsForTest() const { return m_numberSteps; }

private:
    struct Run;
    struct BlockFlags;
    Run layoutPage(int index, const PageStart &start);
    int numberOf(int index) const;
    void changed(int position, int removed, int added);   // the story was edited
    void settle(int index);                               // lays out again what edits changed, until page `index` is right (-1: all)
    void settleStep();                                    // one page of that
    void extend(int index);                               // lays out forward until `index` is known
    std::unique_ptr<QTextDocument> sliceOf(const Run &run, const PageStart &st, int *storyFrom, bool *fromBefore) const;

    // The room a page's text has: the section's text area, less what the header and footer ask for beyond the margins (a header or a footer
    // taller than the margin leaves room for pushes the text away from it, as it does in a word processor), and the stories to draw above and below.
    struct Furniture {
        QRectF body;                                  // the text area, in page coordinates, all columns across
        const QTextDocument *header = nullptr, *footer = nullptr;
        double headerHeight = 0, footerHeight = 0;
    };
    Furniture furnitureOf(int index, const PageStart &st, int number);
    double storyHeight(const QTextDocument *story, double width, int number);
    // Puts the page's header and footer (a copy of each story, in a text box) on a page made for painting, and the counts its fields show.
    void addFurniture(Document *d, const Furniture &f, const Section &sec, int section, int number);
    // The places the objects of a page are anchored: the anchor's object, its column's left edge, and where its paragraph and its line are (the page's y), and
    // the anchor's place across (x). The page's header and footer have theirs.
    struct Anchor {
        int object = 0;
        double columnLeft = 0, paragraphTop = 0, lineTop = 0, x = 0;
    };
    // A line of a story as the page holds it (page coordinates), for finding where an anchor is.
    struct AnchorLine {
        int block = 0, pos = 0, end = 0;
        QRectF rect;
        double columnLeft = 0;
    };
    // The anchors in a story from `from` to `to` (document positions), given the lines that hold it.
    QVector<Anchor> anchorsIn(const QTextDocument *story, int from, int to, const QVector<AnchorLine> &lines) const;
    // Puts the objects of the anchors on a page made for painting: pictures (their data in the page's document), rules, boxes, and text boxes, behind the
    // text or over it, with the wrap each asks of the text.
    void addObjects(Document *d, const QVector<Anchor> &anchors, const Section &sec, int number);
    // Changes the pictures of the inline objects in a story to the Ids they have in the page's document.
    void resolveImages(QTextDocument *story, Document *d);
    QString imageIdOf(Document *d, int image);
    QHash<int, QString> m_imageIds;   // the Ids of the pictures in the page being made
    bool m_wrapObjects = false;       // the text goes round the objects that ask it
    // How many footnote and endnote references the story has from `from` to `to`, document positions.
    void countNotes(int from, int to, int *footnotes, int *endnotes) const;
    // A page made for painting has the footnotes its lines refer to, each in a text box at the place the layout gave it, with the rule above them: the
    // places are the layout's own (`places`, `rules`: its notes, and the frames the columns are in) and `ids` the notes the page's lines cite.
    void addNotes(Document *d, const QVector<StoryLayout::NotePlace> &places, const QVector<StoryLayout::Rule> &rules, int firstFrame, const QStringList &ids,
                  const QVector<QRectF> &cols, const QRectF &body);
    std::map<std::tuple<const QTextDocument *, int, int>, double> m_storyHeights;
    std::map<std::tuple<const QTextDocument *, int, int>, QVector<StoryLayout::LineInfo>> m_storyLines;   // the lines of each, from the same layout   // by story, its width in hundredths of a point, and its revision

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
    // The number of the page asked for last, so that the next one's is one more: a walk back to the section's first page is for a page that doesn't follow it.
    mutable int m_numberOf = -1, m_number = 0;
    mutable qint64 m_numberSteps = 0;
};

} // namespace jp::flow
