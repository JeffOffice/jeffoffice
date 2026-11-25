#include "core/contentcontrolops.h"

#include "core/editops.h"
#include "core/pictureops.h"
#include "core/tableops.h"
#include "core/writeprops.h"
#include "text/textprops.h"
#include "core/syslocale.h"

#include <QCoreApplication>
#include <QLocale>
#include <QPointer>
#include <QTextDocumentFragment>
#include <QRandomGenerator>
#include <QHash>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <algorithm>
#include <optional>

using namespace jp::office::wml;

namespace jp::write {

namespace {

const QChar kObject = QChar::ObjectReplacementCharacter;
const char kClosingXml[] = "</w:sdtContent></w:sdt>";
const QColor kPlaceholderColor(0x80, 0x80, 0x80);
const QString kBoxChecked = QString(QChar(0x2612)), kBoxUnchecked = QString(QChar(0x2610));
const QString kSymbolFont = QStringLiteral("Segoe UI Symbol");

QByteArray escaped(const QString &text)
{
    QString out;
    out.reserve(text.size());
    for (const QChar ch : text) {
        const ushort u = ch.unicode();
        if (u < 0x20 && u != 9 && u != 10 && u != 13) continue;   // (no character XML can't hold)
        if (ch == QLatin1Char('&')) out += QLatin1String("&amp;");
        else if (ch == QLatin1Char('<')) out += QLatin1String("&lt;");
        else if (ch == QLatin1Char('>')) out += QLatin1String("&gt;");
        else if (ch == QLatin1Char('"')) out += QLatin1String("&quot;");
        else out += ch;
    }
    return out.toUtf8();
}

QString unescaped(const QString &text)
{
    if (!text.contains(QLatin1Char('&'))) return text;
    QString out;
    out.reserve(text.size());
    for (qsizetype i = 0; i < text.size(); ++i) {
        if (text[i] != QLatin1Char('&')) {
            out += text[i];
            continue;
        }
        const qsizetype semi = text.indexOf(QLatin1Char(';'), i);
        if (semi < 0 || semi - i > 10) {
            out += text[i];
            continue;
        }
        const QString name = text.mid(i + 1, semi - i - 1);
        QString put;
        if (name == QLatin1String("lt")) put = QStringLiteral("<");
        else if (name == QLatin1String("gt")) put = QStringLiteral(">");
        else if (name == QLatin1String("amp")) put = QStringLiteral("&");
        else if (name == QLatin1String("quot")) put = QStringLiteral("\"");
        else if (name == QLatin1String("apos")) put = QStringLiteral("'");
        else if (name.startsWith(QLatin1Char('#'))) {
            bool ok = false;
            const uint code = name.startsWith(QLatin1String("#x")) || name.startsWith(QLatin1String("#X")) ? name.mid(2).toUInt(&ok, 16) : name.mid(1).toUInt(&ok, 10);
            if (ok && code >= 0x20 && code <= 0x10FFFF && !(code >= 0xD800 && code < 0xE000)) put = QString::fromUcs4(reinterpret_cast<const char32_t *>(&code), 1);
        }
        if (put.isNull()) {
            out += text[i];
            continue;
        }
        out += put;
        i = semi;
    }
    return out;
}

// The index of the '>' that ends the tag begun at `from` (a '<'), where a '>' inside a quoted value does not; -1 for none.
qsizetype tagEnd(const QByteArray &s, qsizetype from)
{
    char quote = 0;
    for (qsizetype i = from + 1; i < s.size(); ++i) {
        const char ch = s[i];
        if (quote) {
            if (ch == quote) quote = 0;
        } else if (ch == '"' || ch == '\'') {
            quote = ch;
        } else if (ch == '>') {
            return i;
        }
    }
    return -1;
}

QString localName(const QByteArray &element)
{
    qsizetype i = 1;
    while (i < element.size() && element[i] != ' ' && element[i] != '/' && element[i] != '>' && element[i] != '\t' && element[i] != '\r' && element[i] != '\n') ++i;
    const QByteArray qname = element.mid(1, i - 1);
    return QString::fromLatin1(qname.mid(qname.indexOf(':') + 1));
}

// The attributes of the start tag of an element, in order: the name as written and the value as written (still escaped).
QVector<std::pair<QByteArray, QByteArray>> attributesOf(const QByteArray &element)
{
    QVector<std::pair<QByteArray, QByteArray>> out;
    const qsizetype end = tagEnd(element, 0);
    const QByteArray tag = end < 0 ? element : element.left(end + 1);
    auto space = [](char c) { return c == ' ' || c == '\t' || c == '\r' || c == '\n'; };
    qsizetype i = 1;
    while (i < tag.size() && !space(tag[i]) && tag[i] != '>' && tag[i] != '/') ++i;
    while (i < tag.size() && out.size() < 64) {
        while (i < tag.size() && space(tag[i])) ++i;
        if (i >= tag.size() || tag[i] == '>' || tag[i] == '/') break;
        const qsizetype nameAt = i;
        while (i < tag.size() && !space(tag[i]) && tag[i] != '=' && tag[i] != '>') ++i;
        const QByteArray name = tag.mid(nameAt, i - nameAt);
        while (i < tag.size() && space(tag[i])) ++i;
        if (i >= tag.size() || tag[i] != '=') continue;
        ++i;
        while (i < tag.size() && space(tag[i])) ++i;
        if (i >= tag.size() || (tag[i] != '"' && tag[i] != '\'')) break;
        const char quote = tag[i++];
        const qsizetype valueAt = i;
        while (i < tag.size() && tag[i] != quote) ++i;
        out.push_back({name, tag.mid(valueAt, i - valueAt)});
        ++i;
    }
    return out;
}

QByteArray localOf(const QByteArray &qname) { return qname.mid(qname.indexOf(':') + 1); }

// The value of an attribute of the start tag of an element (by its local name), or empty.
QString attrOf(const QByteArray &element, const char *name, bool *has = nullptr)
{
    if (has) *has = false;
    for (const auto &a : attributesOf(element))
        if (localOf(a.first) == name) {
            if (has) *has = true;
            return unescaped(QString::fromUtf8(a.second));
        }
    return QString();
}

// The start tag of an element, with the attribute `name` (local name) taken out and `put` (the whole attribute, as written) added when it is not empty: what else the tag says stays as it was.
QByteArray tagWithAttribute(const QByteArray &element, const char *name, const QByteArray &put)
{
    QByteArray tag = "<" + element.mid(1, [&] { qsizetype i = 1; while (i < element.size() && element[i] != ' ' && element[i] != '/' && element[i] != '>' && element[i] != '\t' && element[i] != '\r' && element[i] != '\n') ++i; return i - 1; }());
    for (const auto &a : attributesOf(element))
        if (localOf(a.first) != name) tag += " " + a.first + "=\"" + a.second + "\"";
    if (!put.isEmpty()) tag += " " + put;
    return tag;
}

// The elements that are the children of `inner` (the text between an element's tags); false where it is not well formed.
bool childrenOf(const QByteArray &inner, QVector<QByteArray> *out)
{
    qsizetype i = 0;
    while (i < inner.size()) {
        const char ch = inner[i];
        if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') {
            ++i;
            continue;
        }
        if (ch != '<') return false;
        if (inner.mid(i, 4) == "<!--") {
            const qsizetype e = inner.indexOf("-->", i);
            if (e < 0) return false;
            i = e + 3;
            continue;
        }
        const qsizetype e = tagEnd(inner, i);
        if (e < 0 || inner.mid(i, 2) == "</") return false;
        if (inner[e - 1] == '/') {
            out->push_back(inner.mid(i, e - i + 1));
            i = e + 1;
            continue;
        }
        int depth = 1;
        qsizetype j = e + 1;
        while (j < inner.size() && depth > 0) {
            const qsizetype lt = inner.indexOf('<', j);
            if (lt < 0) return false;
            if (inner.mid(lt, 4) == "<!--") {
                const qsizetype c = inner.indexOf("-->", lt);
                if (c < 0) return false;
                j = c + 3;
                continue;
            }
            const qsizetype g = tagEnd(inner, lt);
            if (g < 0) return false;
            if (inner.mid(lt, 2) == "</") --depth;
            else if (inner[g - 1] != '/') ++depth;
            j = g + 1;
        }
        if (depth > 0) return false;
        out->push_back(inner.mid(i, j - i));
        i = j;
    }
    return true;
}

// The start of a control (what its first atom keeps), in pieces: the tag of the control, what comes before its properties, the properties' children, and what comes after.
struct Pieces {
    bool ok = false;
    QByteArray head, tail, prTag;   // (prTag: the start of the properties when it has attributes, such as a namespace the file declares there)
    QVector<QByteArray> kids;
};

Pieces piecesOf(const QByteArray &raw)
{
    Pieces p;
    if (!raw.startsWith("<w:sdt") || raw.size() < 8 || (raw[6] != '>' && raw[6] != ' ')) return p;
    const qsizetype first = tagEnd(raw, 0);
    if (first < 0 || raw[first - 1] == '/') return p;
    const qsizetype pr = first + 1;
    if (raw.mid(pr, 9) == "<w:sdtPr>" || raw.mid(pr, 9) == "<w:sdtPr/") {
        p.head = raw.left(pr);
        if (raw.mid(pr, 10) == "<w:sdtPr/>") {
            p.tail = raw.mid(pr + 10);
        } else if (raw.mid(pr, 9) == "<w:sdtPr>") {
            const qsizetype close = raw.indexOf("</w:sdtPr>", pr);
            if (close < 0) return p;
            if (!childrenOf(raw.mid(pr + 9, close - pr - 9), &p.kids)) return p;
            p.tail = raw.mid(close + 10);
        } else {
            return p;
        }
    } else if (raw.mid(pr, 8) == "<w:sdtPr") {
        const qsizetype e = tagEnd(raw, pr);
        if (e < 0) return p;
        p.head = raw.left(pr);
        if (raw[e - 1] == '/') {
            p.tail = raw.mid(e + 1);
        } else {
            p.prTag = raw.mid(pr, e - pr + 1);
            const qsizetype close = raw.indexOf("</w:sdtPr>", e);
            if (close < 0 || !childrenOf(raw.mid(e + 1, close - e - 1), &p.kids)) return p;
            p.tail = raw.mid(close + 10);
        }
    } else {
        p.head = raw.left(pr);
        p.tail = raw.mid(pr);
    }
    if (p.tail.lastIndexOf("<w:sdtContent") < 0) return p;
    p.ok = true;
    return p;
}

QByteArray valElement(const char *name, const QString &value) { return QByteArray("<") + name + " w:val=\"" + escaped(value) + "\"/>"; }

QString lockValue(const ControlProps &p)
{
    if (p.lockDelete && p.lockContents) return QStringLiteral("sdtContentLocked");
    if (p.lockDelete) return QStringLiteral("sdtLocked");
    if (p.lockContents) return QStringLiteral("contentLocked");
    return QString();
}

QByteArray checkboxElement(const ControlProps &p, const QByteArray &old)
{
    QVector<QByteArray> kids;
    QByteArray checkedState = "<w14:checkedState w14:val=\"2612\" w14:font=\"" + kSymbolFont.toUtf8() + "\"/>";
    QByteArray uncheckedState = "<w14:uncheckedState w14:val=\"2610\" w14:font=\"" + kSymbolFont.toUtf8() + "\"/>";
    if (!old.isEmpty()) {
        const qsizetype s = tagEnd(old, 0);
        if (s > 0 && old[s - 1] != '/') {
            const qsizetype e = old.lastIndexOf("</");
            if (e > s && childrenOf(old.mid(s + 1, e - s - 1), &kids))
                for (const QByteArray &k : kids) {
                    if (localName(k) == QLatin1String("checkedState")) checkedState = k;
                    else if (localName(k) == QLatin1String("uncheckedState")) uncheckedState = k;
                }
        }
    }
    return "<w14:checkbox><w14:checked w14:val=\"" + QByteArray(p.checked ? "1" : "0") + "\"/>" + checkedState + uncheckedState + "</w14:checkbox>";
}

QByteArray dateElement(const ControlProps &p, const QByteArray &old)
{
    QByteArray tag = "<w:date";
    QVector<QByteArray> kids;
    if (!old.isEmpty()) {
        const qsizetype s = tagEnd(old, 0);
        if (s > 0) {
            tag = tagWithAttribute(old, "fullDate", QByteArray());
            if (old[s - 1] != '/' && old.lastIndexOf("</") > s) childrenOf(old.mid(s + 1, old.lastIndexOf("</") - s - 1), &kids);
        }
    }
    if (p.date.isValid()) tag += " w:fullDate=\"" + jp::fixedText(p.date, QStringLiteral("yyyy-MM-dd")).toLatin1() + "T00:00:00Z\"";
    tag += ">";
    QByteArray out = tag;
    bool hasFormat = false, hasLid = false;
    for (const QByteArray &k : kids) {
        if (localName(k) == QLatin1String("dateFormat")) {
            hasFormat = true;
            out += valElement("w:dateFormat", p.dateFormat.isEmpty() ? QStringLiteral("M/d/yyyy") : p.dateFormat);
        } else {
            hasLid = hasLid || localName(k) == QLatin1String("lid");
            out += k;
        }
    }
    if (!hasFormat) out = tag + valElement("w:dateFormat", p.dateFormat.isEmpty() ? QStringLiteral("M/d/yyyy") : p.dateFormat) + out.mid(tag.size());
    if (kids.isEmpty()) out += valElement("w:lid", QStringLiteral("en-US")) + valElement("w:storeMappedDataAs", QStringLiteral("dateTime")) + valElement("w:calendar", QStringLiteral("gregorian"));
    else if (!hasLid) out += valElement("w:lid", QStringLiteral("en-US"));
    return out + "</w:date>";
}

// A repeating section's element: what the file had in it, and whether items may be added and removed.
QByteArray repeatingSectionElement(const ControlProps &p, const QByteArray &old)
{
    QByteArray open = "<w15:repeatingSection", inner;
    if (!old.isEmpty()) {
        const qsizetype s = tagEnd(old, 0), e = old.lastIndexOf("</");
        QVector<QByteArray> kids;
        if (s <= 0 || (old[s - 1] != '/' && e > s && !childrenOf(old.mid(s + 1, e - s - 1), &kids))) return old;
        open = old.left(s);
        if (open.endsWith('/')) open.chop(1);
        for (const QByteArray &k : kids)
            if (localName(k) != QLatin1String("doNotAllowInsertDeleteSection")) inner += k;
    }
    if (p.fixedItems) inner += "<w15:doNotAllowInsertDeleteSection w15:val=\"1\"/>";
    if (inner.isEmpty()) return open.trimmed() + "/>";
    const qsizetype space = open.indexOf(' ');
    return open + ">" + inner + "</" + open.mid(1, space < 0 ? -1 : space - 1) + ">";
}

QByteArray dropDownElement(const ControlProps &p, const QByteArray &old, const char *name = "w:dropDownList")
{
    const QString last = p.lastValue;
    bool known = last.isEmpty();
    QByteArray items;
    int count = 0;
    for (const ControlItem &it : p.items) {
        if (count >= kMostControlItems) break;
        const QString display = it.display.isEmpty() ? it.value : it.display, value = it.value.isEmpty() ? it.display : it.value;
        if (display.isEmpty()) continue;
        known = known || value == last;
        items += "<w:listItem w:displayText=\"" + escaped(display) + "\" w:value=\"" + escaped(value) + "\"/>";
        ++count;
    }
    const QByteArray put = !last.isEmpty() && known ? "w:lastValue=\"" + escaped(last) + "\"" : QByteArray();
    return (old.isEmpty() ? QByteArray("<") + name + (put.isEmpty() ? QByteArray() : " " + put) : tagWithAttribute(old, "lastValue", put)) + ">" + items + "</" + name + ">";
}

// The XML of a control's start: the properties given over what `old` kept (what is not known is in place, as it was).
QByteArray buildStart(const ControlProps &np, bool placeholder, const Pieces &old, bool newId)
{
    QVector<QByteArray> before, others;
    QByteArray idKid, oldType;
    const char *typeName = np.kind == ControlKind::PlainText ? "text" : np.kind == ControlKind::DropDown ? "dropDownList" : np.kind == ControlKind::Date ? "date" : np.kind == ControlKind::CheckBox ? "checkbox" : np.kind == ControlKind::RichText ? "richText" : np.kind == ControlKind::Picture ? "picture" : np.kind == ControlKind::ComboBox ? "comboBox" : np.kind == ControlKind::RepeatingSection ? "repeatingSection" : "";
    for (const QByteArray &k : old.kids) {
        const QString name = localName(k);
        if (name == QLatin1String("rPr")) before.push_back(k);
        else if (name == QLatin1String("alias") || name == QLatin1String("tag") || name == QLatin1String("lock") || name == QLatin1String("showingPlcHdr")) continue;
        else if (name == QLatin1String("id")) idKid = k;
        else if (*typeName && name == QLatin1String(typeName)) oldType = k;
        else others.push_back(k);
    }
    if (idKid.isEmpty() && newId) idKid = valElement("w:id", QString::number(QRandomGenerator::global()->bounded(1, 2147483647)));
    QByteArray pr;
    for (const QByteArray &k : before) pr += k;
    if (!np.title.isEmpty()) pr += valElement("w:alias", np.title);
    if (!np.tag.isEmpty()) pr += valElement("w:tag", np.tag);
    pr += idKid;
    if (const QString lock = lockValue(np); !lock.isEmpty()) pr += valElement("w:lock", lock);
    // (A placeholder, and what else the file kept, then the kind's own element, as the format orders them.)
    for (const QByteArray &k : others)
        if (localName(k) == QLatin1String("placeholder")) pr += k;
    if (placeholder) pr += "<w:showingPlcHdr/>";
    for (const QByteArray &k : others)
        if (localName(k) != QLatin1String("placeholder")) pr += k;
    switch (np.kind) {
    case ControlKind::PlainText: pr += oldType.isEmpty() ? QByteArray("<w:text/>") : oldType; break;
    case ControlKind::DropDown: pr += dropDownElement(np, oldType); break;
    case ControlKind::Date: pr += dateElement(np, oldType); break;
    case ControlKind::CheckBox: pr += checkboxElement(np, oldType); break;
    case ControlKind::RichText: pr += oldType.isEmpty() ? QByteArray("<w:richText/>") : oldType; break;
    case ControlKind::Picture: pr += oldType.isEmpty() ? QByteArray("<w:picture/>") : oldType; break;
    case ControlKind::ComboBox: pr += dropDownElement(np, oldType, "w:comboBox"); break;
    case ControlKind::RepeatingSection: pr += repeatingSectionElement(np, oldType); break;
    case ControlKind::Other: break;
    }
    return old.head + (pr.isEmpty() && old.prTag.isEmpty() ? QByteArray("<w:sdtPr/>") : (old.prTag.isEmpty() ? QByteArray("<w:sdtPr>") : old.prTag) + pr + "</w:sdtPr>") + old.tail;
}

Inline controlAtom(bool open, const QByteArray &raw)
{
    Inline a;
    a.kind = Inline::ParaAtom;
    a.role = open ? Inline::Open : Inline::Close;
    a.tag = QStringLiteral("sdt");
    a.raw = raw;
    return a;
}

QTextCharFormat charFormatAt(QTextDocument *d, int position)
{
    QTextCursor t(d);
    t.setPosition(position + 1);
    return t.charFormat();
}

// The look of ordinary text put at the place: the text before it, or the mark of the paragraph, with nothing a piece kept as XML has.
QTextCharFormat plainLookAt(QTextDocument *d, int position)
{
    QTextCharFormat look;
    if (position <= d->findBlock(position).position()) look = d->findBlock(position).charFormat();
    else look = charFormatAt(d, position - 1);
    look.clearProperty(prop::Atom);
    look.clearProperty(tp::Field);
    look.clearProperty(tp::LinkResult);
    look.clearProperty(tp::InlineObject);
    look.clearProperty(tp::BlankFieldResult);
    look.clearProperty(prop::TrackedDeleted);
    look.clearProperty(tp::Revision);
    look.clearProperty(tp::RevisionAuthor);
    look.clearProperty(tp::RevisionFormat);
    return look;
}

QTextCharFormat atomLook(QTextCharFormat look, const Inline &atom)
{
    look.clearProperty(QTextFormat::FontFamilies);
    setStored(look, prop::Atom, packAtom(atom));
    look.setProperty(tp::Field, QStringLiteral("raw:"));
    return look;
}

QTextCharFormat grayed(QTextCharFormat look)
{
    look.clearProperty(prop::ColorAttrs);
    look.clearProperty(prop::ColorAttrsRgb);
    look.setForeground(QBrush(kPlaceholderColor));
    look.setProperty(tp::ColorRefP, ColorRef::rgb(kPlaceholderColor).toString());
    return look;
}

QTextCharFormat notGrayed(QTextCharFormat look)
{
    look.clearProperty(prop::ColorAttrs);
    look.clearProperty(prop::ColorAttrsRgb);
    look.clearProperty(QTextFormat::ForegroundBrush);
    look.clearProperty(tp::ColorRefP);
    return look;
}

QString placeholderText(ControlKind kind)
{
    switch (kind) {
    case ControlKind::PlainText: return QCoreApplication::translate("jp::write::ContentControls", "Enter text");
    case ControlKind::DropDown: return QCoreApplication::translate("jp::write::ContentControls", "Choose an item");
    case ControlKind::Date: return QCoreApplication::translate("jp::write::ContentControls", "Pick a date");
    case ControlKind::RichText: return QCoreApplication::translate("jp::write::ContentControls", "Enter text");
    case ControlKind::ComboBox: return QCoreApplication::translate("jp::write::ContentControls", "Choose or type an item");
    case ControlKind::Picture: return QCoreApplication::translate("jp::write::ContentControls", "Add a picture");
    default: return QString();
    }
}

QChar glyphOf(const QByteArray &startXml, bool checked)
{
    const Pieces p = piecesOf(startXml);
    for (const QByteArray &k : p.kids)
        if (localName(k) == QLatin1String("checkbox")) {
            QVector<QByteArray> kids;
            const qsizetype s = tagEnd(k, 0), e = k.lastIndexOf("</");
            if (s > 0 && e > s && childrenOf(k.mid(s + 1, e - s - 1), &kids))
                for (const QByteArray &c : kids)
                    if (localName(c) == (checked ? QLatin1String("checkedState") : QLatin1String("uncheckedState"))) {
                        bool ok = false;
                        const uint code = attrOf(c, "val").toUInt(&ok, 16);
                        if (ok && code > 0 && code < 0xFFFF) return QChar(ushort(code));
                    }
        }
    return QChar(checked ? 0x2612 : 0x2610);
}

QString glyphFontOf(const QByteArray &startXml, bool checked)
{
    const Pieces p = piecesOf(startXml);
    for (const QByteArray &k : p.kids)
        if (localName(k) == QLatin1String("checkbox")) {
            QVector<QByteArray> kids;
            const qsizetype s = tagEnd(k, 0), e = k.lastIndexOf("</");
            if (s > 0 && e > s && childrenOf(k.mid(s + 1, e - s - 1), &kids))
                for (const QByteArray &c : kids)
                    if (localName(c) == (checked ? QLatin1String("checkedState") : QLatin1String("uncheckedState"))) {
                        const QString font = attrOf(c, "font");
                        if (!font.isEmpty()) return font;
                    }
        }
    return kSymbolFont;
}

bool chosenOnly(ControlKind kind) { return kind == ControlKind::DropDown || kind == ControlKind::Date || kind == ControlKind::CheckBox || kind == ControlKind::Picture; }

// A control as the atoms of a paragraph say: positions only (the properties are read from the start by whoever needs them).
struct Span {
    int start, end, depth;
    QByteArray raw;
};

QVector<Span> spansOf(const QTextBlock &block, int *budget = nullptr)
{
    int own = kMostControls;   // (a paragraph that holds more is not looked through further)
    if (!budget) budget = &own;
    QVector<Span> out;
    struct Open {
        int at;
        QByteArray raw;
    };
    QVector<Open> open;
    for (QTextBlock::iterator it = block.begin(); !it.atEnd(); ++it) {
        const QTextFragment fr = it.fragment();
        const QTextCharFormat cf = fr.charFormat();
        if (!cf.hasProperty(prop::Atom)) continue;
        const QByteArray bytes = storedBytes(cf, prop::Atom);
        if (!bytes.contains("sdt")) continue;
        const Inline a = unpackAtom(bytes);
        if (a.kind != Inline::ParaAtom || a.tag != QLatin1String("sdt")) continue;
        for (int i = 0; i < fr.length(); ++i) {
            const int at = fr.position() + i;
            if (a.role == Inline::Open) {
                if (open.size() < 4 * kMostControlDepth) open.push_back({at, a.raw});
            } else if (a.role == Inline::Close && !open.isEmpty()) {
                const Open o = open.takeLast();
                const int depth = int(open.size());
                if (depth >= kMostControlDepth) continue;
                if (budget && *budget <= 0) return out;
                if (budget) --*budget;
                out.push_back({o.at, at, depth, o.raw});
            }
        }
    }
    std::sort(out.begin(), out.end(), [](const Span &a, const Span &b) { return a.start < b.start; });
    return out;
}

ContentControl controlOf(const QTextBlock &block, const Span &s)
{
    ContentControl c;
    c.start = s.start;
    c.end = s.end;
    c.depth = s.depth;
    bool ok = false, placeholder = false;
    c.props = controlPropsOf(s.raw, &ok, &placeholder);
    c.editable = ok;
    c.placeholder = placeholder;
    c.text = block.text().mid(s.start + 1 - block.position(), s.end - s.start - 1).remove(kObject);
    return c;
}

// The raw start of the control that begins at `start`, and the atom's place.
bool startOf(QTextDocument *story, int start, Inline *atom)
{
    if (start < 0 || start >= story->characterCount() - 1) return false;
    const QTextCharFormat f = charFormatAt(story, start);
    if (!f.hasProperty(prop::Atom)) return false;
    *atom = unpackAtom(storedBytes(f, prop::Atom));
    return atom->tag == QLatin1String("sdt") && atom->role == Inline::Open;
}

void setStartXml(QTextDocument *story, int start, const QByteArray &raw)
{
    Inline atom;
    if (!startOf(story, start, &atom)) return;
    atom.raw = raw;
    QTextCharFormat nf;
    setStored(nf, prop::Atom, packAtom(atom));
    QTextCursor t(story);
    t.setPosition(start);
    t.setPosition(start + 1, QTextCursor::KeepAnchor);
    t.mergeCharFormat(nf);
}

// Puts `text` between the atoms of the control, in the look the text had (or the placeholder's look), and says in its start whether it shows a placeholder.
void putContents(WriteDoc &doc, const ContentControl &ctl, const QString &text, bool placeholder, const ControlProps &props, const QChar &glyph = QChar(), const QString &glyphFont = QString())
{
    QTextDocument *story = doc.story();
    QTextCursor edit(story);
    edit.beginEditBlock();
    QTextCharFormat look;
    if (ctl.end > ctl.start + 1) {
        look = charFormatAt(story, ctl.start + 1);
        if (look.hasProperty(prop::Atom)) look = plainLookAt(story, ctl.start + 1);
    } else {
        look = plainLookAt(story, ctl.start + 1);
    }
    look.clearProperty(prop::Atom);
    look.clearProperty(tp::Field);
    look = placeholder ? grayed(look) : notGrayed(look);
    if (!glyphFont.isEmpty()) look.setFontFamilies(QStringList{glyphFont});
    // (What is in the control that is not text, a bookmark or a field's mark, stays; the text goes.)
    {
        QVector<QPair<int, int>> gone;
        const QTextBlock b = story->findBlock(ctl.start);
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment fr = it.fragment();
            const int from = std::max(fr.position(), ctl.start + 1), to = std::min(fr.position() + fr.length(), ctl.end);
            if (to > from && !fr.charFormat().hasProperty(prop::Atom)) gone.push_back({from, to});
        }
        for (qsizetype i = gone.size() - 1; i >= 0; --i) {
            edit.setPosition(gone[i].first);
            edit.setPosition(gone[i].second, QTextCursor::KeepAnchor);
            edit.removeSelectedText();
        }
    }
    edit.setPosition(ctl.start + 1);
    const QString shown = glyph.isNull() ? text : QString(glyph);
    if (!shown.isEmpty()) edit.insertText(shown, look);
    Inline atom;
    if (startOf(story, ctl.start, &atom)) {
        const Pieces old = piecesOf(atom.raw);
        if (old.ok) setStartXml(story, ctl.start, buildStart(props, placeholder, old, false));
    }
    edit.endEditBlock();
}

