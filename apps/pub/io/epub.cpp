#include "io/epub.h"

#include "core/document.h"
#include "core/items.h"
#include "epub/epub.h"
#include "io/zip.h"
#include "render/renderer.h"
#include "text/textengine.h"
#include "text/textprops.h"

#include <QBuffer>
#include <QDateTime>
#include <QHash>
#include <QPainter>
#include <QSaveFile>
#include <QSet>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextList>
#include <QUuid>
#include <algorithm>
#include <cmath>

namespace jp {

namespace {

using office::epub::xml;
using office::epub::page;
using office::epub::NavEntry;

struct Chapter {
    QString title, body, asides;
};

// What a picture becomes in the book.
struct Picture {
    QString href, mediaType;
    QByteArray bytes;
};

class Book {
public:
    Book(const Document &doc, const QString &lang) : m_doc(doc), m_lang(lang) { m_fields.doc = &doc; }

    void build();
    QVector<Chapter> chapters;
    QVector<NavEntry> nav;
    QString endnotes;
    QMap<QString, Picture> pictures;        // by key, in the order first used

private:
    QString file(int chapter) const { return QStringLiteral("chapter%1.xhtml").arg(chapter + 1); }
    Chapter &current()
    {
        if (chapters.isEmpty()) chapters << Chapter();
        return chapters.last();
    }
    void items(const ItemList &list);
    void item(const Item &it);
    void story(const QString &id);
    QString blocks(const QTextDocument *d, bool inNote);
    QString inlines(const QTextBlock &b, bool inNote);
    QString noteRef(bool endnote, const QString &storyId);
    void picture(const PictureItem &pic);
    void table(const TableItem &t);

