#pragma once
// A document that flows through pages made as its text fills them: the sections that say what the pages are like (their size,
// margins, and columns), and the one story that has all the text, as a text box's story in JeffPub is (libs/text's text properties).
// The Paginator (paginator.h) lays the story out in pages.

#include "flow/tabledata.h"
#include "text/textengine.h"

#include <QColor>
#include <QMarginsF>
#include <QChar>
#include <QRectF>
#include <QSizeF>
#include <QString>
#include <QTextFormat>
#include <QVector>
#include <memory>
#include <optional>

class QTextDocument;

namespace jp::flow {

// Paragraph properties that only a flowing document has. The page's flow also reads libs/text's keep with next (tp::KeepWithNext), keep
// lines together (tp::KeepTogether), and widow and orphan control (tp::WidowControl), and Qt's own page break policy: a paragraph that
// always starts a page has QTextFormat::PageBreak_AlwaysBefore, one that always ends a page PageBreak_AlwaysAfter.
namespace fp {
enum : int {
    // bool: the last paragraph of a section. The pages of the next section follow, from a page of their own.
    SectionBreakAfter = QTextFormat::UserProperty + 300,
    // int: the key of the record (FlowDocument::tables) of the table cell the paragraph is in; none (0) for a paragraph that is not in a table.
    Cell = QTextFormat::UserProperty + 302,
    // int, of a character: the object (FlowDocument::objects, counted from 1) that is set on the page where the character is.
    AnchorId = QTextFormat::UserProperty + 301,
    // int: the key of the frame (FlowDocument::frames, from 1) that the paragraph is a part of: it and the paragraphs next to it with the same key are a frame, set at a place of
    // its own on the page of the paragraph after them (the paragraphs also have tp::OutOfLayout, so that the text engine leaves them out of the text); none (0) for the rest.
    Frame = QTextFormat::UserProperty + 303,
    // double: the paragraph's first line is no higher on the page than this many points from the page's top (a word processor's field that sets text at a place); where it would be
    // higher, the text is moved down to the place, and what follows it with it.
    AdvanceTop = QTextFormat::UserProperty + 304,
};
} // namespace fp

// What a section's header or footer holds on its first page, its odd pages, and its even pages: a story each, or none. (A reader gives a section the
// stories of the section before for the slots it states none of: the file's rule.) The pages draw them at the section's header and footer distances,
// and the text area gives way to one that is taller than its margin leaves room for.
struct HeaderFooter {
    std::shared_ptr<QTextDocument> first, odd, even;
};

// A picture, rule, box, or text box that is set at a place on the page its anchor (a character in a story, fp::AnchorId) is on, rather than in a line:
// where it goes is worked out when the page is made. The item has its size and look; the page gives it its place (and a picture its data).
struct FloatingObject {
    // What a position is measured from. The margin areas (the strip between the page's edge and the margin, on the side or at the top or foot) are for an object set
    // in the margin: Inside and Outside are the left and right strips across, and the top and foot ones down.
    enum class Rel { Margin, Page, Column, Paragraph, Line, Character, LeftMargin, RightMargin, TopMargin, BottomMargin, InsideMargin, OutsideMargin };
    enum class Align { None, Start, Center, End };
    // How the text goes round it: not at all (it is behind the text or over it), round a rectangle (Tight is Square until the contours are read), or only above and below.
    enum class Wrap { None, Square, Tight, TopBottom };
    // Where the text goes beside a Square or Tight wrap: on both sides, or only the left, only the right, or the larger of the two.
    enum class Side { Both, Left, Right, Largest };
    ItemPtr item;                           // a PictureItem, ShapeItem, LineItem, or TextItem
    int image = -1;                         // a picture's place in FlowDocument::images
    std::shared_ptr<QTextDocument> story;   // a text box's text
    QMarginsF insets{7.2, 3.6, 7.2, 3.6};   // of a text box
    Rel hRel = Rel::Column, vRel = Rel::Paragraph;
    Align hAlign = Align::None, vAlign = Align::None;
    double dx = 0, dy = 0;                  // from where it is measured (a line's ends are in the item's p1 and p2, from there too)
    bool behind = false;                    // behind the text
    int z = 0;                              // above (or under) the other objects of its place: the page draws them in this order
    Wrap wrap = Wrap::None;
    Side side = Side::Both;
    QMarginsF gap;                          // the room the text round it leaves
    int next = 0;                           // the one after it at the same anchor (counted from 1), or none
    // A frame: the paragraphs of the story that have its key (fp::Frame) are its text. `item` is a TextItem and `story` is not given: the paginator makes both from the paragraphs,
    // at the width `frameWidth` says (0: as wide as the column, or as the longest line of the text if the frame shrinks) and the height the text asks (`frameHeight` is the least of it, or all of it).
    int frame = 0;
    double frameWidth = 0, frameHeight = 0;
    bool frameExactHeight = false;
    // A frame with no width of its own is as wide as the column; one that shrinks (a drop cap) is as wide as its longest line.
    bool frameShrinks = false;
};

// A picture's bytes, shared by every page that shows it (a page of the document holds a copy of the reference).
struct FlowImage {
    QByteArray bytes;
    QString format;
};

// What the id of an inline picture begins with, until a page changes it to the picture's id in the page's own document.
inline const QLatin1String kFlowImagePrefix("flow-img:");

// The character in a story that ends the page. A paragraph that holds one is still one block: the text engine ends the line with it (as it does
// with U+2028), and the paginator ends the page after that line, so the rest of the paragraph is the first lines of the next page (that it
// "continues" is only that its next line is its line 1, never stored). A paragraph that ends with one leaves the next paragraph at the top of the
// next page, with no empty line before it; one at the end of the last paragraph makes no page of its own.
constexpr QChar kPageBreak = QChar(0x000C);

// The most columns a page has (a section that asks for more has this many).
constexpr int kMostColumns = 32;

// The lines round a section's pages (a word processor's page borders).
struct PageBorders {
    struct Side {
        enum class Line { Single, Double, Dotted, Dashed };
        bool present = false;
        double width = 0.5;                  // points (of one line, for a double border)
        double space = 0;                    // from the edge the border is measured from (the page's, or the text's) to the line's outer edge
        QColor color{Qt::black};
        Line line = Line::Single;
        bool art = false;                    // a pattern of little pictures: drawn as a plain line
    };
    enum class Show { All, FirstPage, NotFirstPage };   // the pages of the section that have them
    Side top, left, bottom, right;
    bool fromPage = false;                   // measured from the page's edge; else from the text area's (with the header and the footer inside)
    Show show = Show::All;
    bool any() const { return top.present || left.present || bottom.present || right.present; }
};

// Pages alike: a run of the document's pages from one section break to the next.
struct Section {
    enum class Orientation { Portrait, Landscape };