ContentControl innermostAt(const QTextBlock &block, int position)
{
    ContentControl best;
    for (const Span &s : spansOf(block))
        if (position > s.start && position <= s.end && (!best.valid() || s.start > best.start)) best = controlOf(block, s);
    return best;
}

} // namespace

bool ControlProps::operator==(const ControlProps &o) const
{
    if (kind != o.kind || title != o.title || tag != o.tag || lockDelete != o.lockDelete || lockContents != o.lockContents || dateFormat != o.dateFormat || date != o.date || lid != o.lid || checked != o.checked || fixedItems != o.fixedItems || lastValue != o.lastValue || items.size() != o.items.size()) return false;
    for (int i = 0; i < items.size(); ++i)
        if (items[i].display != o.items[i].display || items[i].value != o.items[i].value) return false;
    return true;
}

namespace {
int s_parses = 0;
constexpr qsizetype kMostStartXml = 65536;   // a start tag longer than this is kept as it is and not read
constexpr qsizetype kMostText = 4096;        // a title, tag, or item longer than this makes the control one that is kept and not read

struct Parsed {
    ControlProps props;
    bool ok = false, placeholder = false;
};

Parsed parseProps(const QByteArray &startXml);
} // namespace

int controlParsesForTest() { return s_parses; }

ControlProps controlPropsOf(const QByteArray &startXml, bool *ok, bool *placeholder)
{
    // (What the start of a control says is read once: the same bytes are asked for at every caret move and every paint.)
    static QHash<QByteArray, Parsed> cache;
    auto found = cache.constFind(startXml);
    if (found == cache.constEnd()) {
        if (cache.size() >= 20000) cache.clear();
        found = cache.insert(startXml, parseProps(startXml));
    }
    if (ok) *ok = found->ok;
    if (placeholder) *placeholder = found->placeholder;
    return found->props;
}

