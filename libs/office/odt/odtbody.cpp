// OpenDocument text to .docx: the body (paragraphs, lists, tables, sections, and what is in a line of text).

#include "odt/odtread.h"
#include "office/barcodemarker.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QRegularExpression>
#include <QFileInfo>
#include <algorithm>
#include <cmath>

namespace jp::office::odt {

namespace {

QByteArray num(qint64 v)
{
    return QByteArray::number(v);
}

// Text fit to be a w:t: the characters XML has no place for are left out.
QString xmlSafe(const QString &s)
{
    QString out;
    out.reserve(s.size());
    for (const QChar c : s) {
        const ushort u = c.unicode();
        if (u < 0x20 && u != 0x9 && u != 0xA && u != 0xD) continue;
        if (u == 0xFFFE || u == 0xFFFF) continue;
        out += c;
    }
    return out;
}

bool isWhite(QChar c)
{
    return c == QLatin1Char(' ') || c == QLatin1Char('\t') || c == QLatin1Char('\n') || c == QLatin1Char('\r');
}

} // namespace

// What a paragraph's content is made of while it is read.
struct Reader::ParaCtx {
    QByteArray runs;
    const Props *base = nullptr;   // the text properties the paragraph's style gives every run (in the cache of the styles)
    bool lastSpace = true;    // the last character was a space, or the line has none yet
    int depth = 0;
    bool inNote = false;
    QString insId;               // an insertion that is open, and where its runs begin
    qsizetype insAt = 0;
    int linkDepth = 0, insLinkDepth = 0;
    qint64 spaces = 0;
    // A field being made over several runs (the table of contents's entries): none here.
};

QByteArray Reader::RunFmt::key() const
{
    QStringList keys = direct.keys();
    keys.sort();
    QByteArray k = rStyle.toUtf8() + '|';
    for (const QString &s : keys) k += s.toUtf8() + '=' + direct.value(s).toUtf8() + ';';
    return k;
}

// ---- runs

void Reader::addRun(ParaCtx *pc, const QByteArray &content, const RunFmt &fmt)
{
    // (The look of a run is made once for each paragraph style and run format.)
    const QByteArray key = QByteArray::number(quintptr(pc->base)) + '#' + fmt.key();
    auto cached = m_rPrCache.constFind(key);
    if (cached == m_rPrCache.constEnd()) {
        Props merged = pc->base ? *pc->base : Props();
        for (auto it = fmt.direct.constBegin(); it != fmt.direct.constEnd(); ++it) merged.insert(it.key(), it.value());
        cached = m_rPrCache.insert(key, rPrXml(merged, fmt.rStyle));
    }
    const QByteArray &rpr = cached.value();
    pc->runs += "<w:r>";
    if (!rpr.isEmpty()) pc->runs += "<w:rPr>" + rpr + "</w:rPr>";
    pc->runs += content + "</w:r>";
}

void Reader::addText(ParaCtx *pc, const QString &text, const RunFmt &fmt)
{
    if (text.isEmpty()) return;
    // White space collapses: a run of it is one space, and a line does not begin with one.
    QString out;
    out.reserve(text.size());
    for (qsizetype i = 0; i < text.size(); ++i) {
        const QChar c = text[i];
        if (isWhite(c)) {
            if (!pc->lastSpace) out += QLatin1Char(' ');
            pc->lastSpace = true;
        } else {
            out += c;
            pc->lastSpace = false;
        }
    }
    out = xmlSafe(out);
    if (out.isEmpty()) return;
    addRun(pc, "<w:t xml:space=\"preserve\">" + esc(out, false) + "</w:t>", fmt);
}

void Reader::addSpaces(ParaCtx *pc, const QString &raw, const RunFmt &fmt)
{
    bool ok = false;
    int n = raw.isEmpty() ? 1 : raw.toInt(&ok);
    if (!raw.isEmpty() && !ok) n = 1;
    n = std::max(1, std::min(n, 10000));
    // (Spaces a file asks for are bounded for a paragraph, and for the document.)
    if (pc->spaces + n > kMaxSpacesPerParagraph || m_spaces + n > kMaxSpacesPerDocument) {
        left(QCoreApplication::translate("odt", "more spaces than are kept"));
        return;
    }
    pc->spaces += n;
    m_spaces += n;
    addRun(pc, "<w:t xml:space=\"preserve\">" + QByteArray(n, ' ') + "</w:t>", fmt);
    pc->lastSpace = false;
}

void Reader::addField(ParaCtx *pc, const QString &instr, const QString &result, const RunFmt &fmt)
{
    QByteArray begin = "<w:fldChar w:fldCharType=\"begin\"/>";
    addRun(pc, begin, fmt);
    addRun(pc, "<w:instrText xml:space=\"preserve\"> " + esc(instr, false) + " </w:instrText>", fmt);
    addRun(pc, "<w:fldChar w:fldCharType=\"separate\"/>", fmt);
    const QString safe = xmlSafe(result);
    if (!safe.isEmpty()) addRun(pc, "<w:t xml:space=\"preserve\">" + esc(safe, false) + "</w:t>", fmt);
    addRun(pc, "<w:fldChar w:fldCharType=\"end\"/>", fmt);
    pc->lastSpace = false;
}

// ---- relationships of the parts that hold a link or a picture


QString Reader::imageRelationship(const Odf &o, int image)
{
    QByteArray bytes;
    QString ext;
    QString key;
    const wml::XmlTree &t = o.tree();
    const QString href = o.at(image, "xlink:href");
    if (!href.isEmpty()) {
        if (href.contains(QLatin1Char(':')) || href.startsWith(QLatin1Char('/')) || href.contains(QLatin1String(".."))) {
            left(QCoreApplication::translate("odt", "pictures that the file only links to"));
            return QString();
        }
        key = QStringLiteral("/") + href;
        const auto done = m_imageRel.constFind(m_owner + QLatin1Char('|') + key);
        if (done != m_imageRel.constEnd()) return done.value();
        if (m_flat || !m_pkg.hasPart(key)) return QString();
        if (m_pkg.partSize(key) > (qint64(64) << 20)) return QString();
        bytes = m_pkg.part(key);
        ext = QFileInfo(href).suffix().toLower();
    } else {
        const int bin = o.child(image, "office:binary-data");
        if (bin < 0) return QString();
        const QString text = t.text(bin);
        if (text.size() > (qint64(96) << 20)) return QString();
        bytes = QByteArray::fromBase64(text.toLatin1());
        key = QStringLiteral("binary:%1").arg(qHash(bytes));
        const auto done = m_imageRel.constFind(m_owner + QLatin1Char('|') + key);
        if (done != m_imageRel.constEnd()) return done.value();
    }
    if (bytes.isEmpty()) return QString();
    const QString sniffed = sniffExtension(bytes);
    if (!sniffed.isEmpty()) ext = sniffed;
    if (!contentTypeOf(ext)) {
        left(QCoreApplication::translate("odt", "pictures of a kind that isn't read"));
        return QString();
    }
    if (m_imageCount >= kMaxMadeParts) return QString();
    if (ext == QLatin1String("jpg")) ext = QStringLiteral("jpeg");
    const QString name = QStringLiteral("/word/media/image%1.%2").arg(++m_imageCount).arg(ext);
    m_out.addDefault(ext, QString::fromLatin1(contentTypeOf(ext)));
    m_out.addPart(name, bytes, QString::fromLatin1(contentTypeOf(ext)));
    const QString rid = m_out.addRelationship(m_owner, QString::fromLatin1(kRelBase) + QStringLiteral("image"), name);
    m_imageRel.insert(m_owner + QLatin1Char('|') + key, rid);
    return rid;
}

// ---- drawings

QByteArray Reader::anchorXml(const Odf &o, int frame, const QString &styleName, qint64 cx, qint64 cy, const QByteArray &graphic, int docPrId)
{
    Q_UNUSED(styleName);
    const Props &gp = resolved(QStringLiteral("graphic"), o.at(frame, "draw:style-name"));
    auto g = [&](const char *k) { return gp.value(QStringLiteral("gr:") + QLatin1String(k)); };
    const QString name = o.at(frame, "draw:name");
    const QString desc = [&] {
        const int d = o.child(frame, "svg:desc");
        return d >= 0 ? o.tree().text(d) : QString();
    }();
    // A picture frame with the marker of a barcode of ours keeps its settings in the drawing's properties (office/barcodemarker.h).
    QByteArray marker;
    if (graphic.contains("<pic:pic>"))
        if (const int jo = o.tree().namespaceId("urn:jeffoffice:names:extension:1.0"); jo >= 0)
            if (const BarcodeMarker found = barcodeOdfMarker(o.tree().attr(frame, "barcode", jo), o.tree().attr(frame, "barcode-sha256", jo)); !found.isEmpty()) marker = barcodeMarkerXml(found.settings, found.pngHash);
    QByteArray docPr = "<wp:docPr id=\"" + num(docPrId) + "\" name=\"" + esc(name.isEmpty() ? QStringLiteral("Object %1").arg(docPrId) : name) + "\"" + (desc.isEmpty() ? QByteArray() : " descr=\"" + esc(desc) + "\"") +
                       (marker.isEmpty() ? QByteArray("/>") : ">" + marker + "</wp:docPr>");
    const QString anchorType = o.at(frame, "text:anchor-type");
    if (anchorType == QLatin1String("as-char") || anchorType.isEmpty()) {
        return "<w:drawing><wp:inline distT=\"0\" distB=\"0\" distL=\"0\" distR=\"0\"><wp:extent cx=\"" + num(cx) + "\" cy=\"" + num(cy) + "\"/><wp:effectExtent l=\"0\" t=\"0\" r=\"0\" b=\"0\"/>" +
               docPr + "<wp:cNvGraphicFramePr/>" + graphic + "</wp:inline></w:drawing>";
    }
    // Where it is set: what its position is measured from, and the wrap.
    auto relH = [&](const QString &r) {
        if (r == QLatin1String("page")) return "page";
        if (r == QLatin1String("page-content") || r == QLatin1String("page-start-margin") || r == QLatin1String("page-end-margin")) return "margin";
        if (r == QLatin1String("char")) return "character";
        return "column";
    };
    auto relV = [&](const QString &r) {
        if (r == QLatin1String("page")) return "page";
        if (r == QLatin1String("page-content")) return "margin";
        if (r == QLatin1String("line") || r == QLatin1String("baseline") || r == QLatin1String("char")) return "line";
        return "paragraph";
    };
    QString hpos = g("style:horizontal-pos"), vpos = g("style:vertical-pos");
    QString hrel = g("style:horizontal-rel"), vrel = g("style:vertical-rel");
    if (anchorType == QLatin1String("page")) {
        if (hrel.isEmpty()) hrel = QStringLiteral("page");
        if (vrel.isEmpty()) vrel = QStringLiteral("page");
    }
    auto offset = [&](const char *attr) {
        bool ok = false;
        const qint64 e = lengthToEmu(o.at(frame, attr), &ok);
        return ok ? e : 0;
    };
    QByteArray posH = "<wp:positionH relativeFrom=\"" + QByteArray(relH(hrel)) + "\">";
    if (hpos == QLatin1String("center")) posH += "<wp:align>center</wp:align>";
    else if (hpos == QLatin1String("right")) posH += "<wp:align>right</wp:align>";
    else if (hpos == QLatin1String("left") && !o.has(frame, "svg:x")) posH += "<wp:align>left</wp:align>";
    else if (hpos == QLatin1String("inside")) posH += "<wp:align>inside</wp:align>";
    else if (hpos == QLatin1String("outside")) posH += "<wp:align>outside</wp:align>";
    else posH += "<wp:posOffset>" + num(offset("svg:x")) + "</wp:posOffset>";
    posH += "</wp:positionH>";
    QByteArray posV = "<wp:positionV relativeFrom=\"" + QByteArray(relV(vrel)) + "\">";
    if (vpos == QLatin1String("middle")) posV += "<wp:align>center</wp:align>";
    else if (vpos == QLatin1String("bottom")) posV += "<wp:align>bottom</wp:align>";
    else if (vpos == QLatin1String("top") && !o.has(frame, "svg:y")) posV += "<wp:align>top</wp:align>";
    else posV += "<wp:posOffset>" + num(offset("svg:y")) + "</wp:posOffset>";
    posV += "</wp:positionV>";
    const QString wrap = g("style:wrap");
    QByteArray wrapXml;
    bool behind = g("style:run-through") == QLatin1String("background");
    if (wrap == QLatin1String("none")) wrapXml = "<wp:wrapTopAndBottom/>";
    else if (wrap == QLatin1String("left")) wrapXml = "<wp:wrapSquare wrapText=\"left\"/>";
    else if (wrap == QLatin1String("right")) wrapXml = "<wp:wrapSquare wrapText=\"right\"/>";
    else if (wrap == QLatin1String("parallel") || wrap == QLatin1String("dynamic")) wrapXml = "<wp:wrapSquare wrapText=\"bothSides\"/>";
    else if (wrap == QLatin1String("biggest")) wrapXml = "<wp:wrapSquare wrapText=\"largest\"/>";
    else wrapXml = "<wp:wrapNone/>";
    auto margin = [&](const char *side) {
        bool ok = false;
        QString v = g((QStringLiteral("fo:margin-") + QLatin1String(side)).toLatin1().constData());
        if (v.isEmpty()) v = g("fo:margin");
        const qint64 e = lengthToEmu(v, &ok);
        return ok ? e : 0;
    };
    const int z = o.at(frame, "draw:z-index").toInt();
    return "<w:drawing><wp:anchor distT=\"" + num(margin("top")) + "\" distB=\"" + num(margin("bottom")) + "\" distL=\"" + num(margin("left")) + "\" distR=\"" + num(margin("right")) +
           "\" simplePos=\"0\" relativeHeight=\"" + num(251658240 + std::max(0, std::min(z, 100000))) + "\" behindDoc=\"" + (behind ? "1" : "0") +
           "\" locked=\"0\" layoutInCell=\"1\" allowOverlap=\"1\"><wp:simplePos x=\"0\" y=\"0\"/>" + posH + posV + "<wp:extent cx=\"" + num(cx) + "\" cy=\"" + num(cy) +
           "\"/><wp:effectExtent l=\"0\" t=\"0\" r=\"0\" b=\"0\"/>" + wrapXml + docPr + "<wp:cNvGraphicFramePr/>" + graphic + "</wp:anchor></w:drawing>";
}

QByteArray Reader::drawing(const Odf &o, int frame, ParaCtx *pc, bool *ok)
{
    *ok = false;
    const wml::XmlTree &t = o.tree();
    bool wOk = false, hOk = false;
    qint64 cx = lengthToEmu(o.at(frame, "svg:width"), &wOk), cy = lengthToEmu(o.at(frame, "svg:height"), &hOk);
    if (!hOk) cy = lengthToEmu(o.at(frame, "fo:min-height"), &hOk);
    double rel = 0;
    if (!wOk && percentOf(o.at(frame, "style:rel-width"), &rel)) {
        cx = qint64(m_pageTextWidth * 635.0 * rel / 100.0);
        wOk = true;
    }
    // (A size is at least a point and at most the width of a poster.)
    cx = std::max<qint64>(12700, std::min<qint64>(cx, qint64(12700) * 3000));
    cy = std::max<qint64>(12700, std::min<qint64>(cy, qint64(12700) * 3000));
    int image = -1, box = -1;
    for (int c = t.firstChild(frame); c >= 0; c = t.next(c)) {
        if (o.is(c, "draw:image") && image < 0) image = c;
        else if (o.is(c, "draw:text-box") && box < 0) box = c;
        else if ((o.is(c, "draw:object") || o.is(c, "draw:object-ole")) && image < 0) {
            const int preview = o.child(c, "draw:image");
            if (preview >= 0) image = preview;
        }
    }
    if (image < 0 && box < 0 && !o.is(frame, "draw:frame")) {
        // A shape that holds text is a text box of its own shape.
        for (int c = t.firstChild(frame); c >= 0; c = t.next(c))
            if (o.is(c, "text:p") || o.is(c, "text:h") || o.is(c, "text:list")) {
                box = frame;
                break;
            }
        if (box < 0) {
            left(QCoreApplication::translate("odt", "drawing shapes"));
            return QByteArray();
        }
    }
    const int id = nextDocPrId();
    if (image >= 0) {
        const QString rid = imageRelationship(o, image);
        if (rid.isEmpty()) return QByteArray();
        const QByteArray graphic = "<a:graphic><a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/picture\"><pic:pic><pic:nvPicPr><pic:cNvPr id=\"" + num(id) +
                                   "\" name=\"Picture " + num(id) + "\"/><pic:cNvPicPr/></pic:nvPicPr><pic:blipFill><a:blip r:embed=\"" + rid.toLatin1() +
                                   "\"/><a:stretch><a:fillRect/></a:stretch></pic:blipFill><pic:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"" + num(cx) + "\" cy=\"" + num(cy) +
                                   "\"/></a:xfrm><a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom></pic:spPr></pic:pic></a:graphicData></a:graphic>";
        *ok = true;
        return anchorXml(o, frame, QString(), cx, cy, graphic, id);
    }
    if (box >= 0) {
        if (pc->depth >= kMaxNesting) return QByteArray();
        QVector<Block> blocks;
        convertBlocks(o, box, &blocks, pc->depth + 1);
        const QByteArray content = bodyBlocksXml(blocks, true);
        const Props &gp = resolved(QStringLiteral("graphic"), o.at(frame, "draw:style-name"));
        auto g = [&](const char *k) { return gp.value(QStringLiteral("gr:") + QLatin1String(k)); };
        // The padding (the room between the box and its text), the fill, and the line the style gives it.
        auto pad = [&](const char *side) {
            QString v = g((QStringLiteral("fo:padding-") + QLatin1String(side)).toLatin1().constData());
            if (v.isEmpty()) v = g("fo:padding");
            bool okp = false;
            const qint64 e = lengthToEmu(v, &okp);
            return okp ? std::max<qint64>(0, e) : qint64(0);
        };
        QByteArray fill = "<a:noFill/>";
        if (g("draw:fill") == QLatin1String("solid") && !colorHex(g("draw:fill-color")).isEmpty()) fill = "<a:solidFill><a:srgbClr val=\"" + colorHex(g("draw:fill-color")).toLatin1() + "\"/></a:solidFill>";
        QByteArray line = "<a:ln><a:noFill/></a:ln>";
        const QString border = g("fo:border");
        if (!border.isEmpty() && border != QLatin1String("none")) line = "<a:ln w=\"6350\"><a:solidFill><a:srgbClr val=\"000000\"/></a:solidFill></a:ln>";
        else if (!g("draw:stroke").isEmpty() && g("draw:stroke") != QLatin1String("none")) {
            const QString c = colorHex(g("svg:stroke-color"));
            bool okw = false;
            const qint64 w = lengthToEmu(g("svg:stroke-width"), &okw);
            line = "<a:ln w=\"" + num(okw && w > 0 ? std::min<qint64>(w, 1270000) : 9525) + "\"><a:solidFill><a:srgbClr val=\"" + (c.isEmpty() ? QByteArray("000000") : c.toLatin1()) + "\"/></a:solidFill></a:ln>";
        }
        const QByteArray graphic = "<a:graphic><a:graphicData uri=\"http://schemas.microsoft.com/office/word/2010/wordprocessingShape\"><wps:wsp><wps:cNvSpPr txBox=\"1\"/><wps:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"" +
                                   num(cx) + "\" cy=\"" + num(cy) + "\"/></a:xfrm><a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom>" + fill + line +
                                   "</wps:spPr><wps:txbx><w:txbxContent>" + content + "</w:txbxContent></wps:txbx><wps:bodyPr rot=\"0\" vert=\"horz\" wrap=\"square\" lIns=\"" + num(pad("left")) + "\" tIns=\"" + num(pad("top")) +
                                   "\" rIns=\"" + num(pad("right")) + "\" bIns=\"" + num(pad("bottom")) + "\" anchor=\"t\"><a:noAutofit/></wps:bodyPr></wps:wsp></a:graphicData></a:graphic>";
        *ok = true;
        return anchorXml(o, frame, QString(), cx, cy, graphic, id);
    }
    left(QCoreApplication::translate("odt", "objects such as formulas and charts"));
    return QByteArray();
}

// ---- what a line of text holds

void Reader::inlineContent(const Odf &o, int parent, ParaCtx *pc, const RunFmt &fmt, int depth)
{
    const wml::XmlTree &t = o.tree();
    if (depth > kMaxNesting * 3) return;
    int prev = -1;
    for (int c = t.firstChild(parent);; c = t.next(c)) {
        addText(pc, t.textAfter(parent, prev), fmt);
        if (c < 0) break;
        inlineElement(o, c, pc, fmt, depth);
        prev = c;
    }
}

namespace {

QString bookmarkNameOf(const QString &n)
{
    return n.left(255);
}

}

void Reader::inlineElement(const Odf &o, int n, ParaCtx *pc, const RunFmt &fmt, int depth)
{
    const wml::XmlTree &t = o.tree();
    if (o.is(n, "text:span")) {
        RunFmt f = fmt;
        const QString name = o.at(n, "text:style-name");
        if (!name.isEmpty()) {
            QString named;
            const Props &own = ownProps(QStringLiteral("text"), name, &named);
            for (auto it = own.constBegin(); it != own.constEnd(); ++it) f.direct.insert(it.key(), it.value());
            if (!named.isEmpty()) f.rStyle = styleId(QStringLiteral("text"), named);
        }
        inlineContent(o, n, pc, f, depth + 1);
    } else if (o.is(n, "text:s")) addSpaces(pc, o.at(n, "text:c"), fmt);
    else if (o.is(n, "text:tab")) {
        addRun(pc, "<w:tab/>", fmt);
        pc->lastSpace = true;
    } else if (o.is(n, "text:line-break")) {
        addRun(pc, "<w:br/>", fmt);
        pc->lastSpace = true;
    } else if (o.is(n, "text:soft-page-break")) {
    } else if (o.is(n, "text:a")) {
        RunFmt f = fmt;
        const QString name = o.at(n, "text:style-name");
        if (!name.isEmpty() && style(QStringLiteral("text"), name)) {
            QString named;
            const Props &own = ownProps(QStringLiteral("text"), name, &named);
            for (auto it = own.constBegin(); it != own.constEnd(); ++it) f.direct.insert(it.key(), it.value());
            if (!named.isEmpty()) f.rStyle = styleId(QStringLiteral("text"), named);
        }
        const QString href = o.at(n, "xlink:href");
        const QByteArray saved = pc->runs;
        pc->runs.clear();
        ++pc->linkDepth;
        inlineContent(o, n, pc, f, depth + 1);
        --pc->linkDepth;
        const QByteArray inner = pc->runs;
        pc->runs = saved;
        if (href.isEmpty() || inner.isEmpty()) pc->runs += inner;
        else if (href.startsWith(QLatin1Char('#'))) pc->runs += "<w:hyperlink w:anchor=\"" + esc(bookmarkNameOf(href.mid(1))) + "\" w:history=\"1\">" + inner + "</w:hyperlink>";
        else {
            const QString key = m_owner + QLatin1Char('|') + href;
            QString rid = m_linkRel.value(key);
            if (rid.isEmpty() && m_linkRel.size() < kMaxMadeParts * 5) {
                rid = m_out.addRelationship(m_owner, QString::fromLatin1(kRelBase) + QStringLiteral("hyperlink"), href, true);
                m_linkRel.insert(key, rid);
            }
            pc->runs += rid.isEmpty() ? inner : "<w:hyperlink r:id=\"" + rid.toLatin1() + "\" w:history=\"1\">" + inner + "</w:hyperlink>";
        }
    } else if (o.is(n, "text:bookmark") || o.is(n, "text:bookmark-start") || o.is(n, "text:bookmark-end") || o.is(n, "text:reference-mark") || o.is(n, "text:reference-mark-start") ||
               o.is(n, "text:reference-mark-end")) {
        const QString name = bookmarkNameOf(o.at(n, "text:name"));
        if (name.isEmpty()) return;
        auto idOf = [&](bool create) {
            const auto it = m_bookmarkIds.constFind(name);
            if (it != m_bookmarkIds.constEnd()) return it.value();
            if (!create) return -1;
            const int id = ++m_bookmarkId;
            m_bookmarkIds.insert(name, id);
            return id;
        };
        const bool point = o.is(n, "text:bookmark") || o.is(n, "text:reference-mark");
        const bool end = o.is(n, "text:bookmark-end") || o.is(n, "text:reference-mark-end");
        if (end) {
            const int id = idOf(false);
            if (id >= 0) pc->runs += "<w:bookmarkEnd w:id=\"" + num(id) + "\"/>";
        } else {
            if (m_bookmarkId >= kMaxMadeParts * 10) return;
            // (A name written twice is made one place: the first stands.)
            if (m_bookmarkIds.contains(name) && !m_bookmarkOpen.contains(name) && m_bookmarkDone.contains(name)) return;
            const int id = idOf(true);
            pc->runs += "<w:bookmarkStart w:id=\"" + num(id) + "\" w:name=\"" + esc(name) + "\"/>";
            m_bookmarkDone.insert(name);
            if (point) pc->runs += "<w:bookmarkEnd w:id=\"" + num(id) + "\"/>";
            else m_bookmarkOpen.insert(name);
        }
    } else if (o.is(n, "draw:frame")) {
        bool ok = false;
        const QByteArray d = drawing(o, n, pc, &ok);
        if (!d.isEmpty()) addRun(pc, d, fmt);
        pc->lastSpace = false;
    } else if (o.is(n, "draw:a")) {
        inlineContent(o, n, pc, fmt, depth + 1);
    } else if (o.is(n, "text:note")) {
        pc->runs += noteReference(o, n, pc, fmt);
        pc->lastSpace = false;
    } else if (o.is(n, "office:annotation")) {
        pc->runs += commentXml(o, n, pc);
    } else if (o.is(n, "office:annotation-end")) {
        const QString name = o.at(n, "office:name");
        const auto it = m_commentOpen.find(name);
        if (it != m_commentOpen.end()) {
            pc->runs += "<w:commentRangeEnd w:id=\"" + num(it.value()) + "\"/><w:r><w:commentReference w:id=\"" + num(it.value()) + "\"/></w:r>";
            m_commentOpen.erase(it);
        }
    } else if (o.is(n, "text:page-number")) {
        const QString sel = o.at(n, "text:select-page");
        if (sel.isEmpty() || sel == QLatin1String("current")) addField(pc, QStringLiteral("PAGE"), t.text(n), fmt);
        else addText(pc, t.text(n), fmt);
    } else if (o.is(n, "text:page-count")) addField(pc, QStringLiteral("NUMPAGES"), t.text(n), fmt);
    else if (o.is(n, "text:expression") && !o.at(n, "text:formula").isEmpty()) {
        // A formula: the cell names are written A1 (a file may write them [.A1:.B3] or <A1:B3>), and the field's own words are the result.
        QString formula = o.at(n, "text:formula");
        for (const char *prefix : {"of:=", "ooow:=", "oooc:=", "ooow:", "oooc:", "of:", "="})
            if (formula.startsWith(QLatin1String(prefix))) {
                formula = formula.mid(int(qstrlen(prefix)));
                break;
            }
        formula.remove(QLatin1String("[.")).remove(QLatin1String(":.")).remove(QLatin1Char('[')).remove(QLatin1Char(']')).remove(QLatin1Char('<')).remove(QLatin1Char('>'));
        formula = formula.trimmed();
        const QString picture = t.attrAny(n, "number-picture");
        if (formula.isEmpty()) addText(pc, t.text(n), fmt);
        else addField(pc, QStringLiteral("= ") + formula + (picture.isEmpty() ? QString() : QStringLiteral(" \\# \"") + picture + QLatin1Char('"')), t.text(n), fmt);
    }
    else if (o.is(n, "text:word-count")) addField(pc, QStringLiteral("NUMWORDS"), t.text(n), fmt);
    else if (o.is(n, "text:character-count")) addField(pc, QStringLiteral("NUMCHARS"), t.text(n), fmt);
    else if (o.is(n, "text:title")) addField(pc, QStringLiteral("TITLE"), t.text(n), fmt);
    else if (o.is(n, "text:subject")) addField(pc, QStringLiteral("SUBJECT"), t.text(n), fmt);
    else if (o.is(n, "text:author-name") || o.is(n, "text:initial-creator")) addField(pc, QStringLiteral("AUTHOR"), t.text(n), fmt);
    else if (o.is(n, "text:file-name")) addField(pc, QStringLiteral("FILENAME"), t.text(n), fmt);
    else if (o.is(n, "text:database-display") && !o.at(n, "text:column-name").isEmpty()) addField(pc, QStringLiteral("MERGEFIELD %1").arg(quoteFieldArgument(o.at(n, "text:column-name"))), t.text(n), fmt);
    else if (o.is(n, "text:bookmark-ref") || o.is(n, "text:reference-ref")) {
        const QString name = bookmarkNameOf(o.at(n, "text:ref-name"));
        const QString format = o.at(n, "text:reference-format");
        QString instr;
        if (format == QLatin1String("page")) instr = QStringLiteral("PAGEREF %1 \\h");
        else if (format == QLatin1String("direction")) instr = QStringLiteral("REF %1 \\p \\h");
        else if (format == QLatin1String("number") || format == QLatin1String("number-no-superior") || format == QLatin1String("number-all-superior")) instr = QStringLiteral("REF %1 \\n \\h");
        else instr = QStringLiteral("REF %1 \\h");
        if (name.isEmpty()) addText(pc, t.text(n), fmt);
        else addField(pc, instr.arg(quoteFieldArgument(name)), t.text(n), fmt);
    } else if (o.is(n, "text:sequence")) {
        const QString name = o.at(n, "text:name");
        const QString refName = o.at(n, "text:ref-name");
        int id = -1;
        if (!refName.isEmpty() && !m_bookmarkIds.contains(refName) && m_bookmarkId < kMaxMadeParts * 10) {
            id = ++m_bookmarkId;
            m_bookmarkIds.insert(refName, id);
            pc->runs += "<w:bookmarkStart w:id=\"" + num(id) + "\" w:name=\"" + esc(refName) + "\"/>";
        }
        if (name.isEmpty()) addText(pc, t.text(n), fmt);
        else addField(pc, QStringLiteral("SEQ %1 \\* ARABIC").arg(quoteFieldArgument(name)), t.text(n), fmt);
        if (id >= 0) pc->runs += "<w:bookmarkEnd w:id=\"" + num(id) + "\"/>";
    } else if (o.is(n, "text:ruby")) {
        const int base = o.child(n, "text:ruby-base");
        if (base >= 0) inlineContent(o, base, pc, fmt, depth + 1);
    } else if (o.is(n, "text:change-start")) {
        const auto it = m_changes.constFind(o.at(n, "text:change-id"));
        if (it != m_changes.constEnd() && it->insertion && pc->insId.isEmpty()) {
            pc->insId = o.at(n, "text:change-id");
            pc->insAt = pc->runs.size();
            pc->insLinkDepth = pc->linkDepth;
        }
    } else if (o.is(n, "text:change-end")) {
        if (!pc->insId.isEmpty() && pc->insId == o.at(n, "text:change-id")) {
            closeInsertion(pc);
            m_openInsertion.clear();
        }
    } else if (o.is(n, "text:change")) {
        const QByteArray d = deletionXml(o, o.at(n, "text:change-id"), pc);
        pc->runs += d;
    } else if (o.is(n, "text:ruby-text") || o.is(n, "text:toc-mark") || o.is(n, "text:toc-mark-start") || o.is(n, "text:toc-mark-end") || o.is(n, "text:alphabetical-index-mark") ||
               o.is(n, "text:alphabetical-index-mark-start") || o.is(n, "text:alphabetical-index-mark-end") || o.is(n, "text:user-index-mark") || o.is(n, "text:bibliography-mark") ||
               o.is(n, "text:hidden-paragraph") || o.is(n, "text:hidden-text") ||
               o.is(n, "svg:title") || o.is(n, "svg:desc") || o.is(n, "office:event-listeners") || o.is(n, "text:script") || o.is(n, "text:sequence-decls")) {
        // (A mark with no text of its own, a change's point, or what a person did not ask to be shown.)
    } else if (o.is(n, "draw:g")) {
        inlineContent(o, n, pc, fmt, depth + 1);
    } else if (o.is(n, "draw:enhanced-geometry") || o.is(n, "draw:glue-point") || o.is(n, "draw:equation") || o.is(n, "draw:handle") || o.is(n, "draw:param")) {
    } else if (o.nameOf(n).startsWith(QLatin1String("draw:")) || o.nameOf(n).startsWith(QLatin1String("dr3d:"))) {
        bool ok = false;
        const QByteArray d = o.nameOf(n).startsWith(QLatin1String("dr3d:")) ? QByteArray() : drawing(o, n, pc, &ok);
        if (!d.isEmpty()) {
            addRun(pc, d, fmt);
            pc->lastSpace = false;
        } else if (o.nameOf(n).startsWith(QLatin1String("dr3d:"))) left(QCoreApplication::translate("odt", "drawing shapes"));
    } else {
        // Anything else (a field with its text, a meta, a text input) holds text that is kept.
        inlineContent(o, n, pc, fmt, depth + 1);
    }
}

void Reader::closeInsertion(ParaCtx *pc)
{
    if (pc->insId.isEmpty()) return;
    const Change c = m_changes.value(pc->insId);
    if (pc->linkDepth == pc->insLinkDepth && pc->insAt <= pc->runs.size() && pc->runs.size() > pc->insAt) {
        QString date = c.date.left(19);
        if (!date.isEmpty()) date += QLatin1Char('Z');
        const QByteArray open = "<w:ins w:id=\"" + num(++m_revisionId) + "\" w:author=\"" + esc(c.author.isEmpty() ? QStringLiteral("Author") : c.author) + "\"" + (date.isEmpty() ? QByteArray() : " w:date=\"" + esc(date) + "\"") + ">";
        pc->runs.insert(pc->insAt, open);
        pc->runs += "</w:ins>";
    }
    pc->insId.clear();
}

QByteArray Reader::deletionXml(const Odf &o, const QString &id, ParaCtx *pc)
{
    const auto it = m_changes.constFind(id);
    if (it == m_changes.constEnd() || !it->deletion || pc->depth > kMaxNesting) return QByteArray();
    const wml::XmlTree &t = o.tree();
    const int del = o.child(it->node, "text:deletion");
    if (del < 0) return QByteArray();
    QByteArray runs;
    bool first = true;
    for (int c = t.firstChild(del); c >= 0; c = t.next(c)) {
        if (!o.is(c, "text:p") && !o.is(c, "text:h")) continue;
        ParaCtx inner;
        inner.depth = pc->depth + 1;
        inner.inNote = true;   // (a note or comment in deleted text is not kept)
        QString named, listStyle;
        inner.base = &ownProps(QStringLiteral("paragraph"), o.at(c, "text:style-name"), &named);
        if (!first) inner.runs += "<w:r><w:br/></w:r>";
        inlineContent(o, c, &inner, RunFmt(), 0);
        runs += inner.runs;
        first = false;
    }
    if (runs.isEmpty()) return QByteArray();
    // (The text of a deleted run is w:delText.)
    runs.replace("<w:t ", "<w:delText ").replace("<w:t>", "<w:delText>").replace("</w:t>", "</w:delText>");
    runs.replace("<w:instrText ", "<w:delInstrText ").replace("<w:instrText>", "<w:delInstrText>").replace("</w:instrText>", "</w:delInstrText>");
    QString date = it->date.left(19);
    if (!date.isEmpty()) date += QLatin1Char('Z');
    return "<w:del w:id=\"" + num(++m_revisionId) + "\" w:author=\"" + esc(it->author.isEmpty() ? QStringLiteral("Author") : it->author) + "\"" + (date.isEmpty() ? QByteArray() : " w:date=\"" + esc(date) + "\"") + ">" + runs + "</w:del>";
}

void Reader::collectChanges(const Odf &o)
{
    const wml::XmlTree &t = o.tree();
    for (int i = 0; i < t.count(); ++i) {
        if (!o.is(i, "text:changed-region")) continue;
        Change c;
        c.node = i;
        for (int k = t.firstChild(i); k >= 0; k = t.next(k)) {
            c.insertion = c.insertion || o.is(k, "text:insertion");
            c.deletion = c.deletion || o.is(k, "text:deletion");
            const int info = o.child(k, "office:change-info");
            if (info >= 0) {
                const int a = o.child(info, "dc:creator"), d = o.child(info, "dc:date");
                if (a >= 0) c.author = t.text(a);
                if (d >= 0) c.date = t.text(d);
            }
        }
        if (m_changes.size() < kMaxMadeParts * 5) m_changes.insert(o.at(i, "text:id"), c);
    }
}

QByteArray Reader::commentXml(const Odf &o, int annotation, ParaCtx *pc)
{
    if (pc->inNote || m_commentCount >= kMaxMadeParts) return QByteArray();
    const wml::XmlTree &t = o.tree();
    const int id = m_commentCount++;
    QByteArray body;
    QString author, date, initials;
    for (int c = t.firstChild(annotation); c >= 0; c = t.next(c)) {
        if (o.is(c, "dc:creator")) author = t.text(c);
        else if (o.is(c, "dc:date")) date = t.text(c);
        else if (o.is(c, "meta:creator-initials")) initials = t.text(c);
    }
    const QString oldOwner = m_owner;
    m_owner = QStringLiteral("/word/comments.xml");
    QVector<Block> blocks;
    const int bodyNode = annotation;
    // The paragraphs of the comment are the text:p and text:list children of the annotation.
    convertBlocks(o, bodyNode, &blocks, kMaxNesting - 4);
    m_owner = oldOwner;
    body = bodyBlocksXml(blocks, true);
    if (date.size() >= 19) date = date.left(19) + QLatin1Char('Z');
    m_commentsBody += "<w:comment w:id=\"" + num(id) + "\" w:author=\"" + esc(author.isEmpty() ? QStringLiteral("Author") : author) + "\"" + (date.isEmpty() ? QByteArray() : " w:date=\"" + esc(date) + "\"") +
                      (initials.isEmpty() ? QByteArray() : " w:initials=\"" + esc(initials) + "\"") + ">" + body + "</w:comment>";
    const QString name = o.at(annotation, "office:name");
    if (!name.isEmpty() && m_annotationEnds.contains(name)) {
        m_commentOpen.insert(name, id);
        return "<w:commentRangeStart w:id=\"" + num(id) + "\"/>";
    }
    return "<w:commentRangeStart w:id=\"" + num(id) + "\"/><w:commentRangeEnd w:id=\"" + num(id) + "\"/><w:r><w:commentReference w:id=\"" + num(id) + "\"/></w:r>";
}

QByteArray Reader::noteReference(const Odf &o, int note, ParaCtx *pc, const RunFmt &fmt)
{
    Q_UNUSED(fmt);
    const wml::XmlTree &t = o.tree();
    if (pc->inNote) return QByteArray();
    const bool endnote = o.at(note, "text:note-class") == QLatin1String("endnote");
    int &count = endnote ? m_endnoteCount : m_footnoteCount;
    if (m_footnoteCount + m_endnoteCount >= kMaxMadeParts) return QByteArray();
    const int id = ++count;
    const int body = o.child(note, "text:note-body");
    QVector<Block> blocks;
    const QString oldOwner = m_owner;
    m_owner = endnote ? QStringLiteral("/word/endnotes.xml") : QStringLiteral("/word/footnotes.xml");
    if (body >= 0) {
        m_inNote = true;
        convertBlocks(o, body, &blocks, kMaxNesting - 4);
        m_inNote = false;
    }
    m_owner = oldOwner;
    const QByteArray ref = endnote ? "<w:r><w:rPr><w:rStyle w:val=\"EndnoteReference\"/></w:rPr><w:endnoteRef/></w:r>" : "<w:r><w:rPr><w:rStyle w:val=\"FootnoteReference\"/></w:rPr><w:footnoteRef/></w:r>";
    if (blocks.isEmpty() || !blocks.first().para) {
        Block b;
        b.para = true;
        blocks.prepend(b);
    }
    blocks.first().content = ref + "<w:r><w:t xml:space=\"preserve\"> </w:t></w:r>" + blocks.first().content;
    const QByteArray tag = endnote ? "w:endnote" : "w:footnote";
    (endnote ? m_endnotesBody : m_footnotesBody) += "<" + tag + " w:id=\"" + num(id) + "\">" + bodyBlocksXml(blocks, true) + "</" + tag + ">";
    Q_UNUSED(t);
    return QByteArray(endnote ? "<w:r><w:rPr><w:rStyle w:val=\"EndnoteReference\"/></w:rPr><w:endnoteReference w:id=\"" : "<w:r><w:rPr><w:rStyle w:val=\"FootnoteReference\"/></w:rPr><w:footnoteReference w:id=\"") + num(id) +
           "\"/></w:r>";
}

// ---- blocks

QByteArray Reader::blockXml(const Block &b) const
{
    if (!b.para) return b.xml;
    QByteArray ppr = b.pPr;
    if (!b.sect.isEmpty()) ppr += b.sect;
    return "<w:p>" + (ppr.isEmpty() ? QByteArray() : "<w:pPr>" + ppr + "</w:pPr>") + b.content + "</w:p>";
}

QByteArray Reader::bodyBlocksXml(const QVector<Block> &blocks, bool lastMustBeParagraph) const
{
    QByteArray x;
    bool prevTable = false;
    for (const Block &b : blocks) {
        // (Two tables with no paragraph between them would be one table.)
        if (!b.para && prevTable) x += "<w:p/>";
        x += blockXml(b);
        prevTable = !b.para;
    }
    if (blocks.isEmpty() || (lastMustBeParagraph && prevTable)) x += "<w:p/>";
    return x;
}

void Reader::convertBlocks(const Odf &o, int parent, QVector<Block> *out, int depth, int listLevel, const QString &listStyle, int listNum)
{
    const wml::XmlTree &t = o.tree();
    if (depth > kMaxNesting) {
        left(QCoreApplication::translate("odt", "content nested too deeply"));
        return;
    }
    for (int c = t.firstChild(parent); c >= 0; c = t.next(c)) convertBlock(o, c, out, depth, listLevel, listStyle, listNum);
}

void Reader::convertBlock(const Odf &o, int n, QVector<Block> *out, int depth, int listLevel, const QString &listStyle, int listNum)
{
    if (o.is(n, "text:p") || o.is(n, "text:h")) convertParagraph(o, n, out, listLevel, listStyle, listNum, false, depth);
    else if (o.is(n, "text:list")) convertList(o, n, out, depth, listLevel, listStyle, listNum);
    else if (o.is(n, "table:table")) convertTable(o, n, out, depth);
    else if (o.is(n, "text:section")) convertSection(o, n, out, depth, listLevel, listStyle, listNum);
    else if (o.is(n, "text:table-of-content") || o.is(n, "text:illustration-index") || o.is(n, "text:alphabetical-index") || o.is(n, "text:table-index") || o.is(n, "text:object-index") ||
             o.is(n, "text:user-index") || o.is(n, "text:bibliography"))
        convertIndex(o, n, out, depth);
    else if (o.is(n, "draw:frame")) {
        // A picture or text box that stands where a paragraph would: it is set in a paragraph of its own.
        ParaCtx pc;
        pc.depth = depth;
        bool ok = false;
        const QByteArray d = drawing(o, n, &pc, &ok);
        if (!d.isEmpty()) {
            Block b;
            b.para = true;
            addRun(&pc, d, RunFmt());
            b.content = pc.runs;
            out->push_back(b);
        }
    } else if (o.is(n, "style:region-left") || o.is(n, "style:region-center") || o.is(n, "style:region-right")) {
        convertBlocks(o, n, out, depth, listLevel, listStyle, listNum);
    } else if (o.is(n, "text:soft-page-break") || o.is(n, "text:tracked-changes") || o.is(n, "text:variable-decls") || o.is(n, "text:sequence-decls") || o.is(n, "text:user-field-decls") ||
               o.is(n, "text:alphabetical-index-auto-mark-file") || o.is(n, "office:forms") || o.is(n, "text:dde-connection-decls") || o.is(n, "text:change") || o.is(n, "text:change-start") ||
               o.is(n, "text:change-end") || o.is(n, "table:calculation-settings") || o.is(n, "text:number-lines-config") || o.is(n, "svg:title") || o.is(n, "svg:desc")) {
    } else if (o.is(n, "office:annotation") || o.is(n, "office:annotation-end")) {
    } else if (o.is(n, "table:table-header-rows") || o.is(n, "table:table-rows") || o.is(n, "table:table-row-group") || o.is(n, "text:list-item") || o.is(n, "text:list-header")) {
        convertBlocks(o, n, out, depth, listLevel, listStyle, listNum);
    } else if (o.is(n, "draw:g") || o.is(n, "draw:custom-shape") || o.is(n, "draw:rect") || o.is(n, "draw:ellipse")) {
        ParaCtx pc;
        pc.depth = depth;
        pc.inNote = m_inNote;
        bool ok = false;
        const QByteArray d = o.is(n, "draw:g") ? QByteArray() : drawing(o, n, &pc, &ok);
        if (o.is(n, "draw:g")) left(QCoreApplication::translate("odt", "drawing shapes"));
        if (!d.isEmpty()) {
            Block b;
            b.para = true;
            addRun(&pc, d, RunFmt());
            b.content = pc.runs;
            out->push_back(b);
        }
    } else if (o.is(n, "draw:enhanced-geometry") || o.is(n, "draw:glue-point") || o.is(n, "draw:equation") || o.is(n, "draw:handle")) {
    } else if (o.nameOf(n).startsWith(QLatin1String("draw:"))) {
        left(QCoreApplication::translate("odt", "drawing shapes"));
    }
}

void Reader::convertSection(const Odf &o, int n, QVector<Block> *out, int depth, int listLevel, const QString &listStyle, int listNum)
{
    const QString name = o.at(n, "text:style-name");
    const Props &sp = resolved(QStringLiteral("section"), name);
    const int count = sp.value(QStringLiteral("sect:count")).toInt();
    const bool main = depth == 0 && listLevel < 0 && m_sectionsOn;
    const SectionState outer = m_sect;
    const bool columns = main && count > 1;
    if (columns) {
        SectionState next = m_sect;
        next.columns = count;
        bool ok = false;
        const qint64 gap = lengthToTwips(sp.value(QStringLiteral("sect:gap")), &ok);
        next.columnGap = ok ? gap : 720;
        next.separator = sp.contains(QStringLiteral("sect:sep"));
        next.continuous = true;
        endSection(out, next);
    }
    convertBlocks(o, n, out, depth, listLevel, listStyle, listNum);
    if (columns) {
        SectionState back = outer;
        back.continuous = true;
        endSection(out, back);
    }
}

void Reader::convertList(const Odf &o, int n, QVector<Block> *out, int depth, int listLevel, const QString &listStyle, int listNum)
{
    const wml::XmlTree &t = o.tree();
    const int level = listLevel + 1;
    if (level >= 9 || depth > kMaxNesting) {
        // Deeper than a list goes: its text is kept at the last level.
        for (int item = t.firstChild(n); item >= 0; item = t.next(item)) convertBlocks(o, item, out, depth + 1, std::min(level, 8), listStyle, listNum);
        return;
    }
    QString style = o.at(n, "text:style-name");
    int num = listNum;
    if (level == 0 || (!style.isEmpty() && style != listStyle)) {
        if (style.isEmpty()) style = listStyle;
        const QString continues = o.at(n, "text:continue-list");
        const bool cont = o.at(n, "text:continue-numbering") == QLatin1String("true") || !continues.isEmpty();
        // (A list that names the list it goes on from goes on from that one, not from the last of its style.)
        const auto named = continues.isEmpty() ? m_listById.constEnd() : m_listById.constFind(continues);
        num = named != m_listById.constEnd() ? named.value() : numberOf(style, !cont);
        const int xmlNs = t.namespaceId("http://www.w3.org/XML/1998/namespace");
        const QString id = xmlNs >= 0 ? t.attr(n, "id", xmlNs) : QString();
        if (!id.isEmpty() && m_listById.size() < kMaxMadeParts) m_listById.insert(id, num);
    } else style = listStyle;
    if (num <= 0) {
        convertBlocks(o, n, out, depth + 1, std::max(listLevel, 0), listStyle, listNum);
        return;
    }
    for (int item = t.firstChild(n); item >= 0; item = t.next(item)) {
        const bool header = o.is(item, "text:list-header");
        if (!header && !o.is(item, "text:list-item")) continue;
        bool first = true;
        int itemNum = num;
        const QString start = o.at(item, "text:start-value");
        if (!header && !start.isEmpty() && m_nums.size() < kMaxMadeParts) {
            // An item that starts the count again: a number of its own that begins there.
            m_nums.push_back(NumUse{m_nums.value(num - 1).listStyle, m_nums.value(num - 1).abstractId, true, level, start.toInt()});
            itemNum = int(m_nums.size());
            m_lastNumOf.insert(m_nums.last().listStyle, itemNum);
            num = itemNum;
        }
        for (int c = t.firstChild(item); c >= 0; c = t.next(c)) {
            if (o.is(c, "text:p") || o.is(c, "text:h")) {
                convertParagraph(o, c, out, level, style, itemNum, first && !header, depth + 1);
                first = false;
            } else convertBlock(o, c, out, depth + 1, level, style, itemNum);
        }
    }
}

void Reader::startSection(const QString &master)
{
    m_sect.master = master;
}

void Reader::endSection(QVector<Block> *out, const SectionState &next)
{
    // The section that has been made ends with the last paragraph before; the one that begins is `next`.
    if (out->isEmpty()) {
        m_sect = next;
        return;
    }
    Block &last = out->last();
    const QByteArray sect = "<w:sectPr>" + sectPrXml(m_sect) + "</w:sectPr>";
    if (last.para && last.sect.isEmpty()) last.sect = sect;
    else {
        Block b;
        b.para = true;
        b.sect = sect;
        out->push_back(b);
    }
    m_sect = next;
}

void Reader::convertParagraph(const Odf &o, int n, QVector<Block> *out, int listLevel, const QString &listStyle, int listNum, bool numbered, int depth)
{
    const QString styleName = o.at(n, "text:style-name");
    QString named, listFromStyle, master;
    QByteArray tabs;
    const Props &pp = ownProps(QStringLiteral("paragraph"), styleName, &named, &tabs, &listFromStyle, &master);
    if (named.isEmpty() && !styleName.isEmpty()) {
        // (A style that has no named parent: the ones the paragraph's own parent chain gave it are the whole.)
        const StyleDef *d = style(QStringLiteral("paragraph"), styleName);
        if (d && !d->automatic) named = styleName;
    }
    // The master page of the paragraph's style starts a new page on it.
    if (master.isEmpty() && !named.isEmpty()) {
        const StyleDef *d = style(QStringLiteral("paragraph"), named);
        if (d) master = d->masterPage;
    }
    const bool main = depth == 0 && listLevel < 0 && m_sectionsOn;
    if (main && !master.isEmpty()) {
        if (out->isEmpty() && !m_sectionStarted) m_sect.master = master;
        else {
            SectionState next = m_sect;
            next.master = master;
            next.continuous = false;
            endSection(out, next);
        }
    }
    if (main) m_sectionStarted = true;
    const bool heading = o.is(n, "text:h");
    int outline = 0;
    if (heading) {
        outline = std::max(1, std::min(9, o.at(n, "text:outline-level").isEmpty() ? 1 : o.at(n, "text:outline-level").toInt()));
        const StyleDef *d = named.isEmpty() ? nullptr : style(QStringLiteral("paragraph"), named);
        if (d && d->outlineLevel > 0) outline = 0;   // (its style says it)
    }
    QByteArray pStyle;
    if (!named.isEmpty()) pStyle = "<w:pStyle w:val=\"" + esc(styleId(QStringLiteral("paragraph"), named)) + "\"/>";
    QByteArray numPr;
    if (numbered && listLevel >= 0 && listNum > 0) numPr = "<w:numPr><w:ilvl w:val=\"" + num(std::min(listLevel, 8)) + "\"/><w:numId w:val=\"" + num(listNum) + "\"/></w:numPr>";
    else if (listLevel >= 0 && !numbered && listNum > 0) numPr = "<w:numPr><w:ilvl w:val=\"" + num(std::min(listLevel, 8)) + "\"/><w:numId w:val=\"0\"/></w:numPr>";
    if (numPr.isEmpty() && listLevel < 0 && listFromStyle == QLatin1String("\x01none")) numPr = "<w:numPr><w:ilvl w:val=\"0\"/><w:numId w:val=\"0\"/></w:numPr>";
    if (numPr.isEmpty() && heading && m_hasOutline && listFromStyle.isEmpty()) {
        // A heading of an outline whose level is numbered is numbered.
        const int lv = std::max(1, std::min(9, o.at(n, "text:outline-level").isEmpty() ? 1 : o.at(n, "text:outline-level").toInt()));
        const StyleDef *d = named.isEmpty() ? nullptr : style(QStringLiteral("paragraph"), named);
        if (m_outline.level[lv - 1].defined && !m_outline.level[lv - 1].numFmt.isEmpty() && !(d && !d->listStyle.isEmpty())) {
            const int numId = numberOf(QStringLiteral("Outline"), false);
            if (numId > 0) numPr = "<w:numPr><w:ilvl w:val=\"" + num(lv - 1) + "\"/><w:numId w:val=\"" + num(numId) + "\"/></w:numPr>";
        }
    }
    // (Made once for each style and each set of the other things it is made from, as a run's look is: three million paragraphs of a few styles are not worked out one by one.)
    QByteArray ppr;
    {
        constexpr int kMostParagraphLooks = 50000;
        const QByteArray key = QByteArray::number(quintptr(&pp)) + '|' + tabs + '|' + pStyle + '|' + numPr + '|' + QByteArray::number(outline) + (m_pendingPageBreak ? "|b" : "|n");
        const auto cached = m_pPrCache.constFind(key);
        if (cached != m_pPrCache.constEnd()) ppr = cached.value();
        else {
            ppr = pPrXml(pp, tabs, pStyle, numPr, outline, m_pendingPageBreak);
            if (m_pPrCache.size() < kMostParagraphLooks) m_pPrCache.insert(key, ppr);
        }
    }
    m_pendingPageBreak = pp.value(QStringLiteral("para:fo:break-after")) == QLatin1String("page");
    QByteArray markProps;
    {
        // The look of the paragraph mark, when the file keeps it (loext:marker-style-name).
        const QString marker = o.at(n, "loext:marker-style-name");
        if (marker.isEmpty()) {
            // (Made once for each paragraph style, as a run's look is: a copy of the style's properties and the markup made from them for each of three million paragraphs takes most of the time of reading them.)
            const QByteArray key = "mark#" + QByteArray::number(quintptr(&pp));
            auto cached = m_rPrCache.constFind(key);
            if (cached == m_rPrCache.constEnd()) cached = m_rPrCache.insert(key, rPrXml(pp, QString()));
            markProps = cached.value();
        } else {
            Props markBase = pp;
            QString markNamed;
            const Props &own = ownProps(QStringLiteral("text"), marker, &markNamed);
            for (auto it = own.constBegin(); it != own.constEnd(); ++it) markBase.insert(it.key(), it.value());
            markProps = rPrXml(markBase, QString());
        }
    }
    if (!markProps.isEmpty()) ppr += "<w:rPr>" + markProps + "</w:rPr>";
    ParaCtx pc;
    pc.base = &pp;
    pc.depth = depth;
    pc.inNote = m_inNote;
    RunFmt fmt;
    if (!m_openInsertion.isEmpty()) {
        pc.insId = m_openInsertion;
        pc.insAt = 0;
        pc.insLinkDepth = 0;
    }
    inlineContent(o, n, &pc, fmt, 0);
    if (!pc.insId.isEmpty()) {
        // An insertion that runs on past this paragraph.
        const QString id = pc.insId;
        closeInsertion(&pc);
        m_openInsertion = id;
    }
    Block b;
    b.para = true;
    b.pPr = ppr;
    b.content = pc.runs;
    out->push_back(b);
}

// ---- the table of contents and the other indexes

void Reader::convertIndex(const Odf &o, int n, QVector<Block> *out, int depth)
{
    const wml::XmlTree &t = o.tree();
    const bool toc = o.is(n, "text:table-of-content");
    QByteArray instr;
    if (toc) {
        const int src = o.child(n, "text:table-of-content-source");
        int levels = 3;
        if (src >= 0 && !o.at(src, "text:outline-level").isEmpty()) levels = std::max(1, std::min(9, o.at(src, "text:outline-level").toInt()));
        instr = " TOC \\o \"1-" + num(levels) + "\" \\h \\z \\u ";
    } else if (o.is(n, "text:illustration-index") || o.is(n, "text:table-index")) {
        const int src = o.child(n, o.is(n, "text:illustration-index") ? "text:illustration-index-source" : "text:table-index-source");
        QString cat = src >= 0 ? o.at(src, "text:caption-sequence-name") : QString();
        if (cat.isEmpty()) cat = o.is(n, "text:illustration-index") ? QStringLiteral("Illustration") : QStringLiteral("Table");
        instr = " TOC \\h \\z \\c " + esc(quoteFieldArgument(cat), false) + " ";
    }
    const int body = o.child(n, "text:index-body");
    if (body < 0) return;
    QVector<Block> blocks;
    // The title is paragraphs of its own, before the field.
    QVector<Block> entries;
    for (int c = t.firstChild(body); c >= 0; c = t.next(c)) {
        if (o.is(c, "text:index-title")) convertBlocks(o, c, out, depth + 1);
        else convertBlock(o, c, &entries, depth + 1, -1, QString(), 0);
    }
    if (!instr.isEmpty() && !entries.isEmpty() && entries.first().para && entries.last().para) {
        entries.first().content = "<w:r><w:fldChar w:fldCharType=\"begin\"/></w:r><w:r><w:instrText xml:space=\"preserve\">" + instr + "</w:instrText></w:r><w:r><w:fldChar w:fldCharType=\"separate\"/></w:r>" +
                                  entries.first().content;
        entries.last().content += "<w:r><w:fldChar w:fldCharType=\"end\"/></w:r>";
    }
    for (const Block &b : std::as_const(entries)) out->push_back(b);
}

// ---- tables

namespace {

struct Cell {
    QByteArray xml;
};

}

void Reader::convertTable(const Odf &o, int n, QVector<Block> *out, int depth)
{
    const wml::XmlTree &t = o.tree();
    if (depth >= kMaxNesting) {
        left(QCoreApplication::translate("odt", "tables nested too deeply"));
        return;
    }
    const QString styleName = o.at(n, "table:style-name");
    const Props &tp = resolved(QStringLiteral("table"), styleName);
    auto tg = [&](const char *k) { return tp.value(QStringLiteral("tbl:") + QLatin1String(k)); };
    // The master page of the table's style begins a page.
    {
        const StyleDef *d = style(QStringLiteral("table"), styleName);
        const bool main = depth == 0 && m_sectionsOn;
        if (d && main && !d->masterPage.isEmpty()) {
            SectionState next = m_sect;
            next.master = d->masterPage;
            next.continuous = false;
            if (out->isEmpty() && !m_sectionStarted) m_sect.master = d->masterPage;
            else endSection(out, next);
        }
        if (main) m_sectionStarted = true;
        if (tg("fo:break-before") == QLatin1String("page")) m_pendingPageBreak = true;
    }
    // The columns: their widths, from the grid's column styles.
    QVector<qint64> widths;
    QVector<int> headerRows;
    struct RowRef {
        int node;
        bool header;
    };
    QVector<RowRef> rows;
    std::vector<int> stack;
    // Walks the table's children in order (the columns and rows may be in groups).
    struct Frame {
        int parent, next;
        bool header;
    };
    QVector<Frame> frames{{n, t.firstChild(n), false}};
    qint64 relTotal = 0;
    QVector<double> rels;
    while (!frames.isEmpty()) {
        Frame &f = frames.back();
        if (f.next < 0) {
            frames.pop_back();
            continue;
        }
        const int c = f.next;
        f.next = t.next(c);
        const bool header = f.header;
        if (o.is(c, "table:table-column")) {
            const Props &cp = resolved(QStringLiteral("table-column"), o.at(c, "table:style-name"));
            bool ok = false;
            qint64 w = lengthToTwips(cp.value(QStringLiteral("col:style:column-width")), &ok);
            double rel = 0;
            const QString rs = cp.value(QStringLiteral("col:style:rel-column-width"));
            if (rs.endsWith(QLatin1Char('*'))) rel = rs.left(rs.size() - 1).toDouble();
            int rep = std::max(1, std::min(o.at(c, "table:number-columns-repeated").isEmpty() ? 1 : o.at(c, "table:number-columns-repeated").toInt(), kMaxColumns));
            for (int i = 0; i < rep && widths.size() < kMaxColumns; ++i) {
                widths.push_back(ok ? w : -1);
                rels.push_back(rel);
            }
        } else if (o.is(c, "table:table-columns") || o.is(c, "table:table-header-columns") || o.is(c, "table:table-column-group")) {
            if (frames.size() < kMaxNesting) frames.push_back(Frame{c, t.firstChild(c), false});
        } else if (o.is(c, "table:table-row")) {
            rows.push_back(RowRef{c, header});
        } else if (o.is(c, "table:table-header-rows") || o.is(c, "table:table-rows") || o.is(c, "table:table-row-group")) {
            if (frames.size() < kMaxNesting) frames.push_back(Frame{c, t.firstChild(c), header || o.is(c, "table:table-header-rows")});
        }
    }
    Q_UNUSED(relTotal);
    Q_UNUSED(headerRows);
    Q_UNUSED(stack);
    // The width of the table, and the columns that gave none.
    bool wOk = false;
    qint64 tableWidth = lengthToTwips(tg("style:width"), &wOk);
    double relWidth = 0;
    if (!wOk && percentOf(tg("style:rel-width"), &relWidth)) {
        tableWidth = qint64(m_pageTextWidth * relWidth / 100.0);
        wOk = true;
    }
    qint64 known = 0;
    int unknown = 0;
    double relSum = 0;
    for (int i = 0; i < widths.size(); ++i) {
        if (widths[i] >= 0) known += widths[i];
        else {
            ++unknown;
            relSum += rels[i];
        }
    }
    if (unknown > 0) {
        const qint64 total = wOk ? tableWidth : qint64(m_pageTextWidth);
        const qint64 rest = std::max<qint64>(total - known, 720 * unknown);
        for (int i = 0; i < widths.size(); ++i)
            if (widths[i] < 0) widths[i] = relSum > 0 ? qint64(rest * rels[i] / relSum) : rest / unknown;
    }
    // A table whose rows have more cells than it has columns has more columns.
    int gridCols = int(widths.size());
    // ---- the rows
    QByteArray rowsXml;
    QVector<int> vmRemaining, vmSpan;
    qint64 cellsMade = 0;
    int rowCount = 0;
    struct RowOut {
        QByteArray xml;
    };
    // (The grid grows as a row has more cells; the rows are made first and the grid written after.)
    QVector<QByteArray> rowXmls;
    for (const RowRef &rr : std::as_const(rows)) {
        const int rowRepeat = std::max(1, std::min(o.at(rr.node, "table:number-rows-repeated").isEmpty() ? 1 : o.at(rr.node, "table:number-rows-repeated").toInt(), kMaxRepeat));
        const Props &rp = resolved(QStringLiteral("table-row"), o.at(rr.node, "table:style-name"));
        QByteArray trPr;
        {
            bool ok = false;
            qint64 h = lengthToTwips(rp.value(QStringLiteral("row:style:row-height")), &ok);
            if (!ok) h = lengthToTwips(rp.value(QStringLiteral("row:style:min-row-height")), &ok);
            if (ok && h > 0) trPr += "<w:trHeight w:val=\"" + num(h) + "\" w:hRule=\"atLeast\"/>";
            if (rp.value(QStringLiteral("row:fo:keep-together")) == QLatin1String("always")) trPr = "<w:cantSplit/>" + trPr;
            if (rr.header) trPr += "<w:tblHeader/>";
        }
        QByteArray cells;
        int col = 0, pendingCover = 0;
        // The cells of the row, with the repeats made.
        for (int c = t.firstChild(rr.node); c >= 0; c = t.next(c)) {
            const bool covered = o.is(c, "table:covered-table-cell");
            if (!covered && !o.is(c, "table:table-cell")) continue;
            int rep = std::max(1, std::min(o.at(c, "table:number-columns-repeated").isEmpty() ? 1 : o.at(c, "table:number-columns-repeated").toInt(), kMaxColumns));
            const bool hasContent = t.firstChild(c) >= 0;
            if (!hasContent && rep > 1 && t.next(c) < 0 && col >= gridCols) break;   // (empty cells that run to the end of the row)
            QByteArray cellXml;
            bool built = false;
            int curSpan = 1, curRowSpan = 1;
            for (int r = 0; r < rep; ++r) {
                if (col >= kMaxColumns || ++cellsMade > kMaxTableCells) break;
                if (col >= vmRemaining.size()) {
                    vmRemaining.resize(col + 1);
                    vmSpan.resize(col + 1);
                }
                auto widthOf = [&](int from, int span) {
                    qint64 w = 0;
                    for (int k = from; k < from + span; ++k) w += k < widths.size() ? widths[k] : 1440;
                    return w;
                };
                if (pendingCover > 0 && covered) {
                    --pendingCover;
                    ++col;
                    continue;
                }
                if (vmRemaining[col] > 0 && covered) {
                    const int span = std::max(1, vmSpan[col]);
                    --vmRemaining[col];
                    cells += "<w:tc><w:tcPr><w:tcW w:w=\"" + num(widthOf(col, span)) + "\" w:type=\"dxa\"/>" + (span > 1 ? "<w:gridSpan w:val=\"" + num(span) + "\"/>" : QByteArray()) + "<w:vMerge/></w:tcPr><w:p/></w:tc>";
                    pendingCover = span - 1;
                    ++col;
                    continue;
                }
                if (covered) {
                    ++col;
                    continue;
                }
                if (!built) {
                    // The cell: its content once, and the properties.
                    int span = std::max(1, std::min(o.at(c, "table:number-columns-spanned").isEmpty() ? 1 : o.at(c, "table:number-columns-spanned").toInt(), kMaxColumns));
                    const int rowSpan = std::max(1, std::min(o.at(c, "table:number-rows-spanned").isEmpty() ? 1 : o.at(c, "table:number-rows-spanned").toInt(), 100000));
                    const Props &cp = resolved(QStringLiteral("table-cell"), o.at(c, "table:style-name"));
                    QVector<Block> blocks;
                    convertBlocks(o, c, &blocks, depth + 1);
                    QByteArray tcPr;
                    // (The width is set after, when the column of the cell is known: a placeholder.)
                    tcPr += "<w:tcW w:w=\"@W@\" w:type=\"dxa\"/>";
                    if (span > 1) tcPr += "<w:gridSpan w:val=\"" + num(span) + "\"/>";
                    if (rowSpan > 1) tcPr += "<w:vMerge w:val=\"restart\"/>";
                    tcPr += bordersXml(cp, QStringLiteral("cell:"), "tcBorders", true);
                    {
                        const QByteArray shd = shadingXml(cp.value(QStringLiteral("cell:fo:background-color")));
                        tcPr += shd;
                    }
                    {
                        auto pad = [&](const char *side) {
                            QString v = cp.value(QStringLiteral("cell:fo:padding-") + QLatin1String(side));
                            if (v.isEmpty()) v = cp.value(QStringLiteral("cell:fo:padding"));
                            bool ok = false;
                            const qint64 tw = lengthToTwips(v, &ok);
                            return ok ? tw : -1;
                        };
                        QByteArray mar;
                        const qint64 top = pad("top"), left = pad("left"), bottom = pad("bottom"), right = pad("right");
                        if (top >= 0) mar += "<w:top w:w=\"" + num(top) + "\" w:type=\"dxa\"/>";
                        if (left >= 0) mar += "<w:left w:w=\"" + num(left) + "\" w:type=\"dxa\"/>";
                        if (bottom >= 0) mar += "<w:bottom w:w=\"" + num(bottom) + "\" w:type=\"dxa\"/>";
                        if (right >= 0) mar += "<w:right w:w=\"" + num(right) + "\" w:type=\"dxa\"/>";
                        if (!mar.isEmpty()) tcPr += "<w:tcMar>" + mar + "</w:tcMar>";
                    }
                    const QString va = cp.value(QStringLiteral("cell:style:vertical-align"));
                    if (va == QLatin1String("middle")) tcPr += "<w:vAlign w:val=\"center\"/>";
                    else if (va == QLatin1String("bottom")) tcPr += "<w:vAlign w:val=\"bottom\"/>";
                    cellXml = "<w:tc><w:tcPr>" + tcPr + "</w:tcPr>" + bodyBlocksXml(blocks, true) + "</w:tc>";
                    built = true;
                    curSpan = span;
                    curRowSpan = rowSpan;
                }
                const int span = curSpan;
                QByteArray xml = cellXml;
                xml.replace("@W@", num(widthOf(col, span)));
                if ((m_repeatedBytes += xml.size()) > kMaxRepeatedBytes) {
                    left(QCoreApplication::translate("odt", "a table with more repeated cells than are kept"));
                    break;
                }
                cells += xml;
                if (curRowSpan > 1) {
                    vmRemaining[col] = curRowSpan - 1;
                    vmSpan[col] = span;
                }
                pendingCover = span - 1;
                ++col;
            }
        }
        gridCols = std::max(gridCols, col);
        // The row has cells to the end of the grid.
        const int hadCols = col;
        QByteArray row = "<w:tr>";
        if (!trPr.isEmpty()) row += "<w:trPr>" + trPr + "</w:trPr>";
        row += cells;
        if (hadCols < gridCols) {
            qint64 w = 0;
            for (int k = hadCols; k < gridCols; ++k) w += k < widths.size() ? widths[k] : 1440;
            row += "<w:tc><w:tcPr><w:tcW w:w=\"" + num(w) + "\" w:type=\"dxa\"/>" + (gridCols - hadCols > 1 ? "<w:gridSpan w:val=\"" + num(gridCols - hadCols) + "\"/>" : QByteArray()) + "</w:tcPr><w:p/></w:tc>";
        }
        row += "</w:tr>";
        if (cells.isEmpty() && hadCols == 0) row = "<w:tr><w:tc><w:p/></w:tc></w:tr>";
        const qint64 cellsInRow = std::max<qint64>(1, col);
        for (int r = 0; r < rowRepeat && rowCount < 200000; ++r, ++rowCount) {
            if (r > 0 && (cellsMade += cellsInRow) > kMaxTableCells) break;
            if ((m_repeatedBytes += row.size()) > kMaxRepeatedBytes) {
                left(QCoreApplication::translate("odt", "a table with more repeated rows than are kept"));
                break;
            }
            rowXmls.push_back(row);
        }
        // (The remaining counts of the merged cells go down with each repeat of the row: they were set for one.)
    }
    if (rowXmls.isEmpty()) return;
    while (widths.size() < gridCols) widths.push_back(1440);
    QByteArray grid = "<w:tblGrid>";
    qint64 sum = 0;
    for (qint64 w : std::as_const(widths)) {
        grid += "<w:gridCol w:w=\"" + num(w) + "\"/>";
        sum += w;
    }
    grid += "</w:tblGrid>";
    QByteArray tblPr;
    tblPr += "<w:tblW w:w=\"" + num(wOk ? tableWidth : sum) + "\" w:type=\"dxa\"/>";
    const QString align = tg("table:align");
    bool mOk = false;
    const qint64 ml = lengthToTwips(tg("fo:margin-left"), &mOk);
    if (align == QLatin1String("center")) tblPr += "<w:jc w:val=\"center\"/>";
    else if (align == QLatin1String("right")) tblPr += "<w:jc w:val=\"right\"/>";
    else if (mOk && ml != 0) tblPr += "<w:tblInd w:w=\"" + num(ml) + "\" w:type=\"dxa\"/>";
    tblPr += "<w:tblLayout w:type=\"fixed\"/>";
    tblPr += "<w:tblCellMar><w:left w:w=\"108\" w:type=\"dxa\"/><w:right w:w=\"108\" w:type=\"dxa\"/></w:tblCellMar>";
    Block b;
    b.xml = "<w:tbl><w:tblPr>" + tblPr + "</w:tblPr>" + grid;
    for (const QByteArray &r : std::as_const(rowXmls)) b.xml += r;
    b.xml += "</w:tbl>";
    out->push_back(b);
    if (tg("fo:break-after") == QLatin1String("page")) m_pendingPageBreak = true;
}

} // namespace jp::office::odt