    QSizeF paper{612, 792};                  // the paper as it comes, upright (Letter)
    Orientation orientation = Orientation::Portrait;
    QMarginsF margins{72, 72, 72, 72};
    int columns = 1;
    double columnGap = 36;                   // between columns
    HeaderFooter header, footer;
    double headerDistance = 36, footerDistance = 36;   // from the page's edge
    bool differentFirstPage = false;         // the first page uses the header and footer's first slot
    bool differentOddEven = false;           // odd and even pages use their own slots
    // The section begins on the page the one before ends on (a continuous section break) when the two have the same paper, turned the same way, whatever their
    // margins; any other begins a page of its own. (The top and bottom margins are the page's, of the page it begins at the top of. With other side margins or
    // other columns it begins below the text of the one before, as the page's text area is of the first's above and the second's below; the same sides and columns,
    // and it is the same text area.)
    bool continuous = false;
    // The section begins on the next odd (or even) page: when the page it would begin has the other number (as the page numbers go on, whatever the section restarts them at), a
    // blank page, one of the section before's, is left before it. (A continuous section has no page of its own to ask this of.)
    enum class Parity { Any, Odd, Even };
    Parity startParity = Parity::Any;
    bool sameSheetAs(const Section &o) const { return paper == o.paper && orientation == o.orientation; }
    bool sameColumnsAs(const Section &o) const { return columns == o.columns && columnGap == o.columnGap; }
    bool sameSidesAs(const Section &o) const { return margins.left() == o.margins.left() && margins.right() == o.margins.right(); }
    bool samePageAs(const Section &o) const { return sameSheetAs(o) && sameColumnsAs(o) && sameSidesAs(o); }
    PageBorders pageBorders;                 // the lines round its pages
    std::optional<int> firstPageNumber;      // the number the section's first page has (0 is a number; none: it goes on from the section before)
    QString pageNumberFormat;                // "", "roman", "ROMAN", "alpha", or "ALPHA" (the fields' own)

