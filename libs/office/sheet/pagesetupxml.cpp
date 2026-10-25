#include "sheet/pagesetupxml.h"

#include "sheet/xmlscan.h"

#include <QXmlStreamAttributes>
#include <QXmlStreamReader>

#include <algorithm>
#include <cmath>

namespace jp::office::sheet {

namespace {

using xmlscan::Tag;

// The order the format has the children of a worksheet in: what an element put in is placed by.
const char *const kOrder[] = {"sheetPr", "dimension", "sheetViews", "sheetFormatPr", "cols", "sheetData", "sheetCalcPr", "sheetProtection", "protectedRanges", "scenarios", "autoFilter", "sortState",
                              "dataConsolidate", "customSheetViews", "mergeCells", "phoneticPr", "conditionalFormatting", "dataValidations", "hyperlinks", "printOptions", "pageMargins", "pageSetup",
                              "headerFooter", "rowBreaks", "colBreaks", "customProperties", "cellWatches", "ignoredErrors", "smartTags", "drawing", "legacyDrawing", "legacyDrawingHF", "drawingHF",
                              "picture", "oleObjects", "controls", "webPublishItems", "tableParts", "extLst"};
constexpr int kOrderCount = int(sizeof(kOrder) / sizeof(kOrder[0]));

int rankOf(QByteArrayView local)
{
    for (int i = 0; i < kOrderCount; ++i)
        if (local == kOrder[i]) return i;
    return kOrderCount;   // (an element it does not know is taken to come last)
}

enum Group { PrintOptions, PageMargins, PageSetupG, HeaderFooterG, RowBreaks, ColBreaks, GroupCount };
const char *const kGroupNames[GroupCount] = {"printOptions", "pageMargins", "pageSetup", "headerFooter", "rowBreaks", "colBreaks"};

bool isSpace(char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }

QByteArray num(double v) { return QByteArray::number(v, 'g', 15); }
QByteArray boolText(bool v) { return v ? "1" : "0"; }

// The attributes of a start tag's bytes, as the readers of the elements take them.
QXmlStreamAttributes attributesOf(QByteArrayView tag)
{
    QXmlStreamReader r;
    r.setNamespaceProcessing(false);
    r.addData(QByteArray(tag.data(), tag.size()));
    if (r.readNext() == QXmlStreamReader::StartDocument) r.readNext();
    return r.tokenType() == QXmlStreamReader::StartElement ? r.attributes() : QXmlStreamAttributes();
}

// Sets an attribute in a start tag's bytes: its value replaced where it is, or the attribute put after the last one.
void setAttr(QByteArray *tag, const QByteArray &name, const QByteArray &value)
{
    qsizetype vs = 0, ve = 0;
    if (xmlscan::attributeValue(*tag, name, &vs, &ve)) {
        tag->replace(vs, ve - vs, value);
        return;
    }
    qsizetype at = tag->size() - 1;   // the ">"
    if (at > 0 && (*tag)[at - 1] == '/') --at;
    while (at > 0 && isSpace((*tag)[at - 1])) --at;
    tag->insert(at, " " + name + "=\"" + value + "\"");
}

// An element found in a run of bytes: where it begins and ends (after its closing tag, or its own end if it closes itself) and where its start tag ends.
struct Found {
    qsizetype start = -1, end = -1, tagEnd = -1;
    bool selfClosing = false;
    QByteArray prefix;
    bool exists() const { return start >= 0; }
};

// The first element with the local name among the children of the element the bytes begin in (depth 0 is the bytes' own top level; with `anyDepth`, anywhere).
Found findElement(const QByteArray &xml, QByteArrayView local, bool anyDepth)
{
    Tag t;
    qsizetype pos = 0;
    int depth = 0;
    while (xmlscan::nextTag(xml, pos, &t)) {
        pos = t.end;
        if (t.closing) {
            --depth;
            continue;
        }
        if ((anyDepth || depth == 0) && t.local == local) {
            Found f;
            f.start = t.start;
            f.tagEnd = t.end;
            f.selfClosing = t.selfClosing;
            f.prefix = QByteArray(xmlscan::prefixOf(t.name));
            f.end = t.end;
            if (!t.selfClosing) {
                int d = 1;
                Tag c;
                qsizetype p = t.end;
                while (d > 0 && xmlscan::nextTag(xml, p, &c)) {
                    p = c.end;
                    if (c.closing) --d;
                    else if (!c.selfClosing) ++d;
                    if (d == 0) f.end = c.end;
                }
                if (d > 0) f.end = xml.size();
            }
            return f;
        }
        if (!t.selfClosing) ++depth;
    }
    return Found();
}

// The text between a start tag and its end tag.
QByteArray innerOf(const QByteArray &xml, const Found &f)
{
    if (f.selfClosing) return QByteArray();
    const qsizetype close = xml.lastIndexOf('<', f.end - 1);
    return close > f.tagEnd ? xml.mid(f.tagEnd, close - f.tagEnd) : QByteArray();
}

// The children of an element the bytes begin in: each child's local name, its start tag's bytes, and its text.
struct Child {
    QByteArray name, tag, text;
};
QVector<Child> childrenOf(const QByteArray &inner)
{
    QVector<Child> out;
    Tag t;
    qsizetype pos = 0;
    while (xmlscan::nextTag(inner, pos, &t)) {
        pos = t.end;
        if (t.closing) continue;
        Child c;
        c.name = QByteArray(t.local);
        c.tag = inner.mid(t.start, t.end - t.start);
        if (!t.selfClosing) {
            Tag close;
            if (xmlscan::nextTag(inner, t.end, &close) && close.closing) {
                c.text = inner.mid(t.end, close.start - t.end);
                pos = close.end;
            }
        }
        out.append(c);
    }
    return out;
}

void readGroup(const QByteArray &xml, Group g, const Found &f, PageSetup *out)
{
    const QByteArray tag = xml.mid(f.start, f.tagEnd - f.start);
    const QXmlStreamAttributes a = attributesOf(tag);
    switch (g) {
    case PrintOptions: readPrintOptionsAttributes(a, out); break;
    case PageMargins: readPageMarginsAttributes(a, out); break;
    case PageSetupG: readPageSetupAttributes(a, out); break;
    case HeaderFooterG:
        readHeaderFooterAttributes(a, out);
        for (const Child &c : childrenOf(innerOf(xml, f)))
            if (QString *text = headerFooterText(out, QString::fromLatin1(c.name))) *text = xmlscan::unescape(c.text).left(kMaxHeaderFooterChars);
        break;
    case RowBreaks:
    case ColBreaks:
        for (const Child &c : childrenOf(innerOf(xml, f)))
            if (c.name == "brk") readBreakAttributes(attributesOf(c.tag), g == RowBreaks, out);
        break;
    default: break;
    }
}

bool sameGroup(Group g, const PageSetup &a, const PageSetup &b)
{
    switch (g) {
    case PrintOptions: return a.gridLines == b.gridLines && a.headings == b.headings && a.centerHorizontally == b.centerHorizontally && a.centerVertically == b.centerVertically;
    case PageMargins: return a.margins == b.margins;
    case PageSetupG:
        return a.paperSize == b.paperSize && a.scale == b.scale && a.firstPageNumber == b.firstPageNumber && a.fitToWidth == b.fitToWidth && a.fitToHeight == b.fitToHeight && a.pageOrder == b.pageOrder
               && a.orientation == b.orientation && a.blackAndWhite == b.blackAndWhite && a.draft == b.draft && a.comments == b.comments && a.errors == b.errors && a.useFirstPageNumber == b.useFirstPageNumber;
    case HeaderFooterG: return a.headerFooter == b.headerFooter;
    case RowBreaks: return a.rowBreaks == b.rowBreaks;
    case ColBreaks: return a.colBreaks == b.colBreaks;
    default: return true;
    }
}

const char *orientationText(PageOrientation o) { return o == PageOrientation::Landscape ? "landscape" : o == PageOrientation::Portrait ? "portrait" : "default"; }
const char *commentsText(PrintComments c) { return c == PrintComments::AtEnd ? "atEnd" : c == PrintComments::AsDisplayed ? "asDisplayed" : "none"; }
const char *errorsText(PrintErrors e) { return e == PrintErrors::Blank ? "blank" : e == PrintErrors::Dash ? "dash" : e == PrintErrors::NA ? "NA" : "displayed"; }

// The attributes of pageSetup in the order a file writes them, for the ones whose value differs from `from` (all those that are not the default when `from` is the default).
void pageSetupAttributes(QByteArray *tag, const PageSetup &from, const PageSetup &now)
{
    if (now.paperSize != from.paperSize) setAttr(tag, "paperSize", QByteArray::number(now.paperSize));
    if (now.scale != from.scale) setAttr(tag, "scale", QByteArray::number(now.scale));
    if (now.firstPageNumber != from.firstPageNumber) setAttr(tag, "firstPageNumber", QByteArray::number(now.firstPageNumber));
    if (now.fitToWidth != from.fitToWidth) setAttr(tag, "fitToWidth", QByteArray::number(now.fitToWidth));
    if (now.fitToHeight != from.fitToHeight) setAttr(tag, "fitToHeight", QByteArray::number(now.fitToHeight));
    if (now.pageOrder != from.pageOrder) setAttr(tag, "pageOrder", now.pageOrder == PageOrder::OverThenDown ? "overThenDown" : "downThenOver");
    if (now.orientation != from.orientation) setAttr(tag, "orientation", orientationText(now.orientation));
    if (now.blackAndWhite != from.blackAndWhite) setAttr(tag, "blackAndWhite", boolText(now.blackAndWhite));
    if (now.draft != from.draft) setAttr(tag, "draft", boolText(now.draft));
    if (now.comments != from.comments) setAttr(tag, "cellComments", commentsText(now.comments));
    if (now.useFirstPageNumber != from.useFirstPageNumber) setAttr(tag, "useFirstPageNumber", boolText(now.useFirstPageNumber));
    if (now.errors != from.errors) setAttr(tag, "errors", errorsText(now.errors));
}

QByteArray breaksXml(const QVector<PageBreak> &list, bool rows, const QByteArray &p)
{
    if (list.isEmpty()) return QByteArray();
    int manual = 0;
    for (const PageBreak &b : list) manual += b.manual ? 1 : 0;
    const QByteArray name = rows ? "rowBreaks" : "colBreaks";
    QByteArray out = "<" + p + name + " count=\"" + QByteArray::number(list.size()) + "\" manualBreakCount=\"" + QByteArray::number(manual) + "\">";
    for (const PageBreak &b : list) {
        out += "<" + p + "brk id=\"" + QByteArray::number(b.before) + "\"";
        if (b.min != 0) out += " min=\"" + QByteArray::number(b.min) + "\"";
        out += " max=\"" + QByteArray::number(b.max != 0 ? b.max : (rows ? 16383u : 1048575u)) + "\"";
        if (b.manual) out += " man=\"1\"";
        out += "/>";
    }
    return out + "</" + p + name + ">";
}

QByteArray headerFooterChildren(const HeaderFooter &h, const QByteArray &p)
{
    QByteArray out;
    auto add = [&](const char *name, const QString &text) {
        if (!text.isEmpty()) out += "<" + p + name + ">" + xmlscan::escapeText(text) + "</" + p + name + ">";
    };
    add("oddHeader", h.oddHeader);
    add("oddFooter", h.oddFooter);
    add("evenHeader", h.evenHeader);
    add("evenFooter", h.evenFooter);
    add("firstHeader", h.firstHeader);
    add("firstFooter", h.firstFooter);
    return out;
}

// The element for a group as it is made new: empty if the setup says nothing for it.
QByteArray newElement(Group g, const PageSetup &now, const QByteArray &p)
{
    const PageSetup def;
    if (sameGroup(g, now, def)) return QByteArray();
    switch (g) {
    case PrintOptions: {
        QByteArray tag = "<" + p + "printOptions/>";
        if (now.centerHorizontally) setAttr(&tag, "horizontalCentered", "1");
        if (now.centerVertically) setAttr(&tag, "verticalCentered", "1");
        if (now.headings) setAttr(&tag, "headings", "1");
        if (now.gridLines) setAttr(&tag, "gridLines", "1");
        return tag;
    }
    case PageMargins:
        return "<" + p + "pageMargins left=\"" + num(now.margins.left) + "\" right=\"" + num(now.margins.right) + "\" top=\"" + num(now.margins.top) + "\" bottom=\"" + num(now.margins.bottom)
               + "\" header=\"" + num(now.margins.header) + "\" footer=\"" + num(now.margins.footer) + "\"/>";
    case PageSetupG: {
        QByteArray tag = "<" + p + "pageSetup/>";
        pageSetupAttributes(&tag, def, now);
        return tag;
    }
    case HeaderFooterG: {
        QByteArray tag = "<" + p + "headerFooter>";
        const HeaderFooter &h = now.headerFooter;
        if (h.differentOddEven) setAttr(&tag, "differentOddEven", "1");
        if (h.differentFirst) setAttr(&tag, "differentFirst", "1");
        if (!h.scaleWithDoc) setAttr(&tag, "scaleWithDoc", "0");
        if (!h.alignWithMargins) setAttr(&tag, "alignWithMargins", "0");
        const QByteArray kids = headerFooterChildren(h, p);
        if (kids.isEmpty()) {
            tag.chop(1);
            return tag + "/>";
        }
        return tag + kids + "</" + p + "headerFooter>";
    }
    case RowBreaks: return breaksXml(now.rowBreaks, true, p);
    case ColBreaks: return breaksXml(now.colBreaks, false, p);
    default: return QByteArray();
    }
}

// An existing element changed to say what `now` says: its attributes that differ set, its children made again. Empty bytes mean the element is taken away.
QByteArray changedElement(Group g, const QByteArray &xml, const Found &f, const PageSetup &old, const PageSetup &now, const QByteArray &p)
{
    QByteArray tag = xml.mid(f.start, f.tagEnd - f.start);
    switch (g) {
    case PrintOptions:
        if (now.centerHorizontally != old.centerHorizontally) setAttr(&tag, "horizontalCentered", boolText(now.centerHorizontally));
        if (now.centerVertically != old.centerVertically) setAttr(&tag, "verticalCentered", boolText(now.centerVertically));
        if (now.headings != old.headings) setAttr(&tag, "headings", boolText(now.headings));
        if (now.gridLines != old.gridLines) setAttr(&tag, "gridLines", boolText(now.gridLines));
        return tag + xml.mid(f.tagEnd, f.end - f.tagEnd);
    case PageMargins:
        if (now.margins.left != old.margins.left) setAttr(&tag, "left", num(now.margins.left));
        if (now.margins.right != old.margins.right) setAttr(&tag, "right", num(now.margins.right));
        if (now.margins.top != old.margins.top) setAttr(&tag, "top", num(now.margins.top));
        if (now.margins.bottom != old.margins.bottom) setAttr(&tag, "bottom", num(now.margins.bottom));
        if (now.margins.header != old.margins.header) setAttr(&tag, "header", num(now.margins.header));
        if (now.margins.footer != old.margins.footer) setAttr(&tag, "footer", num(now.margins.footer));
        return tag + xml.mid(f.tagEnd, f.end - f.tagEnd);
    case PageSetupG:
        pageSetupAttributes(&tag, old, now);
        return tag + xml.mid(f.tagEnd, f.end - f.tagEnd);
    case HeaderFooterG: {
        if (now.headerFooter == HeaderFooter()) return QByteArray();
        const HeaderFooter &h = now.headerFooter, &o = old.headerFooter;
        if (h.differentOddEven != o.differentOddEven) setAttr(&tag, "differentOddEven", boolText(h.differentOddEven));
        if (h.differentFirst != o.differentFirst) setAttr(&tag, "differentFirst", boolText(h.differentFirst));
        if (h.scaleWithDoc != o.scaleWithDoc) setAttr(&tag, "scaleWithDoc", boolText(h.scaleWithDoc));
        if (h.alignWithMargins != o.alignWithMargins) setAttr(&tag, "alignWithMargins", boolText(h.alignWithMargins));
        const bool sameTexts = h.oddHeader == o.oddHeader && h.oddFooter == o.oddFooter && h.evenHeader == o.evenHeader && h.evenFooter == o.evenFooter && h.firstHeader == o.firstHeader && h.firstFooter == o.firstFooter;
        if (sameTexts) return tag + xml.mid(f.tagEnd, f.end - f.tagEnd);
        if (f.selfClosing) {
            tag.chop(2);
            while (!tag.isEmpty() && isSpace(tag.back())) tag.chop(1);
            tag += ">";
        }
        const QByteArray kids = headerFooterChildren(h, p.isEmpty() ? f.prefix : p);
        if (kids.isEmpty()) {
            tag.chop(1);
            return tag + "/>";
        }
        return tag + kids + "</" + f.prefix + "headerFooter>";
    }
    case RowBreaks: return breaksXml(now.rowBreaks, true, f.prefix);
    case ColBreaks: return breaksXml(now.colBreaks, false, f.prefix);
    default: return xml.mid(f.start, f.end - f.start);
    }
}

} // namespace

PageSetup pageSetupOfXml(const QByteArray &xml)
{
    PageSetup out;
    for (int g = 0; g < GroupCount; ++g) {
        const Found f = findElement(xml, kGroupNames[g], true);
        if (f.exists()) readGroup(xml, Group(g), f, &out);
    }
    return out;
}

bool fitToPageOfXml(const QByteArray &xml)
{
    const Found f = findElement(xml, "pageSetUpPr", true);
    if (!f.exists()) return false;
    PageSetup p;
    readFitToPageAttributes(attributesOf(xml.mid(f.start, f.tagEnd - f.start)), &p);
    return p.fitToPage;
}

QByteArray pageSetupElementsXml(const PageSetup &setup, const QByteArray &prefix)
{
    QByteArray out;
    for (int g = 0; g < GroupCount; ++g) out += newElement(Group(g), setup, prefix);
    return out;
}

QByteArray fitToPageXml(const PageSetup &setup, const QByteArray &prefix)
{
    return setup.fitToPage ? "<" + prefix + "pageSetUpPr fitToPage=\"1\"/>" : QByteArray();
}

QByteArray patchPageSetupTail(const QByteArray &after, const PageSetup &now, const QByteArray &prefix)
{
    struct Edit {
        qsizetype start, end;
        QByteArray with;
        int rank;
    };
    std::vector<Edit> edits;
    PageSetup old;
    Found found[GroupCount];
    for (int g = 0; g < GroupCount; ++g) {
        found[g] = findElement(after, kGroupNames[g], false);
        if (found[g].exists()) readGroup(after, Group(g), found[g], &old);
    }
    // where the root ends, and the start of each element at the top level (for the places new elements go)
    qsizetype rootClose = after.size();
    std::vector<std::pair<qsizetype, int>> tops;   // (the start, the rank)
    {
        Tag t;
        qsizetype pos = 0;
        int depth = 0;
        while (xmlscan::nextTag(after, pos, &t)) {
            pos = t.end;
            if (t.closing) {
                if (--depth < 0) {
                    rootClose = t.start;
                    break;
                }
                continue;
            }
            if (depth == 0) tops.emplace_back(t.start, rankOf(t.local));
            if (!t.selfClosing) ++depth;
        }
    }
    for (int g = 0; g < GroupCount; ++g) {
        if (sameGroup(Group(g), old, now) && (found[g].exists() || sameGroup(Group(g), now, PageSetup()))) continue;
        const int rank = rankOf(kGroupNames[g]);
        if (found[g].exists()) {
            edits.push_back({found[g].start, found[g].end, changedElement(Group(g), after, found[g], old, now, prefix), rank});
        } else {
            const QByteArray made = newElement(Group(g), now, prefix);
            if (made.isEmpty()) continue;
            qsizetype at = rootClose;
            for (const auto &top : tops)
                if (top.second > rank) {
                    at = top.first;
                    break;
                }
            edits.push_back({at, at, made, rank});
        }
    }
    if (edits.empty()) return after;
    std::stable_sort(edits.begin(), edits.end(), [](const Edit &a, const Edit &b) { return a.start != b.start ? a.start < b.start : a.rank < b.rank; });
    QByteArray out;
    qsizetype copied = 0;
    for (const Edit &e : edits) {
        out += after.mid(copied, e.start - copied) + e.with;
        copied = e.end;
    }
    return out + after.mid(copied);
}

QByteArray patchFitToPageHead(const QByteArray &head, const PageSetup &now)
{
    const bool was = fitToPageOfXml(head);
    if (was == now.fitToPage) return head;
    const Found pr = findElement(head, "pageSetUpPr", true);
    if (pr.exists()) {
        QByteArray tag = head.mid(pr.start, pr.tagEnd - pr.start);
        setAttr(&tag, "fitToPage", boolText(now.fitToPage));
        return head.left(pr.start) + tag + head.mid(pr.tagEnd);
    }
    const Found sheetPr = findElement(head, "sheetPr", true);
    if (sheetPr.exists()) {
        const QByteArray p = sheetPr.prefix;
        if (sheetPr.selfClosing) {
            QByteArray tag = head.mid(sheetPr.start, sheetPr.tagEnd - sheetPr.start);
            tag.chop(2);
            while (!tag.isEmpty() && isSpace(tag.back())) tag.chop(1);
            return head.left(sheetPr.start) + tag + ">" + fitToPageXml(now, p) + "</" + p + "sheetPr>" + head.mid(sheetPr.end);
        }
        const qsizetype close = head.lastIndexOf('<', sheetPr.end - 1);
        return head.left(close) + fitToPageXml(now, p) + head.mid(close);
    }
    // no sheetPr: it is the first child of the root
    Tag t;
    qsizetype pos = 0;
    while (xmlscan::nextTag(head, pos, &t)) {
        pos = t.end;
        if (t.closing) continue;
        const QByteArray p(xmlscan::prefixOf(t.name));
        return head.left(t.end) + "<" + p + "sheetPr>" + fitToPageXml(now, p) + "</" + p + "sheetPr>" + head.mid(t.end);
    }
    return head;
}

bool patchPrintNames(const QByteArray &xml, const std::vector<PrintNameChange> &changes, QByteArray *out)
{
    *out = xml;
    if (changes.empty()) return true;
    Found names = findElement(xml, "definedNames", true);
    struct Existing {
        Found f;
        QByteArray name;
        int sheet;
        QString text;
        bool removed = false;
    };
    std::vector<Existing> existing;
    QByteArray prefix;
    if (names.exists()) {
        prefix = names.prefix;
        Tag t;
        qsizetype pos = names.tagEnd;
        while (!names.selfClosing && pos < names.end && xmlscan::nextTag(xml, pos, &t)) {
            pos = t.end;
            if (t.closing || t.local != "definedName") continue;
            const QByteArray open = xml.mid(t.start, t.end - t.start);
            Existing e;
            e.f.start = t.start;
            e.f.tagEnd = t.end;
            e.f.selfClosing = t.selfClosing;
            e.f.end = t.end;
            e.f.prefix = QByteArray(xmlscan::prefixOf(t.name));
            e.name = QByteArray(xmlscan::attribute(QByteArrayView(open), "name"));
            bool has = false;
            const QByteArray scope(xmlscan::attribute(QByteArrayView(open), "localSheetId", &has));
            e.sheet = has ? scope.toInt() : -1;
            if (!t.selfClosing) {
                Tag close;
                if (xmlscan::nextTag(xml, t.end, &close) && close.closing) {
                    e.text = xmlscan::unescape(QByteArrayView(xml).sliced(t.end, close.start - t.end));
                    e.f.end = close.end;
                    pos = close.end;
                }
            }
            existing.push_back(e);
        }
    } else {
        prefix = QByteArray(xmlscan::prefixOf([&] {
            Tag t;
            return xmlscan::nextTag(xml, 0, &t) && !t.closing ? t.name : QByteArrayView();
        }()));
    }
    struct Edit {
        qsizetype start, end;
        QByteArray with;
    };
    std::vector<Edit> edits;
    QByteArray additions;
    auto handle = [&](const PrintNameChange &c, const QByteArray &builtin, bool areaKind) {
        Existing *e = nullptr;
        for (Existing &x : existing)
            if (x.sheet == c.sheet && x.name.compare(builtin, Qt::CaseInsensitive) == 0 && !x.removed) {
                e = &x;
                break;
            }
        PageSetup old;
        if (e) {
            if (areaKind) readPrintAreaText(e->text, c.sheetName, &old);
            else readPrintTitlesText(e->text, c.sheetName, &old);
        }
        const bool same = areaKind ? old.printArea == c.setup.printArea : old.titles == c.setup.titles;
        if (same) return;
        const QString text = areaKind ? printAreaText(c.setup, c.sheetName) : printTitlesText(c.setup, c.sheetName);
        if (e) {
            if (text.isEmpty()) {
                edits.push_back({e->f.start, e->f.end, QByteArray()});
                e->removed = true;
            } else if (e->f.selfClosing) {
                QByteArray open = xml.mid(e->f.start, e->f.tagEnd - e->f.start);
                open.chop(2);
                while (!open.isEmpty() && isSpace(open.back())) open.chop(1);
                edits.push_back({e->f.start, e->f.end, open + ">" + xmlscan::escapeText(text) + "</" + e->f.prefix + "definedName>"});
            } else {
                edits.push_back({e->f.tagEnd, e->f.end, xmlscan::escapeText(text) + "</" + e->f.prefix + "definedName>"});
            }
        } else if (!text.isEmpty()) {
            additions += "<" + prefix + "definedName name=\"" + builtin + "\" localSheetId=\"" + QByteArray::number(c.sheet) + "\">" + xmlscan::escapeText(text) + "</" + prefix + "definedName>";
        }
    };
    for (const PrintNameChange &c : changes) {
        handle(c, "_xlnm.Print_Area", true);
        handle(c, "_xlnm.Print_Titles", false);
    }
    if (edits.empty() && additions.isEmpty()) return true;
    if (names.exists() && !names.selfClosing) {
        // names added go at the end of the list; the list that loses its last name goes with its element
        int left = additions.isEmpty() ? 0 : 1;
        for (const Existing &e : existing) left += e.removed ? 0 : 1;
        if (left == 0) {
            *out = xml.left(names.start) + xml.mid(names.end);
            return true;
        }
        if (!additions.isEmpty()) {
            // before the list's closing tag, wherever it starts ("</definedNames >" is allowed too)
            const int close = int(xml.lastIndexOf("</", names.end - 1));
            if (close < names.start) return false;
            edits.push_back({close, close, additions});
        }
    } else if (names.exists()) {
        *out = xml.left(names.start) + "<" + prefix + "definedNames>" + additions + "</" + prefix + "definedNames>" + xml.mid(names.end);
        return true;
    } else {
        // after the last of the elements the format puts before the names
        qsizetype at = -1;
        Tag t;
        qsizetype pos = 0;
        while (xmlscan::nextTag(xml, pos, &t)) {
            pos = t.end;
            const bool before = t.local == "sheets" || t.local == "functionGroups" || t.local == "externalReferences";
            if (before && (t.closing || t.selfClosing)) at = t.end;
        }
        if (at < 0) return false;
        *out = xml.left(at) + "<" + prefix + "definedNames>" + additions + "</" + prefix + "definedNames>" + xml.mid(at);
        return true;
    }
    std::stable_sort(edits.begin(), edits.end(), [](const Edit &a, const Edit &b) { return a.start < b.start; });
    QByteArray result;
    qsizetype copied = 0;
    for (const Edit &e : edits) {
        result += xml.mid(copied, e.start - copied) + e.with;
        copied = e.end;
    }
    *out = result + xml.mid(copied);
    return true;
}

} // namespace jp::office::sheet
