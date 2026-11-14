#include "io/pdfbookmarks.h"

#include <QRegularExpression>

#include <algorithm>

namespace jp {

namespace {

struct Entry {
    PdfBookmark mark;
    int parent = -1, first = -1, last = -1, prev = -1, next = -1, count = 0;
};

// The text of a title as a PDF string: UTF-16 with its byte order mark, in hexadecimal.
QByteArray pdfTitle(const QString &title)
{
    QByteArray bytes("\xFE\xFF", 2);
    for (const QChar c : title.left(200)) {
        bytes.append(char(c.unicode() >> 8));
        bytes.append(char(c.unicode() & 0xFF));
    }
    return '<' + bytes.toHex().toUpper() + '>';
}

// The number after `key` ("/Size 12" gives 12), or -1.
int numberAfter(const QByteArray &text, const char *key)
{
    const int at = text.indexOf(key);
    if (at < 0) return -1;
    int i = at + int(qstrlen(key));
    while (i < text.size() && (text[i] == ' ' || text[i] == '\n' || text[i] == '\r')) ++i;
    int j = i;
    while (j < text.size() && text[j] >= '0' && text[j] <= '9') ++j;
    if (j == i || j - i > 9) return -1;
    return text.mid(i, j - i).toInt();
}

// Where object `n` starts, from the table of the file ("xref" with subsections); -1 if it can't be found. Only the classic table is read.
qint64 objectOffset(const QByteArray &pdf, qint64 xref, int n)
{
    qint64 at = xref + 4;
    for (int guard = 0; guard < 100000 && at < pdf.size(); ++guard) {
        while (at < pdf.size() && (pdf[at] == '\n' || pdf[at] == '\r' || pdf[at] == ' ')) ++at;
        if (pdf.mid(at, 7) == "trailer") return -1;
        const int eol = pdf.indexOf('\n', at);
        if (eol < 0) return -1;
        const QList<QByteArray> head = pdf.mid(at, eol - at).trimmed().split(' ');
        if (head.size() != 2) return -1;
        bool a = false, b = false;
        const int start = head[0].toInt(&a), count = head[1].toInt(&b);
        if (!a || !b || count < 0) return -1;
        at = eol + 1;
        if (n >= start && n < start + count) {
            const qint64 row = at + qint64(n - start) * 20;
            if (row + 20 > pdf.size() || pdf[row + 17] != 'n') return -1;
            return pdf.mid(row, 10).toLongLong();
        }
        at += qint64(count) * 20;
    }
    return -1;
}

// Like objectOffset, but follows the file's chain of updates (each table's /Prev) to the table that has the object.
qint64 objectOffsetChain(const QByteArray &pdf, qint64 xref, int n)
{
    for (int guard = 0; guard < 64 && xref > 0 && xref + 4 < pdf.size() && pdf.mid(xref, 4) == "xref"; ++guard) {
        const qint64 at = objectOffset(pdf, xref, n);
        if (at >= 0) return at;
        const int trailerAt = pdf.indexOf("trailer", xref), end = pdf.indexOf("startxref", xref);
        if (trailerAt < 0 || end < trailerAt) return -1;
        xref = numberAfter(pdf.mid(trailerAt, end - trailerAt), "/Prev");
    }
    return -1;
}

// The dictionary of object `n`: from its "<<" to the matching ">>" (strings with a parenthesis are not read; the objects looked at here have none).
QByteArray dictionaryOf(const QByteArray &pdf, qint64 xref, int n, qint64 *objectStart = nullptr)
{
    const qint64 at = objectOffsetChain(pdf, xref, n);
    if (at < 0 || at >= pdf.size()) return {};
    const QByteArray label = QByteArray::number(n) + " 0 obj";
    if (pdf.mid(at, label.size()) != label) return {};
    const int open = pdf.indexOf("<<", at);
    const int end = pdf.indexOf("endobj", at);
    if (open < 0 || end < 0 || open > end) return {};
    int depth = 0;
    for (int i = open; i + 1 < end; ++i) {
        if (pdf[i] == '<' && pdf[i + 1] == '<') {
            ++depth;
            ++i;
        } else if (pdf[i] == '>' && pdf[i + 1] == '>') {
            --depth;
            ++i;
            if (depth == 0) {
                if (objectStart) *objectStart = at;
                return pdf.mid(open, i + 1 - open);
            }
        }
    }
    return {};
}

} // namespace

QByteArray addPdfBookmarks(const QByteArray &pdf, const QVector<PdfBookmark> &marksIn)
{
    if (marksIn.isEmpty() || !pdf.startsWith("%PDF-")) return pdf;
    const int sx = pdf.lastIndexOf("startxref");
    if (sx < 0) return pdf;
    const qint64 xref = pdf.mid(sx + 9, 24).trimmed().split('\n').value(0).trimmed().toLongLong();
    if (xref <= 0 || xref >= pdf.size() || pdf.mid(xref, 4) != "xref") return pdf;
    const int trailerAt = pdf.indexOf("trailer", xref);
    if (trailerAt < 0 || trailerAt > sx) return pdf;
    const int tOpen = pdf.indexOf("<<", trailerAt), tClose = pdf.lastIndexOf(">>", sx);
    if (tOpen < 0 || tClose < tOpen) return pdf;
    QByteArray trailer = pdf.mid(tOpen + 2, tClose - tOpen - 2);
    const int size = numberAfter(trailer, "/Size");
    const int root = numberAfter(trailer, "/Root");
    if (size < 3 || root < 1 || root >= size) return pdf;

    qint64 rootStart = 0;
    QByteArray catalog = dictionaryOf(pdf, xref, root, &rootStart);
    if (catalog.isEmpty() || catalog.contains("/Outlines")) return pdf;
    const int pagesRef = numberAfter(catalog, "/Pages");
    if (pagesRef < 1) return pdf;
    const QByteArray pagesDict = dictionaryOf(pdf, xref, pagesRef);
    const int kidsAt = pagesDict.indexOf("/Kids");
    if (kidsAt < 0) return pdf;
    const int open = pagesDict.indexOf('[', kidsAt), close = pagesDict.indexOf(']', kidsAt);
    if (open < 0 || close < open) return pdf;
    QVector<int> pageObjects;
    static const QRegularExpression ref(QStringLiteral("(\\d+)\\s+0\\s+R"));
    const QString kids = QString::fromLatin1(pagesDict.mid(open + 1, close - open - 1));
    for (auto it = ref.globalMatch(kids); it.hasNext();) pageObjects << it.next().captured(1).toInt();
    if (pageObjects.isEmpty()) return pdf;
    // (a page tree with levels of its own is not read: the kids must be pages)
    for (int k : pageObjects)
        if (dictionaryOf(pdf, xref, k).contains("/Kids")) return pdf;

    // The tree of entries
    QVector<Entry> e;
    QVector<int> stack;   // the entry at each level now open
    for (const PdfBookmark &m : marksIn) {
        if (e.size() >= kMostPdfBookmarks) break;
        if (m.page < 0 || m.page >= pageObjects.size() || m.title.trimmed().isEmpty()) continue;
        Entry en;
        en.mark = m;
        const int level = std::clamp(m.level, 0, 8);
        while (stack.size() > level) stack.removeLast();
        // (a heading that skips a level goes under the last one open)
        en.parent = stack.isEmpty() ? -1 : stack.last();
        const int id = int(e.size());
        if (en.parent >= 0) {
            Entry &p = e[en.parent];
            if (p.first < 0) p.first = id;
            else {
                en.prev = p.last;
                e[p.last].next = id;
            }
            p.last = id;
        }
        e.push_back(en);
        stack.push_back(id);
    }
    if (e.isEmpty()) return pdf;
    // The top entries are the children of the outline's root: chain them
    int topFirst = -1, topLast = -1;
    for (int i = 0; i < e.size(); ++i)
        if (e[i].parent < 0) {
            if (topFirst < 0) topFirst = i;
            else {
                e[i].prev = topLast;
                e[topLast].next = i;
            }
            topLast = i;
        }
    // How many entries each shows below it (all open)
    for (int i = int(e.size()) - 1; i >= 0; --i)
        if (e[i].parent >= 0) e[e[i].parent].count += 1 + e[i].count;
    int topCount = 0;
    for (const Entry &en : std::as_const(e))
        if (en.parent < 0) topCount += 1 + en.count;

    const int outlinesObj = size;   // the entries follow it
    auto objOf = [&](int i) { return outlinesObj + 1 + i; };
    QByteArray tail;
    QVector<qint64> offsets;   // of the new objects, the outline root first
    qint64 at = pdf.size();
    auto add = [&](const QByteArray &obj) {
        offsets << at;
        tail += obj;
        at += obj.size();
    };
    if (!pdf.endsWith('\n')) {
        tail += '\n';
        at += 1;
    }
    add(QByteArray::number(outlinesObj) + " 0 obj\n<< /Type /Outlines /First " + QByteArray::number(objOf(topFirst)) + " 0 R /Last " + QByteArray::number(objOf(topLast)) + " 0 R /Count " +
        QByteArray::number(topCount) + " >>\nendobj\n");
    for (int i = 0; i < e.size(); ++i) {
        const Entry &en = e[i];
        QByteArray d = QByteArray::number(objOf(i)) + " 0 obj\n<< /Title " + pdfTitle(en.mark.title) + " /Parent " + QByteArray::number(en.parent < 0 ? outlinesObj : objOf(en.parent)) + " 0 R";
        if (en.prev >= 0) d += " /Prev " + QByteArray::number(objOf(en.prev)) + " 0 R";
        if (en.next >= 0) d += " /Next " + QByteArray::number(objOf(en.next)) + " 0 R";
        if (en.first >= 0) d += " /First " + QByteArray::number(objOf(en.first)) + " 0 R /Last " + QByteArray::number(objOf(en.last)) + " 0 R /Count " + QByteArray::number(en.count);
        d += " /Dest [" + QByteArray::number(pageObjects[en.mark.page]) + " 0 R /XYZ null null null] >>\nendobj\n";
        add(d);
    }
    // The catalog again, with the outline in it (the same number: the newer object is the one used)
    const int closeAt = catalog.lastIndexOf(">>");
    QByteArray newCatalog = catalog.left(closeAt) + " /Outlines " + QByteArray::number(outlinesObj) + " 0 R /PageMode /UseOutlines >>";
    if (catalog.contains("/PageMode")) newCatalog = catalog.left(closeAt) + " /Outlines " + QByteArray::number(outlinesObj) + " 0 R >>";
    const qint64 catalogAt = at;
    const QByteArray catalogObj = QByteArray::number(root) + " 0 obj\n" + newCatalog + "\nendobj\n";
    tail += catalogObj;
    at += catalogObj.size();
    // The table of this update: the catalog, then the new objects, which are in a row
    auto row = [](qint64 off) {
        QByteArray r = QByteArray::number(off).rightJustified(10, '0');
        return r + " 00000 n \n";
    };
    const qint64 newXref = at;
    QByteArray x = "xref\n" + QByteArray::number(root) + " 1\n" + row(catalogAt) + QByteArray::number(outlinesObj) + " " + QByteArray::number(int(offsets.size())) + "\n";
    for (qint64 off : std::as_const(offsets)) x += row(off);
    // The trailer keeps what the old one had (the document information and identifier), with the new size and a way back
    QByteArray t = trailer;
    {
        QString s = QString::fromLatin1(t);
        s.replace(QRegularExpression(QStringLiteral("/Size\\s+\\d+")), QString());
        s.replace(QRegularExpression(QStringLiteral("/Prev\\s+\\d+")), QString());
        t = s.toLatin1();
    }
    x += "trailer\n<<" + t + " /Size " + QByteArray::number(outlinesObj + 1 + int(e.size())) + " /Prev " + QByteArray::number(xref) + " >>\nstartxref\n" + QByteArray::number(newXref) + "\n%%EOF\n";
    return pdf + tail + x;
}

QByteArray addPdfLinks(const QByteArray &pdf, const QVector<PdfLink> &linksIn)
{
    if (linksIn.isEmpty() || !pdf.startsWith("%PDF-")) return pdf;
    const int sx = pdf.lastIndexOf("startxref");
    if (sx < 0) return pdf;
    const qint64 xref = pdf.mid(sx + 9, 24).trimmed().split('\n').value(0).trimmed().toLongLong();
    if (xref <= 0 || xref >= pdf.size() || pdf.mid(xref, 4) != "xref") return pdf;
    const int trailerAt = pdf.indexOf("trailer", xref);
    if (trailerAt < 0 || trailerAt > sx) return pdf;
    const int tOpen = pdf.indexOf("<<", trailerAt), tClose = pdf.lastIndexOf(">>", sx);
    if (tOpen < 0 || tClose < tOpen) return pdf;
    const QByteArray trailer = pdf.mid(tOpen + 2, tClose - tOpen - 2);
    const int size = numberAfter(trailer, "/Size");
    const int root = numberAfter(trailer, "/Root");
    if (size < 3 || root < 1 || root >= size) return pdf;
    const int pagesRef = numberAfter(dictionaryOf(pdf, xref, root), "/Pages");
    if (pagesRef < 1) return pdf;
    const QByteArray pagesDict = dictionaryOf(pdf, xref, pagesRef);
    const int kidsAt = pagesDict.indexOf("/Kids");
    if (kidsAt < 0) return pdf;
    const int open = pagesDict.indexOf('[', kidsAt), close = pagesDict.indexOf(']', kidsAt);
    if (open < 0 || close < open) return pdf;
    QVector<int> pageObjects;
    static const QRegularExpression ref(QStringLiteral("(\\d+)\\s+0\\s+R"));
    const QString kids = QString::fromLatin1(pagesDict.mid(open + 1, close - open - 1));
    for (auto it = ref.globalMatch(kids); it.hasNext();) pageObjects << it.next().captured(1).toInt();
    if (pageObjects.isEmpty()) return pdf;
    QVector<QByteArray> pageDicts;
    for (int k : pageObjects) {
        const QByteArray d = dictionaryOf(pdf, xref, k);
        if (d.isEmpty() || d.contains("/Kids")) return pdf;   // (a page tree with levels of its own is not read)
        pageDicts << d;
    }

    // The links, as the objects they are, and which page each is on
    QVector<QByteArray> annots;
    QVector<QVector<int>> mine(pageObjects.size());
    const int firstNew = size;
    auto number = [](double v) { return QByteArray::number(v, 'f', 2); };
    for (const PdfLink &l : linksIn) {
        if (annots.size() >= kMostPdfLinks) break;
        if (l.page < 0 || l.page >= pageObjects.size() || l.rect.isEmpty() || (l.uri.isEmpty() && (l.toPage < 0 || l.toPage >= pageObjects.size()))) continue;
        QByteArray a = QByteArray::number(firstNew + int(annots.size())) + " 0 obj\n<< /Type /Annot /Subtype /Link /Rect [" + number(l.rect.left()) + ' ' + number(l.rect.top()) + ' ' +
                       number(l.rect.right()) + ' ' + number(l.rect.bottom()) + "] /Border [0 0 0] /F 4 ";
        if (!l.uri.isEmpty()) {
            // (an address is 7-bit text: anything else, and the characters a string needs escaped, are written as %xx)
            a += "/A << /S /URI /URI (" + l.uri.toUtf8().toPercentEncoding("-._~:/?#[]@!$&'*+,;=%") + ") >>";
        } else
            a += "/Dest [" + QByteArray::number(pageObjects[l.toPage]) + " 0 R /Fit]";
        a += " >>\nendobj\n";
        mine[l.page] << firstNew + int(annots.size());
        annots << a;
    }
    if (annots.isEmpty()) return pdf;

    // Each page's list of annotations: what it has that is not a link, then the new ones
    struct Replaced { int number; QByteArray object; };
    QVector<Replaced> replaced;
    for (int pg = 0; pg < pageObjects.size(); ++pg) {
        const QByteArray &dict = pageDicts[pg];
        QByteArray list;
        auto keepOthers = [&](const QByteArray &array) {
            const QString text = QString::fromLatin1(array);
            for (auto it = ref.globalMatch(text); it.hasNext();) {
                const int n = it.next().captured(1).toInt();
                if (!dictionaryOf(pdf, xref, n).contains("/Subtype /Link")) list += QByteArray::number(n) + " 0 R ";
            }
        };
        const int at = dict.indexOf("/Annots");
        int arrayObject = 0;
        if (at >= 0) {
            if (dict.mid(at + 7).trimmed().startsWith('[')) return pdf;   // (a list kept in the page itself isn't rewritten)
            arrayObject = numberAfter(dict, "/Annots");
            const qint64 off = arrayObject > 0 ? objectOffsetChain(pdf, xref, arrayObject) : -1;
            const int end = off >= 0 ? pdf.indexOf("endobj", off) : -1;
            if (end < 0) return pdf;   // (a list that can't be read: nothing is guessed)
            keepOthers(pdf.mid(off, end - int(off)));
        }
        if (mine[pg].isEmpty() && list.isEmpty() && at < 0) continue;
        for (int n : std::as_const(mine[pg])) list += QByteArray::number(n) + " 0 R ";
        if (arrayObject > 0) replaced.push_back({arrayObject, QByteArray::number(arrayObject) + " 0 obj\n[ " + list + "]\nendobj\n"});
        else {
            const int closeAt = dict.lastIndexOf(">>");
            replaced.push_back({pageObjects[pg], QByteArray::number(pageObjects[pg]) + " 0 obj\n" + dict.left(closeAt) + " /Annots [ " + list + "] >>\nendobj\n"});
        }
    }

    QByteArray tail;
    qint64 at = pdf.size();
    if (!pdf.endsWith('\n')) {
        tail += '\n';
        at += 1;
    }
    QVector<std::pair<int, qint64>> rows;   // (object number, offset) of everything this update writes
    for (const Replaced &r : std::as_const(replaced)) {
        rows << std::make_pair(r.number, at);
        tail += r.object;
        at += r.object.size();
    }
    for (int i = 0; i < annots.size(); ++i) {
        rows << std::make_pair(firstNew + i, at);
        tail += annots[i];
        at += annots[i].size();
    }
    std::sort(rows.begin(), rows.end());
    const qint64 newXref = at;
    QByteArray x = "xref\n";
    for (int i = 0; i < rows.size();) {
        int j = i + 1;
        while (j < rows.size() && rows[j].first == rows[j - 1].first + 1) ++j;
        x += QByteArray::number(rows[i].first) + ' ' + QByteArray::number(j - i) + '\n';
        for (int k = i; k < j; ++k) x += QByteArray::number(rows[k].second).rightJustified(10, '0') + " 00000 n \n";
        i = j;
    }
    QString t = QString::fromLatin1(trailer);
    t.replace(QRegularExpression(QStringLiteral("/Size\\s+\\d+")), QString());
    t.replace(QRegularExpression(QStringLiteral("/Prev\\s+\\d+")), QString());
    x += "trailer\n<<" + t.toLatin1() + " /Size " + QByteArray::number(firstNew + int(annots.size())) + " /Prev " + QByteArray::number(xref) + " >>\nstartxref\n" + QByteArray::number(newXref) + "\n%%EOF\n";
    return pdf + tail + x;
}

QByteArray pdfPropertiesXmp(const QString &title, const QString &author, const QString &subject, const QString &keywords, const QString &program, bool archive, int archivePart)
{
    auto esc = [](const QString &t) { return t.toHtmlEscaped().toUtf8(); };
    QByteArray x = "<?xpacket begin=\"\xEF\xBB\xBF\" id=\"W5M0MpCehiHzreSzNTczkc9d\"?>\n"
                   "<x:xmpmeta xmlns:x=\"adobe:ns:meta/\"><rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\">\n"
                   "<rdf:Description rdf:about=\"\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:pdf=\"http://ns.adobe.com/pdf/1.3/\" "
                   "xmlns:xmp=\"http://ns.adobe.com/xap/1.0/\"" + QByteArray(archive ? " xmlns:pdfaid=\"http://www.aiim.org/pdfa/ns/id/\"" : "") + ">\n";
    x += "<dc:title><rdf:Alt><rdf:li xml:lang=\"x-default\">" + esc(title) + "</rdf:li></rdf:Alt></dc:title>\n";
    if (!author.isEmpty()) x += "<dc:creator><rdf:Seq><rdf:li>" + esc(author) + "</rdf:li></rdf:Seq></dc:creator>\n";
    if (!subject.isEmpty()) x += "<dc:description><rdf:Alt><rdf:li xml:lang=\"x-default\">" + esc(subject) + "</rdf:li></rdf:Alt></dc:description>\n";
    if (!keywords.isEmpty()) x += "<pdf:Keywords>" + esc(keywords) + "</pdf:Keywords>\n";
    if (archive) x += "<pdfaid:part>" + QByteArray::number(archivePart) + "</pdfaid:part><pdfaid:conformance>B</pdfaid:conformance>\n";
    x += "<xmp:CreatorTool>" + esc(program) + "</xmp:CreatorTool>\n</rdf:Description>\n</rdf:RDF></x:xmpmeta>\n<?xpacket end=\"w\"?>";
    return x;
}

} // namespace jp
