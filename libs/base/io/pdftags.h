#pragma once
// Tagged PDFs. Qt's PDF writer makes no structure tree, so one is added after the pages are written: the app marks the start and the end of each piece of content as it paints
// (PdfTagger::begin and end draw a mark off the page, which no reader shows), and tagPdf turns the marks into marked content and builds the structure tree the screen readers walk,
// in the order the pieces were begun, which is the order they are read.

#include <QByteArray>
#include <QString>
#include <QVector>

class QPainter;

namespace jp {

struct PdfTag {
    QString type;         // the structure type: P, H1, Figure, Table, TR, TD..., or "Artifact" for what is only decoration (it has no place in the tree)
    QString alt;          // the alternative text of a figure
    int parent = -1;      // the tag it belongs to (an index), or -1 for a top one
    bool content = true;  // false for a tag that only holds others (a table, a row)
};

class PdfTagger {
public:
    // Every export gets marks of its own: their size carries two numbers drawn at random, so that a shape a person draws can't be taken for one.
    PdfTagger();
    // Adds a tag (after the ones before it in reading order) and returns its number. A tag with content is given it by begin and end.
    int add(const PdfTag &tag);
    // The mark that opens the marked content of tag `id` is drawn in `p`, and the one that closes it.
    void begin(QPainter *p, int id);
    void end(QPainter *p, int id);
    const QVector<PdfTag> &tags() const { return m_tags; }
    // The size of the squares this export's marks are (device units), as tagPdf looks for them.
    double markWidth() const { return m_w; }
    double markHeight() const { return m_h; }

private:
    QVector<PdfTag> m_tags;
    double m_w = 4, m_h = 4;
};

struct PdfTagOptions {
    QString language;     // as the file says it (en-US)
    QString title;        // shown by a reader in its title bar, instead of the file name
};

struct PdfTagResult {
    QByteArray pdf;       // the PDF with its marks taken out, tagged when `tagged`; empty when the marks could not be taken out (the pages must be made again without them)
    bool tagged = false;
    QString why;          // when it is not tagged: the reason, in words for a person
};

// The PDF with the marks turned into marked content and the structure tree added. It never hands back a PDF that still has marks: where it can't tag (a PDF it doesn't know the
// form of, one tagged already, nothing to tag) it takes the marks out and says why in `why`; where it can't even do that, `pdf` is empty.
PdfTagResult tagPdf(const QByteArray &pdf, const PdfTagger &tagger, const PdfTagOptions &options = {});

// For tests: a way tagPdf is made to fail, 0 for none: 1 the PDF can't be read, 2 it is tagged already, 3 a page's content can't be read, 4 there is nothing to tag.
int &pdfTagTestFailure();

} // namespace jp
