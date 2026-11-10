#include "io/pdftags.h"

#include "io/qtpdf.h"

#include <QHash>
#include <QPainter>
#include <QRegularExpression>

#include <algorithm>

namespace jp {

namespace {

// A mark: a small square far off the page in device units, its place telling which tag, and whether it opens or closes it.
void drawMark(QPainter *p, int tag, bool opens)
{
    p->save();
    p->setWorldTransform(QTransform());
    p->setPen(Qt::NoPen);
    p->setBrush(Qt::black);
    p->setOpacity(1);
    p->drawRect(QRectF(-4000.0 - 8.0 * tag, opens ? -4000 : -4100, 4, 4));
    p->restore();
}

// The text of a string as a PDF string: UTF-16 with its byte order mark, in hexadecimal.
QByteArray pdfText(const QString &text)
{
    QByteArray bytes("\xFE\xFF", 2);
    for (const QChar c : text.left(1000)) {
        bytes.append(char(c.unicode() >> 8));
        bytes.append(char(c.unicode() & 0xFF));
    }
    return '<' + bytes.toHex().toUpper() + '>';
}

// Whether a piece of a page's content draws anything (text, a path painted, a picture, a shading).
bool draws(const QByteArray &part)
{
    static const QRegularExpression ops(QStringLiteral("(^|[\\s\\]>)])(Tj|TJ|f|f\\*|F|S|s|B|B\\*|b|b\\*|Do|sh|BI)(\\s|$)"));
    return ops.match(QString::fromLatin1(part)).hasMatch();
}

struct Spot {
    int page = -1, mcid = -1;
};

} // namespace

int PdfTagger::add(const PdfTag &tag)
{
    m_tags << tag;
    return int(m_tags.size()) - 1;
}

void PdfTagger::begin(QPainter *p, int id) { drawMark(p, id, true); }

void PdfTagger::end(QPainter *p, int id) { drawMark(p, id, false); }

QByteArray tagPdf(const QByteArray &pdf, const PdfTagger &tagger, const PdfTagOptions &options)
{
    const QVector<PdfTag> &tags = tagger.tags();
    if (tags.isEmpty()) return pdf;
    QtPdf q;
    if (!q.loadBytes(pdf)) return pdf;
    const int root = QtPdf::ref(q.trailer, "/Root");
    QtPdf::Obj *catalog = root ? q.object(root) : nullptr;
    if (!catalog || catalog->body.contains("/StructTreeRoot")) return pdf;
    QtPdf::Obj *pagesObj = q.object(QtPdf::ref(catalog->body, "/Pages"));
    if (!pagesObj) return pdf;
    // The pages, in order: the kids must be pages themselves.
    QVector<int> pageIds;
    {
        const int kids = pagesObj->body.indexOf("/Kids");
        const int open = pagesObj->body.indexOf('[', kids), close = pagesObj->body.indexOf(']', kids);
        if (kids < 0 || open < 0 || close < open) return pdf;
        static const QRegularExpression ref(QStringLiteral("(\\d+)\\s+0\\s+R"));
        const QString list = QString::fromLatin1(pagesObj->body.mid(open + 1, close - open - 1));
        for (auto it = ref.globalMatch(list); it.hasNext();) pageIds << it.next().captured(1).toInt();
        for (int id : pageIds) {
            const QtPdf::Obj *o = q.object(id);
            if (!o || o->body.contains("/Kids")) return pdf;
        }
    }
    // The marks in each page's content become marked content.
    static const QRegularExpression mark(QStringLiteral("-(\\d+)(?:\\.\\d+)? -(4000|4100)(?:\\.\\d+)? 4 4 re\\nf\\n"));
    QVector<Spot> spots(tags.size());
    QVector<QVector<int>> mcids(pageIds.size());   // per page, the tag of each marked content in order
    bool any = false;
    for (int k = 0; k < pageIds.size(); ++k) {
        QtPdf::Obj *page = q.object(pageIds[k]);
        const int contents = QtPdf::ref(page->body, "/Contents");
        QtPdf::Obj *co = contents ? q.object(contents) : nullptr;
        if (!co || !QtPdf::isStream(co->body)) continue;
        bool ok = false;
        const QByteArray data = q.streamData(*co, &ok);
        if (!ok || !data.contains("4 4 re")) continue;
        const QString text = QString::fromLatin1(data);
        QByteArray out;
        qsizetype at = 0;
        struct Open {
            int tag;
            qsizetype outAt;   // where its marked content starts in `out`
            qsizetype dataAt;  // and in the data, after its mark
        };
        QVector<Open> open;
        bool changed = false;
        for (auto it = mark.globalMatch(text); it.hasNext();) {
            const auto m = it.next();
            const int id = m.captured(1).toInt();
            const bool opens = m.captured(2) == QLatin1String("4000");
            // The mark is the block the painter wrote it in: from the "q" line before to the "Q" line after.
            auto lineIs = [&](qsizetype p, char c) { return p >= 0 && p + 1 < data.size() && data[p] == c && data[p + 1] == '\n' && (p == 0 || data[p - 1] == '\n'); };
            qsizetype from = m.capturedStart(), to = m.capturedEnd();
            while (from >= at && !lineIs(from, 'q')) --from;
            while (to < data.size() && !lineIs(to, 'Q')) ++to;
            const int n = id - 4000;
            if (n < 0 || n % 8 != 0 || (n >> 3) >= tags.size() || from < at || to >= data.size()) continue;
            to += 2;
            out += data.mid(at, from - at);
            at = to;
            changed = true;
            const int tag = n >> 3;
            const bool artifact = tags[tag].type == QLatin1String("Artifact");
            if (opens) {
                open << Open{tag, out.size(), to};
                out += "%OPEN\n";
            } else if (!open.isEmpty() && open.last().tag == tag) {
                const Open o = open.takeLast();
                const QByteArray inside = data.mid(o.dataAt, from - o.dataAt);
                // (what is between the marks was copied to `out` after the placeholder)
                if (!draws(inside)) {
                    out.remove(o.outAt, 6);
                    continue;
                }
                QByteArray head;
                if (artifact) head = "/Artifact BMC\n";
                else {
                    head = "/" + tags[tag].type.toLatin1() + " <</MCID " + QByteArray::number(int(mcids[k].size())) + ">> BDC\n";
                    spots[tag] = {k, int(mcids[k].size())};
                    mcids[k] << tag;
                    any = true;
                }
                out.replace(o.outAt, 6, head);
                out += "EMC\n";
            }
        }
        if (!changed) continue;
        out += data.mid(at);
        out.replace("%OPEN\n", "");   // (a mark that was never closed)
        QByteArray dict = QtPdf::dictOf(co->body);
        q.setStream(*co, dict, dict.contains("/FlateDecode") ? QtPdf::deflate(out) : out);
    }
    if (!any) return pdf;
    // The tags that stay: those with marked content, and the ones that hold them.
    QVector<bool> keep(tags.size(), false);
    for (int i = 0; i < tags.size(); ++i) keep[i] = tags[i].type != QLatin1String("Artifact") && tags[i].content && spots[i].page >= 0;
    for (int i = int(tags.size()) - 1; i >= 0; --i)
        if (keep[i] && tags[i].parent >= 0) keep[tags[i].parent] = true;
    QVector<int> objectOf(tags.size(), 0);
    const int treeRoot = q.addObject("<< >>");
    const int document = q.addObject("<< >>");
    for (int i = 0; i < tags.size(); ++i)
        if (keep[i]) objectOf[i] = q.addObject("<< >>");
    auto name = [](const QString &type) {
        QString n;
        for (const QChar c : type)
            if (c.isLetterOrNumber() && c.unicode() < 128) n += c;
        return "/" + (n.isEmpty() ? QByteArray("Span") : n.toLatin1());
    };
    QVector<QByteArray> children(tags.size());
    QByteArray top;
    for (int i = 0; i < tags.size(); ++i) {
        if (!keep[i]) continue;
        const QByteArray ref = QByteArray::number(objectOf[i]) + " 0 R ";
        if (tags[i].parent >= 0) children[tags[i].parent] += ref;
        else top += ref;
    }
    for (int i = 0; i < tags.size(); ++i) {
        if (!keep[i]) continue;
        const int parent = tags[i].parent >= 0 ? objectOf[tags[i].parent] : document;
        QByteArray d = "<< /Type /StructElem /S " + name(tags[i].type) + " /P " + QByteArray::number(parent) + " 0 R";
        if (!tags[i].alt.isEmpty()) d += " /Alt " + pdfText(tags[i].alt);
        if (spots[i].page >= 0) d += " /Pg " + QByteArray::number(pageIds[spots[i].page]) + " 0 R /K " + QByteArray::number(spots[i].mcid);
        else d += " /K [" + children[i].trimmed() + "]";
        d += " >>\n";
        q.object(objectOf[i])->body = d;
    }
    q.object(document)->body = "<< /Type /StructElem /S /Document /P " + QByteArray::number(treeRoot) + " 0 R /K [" + top.trimmed() + "] >>\n";
    // The number tree that finds an element from its marked content: one entry for each page that has some.
    QByteArray nums;
    for (int k = 0; k < pageIds.size(); ++k) {
        if (mcids[k].isEmpty()) continue;
        nums += QByteArray::number(k) + " [";
        for (int tag : std::as_const(mcids[k])) nums += QByteArray::number(objectOf[tag]) + " 0 R ";
        nums = nums.trimmed() + "] ";
        QtPdf::Obj *page = q.object(pageIds[k]);
        const qsizetype close = page->body.lastIndexOf(">>");
        if (close >= 0) page->body.insert(close, " /StructParents " + QByteArray::number(k) + " /Tabs /S ");
    }
    q.object(treeRoot)->body = "<< /Type /StructTreeRoot /K [" + QByteArray::number(document) + " 0 R] /ParentTree << /Nums [" + nums.trimmed() + "] >> /ParentTreeNextKey " +
                               QByteArray::number(int(pageIds.size())) + " >>\n";
    QByteArray extra = " /StructTreeRoot " + QByteArray::number(treeRoot) + " 0 R /MarkInfo << /Marked true >>";
    QString lang;
    for (const QChar c : options.language)
        if (c.isLetterOrNumber() && c.unicode() < 128 || c == QLatin1Char('-')) lang += c;
    catalog = q.object(root);
    if (!lang.isEmpty() && !catalog->body.contains("/Lang")) extra += " /Lang (" + lang.toLatin1() + ")";
    if (!options.title.isEmpty() && !catalog->body.contains("/ViewerPreferences")) extra += " /ViewerPreferences << /DisplayDocTitle true >>";
    // (objects were added: the catalog is looked up again)
    catalog = q.object(root);
    const qsizetype close = catalog->body.lastIndexOf(">>");
    if (close < 0) return pdf;
    catalog->body.insert(close, extra + " ");
    return q.toBytes();
}

} // namespace jp