    // The header and footer story a page uses, or none: the first page's of the section when the section asks for one and it is the section's
    // first page, else the even pages' when it asks for those and the page's number is even, else the odd pages' (the ordinary one).
    const QTextDocument *headerFor(bool firstOfSection, int pageNumber) const;
    const QTextDocument *footerFor(bool firstOfSection, int pageNumber) const;

    int columnCount() const;                 // `columns`, from 1 to kMostColumns
    QSizeF pageSize() const;                 // as the paper is turned
    QRectF content() const;                  // the text area, in page coordinates
    double columnWidth() const;
    QRectF columnRect(int column) const;     // a column of the text area, in page coordinates
};

class FlowDocument {
public:
    FlowDocument();
    ~FlowDocument();
    FlowDocument(const FlowDocument &) = delete;
    FlowDocument &operator=(const FlowDocument &) = delete;

    // The text of the whole document, a paragraph for each paragraph. Edit it as any story (the Paginator is told).
    QTextDocument *story() const { return m_story.get(); }

    // The sections in order (always at least one). A paragraph with fp::SectionBreakAfter ends a section: the one after it is in the
    // next, and the paragraphs after the last such break are in the last section (a break past the last section ends it again).
    QVector<Section> sections;
    const Section &section(int index) const;

    // The cells of the tables in the story (a table is paragraphs that have a cell's key; see tabledata.h).
    TableStore tables;
    // The objects set at places (FloatingObject), and the pictures they and the text hold (an inline picture is a tp::InlineObject whose picture is
    // "flow-img:" and a place in `images`, which a page changes to the picture's Id in the page's own document).
    QVector<FloatingObject> objects;
    QVector<FlowImage> images;
    // The frames of the story (fp::Frame): the object (from 1) that each is, by its key from 1.
    QVector<int> frames;
    // Colors, fonts, and the default tab stop the text is laid out with.
    LayoutEnv env;
    // What fields read (page numbers, notes); none by default.
    const FieldSource *fields = nullptr;
    // The notes' numbers begin again at 1 in each section (a word processor's option), where they go on through the document otherwise.
    bool restartNotesInSections = false;
    // Automatic hyphenation (off, as a word processor's is).
    bool hyphenate = false;
    double hyphenZone = 18;
    // The color the pages are filled with (a word processor's page color); none (invalid) by default.
    QColor pageColor;
    // The paragraph mark after a page break character that ends a paragraph goes to the page after the break, where it makes an empty line before the next
    // paragraph (a word processor's option, which its recent files always have); otherwise it stays with the break, and the next paragraph is the top of the page.
    bool markFollowsBreak = false;
    // The paragraph that begins with a page break has its space before again at the top of the page after the break (off where the file says to suppress it, as the 2003 program's does).
    bool spaceBeforeAfterBreak = true;
    // Widow and orphan control, for paragraphs that ask for it (tp::WidowControl): the fewest lines of a paragraph to leave at the foot
    // of a page, and to take to the top of the next. (The text engine keeps two: more than that is asked for here.)
    int widowLines = 2, orphanLines = 2;
    // How many lines of a paragraph that goes on from the page before are laid out again with the page, to set the page as the whole
    // text sets it (tab stops, right-to-left text). Where a paragraph has more lines before the page, the page lays out the rest of it as a
    // paragraph of its own, and a paragraph of many pages costs each page no more than any other.
    int exactLines = 100;

private:
    std::unique_ptr<QTextDocument> m_story;
};

// A copy of a story for a page. (Qt's own copy leaves the first paragraph, when it is empty, without its formatting: its line height, and the size of
// its mark, so it is as tall as the document's default font makes it.)
std::unique_ptr<QTextDocument> cloneStory(const QTextDocument *story);

} // namespace jp::flow
