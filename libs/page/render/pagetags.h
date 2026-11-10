#pragma once
// The tags a page's objects give a PDF (see io/pdftags.h): a text box a paragraph, a title a heading, a picture a figure with its alt text, a table a table of rows and cells, and what is
// only decoration nothing (it is marked as an artifact, so that a screen reader passes over it). They are in the order the page's objects are, back to front, which is the reading
// order of the page.

#include "io/pdftags.h"
#include "render/renderer.h"

#include <QHash>
#include <functional>

namespace jp {

class PageTagger {
public:
    // `isTitle` says whether a text box is the title of its page (a slide's), which makes it a heading.
    explicit PageTagger(PdfTagger *tags, std::function<bool(const Item &)> isTitle = {});
    // Makes `ctx` mark each of the page's objects, and the cells of its tables, as it paints them, and its background and master pages as artifacts.
    void install(PaintContext *ctx);
    PdfTagger *tags() const { return m_tags; }
    // The tag the objects painted from now on belong to (a page's, or a slide's), or -1 for none.
    void setParent(int tag) { m_parent = tag; }
    // A section of the tags for a page or a slide (the objects painted from now on belong to it); returns its tag.
    int beginSection();
    // What is drawn between these is decoration. (They may not be inside an object's own marks.)
    void artifactBegin(QPainter *p);
    void artifactEnd(QPainter *p);
    // The tag of an object that is not a table (a table's are made as it is painted).
    PdfTag tagOf(const Item &item) const;

private:
    void mark(QPainter *p, const Item &item, bool opens);
    void markCell(QPainter *p, const TableItem &table, int row, int col, bool opens);
    PdfTagger *m_tags;
    std::function<bool(const Item &)> m_isTitle;
    int m_parent = -1;
    int m_open = -1;                // the tag of the object being painted, when it has content
    int m_artifact = -1;            // the tag of the artifact being drawn, or -1
    bool m_tableArtifact = false;   // the table being painted is decoration but for the text of its cells
    QHash<int, int> m_cells;        // of the table being painted: its cells' tags by row * columns + column
};

// Draws decoration between its making and its end (nothing, with no tagger).
struct ArtifactSpan {
    ArtifactSpan(PageTagger *tagger, QPainter *p) : m_t(tagger), m_p(p) { if (m_t) m_t->artifactBegin(m_p); }
    ~ArtifactSpan() { if (m_t) m_t->artifactEnd(m_p); }
    ArtifactSpan(const ArtifactSpan &) = delete;
    ArtifactSpan &operator=(const ArtifactSpan &) = delete;

private:
    PageTagger *m_t;
    QPainter *m_p;
};

// The main language of a document's text, as the tag a PDF's catalog names it (en-US): the language most of its letters are in (a run with none given is in `fallback`).
QString documentLanguage(const Document &doc, const QString &fallback = QStringLiteral("en-US"));

} // namespace jp
