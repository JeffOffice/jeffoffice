#include "core/textboxops.h"

#include "core/editops.h"
#include "core/writedoc.h"
#include "core/writeprops.h"
#include "flow/flowdocument.h"
#include "text/textprops.h"
#include "wml/wml.h"

#include <QCoreApplication>
#include <QRegularExpression>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>

#include <algorithm>
#include <cmath>

namespace jp::write {

using namespace office::wml;

std::pair<QPointF, QPointF> textBoxDragEnds(const QPointF &from, const QPointF &to, bool keepProportions)
{
    if (!std::isfinite(from.x()) || !std::isfinite(from.y()) || !std::isfinite(to.x()) || !std::isfinite(to.y())) return {from, from};
    QPointF d = to - from;
    if (std::abs(d.x()) < 4 && std::abs(d.y()) < 4) {
        // (A click: the Simple design's usual size, from the point.)
        const QSizeF usual = textBoxDesigns().first().size;
        return {from, from + QPointF(usual.width(), usual.height())};
    }
    if (keepProportions) {
        const double side = std::max(std::abs(d.x()), std::abs(d.y()));
        d = QPointF(d.x() < 0 ? -side : side, d.y() < 0 ? -side : side);
    }
    const QPointF end = from + d;
    return {QPointF(std::min(from.x(), end.x()), std::min(from.y(), end.y())), QPointF(std::max(from.x(), end.x()), std::max(from.y(), end.y()))};
}

PictureResult insertTextBoxAt(WriteDoc &doc, QTextCursor &c, const QPointF &from, const QPointF &to, bool keepProportions)
{
    if (!std::isfinite(from.x()) || !std::isfinite(from.y()) || !std::isfinite(to.x()) || !std::isfinite(to.y())) return PictureResult::Refused;
    const auto [a, b] = textBoxDragEnds(from, to, keepProportions);
    const WriteDoc::StepGroup step(doc);
    const int at = c.hasSelection() ? c.selectionStart() : c.position();
    TextBoxDesign design = textBoxDesigns().first();
    design.wrap = PictureWrap::InFrontOfText;
    design.size = QSizeF(std::abs(b.x() - a.x()), std::abs(b.y() - a.y()));
    const PictureResult result = insertTextBox(doc, c, design);
    if (result != PictureResult::Ok) return result;
    if (!setPictureBox(doc, at, design.size.width(), design.size.height(), a)) {
        // (The box was put in and could not be set at its place: it is taken out again, so nothing is left behind.)
        QTextCursor remove(doc.story());
        remove.setPosition(at);
        remove.setPosition(at + 1, QTextCursor::KeepAnchor);
        remove.removeSelectedText();
        doc.pictureNotice = QCoreApplication::translate("jp::write::Pictures", "A text box can't be put here.");
        return PictureResult::Refused;
    }
    return PictureResult::Ok;
}

namespace {

// The drawing (or older-format picture) the character after `position` holds. A shape that a newer program wrote with a fallback for older ones is the choice that names the shape format
// (the fallback goes when it is changed, as it does for every other change of a drawn object).
bool drawnAtomAt(const QTextDocument *story, int position, Inline *atom)
{
    if (!story || position < 0 || position + 1 > story->characterCount() - 1) return false;
    const QTextCharFormat cf = charFormatAt(story, position);
    if (!cf.hasProperty(prop::Atom) || cf.hasProperty(prop::TrackedDeleted)) return false;
    const QByteArray bytes = storedBytes(cf, prop::Atom);
    if (!bytes.contains("drawing") && !bytes.contains("pict")) return false;
    const Inline a = unpackAtom(bytes);
    if (a.kind != Inline::RunAtom) return false;
    if (a.tag == QLatin1String("AlternateContent")) {
        const qsizetype fallback = a.raw.indexOf("Fallback");
        const qsizetype from = a.raw.indexOf("<w:drawing");
        const qsizetype to = a.raw.lastIndexOf("</w:drawing>", fallback >= 0 ? fallback : -1);
        if (from < 0 || to < from) return false;
        if (fallback >= 0 && fallback < from) return false;
        Inline drawing = a;
        drawing.tag = QStringLiteral("drawing");
        drawing.raw = a.raw.mid(from, to + 12 - from);
        *atom = drawing;
        return true;
    }
    if (a.tag != QLatin1String("drawing") && a.tag != QLatin1String("pict")) return false;
    *atom = a;
    return true;
}

// Where the end of the start tag that begins at `from` is (past quoted values), or -1.
qsizetype endOfTag(const QByteArray &x, qsizetype from)
{
    char quote = 0;
    for (qsizetype i = from; i < x.size(); ++i) {
        const char ch = x[i];
        if (quote) quote = ch == quote ? 0 : quote;
        else if (ch == '"' || ch == '\'') quote = ch;
        else if (ch == '>') return i;
    }
    return -1;
}

// The start tag of the shape's own body (wps:bodyPr), not one of a shape inside its words: [begin, end] of the tag, or -1 for none.
struct Tag {
    qsizetype begin = -1, end = -1;
};

Tag ownBodyTag(const QByteArray &x)
{
    qsizetype at = 0;
    for (int steps = 0; steps < 100000; ++steps) {
        const qsizetype body = x.indexOf("<wps:bodyPr", at), words = x.indexOf("<w:txbxContent", at);
        if (body < 0) return {};
        if (words < 0 || body < words) {
            const char next = body + 11 < x.size() ? x[body + 11] : 0;
            if (next == ' ' || next == '/' || next == '>') return {body, endOfTag(x, body)};
            at = body + 11;
            continue;
        }
        // (Past the words: the content may hold drawings of its own.)
        int depth = 0;
        qsizetype i = words;
        while (i < x.size()) {
            const qsizetype open = x.indexOf("<w:txbxContent", i), close = x.indexOf("</w:txbxContent>", i);
            if (close < 0) return {};
            if (open >= 0 && open < close) {
                ++depth;
                i = open + 14;
            } else {
                --depth;
                i = close + 16;
                if (depth == 0) break;
            }
        }
        at = i;
    }
    return {};
}

const char *vertWord(TextDirection d) { return d == TextDirection::Rotate90 ? "vert" : d == TextDirection::Rotate270 ? "vert270" : "horz"; }

// The value of an attribute in a start tag (empty if none).
QByteArray attributeIn(const QByteArray &tag, const QByteArray &name)
{
    static const QRegularExpression any(QStringLiteral("\\s([A-Za-z:]+)=\"([^\"]*)\""));
    const QString text = QString::fromUtf8(tag);
    auto it = any.globalMatch(text);
    while (it.hasNext()) {
        const auto m = it.next();
        if (m.captured(1).toUtf8() == name) return m.captured(2).toUtf8();
    }
    return {};
}

TextDirection directionOfDrawing(const QByteArray &raw)
{
    const Tag t = ownBodyTag(raw);
    if (t.begin < 0 || t.end < 0) return TextDirection::Horizontal;
    const QByteArray v = attributeIn(raw.mid(t.begin, t.end - t.begin + 1), "vert");
    return v == "vert270" ? TextDirection::Rotate270 : v == "vert" || v == "eaVert" ? TextDirection::Rotate90 : TextDirection::Horizontal;
}

// A box of the older format: its words' style says how they flow (layout-flow, and mso-layout-flow-alt for the turn the other way).
struct Flow {
    qsizetype begin = -1, end = -1;   // the value of the style attribute of v:textbox
};

Flow flowStyle(const QByteArray &raw)
{
    const qsizetype tag = raw.indexOf("<v:textbox");
    if (tag < 0) return {};
    const qsizetype close = endOfTag(raw, tag);
    if (close < 0) return {};
    const qsizetype style = raw.indexOf(" style=\"", tag);
    if (style < 0 || style > close) return {tag + 10, tag + 10};
    return {style + 8, raw.indexOf('"', style + 8)};
}

TextDirection directionOfPict(const QByteArray &raw)
{
    const Flow f = flowStyle(raw);
    if (f.begin < 0 || f.end < f.begin) return TextDirection::Horizontal;
    const QByteArray style = raw.mid(f.begin, f.end - f.begin);
    if (!style.contains("layout-flow:vertical")) return TextDirection::Horizontal;
    return style.contains("mso-layout-flow-alt:bottom-to-top") ? TextDirection::Rotate270 : TextDirection::Rotate90;
}

QByteArray pictWithDirection(const QByteArray &raw, TextDirection d)
{
    const Flow f = flowStyle(raw);
    if (f.begin < 0) return raw;
    const bool had = f.end > f.begin || raw.mid(f.begin - 8, 8) == " style=\"";
    QStringList keep;
    if (f.end > f.begin)
        for (const QString &part : QString::fromUtf8(raw.mid(f.begin, f.end - f.begin)).split(QLatin1Char(';'), Qt::SkipEmptyParts))
            if (!part.trimmed().startsWith(QLatin1String("layout-flow")) && !part.trimmed().startsWith(QLatin1String("mso-layout-flow-alt"))) keep << part.trimmed();
    if (d != TextDirection::Horizontal) keep << QStringLiteral("layout-flow:vertical");
    if (d == TextDirection::Rotate270) keep << QStringLiteral("mso-layout-flow-alt:bottom-to-top");
    const QByteArray style = keep.join(QLatin1Char(';')).toUtf8();
    if (had) return raw.left(f.begin) + style + raw.mid(f.end);
    return raw.left(f.begin) + " style=\"" + style + "\"" + raw.mid(f.begin);
}

QByteArray drawingWithDirection(const QByteArray &raw, TextDirection d)
{
    const Tag t = ownBodyTag(raw);
    const QByteArray value = vertWord(d);
    if (t.begin < 0) {
        const qsizetype wsp = raw.lastIndexOf("</wps:wsp>");
        if (wsp < 0) return raw;
        return raw.left(wsp) + "<wps:bodyPr vert=\"" + value + "\"/>" + raw.mid(wsp);
    }
    if (t.end < 0) return raw;
    const QByteArray tag = raw.mid(t.begin, t.end - t.begin + 1);
    static const QRegularExpression has(QStringLiteral("\\svert=\"[^\"]*\""));
    const QString text = QString::fromUtf8(tag);
    const QRegularExpressionMatch m = has.match(text);
    QString out = text;
    if (m.hasMatch()) out.replace(m.capturedStart(), m.capturedLength(), QStringLiteral(" vert=\"%1\"").arg(QString::fromLatin1(value)));
    else out.insert(11, QStringLiteral(" vert=\"%1\"").arg(QString::fromLatin1(value)));
    return raw.left(t.begin) + out.toUtf8() + raw.mid(t.end + 1);
}

} // namespace

TextDirection textBoxDirection(const WriteDoc &doc, int position)
{
    Inline atom;
    if (!drawnAtomAt(doc.story(), position, &atom)) return TextDirection::Horizontal;
    return atom.tag == QLatin1String("pict") ? directionOfPict(atom.raw) : directionOfDrawing(atom.raw);
}

bool setTextBoxDirection(WriteDoc &doc, int position, TextDirection direction)
{
    QTextDocument *story = doc.story();
    Inline atom;
    if (!drawnAtomAt(story, position, &atom) || isLocked(story->findBlock(position))) return false;
    // (A group is not one box; a chart or a picture takes no words.)
    if (atom.raw.contains("<wpg:wgp") || atom.raw.contains("<c:chart") || atom.raw.contains("<pic:pic")) return false;
    const bool pict = atom.tag == QLatin1String("pict");
    if ((pict ? directionOfPict(atom.raw) : directionOfDrawing(atom.raw)) == direction) return false;
    if (!pict && !atom.raw.contains("<wps:wsp")) return false;
    atom.raw = pict ? pictWithDirection(atom.raw, direction) : drawingWithDirection(atom.raw, direction);
    QTextCharFormat cf = charFormatAt(story, position);
    cf.clearProperty(flow::fp::AnchorId);
    cf.clearProperty(tp::InlineObject);
    setStored(cf, prop::Atom, packAtom(atom));
    if (!doc.makeDrawnFormat(atom, &cf)) return false;
    QTextCursor edit(story);
    edit.setPosition(position);
    edit.setPosition(position + 1, QTextCursor::KeepAnchor);
    const QTextCharFormat was = charFormatAt(story, position);
    edit.setCharFormat(cf);
    doc.retireDrawnForm(was);
    doc.keepObjectsForTheHistory();
    return true;
}

} // namespace jp::write