namespace {
Parsed parseProps(const QByteArray &startXml)
{
    ++s_parses;
    bool okValue = false, placeholderValue = false;
    bool *ok = &okValue, *placeholder = &placeholderValue;
    ControlProps p;
    p.kind = ControlKind::Other;
    if (ok) *ok = false;
    if (placeholder) *placeholder = false;
    if (startXml.size() > kMostStartXml) return {p, false, false};
    const Pieces pieces = piecesOf(startXml);
    if (!pieces.ok) return {p, false, false};
    bool galleryOrOther = false, typed = false;
    for (const QByteArray &k : pieces.kids) {
        const QString name = localName(k);
        typed = typed || name == QLatin1String("text") || name == QLatin1String("dropDownList") || name == QLatin1String("comboBox") || name == QLatin1String("date") || name == QLatin1String("checkbox") || name == QLatin1String("richText")
                || name == QLatin1String("picture") || name == QLatin1String("repeatingSection");
        if (name == QLatin1String("alias")) {
            p.title = attrOf(k, "val");
        } else if (name == QLatin1String("tag")) {
            p.tag = attrOf(k, "val");
        } else if (name == QLatin1String("lock")) {
            const QString v = attrOf(k, "val");
            p.lockDelete = v == QLatin1String("sdtLocked") || v == QLatin1String("sdtContentLocked");
            p.lockContents = v == QLatin1String("contentLocked") || v == QLatin1String("sdtContentLocked");
        } else if (name == QLatin1String("showingPlcHdr")) {
            if (placeholder) *placeholder = attrOf(k, "val") != QLatin1String("0") && attrOf(k, "val") != QLatin1String("false");
        } else if (name == QLatin1String("text")) {
            p.kind = ControlKind::PlainText;
        } else if (name == QLatin1String("dropDownList") || name == QLatin1String("comboBox")) {
            p.kind = name == QLatin1String("comboBox") ? ControlKind::ComboBox : ControlKind::DropDown;
            p.lastValue = attrOf(k, "lastValue");
            QVector<QByteArray> kids;
            const qsizetype s = tagEnd(k, 0), e = k.lastIndexOf("</");
            if (s > 0 && k[s - 1] != '/' && e > s) {
                if (!childrenOf(k.mid(s + 1, e - s - 1), &kids)) {
                    p.kind = ControlKind::Other;
                } else {
                    for (const QByteArray &item : kids) {
                        if (localName(item) != QLatin1String("listItem")) continue;
                        if (p.items.size() >= kMostControlItems) {
                            p.kind = ControlKind::Other;   // (a list read in part would be written in part: it is left as it is)
                            p.items.clear();
                            break;
                        }
                        p.items.push_back({attrOf(item, "displayText"), attrOf(item, "value")});
                    }
                }
            }
        } else if (name == QLatin1String("date")) {
            p.kind = ControlKind::Date;
            const QString full = attrOf(k, "fullDate");
            p.date = QDate::fromString(full.left(10), QStringLiteral("yyyy-MM-dd"));
            QVector<QByteArray> kids;
            const qsizetype s = tagEnd(k, 0), e = k.lastIndexOf("</");
            if (s > 0 && k[s - 1] != '/' && e > s && childrenOf(k.mid(s + 1, e - s - 1), &kids))
                for (const QByteArray &d : kids)
                    if (localName(d) == QLatin1String("dateFormat")) p.dateFormat = attrOf(d, "val");
                    else if (localName(d) == QLatin1String("lid")) p.lid = attrOf(d, "val");
            if (p.dateFormat.isEmpty()) p.dateFormat = QStringLiteral("M/d/yyyy");
        } else if (name == QLatin1String("checkbox")) {
            p.kind = ControlKind::CheckBox;
            QVector<QByteArray> kids;
            const qsizetype s = tagEnd(k, 0), e = k.lastIndexOf("</");
            if (s > 0 && k[s - 1] != '/' && e > s && childrenOf(k.mid(s + 1, e - s - 1), &kids))
                for (const QByteArray &d : kids)
                    if (localName(d) == QLatin1String("checked")) p.checked = attrOf(d, "val") == QLatin1String("1") || attrOf(d, "val") == QLatin1String("true");
        } else if (name == QLatin1String("repeatingSectionItem")) {
            p.sectionItem = true;
        } else if (name == QLatin1String("richText")) {
            p.kind = ControlKind::RichText;
        } else if (name == QLatin1String("picture")) {
            p.kind = ControlKind::Picture;
        } else if (name == QLatin1String("repeatingSection")) {
            p.kind = ControlKind::RepeatingSection;
            QVector<QByteArray> kids;
            const qsizetype s = tagEnd(k, 0), e = k.lastIndexOf("</");
            if (s > 0 && k[s - 1] != '/' && e > s && childrenOf(k.mid(s + 1, e - s - 1), &kids))
                for (const QByteArray &d : kids)
                    if (localName(d) == QLatin1String("doNotAllowInsertDeleteSection")) {
                        bool has = false;
                        const QString v = attrOf(d, "val", &has);
                        p.fixedItems = !has || (v != QLatin1String("0") && v != QLatin1String("false"));
                    }
        } else if (name == QLatin1String("docPartObj") || name == QLatin1String("docPartList")
                   || name == QLatin1String("group") || name == QLatin1String("citation") || name == QLatin1String("bibliography") || name == QLatin1String("equation")) {
            galleryOrOther = true;
        }
    }
    if (galleryOrOther) p.kind = ControlKind::Other;
    else if (!typed && !p.sectionItem) p.kind = ControlKind::RichText;   // (a control that names no kind holds formatted text)
    bool tooLong = p.title.size() > kMostText || p.tag.size() > kMostText;
    for (const ControlItem &it : p.items) tooLong = tooLong || it.display.size() > kMostText || it.value.size() > kMostText;
    if (tooLong) {
        ControlProps kept;
        kept.kind = ControlKind::Other;
        kept.lockDelete = p.lockDelete;
        kept.lockContents = p.lockContents;
        return {kept, false, placeholderValue};
    }
    return {p, true, placeholderValue};
}
} // namespace

