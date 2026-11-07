#include "wml/wmlhtmlread.h"

#include <QBuffer>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QImageReader>
#include <QTextImageFormat>
#include <QUrl>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextFrame>
#include <QTextList>
#include <QTextTable>
#include <QTextTableCell>

#include <algorithm>
#include <climits>

namespace jp::office::wml {

namespace {

QByteArray escapedAttr(const QString &s) { return s.toHtmlEscaped().toUtf8(); }

const QString kRelBase = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");

struct Reader {
    Document *doc;
    int blocks = 0;
    QString baseDir;
    int images = 0, links = 0, nextNumId = 3, nextDrawingId = 1;
    qint64 imageBytes = 0;
    QHash<const QTextList *, int> listNums;
    QHash<QString, QString> linkIds;   // an address, and the relationship that has it

    // The bytes of a picture an address names: a data: address (base64), or a file under the page's folder that a relative or file: address names. Never the network.
    bool pictureBytes(const QString &src, QByteArray *bytes) const
    {
        if (src.startsWith(QLatin1String("data:"), Qt::CaseInsensitive)) {
            const int comma = src.indexOf(QLatin1Char(','));
            if (comma < 0 || comma > 200 || !src.left(comma).contains(QLatin1String(";base64"), Qt::CaseInsensitive)) return false;
            if (src.size() > kMostHtmlImageBytes * 4 / 3 + 300) return false;
            *bytes = QByteArray::fromBase64(src.mid(comma + 1).toLatin1());
            return !bytes->isEmpty();
        }
        if (baseDir.isEmpty()) return false;
        QString path;
        const int scheme = src.indexOf(QLatin1Char(':'));
        if (scheme > 0 && scheme <= 6 && src.left(scheme).compare(QLatin1String("file"), Qt::CaseInsensitive) == 0) {
            path = QUrl(src).toLocalFile();
        } else if (scheme > 0 && scheme > 1) {
            return false;   // (http, ftp, and any other address)
        } else if (scheme == 1 || src.startsWith(QLatin1Char('/')) || src.startsWith(QLatin1Char('\\'))) {
            return false;   // (a drive or an absolute path: not beside the page)
        } else {
            QString rel = src;
            const int cut = rel.indexOf(QRegularExpression(QStringLiteral("[?#]")));
            if (cut >= 0) rel.truncate(cut);
            path = baseDir + QLatin1Char('/') + QUrl::fromPercentEncoding(rel.toUtf8());
        }
        if (path.isEmpty()) return false;
        const QString root = QDir(baseDir).canonicalPath();
        const QFileInfo info(path);
        const QString real = info.canonicalFilePath();
        if (root.isEmpty() || real.isEmpty() || !real.startsWith(root + QLatin1Char('/')) || !info.isFile() || info.size() > kMostHtmlImageBytes) return false;
        QFile f(real);
        if (!f.open(QIODevice::ReadOnly)) return false;
        *bytes = f.read(kMostHtmlImageBytes + 1);
        return !bytes->isEmpty() && bytes->size() <= kMostHtmlImageBytes;
    }

