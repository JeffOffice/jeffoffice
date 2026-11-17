#include "epub/epub.h"

#include "io/zip.h"

#include <QBuffer>
#include <QDateTime>
#include <QSaveFile>
#include <QUuid>

namespace jp::office::epub {

QString xml(const QString &in)
{
    QString s;
    s.reserve(in.size());
    for (qsizetype i = 0; i < in.size(); ++i) {
        const QChar c = in[i];
        const ushort u = c.unicode();
        if (c.isHighSurrogate() && i + 1 < in.size() && in[i + 1].isLowSurrogate()) {
            s += c;
            s += in[++i];
            continue;
        }
        if (c.isSurrogate() || u == 0xFFFE || u == 0xFFFF || (u < 0x20 && u != '\t' && u != '\n' && u != '\r')) continue;
        if (u == '<') s += QLatin1String("&lt;");
        else if (u == '>') s += QLatin1String("&gt;");
        else if (u == '&') s += QLatin1String("&amp;");
        else if (u == '"') s += QLatin1String("&quot;");
        else s += c;
    }
    return s;
}

QString page(const QString &title, const QString &lang, const QString &body)
{
    return QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<!DOCTYPE html>\n"
                          "<html xmlns=\"http://www.w3.org/1999/xhtml\" xmlns:epub=\"http://www.idpf.org/2007/ops\" lang=\"%2\" xml:lang=\"%2\">\n"
                          "<head><meta charset=\"utf-8\"/><title>%1</title><link rel=\"stylesheet\" type=\"text/css\" href=\"style.css\"/></head>\n"
                          "<body>\n%3</body>\n</html>\n")
        .arg(xml(title), xml(lang), body);
}

const char *styleSheet()
{
    return "body { margin: 0 4%; line-height: 1.4; }\n"
           "p { margin: 0 0 0.6em; }\n"
           "h1 { font-size: 1.6em; margin: 1.2em 0 0.6em; line-height: 1.2; }\n"
           "h2 { font-size: 1.3em; margin: 1em 0 0.5em; line-height: 1.2; }\n"
           "h3 { font-size: 1.1em; margin: 1em 0 0.4em; }\n"
           ".center { text-align: center; }\n.right { text-align: right; }\n.justify { text-align: justify; }\n"
           ".art { font-size: 1.4em; font-weight: bold; text-align: center; }\n"
           "figure { margin: 1em 0; text-align: center; }\n"
           "figure img { max-width: 100%; height: auto; }\n"
           "figcaption { font-size: 0.9em; }\n"
           "table { border-collapse: collapse; margin: 1em 0; }\n"
           "td { border: 1px solid #999; padding: 0.2em 0.4em; vertical-align: top; }\n"
           "td p { margin: 0; }\n"
           "a.noteref { font-size: 0.7em; vertical-align: super; line-height: 0; text-decoration: none; }\n"
           "aside { font-size: 0.9em; margin: 0.4em 0; }\n"
           "ol.notes li { margin-bottom: 0.5em; }\n"
           ".cover { margin: 0; padding: 0; text-align: center; }\n"
           ".cover img { max-width: 100%; max-height: 100%; }\n";
}

namespace {

// The table of contents as nested lists: each heading under the last one of a higher level.
QString navList(const QVector<NavEntry> &nav, int &i, int level)
{
    QString out = QStringLiteral("<ol>\n");
    while (i < nav.size() && nav[i].level >= level) {
        const NavEntry &e = nav[i++];
        out += QStringLiteral("<li><a href=\"%1\">%2</a>").arg(xml(e.href), xml(e.title));
        if (i < nav.size() && nav[i].level > e.level) out += navList(nav, i, nav[i].level);
        out += QStringLiteral("</li>\n");
    }
    return out + QStringLiteral("</ol>\n");
}

// The same list for an NCX: a navPoint for each heading, the ones under it inside it; play order counts the points as they come.
QString ncxPoints(const QVector<NavEntry> &nav, int &i, int level, int &order)
{
    QString out;
    while (i < nav.size() && nav[i].level >= level) {
        const NavEntry &e = nav[i++];
        const int mine = ++order;
        out += QStringLiteral("<navPoint id=\"n%1\" playOrder=\"%1\"><navLabel><text>%2</text></navLabel><content src=\"%3\"/>\n").arg(mine).arg(xml(e.title), xml(e.href));
        if (i < nav.size() && nav[i].level > e.level) out += ncxPoints(nav, i, nav[i].level, order);
        out += QStringLiteral("</navPoint>\n");
    }
    return out;
}

} // namespace