QVector<ContentControl> contentControlsOf(const QTextBlock &block)
{
    QVector<ContentControl> out;
    for (const Span &s : spansOf(block)) out.push_back(controlOf(block, s));
    return out;
}

QVector<ContentControl> contentControls(WriteDoc &doc)
{
    QVector<ContentControl> out;
    int budget = kMostControls;
    for (QTextBlock b = doc.story()->begin(); b.isValid() && budget > 0; b = b.next())
        for (const Span &s : spansOf(b, &budget)) out.push_back(controlOf(b, s));
    return out;
}

ContentControl contentControlAt(WriteDoc &doc, int position, QTextDocument *story)
{
    if (!story) story = doc.story();
    if (position < 0 || position >= story->characterCount()) return ContentControl();
    return innermostAt(story->findBlock(position), position);
}

bool insertContentControl(WriteDoc &doc, QTextCursor &c, ControlKind kind, QString *problem)
{
    QTextDocument *story = doc.story();
    if (kind == ControlKind::RepeatingSection) return insertBlockControl(doc, c, kind, problem);
    if (kind == ControlKind::RichText && c.document() == story) {
        // (Over paragraphs, or in a paragraph with nothing in it, a rich text control holds paragraphs; elsewhere it holds the words of one.)
        const QTextBlock first = story->findBlock(c.selectionStart()), last = story->findBlock(c.hasSelection() ? c.selectionEnd() - 1 : c.position());
        if (first != last || (!c.hasSelection() && first.text().isEmpty() && first.length() == 1 && !isLocked(first))) return insertBlockControl(doc, c, kind, problem);
    }
    if (c.document() != story || kind == ControlKind::Other) {
        if (problem) *problem = QCoreApplication::translate("jp::write::ContentControls", "A content control goes in the text, not in a header, a footer, or a note.");
        return false;
    }
    int from = c.position(), to = c.position();
    if (c.hasSelection()) {
        const int a = c.selectionStart(), b = c.selectionEnd();
        const bool oneParagraph = story->findBlock(a) == story->findBlock(b > a ? b - 1 : b);
        if ((kind == ControlKind::PlainText || kind == ControlKind::RichText) && oneParagraph) {
            from = a;
            to = b;
        } else {
            from = to = b;
        }
    }
    if (isLocked(story->findBlock(from))) return false;
    if (splitsContentControl(story, from, to)) {
        if (problem) *problem = QCoreApplication::translate("jp::write::ContentControls", "The selection holds only part of another content control.");
        return false;
    }
    if (int(contentControls(doc).size()) >= kMostControls) {
        if (problem) *problem = QCoreApplication::translate("jp::write::ContentControls", "The document has the most content controls it can hold.");
        return false;
    }
    const ContentControl outer = contentControlAt(doc, from, story);
    if (outer.valid() && outer.depth + 1 >= kMostControlDepth) {
        if (problem) *problem = QCoreApplication::translate("jp::write::ContentControls", "Content controls can hold each other only so deep.");
        return false;
    }
    if (!contentControlsAllow(doc, story, from, to, true)) {
        if (problem) *problem = QCoreApplication::translate("jp::write::ContentControls", "The content control there can't be changed.");
        return false;
    }
    ControlProps props;
    props.kind = kind;
    if (kind == ControlKind::Date) props.dateFormat = QStringLiteral("M/d/yyyy");
    const bool wrap = to > from;
    const bool placeholder = !wrap && kind != ControlKind::CheckBox;
    Pieces blank;
    blank.head = "<w:sdt>";
    blank.tail = "<w:sdtContent>";
    blank.ok = true;
    const QByteArray open = buildStart(props, placeholder, blank, true);
    const WriteDoc::StepGroup group(doc);
    EditScope scope(doc, story);
    QTextCursor edit(story);
    edit.beginEditBlock();
    const QTextCharFormat look = plainLookAt(story, from);
    edit.setPosition(to);
    edit.insertText(QString(kObject), atomLook(look, controlAtom(false, kClosingXml)));
    if (!wrap) {
        edit.setPosition(to);
        QTextCharFormat content = look;
        if (kind == ControlKind::CheckBox) {
            content.setFontFamilies(QStringList{kSymbolFont});
            edit.insertText(kBoxUnchecked, content);
        } else {
            edit.insertText(placeholderText(kind), grayed(content));
        }
    }
    edit.setPosition(from);
    edit.insertText(QString(kObject), atomLook(look, controlAtom(true, open)));
    edit.endEditBlock();
    const int end = from + 1 + (wrap ? to - from : int(kind == ControlKind::CheckBox ? 1 : placeholderText(kind).size()));
    if (placeholder) {
        c.setPosition(from + 1);
        c.setPosition(end, QTextCursor::KeepAnchor);
    } else {
        c.setPosition(end + 1);
    }
    return true;
}

bool setContentControlProps(WriteDoc &doc, int start, const ControlProps &props, QString *problem)
{
    QTextDocument *story = doc.story();
    Inline atom;
    if (!startOf(story, start, &atom)) return false;
    bool ok = false, placeholder = false;
    const ControlProps old = controlPropsOf(atom.raw, &ok, &placeholder);
    if (!ok || old.kind != props.kind) return false;
    if (props.items.size() > kMostControlItems) {
        if (problem) *problem = QCoreApplication::translate("jp::write::ContentControls", "A list holds at most %1 items.").arg(kMostControlItems);
        return false;
    }
    if (props == old) return true;
    ControlProps np = props;
    const Pieces pieces = piecesOf(atom.raw);
    if (!pieces.ok) return false;
    const WriteDoc::StepGroup group(doc);
    if (np.kind == ControlKind::Date && np.date.isValid() && !placeholder && np.dateFormat != old.dateFormat) {
        // (The date shown is written again in the new format.)
        const ContentControl ctl = contentControlAt(doc, start + 1, story);
        if (ctl.valid() && ctl.start == start) {
            putContents(doc, ctl, formatControlDate(np.date, np.dateFormat, np.lid), false, np);
            return true;
        }
    }
    setStartXml(story, start, buildStart(np, false || placeholder, pieces, false));
    return true;
}

bool chooseControlItem(WriteDoc &doc, int start, int index)
{
    QTextDocument *story = doc.story();
    const ContentControl ctl = contentControlAt(doc, start + 1, story);
    if (!ctl.valid() || ctl.start != start || (ctl.props.kind != ControlKind::DropDown && ctl.props.kind != ControlKind::ComboBox) || !ctl.editable || index < 0 || index >= ctl.props.items.size()) return false;
    ControlProps np = ctl.props;
    const ControlItem item = np.items[index];
    np.lastValue = item.value.isEmpty() ? item.display : item.value;
    const WriteDoc::StepGroup group(doc);
    putContents(doc, ctl, item.display.isEmpty() ? item.value : item.display, false, np);
    return true;
}

bool setControlDate(WriteDoc &doc, int start, const QDate &date)
{
    QTextDocument *story = doc.story();
    const ContentControl ctl = contentControlAt(doc, start + 1, story);
    if (!ctl.valid() || ctl.start != start || ctl.props.kind != ControlKind::Date || !ctl.editable) return false;
    ControlProps np = ctl.props;
    np.date = date;
    const WriteDoc::StepGroup group(doc);
    putContents(doc, ctl, date.isValid() ? formatControlDate(date, np.dateFormat, np.lid) : placeholderText(ControlKind::Date), !date.isValid(), np);
    return true;
}

bool setControlChecked(WriteDoc &doc, int start, bool checked)
{
    QTextDocument *story = doc.story();
    const ContentControl ctl = contentControlAt(doc, start + 1, story);
    if (!ctl.valid() || ctl.start != start || ctl.props.kind != ControlKind::CheckBox || !ctl.editable) return false;
    Inline atom;
    if (!startOf(story, start, &atom)) return false;
    ControlProps np = ctl.props;
    np.checked = checked;
    const WriteDoc::StepGroup group(doc);
    putContents(doc, ctl, QString(), false, np, glyphOf(atom.raw, checked), glyphFontOf(atom.raw, checked));
    return true;
}

bool toggleControlCheck(WriteDoc &doc, int start)
{
    const ContentControl ctl = contentControlAt(doc, start + 1);
    if (!ctl.valid() || ctl.start != start || ctl.props.kind != ControlKind::CheckBox) return false;
    return setControlChecked(doc, start, !ctl.props.checked);
}

void selectControlContents(QTextCursor &c, const ContentControl &control)
{
    c.setPosition(control.start + 1);
    c.setPosition(control.end, QTextCursor::KeepAnchor);
}