    // A picture as an inline drawing of the page's own: the media part, its relationship, and the XML (empty when it is not a picture that can be shown, or a limit is met).
    QByteArray drawing(const QString &src, double width, double height)
    {
        if (images >= kMostHtmlImages) return QByteArray();
        QByteArray bytes;
        if (!pictureBytes(src, &bytes) || imageBytes + bytes.size() > 4 * kMostHtmlImageBytes) return QByteArray();
        QString ext, type;
        if (bytes.startsWith("\x89PNG\r\n\x1a\n")) ext = QStringLiteral("png"), type = QStringLiteral("image/png");
        else if (bytes.startsWith("\xFF\xD8\xFF")) ext = QStringLiteral("jpeg"), type = QStringLiteral("image/jpeg");
        else if (bytes.startsWith("GIF8")) ext = QStringLiteral("gif"), type = QStringLiteral("image/gif");
        else if (bytes.startsWith("BM")) ext = QStringLiteral("bmp"), type = QStringLiteral("image/bmp");
        else return QByteArray();
        QBuffer buffer(&bytes);
        buffer.open(QIODevice::ReadOnly);
        const QSize natural = QImageReader(&buffer).size();
        if (natural.width() <= 0 || natural.height() <= 0 || natural.width() > 30000 || natural.height() > 30000 || qint64(natural.width()) * natural.height() > 100000000) return QByteArray();
        // The size the page gives it (pixels, 96 to the inch), else its own; no wider than the text and no taller than a page.
        double w = width > 0 ? width : natural.width(), h = height > 0 ? height : natural.height();
        if (width > 0 && height <= 0) h = natural.height() * width / natural.width();
        if (height > 0 && width <= 0) w = natural.width() * height / natural.height();
        w = std::max(1.0, w) * 0.75;
        h = std::max(1.0, h) * 0.75;   // (points)
        const double scale = std::min({1.0, 468.0 / w, 648.0 / h});
        const qint64 cx = qint64(w * scale * 12700), cy = qint64(h * scale * 12700);
        ++images;
        imageBytes += bytes.size();
        const QString name = QStringLiteral("/word/media/image%1.%2").arg(images).arg(ext);
        const QString rid = QStringLiteral("rIdHtmlImage%1").arg(images);
        doc->parts.push_back({name, type, bytes});
        doc->mainRelationships.push_back({rid, kRelBase + QStringLiteral("image"), name.mid(6), false});
        auto n = [](qint64 v) { return QByteArray::number(v); };
        const int id = nextDrawingId++;
        return "<w:drawing xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" "
               "xmlns:pic=\"http://schemas.openxmlformats.org/drawingml/2006/picture\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">"
               "<wp:inline distT=\"0\" distB=\"0\" distL=\"0\" distR=\"0\"><wp:extent cx=\"" + n(cx) + "\" cy=\"" + n(cy) + "\"/><wp:docPr id=\"" + n(id) + "\" name=\"Picture " + n(id) +
               "\"/><wp:cNvGraphicFramePr><a:graphicFrameLocks noChangeAspect=\"1\"/></wp:cNvGraphicFramePr><a:graphic><a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/picture\">"
               "<pic:pic><pic:nvPicPr><pic:cNvPr id=\"" + n(id) + "\" name=\"Picture " + n(id) + "\"/><pic:cNvPicPr/></pic:nvPicPr><pic:blipFill><a:blip r:embed=\"" + rid.toLatin1() +
               "\"/><a:stretch><a:fillRect/></a:stretch></pic:blipFill><pic:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"" + n(cx) + "\" cy=\"" + n(cy) +
               "\"/></a:xfrm><a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom></pic:spPr></pic:pic></a:graphicData></a:graphic></wp:inline></w:drawing>";
    }

    // The relationship of a link that goes to the web or to a mail address (http, https, mailto); empty for any other address.
    QString linkId(const QString &href)
    {
        const QUrl url(href.trimmed());
        const QString scheme = url.scheme().toLower();
        if (scheme != QLatin1String("http") && scheme != QLatin1String("https") && scheme != QLatin1String("mailto")) return QString();
        const QString address = url.toEncoded().left(2000);
        const auto done = linkIds.constFind(address);
        if (done != linkIds.constEnd()) return done.value();
        if (links >= kMostHtmlLinks) return QString();
        const QString rid = QStringLiteral("rIdHtmlLink%1").arg(++links);
        doc->mainRelationships.push_back({rid, kRelBase + QStringLiteral("hyperlink"), QString::fromUtf8(QUrl::fromEncoded(address.toUtf8()).toString(QUrl::FullyDecoded).toUtf8()), true});
        linkIds.insert(address, rid);
        return rid;
    }

