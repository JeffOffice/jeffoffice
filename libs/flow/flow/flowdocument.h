#pragma once
// A document that flows through pages made as its text fills them: the sections that say what the pages are like (their size,
// margins, and columns), and the one story that has all the text, as a text box's story in JeffPub is (libs/text's text properties).
// The Paginator (paginator.h) lays the story out in pages.

#include "text/textengine.h"

#include <QMarginsF>
#include <QRectF>
#include <QSizeF>
#include <QString>
#include <QTextFormat>
#include <QVector>
#include <memory>

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

// What a section's header or footer holds on its first page, its odd pages, and its even pages: a story each, or none (the section before's
// goes on). Only the slots are here; the pages don't draw them yet.
struct HeaderFooter {
    std::shared_ptr<QTextDocument> first, odd, even;
};

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
    int firstPageNumber = 0;                 // the number the section's first page has (0: it goes on from the section before)
    QString pageNumberFormat;                // "", "roman", "ROMAN", "alpha", or "ALPHA" (the fields' own)

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
    // Automatic hyphenation (off, as a word processor's is).
    bool hyphenate = false;
    double hyphenZone = 18;
    // Widow and orphan control, for paragraphs that ask for it (tp::WidowControl): the fewest lines of a paragraph to leave at the foot
    // of a page, and to take to the top of the next. (The text engine keeps two: more than that is asked for here.)
    int widowLines = 2, orphanLines = 2;

private:
    std::unique_ptr<QTextDocument> m_story;
};

} // namespace jp::flow