QString formatControlDate(const QDate &date, const QString &format, const QString &lid)
{
    QLocale locale;
    if (!lid.isEmpty()) {
        const QLocale named(QString(lid).replace(QLatin1Char('-'), QLatin1Char('_')));
        if (named.language() != QLocale::C) locale = named;
    }
    return locale.toString(date, format.isEmpty() ? QStringLiteral("M/d/yyyy") : format);
}

int controlItemShown(const ContentControl &control)
{
    if ((control.props.kind != ControlKind::DropDown && control.props.kind != ControlKind::ComboBox) || control.placeholder) return -1;
    for (int i = 0; i < control.props.items.size(); ++i) {
        const ControlItem &it = control.props.items[i];
        if ((it.display.isEmpty() ? it.value : it.display) == control.text) return i;
    }
    return -1;
}

bool splitsContentControl(QTextDocument *story, int from, int to)
{
    if (!story || to <= from) return false;
    for (QTextBlock b = story->findBlock(from); b.isValid() && b.position() < to; b = b.next())
        for (const Span &s : spansOf(b)) {
            const bool a = from <= s.start && s.start < to, z = from <= s.end && s.end < to;
            if (a != z) return true;
        }
    return false;
}

bool restoreEmptyPlaceholder(WriteDoc &doc, int position)
{
    const ContentControl ctl = contentControlAt(doc, position);
    if (!ctl.valid() || !ctl.editable || (ctl.props.kind != ControlKind::PlainText && ctl.props.kind != ControlKind::RichText && ctl.props.kind != ControlKind::ComboBox) || ctl.placeholder || ctl.end != ctl.start + 1) return false;
    const WriteDoc::StepGroup group(doc);
    putContents(doc, ctl, placeholderText(ctl.props.kind), true, ctl.props);
    return true;
}

namespace {

struct BlockSpan {
    int first = 0, last = 0;   // the places of its first and last paragraph
    int openIndex = -1;        // which of the first paragraph's kept pieces opens it
    int depth = 0;
    bool rowish = false;       // it holds rows or cells (the table's own structure), not paragraphs: not a control to edit
    QByteArray raw;
};

QVector<Block> keptOf(const QTextBlock &b, int key)
{
    return b.isValid() && b.blockFormat().hasProperty(key) ? unpackBlocks(storedBytes(b.blockFormat(), key)) : QVector<Block>();
}

void setKept(QTextBlock b, int key, const QVector<Block> &list)
{
    QTextBlockFormat nf = b.blockFormat();
    if (list.isEmpty()) nf.clearProperty(key);
    else setStored(nf, key, packBlocks(list));
    QTextCursor c(b);
    c.setBlockFormat(nf);
}

bool isRowTag(const QString &tag) { return tag == QLatin1String("tr") || tag == QLatin1String("tc"); }

QVector<BlockSpan> scanBlockSpans(QTextDocument *story)
{
    struct Open {
        int block, index;
        bool rowish;
        QByteArray raw;
    };
    struct Item {
        const Block *k;   // null: the paragraph itself
        bool beforeParagraph;
        int index;
    };
    QVector<Open> stack;
    QVector<BlockSpan> out;
    int budget = kMostControls;
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
        const QTextBlockFormat bf = b.blockFormat();
        if (!bf.hasProperty(prop::RawBefore) && !bf.hasProperty(prop::RawAfter)) continue;
        const QVector<Block> before = keptOf(b, prop::RawBefore), after = keptOf(b, prop::RawAfter);
        QVector<Item> seq;   // (what is kept before, the paragraph, and what is kept after, in the order the file has them)
        for (int i = 0; i < before.size(); ++i) seq.push_back({&before[i], true, i});
        seq.push_back({nullptr, false, -1});
        for (int i = 0; i < after.size(); ++i) seq.push_back({&after[i], false, i});
        for (qsizetype s = 0; s < seq.size(); ++s) {
            const Item &it = seq[s];
            if (!it.k || it.k->tag != QLatin1String("sdt")) continue;
            if (it.k->role == Inline::Open) {
                const bool rowish = s + 1 < seq.size() && seq[s + 1].k && seq[s + 1].k->role == Inline::Open && isRowTag(seq[s + 1].k->tag);
                const QTextBlock at = it.beforeParagraph ? b : b.next();
                if (stack.size() < 4 * kMostControlDepth) stack.push_back({at.isValid() ? at.position() : -1, it.beforeParagraph ? it.index : -1, rowish, it.k->raw});
            } else if (it.k->role == Inline::Close && !stack.isEmpty()) {
                const Open o = stack.takeLast();
                const QTextBlock last = it.beforeParagraph ? b.previous() : b;
                const int depth = int(stack.size());
                if (o.block < 0 || !last.isValid() || last.position() < o.block || depth >= kMostControlDepth || budget <= 0) continue;
                --budget;
                const bool closesRow = s > 0 && seq[s - 1].k && seq[s - 1].k->role == Inline::Close && isRowTag(seq[s - 1].k->tag);
                BlockSpan span;
                span.first = o.block;
                span.last = last.position();
                span.openIndex = o.index;
                span.depth = depth;
                span.rowish = o.rowish || closesRow;
                span.raw = o.raw;
                out.push_back(span);
            }
        }
    }
    std::sort(out.begin(), out.end(), [](const BlockSpan &a, const BlockSpan &z) { return a.first != z.first ? a.first < z.first : a.depth < z.depth; });
    return out;
}

// The block controls of a story, found again after any change to it.
struct BlockWatch : QObject {
    QPointer<QTextDocument> story;
    QVector<BlockSpan> spans;
    bool fresh = false;
    QMetaObject::Connection link;
};

const QVector<BlockSpan> &blockSpans(QTextDocument *story)
{
    static BlockWatch watch;
    if (watch.story != story) {
        QObject::disconnect(watch.link);
        watch.story = story;
        watch.fresh = false;
        watch.link = QObject::connect(story, &QTextDocument::contentsChanged, &watch, [] { watch.fresh = false; });
    }
    if (!watch.fresh) {
        watch.spans = scanBlockSpans(story);
        watch.fresh = true;
    }
    return watch.spans;
}

ContentControl blockControlOf(QTextDocument *story, const BlockSpan &s)
{
    ContentControl c;
    c.block = true;
    c.start = s.first;
    const QTextBlock last = story->findBlock(s.last);
    c.end = last.position() + last.length() - 1;
    c.depth = s.depth;
    c.openIndex = s.openIndex;
    bool ok = false, placeholder = false;
    c.props = controlPropsOf(s.raw, &ok, &placeholder);
    c.placeholder = placeholder;
    c.editable = ok && !s.rowish && s.openIndex >= 0 && !inTable(story->findBlock(s.first)) && !inTable(last);
    if (s.rowish) c.props.kind = ControlKind::Other;
    return c;
}

} // namespace

bool contentControlsAllow(WriteDoc &doc, QTextDocument *story, int from, int to, bool typing)
{
    Q_UNUSED(doc);
    return contentControlsAllowIn(story, from, to, typing);
}

bool contentControlsAllowIn(QTextDocument *story, int from, int to, bool typing)
{
    if (!story || from < 0) return true;
    if (to < from) std::swap(from, to);
    const int last = std::max(from, to);
    for (QTextBlock b = story->findBlock(from); b.isValid() && b.position() <= last; b = b.next()) {
        for (const Span &s : spansOf(b)) {
            const bool coversOpen = from <= s.start && s.start < to, coversClose = from <= s.end && s.end < to;
            const int cFrom = std::max(from, s.start + 1), cTo = std::min(to, s.end);
            const bool intoContents = cFrom < cTo || (typing && from > s.start && from <= s.end && !coversOpen && !coversClose);
            if (!coversOpen && !coversClose && !intoContents) continue;
            const ControlProps props = controlPropsOf(s.raw);
            if ((coversOpen || coversClose) && props.lockDelete) return false;
            if (coversOpen != coversClose) return false;   // (half a control would be left: its two ends go together or stay)
            if (coversOpen && coversClose) continue;
            if (intoContents && (props.lockContents || chosenOnly(props.kind))) return false;
        }
    }
    // (A control round whole paragraphs: its lock on the paragraphs' words, and on all of them taken out together.)
    for (const BlockSpan &s : blockSpans(story)) {
        const QTextBlock lastBlock = story->findBlock(s.last);
        const int sEnd = lastBlock.position() + lastBlock.length() - 1;
        if (s.first > last || sEnd < from || s.rowish) continue;
        const ControlProps props = controlPropsOf(s.raw);
        // (A break taken out at either edge moves text into or out of the control, or changes which paragraphs it holds: refused where it can't be deleted or its contents can't be edited.)
        const int breakBefore = s.first - 1;
        const bool crossesStart = breakBefore >= 0 && from <= breakBefore && breakBefore < to, crossesEnd = from <= sEnd && sEnd < to && lastBlock.next().isValid();
        if ((crossesStart || crossesEnd) && (props.lockDelete || props.lockContents)) return false;
        const bool covers = to > from && from <= s.first && to >= sEnd;
        if (props.lockDelete && covers) return false;
        const bool touches = (to > from && to > s.first && from < sEnd) || (typing && from >= s.first && from <= sEnd);
        if (props.lockContents && touches && !covers) return false;
    }
    return true;
}

QVector<std::pair<int, int>> lockedContentRanges(QTextDocument *story, bool withEdges)
{
    QVector<std::pair<int, int>> out;
    if (!story) return out;
    int budget = kMostControls;
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next())
        for (const Span &s : spansOf(b, &budget)) {
            const ControlProps props = controlPropsOf(s.raw);
            if (props.lockContents || chosenOnly(props.kind)) out.push_back(withEdges ? std::make_pair(s.start, s.end + 1) : std::make_pair(s.start + 1, s.end));
        }
    for (const BlockSpan &s : blockSpans(story)) {
        if (s.rowish || !controlPropsOf(s.raw).lockContents) continue;
        const QTextBlock last = story->findBlock(s.last);
        const int sEnd = last.position() + last.length() - 1;
        out.push_back(withEdges ? std::make_pair(std::max(0, s.first - 1), sEnd + 1) : std::make_pair(s.first, sEnd));
    }
    std::sort(out.begin(), out.end());
    QVector<std::pair<int, int>> merged;
    for (const auto &r : std::as_const(out)) {
        if (r.second <= r.first) continue;
        if (!merged.isEmpty() && r.first <= merged.last().second) merged.last().second = std::max(merged.last().second, r.second);
        else merged.push_back(r);
    }
    return merged;
}

bool overlapsRanges(const QVector<std::pair<int, int>> &ranges, int from, int to)
{
    if (to <= from) to = from + 1;
    const auto it = std::lower_bound(ranges.begin(), ranges.end(), from, [](const std::pair<int, int> &r, int at) { return r.second <= at; });
    return it != ranges.end() && it->first < to;
}

bool contentControlsAllowFormat(QTextDocument *story, int from, int to)
{
    if (!story || to <= from) return true;
    return !overlapsRanges(lockedContentRanges(story, false), from, to);
}