    // The list a paragraph is in: bullets are the document's first list; each numbered list is a numbering of its own that begins again at 1.
    int listFor(const QTextList *list, bool ordered)
    {
        if (!ordered) return 1;
        const auto done = listNums.constFind(list);
        if (done != listNums.constEnd()) return done.value();
        if (listNums.size() >= 2000) return 2;
        Num num;
        num.id = nextNumId++;
        num.abstractId = 1;
        for (int level = 0; level < 9; ++level) num.startOverrides.insert(level, 1);
        doc->numbering.nums.push_back(num);
        listNums.insert(list, num.id);
        return num.id;
    }

    void paragraph(const QTextBlock &b, QVector<Block> *out)
    {
        if (++blocks > kMostHtmlBlocks) return;
        Block blk;
        const QTextBlockFormat bf = b.blockFormat();
        const int level = bf.headingLevel();
        if (level >= 1 && level <= 6) blk.para.props.style = QStringLiteral("Heading%1").arg(level);
        const Qt::Alignment al = bf.alignment();
        if (al & Qt::AlignHCenter) blk.para.props.jc = QStringLiteral("center");
        else if (al & Qt::AlignRight) blk.para.props.jc = QStringLiteral("right");
        else if (al & Qt::AlignJustify) blk.para.props.jc = QStringLiteral("both");
        if (QTextList *list = b.textList()) {
            const int depth = std::clamp(list->format().indent(), 1, 9);
            const QTextListFormat::Style style = list->format().style();
            const bool ordered = style == QTextListFormat::ListDecimal || style == QTextListFormat::ListLowerAlpha || style == QTextListFormat::ListUpperAlpha || style == QTextListFormat::ListLowerRoman || style == QTextListFormat::ListUpperRoman;
            blk.para.props.numId = listFor(list, ordered);
            blk.para.props.ilvl = depth - 1;
        }
        QString openHref;
        auto closeLink = [&] {
            if (openHref.isEmpty()) return;
            Inline close;
            close.kind = Inline::ParaAtom;
            close.role = Inline::Close;
            close.tag = QStringLiteral("hyperlink");
            close.raw = "</w:hyperlink>";
            blk.para.content.push_back(close);
            openHref.clear();
        };
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment fr = it.fragment();
            const QTextCharFormat cf = fr.charFormat();
            QString rid;
            const QString href = cf.isAnchor() ? cf.anchorHref() : QString();
            if (href != openHref) {
                closeLink();
                if (!href.isEmpty()) rid = linkId(href);
                if (!rid.isEmpty()) {
                    Inline open;
                    open.kind = Inline::ParaAtom;
                    open.role = Inline::Open;
                    open.tag = QStringLiteral("hyperlink");
                    open.raw = "<w:hyperlink r:id=\"" + rid.toLatin1() + "\" w:history=\"1\">";
                    blk.para.content.push_back(open);
                    openHref = href;
                }
            }
            if (cf.isImageFormat()) {
                const QTextImageFormat image = cf.toImageFormat();
                const QByteArray xml = drawing(image.name(), image.width(), image.height());
                if (!xml.isEmpty()) {
                    Inline in;
                    in.kind = Inline::RunAtom;
                    in.tag = QStringLiteral("drawing");
                    in.raw = xml;
                    blk.para.content.push_back(in);
                }
                continue;
            }
            QString text = fr.text();
            text.remove(QChar::ObjectReplacementCharacter);
            text.replace(QChar::ParagraphSeparator, QChar(QChar::LineSeparator));
            if (text.isEmpty()) continue;
            Inline in;
            in.kind = Inline::Text;
            in.text = text;
            if (!openHref.isEmpty()) in.props.style = QStringLiteral("Hyperlink");
            if (cf.fontWeight() >= QFont::Bold && level == 0) in.props.bold = true;
            if (cf.fontItalic()) in.props.italic = true;
            if (cf.fontUnderline() && !cf.isAnchor()) {
                Underline u;
                u.val = QStringLiteral("single");
                in.props.underline = u;
            }
            if (cf.fontStrikeOut()) in.props.strike = true;
            if (cf.verticalAlignment() == QTextCharFormat::AlignSuperScript) in.props.vertAlign = QStringLiteral("superscript");
            else if (cf.verticalAlignment() == QTextCharFormat::AlignSubScript) in.props.vertAlign = QStringLiteral("subscript");
            if (!cf.isAnchor() && cf.hasProperty(QTextFormat::ForegroundBrush) && cf.foreground().style() == Qt::SolidPattern) {
                const QColor c = cf.foreground().color();
                if (c != Qt::black) {
                    Color col;
                    col.val = c.name().mid(1).toUpper();
                    in.props.color = col;
                }
            }
            blk.para.content.push_back(in);
        }
        closeLink();
        out->push_back(blk);
    }

