#pragma once
// The tags JeffWrite's pages give a PDF (see io/pdftags.h and render/pagetags.h): each paragraph of the text is a paragraph (P) or a heading (H1 to H6), the paragraphs of a list are
// the items of a list (L, LI, and LBody), a table is its rows and cells (Table, TR, and TH for a header row, TD), a picture is a figure with its alt text, and what is only decoration
// (the headers and footers, the page's color, borders, and the lines between columns) is marked as an artifact, so that a screen reader passes over it. The tags are in the order they
// are painted, which is the reading order: the text of a page, its tables, and its objects, page after page.

#include "flow/paginator.h"
#include "io/pdftags.h"
#include "render/pagetags.h"

#include <QHash>
#include <QSet>

namespace jp::write {

class WriteTagger {
public:
    WriteTagger();
    PdfTagger &tags() { return m_tags; }
    // The next page is going to be painted: its tags are put in a section of their own.
    void beginPage();
    // Makes the painting of `page` mark what it draws (`page` must outlive the painting).
    void install(PaintContext *ctx, const flow::MaterializedPage &page);
    // Makes the painting of the page's tables mark their rows and cells, and the decoration of the tables in its header and footer.
    void install(flow::TablePaint *tables);

private:
    void markItem(QPainter *p, const Item &item, bool opens);
    void markBlock(QPainter *p, const Item &item, int position, double top, bool opens);
    void markTable(QPainter *p, const flow::TableFragment &table, bool opens);
    void markCell(QPainter *p, const flow::TableFragment &table, int cell, bool opens);
    // Where something is in the reading order of the page: its column, then how far down it is.
    double readingOrder(double x, double y) const;
    PdfTagger m_tags;
    PageTagger m_page;
    const flow::MaterializedPage *m_current = nullptr;
    int m_section = -1;
    QVector<QRectF> m_columns;     // the page's columns of text
    double m_bodyTop = 0;          // how far down the column being painted is
    double m_bodyLeft = 0;
    int m_open = -1;               // the tag whose content is being painted (-1: none)
    bool m_artifact = false;       // the item being painted is decoration
    bool m_body = false;           // the item being painted is a column of the page's text: its paragraphs are tagged one by one
    int m_inline = -1;             // the tag of the picture or object set in the text that is being painted
    bool m_inlineArtifact = false;
    int m_list = -1;               // the list the last paragraph was an item of (-1 when it was not in one)
    QHash<int, int> m_blocks;      // the tag of each paragraph of the page's text, by where it begins (a paragraph in two columns is one tag)
    int m_table = -1;
    bool m_tableArtifact = false;  // the table being painted is decoration but for the text of its cells
    QHash<int, int> m_rows, m_cells;
};

} // namespace jp::write