bool beginJoinAtEdge(WriteDoc &doc, const QTextBlock &a, bool inSecond, EdgeJoin *join)
{
    Q_UNUSED(doc);
    const QTextBlock b = a.next();
    QTextDocument *story = const_cast<QTextDocument *>(a.document());
    if (!join || !b.isValid()) return true;
    *join = EdgeJoin();
    const QVector<Block> between = keptOf(a, prop::RawAfter) + keptOf(b, prop::RawBefore);
    bool controls = false;
    for (const Block &k : between) controls |= k.tag == QLatin1String("sdt");
    if (!controls) return true;
    // (A control with one paragraph is left with none when that paragraph leaves it: where the caret's side would do that, the joined paragraph is the control's instead.)
    bool aAlone = false, bAlone = false;
    for (const BlockSpan &s : blockSpans(story)) {
        if (s.rowish || s.first != s.last) continue;
        aAlone |= s.first == a.position();
        bAlone |= s.first == b.position();
    }
    if (aAlone && bAlone) return false;   // (each paragraph is all of a control: the pieces between them are the two controls' own)
    const bool before = aAlone ? false : bAlone ? true : inSecond;
    join->story = story;
    join->position = a.position();
    join->blocks = story->blockCount();
    join->moved = before ? int(between.size()) : 0;
    return true;
}

void endJoinAtEdge(const EdgeJoin &join)
{
    if (!join.story || join.moved <= 0 || join.story->blockCount() != join.blocks - 1) return;
    const QTextBlock m = join.story->findBlock(join.position);
    QVector<Block> before = keptOf(m, prop::RawBefore), after = keptOf(m, prop::RawAfter);
    if (after.size() < join.moved) return;
    before += after.mid(0, join.moved);
    after.remove(0, join.moved);
    setKept(m, prop::RawBefore, before);
    setKept(m, prop::RawAfter, after);
}

QVector<Block> controlPiecesOf(const QTextBlock &b, bool *only)
{
    QVector<Block> pieces = keptOf(b, prop::RawBefore);
    pieces += keptOf(b, prop::RawAfter);
    if (only) {
        *only = !pieces.isEmpty();
        for (const Block &k : pieces) *only &= k.tag == QLatin1String("sdt");
    }
    return pieces;
}

void appendKeptAfter(const QTextBlock &b, const QVector<Block> &pieces)
{
    if (!b.isValid() || pieces.isEmpty()) return;
    setKept(b, prop::RawAfter, keptOf(b, prop::RawAfter) + pieces);
}

// What a gap between two paragraphs keeps, with each control that nothing is between the start and the end of taken out (and the ones that holds the same way, once those are gone).
// `keep` holds the starts of the controls that were empty before: those stay. `found`, when given, gets the starts of the ones that are empty.
static int emptyControlsIn(QTextDocument *story, const QSet<QByteArray> *keep, QSet<QByteArray> *found, bool change)
{
    struct Piece {
        Block k;
        bool fromBefore;   // (kept before the paragraph after the gap, not after the one before it)
    };
    int dropped = 0;
    auto tidy = [&](const QTextBlock &z, const QTextBlock &next) {
        const QVector<Block> after = z.isValid() ? keptOf(z, prop::RawAfter) : QVector<Block>(), before = next.isValid() ? keptOf(next, prop::RawBefore) : QVector<Block>();
        QVector<Piece> kept;
        int gone = 0;
        auto add = [&](const QVector<Block> &list, bool fromBefore) {
            for (const Block &k : list) {
                if (!kept.isEmpty() && k.tag == QLatin1String("sdt") && k.role == Inline::Close && kept.last().k.tag == QLatin1String("sdt") && kept.last().k.role == Inline::Open) {
                    if (found) found->insert(kept.last().k.raw);
                    if (keep && keep->contains(kept.last().k.raw)) {
                        kept.push_back({k, fromBefore});
                        continue;
                    }
                    kept.removeLast();
                    gone += 2;
                } else {
                    kept.push_back({k, fromBefore});
                }
            }
        };
        add(after, false);
        add(before, true);
        if (!gone || !change) return;
        QVector<Block> a, b;
        for (const Piece &p : std::as_const(kept)) (p.fromBefore ? b : a).push_back(p.k);
        if (z.isValid()) setKept(z, prop::RawAfter, a);
        if (next.isValid()) setKept(next, prop::RawBefore, b);
        dropped += gone / 2;
    };
    tidy(QTextBlock(), story->begin());
    for (QTextBlock z = story->begin(); z.isValid(); z = z.next()) {
        const QTextBlock next = z.next();
        if (z.blockFormat().hasProperty(prop::RawAfter) || (next.isValid() && next.blockFormat().hasProperty(prop::RawBefore))) tidy(z, next);
    }
    return dropped;
}

QSet<QByteArray> emptyControlStarts(QTextDocument *story)
{
    QSet<QByteArray> found;
    emptyControlsIn(story, nullptr, &found, false);
    return found;
}

int dropEmptyControls(QTextDocument *story, const QSet<QByteArray> &wasEmpty) { return emptyControlsIn(story, &wasEmpty, nullptr, true); }

// The control whose placeholder what is typed at the caret takes the place of (not valid where there is none).
static ContentControl placeholderControlFor(WriteDoc &doc, const QTextCursor &c)
{
    QTextDocument *story = c.document();
    if (story != doc.story()) return ContentControl();
    const int at = c.hasSelection() ? c.selectionStart() : c.position();
    const ContentControl ctl = contentControlAt(doc, c.hasSelection() ? at + 1 : at, story);
    if (!ctl.valid() || !ctl.placeholder || !ctl.editable || (ctl.props.kind != ControlKind::PlainText && ctl.props.kind != ControlKind::RichText && ctl.props.kind != ControlKind::ComboBox) || ctl.props.lockContents) return ContentControl();
    if (c.hasSelection() && (c.selectionStart() < ctl.start + 1 || c.selectionEnd() > ctl.end)) return ContentControl();
    return ctl;
}

bool typingReplacesPlaceholder(WriteDoc &doc, const QTextCursor &c) { return placeholderControlFor(doc, c).valid(); }

void prepareTypingInControl(WriteDoc &doc, QTextCursor &c)
{
    const ContentControl ctl = placeholderControlFor(doc, c);
    if (!ctl.valid()) return;
    QTextDocument *story = c.document();
    selectControlContents(c, ctl);
    Inline atom;
    if (!startOf(story, ctl.start, &atom)) return;
    const Pieces pieces = piecesOf(atom.raw);
    if (!pieces.ok) return;
    setStartXml(story, ctl.start, buildStart(ctl.props, false, pieces, false));
}

bool caretInKnownControl(WriteDoc &doc, const QTextCursor &c)
{
    if (c.document() != doc.story()) return false;
    const ContentControl ctl = contentControlAt(doc, c.position(), c.document());
    return ctl.valid() && ctl.props.kind != ControlKind::Other && ctl.props.kind != ControlKind::RichText;   // (an Enter in a rich text control splits it, as it splits a link: both halves keep it)
}

// ---- picture controls

namespace {
void removeContentsBetween(QTextDocument *story, int from, int to)
{
    // (What is in the control that is not text, a bookmark or a field's mark, stays; the rest goes.)
    QVector<QPair<int, int>> gone;
    const QTextBlock b = story->findBlock(from);
    for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
        const QTextFragment fr = it.fragment();
        const int a = std::max(fr.position(), from), z = std::min(fr.position() + fr.length(), to);
        if (z > a && !fr.charFormat().hasProperty(prop::Atom)) gone.push_back({a, z});
    }
    QTextCursor edit(story);
    for (qsizetype i = gone.size() - 1; i >= 0; --i) {
        edit.setPosition(gone[i].first);
        edit.setPosition(gone[i].second, QTextCursor::KeepAnchor);
        edit.removeSelectedText();
    }
}
constexpr double kPicturePlaceholderSide = 144;   // the box a first picture is fitted in: two inches
} // namespace

PictureResult setControlPicture(WriteDoc &doc, int start, const QByteArray &bytes, const QString &name)
{
    QTextDocument *story = doc.story();
    const ContentControl ctl = contentControlAt(doc, start + 1, story);
    if (!ctl.valid() || ctl.start != start || ctl.props.kind != ControlKind::Picture || !ctl.editable || ctl.props.lockContents) return PictureResult::Refused;
    // (A picture put in the place of another takes the size the first had, kept in proportion; the first takes two inches.)
    double fitW = kPicturePlaceholderSide, fitH = kPicturePlaceholderSide;
    if (!ctl.placeholder) {
        const PictureInfo old = pictureAt(doc, story, ctl.start + 1);
        if (old.valid && old.width > 0 && old.height > 0) {
            fitW = old.width;
            fitH = old.height;
        }
    }
    Inline atom;
    if (!startOf(story, ctl.start, &atom)) return PictureResult::Refused;
    const Pieces pieces = piecesOf(atom.raw);
    if (!pieces.ok) return PictureResult::Refused;
    const WriteDoc::StepGroup group(doc);
    EditScope scope(doc, story);
    QTextCursor edit(story);
    edit.beginEditBlock();
    edit.setPosition(ctl.start + 1);
    const PictureResult made = insertPicture(doc, edit, bytes, name);
    if (made != PictureResult::Ok) {
        edit.endEditBlock();
        return made;
    }
    const ContentControl now = contentControlAt(doc, start + 1, story);
    if (now.valid() && now.start == start) removeContentsBetween(story, ctl.start + 2, now.end);
    const PictureInfo shown = pictureAt(doc, story, ctl.start + 1);
    if (shown.valid && shown.width > 0 && shown.height > 0) {
        const double scale = std::min(fitW / shown.width, fitH / shown.height);
        double w = shown.width * scale, h = shown.height * scale;
        clampKeepingRatio(&w, &h);
        setPictureSize(doc, ctl.start + 1, w, h);
    }
    setStartXml(story, ctl.start, buildStart(ctl.props, false, pieces, false));
    edit.endEditBlock();
    return PictureResult::Ok;
}

bool clearControlPicture(WriteDoc &doc, int start)
{
    QTextDocument *story = doc.story();
    const ContentControl ctl = contentControlAt(doc, start + 1, story);
    if (!ctl.valid() || ctl.start != start || ctl.props.kind != ControlKind::Picture || !ctl.editable || ctl.props.lockContents || ctl.placeholder) return false;
    const WriteDoc::StepGroup group(doc);
    putContents(doc, ctl, placeholderText(ControlKind::Picture), true, ctl.props);
    return true;
}

// ---- block controls: a control round whole paragraphs (and what they hold: lists, tables, other controls), kept as the pieces that open it before its first paragraph and close it after its last

QVector<ContentControl> blockControls(WriteDoc &doc, QTextDocument *story)
{
    if (!story) story = doc.story();
    QVector<ContentControl> out;
    for (const BlockSpan &s : blockSpans(story)) out.push_back(blockControlOf(story, s));
    return out;
}

ContentControl blockControlAt(WriteDoc &doc, int position, QTextDocument *story)
{
    if (!story) story = doc.story();
    ContentControl best;
    for (const BlockSpan &s : blockSpans(story)) {
        const ContentControl c = blockControlOf(story, s);
        if (position >= c.start && position <= c.end && (!best.valid() || c.depth >= best.depth)) best = c;
    }
    return best;
}

