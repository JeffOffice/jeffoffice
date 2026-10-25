// Saving a workbook as the old file with only what changed put in anew: see xlsxsave.h.

#include "sheet/xlsxsave.h"

#include "io/zip.h"
#include "io/zipindex.h"
#include "sheet/structsave.h"
#include "sheet/xlsxbase.h"
#include "sheet/xmlscan.h"

#include <QCoreApplication>
#include <QBuffer>
#include <QDir>
#include <QFileInfo>
#include <QHash>
#include <QSaveFile>
#include <QRegularExpression>
#include <QSet>

#include <algorithm>

namespace jp::office::sheet {

namespace {

using xmlscan::Tag;


bool isSpace(char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }

// A name the way two spellings of one entry compare equal: slashes, no leading slash, lower case.
QString plain(const QString &name)
{
    QString n = name;
    n.replace(QLatin1Char('\\'), QLatin1Char('/'));
    while (n.startsWith(QLatin1Char('/'))) n.remove(0, 1);
    return n.toLower();
}

// Sets an attribute in a tag's bytes (a start tag, `<xf ...>` or `<xf .../>`): its value replaced, or the attribute put in before the end of the tag.
void setAttribute(QByteArray *tag, const char *name, const QByteArray &value)
{
    qsizetype vs = 0, ve = 0;
    if (xmlscan::attributeValue(*tag, name, &vs, &ve)) {
        tag->replace(vs, ve - vs, value);
        return;
    }
    qsizetype at = tag->size() - 1;   // the ">"
    if (at > 0 && (*tag)[at - 1] == '/') --at;
    tag->insert(at, QByteArray(" ") + name + "=\"" + value + "\"");
}

// The first start tag of an element with the local name (anywhere); false if there is none.
bool findStart(const QByteArray &xml, QByteArrayView local, Tag *tag, qsizetype from = 0)
{
    qsizetype pos = from;
    while (xmlscan::nextTag(xml, pos, tag)) {
        pos = tag->end;
        if (!tag->closing && tag->local == local) return true;
    }
    return false;
}

// The close tag that matches a start tag of the same local name (no element of that name is nested in the ones this is used for); false if there is none.
bool findEnd(const QByteArray &xml, const Tag &start, Tag *close)
{
    qsizetype pos = start.end;
    while (xmlscan::nextTag(xml, pos, close)) {
        pos = close->end;
        if (close->closing && close->local == start.local) return true;
    }
    return false;
}

QByteArray prefixOfRoot(const QByteArray &xml)
{
    Tag t;
    return xmlscan::nextTag(xml, 0, &t) && !t.closing ? QByteArray(xmlscan::prefixOf(t.name)) : QByteArray();
}

void addToCount(QByteArray *tag, const char *name, qint64 more)
{
    qsizetype vs = 0, ve = 0;
    if (!xmlscan::attributeValue(*tag, name, &vs, &ve)) return;
    bool ok = false;
    const qint64 now = tag->mid(vs, ve - vs).toLongLong(&ok);
    if (ok) tag->replace(vs, ve - vs, QByteArray::number(now + more));
}

} // namespace

// ---- the shared strings

bool patchSharedStrings(const QByteArray &xml, const QStringList &added, QByteArray *out, QString *error)
{
    Tag root;
    if (!findStart(xml, "sst", &root)) {
        if (error) *error = QCoreApplication::translate("jp::office::sheet::Save", "The shared strings part is not in the form this program changes.");
        return false;
    }
    const QByteArray p(xmlscan::prefixOf(root.name));
    QByteArray items;
    for (const QString &s : added) {
        const bool preserve = !s.isEmpty() && (s.front().isSpace() || s.back().isSpace() || s.contains(QLatin1Char('\n')) || s.contains(QLatin1Char('\t')) || s.contains(QLatin1String("  ")));
        items += "<" + p + "si><" + p + "t" + (preserve ? " xml:space=\"preserve\"" : "") + ">" + xmlscan::escapeText(s) + "</" + p + "t></" + p + "si>";
    }
    QByteArray rootTag = xml.mid(root.start, root.end - root.start);
    addToCount(&rootTag, "uniqueCount", added.size());
    addToCount(&rootTag, "count", added.size());
    if (root.selfClosing) {
        rootTag.chop(2);
        while (!rootTag.isEmpty() && isSpace(rootTag.back())) rootTag.chop(1);
        *out = xml.left(root.start) + rootTag + ">" + items + "</" + p + "sst>" + xml.mid(root.end);
        return true;
    }
    const QByteArray closeTag = "</" + p + "sst>";
    const qsizetype close = xml.lastIndexOf(closeTag);
    if (close < root.end) {
        if (error) *error = QCoreApplication::translate("jp::office::sheet::Save", "The shared strings part is not in the form this program changes.");
        return false;
    }
    *out = xml.left(root.start) + rootTag + xml.mid(root.end, close - root.end) + items + xml.mid(close);
    return true;
}

// ---- the styles

namespace {

// The XML with a prefix put in front of every element name (a styles part whose elements have one).
QByteArray prefixed(const QByteArray &xml, const QByteArray &prefix)
{
    if (prefix.isEmpty()) return xml;
    QByteArray out;
    out.reserve(xml.size() + xml.size() / 8);
    for (qsizetype i = 0; i < xml.size(); ++i) {
        out += xml[i];
        if (xml[i] == '<' && i + 1 < xml.size()) {
            if (xml[i + 1] == '/') {
                out += '/';
                ++i;
            }
            out += prefix;   // (a prefix as xmlscan::prefixOf gives it has its colon)
        }
    }
    return out;
}

// Removes an attribute from a start tag.
void removeAttr(QByteArray *tag, const char *name)
{
    qsizetype vs = 0, ve = 0;
    if (!xmlscan::attributeValue(*tag, name, &vs, &ve)) return;
    qsizetype n = vs - 2;   // before the quote, the "="
    while (n >= 0 && isSpace((*tag)[n])) --n;
    if (n >= 0 && (*tag)[n] == '=') --n;
    while (n >= 0 && isSpace((*tag)[n])) --n;
    qsizetype nameStart = n;
    while (nameStart > 0 && !isSpace((*tag)[nameStart - 1])) --nameStart;
    qsizetype a = nameStart;
    while (a > 0 && isSpace((*tag)[a - 1])) --a;
    tag->remove(a, ve + 1 - a);
}

// An attribute set to a value, or taken away when the value is empty.
void setOrRemove(QByteArray *tag, const char *name, const QByteArray &value)
{
    if (value.isEmpty()) removeAttr(tag, name);
    else setAttribute(tag, name, value);
}

const char *horizontalText(HAlign a)
{
    switch (a) {
    case HAlign::Left: return "left";
    case HAlign::Center: return "center";
    case HAlign::Right: return "right";
    case HAlign::Fill: return "fill";
    case HAlign::Justify: return "justify";
    case HAlign::CenterContinuous: return "centerContinuous";
    case HAlign::Distributed: return "distributed";
    default: return "";
    }
}

const char *verticalText(VAlign a)
{
    switch (a) {
    case VAlign::Center: return "center";
    case VAlign::Top: return "top";
    case VAlign::Justify: return "justify";
    case VAlign::Distributed: return "distributed";
    default: return "";
    }
}

// A raw xf element made to be `f`: its attributes (the places of the number format, font, fill, and border, the style it follows) and its alignment and protection children, with
// anything else the file said of it (an extension, an attribute this program does not know) kept.
bool xfMadeInto(const QByteArray &raw, const CellFormat &f, const QByteArray &prefix, QByteArray *out)
{
    Tag open;
    if (!xmlscan::nextTag(raw, 0, &open) || open.closing) return false;
    QByteArray tag = raw.left(open.end);
    const bool selfClosing = open.selfClosing;
    if (selfClosing) {
        tag.chop(2);
        while (!tag.isEmpty() && isSpace(tag.back())) tag.chop(1);
        tag += ">";
    }
    setAttribute(&tag, "numFmtId", QByteArray::number(f.numFmtId));
    setAttribute(&tag, "fontId", QByteArray::number(f.font));
    setAttribute(&tag, "fillId", QByteArray::number(f.fill));
    setAttribute(&tag, "borderId", QByteArray::number(f.border));
    setAttribute(&tag, "xfId", QByteArray::number(f.xfId));
    for (const char *apply : {"applyNumberFormat", "applyFont", "applyFill", "applyBorder", "applyAlignment", "applyProtection"}) setAttribute(&tag, apply, "1");
    setOrRemove(&tag, "quotePrefix", f.quotePrefix ? "1" : "");
    // the children: those that are not the alignment and protection are kept
    QByteArray alignment, protection, others;
    if (!selfClosing) {
        const qsizetype innerStart = open.end;
        qsizetype innerEnd = raw.lastIndexOf("</");
        if (innerEnd < innerStart) innerEnd = raw.size();
        const QByteArray inner = raw.mid(innerStart, innerEnd - innerStart);
        qsizetype pos = 0;
        Tag t;
        int depth = 0;
        qsizetype keepFrom = 0;
        while (xmlscan::nextTag(inner, pos, &t)) {
            pos = t.end;
            if (t.closing) {
                --depth;
                continue;
            }
            if (depth == 0 && (t.local == "alignment" || t.local == "protection")) {
                qsizetype end = t.end;
                if (!t.selfClosing) {
                    Tag close;
                    if (!findEnd(inner, t, &close)) return false;
                    end = close.end;
                }
                others += inner.mid(keepFrom, t.start - keepFrom);
                (t.local == "alignment" ? alignment : protection) = inner.mid(t.start, end - t.start);
                keepFrom = end;
                pos = end;
                continue;
            }
            if (!t.selfClosing) ++depth;
        }
        others += inner.mid(keepFrom);
    }
    // the alignment: the attributes this program knows are set to the format's, the others stay
    QByteArray al = alignment.isEmpty() ? QByteArray("<alignment/>") : alignment;
    Tag alTag;
    if (!xmlscan::nextTag(al, 0, &alTag)) return false;
    QByteArray alOpen = al.left(alTag.end);
    if (alTag.selfClosing) {
        alOpen.chop(2);
        while (!alOpen.isEmpty() && isSpace(alOpen.back())) alOpen.chop(1);
        alOpen += "/>";
    }
    setOrRemove(&alOpen, "horizontal", horizontalText(f.horizontal));
    setOrRemove(&alOpen, "vertical", verticalText(f.vertical));
    setOrRemove(&alOpen, "textRotation", f.rotation ? QByteArray::number(f.rotation) : QByteArray());
    setOrRemove(&alOpen, "wrapText", f.wrap ? "1" : "");
    setOrRemove(&alOpen, "indent", f.indent ? QByteArray::number(f.indent) : QByteArray());
    setOrRemove(&alOpen, "shrinkToFit", f.shrink ? "1" : "");
    setOrRemove(&alOpen, "readingOrder", f.readingOrder ? QByteArray::number(f.readingOrder) : QByteArray());
    QByteArray alignmentNow = alTag.selfClosing ? alOpen : alOpen + al.mid(alTag.end);
    // (an alignment that says nothing is left out)
    Tag check;
    xmlscan::nextTag(alignmentNow, 0, &check);
    {
        QByteArray bare = alOpen;
        bare.replace("<alignment", "").replace("/>", "").replace(">", "");
        if (bare.trimmed().isEmpty() && alTag.selfClosing) alignmentNow.clear();
    }
    QByteArray protectionNow;
    if (!f.locked || f.formulaHidden || !protection.isEmpty()) {
        QByteArray pr = protection.isEmpty() ? QByteArray("<protection/>") : protection;
        Tag prTag;
        if (!xmlscan::nextTag(pr, 0, &prTag)) return false;
        QByteArray prOpen = pr.left(prTag.end);
        if (prTag.selfClosing) {
            prOpen.chop(2);
            while (!prOpen.isEmpty() && isSpace(prOpen.back())) prOpen.chop(1);
            prOpen += "/>";
        }
        setOrRemove(&prOpen, "locked", f.locked ? "" : "0");
        setOrRemove(&prOpen, "hidden", f.formulaHidden ? "1" : "");
        protectionNow = prTag.selfClosing ? prOpen : prOpen + pr.mid(prTag.end);
    }
    const QByteArray inner = alignmentNow + protectionNow + others;
    if (inner.isEmpty()) {
        tag.chop(1);
        *out = tag + "/>";
    } else {
        *out = tag + inner + QByteArray("</") + prefix + "xf>";
    }
    return true;
}

// The container (an element that holds a list) with `n` items put at its end and its count raised; false if it is not in the part.
bool appendToList(QByteArray *xml, const char *container, const QByteArray &items, qint64 n)
{
    if (n <= 0) return true;
    Tag open;
    if (!findStart(*xml, container, &open)) return false;
    QByteArray openTag = xml->mid(open.start, open.end - open.start);
    addToCount(&openTag, "count", n);
    if (open.selfClosing) {
        const QByteArray p(xmlscan::prefixOf(open.name));
        openTag.chop(2);
        while (!openTag.isEmpty() && isSpace(openTag.back())) openTag.chop(1);
        *xml = xml->left(open.start) + openTag + ">" + items + "</" + p + container + ">" + xml->mid(open.end);
        return true;
    }
    Tag close;
    if (!findEnd(*xml, open, &close)) return false;
    *xml = xml->left(open.start) + openTag + xml->mid(open.end, close.start - open.end) + items + xml->mid(close.start);
    return true;
}

} // namespace

bool patchStyles(const QByteArray &xml, const SaveChanges &changes, QByteArray *out, QString *error)
{
    auto bad = [&] {
        if (error) *error = QCoreApplication::translate("jp::office::sheet::Save", "The styles part is not in the form this program changes.");
        return false;
    };
    Tag root;
    if (!findStart(xml, "styleSheet", &root)) return bad();
    const QByteArray p(xmlscan::prefixOf(root.name));
    QByteArray result = xml;
    // the cell formats first (they are later in the part, so the places of the number formats' change do not move them)
    if (!changes.newCellFormats.isEmpty()) {
        Tag open;
        if (!findStart(result, "cellXfs", &open, root.end)) return bad();
        if (open.selfClosing) return bad();
        Tag close;
        if (!findEnd(result, open, &close)) return bad();
        // the xf elements the new ones are copies of
        QSet<quint32> needed;
        for (const SaveChanges::CellFormatAdded &f : changes.newCellFormats)
            if (qint64(f.base) < changes.cellFormatsBefore) needed.insert(f.base);
        QHash<quint32, QByteArray> raw;
        qsizetype pos = open.end;
        quint32 index = 0;
        Tag t;
        int depth = 0;
        while (xmlscan::nextTag(result, pos, &t) && t.start < close.start) {
            pos = t.end;
            if (t.closing) {
                --depth;
                continue;
            }
            if (depth == 0 && t.local == "xf") {
                qsizetype end = t.end;
                if (!t.selfClosing) {
                    Tag xfClose;
                    if (!findEnd(result, t, &xfClose)) return bad();
                    end = xfClose.end;
                }
                if (needed.contains(index)) raw.insert(index, result.mid(t.start, end - t.start));
                ++index;
                pos = end;
                continue;
            }
            if (!t.selfClosing) ++depth;
        }
        if (qint64(index) != changes.cellFormatsBefore) return bad();
        QByteArray added;
        QVector<QByteArray> made;
        for (const SaveChanges::CellFormatAdded &f : changes.newCellFormats) {
            QByteArray base;
            if (qint64(f.base) < changes.cellFormatsBefore) base = raw.value(f.base);
            else if (qint64(f.base) - changes.cellFormatsBefore < made.size()) base = made[int(qint64(f.base) - changes.cellFormatsBefore)];
            if (base.isEmpty()) return bad();
            QByteArray one;
            if (f.full) {
                if (!xfMadeInto(base, f.format, p, &one)) return bad();
            } else {
                Tag baseTag;
                if (!xmlscan::nextTag(base, 0, &baseTag)) return bad();
                QByteArray tag = base.left(baseTag.end);
                setAttribute(&tag, "numFmtId", QByteArray::number(f.numFmtId));
                setAttribute(&tag, "applyNumberFormat", "1");
                one = tag + base.mid(baseTag.end);
            }
            made.append(one);
            added += one;
        }
        QByteArray openTag = result.mid(open.start, open.end - open.start);
        addToCount(&openTag, "count", changes.newCellFormats.size());
        result = result.left(open.start) + openTag + result.mid(open.end, close.start - open.end) + added + result.mid(close.start);
    }
    // the named styles: their formats among the style formats, and their names
    if (!changes.newStyles.isEmpty()) {
        QByteArray xfs, names;
        qint64 at = changes.styleXfsBefore;
        for (const SaveChanges::StyleAdded &s : changes.newStyles) {
            xfs += prefixed(xfXml(s.format, 0, false), p);
            names += prefixed("<cellStyle name=\"" + xmlscan::escapeAttribute(s.name.name) + "\" xfId=\"" + QByteArray::number(at++) + "\"" + (s.name.builtin >= 0 ? " builtinId=\"" + QByteArray::number(s.name.builtin) + "\"" : QByteArray()) + "/>", p);
        }
        if (!appendToList(&result, "cellStyleXfs", xfs, changes.newStyles.size())) return bad();
        if (!appendToList(&result, "cellStyles", names, changes.newStyles.size())) return bad();
    }
    // the borders, fills, and fonts (before the cell formats in the part; each is found again, so that the places do not matter)
    if (!changes.newBorders.isEmpty()) {
        QByteArray items;
        for (const BorderSpec &b : changes.newBorders) items += prefixed(borderXml(b), p);
        if (!appendToList(&result, "borders", items, changes.newBorders.size())) return bad();
    }
    if (!changes.newFills.isEmpty()) {
        QByteArray items;
        for (const FillSpec &f : changes.newFills) items += prefixed(fillXml(f), p);
        if (!appendToList(&result, "fills", items, changes.newFills.size())) return bad();
    }
    if (!changes.newFonts.isEmpty()) {
        QByteArray items;
        for (const FontSpec &f : changes.newFonts) items += prefixed(fontXml(f), p);
        if (!appendToList(&result, "fonts", items, changes.newFonts.size())) return bad();
    }
    if (!changes.newDxfs.isEmpty()) {
        QByteArray items;
        for (const DifferentialFormat &d : changes.newDxfs) {
            QByteArray one = "<dxf>";
            if (d.fontColor.isSet()) one += "<font>" + colorXml("color", d.fontColor) + "</font>";
            if (d.hasFill) one += "<fill><patternFill patternType=\"solid\">" + colorXml("fgColor", d.fill.bg.isSet() ? d.fill.bg : d.fill.fg) + colorXml("bgColor", d.fill.bg.isSet() ? d.fill.bg : d.fill.fg) + "</patternFill></fill>";
            one += "</dxf>";
            items += prefixed(one, p);
        }
        Tag dxfsTag;
        if (findStart(result, "dxfs", &dxfsTag, 0)) {
            if (!appendToList(&result, "dxfs", items, changes.newDxfs.size())) return bad();
        } else {
            // (a part with no list of them: made, in the place the format has it, before the table styles and what follows)
            Tag at;
            qsizetype insertAt = -1;
            for (const char *after : {"tableStyles", "colors", "extLst"})
                if (findStart(result, after, &at, 0)) {
                    insertAt = insertAt < 0 ? at.start : std::min<qsizetype>(insertAt, at.start);
                }
            if (insertAt < 0) {
                Tag close;
                if (!findEnd(result, root, &close)) return bad();
                insertAt = close.start;
            }
            result = result.left(insertAt) + "<" + p + "dxfs count=\"" + QByteArray::number(changes.newDxfs.size()) + "\">" + items + "</" + p + "dxfs>" + result.mid(insertAt);
        }
    }
    if (!changes.newNumberFormats.isEmpty()) {
        QByteArray added;
        for (const SaveChanges::NumberFormat &f : changes.newNumberFormats)
            added += "<" + p + "numFmt numFmtId=\"" + QByteArray::number(f.id) + "\" formatCode=\"" + xmlscan::escapeAttribute(f.code) + "\"/>";
        Tag open;
        if (findStart(result, "numFmts", &open, root.end)) {
            if (!appendToList(&result, "numFmts", added, changes.newNumberFormats.size())) return bad();
        } else {
            const QByteArray element = "<" + p + "numFmts count=\"" + QByteArray::number(changes.newNumberFormats.size()) + "\">" + added + "</" + p + "numFmts>";
            Tag again;
            if (!findStart(result, "styleSheet", &again)) return bad();
            result.insert(again.end, element);
        }
    }
    *out = result;
    return true;
}

// ---- the package's own parts

bool removeContentTypeOverride(const QByteArray &xml, const QString &partName, QByteArray *out)
{
    Tag t;
    qsizetype pos = 0;
    const QString want = plain(partName);
    while (xmlscan::nextTag(xml, pos, &t)) {
        pos = t.end;
        if (t.closing || t.local != "Override") continue;
        bool found = false;
        const QByteArrayView name = xmlscan::attribute(QByteArrayView(xml).sliced(t.start, t.end - t.start), "PartName", &found);
        if (!found || plain(xmlscan::unescape(name)) != want) continue;
        qsizetype from = t.start;
        while (from > 0 && isSpace(xml[from - 1])) --from;
        *out = xml.left(from) + xml.mid(t.end);
        return true;
    }
    return false;
}

bool removeRelationshipsById(const QByteArray &xml, const QStringList &ids, QByteArray *out)
{
    Tag t;
    qsizetype pos = 0, copied = 0;
    QByteArray result;
    bool any = false;
    while (xmlscan::nextTag(xml, pos, &t)) {
        pos = t.end;
        if (t.closing || t.local != "Relationship") continue;
        bool found = false;
        const QByteArrayView id = xmlscan::attribute(QByteArrayView(xml).sliced(t.start, t.end - t.start), "Id", &found);
        if (!found || !ids.contains(xmlscan::unescape(id))) continue;
        qsizetype end = t.end;
        if (!t.selfClosing) {
            Tag close;
            if (!xmlscan::nextTag(xml, t.end, &close) || !close.closing || close.local != "Relationship") continue;
            end = close.end;
        }
        qsizetype from = t.start;
        while (from > copied && isSpace(xml[from - 1])) --from;
        result += xml.mid(copied, from - copied);
        copied = end;
        pos = end;
        any = true;
    }
    if (!any) return false;
    *out = result + xml.mid(copied);
    return true;
}

bool removeRelationshipOfType(const QByteArray &xml, const QString &typeSuffix, QByteArray *out)
{
    Tag t;
    qsizetype pos = 0;
    while (xmlscan::nextTag(xml, pos, &t)) {
        pos = t.end;
        if (t.closing || t.local != "Relationship") continue;
        bool found = false;
        const QByteArrayView type = xmlscan::attribute(QByteArrayView(xml).sliced(t.start, t.end - t.start), "Type", &found);
        if (!found || !xmlscan::unescape(type).endsWith(typeSuffix)) continue;
        qsizetype from = t.start;
        while (from > 0 && isSpace(xml[from - 1])) --from;
        *out = xml.left(from) + xml.mid(t.end);
        return true;
    }
    return false;
}

bool setFullCalcOnLoad(const QByteArray &xml, QByteArray *out)
{
    Tag calc;
    if (findStart(xml, "calcPr", &calc)) {
        QByteArray tag = xml.mid(calc.start, calc.end - calc.start);
        setAttribute(&tag, "fullCalcOnLoad", "1");
        *out = xml.left(calc.start) + tag + xml.mid(calc.end);
        return true;
    }
    // (the element goes after the last of the ones the format puts before it)
    const QByteArray p = prefixOfRoot(xml);
    qsizetype at = -1;
    Tag t;
    qsizetype pos = 0;
    while (xmlscan::nextTag(xml, pos, &t)) {
        pos = t.end;
        const bool before = t.local == "sheets" || t.local == "functionGroups" || t.local == "externalReferences" || t.local == "definedNames";
        if (before && (t.closing || t.selfClosing)) at = t.end;
    }
    if (at < 0) return false;
    *out = xml.left(at) + "<" + p + "calcPr fullCalcOnLoad=\"1\"/>" + xml.mid(at);
    return true;
}

bool patchFilterNames(const QByteArray &xml, const std::vector<FilterName> &names, QByteArray *out, QString *error)
{
    if (names.empty()) {
        *out = xml;
        return true;
    }
    auto bad = [&] {
        if (error) *error = QCoreApplication::translate("jp::office::sheet::Save", "The workbook part is not in the form this program changes.");
        return false;
    };
    const QByteArray p = prefixOfRoot(xml);
    QByteArray result;
    std::vector<bool> done(names.size(), false);
    // the names the file has: those for these sheets are made again (or go)
    {
        qsizetype copied = 0, pos = 0;
        Tag t;
        while (xmlscan::nextTag(xml, pos, &t)) {
            pos = t.end;
            if (t.closing || t.local != "definedName") continue;
            const QByteArray tag = xml.mid(t.start, t.end - t.start);
            qsizetype vs = 0, ve = 0;
            if (!xmlscan::attributeValue(tag, "name", &vs, &ve) || tag.mid(vs, ve - vs) != "_xlnm._FilterDatabase") continue;
            if (!xmlscan::attributeValue(tag, "localSheetId", &vs, &ve)) continue;
            const int id = tag.mid(vs, ve - vs).toInt();
            size_t which = names.size();
            for (size_t i = 0; i < names.size(); ++i)
                if (names[i].localSheetId == id) which = i;
            if (which == names.size()) continue;
            qsizetype end = t.end;
            if (!t.selfClosing) {
                Tag close;
                if (!findEnd(xml, t, &close)) return bad();
                end = close.end;
            }
            result += xml.mid(copied, t.start - copied);
            if (!done[which] && !names[which].range.isEmpty()) {
                result += "<" + p + "definedName name=\"_xlnm._FilterDatabase\" localSheetId=\"" + QByteArray::number(id) + "\" hidden=\"1\">" + xmlscan::escapeAttribute(names[which].range) + "</" + p + "definedName>";
            }
            done[which] = true;
            copied = end;
            pos = end;
        }
        result += xml.mid(copied);
    }
    // the ones it did not have
    QByteArray added;
    for (size_t i = 0; i < names.size(); ++i)
        if (!done[i] && !names[i].range.isEmpty())
            added += "<" + p + "definedName name=\"_xlnm._FilterDatabase\" localSheetId=\"" + QByteArray::number(names[i].localSheetId) + "\" hidden=\"1\">" + xmlscan::escapeAttribute(names[i].range) + "</" + p + "definedName>";
    if (added.isEmpty()) {
        *out = result;
        return true;
    }
    Tag open, close;
    if (findStart(result, "definedNames", &open)) {
        if (open.selfClosing) {
            QByteArray tag = result.mid(open.start, open.end - open.start);
            tag.chop(2);
            while (!tag.isEmpty() && isSpace(tag.back())) tag.chop(1);
            *out = result.left(open.start) + tag + ">" + added + "</" + p + "definedNames>" + result.mid(open.end);
            return true;
        }
        if (!findEnd(result, open, &close)) return bad();
        *out = result.left(close.start) + added + result.mid(close.start);
        return true;
    }
    // (the list is made, after the ones the format puts before it)
    qsizetype at = -1;
    Tag t;
    qsizetype pos = 0;
    while (xmlscan::nextTag(result, pos, &t)) {
        pos = t.end;
        const bool before = t.local == "sheets" || t.local == "functionGroups" || t.local == "externalReferences";
        if (before && (t.closing || t.selfClosing)) at = t.end;
    }
    if (at < 0) return bad();
    *out = result.left(at) + "<" + p + "definedNames>" + added + "</" + p + "definedNames>" + result.mid(at);
    return true;
}

// ---- the workbook's list of sheets

namespace {

void dropAttribute(QByteArray *tag, const char *name)
{
    qsizetype vs = 0, ve = 0;
    if (!xmlscan::attributeValue(*tag, name, &vs, &ve)) return;
    qsizetype n = vs - 2;
    while (n >= 0 && isSpace((*tag)[n])) --n;
    if (n >= 0 && (*tag)[n] == '=') --n;
    while (n >= 0 && isSpace((*tag)[n])) --n;
    qsizetype nameStart = n;
    while (nameStart > 0 && !isSpace((*tag)[nameStart - 1])) --nameStart;
    qsizetype a = nameStart;
    while (a > 0 && isSpace((*tag)[a - 1])) --a;
    tag->remove(a, ve + 1 - a);
}

// Whether a namespace is the one the attributes that name relationships are in (the usual form of the format, or the strict one).
bool isRelationshipNamespace(const QString &uri)
{
    return uri == QLatin1String("http://schemas.openxmlformats.org/officeDocument/2006/relationships") || uri == QLatin1String("http://purl.oclc.org/ooxml/officeDocument/relationships");
}

// The prefix a tag declares for the relationships' namespace ("r" for xmlns:r="..."), or empty if it declares none. What an attribute is called does not say what it is: the prefix the
// namespace is declared with does.
QByteArray relationshipPrefixDeclaredIn(const QByteArray &tag)
{
    static const QRegularExpression re(QStringLiteral("\\sxmlns:([A-Za-z_][A-Za-z0-9_.-]*)\\s*=\\s*(?:\"([^\"]*)\"|'([^']*)')"));
    const QString text = QString::fromUtf8(tag);
    for (auto it = re.globalMatch(text); it.hasNext();) {
        const QRegularExpressionMatch m = it.next();
        if (isRelationshipNamespace(m.captured(2).isEmpty() ? m.captured(3) : m.captured(2))) return m.captured(1).toUtf8();
    }
    return QByteArray();
}

} // namespace

// The workbook part with its list of sheets made from the workbook's own (the names they have now, whether each is shown, and the order of the tabs; a sheet that was deleted is out, and
// one that was added is in, with the sheet number and relationship the workbook planned for it); the names that belong to one sheet follow it to its place in the list, or go with it; and
// the sheet that is open is the one the file said, where it is now.
bool patchWorkbookSheets(const QByteArray &xml, const Workbook &book, QByteArray *out, QString *error)
{
    auto bad = [&] {
        if (error) *error = QCoreApplication::translate("jp::office::sheet::Save", "The workbook part is not in the form this program changes.");
        return false;
    };
    const WorkbookInfo &info = book.info();
    Tag open, close;
    if (!findStart(xml, "sheets", &open) || open.selfClosing || !findEnd(xml, open, &close)) return bad();
    const QByteArray p(xmlscan::prefixOf(open.name));
    // the sheet elements the file has, in the order it has them (a sheet's place in the file is what ties it to its element: an element may have no relationship, or name it by any prefix)
    QVector<QByteArray> inFile;
    // the prefix the relationships' namespace has in the part: declared on the workbook element or on a sheet element; else "r", which is declared when a sheet that is added needs it
    Tag rootTag;
    if (!xmlscan::nextTag(xml, 0, &rootTag) || rootTag.closing) return bad();
    QByteArray relPrefix = relationshipPrefixDeclaredIn(xml.mid(rootTag.start, rootTag.end - rootTag.start));
    const bool declared = !relPrefix.isEmpty();   // (on the workbook element)
    {
        Tag t;
        qsizetype pos = open.end;
        while (xmlscan::nextTag(xml, pos, &t) && t.start < close.start) {
            pos = t.end;
            if (t.closing || t.local != "sheet") continue;
            const QByteArray tag = xml.mid(t.start, t.end - t.start);
            if (relPrefix.isEmpty()) relPrefix = relationshipPrefixDeclaredIn(tag);
            inFile.append(tag);
        }
    }
    if (relPrefix.isEmpty()) relPrefix = "r";
    const QByteArray relAttribute = relPrefix + ":id";
    QByteArray sheets;
    for (const int place : info.order) {
        const SheetEntry &e = info.sheets[place];
        QByteArray tag;
        if (!e.isNew) {
            const int at = int(info.fileOrder.indexOf(place));
            if (at < 0 || at >= inFile.size()) return bad();
            tag = inFile[at];
        } else if (!e.relId.isEmpty()) {
            tag = "<" + p + "sheet name=\"x\" sheetId=\"" + QByteArray::number(e.sheetId) + "\" " + relAttribute + "=\"" + e.relId.toUtf8() + "\"/>";
        } else {
            return bad();
        }
        setAttribute(&tag, "name", xmlscan::escapeAttribute(e.name));
        if (e.state == SheetState::Hidden) setAttribute(&tag, "state", "hidden");
        else if (e.state == SheetState::VeryHidden) setAttribute(&tag, "state", "veryHidden");
        else dropAttribute(&tag, "state");
        sheets += tag;
    }
    QByteArray result = xml.left(open.end) + sheets + xml.mid(close.start);
    // the namespace the relationships' attribute is in must be declared (a new sheet's element names it)
    if (!declared) {
        Tag root;
        if (xmlscan::nextTag(result, 0, &root)) {
            QByteArray rootText = result.mid(root.start, root.end - root.start);
            setAttribute(&rootText, ("xmlns:" + relPrefix).constData(), "http://schemas.openxmlformats.org/officeDocument/2006/relationships");
            result = result.left(root.start) + rootText + result.mid(root.end);
        }
    }
    // the names that belong to one sheet: by the place of the sheet in the list (or gone with it)
    {
        QByteArray rewritten;
        qsizetype copied = 0, pos = 0;
        Tag t;
        while (xmlscan::nextTag(result, pos, &t)) {
            pos = t.end;
            if (t.closing || t.local != "definedName") continue;
            QByteArray tag = result.mid(t.start, t.end - t.start);
            qsizetype vs = 0, ve = 0;
            if (!xmlscan::attributeValue(tag, "localSheetId", &vs, &ve)) continue;
            const int was = tag.mid(vs, ve - vs).toInt();
            const int place = was >= 0 && was < info.fileOrder.size() ? info.fileOrder[was] : -1;
            const int now = place >= 0 && place < info.sheets.size() && !info.sheets[place].deleted ? int(info.order.indexOf(place)) : -1;
            qsizetype end = t.end;
            if (!t.selfClosing) {
                Tag closeName;
                if (!findEnd(result, t, &closeName)) return bad();
                end = closeName.end;
            }
            if (now < 0) {
                rewritten += result.mid(copied, t.start - copied);
                copied = end;
                pos = end;
                continue;
            }
            if (now == was) continue;
            setAttribute(&tag, "localSheetId", QByteArray::number(now));
            rewritten += result.mid(copied, t.start - copied) + tag;
            copied = t.end;
        }
        rewritten += result.mid(copied);
        result = rewritten;
    }
    // the sheet that is open
    {
        Tag view;
        if (findStart(result, "workbookView", &view)) {
            QByteArray tag = result.mid(view.start, view.end - view.start);
            const int active = book.activeTabNow();
            qsizetype a = 0, b = 0;
            if (active != 0 || xmlscan::attributeValue(tag, "activeTab", &a, &b)) setAttribute(&tag, "activeTab", QByteArray::number(active));
            if (xmlscan::attributeValue(tag, "firstSheet", &a, &b)) setAttribute(&tag, "firstSheet", "0");
            result = result.left(view.start) + tag + result.mid(view.end);
        }
    }
    *out = result;
    return true;
}

// ---- the workbook's defined names and calculation settings

namespace {

// Where a calcPr or definedNames element is made when the file has none: after the last of the elements the format puts before it.
qsizetype placeAfterNames(const QByteArray &xml, bool includeNames)
{
    qsizetype at = -1;
    Tag t;
    qsizetype pos = 0;
    while (xmlscan::nextTag(xml, pos, &t)) {
        pos = t.end;
        const bool before = t.local == "sheets" || t.local == "functionGroups" || t.local == "externalReferences" || (includeNames && t.local == "definedNames");
        if (before && (t.closing || t.selfClosing)) at = t.end;
    }
    return at;
}

QByteArray nameElement(const QByteArray &p, const NameChange &c, int id)
{
    QByteArray e = "<" + p + "definedName name=\"" + xmlscan::escapeAttribute(c.name) + "\"";
    if (!c.comment.isEmpty()) e += " comment=\"" + xmlscan::escapeAttribute(c.comment) + "\"";
    if (id >= 0) e += " localSheetId=\"" + QByteArray::number(id) + "\"";
    if (c.hidden) e += " hidden=\"1\"";
    return e + ">" + xmlscan::escapeText(c.text) + "</" + p + "definedName>";
}

} // namespace

bool patchDefinedNames(const QByteArray &xml, const std::vector<NameChange> &changes, const QVector<int> &order, QByteArray *out)
{
    *out = xml;
    if (changes.empty()) return true;
    auto idOf = [&](int place) { return place < 0 ? -1 : int(order.indexOf(place)); };
    struct Existing {
        qsizetype start, tagEnd, end, textEnd;   // the element, its start tag, and where its text ends (before the closing tag)
        bool selfClosing;
        QString name;
        int id;
        bool used = false;
    };
    std::vector<Existing> existing;
    Tag list, listClose;
    const bool hasList = findStart(xml, "definedNames", &list);
    if (hasList && !list.selfClosing) {
        if (!findEnd(xml, list, &listClose)) return false;
        Tag t;
        qsizetype pos = list.end;
        while (xmlscan::nextTag(xml, pos, &t) && t.start < listClose.start) {
            pos = t.end;
            if (t.closing || t.local != "definedName") continue;
            const QByteArray tag = xml.mid(t.start, t.end - t.start);
            Existing e;
            e.start = t.start;
            e.tagEnd = t.end;
            e.end = t.end;
            e.textEnd = t.end;
            e.selfClosing = t.selfClosing;
            qsizetype vs = 0, ve = 0;
            e.name = xmlscan::attributeValue(tag, "name", &vs, &ve) ? xmlscan::unescape(QByteArrayView(tag).sliced(vs, ve - vs)) : QString();
            e.id = xmlscan::attributeValue(tag, "localSheetId", &vs, &ve) ? tag.mid(vs, ve - vs).toInt() : -1;
            if (!t.selfClosing) {
                Tag close;
                if (!findEnd(xml, t, &close)) return false;
                e.end = close.end;
                e.textEnd = close.start;
                pos = close.end;
            }
            existing.push_back(e);
        }
    }
    const QByteArray p = hasList ? QByteArray(xmlscan::prefixOf(list.name)) : prefixOfRoot(xml);
    struct Edit {
        qsizetype start, end;
        QByteArray with;
    };
    std::vector<Edit> edits;
    QByteArray added;
    qint64 removed = 0, addedCount = 0;
    for (const NameChange &c : changes) {
        Existing *found = nullptr;
        if (c.hadOld)
            for (Existing &e : existing)
                if (!e.used && e.id == idOf(c.oldPlace) && e.name.compare(c.oldName, Qt::CaseInsensitive) == 0) {
                    found = &e;
                    break;
                }
        if (found) found->used = true;
        if (c.remove) {
            if (found) {
                edits.push_back({found->start, found->end, QByteArray()});
                ++removed;
            }
            continue;
        }
        if (!found) {
            added += nameElement(p, c, idOf(c.place));
            ++addedCount;
            continue;
        }
        QByteArray tag = xml.mid(found->start, found->tagEnd - found->start);
        const QByteArray was = tag;
        if (found->name != c.name) setAttribute(&tag, "name", xmlscan::escapeAttribute(c.name));
        qsizetype vs = 0, ve = 0;
        const QString haveComment = xmlscan::attributeValue(tag, "comment", &vs, &ve) ? xmlscan::unescape(QByteArrayView(tag).sliced(vs, ve - vs)) : QString();
        if (haveComment != c.comment) {
            if (c.comment.isEmpty()) dropAttribute(&tag, "comment");
            else setAttribute(&tag, "comment", xmlscan::escapeAttribute(c.comment));
        }
        const bool haveHidden = xmlscan::attributeValue(tag, "hidden", &vs, &ve) && (tag.mid(vs, ve - vs) == "1" || tag.mid(vs, ve - vs) == "true");
        if (haveHidden != c.hidden) {
            if (c.hidden) setAttribute(&tag, "hidden", "1");
            else dropAttribute(&tag, "hidden");
        }
        const QString haveText = found->selfClosing ? QString() : xmlscan::unescape(QByteArrayView(xml).sliced(found->tagEnd, found->textEnd - found->tagEnd));
        const bool textChanges = haveText != c.text;
        if (tag == was && !textChanges) continue;
        if (found->selfClosing) {
            if (!textChanges) {
                edits.push_back({found->start, found->tagEnd, tag});
            } else {
                tag.chop(2);
                while (!tag.isEmpty() && isSpace(tag.back())) tag.chop(1);
                edits.push_back({found->start, found->end, tag + ">" + xmlscan::escapeText(c.text) + "</" + p + "definedName>"});
            }
            continue;
        }
        if (tag != was) edits.push_back({found->start, found->tagEnd, tag});
        if (textChanges) edits.push_back({found->tagEnd, found->textEnd, xmlscan::escapeText(c.text)});
    }
    if (edits.empty() && added.isEmpty()) return true;
    const qint64 left = qint64(existing.size()) - removed + addedCount;
    if (hasList && !list.selfClosing) {
        if (left == 0) {
            // (a list with no name in it is not allowed: it goes)
            *out = xml.left(list.start) + xml.mid(listClose.end);
            return true;
        }
        if (!added.isEmpty()) edits.push_back({listClose.start, listClose.start, added});
    } else if (hasList) {
        if (added.isEmpty()) return true;
        *out = xml.left(list.start) + "<" + p + "definedNames>" + added + "</" + p + "definedNames>" + xml.mid(list.end);
        return true;
    } else {
        const qsizetype at = placeAfterNames(xml, false);
        if (at < 0) return false;
        *out = xml.left(at) + "<" + p + "definedNames>" + added + "</" + p + "definedNames>" + xml.mid(at);
        return true;
    }
    std::stable_sort(edits.begin(), edits.end(), [](const Edit &a, const Edit &b) { return a.start < b.start; });
    QByteArray result;
    qsizetype copied = 0;
    for (const Edit &e : edits) {
        if (e.start < copied) return false;
        result += xml.mid(copied, e.start - copied) + e.with;
        copied = e.end;
    }
    *out = result + xml.mid(copied);
    return true;
}

bool patchCalcPr(const QByteArray &xml, CalcMode mode, bool iterate, int iterateCount, double iterateDelta, QByteArray *out)
{
    Tag calc;
    const bool has = findStart(xml, "calcPr", &calc);
    const QByteArray p = prefixOfRoot(xml);
    QByteArray tag = has ? xml.mid(calc.start, calc.end - calc.start) : "<" + p + "calcPr/>";
    const QByteArray was = tag;
    if (mode == CalcMode::Manual) setAttribute(&tag, "calcMode", "manual");
    else if (mode == CalcMode::AutoNoTable) setAttribute(&tag, "calcMode", "autoNoTable");
    else dropAttribute(&tag, "calcMode");
    if (iterate) {
        setAttribute(&tag, "iterate", "1");
        setAttribute(&tag, "iterateCount", QByteArray::number(iterateCount));
        setAttribute(&tag, "iterateDelta", QByteArray::number(iterateDelta, 'g', 12));
    } else {
        dropAttribute(&tag, "iterate");
    }
    if (has && tag == was) {
        *out = xml;
        return true;
    }
    if (has) {
        *out = xml.left(calc.start) + tag + xml.mid(calc.end);
        return true;
    }
    const qsizetype at = placeAfterNames(xml, true);
    if (at < 0) return false;
    *out = xml.left(at) + tag + xml.mid(at);
    return true;
}

// ---- the whole file

namespace {

// A relationship of the part's relationships taken out by its Id; false (nothing changed) if there is none.
bool removeRelationshipById(const QByteArray &xml, const QString &id, QByteArray *out)
{
    Tag t;
    qsizetype pos = 0;
    while (xmlscan::nextTag(xml, pos, &t)) {
        pos = t.end;
        if (t.closing || t.local != "Relationship") continue;
        bool found = false;
        const QByteArrayView value = xmlscan::attribute(QByteArrayView(xml).sliced(t.start, t.end - t.start), "Id", &found);
        if (!found || xmlscan::unescape(value) != id) continue;
        qsizetype from = t.start;
        while (from > 0 && isSpace(xml[from - 1])) --from;
        *out = xml.left(from) + xml.mid(t.end);
        return true;
    }
    return false;
}

// A relationship made, at the end of the part's list; an Override of [Content_Types].xml the same.
bool appendBefore(const QByteArray &xml, const char *container, const QByteArray &element, QByteArray *out)
{
    Tag open, close;
    if (!findStart(xml, container, &open) || open.selfClosing || !findEnd(xml, open, &close)) return false;
    const QByteArray p(xmlscan::prefixOf(open.name));
    QByteArray e = element;
    e.replace("<", "<" + p);
    *out = xml.left(close.start) + e + xml.mid(close.start);
    return true;
}

// Where a part's own relationships are kept: "/xl/worksheets/sheet2.xml" has "/xl/worksheets/_rels/sheet2.xml.rels".
QString relationshipsPartOf(const QString &part)
{
    const int slash = int(part.lastIndexOf(QLatin1Char('/')));
    return (slash < 0 ? QString() : part.left(slash + 1)) + QStringLiteral("_rels/") + (slash < 0 ? part : part.mid(slash + 1)) + QStringLiteral(".rels");
}

class ZipSink : public PartSink {
public:
    explicit ZipSink(ZipWriter *zip) : m_zip(zip) {}
    void write(const char *data, qsizetype size) override { m_zip->write(data, size); }

private:
    ZipWriter *m_zip;
};

class BufferSink : public PartSink {
public:
    void write(const char *data, qsizetype size) override { bytes.append(data, size); }
    QByteArray bytes;
};

} // namespace

bool saveXlsx(const Workbook &book, const SaveChanges &requested, QByteArray *out, QString *error, SaveReport *report, const std::atomic<bool> *cancel)
{
    SaveReport local;
    SaveReport &rep = report ? *report : local;
    rep = SaveReport();
    QString why;
    auto fail = [&](const QString &text) {
        if (error) *error = text;
        return false;
    };
    // A workbook read from the older binary form has no package yet: the first save writes one from what was read (xlsxbase.h), and the changes are put in that.
    Package baseFile;
    const Package *base = &book.package();
    if (!base->isLazy() && book.isConverted()) {
        QByteArray bytes;
        if (!writeBaseXlsx(book, &bytes, &why, cancel)) return fail(why);
        if (!baseFile.readLazy(bytes, &why)) return fail(why);
        base = &baseFile;
    }
    const Package &package = *base;
    if (!package.isLazy()) return fail(QCoreApplication::translate("jp::office::sheet::Save", "The workbook was not opened from a file this program can save over."));
    // ---- a sheet that gets its first drawing is given the relationship that names it, and the element in its XML (the sheet's changes say so)
    SaveChanges adjusted;
    const SaveChanges *useChanges = &requested;
    std::map<int, QString> newDrawingRel;   // by the sheet's place: the id of the relationship from the sheet to the drawing that is made for it
    if (!requested.addedObjects.empty()) {
        adjusted = requested;
        useChanges = &adjusted;
        qint64 objects = 0;
        for (const auto &[sheet, add] : requested.addedObjects) {
            if (sheet < 0 || sheet >= book.info().sheets.size()) return fail(QCoreApplication::translate("jp::office::sheet::Save", "A change is to a sheet the workbook does not have."));
            const SheetEntry &e = book.info().sheets[sheet];
            if (e.deleted || add.objects.empty()) continue;
            objects += qint64(add.objects.size());
            if (e.kind != SheetKind::Worksheet || add.part.isEmpty() || objects > kMaxAddedObjects)
                return fail(QCoreApplication::translate("jp::office::sheet::Save", "Objects were put on a sheet this program can't put them on."));
            if (!e.isNew && !drawingPartOf(package, e.part).isEmpty()) continue;
            QStringList used;
            if (!e.isNew)
                for (const Relationship &r : package.relationships(e.part)) used << r.id;
            else if (const auto own = requested.sheets.find(sheet); own != requested.sheets.end() && !own->second.copyFromPart.isEmpty())
                for (const Relationship &r : package.relationships(own->second.copyFromPart)) used << r.id;   // (a copy keeps the ids its links have)
            const QString id = freeRelationshipId(used);
            newDrawingRel[sheet] = id;
            adjusted.sheets[sheet].drawingRelId = id;
        }
    }
    const SaveChanges &changes = *useChanges;
    const QByteArray bytes = package.fileBytes();
    QVector<ZipEntry> directory;
    if (!readZipDirectory(bytes, &directory)) return fail(QCoreApplication::translate("jp::office::sheet::Save", "The file this workbook was opened from is damaged."));
    QHash<QString, int> entryAt;   // by the plain name, the first entry of it
    for (int i = 0; i < directory.size(); ++i) {
        const QString key = plain(directory[i].name);
        if (!entryAt.contains(key)) entryAt.insert(key, i);
    }
    auto entryOfPart = [&](const QString &part) -> int {
        const QString name = package.entryName(part);
        return name.isEmpty() ? -1 : entryAt.value(plain(name), -1);
    };
    auto inflate = [&](int index, QByteArray *data) { return index >= 0 && inflateZipEntry(bytes, directory[index], data); };

    QHash<int, QByteArray> replaced;   // entries written anew from bytes in memory
    QHash<int, int> sheetOfEntry;      // entries streamed through the sheet patcher: the sheet
    QSet<int> dropped;

    // ---- the sheets
    bool anyCellChanged = false;
    for (const auto &[sheet, sc] : changes.sheets) {
        if (sc.empty()) continue;
        if (sheet < 0 || sheet >= book.info().sheets.size()) return fail(QCoreApplication::translate("jp::office::sheet::Save", "A change is to a sheet the workbook does not have."));
        if (book.info().sheets[sheet].deleted) continue;   // (it is not in the file)
        if (book.info().sheets[sheet].isNew) {
            // (a sheet that was added is made whole: see below)
            if (!sc.cells.empty()) anyCellChanged = true;
            continue;
        }
        const int index = entryOfPart(book.info().sheets[sheet].part);
        if (index < 0) return fail(QCoreApplication::translate("jp::office::sheet::Save", "A sheet that changed has no part in the file."));
        sheetOfEntry.insert(index, sheet);
        if (!sc.cells.empty()) anyCellChanged = true;
        rep.cellsChanged += qint64(sc.cells.size());
        rep.resultsChanged += qint64(sc.results.size());
    }
    // ---- rows, columns, and cells inserted and deleted: the sheets they move cells on are renumbered, and any sheet (and chart, drawing, note, name, ...) that refers to cells follows
    MoveContext ctx;
    ctx.edits = changes.edits;
    ctx.renames = changes.renames;
    ctx.nameRenames = changes.nameRenames;
    ctx.permutes = changes.permutes;
    // (the file's own names: the edits to rows and columns say which sheet by its place, and the file has not been told of the renames yet)
    for (const SheetEntry &se : book.info().sheets) ctx.sheetNames << (se.isNew ? QString() : se.fileName);
    ctx.sheetOps = changes.sheetOps;
    const bool moves = !ctx.edits.empty() || !ctx.renames.empty() || !ctx.nameRenames.empty() || !ctx.sheetOps.empty() || !ctx.permutes.empty();
    QSet<int> moveSheets;   // the sheets (by their place) whose part is made by the mover
    if (moves) {
        anyCellChanged = true;
        for (int sheet = 0; sheet < book.info().sheets.size(); ++sheet) {
            if (book.info().sheets[sheet].kind != SheetKind::Worksheet || book.info().sheets[sheet].deleted || book.info().sheets[sheet].isNew) continue;
            const int index = entryOfPart(book.info().sheets[sheet].part);
            if (index < 0) continue;
            if (cancel && cancel->load()) return fail(QCoreApplication::translate("jp::office::sheet::Save", "The save was stopped."));
            bool needed = movesCellsOf(ctx, sheet);
            if (!needed) {
                // (a sheet the edits do not move cells on may still have what refers to cells that moved: looked at, and left as it is if not)
                ZipEntryDevice device(bytes, directory[index]);
                if (!device.open(QIODevice::ReadOnly)) return fail(QCoreApplication::translate("jp::office::sheet::Save", "A sheet of the file could not be read to be saved."));
                if (!moveSheetPart(&device, nullptr, ctx, sheet, &needed, &why, cancel)) return fail(why);
            }
            if (!needed) continue;
            moveSheets.insert(sheet);
            sheetOfEntry.insert(index, sheet);
        }
        // the parts that name cells
        auto replacePart = [&](const QString &part, const std::function<bool(const QByteArray &, QByteArray *, bool *)> &fn) {
            const int index = entryOfPart(part);
            if (index < 0 || replaced.contains(index)) return;
            QByteArray xml, now;
            bool changed = false;
            if (!inflate(index, &xml) || !fn(xml, &now, &changed) || !changed) return;
            replaced.insert(index, now);
        };
        // the sheet each chart is on (the drawing of a sheet names it): what a reference with no sheet in it, or a name of the sheet, means in it
        QHash<QString, int> chartSheet;
        for (int sheet = 0; sheet < book.info().sheets.size(); ++sheet)
            for (const Relationship &d : package.relationshipsOfType(book.info().sheets[sheet].part, QStringLiteral("/drawing")))
                if (!d.external)
                    for (const Relationship &c : package.relationshipsOfType(d.target, QStringLiteral("/chart")))
                        if (!c.external) chartSheet.insert(c.target.toLower(), sheet);
        static const QRegularExpression chartPart(QStringLiteral("/charts/(chart|chartEx)\\d*\\.xml$")), cachePart(QStringLiteral("/pivotCacheDefinition\\d*\\.xml$"));
        for (const QString &part : package.partNames()) {
            if (chartPart.match(part).hasMatch()) replacePart(part, [&](const QByteArray &x, QByteArray *o, bool *c) { return moveChartPart(x, ctx, o, c, chartSheet.value(part.toLower(), 0)); });
            else if (cachePart.match(part).hasMatch()) replacePart(part, [&](const QByteArray &x, QByteArray *o, bool *c) { return movePivotCachePart(x, ctx, o, c); });
        }
        for (int sheet = 0; sheet < book.info().sheets.size(); ++sheet) {
            if (book.info().sheets[sheet].kind != SheetKind::Worksheet || book.info().sheets[sheet].deleted || book.info().sheets[sheet].isNew) continue;
            const QString sheetPart = book.info().sheets[sheet].part;
            for (const Relationship &r : package.relationshipsOfType(sheetPart, QStringLiteral("/drawing")))
                if (!r.external) replacePart(r.target, [&](const QByteArray &x, QByteArray *o, bool *c) { return moveDrawingPart(x, ctx, sheet, o, c); });
            for (const Relationship &r : package.relationshipsOfType(sheetPart, QStringLiteral("/comments")))
                if (!r.external) replacePart(r.target, [&](const QByteArray &x, QByteArray *o, bool *c) { return moveCommentsPart(x, ctx, sheet, o, c); });
            for (const Relationship &r : package.relationshipsOfType(sheetPart, QStringLiteral("/threadedComment")))
                if (!r.external) replacePart(r.target, [&](const QByteArray &x, QByteArray *o, bool *c) { return moveThreadedCommentsPart(x, ctx, sheet, o, c); });
            for (const Relationship &r : package.relationshipsOfType(sheetPart, QStringLiteral("/ctrlProp")))
                if (!r.external) replacePart(r.target, [&](const QByteArray &x, QByteArray *o, bool *c) { return moveControlPart(x, ctx, sheet, o, c); });
            for (const Relationship &r : package.relationshipsOfType(sheetPart, QStringLiteral("/vmlDrawing")))
                if (!r.external) replacePart(r.target, [&](const QByteArray &x, QByteArray *o, bool *c) { return moveNotesDrawing(x, ctx, sheet, o, c); });
        }
    }
    // ---- the objects of drawings that were deleted, and the caches of the charts whose cells changed (on what the moves above made of the parts)
    auto partBytes = [&](const QString &part, QByteArray *xml) {
        const int index = entryOfPart(part);
        if (index < 0) return -1;
        if (replaced.contains(index)) *xml = replaced.value(index);
        else if (!inflate(index, xml)) return -1;
        return index;
    };
    for (const auto &[part, anchors] : changes.anchors) {
        QByteArray xml, now;
        bool changed = false;
        const int index = partBytes(part, &xml);
        if (index >= 0 && setDrawingAnchors(xml, anchors, &now, &changed) && changed) replaced.insert(index, now);
    }
    for (const auto &[part, names] : changes.objectNames) {
        QByteArray xml, now;
        bool changed = false;
        const int index = partBytes(part, &xml);
        if (index >= 0 && setDrawingObjectNames(xml, names, &now, &changed) && changed) replaced.insert(index, now);
    }
    // (what the objects taken away relate to: the links of the drawing they alone used, and the parts nothing else relates to)
    std::vector<std::pair<QString, QStringList>> released;
    for (const auto &[part, ordinals] : changes.removedObjects) {
        QByteArray xml, now;
        bool changed = false;
        QStringList ids;
        const int index = partBytes(part, &xml);
        if (index >= 0 && removeDrawingObjects(xml, ordinals, &now, &changed, &ids) && changed) {
            replaced.insert(index, now);
            // the ids the drawing still names are kept
            for (const QString &id : namedRelationships(now)) ids.removeAll(id);
            if (!ids.isEmpty()) released.emplace_back(part, ids);
        }
    }
    if (!released.empty()) {
        QHash<QString, QVector<QPair<QString, QString>>> into;   // by the part a relationship goes to (lower case): (the part it is from, its id), both lower case
        auto addRelationships = [&](const QString &source) {
            for (const Relationship &r : package.relationships(source))
                if (!r.external) into[r.target.toLower()].append({source.toLower(), r.id});
        };
        addRelationships(QStringLiteral("/"));
        for (const QString &name : package.partNames()) addRelationships(name);
        QSet<QPair<QString, QString>> gone;   // relationships that are dropped: (the part they are from, the id)
        QSet<QString> goneParts;              // parts that are dropped
        for (const auto &[part, ids] : released)
            for (const QString &id : ids) gone.insert({part.toLower(), id});
        std::function<void(const QString &, int)> consider = [&](const QString &target, int depth) {
            const QString key = target.toLower();
            if (goneParts.contains(key) || depth > 8) return;
            const auto &refs = into.value(key);
            for (const auto &r : refs)
                if (!gone.contains(r) && !goneParts.contains(r.first)) return;   // something that stays still relates to it
            goneParts.insert(key);
            for (const Relationship &r : package.relationships(target))
                if (!r.external) {
                    gone.insert({key, r.id});
                    consider(r.target, depth + 1);
                }
        };
        for (const auto &[part, ids] : released)
            for (const QString &id : ids) {
                const Relationship r = package.relationship(part, id);
                if (!r.id.isEmpty() && !r.external) consider(r.target, 0);
            }
        auto relsEntryOf = [&](const QString &part) {
            const QString name = package.entryName(part);
            if (name.isEmpty()) return -1;
            const int slash = name.lastIndexOf(QLatin1Char('/'));
            return entryAt.value(plain((slash < 0 ? QString() : name.left(slash + 1)) + QStringLiteral("_rels/") + (slash < 0 ? name : name.mid(slash + 1)) + QStringLiteral(".rels")), -1);
        };
        for (const auto &[part, ids] : released) {
            QByteArray rels, now;
            const int relsEntry = relsEntryOf(part);
            if (relsEntry < 0) continue;
            if (replaced.contains(relsEntry)) rels = replaced.value(relsEntry);
            else if (!inflate(relsEntry, &rels)) continue;
            if (removeRelationshipsById(rels, ids, &now)) replaced.insert(relsEntry, now);
        }
        int typesEntry = -1;
        for (int i = 0; i < directory.size(); ++i)
            if (plain(directory[i].name) == QLatin1String("[content_types].xml")) {
                typesEntry = i;
                break;
            }
        for (const QString &key : std::as_const(goneParts)) {
            // (the part by the name the package has for it)
            QString name;
            for (const QString &n : package.partNames())
                if (n.toLower() == key) {
                    name = n;
                    break;
                }
            if (name.isEmpty()) continue;
            const int entry = entryOfPart(name);
            if (entry >= 0) dropped.insert(entry);
            const int rels = relsEntryOf(name);
            if (rels >= 0) {
                dropped.insert(rels);
                replaced.remove(rels);
            }
            QByteArray types, now;
            if (typesEntry >= 0) {
                if (replaced.contains(typesEntry)) types = replaced.value(typesEntry);
                else inflate(typesEntry, &types);
                if (!types.isEmpty() && removeContentTypeOverride(types, name, &now)) replaced.insert(typesEntry, now);
            }
        }
    }
    // ---- the objects a person put on sheets: their anchors at the end of the drawing, the parts they relate to, and the links and types that name them
    std::vector<std::pair<QString, QByteArray>> newEntries;   // parts the file did not have (the name without its first slash)
    std::map<int, QByteArray> newSheetRels;                   // for a sheet that was added: the relationship to its new drawing (its relationships part is made with it)
    if (!changes.addedObjects.empty()) {
        int typesEntry = -1;
        for (int i = 0; i < directory.size(); ++i)
            if (plain(directory[i].name) == QLatin1String("[content_types].xml")) {
                typesEntry = i;
                break;
            }
        QByteArray types;
        if (typesEntry < 0 || !(replaced.contains(typesEntry) ? (types = replaced.value(typesEntry), true) : inflate(typesEntry, &types)))
            return fail(QCoreApplication::translate("jp::office::sheet::Save", "The file's list of part types is not in it."));
        QSet<QString> taken;
        for (const QString &n : package.partNames()) taken.insert(n.toLower());
        auto relsEntryFor = [&](const QString &part) {
            const QString name = package.entryName(part);
            if (name.isEmpty()) return -1;
            const int slash = name.lastIndexOf(QLatin1Char('/'));
            return entryAt.value(plain((slash < 0 ? QString() : name.left(slash + 1)) + QStringLiteral("_rels/") + (slash < 0 ? name : name.mid(slash + 1)) + QStringLiteral(".rels")), -1);
        };
        auto current = [&](int index, QByteArray *data) { return index >= 0 && (replaced.contains(index) ? (*data = replaced.value(index), true) : inflate(index, data)); };
        auto bad = [&] { return fail(QCoreApplication::translate("jp::office::sheet::Save", "A drawing of the file is not in the form this program changes.")); };
        qint64 bytesAdded = 0;
        for (const auto &[sheet, add] : changes.addedObjects) {
            const SheetEntry &se = book.info().sheets[sheet];
            if (se.deleted || add.objects.empty()) continue;
            const bool newDrawing = newDrawingRel.count(sheet) != 0;
            const QString drawingPart = add.part;
            QByteArray drawingRels;
            int drawingRelsEntry = -1;
            if (!newDrawing) {
                drawingRelsEntry = relsEntryFor(drawingPart);
                if (drawingRelsEntry >= 0 && !current(drawingRelsEntry, &drawingRels)) return bad();
            } else if (taken.contains(drawingPart.toLower())) {
                return bad();
            }
            QStringList used = relationshipIds(drawingRels);
            QByteArray anchors;
            for (const AddedObject &obj : add.objects) {
                QByteArray xml = obj.xml;
                for (const AddedPart &part : obj.parts) {
                    const QString id = freeRelationshipId(used);
                    used << id;
                    xml.replace(xmlscan::escapeAttribute(part.token), xmlscan::escapeAttribute(id));
                    if (!part.contentType.isEmpty()) {
                        // a part the file does not have
                        const QByteArray data = part.produce ? part.produce() : part.bytes;
                        bytesAdded += data.size();
                        if (bytesAdded > kMaxAddedBytes || taken.contains(part.part.toLower()) || data.isEmpty()) return bad();
                        taken.insert(part.part.toLower());
                        newEntries.emplace_back(part.part.startsWith(QLatin1Char('/')) ? part.part.mid(1) : part.part, data);
                        QByteArray t2;
                        const int dot = int(part.part.lastIndexOf(QLatin1Char('.')));
                        const bool media = part.part.startsWith(QLatin1String("/xl/media/"), Qt::CaseInsensitive) && dot > 0;
                        if (!(media ? addContentDefault(types, part.part.mid(dot + 1), part.contentType, &t2) : addContentOverride(types, part.part, part.contentType, &t2))) return bad();
                        types = t2;
                    }
                    QByteArray r2;
                    if (!addRelationship(drawingRels, id, part.relType, relativeTarget(drawingPart, part.part), &r2)) return bad();
                    drawingRels = r2;
                }
                anchors += xml;
            }
            if (newDrawing) {
                newEntries.emplace_back(drawingPart.startsWith(QLatin1Char('/')) ? drawingPart.mid(1) : drawingPart, newDrawingXml(anchors));
                taken.insert(drawingPart.toLower());
                QByteArray t2;
                if (!addContentOverride(types, drawingPart, QStringLiteral("application/vnd.openxmlformats-officedocument.drawing+xml"), &t2)) return bad();
                types = t2;
                QString relsName = relationshipsPartOf(drawingPart);
                newEntries.emplace_back(relsName.startsWith(QLatin1Char('/')) ? relsName.mid(1) : relsName, drawingRels);
                // the sheet's link to it
                const QString target = relativeTarget(se.part, drawingPart);
                if (se.isNew) {
                    newSheetRels[sheet] = relationshipXml(newDrawingRel[sheet], QStringLiteral("drawing"), target);
                } else {
                    const int sheetRelsEntry = relsEntryFor(se.part);
                    QByteArray sheetRels, r2;
                    if (sheetRelsEntry >= 0 && !current(sheetRelsEntry, &sheetRels)) return bad();
                    if (!addRelationship(sheetRels, newDrawingRel[sheet], QStringLiteral("drawing"), target, &r2)) return bad();
                    if (sheetRelsEntry >= 0) {
                        replaced.insert(sheetRelsEntry, r2);
                    } else {
                        const QString name = relationshipsPartOf(se.part);
                        newEntries.emplace_back(name.startsWith(QLatin1Char('/')) ? name.mid(1) : name, r2);
                    }
                }
            } else {
                const int drawingEntry = entryOfPart(drawingPart);
                QByteArray xml;
                if (!current(drawingEntry, &xml)) return bad();
                const QByteArray now = drawingWithAnchors(xml, anchors);
                if (now.isEmpty()) return bad();
                replaced.insert(drawingEntry, now);
                if (drawingRelsEntry >= 0) replaced.insert(drawingRelsEntry, drawingRels);
                else if (!drawingRels.isEmpty()) {
                    const QString name = relationshipsPartOf(drawingPart);
                    newEntries.emplace_back(name.startsWith(QLatin1Char('/')) ? name.mid(1) : name, drawingRels);
                }
            }
        }
        replaced.insert(typesEntry, types);
    }
    // (the objects drawn in another order: the children of the drawing, as what is above leaves them)
    for (const auto &[part, order] : changes.drawingOrder) {
        QByteArray xml, now;
        bool changed = false;
        const int index = partBytes(part, &xml);
        if (index >= 0 && reorderDrawingChildren(xml, order, &now, &changed) && changed) replaced.insert(index, now);
    }
    for (const auto &[part, caches] : changes.chartCaches) {
        QByteArray xml, now;
        bool changed = false;
        const int index = partBytes(part, &xml);
        if (index >= 0 && refreshChartCaches(xml, caches, &now, &changed) && changed) replaced.insert(index, now);
    }
    // ---- the shared strings
    if (!changes.newSharedStrings.isEmpty()) {
        const int index = entryOfPart(book.sharedStringsPart());
        QByteArray xml, patched;
        if (index < 0 || !inflate(index, &xml)) return fail(QCoreApplication::translate("jp::office::sheet::Save", "The shared strings part is not in the file."));
        if (!patchSharedStrings(xml, changes.newSharedStrings, &patched, &why)) return fail(why);
        replaced.insert(index, patched);
    }
    // ---- the styles
    if (!changes.newCellFormats.isEmpty() || !changes.newNumberFormats.isEmpty() || !changes.newFonts.isEmpty() || !changes.newFills.isEmpty() || !changes.newBorders.isEmpty() || !changes.newStyles.isEmpty() || !changes.newDxfs.isEmpty()) {
        const int index = entryOfPart(book.stylesPart());
        QByteArray xml, patched;
        if (index < 0 || !inflate(index, &xml)) return fail(QCoreApplication::translate("jp::office::sheet::Save", "The styles part is not in the file."));
        if (!patchStyles(xml, changes, &patched, &why)) return fail(why);
        replaced.insert(index, patched);
    }
    // ---- the table parts whose columns were renamed
    for (const auto &[part, data] : changes.replacedParts) {
        const int index = entryOfPart(part);
        if (index < 0) return fail(QCoreApplication::translate("jp::office::sheet::Save", "A table that changed has no part in the file."));
        replaced.insert(index, data);
    }
    for (const auto &[part, make] : changes.producedParts) {
        const int index = entryOfPart(part);
        if (index < 0) return fail(QCoreApplication::translate("jp::office::sheet::Save", "A chart that changed has no part in the file."));
        replaced.insert(index, make());
    }
    // ---- the calculation chain: left out when a cell changed (it may name a cell that no longer holds a formula)
    if (anyCellChanged) {
        const QVector<Relationship> chains = package.relationshipsOfType(book.mainPart(), QStringLiteral("/calcChain"));
        if (!chains.isEmpty() && !chains.first().external) {
            const QString part = chains.first().target;
            const int chainEntry = entryOfPart(part);
            // the three places that name it (the part, its type in [Content_Types].xml, and the workbook's link to it) all go, or none does
            int typesEntry = -1;
            for (int i = 0; i < directory.size(); ++i)
                if (plain(directory[i].name) == QLatin1String("[content_types].xml")) {
                    typesEntry = i;
                    break;
                }
            const QString mainEntryName = package.entryName(book.mainPart());
            const int slash = mainEntryName.lastIndexOf(QLatin1Char('/'));
            const QString relsName = (slash < 0 ? QString() : mainEntryName.left(slash + 1)) + QStringLiteral("_rels/") + (slash < 0 ? mainEntryName : mainEntryName.mid(slash + 1)) + QStringLiteral(".rels");
            const int relsEntry = entryAt.value(plain(relsName), -1);
            QByteArray types, rels, newTypes, newRels;
            const bool haveTypes = typesEntry >= 0 && (replaced.contains(typesEntry) ? (types = replaced.value(typesEntry), true) : inflate(typesEntry, &types));
            if (chainEntry >= 0 && haveTypes && relsEntry >= 0 && inflate(relsEntry, &rels) && removeContentTypeOverride(types, part, &newTypes)
                && removeRelationshipOfType(rels, QStringLiteral("/calcChain"), &newRels)) {
                replaced.insert(typesEntry, newTypes);
                replaced.insert(relsEntry, newRels);
                dropped.insert(chainEntry);
                rep.calcChainDropped = true;
            }
        }
    }
    // ---- the sheets: added (made whole, below), deleted (their parts, and what only they use, are left out), renamed, moved, hidden, and shown
    struct NewSheet {
        int sheet;
        QString part;
    };
    std::vector<NewSheet> newSheets;
    if (changes.sheetsChanged) {
        const WorkbookInfo &info = book.info();
        // the parts that go with the sheets deleted: those they have and the rest of the workbook does not
        QHash<QString, QString> gone;   // by the plain name
        {
            QHash<QString, QString> roots;
            for (const SheetEntry &e : info.sheets)
                if (e.deleted && !e.isNew && !e.part.isEmpty()) roots.insert(plain(e.part), e.part);
            if (!roots.isEmpty()) {
                QSet<QString> kept;
                std::vector<QString> stack{QStringLiteral("/")};
                while (!stack.empty()) {
                    const QString source = stack.back();
                    stack.pop_back();
                    for (const Relationship &r : package.relationships(source)) {
                        if (r.external) continue;
                        const QString key = plain(r.target);
                        if (roots.contains(key) || kept.contains(key)) continue;
                        kept.insert(key);
                        stack.push_back(r.target);
                    }
                }
                gone = roots;
                std::vector<QString> work;
                for (const QString &part : roots) work.push_back(part);
                while (!work.empty()) {
                    const QString source = work.back();
                    work.pop_back();
                    for (const Relationship &r : package.relationships(source)) {
                        if (r.external) continue;
                        const QString key = plain(r.target);
                        if (kept.contains(key) || gone.contains(key)) continue;
                        gone.insert(key, r.target);
                        work.push_back(r.target);
                    }
                }
            }
        }
        const QString mainEntryName = package.entryName(book.mainPart());
        const int slash = mainEntryName.lastIndexOf(QLatin1Char('/'));
        const QString relsName = (slash < 0 ? QString() : mainEntryName.left(slash + 1)) + QStringLiteral("_rels/") + (slash < 0 ? mainEntryName : mainEntryName.mid(slash + 1)) + QStringLiteral(".rels");
        int typesEntry = -1;
        for (int i = 0; i < directory.size(); ++i)
            if (plain(directory[i].name) == QLatin1String("[content_types].xml")) {
                typesEntry = i;
                break;
            }
        const int relsEntry = entryAt.value(plain(relsName), -1);
        if (typesEntry < 0 || relsEntry < 0) return fail(QCoreApplication::translate("jp::office::sheet::Save", "The workbook's relationships are not in the file."));
        QByteArray types, rels;
        if (replaced.contains(typesEntry)) types = replaced.value(typesEntry);
        else if (!inflate(typesEntry, &types)) return fail(QCoreApplication::translate("jp::office::sheet::Save", "The workbook's relationships are not in the file."));
        if (replaced.contains(relsEntry)) rels = replaced.value(relsEntry);
        else if (!inflate(relsEntry, &rels)) return fail(QCoreApplication::translate("jp::office::sheet::Save", "The workbook's relationships are not in the file."));
        for (auto it = gone.cbegin(); it != gone.cend(); ++it) {
            const QString &part = it.value();
            QByteArray t2;
            if (removeContentTypeOverride(types, part, &t2)) types = t2;
            const int entry = entryOfPart(part);
            if (entry >= 0) dropped.insert(entry);
            const int relsOfPart = entryAt.value(plain(relationshipsPartOf(package.entryName(part).isEmpty() ? part : QStringLiteral("/") + package.entryName(part))), -1);
            if (relsOfPart >= 0) dropped.insert(relsOfPart);
        }
        for (const SheetEntry &e : info.sheets) {
            if (e.deleted && !e.isNew && !e.relId.isEmpty()) {
                QByteArray r2;
                if (removeRelationshipById(rels, e.relId, &r2)) rels = r2;
            }
        }
        QString mainDir = book.mainPart().left(book.mainPart().lastIndexOf(QLatin1Char('/')) + 1);
        for (int sheet = 0; sheet < info.sheets.size(); ++sheet) {
            const SheetEntry &e = info.sheets[sheet];
            if (!e.isNew || e.deleted || e.part.isEmpty() || e.relId.isEmpty()) continue;
            QByteArray t2, r2;
            if (!appendBefore(types, "Types", "<Override PartName=\"" + xmlscan::escapeAttribute(e.part) + "\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml\"/>", &t2))
                return fail(QCoreApplication::translate("jp::office::sheet::Save", "The workbook's relationships are not in the form this program changes."));
            types = t2;
            const QString target = e.part.startsWith(mainDir, Qt::CaseInsensitive) ? e.part.mid(mainDir.size()) : e.part;
            if (!appendBefore(rels, "Relationships",
                              "<Relationship Id=\"" + xmlscan::escapeAttribute(e.relId) + "\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet\" Target=\"" + xmlscan::escapeAttribute(target) + "\"/>", &r2))
                return fail(QCoreApplication::translate("jp::office::sheet::Save", "The workbook's relationships are not in the form this program changes."));
            rels = r2;
            newSheets.push_back({sheet, e.part});
        }
        replaced.insert(typesEntry, types);
        replaced.insert(relsEntry, rels);
        anyCellChanged = true;
    }
    // ---- the workbook part: the names follow the cells, and the other program is asked to work the formulas out again when it is to
    // (and the hidden names of the sheets' filters, for the sheets whose filter was changed)
    std::vector<FilterName> filterNames;
    const WorkbookInfo &listed = book.info();
    for (const auto &entry : changes.sheets) {
        if (!entry.second.replaceAutoFilter) continue;
        const int place = entry.first;
        if (place < 0 || place >= listed.sheets.size() || listed.sheets[place].deleted) continue;
        const int id = int(listed.order.indexOf(place));
        if (id < 0) continue;
        FilterName name;
        name.localSheetId = id;
        const CellRange &r = entry.second.autoFilter.ref;
        if (entry.second.autoFilter.valid()) {
            QString sheetName = listed.sheets[place].name;
            sheetName.replace(QLatin1Char('\''), QStringLiteral("''"));
            name.range = QLatin1Char('\'') + sheetName + QStringLiteral("'!$") + columnName(r.col1) + QLatin1Char('$') + QString::number(r.row1 + 1) + QStringLiteral(":$") + columnName(r.col2)
                         + QLatin1Char('$') + QString::number(r.row2 + 1);
        }
        filterNames.push_back(name);
    }
    if (changes.fullCalcOnLoad || moves || changes.sheetsChanged || !changes.printNames.empty() || !filterNames.empty() || !changes.nameChanges.empty() || changes.calcChanged) {
        const int index = entryOfPart(book.mainPart());
        QByteArray xml, patched;
        if (index >= 0 && inflate(index, &xml)) {
            bool changed = false;
            if (moves && moveWorkbookPart(xml, ctx, &patched, &changed) && changed) xml = patched;
            if (changes.sheetsChanged) {
                if (!patchWorkbookSheets(xml, book, &patched, &why)) return fail(why);
                xml = patched;
                changed = true;
            }
            if (!changes.printNames.empty() && patchPrintNames(xml, changes.printNames, &patched) && patched != xml) {
                xml = patched;
                changed = true;
            }
            if (!filterNames.empty()) {
                if (!patchFilterNames(xml, filterNames, &patched, &why)) return fail(why);
                changed = changed || patched != xml;
                xml = patched;
            }
            if (!changes.nameChanges.empty()) {
                if (!patchDefinedNames(xml, changes.nameChanges, book.info().order, &patched)) return fail(QCoreApplication::translate("jp::office::sheet::Save", "The workbook part is not in the form this program changes."));
                changed = changed || patched != xml;
                xml = patched;
            }
            if (changes.calcChanged) {
                if (!patchCalcPr(xml, changes.calcMode, changes.iterate, changes.iterateCount, changes.iterateDelta, &patched)) return fail(QCoreApplication::translate("jp::office::sheet::Save", "The workbook part is not in the form this program changes."));
                changed = changed || patched != xml;
                xml = patched;
            }
            if (changes.fullCalcOnLoad && setFullCalcOnLoad(xml, &patched)) {
                xml = patched;
                changed = true;
            }
            if (changed) replaced.insert(index, xml);
        }
    }

    // ---- the new ZIP, entry by entry in the old order
    ZipWriter zip;
    ZipSink sink(&zip);
    QSet<QString> written;
    const bool hasShared = !book.sharedStringsPart().isEmpty();
    for (int i = 0; i < directory.size(); ++i) {
        const ZipEntry &e = directory[i];
        if (cancel && cancel->load()) return fail(QCoreApplication::translate("jp::office::sheet::Save", "The save was stopped."));
        const QString key = plain(e.name);
        if (entryAt.value(key) != i && written.contains(key)) continue;   // (of two entries of one name the first stands)
        if (dropped.contains(i)) continue;
        written.insert(key);
        if (replaced.contains(i)) {
            zip.add(e.name, replaced.value(i), true);
            rep.changedParts << e.name;
            ++rep.partsChanged;
        } else if (sheetOfEntry.contains(i)) {
            const int sheet = sheetOfEntry.value(i);
            const auto changed = changes.sheets.find(sheet);
            const bool patch = changed != changes.sheets.end() && !changed->second.empty();
            const bool mover = moveSheets.contains(sheet);
            zip.beginStream(e.name, true);
            ZipEntryDevice device(bytes, e);
            bool ok = device.open(QIODevice::ReadOnly);
            if (ok && mover && patch) {
                // (the file's cells are put in the places the edits leave them, and then the cells that changed are put in)
                BufferSink moved;
                ok = moveSheetPart(&device, &moved, ctx, sheet, nullptr, &why, cancel);
                QBuffer in(&moved.bytes);
                ok = ok && in.open(QIODevice::ReadOnly) && patchSheet(&in, &sink, changed->second, hasShared, &why, cancel);
            } else if (ok && mover) {
                ok = moveSheetPart(&device, &sink, ctx, sheet, nullptr, &why, cancel);
            } else if (ok) {
                ok = patchSheet(&device, &sink, changes.sheets.at(sheet), hasShared, &why, cancel);
            }
            if (!ok) {
                zip.endStream();
                return fail(why.isEmpty() ? QCoreApplication::translate("jp::office::sheet::Save", "A sheet of the file could not be read to be saved.") : why);
            }
            zip.endStream();
            rep.changedParts << e.name;
            ++rep.partsChanged;
            ++rep.sheetsRewritten;
        } else {
            if (e.dataAt < 0 || e.dataAt + e.csize > bytes.size()) return fail(QCoreApplication::translate("jp::office::sheet::Save", "The file this workbook was opened from is damaged."));
            zip.addRaw(e.name, e.method, e.crc, quint32(e.csize), quint32(e.usize), bytes.constData() + e.dataAt);
            ++rep.partsCopied;
        }
    }
    for (const auto &[name, data] : newEntries) {
        zip.add(name, data, true);
        rep.changedParts << name;
        ++rep.partsChanged;
    }
    // the sheets that were added, made whole: the skeleton with their cells, sizes, and merged ranges put in
    for (const NewSheet &n : newSheets) {
        if (cancel && cancel->load()) return fail(QCoreApplication::translate("jp::office::sheet::Save", "The save was stopped."));
        const auto found = changes.sheets.find(n.sheet);
        SheetChanges sc;
        if (found != changes.sheets.end()) sc = found->second;
        QByteArray skeleton = sc.skeleton;
        if (skeleton.isEmpty())
            skeleton = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<worksheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\"><sheetData/></worksheet>";
        // a copy takes what the sheet it was copied from has that the program keeps no model of (protection, filter, links, page setup, breaks), moved by the edits made to rows and columns
        QByteArray linkRelationships;
        if (!sc.copyFromPart.isEmpty()) {
            const auto source = package.openPart(sc.copyFromPart);
            SheetExtras extras;
            QString carryWhy;
            if (!source || !readSheetExtras(source.get(), &extras, &carryWhy))
                return fail(carryWhy.isEmpty() ? QCoreApplication::translate("jp::office::sheet::Save", "The sheet that was copied could not be read again.") : carryWhy);
            MoveContext copyCtx = ctx;
            copyCtx.edits = sc.copyEdits;
            copyCtx.edits.insert(copyCtx.edits.end(), ctx.edits.begin(), ctx.edits.end());
            if (!moveSheetExtras(&extras, copyCtx, n.sheet, &carryWhy, cancel)) return fail(carryWhy);
            skeleton = skeletonWithExtras(skeleton, extras, sc.isProtected);
            linkRelationships = hyperlinkRelationshipsXml(package.relationships(sc.copyFromPart));
        } else if (sc.isProtected) {
            skeleton = skeletonWithExtras(skeleton, SheetExtras(), true);
        }
        QBuffer in(&skeleton);
        in.open(QIODevice::ReadOnly);
        zip.beginStream(n.part.startsWith(QLatin1Char('/')) ? n.part.mid(1) : n.part, true);
        const bool ok = patchSheet(&in, &sink, sc, hasShared, &why, cancel);
        zip.endStream();
        if (!ok) return fail(why.isEmpty() ? QCoreApplication::translate("jp::office::sheet::Save", "A sheet that was added could not be made.") : why);
        if (const auto drawingLink = newSheetRels.find(n.sheet); drawingLink != newSheetRels.end()) linkRelationships += drawingLink->second;
        if (!linkRelationships.isEmpty()) {
            // (the copy's links to places outside the workbook: its own relationships, with the ids the links name)
            QString relsOfNew = relationshipsPartOf(n.part);
            if (relsOfNew.startsWith(QLatin1Char('/'))) relsOfNew.remove(0, 1);
            zip.add(relsOfNew, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">" + linkRelationships + "</Relationships>", true);
        }
        rep.changedParts << n.part;
        ++rep.partsChanged;
        ++rep.sheetsRewritten;
    }
    *out = zip.finish();
    return true;
}

bool writeFileAtomically(const QString &path, const QByteArray &bytes, QString *error)
{
    QSaveFile file(path);
    file.setDirectWriteFallback(false);
    if (!file.open(QIODevice::WriteOnly)) {
        if (error) *error = QCoreApplication::translate("jp::office::sheet::Save", "The file couldn't be written: %1").arg(file.errorString());
        return false;
    }
    if (file.write(bytes) != bytes.size() || !file.commit()) {
        if (error) *error = QCoreApplication::translate("jp::office::sheet::Save", "The file couldn't be written: %1").arg(file.errorString());
        return false;
    }
    return true;
}

} // namespace jp::office::sheet
