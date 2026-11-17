#include "core/contentcontrolops.h"

#include "core/editops.h"
#include "core/writeprops.h"
#include "text/textprops.h"

#include <QCoreApplication>
#include <QLocale>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <algorithm>

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

QString unescaped(QString text)
{
    text.replace(QLatin1String("&lt;"), QLatin1String("<")).replace(QLatin1String("&gt;"), QLatin1String(">")).replace(QLatin1String("&quot;"), QLatin1String("\"")).replace(QLatin1String("&apos;"), QLatin1String("'"));
    text.replace(QLatin1String("&amp;"), QLatin1String("&"));
    return text;
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

// The value of an attribute of the start tag of an element (by its local name), or `none`.
QString attrOf(const QByteArray &element, const char *name, bool *has = nullptr)
{
    const qsizetype end = tagEnd(element, 0);
    const QString tag = QString::fromUtf8(end < 0 ? element : element.left(end + 1));
    const QRegularExpression re(QStringLiteral("\\s(?:[A-Za-z0-9_]+:)?") + QString::fromLatin1(name) + QStringLiteral("\\s*=\\s*(?:\"([^\"]*)\"|'([^']*)')"));
    const QRegularExpressionMatch m = re.match(tag);
    if (has) *has = m.hasMatch();
    if (!m.hasMatch()) return QString();
    return unescaped(m.captured(m.capturedStart(1) >= 0 ? 1 : 2));
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
    QByteArray head, tail;
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
        if (e < 0 || raw[e - 1] != '/') return p;
        p.head = raw.left(pr);
        p.tail = raw.mid(e + 1);
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
            QByteArray t = old.left(s + 1);
            if (t.endsWith("/>")) t = t.left(t.size() - 2) + ">";
            else if (old.lastIndexOf("</") > s) childrenOf(old.mid(s + 1, old.lastIndexOf("</") - s - 1), &kids);
            // (Its own attributes stay, except the date.)
            QString text = QString::fromUtf8(t);
            text.replace(QRegularExpression(QStringLiteral("\\s(?:[A-Za-z0-9_]+:)?fullDate\\s*=\\s*(?:\"[^\"]*\"|'[^']*')")), QString());
            tag = text.chopped(1).toUtf8();
        }
    }
    if (p.date.isValid()) tag += " w:fullDate=\"" + p.date.toString(QStringLiteral("yyyy-MM-dd")).toLatin1() + "T00:00:00Z\"";
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

QByteArray dropDownElement(const ControlProps &p, const QByteArray &old)
{
    Q_UNUSED(old)
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
    return QByteArray("<w:dropDownList") + (!last.isEmpty() && known ? " w:lastValue=\"" + escaped(last) + "\"" : QByteArray()) + ">" + items + "</w:dropDownList>";
}

// The XML of a control's start: the properties given over what `old` kept (what is not known is in place, as it was).
QByteArray buildStart(const ControlProps &np, bool placeholder, const Pieces &old, bool newId)
{
    QVector<QByteArray> before, others;
    QByteArray idKid, oldType;
    const char *typeName = np.kind == ControlKind::PlainText ? "text" : np.kind == ControlKind::DropDown ? "dropDownList" : np.kind == ControlKind::Date ? "date" : np.kind == ControlKind::CheckBox ? "checkbox" : "";
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
    case ControlKind::Other: break;
    }
    return old.head + (pr.isEmpty() ? QByteArray("<w:sdtPr/>") : "<w:sdtPr>" + pr + "</w:sdtPr>") + old.tail;
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
    QTextCharFormat look = charFormatAt(d, std::max(0, position - 1));
    if (position <= d->findBlock(position).position()) look = d->findBlock(position).charFormat();
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

bool chosenOnly(ControlKind kind) { return kind == ControlKind::DropDown || kind == ControlKind::Date || kind == ControlKind::CheckBox; }

// A control as the atoms of a paragraph say: positions only (the properties are read from the start by whoever needs them).
struct Span {
    int start, end, depth;
    QByteArray raw;
};