ContentControl controlAtCaret(WriteDoc &doc, const QTextCursor &c)
{
    QTextDocument *story = c.document();
    if (story != doc.story()) return ContentControl();
    const int at = c.hasSelection() ? c.selectionStart() + 1 : c.position();
    const ContentControl inner = contentControlAt(doc, at, story);
    if (inner.valid()) return inner;
    const int place = c.hasSelection() ? c.selectionStart() : c.position();
    ContentControl block = blockControlAt(doc, place, story);
    ContentControl section;
    if (block.props.sectionItem && repeatingItemAt(doc, place, &section, story).valid()) return section;   // (Properties of an item are the section's: its title, its tag, whether items may be added)
    return block;
}

namespace {
QByteArray newIdXml() { return "<w:id w:val=\"" + QByteArray::number(QRandomGenerator::global()->bounded(1, 2147483647)) + "\"/>"; }

// Puts a new id in the start of a control, whatever it had.
QByteArray withNewId(const QByteArray &raw)
{
    const Pieces p = piecesOf(raw);
    if (!p.ok) return raw;
    QByteArray pr;
    bool placed = false;
    for (const QByteArray &k : p.kids) {
        if (localName(k) == QLatin1String("id")) {
            pr += newIdXml();
            placed = true;
        } else {
            pr += k;
        }
    }
    if (!placed) pr += newIdXml();
    return p.head + (p.prTag.isEmpty() ? QByteArray("<w:sdtPr>") : p.prTag) + pr + "</w:sdtPr>" + p.tail;
}

} // namespace

bool setBlockControlProps(WriteDoc &doc, const ContentControl &ctl, const ControlProps &props, QString *problem)
{
    QTextDocument *story = doc.story();
    if (!ctl.valid() || !ctl.block || !ctl.editable || ctl.props.kind != props.kind) return false;
    if (props.items.size() > kMostControlItems) {
        if (problem) *problem = QCoreApplication::translate("jp::write::ContentControls", "A list holds at most %1 items.").arg(kMostControlItems);
        return false;
    }
    if (props == ctl.props) return true;
    const QTextBlock b = story->findBlock(ctl.start);
    QVector<Block> list = keptOf(b, prop::RawBefore);
    if (ctl.openIndex < 0 || ctl.openIndex >= list.size() || list[ctl.openIndex].tag != QLatin1String("sdt") || list[ctl.openIndex].role != Inline::Open) return false;
    const Pieces pieces = piecesOf(list[ctl.openIndex].raw);
    if (!pieces.ok) return false;
    list[ctl.openIndex].raw = buildStart(props, ctl.placeholder, pieces, false);
    const WriteDoc::StepGroup group(doc);
    EditScope scope(doc, story);
    QTextCursor edit(story);
    edit.beginEditBlock();
    setKept(b, prop::RawBefore, list);
    edit.endEditBlock();
    return true;
}

namespace {
// The pieces kept between the paragraph `z` and the one after it, in the order of the file: what the paragraph keeps after it and what the next keeps before it (a file's reader puts
// the end of a control in either).
QVector<Block> gapAfter(const QTextBlock &z)
{
    QVector<Block> gap = keptOf(z, prop::RawAfter);
    if (z.next().isValid()) gap += keptOf(z.next(), prop::RawBefore);
    return gap;
}

// Puts the pieces between `z` and the paragraph after it: the first `inAfter` kept after `z` (where a split paragraph's end goes with its last half), the rest before the next.
void setGapAfter(const QTextBlock &z, const QVector<Block> &gap, int inAfter)
{
    const QTextBlock next = z.next();
    if (!next.isValid()) inAfter = int(gap.size());
    setKept(z, prop::RawAfter, gap.mid(0, inAfter));
    if (next.isValid()) setKept(next, prop::RawBefore, gap.mid(inAfter));
}

// Whether the paragraphs from `a` to `z` hold the pieces kept round them in balance: what they open, they close, and (in `*closing`) how many of the pieces that come after `z` close what
// they opened. `*skip` pieces at the start of what is kept before `a` close what was opened before it, and are not the control's.
bool balancedRange(const QTextBlock &a, const QTextBlock &z, int *skip, int *closing)
{
    const QVector<Block> head = keptOf(a, prop::RawBefore);
    int leading = 0;
    while (leading < head.size() && head[leading].role == Inline::Close) ++leading;
    int depth = 0, visited = 0;
    for (QTextBlock b = a; b.isValid() && visited < 200000; b = b.next(), ++visited) {
        if (b.blockFormat().hasProperty(prop::RawBlock)) return false;
        const QVector<Block> before = keptOf(b, prop::RawBefore);
        for (qsizetype i = b == a ? leading : 0; i < before.size(); ++i) {
            depth += before[i].role == Inline::Open ? 1 : before[i].role == Inline::Close ? -1 : 0;
            if (depth < 0) return false;
        }
        if (b == z) break;
        for (const Block &k : keptOf(b, prop::RawAfter)) {
            depth += k.role == Inline::Open ? 1 : k.role == Inline::Close ? -1 : 0;
            if (depth < 0) return false;
        }
    }
    // (The pieces after the last paragraph that close what is open are the first of them.)
    const QVector<Block> gap = gapAfter(z);
    if (depth > gap.size()) return false;
    for (int i = 0; i < depth; ++i)
        if (gap[i].role != Inline::Close) return false;
    *skip = leading;
    *closing = depth;
    return true;
}
} // namespace

bool insertBlockControl(WriteDoc &doc, QTextCursor &c, ControlKind kind, QString *problem)
{
    QTextDocument *story = doc.story();
    auto refuse = [&](const QString &why) {
        if (problem) *problem = why;
        return false;
    };
    if (c.document() != story || (kind != ControlKind::RichText && kind != ControlKind::RepeatingSection))
        return refuse(QCoreApplication::translate("jp::write::ContentControls", "A content control goes in the text, not in a header, a footer, or a note."));
    const int a = c.selectionStart(), z = c.selectionEnd();
    const QTextBlock first = story->findBlock(a), last = story->findBlock(z > a ? z - 1 : z);
    if (!first.isValid() || !last.isValid()) return false;
    int skip = 0, closing = 0;
    // (A control round paragraphs starts and ends outside a table; a table in between is inside it whole.)
    if (inTable(first) || inTable(last) || !balancedRange(first, last, &skip, &closing))
        return refuse(QCoreApplication::translate("jp::write::ContentControls", "The paragraphs chosen hold only part of a table or of another content control."));
    if (int(blockSpans(story).size() + contentControls(doc).size()) >= kMostControls) return refuse(QCoreApplication::translate("jp::write::ContentControls", "The document has the most content controls it can hold."));
    for (QTextBlock b = first;; b = b.next()) {
        if (!contentControlsAllow(doc, story, b.position(), b.position() + b.length() - 1, true)) return refuse(QCoreApplication::translate("jp::write::ContentControls", "The content control there can't be changed."));
        if (b == last) break;
    }
    const ContentControl outer = blockControlAt(doc, first.position(), story);
    if (outer.valid() && outer.depth + (kind == ControlKind::RepeatingSection ? 3 : 1) >= kMostControlDepth) return refuse(QCoreApplication::translate("jp::write::ContentControls", "Content controls can hold each other only so deep."));
    ControlProps props;
    props.kind = kind;
    Pieces blank;
    blank.head = "<w:sdt>";
    blank.tail = "<w:sdtContent>";
    blank.ok = true;
    Block open, close;
    open.isRaw = close.isRaw = true;
    open.tag = close.tag = QStringLiteral("sdt");
    open.role = Inline::Open;
    close.role = Inline::Close;
    open.raw = buildStart(props, false, blank, true);
    close.raw = kClosingXml;
    QVector<Block> opens{open}, closes{close};
    if (kind == ControlKind::RepeatingSection) {
        Block item = open, itemClose = close;
        item.raw = "<w:sdt><w:sdtPr>" + newIdXml() + "<w15:repeatingSectionItem/></w:sdtPr><w:sdtContent>";
        opens.push_back(item);
        closes.push_front(itemClose);
    }
    const WriteDoc::StepGroup group(doc);
    EditScope scope(doc, story);
    QTextCursor edit(story);
    edit.beginEditBlock();
    QVector<Block> gap = gapAfter(last);
    for (qsizetype i = 0; i < closes.size(); ++i) gap.insert(closing + i, closes[i]);
    QVector<Block> head = keptOf(first, prop::RawBefore);
    for (qsizetype i = 0; i < opens.size(); ++i) head.insert(skip + i, opens[i]);
    // (The pieces after the last paragraph: those that end what it holds and the new control's end are kept after it, the rest before the next paragraph.)
    setGapAfter(last, gap, closing + int(closes.size()));
    setKept(story->findBlock(first.position()), prop::RawBefore, head);
    edit.endEditBlock();
    c.setPosition(story->findBlock(first.position()).position());
    return true;
}


ContentControl repeatingItemAt(WriteDoc &doc, int position, ContentControl *section, QTextDocument *story)
{
    if (!story) story = doc.story();
    ContentControl item, outer;
    for (const BlockSpan &s : blockSpans(story)) {
        const ContentControl c = blockControlOf(story, s);
        if (position < c.start || position > c.end) continue;
        if (c.props.sectionItem) item = c;
        else if (c.props.kind == ControlKind::RepeatingSection) outer = c;
    }
    if (item.valid() && outer.valid() && item.depth > outer.depth) {
        if (section) *section = outer;
        return item;
    }
    return ContentControl();
}

namespace {
// Gives a copy of a repeating item's paragraphs what a new item starts with: every control in it shows its placeholder (a check box is empty), and every control has its own id.
void freshenItem(WriteDoc &doc, int fromBlock, int paragraphs)
{
    QTextDocument *story = doc.story();
    QTextBlock b = story->findBlock(fromBlock);
    for (int count = 0; b.isValid() && count < paragraphs; b = b.next(), ++count) {
        QVector<Block> before = keptOf(b, prop::RawBefore), after = keptOf(b, prop::RawAfter);
        bool changed = false;
        for (Block &k : before)
            if (k.tag == QLatin1String("sdt") && k.role == Inline::Open) {
                k.raw = withNewId(k.raw);
                changed = true;
            }
        if (changed) setKept(b, prop::RawBefore, before);
        // (Controls in the text of the paragraph, the last first, so that what comes before has not moved.)
        const QVector<ContentControl> inline_ = contentControlsOf(b);
        for (qsizetype i = inline_.size() - 1; i >= 0; --i) {
            ContentControl ctl = inline_[i];
            Inline atom;
            if (startOf(story, ctl.start, &atom)) {
                const Pieces p = piecesOf(atom.raw);
                if (p.ok) setStartXml(story, ctl.start, withNewId(atom.raw));
            }
            ctl = contentControlAt(doc, ctl.start + 1, story);
            if (!ctl.valid() || !ctl.editable || ctl.props.lockContents) continue;
            const ControlKind k = ctl.props.kind;
            if (k == ControlKind::CheckBox) {
                ControlProps np = ctl.props;
                np.checked = false;
                putContents(doc, ctl, QString(), false, np, glyphOf(atom.raw, false), glyphFontOf(atom.raw, false));
            } else if (k == ControlKind::PlainText || k == ControlKind::RichText || k == ControlKind::ComboBox || k == ControlKind::DropDown || k == ControlKind::Date || k == ControlKind::Picture) {
                ControlProps np = ctl.props;
                np.date = QDate();
                np.lastValue.clear();
                putContents(doc, ctl, placeholderText(k), true, np);
            }
        }
    }
}
} // namespace

