#pragma once
// The tags a page's objects give a PDF (see io/pdftags.h): a text box a paragraph, a title a heading, a picture a figure with its alt text, a table a table of rows and cells, and what is
// only decoration nothing. They are in the order the page's objects are, back to front, which is the reading order of the page.

#include "io/pdftags.h"
#include "render/renderer.h"

#include <QHash>
#include <functional>

namespace jp {

class PageTagger {
public:
    // `isTitle` says whether a text box is the title of its page (a slide's), which makes it a heading.
    explicit PageTagger(PdfTagger *tags, std::function<bool(const Item &)> isTitle = {});
    // Makes `ctx` mark each of the page's objects, and the cells of its tables, as it paints them.
    void install(PaintContext *ctx);
    // The tag the objects painted from now on belong to (a page's, or a slide's), or -1 for none.
    PdfTagger *tags() const { return m_tags; }
    void setParent(int tag) { m_parent = tag; }
    // The tag of an object that is not a table (a table's are made as it is painted).
    PdfTag tagOf(const Item &item) const;

private:
    void mark(QPainter *p, const Item &item, bool opens);
    void markCell(QPainter *p, const TableItem &table, int row, int col, bool opens);
    PdfTagger *m_tags;
    std::function<bool(const Item &)> m_isTitle;
    int m_parent = -1;
    int m_open = -1;                // the tag of the object being painted, when it has content
    QHash<int, int> m_cells;        // of the table being painted: its cells' tags by row * columns + column
};

} // namespace jp