QVector<Span> spansOf(const QTextBlock &block, int *budget = nullptr)
{
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
    if (ctl.end > ctl.start + 1) {
        edit.setPosition(ctl.start + 1);
        edit.setPosition(ctl.end, QTextCursor::KeepAnchor);
        edit.removeSelectedText();
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
    if (kind != o.kind || title != o.title || tag != o.tag || lockDelete != o.lockDelete || lockContents != o.lockContents || dateFormat != o.dateFormat || date != o.date || checked != o.checked || lastValue != o.lastValue || items.size() != o.items.size()) return false;
    for (int i = 0; i < items.size(); ++i)
        if (items[i].display != o.items[i].display || items[i].value != o.items[i].value) return false;
    return true;
}

ControlProps controlPropsOf(const QByteArray &startXml, bool *ok, bool *placeholder)
{
    ControlProps p;
    p.kind = ControlKind::Other;
    if (ok) *ok = false;
    if (placeholder) *placeholder = false;
    const Pieces pieces = piecesOf(startXml);
    if (!pieces.ok) return p;
    bool galleryOrOther = false;
    for (const QByteArray &k : pieces.kids) {
        const QString name = localName(k);
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
        } else if (name == QLatin1String("dropDownList")) {
            p.kind = ControlKind::DropDown;
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
            if (p.dateFormat.isEmpty()) p.dateFormat = QStringLiteral("M/d/yyyy");
        } else if (name == QLatin1String("checkbox")) {
            p.kind = ControlKind::CheckBox;
            QVector<QByteArray> kids;
            const qsizetype s = tagEnd(k, 0), e = k.lastIndexOf("</");
            if (s > 0 && k[s - 1] != '/' && e > s && childrenOf(k.mid(s + 1, e - s - 1), &kids))
                for (const QByteArray &d : kids)
                    if (localName(d) == QLatin1String("checked")) p.checked = attrOf(d, "val") == QLatin1String("1") || attrOf(d, "val") == QLatin1String("true");
        } else if (name == QLatin1String("docPartObj") || name == QLatin1String("docPartList") || name == QLatin1String("comboBox") || name == QLatin1String("richText") || name == QLatin1String("picture")
                   || name == QLatin1String("group") || name == QLatin1String("citation") || name == QLatin1String("bibliography") || name == QLatin1String("equation") || name == QLatin1String("repeatingSection")) {
            galleryOrOther = true;
        }
    }
    if (galleryOrOther) p.kind = ControlKind::Other;
    if (ok) *ok = true;
    return p;
}

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
    if (c.document() != story || kind == ControlKind::Other) {
        if (problem) *problem = QCoreApplication::translate("jp::write::ContentControls", "A content control goes in the text, not in a header, a footer, or a note.");
        return false;
    }
    int from = c.position(), to = c.position();
    if (c.hasSelection()) {
        const int a = c.selectionStart(), b = c.selectionEnd();
        const bool oneParagraph = story->findBlock(a) == story->findBlock(b > a ? b - 1 : b);
        if (kind == ControlKind::PlainText && oneParagraph) {
            from = a;
            to = b;
        } else {
            from = to = b;
        }
    }
    if (isLocked(story->findBlock(from))) return false;
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
    np.title = np.title.left(255);
    np.tag = np.tag.left(255);
    const Pieces pieces = piecesOf(atom.raw);
    if (!pieces.ok) return false;
    const WriteDoc::StepGroup group(doc);
    setStartXml(story, start, buildStart(np, placeholder, pieces, false));
    return true;
}

bool chooseControlItem(WriteDoc &doc, int start, int index)
{
    QTextDocument *story = doc.story();
    const ContentControl ctl = contentControlAt(doc, start + 1, story);
    if (!ctl.valid() || ctl.start != start || ctl.props.kind != ControlKind::DropDown || !ctl.editable || index < 0 || index >= ctl.props.items.size()) return false;
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
    putContents(doc, ctl, date.isValid() ? formatControlDate(date, np.dateFormat) : placeholderText(ControlKind::Date), !date.isValid(), np);
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

QString formatControlDate(const QDate &date, const QString &format)
{
    return QLocale().toString(date, format.isEmpty() ? QStringLiteral("M/d/yyyy") : format);
}

int controlItemShown(const ContentControl &control)
{
    if (control.props.kind != ControlKind::DropDown || control.placeholder) return -1;
    for (int i = 0; i < control.props.items.size(); ++i) {
        const ControlItem &it = control.props.items[i];
        if ((it.display.isEmpty() ? it.value : it.display) == control.text) return i;
    }
    return -1;
}

bool contentControlsAllow(WriteDoc &doc, QTextDocument *story, int from, int to, bool typing)
{
    Q_UNUSED(doc)
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
            if (coversOpen && coversClose) continue;
            if (intoContents && (props.lockContents || chosenOnly(props.kind))) return false;
        }
    }
    return true;
}

// The control whose placeholder what is typed at the caret takes the place of (not valid where there is none).
static ContentControl placeholderControlFor(WriteDoc &doc, const QTextCursor &c)
{
    QTextDocument *story = c.document();
    if (story != doc.story()) return ContentControl();
    const int at = c.hasSelection() ? c.selectionStart() : c.position();
    const ContentControl ctl = contentControlAt(doc, c.hasSelection() ? at + 1 : at, story);
    if (!ctl.valid() || !ctl.placeholder || !ctl.editable || ctl.props.kind != ControlKind::PlainText || ctl.props.lockContents) return ContentControl();
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
    return ctl.valid() && ctl.props.kind != ControlKind::Other;
}

} // namespace jp::write
