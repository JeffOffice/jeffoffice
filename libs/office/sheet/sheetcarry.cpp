// What a copy of a sheet takes from the file of the sheet it was copied from: the parts of a sheet that the program keeps no model of (its protection, filter, links, page setup, and page
// breaks), read from the head and the tail of the sheet's part (the cells in between are never read: the copy's cells come from the model), moved by the edits to rows and columns made
// before and after the copy, and put in the XML the copy begins as (see sheetskeleton.cpp and SheetChanges::copyFromPart).

#include "sheet/xlsxbase.h"

#include "sheet/structsave.h"
#include "sheet/xmlscan.h"

#include <QBuffer>
#include <QCoreApplication>
#include <QRegularExpression>

#include <algorithm>
#include <vector>

namespace jp::office::sheet {

namespace {

using xmlscan::Tag;

struct Piece {
    QByteArray name;   // without the prefix
    QByteArray raw;
};

// The children of the root element of the XML, in order, each as its bytes (the root's start tag, if asked for, is `rootOpen`). The XML may end before the root does.
std::vector<Piece> childrenOf(const QByteArray &xml, QByteArray *rootOpen = nullptr)
{
    std::vector<Piece> out;
    Tag t;
    qsizetype pos = 0, start = 0;
    int depth = 0;
    QByteArray name;
    while (xmlscan::nextTag(xml, pos, &t)) {
        pos = t.end;
        if (t.closing) {
            --depth;
            if (depth == 1) out.push_back({name, xml.mid(start, t.end - start)});
            continue;
        }
        if (depth == 0) {
            if (rootOpen) *rootOpen = xml.mid(t.start, t.end - t.start);
        } else if (depth == 1) {
            start = t.start;
            name = QByteArray(t.local);
            if (t.selfClosing) out.push_back({name, xml.mid(t.start, t.end - t.start)});
        }
        if (!t.selfClosing) ++depth;
    }
    return out;
}

bool isAfterData(const QByteArray &name) { return name == "sheetProtection" || name == "protectedRanges" || name == "autoFilter" || name == "sortState"; }
bool isTail(const QByteArray &name)
{
    return name == "hyperlinks" || name == "printOptions" || name == "pageMargins" || name == "pageSetup" || name == "headerFooter" || name == "rowBreaks" || name == "colBreaks" || name == "ignoredErrors";
}

// The element without the attributes that name a part of the package (`r:id` and the like): the copy has no such parts.
QByteArray withoutRelationships(const QByteArray &element)
{
    static const QRegularExpression ids(QStringLiteral("\\s+[A-Za-z_][\\w.\\-]*:id=\"[^\"]*\""));
    const qsizetype end = element.indexOf('>');
    if (end < 0) return element;
    QString open = QString::fromUtf8(element.left(end));
    open.remove(ids);
    return open.toUtf8() + element.mid(end);
}

class ByteSink : public PartSink {
public:
    void write(const char *data, qsizetype size) override { bytes.append(data, size); }
    QByteArray bytes;
};

// The declarations of namespaces (`xmlns:x="..."`) of a start tag.
QByteArray namespacesOf(const QByteArray &open)
{
    static const QRegularExpression declaration(QStringLiteral("\\sxmlns:[A-Za-z_][\\w.\\-]*=\"[^\"]*\""));
    QByteArray out;
    const QString text = QString::fromUtf8(open);
    for (auto it = declaration.globalMatch(text); it.hasNext();) out += it.next().captured(0).toUtf8();
    return out;
}

const char *kMainNamespace = "http://schemas.openxmlformats.org/spreadsheetml/2006/main";
const char *kRelationshipsNamespace = "http://schemas.openxmlformats.org/officeDocument/2006/relationships";

QByteArray miniSheet(const SheetExtras &x)
{
    return QByteArray("<worksheet xmlns=\"") + kMainNamespace + "\" xmlns:r=\"" + kRelationshipsNamespace + "\"" + x.namespaces + "><sheetData/>" + x.afterData + x.tail + "</worksheet>";
}

} // namespace

bool readSheetExtras(QIODevice *in, SheetExtras *out, QString *error)
{
    constexpr qint64 kChunk = 65536, kMostKept = qint64(64) << 20;
    *out = SheetExtras();
    auto fail = [&](const QString &why) {
        if (error) *error = why;
        return false;
    };
    // the head: up to the sheet's data
    QByteArray buf;
    qsizetype scanFrom = 0;
    Tag t;
    bool found = false, eof = false;
    while (!found) {
        qsizetype pos = scanFrom;
        while (xmlscan::nextTag(buf, pos, &t)) {
            if (!t.closing && t.local == "sheetData") {
                found = true;
                break;
            }
            pos = t.end;
        }
        if (found) break;
        scanFrom = pos;
        if (eof || buf.size() > kMostKept) return fail(QCoreApplication::translate("jp::office::sheet::Carry", "The sheet's data could not be found in its part."));
        const qsizetype old = buf.size();
        buf.resize(old + kChunk);
        const qint64 n = in->read(buf.data() + old, kChunk);
        buf.resize(old + (n > 0 ? qsizetype(n) : 0));
        if (n <= 0) eof = true;
    }
    const QByteArray head = buf.left(t.start);
    QByteArray tail;
    if (t.selfClosing) {
        tail = buf.mid(t.end);
    } else {
        // the data: passed over without being kept, to the end tag
        const QByteArray close = "</" + QByteArray(xmlscan::prefixOf(t.name)) + "sheetData>";
        QByteArray window = buf.mid(t.end);
        for (;;) {
            const qsizetype at = window.indexOf(close);
            if (at >= 0) {
                tail = window.mid(at + close.size());
                break;
            }
            if (eof) return fail(QCoreApplication::translate("jp::office::sheet::Carry", "The sheet's data is cut short."));
            if (window.size() >= close.size()) window = window.right(close.size() - 1);
            const qsizetype old = window.size();
            window.resize(old + kChunk);
            const qint64 n = in->read(window.data() + old, kChunk);
            window.resize(old + (n > 0 ? qsizetype(n) : 0));
            if (n <= 0) eof = true;
        }
    }
    // (what follows the data is read to its end: it is small, however large the sheet)
    for (;;) {
        if (tail.size() > kMostKept) return fail(QCoreApplication::translate("jp::office::sheet::Carry", "What follows the sheet's data is larger than any real file has."));
        const QByteArray more = in->read(kChunk);
        if (more.isEmpty()) break;
        tail += more;
    }
    QByteArray rootOpen;
    const std::vector<Piece> headPieces = childrenOf(head, &rootOpen);
    out->namespaces = namespacesOf(rootOpen);
    for (const Piece &p : headPieces) {
        if (p.name != "sheetPr") continue;
        // the properties that say how the sheet is printed and outlined (its tab color is the model's)
        for (const Piece &c : childrenOf(p.raw))
            if (c.name == "outlinePr" || c.name == "pageSetUpPr") out->sheetPr += c.raw;
    }
    for (const Piece &p : childrenOf("<worksheet>" + tail)) {
        if (isAfterData(p.name)) out->afterData += p.raw;
        else if (isTail(p.name)) out->tail += p.name == "pageSetup" ? withoutRelationships(p.raw) : p.raw;
    }
    out->found = true;
    return true;
}

bool moveSheetExtras(SheetExtras *x, const MoveContext &ctx, int sheet, QString *error, const std::atomic<bool> *cancel)
{
    if (x->afterData.isEmpty() && x->tail.isEmpty()) return true;
    QByteArray mini = miniSheet(*x);
    QBuffer in(&mini);
    in.open(QIODevice::ReadOnly);
    ByteSink sink;
    bool changed = false;
    if (!moveSheetPart(&in, &sink, ctx, sheet, &changed, error, cancel)) return false;
    if (!changed) return true;
    SheetExtras moved = *x;
    moved.afterData.clear();
    moved.tail.clear();
    for (const Piece &p : childrenOf(sink.bytes)) {
        if (isAfterData(p.name)) moved.afterData += p.raw;
        else if (isTail(p.name)) moved.tail += p.raw;
    }
    *x = moved;
    return true;
}

QByteArray hyperlinkRelationshipsXml(const QVector<Relationship> &relationships)
{
    QByteArray out;
    for (const Relationship &r : relationships) {
        if (!r.type.endsWith(QLatin1String("/hyperlink"))) continue;
        out += "<Relationship Id=\"" + xmlscan::escapeAttribute(r.id) + "\" Type=\"" + xmlscan::escapeAttribute(r.type) + "\" Target=\"" + xmlscan::escapeAttribute(r.target) + "\"";
        if (r.external) out += " TargetMode=\"External\"";
        out += "/>";
    }
    return out;
}

QByteArray skeletonWithExtras(const QByteArray &skeleton, const SheetExtras &x, bool isProtected)
{
    QByteArray out = skeleton;
    // the namespaces the pieces use, on the root element
    const qsizetype rootAt = out.indexOf("<worksheet");
    const qsizetype rootEnd = rootAt < 0 ? -1 : out.indexOf('>', rootAt);
    if (rootEnd > 0 && !x.namespaces.isEmpty()) {
        const QString declared = QString::fromUtf8(out.mid(rootAt, rootEnd - rootAt));
        QByteArray add;
        const QString extra = QString::fromUtf8(x.namespaces);
        static const QRegularExpression one(QStringLiteral("\\s(xmlns:[A-Za-z_][\\w.\\-]*)=\"[^\"]*\""));
        for (auto it = one.globalMatch(extra); it.hasNext();) {
            const QRegularExpressionMatch m = it.next();
            if (!declared.contains(QLatin1Char(' ') + m.captured(1) + QLatin1Char('='))) add += m.captured(0).toUtf8();
        }
        out.insert(rootEnd, add);
    }
    // sheetPr: the properties after the tab color
    if (!x.sheetPr.isEmpty()) {
        const qsizetype close = out.indexOf("</sheetPr>");
        if (close >= 0) {
            out.insert(close, x.sheetPr);
        } else {
            const qsizetype start = out.indexOf("<worksheet");
            const qsizetype at = start < 0 ? -1 : out.indexOf('>', start) + 1;
            if (at > 0) out.insert(at, "<sheetPr>" + x.sheetPr + "</sheetPr>");
        }
    }
    // the protection, filter, and the like follow the data
    QByteArray after = x.afterData;
    if (isProtected && !after.contains("sheetProtection")) after = "<sheetProtection sheet=\"1\" objects=\"1\" scenarios=\"1\"/>" + after;
    if (!after.isEmpty()) {
        const qsizetype at = out.indexOf("<sheetData/>");
        if (at >= 0) out.insert(at + 12, after);
    }
    // the links and the page setup take the place of the default margins
    if (!x.tail.isEmpty()) {
        const qsizetype at = out.indexOf("<pageMargins ");
        if (at >= 0) {
            const qsizetype end = out.indexOf("/>", at) + 2;
            out.replace(at, end - at, x.tail);
        }
    }
    return out;
}

} // namespace jp::office::sheet
