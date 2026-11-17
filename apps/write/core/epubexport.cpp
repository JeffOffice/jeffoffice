#include "core/epubexport.h"

#include "core/wordformat.h"
#include "epub/epub.h"
#include "render/metafile.h"
#include "wml/wmlhtml.h"

#include <QBuffer>
#include <QCoreApplication>
#include <QHash>
#include <QImage>
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
            if (n.type.isEmpty()) out.insert(n.id, n.blocks);
    }
    return out;
}

// A BCP 47 tag the book may name its language by: letters and digits in parts of up to eight, the first of two or three letters ("en_US" is "en-US"); else US English.
QString validLanguage(QString tag)
{
    tag = tag.trimmed();
    tag.replace(QLatin1Char('_'), QLatin1Char('-'));
    static const QRegularExpression valid(QStringLiteral("^[A-Za-z]{2,3}(-[A-Za-z0-9]{1,8}){0,4}$"));
    return tag.size() <= 35 && valid.match(tag).hasMatch() ? tag : QStringLiteral("en-US");
}

// A note's words alone, a paragraph each, for the notes that are past the bound on what is written with its looks.
QString plainNote(const QVector<wml::Block> &blocks)
{
    QString out;
    for (const wml::Block &b : blocks) {
        if (b.isRaw) continue;
        QString text;
        for (const wml::Inline &in : b.para.content) {
            if (in.kind == wml::Inline::Text) text += in.text;
            else if (!in.display.isEmpty() && in.tag != QLatin1String("footnoteRef") && in.tag != QLatin1String("endnoteRef")) text += in.display;
        }
        text = text.trimmed();
        if (!text.isEmpty()) out += QStringLiteral("<p>") + office::epub::xml(text) + QStringLiteral("</p>\n");
    }
    return out;
}