    static Block part(const QString &tag, Inline::Role role, const QByteArray &raw, std::shared_ptr<const TableMark> mark = nullptr)
    {
        Block b;
        b.isRaw = true;
        b.role = role;
        b.tag = tag;
        b.raw = raw;
        b.table = std::move(mark);
        return b;
    }

    void frameContent(QTextFrame::iterator begin, QTextFrame::iterator end, QVector<Block> *out, int depth)
    {
        for (QTextFrame::iterator it = begin; it != end; ++it) {
            if (blocks > kMostHtmlBlocks) return;
            if (it.currentBlock().isValid()) {
                paragraph(it.currentBlock(), out);
            } else if (auto *table = qobject_cast<QTextTable *>(it.currentFrame())) {
                if (depth < kMaxTableDepth) tableBlocks(table, out, depth + 1);
            }
        }
    }

    void tableBlocks(QTextTable *t, QVector<Block> *out, int depth)
    {
        const int rows = t->rows(), cols = t->columns();
        if (rows <= 0 || cols <= 0 || qint64(rows) * cols > kMaxTableCells) return;
        auto tm = std::make_shared<TableMark>();
        tm->kind = TableMark::Table;
        QByteArray grid = "<w:tblGrid>";
        const int each = 9000 / cols;
        for (int c = 0; c < cols; ++c) {
            tm->grid << each;
            grid += "<w:gridCol w:w=\"" + QByteArray::number(each) + "\"/>";
        }
        grid += "</w:tblGrid>";
        const QByteArray props = "<w:tblPr><w:tblW w:w=\"0\" w:type=\"auto\"/>" + QByteArray(t->format().border() > 0 ? "<w:tblBorders><w:top w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/><w:left w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/><w:bottom w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/><w:right w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/><w:insideH w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/><w:insideV w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/></w:tblBorders>" : "") + "</w:tblPr>";
        if (t->format().border() > 0) {
            TableBorder b;
            b.val = QStringLiteral("single");
            b.size = 4;
            tm->top = tm->left = tm->bottom = tm->right = tm->insideH = tm->insideV = b;
        }
        QByteArray open = "<w:tbl>" + props;
        tm->gridBegin = open.size();
        open += grid;
        tm->gridEnd = open.size();
        out->push_back(part(QStringLiteral("tbl"), Inline::Open, open, tm));
        for (int r = 0; r < rows; ++r) {
            auto rm = std::make_shared<TableMark>();
            rm->kind = TableMark::Row;
            out->push_back(part(QStringLiteral("tr"), Inline::Open, "<w:tr>", rm));
            for (int c = 0; c < cols; ++c) {
                const QTextTableCell cell = t->cellAt(r, c);
                if (!cell.isValid() || cell.row() != r || cell.column() != c) continue;
                auto cm = std::make_shared<TableMark>();
                cm->kind = TableMark::Cell;
                cm->span = std::max(1, cell.columnSpan());
                QByteArray tc = "<w:tc><w:tcPr><w:tcW w:w=\"" + QByteArray::number(each * cm->span) + "\" w:type=\"dxa\"/>";
                if (cm->span > 1) tc += "<w:gridSpan w:val=\"" + QByteArray::number(cm->span) + "\"/>";
                tc += "</w:tcPr>";
                out->push_back(part(QStringLiteral("tc"), Inline::Open, tc, cm));
                const qsizetype before = out->size();
                frameContent(cell.begin(), cell.end(), out, depth);
                // A cell holds a paragraph, or a table followed by one: an empty cell is given an empty paragraph.
                bool content = false;
                for (qsizetype i = before; i < out->size() && !content; ++i) content = !(*out)[i].isRaw;
                if (!content) out->push_back(Block());
                out->push_back(part(QStringLiteral("tc"), Inline::Close, "</w:tc>"));
            }
            out->push_back(part(QStringLiteral("tr"), Inline::Close, "</w:tr>"));
        }
        out->push_back(part(QStringLiteral("tbl"), Inline::Close, "</w:tbl>"));
        // (A paragraph after a table keeps it from joining what follows.)
        out->push_back(Block());
    }
};

} // namespace