namespace {
// Where the closing piece of the item that ends in the paragraph `z` is among the pieces kept after it (the closes of controls inside the item come first, in the order they were
// opened); -1 for none.
int itemCloseIndex(QTextDocument *story, const QVector<Block> &gap, const QTextBlock &z, int itemDepth)
{
    int inner = 0;
    for (const BlockSpan &s : blockSpans(story))
        if (s.last == z.position() && s.depth > itemDepth) ++inner;
    int seen = 0;
    for (qsizetype i = 0; i < gap.size(); ++i)
        if (gap[i].tag == QLatin1String("sdt") && gap[i].role == Inline::Close && seen++ == inner) return int(i);
    return -1;
}

int itemsIn(QTextDocument *story, const ContentControl &section, const ContentControl &item, ContentControl *last)
{
    int count = 0;
    for (const BlockSpan &s : blockSpans(story)) {
        const ContentControl c = blockControlOf(story, s);
        if (!c.props.sectionItem || c.depth != item.depth || c.start < section.start || c.end > section.end) continue;
        if (markFormat(story->findBlock(c.start)).hasProperty(prop::TrackedDeleted)) continue;   // (an item a tracked change deleted is not one the section still has)
        ++count;
        if (last && (!last->valid() || c.start > last->start)) *last = c;
    }
    return count;
}

// How many of the pieces from `from` on are closes, one after another.
int closesFrom(const QVector<Block> &gap, int from)
{
    int n = from;
    while (n < gap.size() && gap[n].role == Inline::Close) ++n;
    return n;
}

// Whether a paragraph from `a` to `z` is in a table (a table in a cell of another is in one too).
bool holdsTable(const QTextBlock &a, const QTextBlock &z)
{
    for (QTextBlock b = a; b.isValid(); b = b.next()) {
        if (inTable(b)) return true;
        if (b == z) break;
    }
    return false;
}
} // namespace

bool addRepeatingItem(WriteDoc &doc, int position, QString *problem)
{
    QTextDocument *story = doc.story();
    ContentControl section;
    const ContentControl item = repeatingItemAt(doc, position, &section, story);
    if (!item.valid() || !section.valid() || !item.editable || !section.editable) return false;
    if (section.props.lockContents) {
        if (problem) *problem = QCoreApplication::translate("jp::write::ContentControls", "The contents of this section can't be edited.");
        return false;
    }
    if (section.props.fixedItems) {
        if (problem) *problem = QCoreApplication::translate("jp::write::ContentControls", "Items can't be added to or removed from this section.");
        return false;
    }
    // (The last item of the section is the one copied, and the new one comes after it.)
    ContentControl last;
    const int count = itemsIn(story, section, item, &last);
    if (count >= kMostRepeatingItems) {
        if (problem) *problem = QCoreApplication::translate("jp::write::ContentControls", "A repeating section holds at most %1 items.").arg(kMostRepeatingItems);
        return false;
    }
    if (int(blockSpans(story).size() + contentControls(doc).size()) >= kMostControls) {
        if (problem) *problem = QCoreApplication::translate("jp::write::ContentControls", "The document has the most content controls it can hold.");
        return false;
    }
    const QTextBlock a = story->findBlock(last.start), z = story->findBlock(last.end);
    const bool record = doc.trackChanges() && story == doc.story();
    if (record && !z.next().isValid()) {
        // (The paragraph that ends the story can't be taken away again by Reject All, which the new item must be able to be.)
        if (problem) *problem = QCoreApplication::translate("jp::write::ContentControls", "While Track Changes is on, an item can't be added to a section that ends the document.");
        return false;
    }
    if (record && holdsTable(a, z)) {
        // (The rows of its table would have to be recorded as inserted ones too.)
        if (problem) *problem = QCoreApplication::translate("jp::write::ContentControls", "While Track Changes is on, an item that holds a table can't be added.");
        return false;
    }
    QVector<QVector<Block>> was[2];   // (the pieces each paragraph of the item keeps, before and after)
    int paragraphs = 0;
    for (QTextBlock b = a; b.isValid(); b = b.next(), ++paragraphs) {
        was[0].push_back(keptOf(b, prop::RawBefore));
        was[1].push_back(keptOf(b, prop::RawAfter));
        if (b == z) break;
    }
    ++paragraphs;
    const QVector<Block> gap = gapAfter(z);
    const int closeAt = itemCloseIndex(story, gap, z, last.depth);
    if (closeAt < 0 || last.openIndex < 0 || last.openIndex >= was[0].first().size()) return false;
    const int tailEnd = closesFrom(gap, closeAt + 1);   // (what closes what holds the section goes with the new item, what opens the next paragraph's stays)
    const int lastPos = z.position();
    const WriteDoc::StepGroup group(doc);
    EditScope scope(doc, story);
    std::optional<UntrackedEdit> untracked;   // (the copy is made as it is, and then recorded as one insertion)
    if (record) untracked.emplace();
    QTextCursor edit(story);
    edit.beginEditBlock();
    QTextCursor copy(story);
    copy.setPosition(a.position());
    copy.setPosition(z.position() + z.length() - 1, QTextCursor::KeepAnchor);
    const QTextDocumentFragment fragment = copy.selection();
    QTextCursor put(story);
    put.setPosition(z.position() + z.length() - 1);
    put.insertBlock();
    put.insertFragment(fragment);
    QVector<int> made;
    QTextBlock n = story->findBlock(lastPos).next();
    for (int i = 0; i < paragraphs && n.isValid(); ++i, n = n.next()) made.push_back(n.position());
    if (made.size() != paragraphs) {
        edit.endEditBlock();
        doc.undo();
        if (problem) *problem = QCoreApplication::translate("jp::write::ContentControls", "The item could not be added.");
        return false;
    }
    // The copy's first paragraph has the item's start and what was opened with it, its last has the item's end and what closes the section; the old last paragraph ends the old item, and the
    // next paragraph keeps what was before it and not the ends that have gone to the copy.
    for (int i = 0; i < paragraphs; ++i) {
        QVector<Block> before = i == 0 ? was[0][0].mid(last.openIndex) : was[0][i];
        for (Block &k : before)
            if (k.tag == QLatin1String("sdt") && k.role == Inline::Open) k.raw = withNewId(k.raw);
        setKept(story->findBlock(made[i]), prop::RawBefore, before);
        setKept(story->findBlock(made[i]), prop::RawAfter, i == paragraphs - 1 ? gap.mid(0, tailEnd) : was[1][i]);
    }
    const QTextBlock oldLast = story->findBlock(lastPos);
    setKept(oldLast, prop::RawAfter, gap.mid(0, closeAt + 1));
    const QTextBlock after = story->findBlock(made.last()).next();
    if (after.isValid()) setKept(after, prop::RawBefore, gap.mid(tailEnd));
    freshenItem(doc, made.first(), paragraphs);
    if (record) {
        // (The break after the old last paragraph and the new paragraphs' own are inserted ones; Reject takes them and the pieces of the new item away, Accept keeps them.)
        const QTextBlock oldEnd = story->findBlock(lastPos), newEnd = story->findBlockByNumber(oldEnd.blockNumber() + paragraphs);
        int to = newEnd.position() + newEnd.length() - 1;
        trackRangeInserted(doc, story, oldEnd.position() + oldEnd.length() - 1, &to);
    }
    edit.endEditBlock();
    return true;
}

bool removeRepeatingItem(WriteDoc &doc, int position, QString *problem)
{
    QTextDocument *story = doc.story();
    ContentControl section;
    const ContentControl item = repeatingItemAt(doc, position, &section, story);
    if (!item.valid() || !section.valid() || !item.editable) return false;
    if (section.props.fixedItems) {
        if (problem) *problem = QCoreApplication::translate("jp::write::ContentControls", "Items can't be added to or removed from this section.");
        return false;
    }
    if (itemsIn(story, section, item, nullptr) <= 1) {
        if (problem) *problem = QCoreApplication::translate("jp::write::ContentControls", "A repeating section keeps at least one item.");
        return false;
    }
    if (item.props.lockDelete || section.props.lockContents) {
        if (problem) *problem = QCoreApplication::translate("jp::write::ContentControls", "The item can't be deleted.");
        return false;
    }
    const QTextBlock a = story->findBlock(item.start), z = story->findBlock(item.end);
    if (doc.trackChanges() && story == doc.story()) {
        // With Track Changes on the item's paragraphs are marked as deleted, and Reject All and Accept All do the rest (the controls' own pieces stay where they are until then).
        if (markFormat(a).hasProperty(prop::TrackedDeleted)) {
            if (problem) *problem = QCoreApplication::translate("jp::write::ContentControls", "The item is already marked as deleted.");
            return false;
        }
        if (!z.next().isValid() || a.position() == 0) {
            if (problem) *problem = QCoreApplication::translate("jp::write::ContentControls", "While Track Changes is on, an item can't be removed from the end of the document.");
            return false;
        }
        if (holdsTable(a, z)) {
            if (problem) *problem = QCoreApplication::translate("jp::write::ContentControls", "While Track Changes is on, an item that holds a table can't be removed.");
            return false;
        }
        const WriteDoc::StepGroup group(doc);
        EditScope scope(doc, story);
        const int blocks = story->blockCount();
        const QSet<QByteArray> emptyBefore = emptyControlStarts(story);
        trackRangeDeleted(doc, story, a.position(), z.next().position());
        if (story->blockCount() < blocks) dropEmptyControls(story, emptyBefore);   // (what the reviewer inserted themselves goes for good)
        return true;
    }
    const QVector<Block> aBefore = keptOf(a, prop::RawBefore), gap = gapAfter(z);
    const int closeAt = itemCloseIndex(story, gap, z, item.depth);
    if (closeAt < 0 || item.openIndex < 0 || item.openIndex >= aBefore.size()) return false;
    // What stands before the item's start (the section's own start, when it is the first item) and after its end (the section's end, when it is the last) stays: the pieces between the
    // paragraph before the item and the paragraph after it are what was before the item's start and what was after its end.
    const QTextBlock previous = a.previous(), next = z.next();
    QVector<Block> between = previous.isValid() ? keptOf(previous, prop::RawAfter) : QVector<Block>();
    between += aBefore.mid(0, item.openIndex);
    between += gap.mid(closeAt + 1);
    const int previousPos = previous.isValid() ? previous.position() : -1;
    const WriteDoc::StepGroup group(doc);
    EditScope scope(doc, story);
    QTextCursor edit(story);
    edit.beginEditBlock();
    QTextCursor del(story);
    if (next.isValid()) {
        del.setPosition(a.position());
        del.setPosition(next.position(), QTextCursor::KeepAnchor);
    } else {
        del.setPosition(previous.position() + previous.length() - 1);
        del.setPosition(z.position() + z.length() - 1, QTextCursor::KeepAnchor);
    }
    del.removeSelectedText();
    // (Closes go after the paragraph before, from the first piece that opens on they go before the paragraph after.)
    int closing = 0;
    while (closing < between.size() && between[closing].role == Inline::Close) ++closing;
    if (previousPos >= 0 && next.isValid()) {
        setGapAfter(story->findBlock(previousPos), between, closing);
    } else if (previousPos >= 0) {
        setKept(story->findBlock(previousPos), prop::RawAfter, between);
    } else if (next.isValid()) {
        setKept(story->findBlock(a.position()), prop::RawBefore, between);
    }
    edit.endEditBlock();
    return true;
}


} // namespace jp::write
