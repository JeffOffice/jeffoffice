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
    // Adds a tag (after the ones before it in reading order) and returns its number. A tag with content is given it by begin and end.
    int add(const PdfTag &tag);
    // The mark that opens the marked content of tag `id` is drawn in `p`, and the one that closes it.
    void begin(QPainter *p, int id);
    void end(QPainter *p, int id);
    const QVector<PdfTag> &tags() const { return m_tags; }

private:
    QVector<PdfTag> m_tags;
};

struct PdfTagOptions {
    QString language;     // as the file says it (en-US)
    QString title;        // shown by a reader in its title bar, instead of the file name
};

// The PDF with the marks turned into marked content and the structure tree added. The PDF as it was if it holds no mark, or is made in a way this doesn't know (nothing is guessed);
// marks of a PDF that is tagged without them (none were drawn) are not looked for.
QByteArray tagPdf(const QByteArray &pdf, const PdfTagger &tagger, const PdfTagOptions &options = {});

} // namespace jp