void addPackageStart(ZipWriter &zip)
{
    // The type first and uncompressed, so a reader can tell what the file is.
    zip.add(QStringLiteral("mimetype"), QByteArrayLiteral("application/epub+zip"));
    zip.add(QStringLiteral("META-INF/container.xml"),
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<container version=\"1.0\" xmlns=\"urn:oasis:names:tc:opendocument:xmlns:container\">\n"
            "<rootfiles><rootfile full-path=\"OEBPS/content.opf\" media-type=\"application/oebps-package+xml\"/></rootfiles>\n</container>\n",
            true);
    zip.add(QStringLiteral("OEBPS/style.css"), styleSheet(), true);
}

QByteArray coverJpeg(const QImage &cover)
{
    QByteArray jpg;
    QBuffer buf(&jpg);
    buf.open(QIODevice::WriteOnly);
    cover.convertToFormat(QImage::Format_RGB32).save(&buf, "JPEG", 90);
    return jpg;
}

QString newIdentifier()
{
    return QStringLiteral("urn:uuid:") + QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QString metadata(const QString &id, const QString &title, const QString &lang, const QString &author)
{
    QString meta = QStringLiteral("<dc:identifier id=\"bookid\">%1</dc:identifier>\n<dc:title>%2</dc:title>\n<dc:language>%3</dc:language>\n")
                       .arg(xml(id), xml(title), xml(lang));
    if (!author.isEmpty()) meta += QStringLiteral("<dc:creator>%1</dc:creator>\n").arg(xml(author));
    meta += QStringLiteral("<meta property=\"dcterms:modified\">%1</meta>\n").arg(QDateTime::currentDateTimeUtc().toString(QStringLiteral("yyyy-MM-ddThh:mm:ssZ")));
    return meta;
}

QString packageDocument(const QString &lang, const QString &metadata, const QString &manifest, const QString &spine, const QString &prefix)
{
    return QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                          "<package xmlns=\"http://www.idpf.org/2007/opf\" version=\"3.0\" unique-identifier=\"bookid\" xml:lang=\"%1\"%5>\n"
                          "<metadata xmlns:dc=\"http://purl.org/dc/elements/1.1/\">\n%2</metadata>\n"
                          "<manifest>\n%3</manifest>\n<spine%6>\n%4</spine>\n</package>\n")
        .arg(xml(lang), metadata, manifest, spine, prefix.isEmpty() ? QString() : QStringLiteral(" prefix=\"%1\"").arg(prefix), QString());
}

