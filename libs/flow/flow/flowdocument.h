#pragma once
// A document that flows through pages made as its text fills them: the sections that say what the pages are like (their size,
// margins, and columns), and the one story that has all the text, as a text box's story in JeffPub is (libs/text's text properties).
// The Paginator (paginator.h) lays the story out in pages.

#include "text/textengine.h"

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
};
} // namespace fp

// What a section's header or footer holds on its first page, its odd pages, and its even pages: a story each, or none. (A reader gives a section the
// stories of the section before for the slots it states none of: the file's rule.) The pages draw them at the section's header and footer distances,
// and the text area gives way to one that is taller than its margin leaves room for.
struct HeaderFooter {
    std::shared_ptr<QTextDocument> first, odd, even;
};

// The character in a story that ends the page. A paragraph that holds one is still one block: the text engine ends the line with it (as it does
// with U+2028), and the paginator ends the page after that line, so the rest of the paragraph is the first lines of the next page (that it
// "continues" is only that its next line is its line 1, never stored). A paragraph that ends with one leaves the next paragraph at the top of the
// next page, with no empty line before it; one at the end of the last paragraph makes no page of its own.
constexpr QChar kPageBreak = QChar(0x000C);

// The most columns a page has (a section that asks for more has this many).
constexpr int kMostColumns = 32;

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

    // Colors, fonts, and the default tab stop the text is laid out with.
    LayoutEnv env;
    // What fields read (page numbers, notes); none by default.
    const FieldSource *fields = nullptr;
    // The notes' numbers begin again at 1 in each section (a word processor's option), where they go on through the document otherwise.
    bool restartNotesInSections = false;
    // Automatic hyphenation (off, as a word processor's is).
    bool hyphenate = false;
    double hyphenZone = 18;
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

} // namespace jp::flow