// A picture as a file a reader shows: as it is for the kinds every reader has, else drawn into a PNG (a metafile with our own drawing, a bitmap with the image readers); false when it is neither.
bool readableForm(const QByteArray &data, const QString &mime, QByteArray *bytes, QString *type)
{
    if (!mime.isEmpty()) {
        *bytes = data;
        *type = mime;
        return true;
    }
    QImage image;
    if (Metafile::looksLikeMetafile(data)) {
        Metafile mf;
        if (mf.load(data) && mf.isValid()) image = mf.toImage(1600);
    } else {
        image = QImage::fromData(data);
    }
    if (image.isNull()) return false;
    if (qMax(image.width(), image.height()) > 4000) image = image.scaled(4000, 4000, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    bytes->clear();
    QBuffer buffer(bytes);
    buffer.open(QIODevice::WriteOnly);
    if (!image.save(&buffer, "PNG")) return false;
    *type = QStringLiteral("image/png");
    return true;
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
    language = validLanguage(language);

    // Pictures become files of the book, each part once, within the bounds.
    office::epub::Book book;
    QHash<QString, QString> hrefOf;
    qint64 pictureBytes = 0;
    auto pictureHref = [&](const QString &name, const QByteArray &data, const QString &mime) -> QString {
        // (A part is made a file once, and counted once, however many times the text shows it; one that can't be a file is not looked at again.)
        const auto known = hrefOf.constFind(name);
        if (known != hrefOf.cend()) return known.value();
        QString href;
        QByteArray bytes;
        QString type;
        if (book.resources.size() < options.mostPictures && data.size() <= options.mostPictureBytes && readableForm(data, mime, &bytes, &type) && bytes.size() <= options.mostPictureBytes
            && pictureBytes + bytes.size() <= options.mostPicturesBytes) {
            pictureBytes += bytes.size();
            href = QStringLiteral("images/pic%1.%2").arg(book.resources.size() + 1).arg(extensionOf(type));
            book.resources << office::epub::Resource{href, type, bytes};
        }
        hrefOf.insert(name, href);
        return href;
    };

    const QHash<int, QVector<wml::Block>> footnotes = notesOf(doc, QStringLiteral("footnotes+xml")), endnotes = notesOf(doc, QStringLiteral("endnotes+xml"));
    wml::Document work = doc;
    int notesLeft = options.mostNotes, firstFootnote = 1, firstEndnote = 1;
    qint64 noteBytes = 0;
    QHash<qint64, QString> noteFirst;     // the notes whose words are written, by kind and Id: the id of the first reference's note, where they are (one reference or many, they are written once)
    QVector<QStringList> bookmarksOf;     // the bookmark ids each chapter holds

    auto html = [&](const QVector<wml::Block> &blocks, wml::HtmlOptions htmlOptions, QStringList *bookmarks) {
        work.body = blocks;
        htmlOptions.xhtml = true;
        htmlOptions.bodyOnly = true;
        htmlOptions.language = language;
        htmlOptions.pictureHref = pictureHref;
        htmlOptions.bookmarks = bookmarks;
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
        QStringList bookmarks;
        wml::HtmlOptions chapterOptions;
        chapterOptions.headingIdPrefix = QStringLiteral("h");
        chapterOptions.headings = &headings;
        chapterOptions.notes = &notes;
        chapterOptions.mostNotes = notesLeft;
        chapterOptions.firstFootnote = firstFootnote;
        chapterOptions.firstEndnote = firstEndnote;
        QString body = html(doc.body.mid(from, to - from), chapterOptions, &bookmarks);
        if (c == 0 && body.trimmed().isEmpty() && starts.size() > 1) continue;   // nothing before the first chapter
        notesLeft -= int(notes.size());
        for (const wml::HtmlNote &n : std::as_const(notes)) ++(n.endnote ? firstEndnote : firstFootnote);
        const int number = int(book.chapters.size()) + 1;
        // The notes of the chapter, at its end, each with the link back to its reference. A note's words are written once, with their looks while the bound allows, else as plain words;
        // another reference to the same note is a line that says so and links back.
        QString asides;
        for (const wml::HtmlNote &n : std::as_const(notes)) {
            const qint64 key = (qint64(n.endnote) << 32) | quint32(n.id);
            const QString link = QStringLiteral("<a href=\"#%1\">%2</a>").arg(n.referenceId, office::epub::xml(n.number));
            QString text;
            if (noteFirst.contains(key)) {
                text = QStringLiteral("<p>") + link + QLatin1Char(' ')
                    + office::epub::xml(QCoreApplication::translate("EpubExport", "This reference is to the note above.")) + QStringLiteral("</p>\n");
            } else {
                const QVector<wml::Block> &blocks = (n.endnote ? endnotes : footnotes).value(n.id);
                QString words = plainNote(blocks);   // (the least it comes to: with its looks only while the total stays within the bound)
                if (noteBytes + words.size() <= options.mostNoteBytes) {
                    const QString looks = html(blocks, wml::HtmlOptions(), &bookmarks);
                    if (!looks.isEmpty()) words = looks;
                }
                noteBytes += words.size();
                noteFirst.insert(key, n.noteId);
                text = words.startsWith(QLatin1String("<p>")) ? words.insert(3, link + QLatin1Char(' ')) : QStringLiteral("<p>") + link + QStringLiteral("</p>\n") + words;
            }
            asides += QStringLiteral("<aside epub:type=\"%1\" id=\"%2\">\n%3</aside>\n").arg(n.endnote ? QStringLiteral("endnote") : QStringLiteral("footnote"), n.noteId, text);
        }
        if (!asides.isEmpty()) body += QStringLiteral("<section epub:type=\"footnotes\">\n<hr/>\n") + asides + QStringLiteral("</section>\n");

        office::epub::Chapter chapter;
        for (int h = 0; h < headings.size(); ++h) {
            const bool opening = h == 0 && headings[0].level == 1 && (c > 0 || firstIsHeading);
            if (opening) chapter.title = headings[0].text;
            if (headings[h].level > kMostEpubHeadingLevel || navEntries >= options.mostNavEntries || headings[h].text.isEmpty()) continue;
            ++navEntries;
            book.nav << office::epub::NavEntry{headings[h].level, headings[h].text, QStringLiteral("chapter%1.xhtml").arg(number) + (opening ? QString() : QLatin1Char('#') + headings[h].id)};
        }
        chapter.body = body;
        book.chapters << chapter;
        bookmarksOf << bookmarks;
        if (options.progress && !options.progress(c + 1, int(starts.size()))) {
            if (error) *error = QCoreApplication::translate("EpubExport", "Cancelled.");
            return QByteArray();
        }
    }

    // A link to a bookmark goes to the chapter that holds it; one whose bookmark is not in the book is its words alone.
    QHash<QString, int> chapterOf;
    for (int c = 0; c < bookmarksOf.size(); ++c)
        for (const QString &id : std::as_const(bookmarksOf[c]))
            if (!chapterOf.contains(id)) chapterOf.insert(id, c + 1);
    static const QRegularExpression internal(QStringLiteral("<a href=\"#(bm_[^\"]*)\">(.*?)</a>"), QRegularExpression::DotMatchesEverythingOption);
    for (office::epub::Chapter &chapter : book.chapters) {
        if (!chapter.body.contains(QLatin1String("<a href=\"#bm_"))) continue;
        QString out;
        qsizetype at = 0;
        QRegularExpressionMatchIterator it = internal.globalMatch(chapter.body);
        while (it.hasNext()) {
            const QRegularExpressionMatch m = it.next();
            out += QStringView(chapter.body).mid(at, m.capturedStart() - at);
            const int target = chapterOf.value(m.captured(1));
            if (target > 0) out += QStringLiteral("<a href=\"chapter%1.xhtml#%2\">%3</a>").arg(target).arg(m.captured(1), m.captured(2));
            else out += m.captured(2);
            at = m.capturedEnd();
        }
        out += QStringView(chapter.body).mid(at);
        chapter.body = out;
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
