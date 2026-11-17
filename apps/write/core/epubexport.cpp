#include "core/epubexport.h"

#include "core/wordformat.h"
#include "epub/epub.h"
#include "wml/wmlhtml.h"

#include <QHash>
#include <QRegularExpression>

namespace jp::write {

namespace {

namespace wml = office::wml;

QString extensionOf(const QString &mime)
{
    if (mime == QLatin1String("image/jpeg")) return QStringLiteral("jpg");
    if (mime == QLatin1String("image/svg+xml")) return QStringLiteral("svg");
    return mime.mid(mime.indexOf(QLatin1Char('/')) + 1);
}

// The notes of a part of the file by their Ids (the separators left out).
QHash<int, QVector<wml::Block>> notesOf(const wml::Document &doc, const QString &typeEnd)
{
    QHash<int, QVector<wml::Block>> out;
    for (const wml::KeptPart &part : doc.parts) {
        if (!part.contentType.endsWith(typeEnd)) continue;
        QVector<wml::NotePart> notes;
        if (!wml::readNotes(part.data, &notes)) continue;
        for (const wml::NotePart &n : std::as_const(notes))
        for (const wml::NotePart &n : std::as_const(notes))
            if (n.type.isEmpty()) out.insert(n.id, n.blocks);
    }
    return out;
}

} // namespace

QByteArray epubFromDocument(const wml::Document &doc, const EpubExportOptions &options, QString *error)
{
    // Where the chapters begin: each Heading 1 (not a table's own, and not the Title) up to the most chapters; the part before the first is a chapter too.
    QVector<int> starts{0};
    {
        int inTable = 0;
        for (int i = 0; i < doc.body.size(); ++i) {
            const wml::Block &b = doc.body[i];
            if (b.isRaw) {
                if (b.tag == QLatin1String("tbl")) inTable += b.role == wml::Inline::Open ? 1 : b.role == wml::Inline::Close ? -1 : 0;
                continue;
            }
            bool title = false;
            if (inTable <= 0 && wml::htmlHeadingLevel(doc, b.para, &title) == 1 && !title && i > 0 && starts.size() < options.mostChapters) starts << i;
        }
    }

    QString language = options.language;
    if (language.isEmpty()) language = languageOf(doc.styles.run(doc.styles.defaultId(QStringLiteral("paragraph")), QString()).extra);
    if (language.isEmpty()) language = QStringLiteral("en-US");

    // Pictures become files of the book, each part once, within the bounds.
    office::epub::Book book;
    QHash<QString, QString> hrefOf;
    qint64 pictureBytes = 0;
    auto pictureHref = [&](const QString &name, const QByteArray &data, const QString &mime) -> QString {
        const auto known = hrefOf.constFind(name);
        if (known != hrefOf.cend()) return known.value();
        if (book.resources.size() >= options.mostPictures || data.size() > options.mostPictureBytes || pictureBytes + data.size() > options.mostPicturesBytes) return QString();
        pictureBytes += data.size();
        const QString href = QStringLiteral("images/pic%1.%2").arg(book.resources.size() + 1).arg(extensionOf(mime));
        book.resources << office::epub::Resource{href, mime, data};
        hrefOf.insert(name, href);
        return href;
    };

    const QHash<int, QVector<wml::Block>> footnotes = notesOf(doc, QStringLiteral("footnotes+xml")), endnotes = notesOf(doc, QStringLiteral("endnotes+xml"));
    wml::Document work = doc;
    int notesLeft = kMostEpubNotes, firstFootnote = 1, firstEndnote = 1;
    auto html = [&](const QVector<wml::Block> &blocks, wml::HtmlOptions htmlOptions) {
        work.body = blocks;
        htmlOptions.xhtml = true;
        htmlOptions.bodyOnly = true;
        htmlOptions.language = language;
        htmlOptions.pictureHref = pictureHref;
        return QString::fromUtf8(wml::documentToHtml(work, htmlOptions));
    };

    // (The first chapter begins at a Heading 1 when the document does.)
    bool firstIsHeading = false;
    if (!doc.body.isEmpty() && !doc.body[0].isRaw) {
        bool title = false;
        firstIsHeading = wml::htmlHeadingLevel(doc, doc.body[0].para, &title) == 1 && !title;
    }
    int navEntries = 0;
    for (int c = 0; c < starts.size(); ++c) {
        const int from = starts[c], to = c + 1 < starts.size() ? starts[c + 1] : int(doc.body.size());
        QVector<wml::HtmlHeading> headings;
        QVector<wml::HtmlNote> notes;
        wml::HtmlOptions chapterOptions;
        chapterOptions.headingIdPrefix = QStringLiteral("h");
        chapterOptions.headings = &headings;
        chapterOptions.notes = &notes;
        chapterOptions.mostNotes = notesLeft;
        chapterOptions.firstFootnote = firstFootnote;
        chapterOptions.firstEndnote = firstEndnote;
        QString body = html(doc.body.mid(from, to - from), chapterOptions);
        if (c == 0 && body.trimmed().isEmpty() && starts.size() > 1) continue;   // nothing before the first chapter
        notesLeft -= int(notes.size());
        for (const wml::HtmlNote &n : std::as_const(notes)) ++(n.endnote ? firstEndnote : firstFootnote);
        // The notes of the chapter, at its end, each with the link back to its reference.
        QString asides;
        for (const wml::HtmlNote &n : std::as_const(notes)) {
            const QHash<int, QVector<wml::Block>> &from_ = n.endnote ? endnotes : footnotes;
            const QString link = QStringLiteral("<a href=\"#%1\">%2</a>").arg(n.referenceId, office::epub::xml(n.number));
            QString text = html(from_.value(n.id), wml::HtmlOptions());
            if (text.startsWith(QLatin1String("<p>"))) text.insert(3, link + QLatin1Char(' '));
            else text = QStringLiteral("<p>") + link + QStringLiteral("</p>\n") + text;
            asides += QStringLiteral("<aside epub:type=\"%1\" id=\"%2\">\n%3</aside>\n").arg(n.endnote ? QStringLiteral("endnote") : QStringLiteral("footnote"), n.noteId, text);
        }
        if (!asides.isEmpty()) body += QStringLiteral("<section epub:type=\"footnotes\">\n<hr/>\n") + asides + QStringLiteral("</section>\n");

        office::epub::Chapter chapter;
        const int number = int(book.chapters.size()) + 1;
        for (int h = 0; h < headings.size(); ++h) {
            const bool opening = h == 0 && headings[0].level == 1 && (c > 0 || firstIsHeading);
            if (opening) chapter.title = headings[0].text;
            if (headings[h].level > kMostEpubHeadingLevel || navEntries >= options.mostNavEntries || headings[h].text.isEmpty()) continue;
            ++navEntries;
            book.nav << office::epub::NavEntry{headings[h].level, headings[h].text, QStringLiteral("chapter%1.xhtml").arg(number) + (opening ? QString() : QLatin1Char('#') + headings[h].id)};
        }
        chapter.body = body;
        book.chapters << chapter;
    }

    book.title = options.title.isEmpty() ? QStringLiteral("Untitled") : options.title;
    book.author = options.author;
    book.language = language;
    book.identifier = options.identifier;
    book.ncx = true;
    if (options.cover)
        for (const office::epub::Resource &r : std::as_const(book.resources)) {
            if (r.mediaType == QLatin1String("image/svg+xml")) continue;
            const QImage image = QImage::fromData(r.data);
            if (image.isNull()) continue;
            book.cover = image;
            break;
        }
    if (error) error->clear();
    return office::epub::build(book);
}

} // namespace jp::write