    const Document &m_doc;
    QString m_lang;
    FieldContext m_fields;
    QSet<QString> m_storiesDone;
    int m_footnotes = 0, m_endnotes = 0, m_headings = 0;
};

void Book::build()
{
    for (const auto &pg : m_doc.pages) items(pg->items);
    if (chapters.isEmpty()) chapters << Chapter();
}

// A page's objects in reading order: top to bottom, and left to right
// across objects that start at about the same height. Groups open up.
void Book::items(const ItemList &list)
{
    QVector<const Item *> flat;
    std::function<void(const ItemList &)> add = [&](const ItemList &l) {
        for (const auto &it : l) {
            if (it->hidden) continue;   // (a hidden object is left out of the book)
            if (it->type() == ItemType::Group) add(static_cast<const GroupItem &>(*it).children);
            else flat << it.get();
        }
    };
    add(list);
    std::stable_sort(flat.begin(), flat.end(), [](const Item *a, const Item *b) {
        const double ta = std::floor(a->rect.top() / 12), tb = std::floor(b->rect.top() / 12);
        return ta != tb ? ta < tb : a->rect.left() < b->rect.left();
    });
    for (const Item *it : std::as_const(flat)) item(*it);
}

void Book::item(const Item &it)
{
    switch (it.type()) {
    case ItemType::Text: story(static_cast<const TextItem &>(it).storyId); break;
    case ItemType::Shape: story(static_cast<const ShapeItem &>(it).storyId); break;
    case ItemType::Picture: picture(static_cast<const PictureItem &>(it)); break;
    case ItemType::Table: table(static_cast<const TableItem &>(it)); break;
    case ItemType::TextArt: {
        const QString text = static_cast<const TextArtItem &>(it).text.trimmed();
        if (!text.isEmpty()) current().body += QStringLiteral("<p class=\"art\">%1</p>\n").arg(xml(text));
        break;
    }
    case ItemType::Equation: {
        const QString line = static_cast<const EquationItem &>(it).linear.trimmed();
        if (!line.isEmpty()) current().body += QStringLiteral("<p class=\"equation\">%1</p>\n").arg(xml(line));
        break;
    }
    default: break;   // lines and plain shapes are decoration
    }
}

// A story once, where its first box comes (linked boxes continue it).
void Book::story(const QString &id)
{
    if (id.isEmpty() || m_storiesDone.contains(id)) return;
    m_storiesDone.insert(id);
    const QTextDocument *d = m_doc.storyDoc(id);
    if (!d) return;
    for (QTextBlock b = d->begin(); b.isValid(); b = b.next()) {
        const QTextBlockFormat bf = b.blockFormat();
        if (bf.hasProperty(tp::TocLevel)) continue;   // the book has its own contents
        const QString style = bf.stringProperty(tp::StyleName);
        int level = 0;
        if (style.startsWith(QLatin1String("Heading ")) && style.size() == 9 && style[8] >= '1' && style[8] <= '3') level = style[8].digitValue();
        // A Heading 1 starts a chapter before its own words (and any note
        // in them) go anywhere.
        if (level == 1 && !b.text().trimmed().isEmpty() && !current().body.trimmed().isEmpty()) chapters << Chapter();
        current();
        const QString text = inlines(b, false);
        if (text.trimmed().isEmpty()) continue;   // spacing paragraphs
        if (level) {
            QString plain = b.text();
            plain.remove(QChar(0xFFFC)).remove(QChar(0x00AD)).replace(QChar(0x2028), ' ');
            plain = plain.simplified();
            const QString anchor = QStringLiteral("h%1").arg(++m_headings);
            if (level == 1 && current().title.isEmpty()) current().title = plain;
            nav << NavEntry{level, plain, file(int(chapters.size()) - 1) + '#' + anchor};
            current().body += QStringLiteral("<h%1 id=\"%2\">%3</h%1>\n").arg(level).arg(anchor, text);
            continue;
        }
        QString cls;
        const Qt::Alignment a = bf.alignment() & Qt::AlignHorizontal_Mask;
        if (a & Qt::AlignHCenter) cls = QStringLiteral(" class=\"center\"");
        else if (a & Qt::AlignRight) cls = QStringLiteral(" class=\"right\"");
        else if (a & Qt::AlignJustify) cls = QStringLiteral(" class=\"justify\"");
        if (QTextList *list = b.textList()) {
            // A list's items, together, as one list.
            const QTextListFormat::Style ls = list->format().style();
            const bool numbered = ls == QTextListFormat::ListDecimal || ls == QTextListFormat::ListLowerAlpha || ls == QTextListFormat::ListUpperAlpha ||
                                  ls == QTextListFormat::ListLowerRoman || ls == QTextListFormat::ListUpperRoman;
            const bool first = list->itemNumber(b) == 0 || !b.previous().isValid() || b.previous().textList() != list;
            const bool last = !b.next().isValid() || b.next().textList() != list;
            QString &out = current().body;
            if (first) {
                const char *type = ls == QTextListFormat::ListLowerAlpha ? "a" : ls == QTextListFormat::ListUpperAlpha ? "A"
                                 : ls == QTextListFormat::ListLowerRoman ? "i" : ls == QTextListFormat::ListUpperRoman ? "I" : "1";
                const int start = list->itemNumber(b) + 1;
                out += numbered ? QStringLiteral("<ol type=\"%1\"%2>\n").arg(QLatin1String(type), start > 1 ? QStringLiteral(" start=\"%1\"").arg(start) : QString())
                                : QStringLiteral("<ul>\n");
            }
            out += QStringLiteral("<li%1>%2</li>\n").arg(cls, text);
            if (last) out += numbered ? QStringLiteral("</ol>\n") : QStringLiteral("</ul>\n");
            continue;
        }
        current().body += QStringLiteral("<p%1>%2</p>\n").arg(cls, text);
    }
}

// A note's own paragraphs (or a table cell's), with no headings or lists.
QString Book::blocks(const QTextDocument *d, bool inNote)
{
    QString out;
    if (!d) return out;
    for (QTextBlock b = d->begin(); b.isValid(); b = b.next()) {
        const QString text = inlines(b, inNote);
        if (!text.trimmed().isEmpty()) out += QStringLiteral("<p>%1</p>").arg(text);
    }
    return out;
}

QString Book::inlines(const QTextBlock &b, bool inNote)
{
    QString out;
    for (auto it = b.begin(); !it.atEnd(); ++it) {
        const QTextFragment f = it.fragment();
        if (!f.isValid()) continue;
        const QTextCharFormat cf = f.charFormat();
        QString text;
        if (cf.hasProperty(tp::Field)) {
            const QString code = cf.stringProperty(tp::Field);
            const QString kind = code.section(':', 0, 0);
            // Each U+FFFC is the field; any other letters carrying its
            // format are plain text.
            for (const QChar ch : f.text()) {
                if (ch != QChar(0xFFFC)) {
                    text += xml(QString(ch));
                } else if (kind == QLatin1String("footnote") || kind == QLatin1String("endnote")) {
                    // Notes inside notes have nowhere to go.
                    if (!inNote) text += noteRef(kind == QLatin1String("endnote"), code.section(':', 1));
                } else if (kind != QLatin1String("page") && kind != QLatin1String("pages")) {
                    // Page numbers mean nothing once the text reflows.
                    text += xml(m_fields.resolve(code));
                }
            }
            if (text.isEmpty()) continue;
        } else {
            text = xml(f.text());
            text.replace(QChar(0x2028), QLatin1String("<br/>")).replace(QChar('\t'), QChar(' ')).remove(QChar(0xFFFC));
        }
        if (cf.fontCapitalization() == QFont::SmallCaps) text = QStringLiteral("<span style=\"font-variant: small-caps\">%1</span>").arg(text);
        else if (cf.fontCapitalization() == QFont::AllUppercase) text = QStringLiteral("<span style=\"text-transform: uppercase\">%1</span>").arg(text);
        if (cf.verticalAlignment() == QTextCharFormat::AlignSuperScript) text = QStringLiteral("<sup>%1</sup>").arg(text);
        else if (cf.verticalAlignment() == QTextCharFormat::AlignSubScript) text = QStringLiteral("<sub>%1</sub>").arg(text);
        if (cf.fontStrikeOut()) text = QStringLiteral("<s>%1</s>").arg(text);
        if (cf.fontUnderline() && !cf.isAnchor()) text = QStringLiteral("<u>%1</u>").arg(text);
        if (cf.fontItalic()) text = QStringLiteral("<em>%1</em>").arg(text);
        if (cf.fontWeight() >= QFont::DemiBold) text = QStringLiteral("<strong>%1</strong>").arg(text);
        if (cf.isAnchor() && !cf.anchorHref().isEmpty()) text = QStringLiteral("<a href=\"%1\">%2</a>").arg(xml(cf.anchorHref()), text);
        out += text;
    }
    return out;
}

// A note's number in the text. Footnotes pop up from the chapter they're in;
// endnotes gather at the back, each linking back to its number.
QString Book::noteRef(bool endnote, const QString &storyId)
{
    current();
    const QString chapterFile = file(int(chapters.size()) - 1);
    const QString text = blocks(m_doc.storyDoc(storyId), true);
    if (endnote) {
        const int n = ++m_endnotes;
        endnotes += QStringLiteral("<li id=\"en%1\">%2 <a href=\"%3#enref%1\">Back</a></li>\n").arg(n).arg(text, chapterFile);
        return QStringLiteral("<a epub:type=\"noteref\" class=\"noteref\" id=\"enref%1\" href=\"notes.xhtml#en%1\">%1</a>").arg(n);
    }
    const int n = ++m_footnotes;
    current().asides += QStringLiteral("<aside epub:type=\"footnote\" id=\"fn%1\"><p><a href=\"#fnref%1\">%1.</a></p>%2</aside>\n").arg(n).arg(text);
    return QStringLiteral("<a epub:type=\"noteref\" class=\"noteref\" id=\"fnref%1\" href=\"#fn%1\">%1</a>").arg(n);
}

void Book::picture(const PictureItem &pic)
{
    if (pic.imageId.isEmpty() || !m_doc.images.contains(pic.imageId)) return;
    const ImageData &data = m_doc.images[pic.imageId];
    const QString fmt = data.format.toLower();
    // A picture used as it is keeps its own file; one that is cropped,
    // cut to a shape or adjusted is drawn as it shows on the page.
    const bool asIs = pic.brightness == 0 && pic.contrast == 0 && pic.recolor == PictureItem::NoRecolor && !pic.hasTransparentColor &&
                      pic.maskShape == QLatin1String("rect") && std::abs(pic.imgRect.left()) < 0.5 && std::abs(pic.imgRect.top()) < 0.5 &&
                      std::abs(pic.imgRect.width() - pic.rect.width()) < 0.5 && std::abs(pic.imgRect.height() - pic.rect.height()) < 0.5;
    static const QHash<QString, QString> types = {{"jpg", "image/jpeg"}, {"jpeg", "image/jpeg"}, {"png", "image/png"},
                                                  {"gif", "image/gif"}, {"svg", "image/svg+xml"}, {"webp", "image/webp"}};
    const QString key = asIs ? pic.imageId : pic.id;
    if (!pictures.contains(key)) {
        Picture out;
        if (asIs && types.contains(fmt)) {
            out.bytes = data.bytes;
            out.mediaType = types.value(fmt);
            out.href = QStringLiteral("images/pic%1.%2").arg(pictures.size() + 1).arg(fmt == QLatin1String("jpeg") ? QStringLiteral("jpg") : fmt);
        } else {
            QImage img;
            if (asIs) {
                img = data.image();
            } else {
                // At the picture's own resolution where it is known, within reason.
                const QSize px = data.pixelSize.isValid() ? data.pixelSize : QSize(1200, 1200);
                // At least a pixel a point, at most 1,600 pixels across (std::clamp
                // would need the smaller limit first).
                const double native = std::max(1.0, px.width() / std::max(1.0, pic.imgRect.width()));
                const double scale = std::min(native, 1600 / std::max(1.0, std::max(pic.rect.width(), pic.rect.height())));
                img = QImage((pic.rect.size() * scale).toSize().expandedTo(QSize(1, 1)), QImage::Format_ARGB32_Premultiplied);
                img.fill(Qt::transparent);
                QPainter p(&img);
                p.setRenderHint(QPainter::Antialiasing);
                p.setRenderHint(QPainter::SmoothPixmapTransform);
                p.scale(scale, scale);
                p.translate(-pic.rect.topLeft());
                auto flat = std::static_pointer_cast<PictureItem>(pic.clone());
                flat->rotation = 0;
                PaintContext ctx;
                ctx.doc = &m_doc;
                ctx.opt.output = true;
                Renderer::paintItem(&p, ctx, *flat);
            }
            if (img.isNull()) return;
            QBuffer buf(&out.bytes);
            buf.open(QIODevice::WriteOnly);
            // Photos as JPEG; anything see-through as PNG.
            const bool alpha = img.hasAlphaChannel() && (img.convertToFormat(QImage::Format_ARGB32).pixelColor(0, 0).alpha() < 255 || pic.maskShape != QLatin1String("rect") || fmt == QLatin1String("png") || fmt == QLatin1String("gif"));
            if (alpha) {
                img.save(&buf, "PNG");
                out.mediaType = QStringLiteral("image/png");
            } else {
                img.convertToFormat(QImage::Format_RGB32).save(&buf, "JPEG", 88);
                out.mediaType = QStringLiteral("image/jpeg");
            }
            out.href = QStringLiteral("images/pic%1.%2").arg(pictures.size() + 1).arg(alpha ? QStringLiteral("png") : QStringLiteral("jpg"));
        }
        pictures.insert(key, out);
    }
    const double widthPct = std::clamp(pic.rect.width() / std::max(1.0, m_doc.pageSize().width()) * 100 * 1.25, 20.0, 100.0);
    current().body += QStringLiteral("<figure><img src=\"%1\" alt=\"%2\" style=\"width: %3%\"/></figure>\n")
                          .arg(pictures.value(key).href, xml(pic.altText.simplified()), QString::number(qRound(widthPct)));
}

void Book::table(const TableItem &t)
{
    QString out = QStringLiteral("<table>\n");
    for (int r = 0; r < t.rows; ++r) {
        out += QStringLiteral("<tr>");
        for (int c = 0; c < t.cols; ++c) {
            const qsizetype i = qsizetype(r) * t.cols + c;
            if (i >= t.cells.size()) break;
            const TableCell &cell = t.cells[i];
            if (cell.covered) continue;
            QString span;
            if (cell.rowSpan > 1) span += QStringLiteral(" rowspan=\"%1\"").arg(cell.rowSpan);
            if (cell.colSpan > 1) span += QStringLiteral(" colspan=\"%1\"").arg(cell.colSpan);
            out += QStringLiteral("<td%1>%2%3</td>").arg(span, blocks(m_doc.storyDoc(cell.storyId), false), cell.divided() ? blocks(m_doc.storyDoc(cell.storyId2), false) : QString());
        }
        out += QStringLiteral("</tr>\n");
    }
    current().body += out + QStringLiteral("</table>\n");
}

} // namespace

bool exportEpub(const Document &doc, const QString &path, const EpubOptions &opt, QString *error)
{
    const QString title = !opt.title.isEmpty() ? opt.title : !doc.props.title.isEmpty() ? doc.props.title : QStringLiteral("Untitled");
    const QString author = !opt.author.isEmpty() ? opt.author : doc.props.author;
    QString lang = opt.language;
    if (lang.isEmpty()) {
        // The language most of the first story is marked in.
        for (const auto &pg : doc.pages)
            for (const auto &it : pg->items)
                if (lang.isEmpty() && it->type() == ItemType::Text)
                    if (const QTextDocument *d = doc.storyDoc(static_cast<const TextItem &>(*it).storyId))
                        lang = d->begin().charFormat().stringProperty(tp::Language);
        if (lang.isEmpty()) lang = QStringLiteral("en-US");
    }
    const QString id = !opt.identifier.isEmpty() ? opt.identifier : office::epub::newIdentifier();

    Book book(doc, lang);
    book.build();

    office::epub::Book out;
    out.title = title;
    out.author = author;
    out.language = lang;
    out.identifier = id;
    out.cover = opt.cover;
    for (const Chapter &c : std::as_const(book.chapters)) out.chapters << office::epub::Chapter{c.title, c.body + c.asides};
    out.nav = book.nav;
    out.endnotes = book.endnotes;
    for (auto it = book.pictures.cbegin(); it != book.pictures.cend(); ++it) out.resources << office::epub::Resource{it->href, it->mediaType, it->bytes};
    return office::epub::save(office::epub::build(out), path, error);
}


bool exportFixedEpub(const Document &doc, const QString &path, const QVector<FixedPage> &pages, const FixedEpubOptions &opt, QString *error)
{
    const QString title = !opt.title.isEmpty() ? opt.title : !doc.props.title.isEmpty() ? doc.props.title : QStringLiteral("Untitled");
    const QString author = !opt.author.isEmpty() ? opt.author : doc.props.author;
    const QString lang = opt.language.isEmpty() ? QStringLiteral("en-US") : opt.language;
    const QString id = !opt.identifier.isEmpty() ? opt.identifier : office::epub::newIdentifier();
    if (pages.isEmpty()) {
        if (error) *error = QStringLiteral("The publication has no pages.");
        return false;
    }
    // The page in CSS pixels (96 to the inch), which readers scale to the screen.
    const int w = std::max(1, int(std::lround(opt.pageSize.width() * 96 / 72))), h = std::max(1, int(std::lround(opt.pageSize.height() * 96 / 72)));

    ZipWriter zip;
    office::epub::addPackageStart(zip);
    // The contents page links the style sheet, so it is in the book and listed.
    QString manifest = QStringLiteral("<item id=\"nav\" href=\"nav.xhtml\" media-type=\"application/xhtml+xml\" properties=\"nav\"/>\n"
                                      "<item id=\"css\" href=\"style.css\" media-type=\"text/css\"/>\n");
    QString spine;
    if (!opt.cover.isNull()) {
        zip.add(QStringLiteral("OEBPS/images/cover.jpg"), office::epub::coverJpeg(opt.cover));
        manifest += QStringLiteral("<item id=\"cover-image\" href=\"images/cover.jpg\" media-type=\"image/jpeg\" properties=\"cover-image\"/>\n");
    }
    QString toc, pageList;
    for (int i = 0; i < pages.size(); ++i) {
        const QString name = QStringLiteral("page%1.xhtml").arg(i + 1), svgName = QStringLiteral("pages/page%1.svg").arg(i + 1);
        // The page's words sit under the drawing, out of sight but there for
        // search, copying, and screen readers; the drawing says nothing itself.
        QString words;
        for (const QString &para : pages[i].text.split(QLatin1Char('\n'), Qt::SkipEmptyParts))
            if (!para.trimmed().isEmpty()) words += QStringLiteral("<p>%1</p>\n").arg(xml(para.trimmed()));
        const QString body = QStringLiteral("<!DOCTYPE html>\n<html xmlns=\"http://www.w3.org/1999/xhtml\" xmlns:epub=\"http://www.idpf.org/2007/ops\" lang=\"%1\" xml:lang=\"%1\">\n"
                                            "<head><meta charset=\"utf-8\"/><title>%2</title>\n<meta name=\"viewport\" content=\"width=%3, height=%4\"/>\n"
                                            "<style>html, body { margin: 0; padding: 0; width: %3px; height: %4px; overflow: hidden; }\n"
                                            "img.page { position: absolute; left: 0; top: 0; width: %3px; height: %4px; }\n"
                                            ".words { position: absolute; left: 0; top: 0; width: 1px; height: 1px; overflow: hidden; clip-path: inset(50%); }</style></head>\n"
                                            "<body epub:type=\"bodymatter\"><img class=\"page\" src=\"%5\" alt=\"\"/>\n<div class=\"words\">\n%6</div></body></html>\n")
                                 .arg(xml(lang), xml(QStringLiteral("%1, page %2").arg(title, QString::number(i + 1))), QString::number(w), QString::number(h), svgName, words);
        zip.add(QStringLiteral("OEBPS/") + name, body.toUtf8(), true);
        zip.add(QStringLiteral("OEBPS/") + svgName, pages[i].svg, true);
        manifest += QStringLiteral("<item id=\"p%1\" href=\"%2\" media-type=\"application/xhtml+xml\"/>\n"
                                   "<item id=\"s%1\" href=\"%3\" media-type=\"image/svg+xml\"/>\n").arg(i + 1).arg(name, svgName);
        // Facing pages: the first alone on the right, then left and right.
        const QString side = opt.spreads ? (i % 2 == 0 ? QStringLiteral(" properties=\"page-spread-right\"") : QStringLiteral(" properties=\"page-spread-left\"")) : QString();
        spine += QStringLiteral("<itemref idref=\"p%1\"%2/>\n").arg(i + 1).arg(side);
        for (const auto &hd : pages[i].headings)
            toc += QStringLiteral("<li class=\"h%1\"><a href=\"%2\">%3</a></li>\n").arg(hd.level).arg(name, xml(hd.text));
        pageList += QStringLiteral("<li><a href=\"%1\">%2</a></li>\n").arg(name).arg(i + 1);
    }
    if (toc.isEmpty()) toc = QStringLiteral("<li><a href=\"page1.xhtml\">%1</a></li>\n").arg(xml(title));
    const QString navBody = QStringLiteral("<nav epub:type=\"toc\" id=\"toc\"><h1>Contents</h1>\n<ol>\n%1</ol></nav>\n"
                                           "<nav epub:type=\"page-list\" hidden=\"hidden\"><ol>\n%2</ol></nav>\n"
                                           "<nav epub:type=\"landmarks\" hidden=\"hidden\"><ol>\n<li><a epub:type=\"cover\" href=\"page1.xhtml\">Cover</a></li>\n"
                                           "<li><a epub:type=\"bodymatter\" href=\"page1.xhtml\">Start</a></li>\n</ol></nav>\n")
                                .arg(toc, pageList);
    zip.add(QStringLiteral("OEBPS/nav.xhtml"), page(QStringLiteral("Contents"), lang, navBody).toUtf8(), true);

    QString meta = office::epub::metadata(id, title, lang, author);
    meta += QStringLiteral("<meta property=\"rendition:layout\">pre-paginated</meta>\n<meta property=\"rendition:orientation\">auto</meta>\n"
                           "<meta property=\"rendition:spread\">%1</meta>\n").arg(opt.spreads ? QStringLiteral("landscape") : QStringLiteral("none"));
    if (!opt.cover.isNull()) meta += QStringLiteral("<meta name=\"cover\" content=\"cover-image\"/>\n");
    const QString opf = office::epub::packageDocument(lang, meta, manifest, spine, QStringLiteral("rendition: http://www.idpf.org/vocab/rendition/#"));
    zip.add(QStringLiteral("OEBPS/content.opf"), opf.toUtf8(), true);
    return office::epub::save(zip.finish(), path, error);
}

} // namespace jp