bool looksLikeHtml(const QByteArray &bytes)
{
    QByteArray head = bytes.left(512);
    if (head.startsWith("\xEF\xBB\xBF")) head.remove(0, 3);
    const QByteArray t = head.trimmed().toLower();
    return t.startsWith("<!doctype html") || t.startsWith("<html") || t.startsWith("<head") || t.startsWith("<body");
}

QVector<HtmlTable> readHtmlTables(const QByteArray &html, bool *tooMany, QString *error)
{
    QVector<HtmlTable> tables;
    if (tooMany) *tooMany = false;
    if (html.size() > kMostHtmlBytes) {
        if (error) *error = QCoreApplication::translate("jp::office::wml", "The web page is too large to open.");
        return tables;
    }
    QTextDocument page;
    page.setUndoRedoEnabled(false);
    page.setHtml(QString::fromUtf8(html));
    qint64 cells = 0;
    for (QTextFrame::iterator it = page.rootFrame()->begin(); !it.atEnd(); ++it) {
        auto *table = qobject_cast<QTextTable *>(it.currentFrame());
        if (!table) continue;
        if (tables.size() >= kMostHtmlTables) {
            if (tooMany) *tooMany = true;
            break;
        }
        const int rows = table->rows(), cols = table->columns();
        if (rows <= 0 || cols <= 0 || cells + qint64(rows) * cols > 4 * qint64(kMaxTableCells)) continue;
        cells += qint64(rows) * cols;
        HtmlTable out;
        for (int r = 0; r < rows; ++r) {
            QStringList row;
            for (int c = 0; c < cols; ++c) {
                const QTextTableCell cell = table->cellAt(r, c);
                QString text;
                if (cell.isValid() && cell.row() == r && cell.column() == c) {
                    QTextCursor cursor = cell.firstCursorPosition();
                    cursor.setPosition(cell.lastCursorPosition().position(), QTextCursor::KeepAnchor);
                    text = cursor.selectedText();
                    text.replace(QChar(0x2029), QLatin1Char('\n')).replace(QChar(0x2028), QLatin1Char('\n'));
                    text = text.trimmed();
                }
                row << text;
            }
            out.rows.push_back(row);
        }
        tables.push_back(out);
    }
    return tables;
}

bool readHtml(const QByteArray &html, Document *out, QString *error, const QString &baseDir)
{
    if (html.size() > kMostHtmlBytes) {
        if (error) *error = QCoreApplication::translate("jp::office::wml", "The web page is too large to open.");
        return false;
    }
    QTextDocument page;
    page.setUndoRedoEnabled(false);
    page.setHtml(QString::fromUtf8(html));
    Document d = newDocument();
    Reader r{&d};
    r.baseDir = baseDir;
    for (const Num &n : d.numbering.nums) r.nextNumId = std::max(r.nextNumId, n.id < INT_MAX ? n.id + 1 : n.id);
    QVector<Block> body;
    r.frameContent(page.rootFrame()->begin(), page.rootFrame()->end(), &body, 0);
    if (body.isEmpty()) body.push_back(Block());
    d.body = body;
    *out = std::move(d);
    return true;
}

} // namespace jp::office::wml