QByteArray build(const Book &book)
{
    const QString &title = book.title, &lang = book.language;
    const QString id = !book.identifier.isEmpty() ? book.identifier : newIdentifier();
    ZipWriter zip;
    addPackageStart(zip);

    QString manifest, spine;
    manifest += QStringLiteral("<item id=\"nav\" href=\"nav.xhtml\" media-type=\"application/xhtml+xml\" properties=\"nav\"/>\n"
                               "<item id=\"css\" href=\"style.css\" media-type=\"text/css\"/>\n");
    if (book.ncx) manifest += QStringLiteral("<item id=\"ncx\" href=\"toc.ncx\" media-type=\"application/x-dtbncx+xml\"/>\n");
    if (!book.cover.isNull()) {
        zip.add(QStringLiteral("OEBPS/images/cover.jpg"), coverJpeg(book.cover));
        zip.add(QStringLiteral("OEBPS/cover.xhtml"),
                page(title, lang, QStringLiteral("<section epub:type=\"cover\" class=\"cover\"><img src=\"images/cover.jpg\" alt=\"%1\"/></section>\n").arg(xml(title))).toUtf8(), true);
        manifest += QStringLiteral("<item id=\"cover-image\" href=\"images/cover.jpg\" media-type=\"image/jpeg\" properties=\"cover-image\"/>\n"
                                   "<item id=\"cover\" href=\"cover.xhtml\" media-type=\"application/xhtml+xml\"/>\n");
        spine += QStringLiteral("<itemref idref=\"cover\"/>\n");
    }
    QVector<NavEntry> nav = book.nav;
    for (NavEntry &e : nav) e.level = qBound(1, e.level, kMostNavLevel);
    QVector<Chapter> chapters = book.chapters;
    if (chapters.isEmpty()) chapters << Chapter();
    for (int i = 0; i < chapters.size(); ++i) {
        const Chapter &c = chapters[i];
        const QString name = QStringLiteral("chapter%1.xhtml").arg(i + 1);
        const QString heading = c.title.isEmpty() ? (i == 0 ? title : QStringLiteral("Part %1").arg(i + 1)) : c.title;
        zip.add(QStringLiteral("OEBPS/") + name, page(heading, lang, c.body).toUtf8(), true);
        manifest += QStringLiteral("<item id=\"c%1\" href=\"%2\" media-type=\"application/xhtml+xml\"/>\n").arg(i + 1).arg(name);
        spine += QStringLiteral("<itemref idref=\"c%1\"/>\n").arg(i + 1);
    }
    if (!book.endnotes.isEmpty()) {
        zip.add(QStringLiteral("OEBPS/notes.xhtml"),
                page(QStringLiteral("Notes"), lang, QStringLiteral("<section epub:type=\"endnotes\"><h1>Notes</h1>\n<ol class=\"notes\">\n%1</ol></section>\n").arg(book.endnotes)).toUtf8(), true);
        manifest += QStringLiteral("<item id=\"notes\" href=\"notes.xhtml\" media-type=\"application/xhtml+xml\"/>\n");
        spine += QStringLiteral("<itemref idref=\"notes\"/>\n");
        nav << NavEntry{1, QStringLiteral("Notes"), QStringLiteral("notes.xhtml")};
    }
    // A book without headings still lists where it starts.
    if (nav.isEmpty()) nav << NavEntry{1, title, QStringLiteral("chapter1.xhtml")};
    int at = 0;
    QString navBody = QStringLiteral("<nav epub:type=\"toc\" id=\"toc\"><h1>Contents</h1>\n");
    while (at < nav.size()) navBody += navList(nav, at, nav[at].level);
    navBody += QStringLiteral("</nav>\n<nav epub:type=\"landmarks\" hidden=\"hidden\"><ol>\n");
    if (!book.cover.isNull()) navBody += QStringLiteral("<li><a epub:type=\"cover\" href=\"cover.xhtml\">Cover</a></li>\n");
    navBody += QStringLiteral("<li><a epub:type=\"bodymatter\" href=\"chapter1.xhtml\">Start</a></li>\n</ol></nav>\n");
    zip.add(QStringLiteral("OEBPS/nav.xhtml"), page(QStringLiteral("Contents"), lang, navBody).toUtf8(), true);
    if (book.ncx) {
        int order = 0;
        QString points;
        at = 0;
        while (at < nav.size()) points += ncxPoints(nav, at, nav[at].level, order);
        zip.add(QStringLiteral("OEBPS/toc.ncx"),
                QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<ncx xmlns=\"http://www.daisy.org/z3986/2005/ncx/\" version=\"2005-1\" xml:lang=\"%1\">\n"
                               "<head><meta name=\"dtb:uid\" content=\"%2\"/><meta name=\"dtb:depth\" content=\"%3\"/><meta name=\"dtb:totalPageCount\" content=\"0\"/><meta name=\"dtb:maxPageNumber\" content=\"0\"/></head>\n"
                               "<docTitle><text>%4</text></docTitle>\n<navMap>\n%5</navMap>\n</ncx>\n")
                    .arg(xml(lang), xml(id))
                    .arg([&nav] { int m = 1; for (const NavEntry &e : nav) m = qMax(m, e.level); return m; }())
                    .arg(xml(title), points)
                    .toUtf8(),
                true);
    }
    int n = 0;
    for (const Resource &r : book.resources) {
        zip.add(QStringLiteral("OEBPS/") + r.href, r.data, r.mediaType == QLatin1String("image/svg+xml"));
        manifest += QStringLiteral("<item id=\"img%1\" href=\"%2\" media-type=\"%3\"/>\n").arg(++n).arg(r.href, r.mediaType);
    }

    QString meta = metadata(id, title, lang, book.author);
    // Older readers find the cover by this.
    if (!book.cover.isNull()) meta += QStringLiteral("<meta name=\"cover\" content=\"cover-image\"/>\n");
    QString opf = packageDocument(lang, meta, manifest, spine);
    if (book.ncx) opf.replace(QLatin1String("<spine>"), QStringLiteral("<spine toc=\"ncx\">"));
    zip.add(QStringLiteral("OEBPS/content.opf"), opf.toUtf8(), true);
    return zip.finish();
}

bool save(const QByteArray &bytes, const QString &path, QString *error)
{
    QSaveFile f(path);
    if (!f.open(QIODevice::WriteOnly) || f.write(bytes) != bytes.size() || !f.commit()) {
        if (error) *error = f.errorString();
        return false;
    }
    return true;
}

} // namespace jp::office::epub
