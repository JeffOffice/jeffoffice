#include "io/pdftags.h"

#include "io/qtpdf.h"

#include <QCoreApplication>
#include <QHash>
#include <QPainter>
#include <QRandomGenerator>
#include <QRegularExpression>

#include <algorithm>
#include <cmath>

namespace jp {

namespace {

// A mark: a small square far off the page in device units, its place telling which tag, and whether it opens or closes it.
void drawMark(QPainter *p, int tag, bool opens, double w, double h)
{
    p->save();
    p->setWorldTransform(QTransform());
    p->setPen(Qt::NoPen);
    p->setBrush(Qt::black);
    p->setOpacity(1);
    p->drawRect(QRectF(-4000.0 - 8.0 * tag, opens ? -4000 : -4100, w, h));
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

// The squares drawn as marks look like this in a page's content: "x y w h re", then "f".
const QRegularExpression &markPattern()
{
    static const QRegularExpression re(QStringLiteral("(-?\\d+(?:\\.\\d+)?) (-?\\d+(?:\\.\\d+)?) (\\d+(?:\\.\\d+)?) (\\d+(?:\\.\\d+)?) re\\nf\\n"));
    return re;
}

// Whether a match of the pattern is one of this export's marks: its size is the one drawn at random for it, and it is where a mark goes.
bool isMark(const QRegularExpressionMatch &m, const PdfTagger &t, int *tag, bool *opens)
{
    const double x = m.captured(1).toDouble(), y = m.captured(2).toDouble();
    if (std::abs(m.captured(3).toDouble() - t.markWidth()) > 2e-4 || std::abs(m.captured(4).toDouble() - t.markHeight()) > 2e-4) return false;
    const double n = -x - 4000;
    if (n < -0.5 || std::abs(n / 8 - std::round(n / 8)) > 1e-3 || (std::abs(y + 4000) > 1e-3 && std::abs(y + 4100) > 1e-3)) return false;
    *tag = int(std::lround(n / 8));
    *opens = std::abs(y + 4000) < 1e-3;
    return *tag >= 0 && *tag < t.tags().size();
}

} // namespace

PdfTagger::PdfTagger()
{
    m_w = 1 + QRandomGenerator::global()->bounded(1000) / 1000.0;
    m_h = 1 + QRandomGenerator::global()->bounded(1000) / 1000.0;
}

int &pdfTagTestFailure()
{
    static int failure = 0;
    return failure;
}

int PdfTagger::add(const PdfTag &tag)
{
    m_tags << tag;
    return int(m_tags.size()) - 1;
}

void PdfTagger::begin(QPainter *p, int id) { drawMark(p, id, true, m_w, m_h); }

void PdfTagger::end(QPainter *p, int id) { drawMark(p, id, false, m_w, m_h); }

PdfTagResult tagPdf(const QByteArray &pdf, const PdfTagger &tagger, const PdfTagOptions &options)
{
    const QVector<PdfTag> &tags = tagger.tags();
    auto fail = [](const QString &why) { return PdfTagResult{QByteArray(), false, why}; };
    const int forced = pdfTagTestFailure();
    if (forced == 1) return fail(QCoreApplication::translate("PdfTags", "test"));
    QtPdf q;
    if (!q.loadBytes(pdf)) return fail(QCoreApplication::translate("PdfTags", "the PDF is in a form that can't be tagged"));
    const int root = QtPdf::ref(q.trailer, "/Root");
    QtPdf::Obj *catalog = root ? q.object(root) : nullptr;
    if (!catalog) return fail(QCoreApplication::translate("PdfTags", "the PDF has no catalog"));
    // (a PDF that is tagged already is not tagged again: the marks are only taken out)
    const bool already = forced == 2 || catalog->body.contains("/StructTreeRoot");
    QtPdf::Obj *pagesObj = q.object(QtPdf::ref(catalog->body, "/Pages"));
    if (!pagesObj) return fail(QCoreApplication::translate("PdfTags", "the PDF has no pages"));
    // The pages, in order: the kids must be pages themselves.
    QVector<int> pageIds;
    {
        const int kids = pagesObj->body.indexOf("/Kids");
        const int open = pagesObj->body.indexOf('[', kids), close = pagesObj->body.indexOf(']', kids);
        if (kids < 0 || open < 0 || close < open) return fail(QCoreApplication::translate("PdfTags", "the pages are in a form that can't be tagged"));
        static const QRegularExpression ref(QStringLiteral("(\\d+)\\s+0\\s+R"));
        const QString list = QString::fromLatin1(pagesObj->body.mid(open + 1, close - open - 1));
        for (auto it = ref.globalMatch(list); it.hasNext();) pageIds << it.next().captured(1).toInt();
        for (int id : pageIds) {
            const QtPdf::Obj *o = q.object(id);
            if (!o || o->body.contains("/Kids")) return fail(QCoreApplication::translate("PdfTags", "the pages are in a form that can't be tagged"));
        }
    }
    // The marks in each page's content become marked content (or, where nothing is to be tagged, are taken out).
    const bool emit = !already && forced != 4 && !tags.isEmpty();
    QVector<QVector<Spot>> spots(tags.size());
    QVector<QVector<int>> mcids(pageIds.size());   // per page, the tag of each marked content in order
    bool any = false;
    for (int k = 0; k < pageIds.size(); ++k) {
        QtPdf::Obj *page = q.object(pageIds[k]);
        const int contents = QtPdf::ref(page->body, "/Contents");
        if (!contents) {
            if (page->body.contains("/Contents")) return fail(QCoreApplication::translate("PdfTags", "a page's content is in a form that can't be tagged"));
            continue;
        }
        QtPdf::Obj *co = q.object(contents);
        if (!co || !QtPdf::isStream(co->body)) return fail(QCoreApplication::translate("PdfTags", "a page's content is in a form that can't be tagged"));
        bool ok = false;
        const QByteArray data = q.streamData(*co, &ok);
        if (!ok || forced == 3) return fail(QCoreApplication::translate("PdfTags", "a page's content can't be read"));
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
        for (auto it = markPattern().globalMatch(text); it.hasNext();) {
            const auto m = it.next();
            int tag = 0;
            bool opens = false;
            if (!isMark(m, tagger, &tag, &opens)) continue;
            // The mark is the block the painter wrote it in: from the "q" line before to the "Q" line after.
            auto lineIs = [&](qsizetype p, char c) { return p >= 0 && p + 1 < data.size() && data[p] == c && data[p + 1] == '\n' && (p == 0 || data[p - 1] == '\n'); };
            qsizetype from = m.capturedStart(), to = m.capturedEnd();
            while (from >= at && !lineIs(from, 'q')) --from;
            while (to < data.size() && !lineIs(to, 'Q')) ++to;
            if (from < at || to >= data.size()) continue;   // (it stays, and is found below)
            to += 2;
            out += data.mid(at, from - at);
            at = to;
            changed = true;
            const bool artifact = tags[tag].type == QLatin1String("Artifact");
            if (opens) {
                open << Open{tag, out.size(), to};
                out += "%OPEN\n";
            } else if (!open.isEmpty() && open.last().tag == tag) {
                const Open o = open.takeLast();
                const QByteArray inside = data.mid(o.dataAt, from - o.dataAt);
                // (what is between the marks was copied to `out` after the placeholder)
                if (!emit || !draws(inside)) {
                    out.remove(o.outAt, 6);
                    continue;
                }
                QByteArray head;
                if (artifact) head = "/Artifact BMC\n";
                else {
                    head = "/" + tags[tag].type.toLatin1() + " <</MCID " + QByteArray::number(int(mcids[k].size())) + ">> BDC\n";
                    spots[tag].push_back({k, int(mcids[k].size())});
                    mcids[k] << tag;
                    any = true;
                }
                out.replace(o.outAt, 6, head);
                out += "EMC\n";
            }
        }
        if (changed) {
            out += data.mid(at);
            out.replace("%OPEN\n", "");   // (a mark that was never closed)
        }
        // (nothing may be left of a mark)
        const QString rest = QString::fromLatin1(changed ? out : data);
        for (auto it = markPattern().globalMatch(rest); it.hasNext();) {
            int tag = 0;
            bool opens = false;
            if (isMark(it.next(), tagger, &tag, &opens)) return fail(QCoreApplication::translate("PdfTags", "a mark could not be taken out of a page"));
        }
        if (!changed) continue;
        QByteArray dict = QtPdf::dictOf(co->body);
        q.setStream(*co, dict, dict.contains("/FlateDecode") ? QtPdf::deflate(out) : out);
    }
    if (!any) return PdfTagResult{q.toBytes(), false, already ? QCoreApplication::translate("PdfTags", "it was tagged already") : QCoreApplication::translate("PdfTags", "there was nothing to tag")};
    // The tags that stay: those with marked content, and the ones that hold them.
    QVector<bool> keep(tags.size(), false);
    for (int i = 0; i < tags.size(); ++i) keep[i] = tags[i].type != QLatin1String("Artifact") && tags[i].content && !spots[i].isEmpty();
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
        if (!spots[i].isEmpty()) {
            // A tag drawn in more than one piece (the two halves of a divided cell) lists each: marked content numbers when they are on one page, references to each otherwise.
            bool onePage = true;
            for (const Spot &sp : std::as_const(spots[i])) onePage = onePage && sp.page == spots[i][0].page;
            d += " /Pg " + QByteArray::number(pageIds[spots[i][0].page]) + " 0 R";
            if (spots[i].size() == 1) d += " /K " + QByteArray::number(spots[i][0].mcid);
            else if (onePage) {
                d += " /K [";
                for (const Spot &sp : std::as_const(spots[i])) d += QByteArray::number(sp.mcid) + " ";
                d = d.trimmed() + "]";
            } else {
                d += " /K [";
                for (const Spot &sp : std::as_const(spots[i]))
                    d += "<< /Type /MCR /Pg " + QByteArray::number(pageIds[sp.page]) + " 0 R /MCID " + QByteArray::number(sp.mcid) + " >> ";
                d = d.trimmed() + "]";
            }
        } else d += " /K [" + children[i].trimmed() + "]";
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
    // (objects were added: the catalog is looked up again)
    catalog = q.object(root);
    if (!lang.isEmpty() && !catalog->body.contains("/Lang")) extra += " /Lang (" + lang.toLatin1() + ")";
    if (!options.title.isEmpty() && !catalog->body.contains("/ViewerPreferences")) extra += " /ViewerPreferences << /DisplayDocTitle true >>";
    const qsizetype close = catalog->body.lastIndexOf(">>");
    if (close < 0) return fail(QCoreApplication::translate("PdfTags", "the PDF has no catalog"));
    catalog->body.insert(close, extra + " ");
    return PdfTagResult{q.toBytes(), true, QString()};
}

} // namespace jp
