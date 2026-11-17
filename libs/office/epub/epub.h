#pragma once
// EPUB 3 packages, made once for every app: the container, the package document (metadata, manifest, spine), the contents as an EPUB 3 navigation
// document (and, when asked, an NCX for older readers), the chapters, a cover, and pictures and fonts as resources, in a zip whose first entry is the
// uncompressed "mimetype". The callers bring the chapters' XHTML (JeffPub from its pages, JeffWrite from the document's HTML); nothing here knows either.

#include <QByteArray>
#include <QImage>
#include <QString>
#include <QVector>

namespace jp {
class ZipWriter;
}

namespace jp::office::epub {

// Text as XML: escaped, and without the characters XML 1.0 forbids.
QString xml(const QString &in);
// An XHTML page (the style sheet linked as style.css) holding `body`.
QString page(const QString &title, const QString &lang, const QString &body);
// The style sheet every book is given.
const char *styleSheet();

struct NavEntry {
    int level;              // 1 and up; a deeper one is listed under the last one above it (at most kMostNavLevel deep)
    QString title, href;    // href is relative to the OEBPS folder, "chapter2.xhtml#h5"
};
constexpr int kMostNavLevel = 6;

struct Chapter {
    QString title;          // empty: the book's title for the first, "Part N" after
    QString body;           // what goes inside <body>, as XHTML
};

// A file of the book: a picture or a font. `href` is relative to the OEBPS folder ("images/pic1.png"); an SVG picture is deflated, the others are stored.
struct Resource {
    QString href, mediaType;
    QByteArray data;
};

struct Book {
    QString title, author, language, identifier;   // the caller fills the title and language; an empty identifier is a new one
    QImage cover;                                   // no cover when null
    QVector<Chapter> chapters;                      // chapter1.xhtml, chapter2.xhtml...
    QVector<NavEntry> nav;                          // empty: one entry for the book
    QString endnotes;                               // list items of a Notes page at the back; none when empty
    QVector<Resource> resources;                    // the manifest's img1, img2...
    bool ncx = false;                               // also write toc.ncx and name it in the spine, for readers older than EPUB 3
};

// The book as bytes (an empty chapter list is one empty chapter).
QByteArray build(const Book &book);
// Writes the bytes to `path` all at once; false, with `error` the reason, when that fails.
bool save(const QByteArray &bytes, const QString &path, QString *error);

// The parts a book of a different shape (JeffPub's page-for-page one) is made of too.
void addPackageStart(ZipWriter &zip);                 // mimetype (first, stored), META-INF/container.xml, OEBPS/style.css
QByteArray coverJpeg(const QImage &cover);
QString newIdentifier();
QString metadata(const QString &id, const QString &title, const QString &lang, const QString &author);   // dc:identifier, title, language, creator, and dcterms:modified
QString packageDocument(const QString &lang, const QString &metadata, const QString &manifest, const QString &spine, const QString &prefix = QString());

} // namespace jp::office::epub
