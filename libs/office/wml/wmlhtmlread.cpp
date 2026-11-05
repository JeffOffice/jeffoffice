#include "wml/wmlhtmlread.h"

#include <QCoreApplication>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextFrame>
#include <QTextList>
#include <QTextTable>
#include <QTextTableCell>

#include <algorithm>

namespace jp::office::wml {

namespace {

QByteArray escapedAttr(const QString &s) { return s.toHtmlEscaped().toUtf8(); }

struct Reader {
    Document *doc;
    int blocks = 0;

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
        QString marker;
        if (QTextList *list = b.textList()) {
            const int depth = std::clamp(list->format().indent(), 1, 9);
            blk.para.props.indLeft = 360 * depth + 360;
            blk.para.props.indFirst = -360;
            const QTextListFormat::Style style = list->format().style();
            if (style == QTextListFormat::ListDecimal || style == QTextListFormat::ListLowerAlpha || style == QTextListFormat::ListUpperAlpha || style == QTextListFormat::ListLowerRoman || style == QTextListFormat::ListUpperRoman)
                marker = list->itemText(b) + QLatin1Char('\t');
            else marker = QString(QChar(0x2022)) + QLatin1Char('\t');
        }
        if (!marker.isEmpty()) {
            Inline m;
            m.kind = Inline::Text;
            m.text = marker;
            blk.para.content.push_back(m);
        }
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment fr = it.fragment();
            const QTextCharFormat cf = fr.charFormat();
            QString text = fr.text();
            text.remove(QChar::ObjectReplacementCharacter);
            text.replace(QChar::ParagraphSeparator, QChar(QChar::LineSeparator));
            if (text.isEmpty()) continue;
            Inline in;
            in.kind = Inline::Text;
            in.text = text;
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

bool readHtml(const QByteArray &html, Document *out, QString *error)
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
    QVector<Block> body;
    r.frameContent(page.rootFrame()->begin(), page.rootFrame()->end(), &body, 0);
    if (body.isEmpty()) body.push_back(Block());
    d.body = body;
    *out = std::move(d);
    return true;
}

} // namespace jp::office::wml
