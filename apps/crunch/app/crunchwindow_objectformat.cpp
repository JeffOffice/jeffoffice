// Format of the picture or shape that is picked: fill, outline, rotation, crop, and the text in a shape. The element the drawing holds for the object is copied, changed, and put back as
// the object (the old one taken away and the new one made in its place, with its name, alternative text, place, and order among the objects), as one step of Undo. What a file's object holds
// that these commands do not touch stays in the copy as it was.
#include <memory>
#include <QXmlStreamWriter>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextBlock>
#include <QTimer>
#include <QApplication>
#include "office/drawingml_write.h"
#include "office/drawingml.h"
#include "office/drawingml_color.h"
#include "app/shapetextedit.h"
#include "app/spelling.h"
#include "app/settings.h"
#include "app/crunchwindow.h"

#include "app/compresspictures.h"
#include "app/picturetools.h"
#include "core/picturestyles.h"
#include "core/picturecrop.h"
#include "office/drawingml_geometry.h"
#include "app/icons.h"
#include "app/plaintext.h"
#include "app/ribbonbuilder.h"
#include "core/objectclip.h"
#include "core/sheetobjects.h"

#include <QAction>
#include <QComboBox>
#include <QBuffer>
#include <QDialog>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QImageReader>
#include <QStandardPaths>
#include <QDialogButtonBox>
#include <QDomDocument>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QMenu>
#include <QPlainTextEdit>
#include <QSpinBox>
#include <QTextStream>
#include <QUndoStack>
#include <QVBoxLayout>

#include <cmath>

namespace jp {

using namespace jp::office::sheet;

namespace {

const QString kA = QStringLiteral("http://schemas.openxmlformats.org/drawingml/2006/main");
const QString kXdr = QStringLiteral("http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing");
const QString kR = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships");

QDomElement childNamed(const QDomElement &e, const QString &local)
{
    for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement())
        if (c.localName() == local) return c;
    return QDomElement();
}

bool isOneOf(const QDomElement &e, std::initializer_list<const char *> names)
{
    for (const char *n : names)
        if (e.localName() == QLatin1String(n)) return true;
    return false;
}

QDomElement newA(QDomDocument &doc, const char *name) { return doc.createElementNS(kA, QStringLiteral("a:") + QLatin1String(name)); }

// Puts `e` in `parent` after the last child named in `after` (first of all if there is none).
void insertAfterLast(QDomElement &parent, const QDomElement &e, std::initializer_list<const char *> after)
{
    QDomNode last;
    for (QDomElement c = parent.firstChildElement(); !c.isNull(); c = c.nextSiblingElement())
        if (isOneOf(c, after)) last = c;
    if (last.isNull()) parent.insertBefore(e, parent.firstChild());
    else parent.insertAfter(e, last);
}

QDomElement spPrOf(const QDomElement &object) { return childNamed(object, QStringLiteral("spPr")); }

QDomElement colorElement(QDomDocument &doc, const QString &hex)
{
    QDomElement fill = newA(doc, "solidFill");
    QDomElement c = newA(doc, "srgbClr");
    c.setAttribute(QStringLiteral("val"), hex.toUpper());
    fill.appendChild(c);
    return fill;
}

void removeFills(QDomElement &parent, bool all)
{
    QList<QDomElement> go;
    for (QDomElement c = parent.firstChildElement(); !c.isNull(); c = c.nextSiblingElement())
        if (isOneOf(c, {"noFill", "solidFill", "gradFill", "pattFill"}) || (all && isOneOf(c, {"blipFill", "grpFill"}))) go << c;
    for (QDomElement &c : go) parent.removeChild(c);
}

// The fill of the shape: a color (6 hex digits), or none (empty `hex`).
bool setFill(QDomElement &object, const QString &hex)
{
    QDomElement spPr = spPrOf(object);
    if (spPr.isNull() || object.localName() != QLatin1String("sp")) return false;
    removeFills(spPr, true);
    QDomDocument doc = spPr.ownerDocument();
    insertAfterLast(spPr, hex.isEmpty() ? newA(doc, "noFill") : colorElement(doc, hex), {"xfrm", "custGeom", "prstGeom"});
    return true;
}

QDomElement lineOf(QDomElement &spPr)
{
    QDomElement ln = childNamed(spPr, QStringLiteral("ln"));
    if (ln.isNull()) {
        QDomDocument doc = spPr.ownerDocument();
        ln = newA(doc, "ln");
        insertAfterLast(spPr, ln, {"xfrm", "custGeom", "prstGeom", "noFill", "solidFill", "gradFill", "blipFill", "pattFill", "grpFill"});
    }
    return ln;
}

// The outline: its color (6 hex digits; empty with `none` for no line), its weight in points (0: as it is), and its dash ("" as it is, "solid", "dash", "sysDot").
bool setLine(QDomElement &object, const QString &hex, bool none, double weightPoints, const QString &dash)
{
    QDomElement spPr = spPrOf(object);
    if (spPr.isNull()) return false;
    QDomDocument doc = spPr.ownerDocument();
    QDomElement ln = lineOf(spPr);
    if (none || !hex.isEmpty()) {
        removeFills(ln, false);
        ln.insertBefore(none ? newA(doc, "noFill") : colorElement(doc, hex), ln.firstChild());
    }
    if (weightPoints > 0) ln.setAttribute(QStringLiteral("w"), qint64(std::llround(weightPoints * 12700)));
    if (!dash.isEmpty()) {
        QDomElement old = childNamed(ln, QStringLiteral("prstDash"));
        if (!old.isNull()) ln.removeChild(old);
        QDomElement d = newA(doc, "prstDash");
        d.setAttribute(QStringLiteral("val"), dash);
        insertAfterLast(ln, d, {"noFill", "solidFill", "gradFill", "pattFill"});
    }
    return true;
}

bool rotate(QDomElement &object, int quarterTurns, bool flipH, bool flipV)
{
    QDomElement spPr = spPrOf(object);
    if (spPr.isNull()) return false;
    QDomElement xfrm = childNamed(spPr, QStringLiteral("xfrm"));
    if (xfrm.isNull()) {
        QDomDocument doc = spPr.ownerDocument();
        xfrm = newA(doc, "xfrm");
        spPr.insertBefore(xfrm, spPr.firstChild());
    }
    if (quarterTurns) {
        qint64 rot = xfrm.attribute(QStringLiteral("rot"), QStringLiteral("0")).toLongLong() + qint64(quarterTurns) * 5400000;
        rot = ((rot % 21600000) + 21600000) % 21600000;
        if (rot) xfrm.setAttribute(QStringLiteral("rot"), rot);
        else xfrm.removeAttribute(QStringLiteral("rot"));
    }
    auto toggle = [&](const char *name) {
        const QString key = QLatin1String(name);
        if (xfrm.attribute(key) == QLatin1String("1") || xfrm.attribute(key) == QLatin1String("true")) xfrm.removeAttribute(key);
        else xfrm.setAttribute(key, QStringLiteral("1"));
    };
    if (flipH) toggle("flipH");
    if (flipV) toggle("flipV");
    return true;
}

// The part of the picture that shows, as the share cut off each side in thousandths of a percent.
struct Crop {
    qint64 l = 0, t = 0, r = 0, b = 0;
};

Crop cropOf(const QDomElement &pic)
{
    Crop c;
    const QDomElement blip = childNamed(pic, QStringLiteral("blipFill"));
    const QDomElement src = blip.isNull() ? QDomElement() : childNamed(blip, QStringLiteral("srcRect"));
    if (!src.isNull()) {
        c.l = src.attribute(QStringLiteral("l"), QStringLiteral("0")).toLongLong();
        c.t = src.attribute(QStringLiteral("t"), QStringLiteral("0")).toLongLong();
        c.r = src.attribute(QStringLiteral("r"), QStringLiteral("0")).toLongLong();
        c.b = src.attribute(QStringLiteral("b"), QStringLiteral("0")).toLongLong();
    }
    c.l = std::clamp<qint64>(c.l, 0, 90000);
    c.r = std::clamp<qint64>(c.r, 0, 90000 - c.l);
    c.t = std::clamp<qint64>(c.t, 0, 90000);
    c.b = std::clamp<qint64>(c.b, 0, 90000 - c.t);
    return c;
}

bool setCrop(QDomElement &pic, const Crop &c)
{
    QDomElement blip = childNamed(pic, QStringLiteral("blipFill"));
    if (blip.isNull() || pic.localName() != QLatin1String("pic")) return false;
    QDomElement old = childNamed(blip, QStringLiteral("srcRect"));
    if (!old.isNull()) blip.removeChild(old);
    if (c.l || c.t || c.r || c.b) {
        QDomDocument doc = pic.ownerDocument();
        QDomElement src = newA(doc, "srcRect");
        if (c.l) src.setAttribute(QStringLiteral("l"), c.l);
        if (c.t) src.setAttribute(QStringLiteral("t"), c.t);
        if (c.r) src.setAttribute(QStringLiteral("r"), c.r);
        if (c.b) src.setAttribute(QStringLiteral("b"), c.b);
        insertAfterLast(blip, src, {"blip"});
    }
    return true;
}

Crop cropOfShares(const CropShares &c)
{
    Crop out;
    out.l = std::clamp<qint64>(std::llround(c.left * 100000.0), 0, 90000);
    out.r = std::clamp<qint64>(std::llround(c.right * 100000.0), 0, 90000 - out.l);
    out.t = std::clamp<qint64>(std::llround(c.top * 100000.0), 0, 90000);
    out.b = std::clamp<qint64>(std::llround(c.bottom * 100000.0), 0, 90000 - out.t);
    return out;
}

QString textOf(const QDomElement &sp)
{
    const QDomElement body = childNamed(sp, QStringLiteral("txBody"));
    QStringList paragraphs;
    for (QDomElement p = body.firstChildElement(); !p.isNull(); p = p.nextSiblingElement()) {
        if (p.localName() != QLatin1String("p")) continue;
        QString line;
        for (QDomElement r = p.firstChildElement(); !r.isNull(); r = r.nextSiblingElement()) {
            if (r.localName() == QLatin1String("br")) line += QLatin1Char('\n');
            else if (r.localName() == QLatin1String("r") || r.localName() == QLatin1String("fld")) line += childNamed(r, QStringLiteral("t")).text();
        }
        paragraphs << line;
    }
    return paragraphs.join(QLatin1Char('\n'));
}

bool setText(QDomElement &sp, const CrunchWindow::ShapeTextChoices &c)
{
    if (sp.localName() != QLatin1String("sp")) return false;
    sp.removeAttribute(QStringLiteral("textlink"));   // (words typed in a shape take it off its cell, as the original does)
    QDomDocument doc = sp.ownerDocument();
    QDomElement body = childNamed(sp, QStringLiteral("txBody"));
    if (body.isNull()) {
        body = doc.createElementNS(kXdr, QStringLiteral("xdr:txBody"));
        sp.appendChild(body);
    }
    QDomElement props = childNamed(body, QStringLiteral("bodyPr"));
    if (props.isNull()) {
        props = newA(doc, "bodyPr");
        body.insertBefore(props, body.firstChild());
    }
    props.setAttribute(QStringLiteral("anchor"), c.vertical == 0 ? QStringLiteral("t") : c.vertical == 1 ? QStringLiteral("ctr") : QStringLiteral("b"));
    props.setAttribute(QStringLiteral("wrap"), c.wrap ? QStringLiteral("square") : QStringLiteral("none"));
    QList<QDomElement> go;
    for (QDomElement e = body.firstChildElement(); !e.isNull(); e = e.nextSiblingElement())
        if (e.localName() == QLatin1String("p")) go << e;
    for (QDomElement &e : go) body.removeChild(e);
    if (childNamed(body, QStringLiteral("lstStyle")).isNull()) body.insertAfter(newA(doc, "lstStyle"), props);
    const QString algn = c.horizontal == 0 ? QStringLiteral("l") : c.horizontal == 1 ? QStringLiteral("ctr") : QStringLiteral("r");
    for (const QString &line : c.text.split(QLatin1Char('\n'))) {
        QDomElement p = newA(doc, "p");
        QDomElement pPr = newA(doc, "pPr");
        pPr.setAttribute(QStringLiteral("algn"), algn);
        p.appendChild(pPr);
        auto runProps = [&](const char *name) {
            QDomElement r = newA(doc, name);
            r.setAttribute(QStringLiteral("lang"), QStringLiteral("en-US"));
            r.setAttribute(QStringLiteral("sz"), qint64(std::clamp(c.sizePoints, 1, 400)) * 100);
            if (c.bold) r.setAttribute(QStringLiteral("b"), QStringLiteral("1"));
            if (!c.colorHex.isEmpty()) r.appendChild(colorElement(doc, c.colorHex));
            return r;
        };
        if (line.isEmpty()) {
            p.appendChild(runProps("endParaRPr"));
        } else {
            QDomElement run = newA(doc, "r");
            run.appendChild(runProps("rPr"));
            QDomElement t = newA(doc, "t");
            t.appendChild(doc.createTextNode(line));
            run.appendChild(t);
            p.appendChild(run);
        }
        body.appendChild(p);
    }
    return true;
}

QDomElement firstNamed(const QDomElement &e, const QString &local, int depth = 0)
{
    if (e.localName() == local) return e;
    if (depth > 8) return QDomElement();
    for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) {
        const QDomElement f = firstNamed(c, local, depth + 1);
        if (!f.isNull()) return f;
    }
    return QDomElement();
}

QString hexOf(const QColor &c) { return c.name().mid(1).toUpper(); }

// Whether the words of a text body are only what the editor reads and writes back: paragraphs of runs, line breaks, and fields, as many as it reads. Anything else (an equation in an
// alternate-content element, say) would be lost when the paragraphs are written again, and so is not edited here.
bool wordsFitTheEditor(const QDomElement &body)
{
    int paragraphs = 0, runs = 0;
    for (QDomElement e = body.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
        const QString name = e.localName();
        if (name == QLatin1String("bodyPr") || name == QLatin1String("lstStyle")) continue;
        if (name != QLatin1String("p") || ++paragraphs > 10000) return false;
        for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) {
            const QString n = c.localName();
            if (n == QLatin1String("r") || n == QLatin1String("fld") || n == QLatin1String("br")) {
                if (++runs > 20000) return false;
                // (a run that links somewhere has a relationship that the copy of the shape does not carry)
                if (!firstNamed(c, QStringLiteral("hlinkClick")).isNull() || !firstNamed(c, QStringLiteral("hlinkMouseOver")).isNull()) return false;
            } else if (n != QLatin1String("pPr") && n != QLatin1String("endParaRPr")) {
                return false;
            }
        }
    }
    return true;
}

// The color the shape's style gives its words (the style's font reference), read as the workbook's theme gives it; invalid when it gives none.
QColor styleTextColor(const QDomElement &sp, const jp::office::Theme &theme)
{
    const QDomElement ref = firstNamed(childNamed(sp, QStringLiteral("style")), QStringLiteral("fontRef"));
    if (ref.isNull()) return QColor();
    const QDomElement clr = ref.firstChildElement();
    if (clr.isNull()) return QColor();
    const QString v = clr.attribute(QStringLiteral("val"));
    if (clr.localName() == QLatin1String("srgbClr")) return QColor(QLatin1Char('#') + v);
    static const QHash<QString, int> index = {{QStringLiteral("dk1"), 0}, {QStringLiteral("lt1"), 1}, {QStringLiteral("dk2"), 2}, {QStringLiteral("lt2"), 3}, {QStringLiteral("accent1"), 4}, {QStringLiteral("accent2"), 5},
                                              {QStringLiteral("accent3"), 6}, {QStringLiteral("accent4"), 7}, {QStringLiteral("accent5"), 8}, {QStringLiteral("accent6"), 9}, {QStringLiteral("hlink"), 10},
                                              {QStringLiteral("folHlink"), 11}, {QStringLiteral("tx1"), 0}, {QStringLiteral("bg1"), 1}, {QStringLiteral("tx2"), 2}, {QStringLiteral("bg2"), 3}};
    const auto it = index.constFind(v);
    return it == index.cend() ? QColor() : theme.colors[it.value()];
}

// The words of a shape's text body written back as its paragraphs (the body's properties and list styles stay): the shape's txBody is made if it has none.
bool setTextStory(QDomElement &sp, QTextDocument *story, const jp::office::Theme &theme)
{
    if (sp.localName() != QLatin1String("sp")) return false;
    sp.removeAttribute(QStringLiteral("textlink"));   // (words typed in a shape take it off its cell, as the original does)
    QDomDocument doc = sp.ownerDocument();
    QDomElement body = childNamed(sp, QStringLiteral("txBody"));
    if (body.isNull()) {
        body = doc.createElementNS(kXdr, QStringLiteral("xdr:txBody"));
        sp.appendChild(body);
    }
    QDomElement props = childNamed(body, QStringLiteral("bodyPr"));
    if (props.isNull()) {
        props = newA(doc, "bodyPr");
        body.insertBefore(props, body.firstChild());
    }
    if (childNamed(body, QStringLiteral("lstStyle")).isNull()) body.insertAfter(newA(doc, "lstStyle"), props);
    // (a size every run says: the sheet draws the words as it reads them, at the size they say)
    const double base = story->defaultFont().pointSizeF() > 0 ? story->defaultFont().pointSizeF() : 11;
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next())
        for (auto it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment fr = it.fragment();
            if (fr.charFormat().hasProperty(QTextFormat::FontPointSize)) continue;
            QTextCursor c(story);
            c.setPosition(fr.position());
            c.setPosition(fr.position() + fr.length(), QTextCursor::KeepAnchor);
            QTextCharFormat f;
            f.setFontPointSize(base);
            c.mergeCharFormat(f);
        }
    QByteArray xml;
    {
        QXmlStreamWriter w(&xml);
        jp::office::DrawingWriteContext ctx;
        ctx.theme = theme;   // (a color of the workbook's theme is written as that color of the theme, not as the color it is today)
        jp::office::writeParagraphs(w, story, ctx);
    }
    QDomDocument made;
    if (!made.setContent("<w xmlns:a=\"" + kA.toUtf8() + "\">" + xml + "</w>", QDomDocument::ParseOption::UseNamespaceProcessing)) return false;
    // The colors the words had as colors of the theme (the text engine holds the color they come to): each, by what it comes to, to be put back as the theme color it was.
    QHash<QString, QDomElement> themed;
    {
        QList<QDomElement> pending{body};
        for (int steps = 0; !pending.isEmpty() && steps < 100000; ++steps) {
            const QDomElement e = pending.takeLast();
            if (e.localName() == QLatin1String("schemeClr") && e.parentNode().toElement().localName() == QLatin1String("solidFill")) {
                const QColor c = jp::office::readColor(e.parentNode().toElement(), theme, jp::office::ColorMap());
                if (c.isValid() && !themed.contains(c.name().toUpper())) themed.insert(c.name().toUpper(), e.cloneNode(true).toElement());
            }
            for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) pending << c;
        }
    }
    QList<QDomElement> go;
    for (QDomElement e = body.firstChildElement(); !e.isNull(); e = e.nextSiblingElement())
        if (e.localName() == QLatin1String("p")) go << e;
    for (QDomElement &e : go) body.removeChild(e);
    bool any = false;
    for (QDomElement p = made.documentElement().firstChildElement(); !p.isNull(); p = p.nextSiblingElement()) {
        QDomElement imported = doc.importNode(p, true).toElement();
        QList<QDomElement> walk{imported};
        QList<QDomElement> plain;
        for (int steps = 0; !walk.isEmpty() && steps < 100000; ++steps) {
            const QDomElement e = walk.takeLast();
            if (e.localName() == QLatin1String("srgbClr") && e.parentNode().toElement().localName() == QLatin1String("solidFill") && e.childNodes().isEmpty()) plain << e;
            for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) walk << c;
        }
        for (const QDomElement &e : plain) {
            const auto it = themed.constFind((QLatin1Char('#') + e.attribute(QStringLiteral("val"))).toUpper());
            if (it != themed.cend()) e.parentNode().replaceChild(doc.importNode(it.value(), true), e);
        }
        body.appendChild(imported);
        any = true;
    }
    if (!any) body.appendChild(newA(doc, "p"));
    return true;
}

} // namespace

// Changes the element of the picked object with `edit` (which may also read what the object is, and says false when there is nothing to change), and puts the result in its place.
bool CrunchWindow::restyleObject(const QString &stepText, const std::function<bool(QDomElement &, grid::ObjectBox *)> &edit, bool pictureOnly, int forId, const std::function<void(crunch::ObjectClip &)> &withClip)
{
    const int id = forId >= 0 ? forId : pickedObject();
    if (id < 0 || !m_session || !m_objects || m_chartSheetShown) return false;
    const ObjectKind kind = m_objects->kindOf(m_sheet, id);
    if (kind != ObjectKind::Picture && kind != ObjectKind::Shape && kind != ObjectKind::Connector) {
        notice(stepText, tr("This can be done to a picture, a shape, or a text box. Pick one first."));
        return false;
    }
    if (pictureOnly && kind != ObjectKind::Picture) return false;
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !editingAllowed()) return false;
    QString why;
    if (!m_objects->canDelete(m_sheet, id, &why)) {
        notice(stepText, why);
        return false;
    }
    crunch::ObjectClip clip;
    if (!m_objects->copyElement(m_sheet, id, &clip, &why)) {
        notice(stepText, why);
        return false;
    }
    if (withClip) withClip(clip);
    const QByteArray wrapped = "<w xmlns:xdr=\"" + kXdr.toUtf8() + "\" xmlns:a=\"" + kA.toUtf8() + "\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"" + clip.namespaces
                               + ">" + clip.elementXml + "</w>";
    QDomDocument doc;
    if (!doc.setContent(wrapped, QDomDocument::ParseOption::UseNamespaceProcessing)) {
        notice(stepText, tr("This object can't be changed here."));
        return false;
    }
    QDomElement el = doc.documentElement().firstChildElement();
    if (el.isNull()) return false;
    grid::ObjectBox box = m_objects->layer()->boxNow(id);
    if (!edit(el, &box)) return false;
    QDomElement props = firstNamed(el, QStringLiteral("cNvPr"));
    if (!props.isNull()) {
        props.setAttribute(QStringLiteral("name"), m_objects->nameOf(m_sheet, id));
        const QString d = m_objects->descriptionOf(m_sheet, id);
        if (d.isEmpty()) props.removeAttribute(QStringLiteral("descr"));
        else props.setAttribute(QStringLiteral("descr"), d);
    }
    QString text;
    {
        QTextStream ts(&text);
        el.save(ts, -1);
    }
    clip.elementXml = text.toUtf8();
    clip.width = std::clamp(box.w, 4.0, 20000.0);
    clip.height = std::clamp(box.h, 1.0, 20000.0);
    const DrawingAnchor was = m_objects->anchorOf(m_sheet, id);
    DrawingAnchor anchor = was;
    m_objects->layer()->attach(&m_view->sheet().layout());
    if (!m_objects->layer()->anchorAtBox(box, was.kind, &anchor)) anchor = was;
    anchor.editAs = was.editAs;
    const int order = m_objects->stateOf(m_sheet, id).order;
    QUndoStack *u = undoStack();
    u->beginMacro(stepText);
    deleteObject(id);
    bool done = false;
    if (m_objects->isDeleted(m_sheet, id)) done = placeObjectClip(clip, stepText, &anchor, order);
    u->endMacro();
    if (!done && m_objects->isDeleted(m_sheet, id) && u->canUndo()) u->undo();
    return done;
}

bool CrunchWindow::setObjectFill(const QColor &color, bool none)
{
    const QString hex = none ? QString() : hexOf(color);
    return restyleObject(none ? tr("No Fill") : tr("Shape Fill"), [&](QDomElement &el, grid::ObjectBox *) { return setFill(el, hex); });
}

bool CrunchWindow::setObjectOutline(const QColor &color, bool none)
{
    const QString hex = none ? QString() : hexOf(color);
    return restyleObject(none ? tr("No Outline") : tr("Outline"), [&](QDomElement &el, grid::ObjectBox *) { return setLine(el, hex, none, 0, QString()); });
}

bool CrunchWindow::setObjectOutlineStyle(double weightPoints, const QString &dash)
{
    return restyleObject(tr("Outline"), [&](QDomElement &el, grid::ObjectBox *) { return setLine(el, QString(), false, weightPoints, dash); });
}

namespace {

// The elements of `xml` (the drawing format's, as text) put in the shape properties in place of those named `names`, after the elements of the kinds that come before them in the schema's
// order (a list of what comes before: the geometry, the fills, the line, and the effects).
void putElements(QDomElement &spPr, std::initializer_list<const char *> names, const QByteArray &xml, std::initializer_list<const char *> after)
{
    QList<QDomElement> go;
    for (QDomElement c = spPr.firstChildElement(); !c.isNull(); c = c.nextSiblingElement())
        if (isOneOf(c, names)) go << c;
    for (QDomElement &c : go) spPr.removeChild(c);
    if (xml.isEmpty()) return;
    QDomDocument tmp;
    if (!tmp.setContent("<x xmlns:a=\"" + kA.toUtf8() + "\">" + xml + "</x>", QDomDocument::ParseOption::UseNamespaceProcessing)) return;
    QDomDocument target = spPr.ownerDocument();
    for (QDomElement c = tmp.documentElement().firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) {
        QDomElement copy = target.importNode(c, true).toElement();
        insertAfterLast(spPr, copy, after);
    }
}

constexpr std::initializer_list<const char *> kBeforeLine = {"xfrm", "custGeom", "prstGeom", "noFill", "solidFill", "gradFill", "blipFill", "pattFill", "grpFill"};

int effectRank(const QString &local)
{
    static const QStringList order = {QStringLiteral("blur"), QStringLiteral("fillOverlay"), QStringLiteral("glow"), QStringLiteral("innerShdw"), QStringLiteral("outerShdw"), QStringLiteral("prstShdw"),
                                      QStringLiteral("reflection"), QStringLiteral("softEdge")};
    const int i = int(order.indexOf(local));
    return i < 0 ? int(order.size()) : i;
}

// The effect list of the shape properties with the kinds the menus set taken out and those of `made` (an a:effectLst as text, or empty) put in; the list's other effects stay, all in the schema's order.
void mergeEffectList(QDomElement &spPr, const QByteArray &made)
{
    QList<QDomElement> kept;
    QDomElement old = childNamed(spPr, QStringLiteral("effectLst"));
    QDomDocument target = spPr.ownerDocument();
    for (QDomElement c = old.firstChildElement(); !c.isNull(); c = c.nextSiblingElement())
        if (!isOneOf(c, {"glow", "innerShdw", "outerShdw", "reflection", "softEdge"})) kept << c.cloneNode(true).toElement();
    QList<QDomElement> fresh;
    if (!made.isEmpty()) {
        QDomDocument tmp;
        if (tmp.setContent("<x xmlns:a=\"" + kA.toUtf8() + "\">" + made + "</x>", QDomDocument::ParseOption::UseNamespaceProcessing))
            for (QDomElement c = tmp.documentElement().firstChildElement().firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) fresh << target.importNode(c, true).toElement();
    }
    QList<QDomElement> all = kept + fresh;
    if (!old.isNull()) spPr.removeChild(old);
    if (all.isEmpty()) return;
    std::stable_sort(all.begin(), all.end(), [](const QDomElement &a, const QDomElement &b) { return effectRank(a.localName()) < effectRank(b.localName()); });
    QDomElement list = newA(target, "effectLst");
    for (QDomElement &c : all) list.appendChild(c);
    insertAfterLast(spPr, list, {"xfrm", "custGeom", "prstGeom", "noFill", "solidFill", "gradFill", "blipFill", "pattFill", "grpFill", "ln"});
}

bool flatEffectsDiffer(Effects a, Effects b)
{
    a.bevel = b.bevel = BevelFx();
    a.rot3d = b.rot3d = Rotation3D();
    return a.toJson() != b.toJson();
}

bool solidEffectsDiffer(const Effects &a, const Effects &b)
{
    return a.bevel.type != b.bevel.type || a.bevel.width != b.bevel.width || a.bevel.height != b.bevel.height || a.rot3d.x != b.rot3d.x || a.rot3d.y != b.rot3d.y ||
           a.rot3d.perspective != b.rot3d.perspective;
}

// Puts the effects that changed from `was` in the picture's shape properties: the effect list for the flat ones, the scene and the solid for a bevel or a turn.
bool putEffects(QDomElement &spPr, const Effects &fx, const Effects &was, const ColorScheme &colors)
{
    const office::EffectElements made = office::writeEffectElements(fx, colors);
    bool changed = false;
    if (flatEffectsDiffer(fx, was)) {
        mergeEffectList(spPr, made.list);
        changed = true;
    }
    if (solidEffectsDiffer(fx, was)) {
        putElements(spPr, {"scene3d"}, made.scene, {"xfrm", "custGeom", "prstGeom", "noFill", "solidFill", "gradFill", "blipFill", "pattFill", "grpFill", "ln", "effectLst", "effectDag"});
        putElements(spPr, {"sp3d"}, made.solid, {"xfrm", "custGeom", "prstGeom", "noFill", "solidFill", "gradFill", "blipFill", "pattFill", "grpFill", "ln", "effectLst", "effectDag", "scene3d"});
        changed = true;
    }
    return changed;
}

Effects effectsOfPicture(const QDomElement &spPr)
{
    office::DrawingContext ctx;
    return office::readEffects(spPr, QDomElement(), ctx);
}

} // namespace

bool CrunchWindow::applyPictureStyle(int style)
{
    PictureItem look;
    if (!jp::applyPictureStyle(&look, style)) return false;
    const ColorScheme colors = workbookColors();
    return restyleObject(tr("Picture Style"), [&](QDomElement &el, grid::ObjectBox *) {
        QDomElement spPr = spPrOf(el);
        if (spPr.isNull() || el.localName() != QLatin1String("pic")) return false;
        QDomDocument doc = spPr.ownerDocument();
        // The shape the picture is cut to.
        const QString preset = office::presetForShape(look.maskShape).isEmpty() ? QStringLiteral("rect") : office::presetForShape(look.maskShape);
        putElements(spPr, {"prstGeom", "custGeom"}, "<a:prstGeom prst=\"" + preset.toUtf8() + "\"><a:avLst/></a:prstGeom>", {"xfrm"});
        // The border.
        const QColor c = look.stroke.isNone() ? QColor() : look.stroke.color.resolve(colors);
        QByteArray line = "<a:ln><a:noFill/></a:ln>";
        if (c.isValid() && look.stroke.width > 0) {
            static const char *const dashes[] = {"solid", "dot", "sysDot", "dash", "dashDot", "lgDash", "lgDashDot", "lgDashDotDot"};
            line = "<a:ln w=\"" + QByteArray::number(qint64(std::llround(look.stroke.width * 12700))) + "\"" + (look.stroke.compound == Stroke::Double ? " cmpd=\"dbl\"" : "") + "><a:solidFill><a:srgbClr val=\"" +
                   c.name(QColor::HexRgb).mid(1).toUpper().toLatin1() + "\"/></a:solidFill>" + (look.stroke.dash != Stroke::SolidLine ? "<a:prstDash val=\"" + QByteArray(dashes[std::clamp(int(look.stroke.dash), 0, 7)]) + "\"/>" : "") + "</a:ln>";
        }
        putElements(spPr, {"ln"}, line, kBeforeLine);
        putEffects(spPr, look.fx, effectsOfPicture(spPr), colors);
        return true;
    }, true);
}

bool CrunchWindow::setPictureEffects(const std::function<void(Effects &)> &change)
{
    const ColorScheme colors = workbookColors();
    return restyleObject(tr("Picture Effects"), [&](QDomElement &el, grid::ObjectBox *) {
        QDomElement spPr = spPrOf(el);
        if (spPr.isNull() || el.localName() != QLatin1String("pic")) return false;
        const Effects was = effectsOfPicture(spPr);
        Effects fx = was;
        change(fx);
        if (fx.toJson() == was.toJson()) return false;
        return putEffects(spPr, fx, was, colors);
    }, true);
}

bool CrunchWindow::resetPicture(bool andSize)
{
    QSize pixels;
    return restyleObject(andSize ? tr("Reset Picture & Size") : tr("Reset Picture"), [&](QDomElement &el, grid::ObjectBox *box) {
        QDomElement spPr = spPrOf(el);
        if (spPr.isNull() || el.localName() != QLatin1String("pic")) return false;
        // The shape, the border, and the effects go; the crop goes, and the frame shows the whole picture.
        putElements(spPr, {"prstGeom", "custGeom"}, "<a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom>", {"xfrm"});
        putElements(spPr, {"ln", "effectLst", "effectDag", "scene3d", "sp3d"}, QByteArray(), {});
        const Crop now = cropOf(el);
        const double showW = 1.0 - double(now.l + now.r) / 100000.0, showH = 1.0 - double(now.t + now.b) / 100000.0;
        box->w = box->w / std::max(0.1, showW);
        box->h = box->h / std::max(0.1, showH);
        setCrop(el, Crop());
        if (andSize && !pixels.isEmpty()) {
            box->w = pixels.width();
            box->h = pixels.height();
        }
        box->w = std::clamp(std::round(box->w), 4.0, 20000.0);
        box->h = std::clamp(std::round(box->h), 4.0, 20000.0);
        return true;
    }, true, -1, [&](crunch::ObjectClip &clip) {
        if (clip.media.size() != 1) return;
        QBuffer in(&clip.media[0].bytes);
        in.open(QIODevice::ReadOnly);
        pixels = QImageReader(&in).size();
    });
}

bool CrunchWindow::changePictureFromFile(const QString &path)
{
    if (pickedObject() < 0 || !m_objects || m_objects->kindOf(m_sheet, pickedObject()) != ObjectKind::Picture) return false;
    const QString title = tr("Change Picture");
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly) || file.size() > crunch::kMaxClipMediaBytes) {
        notice(title, tr("\"%1\" is not a picture JeffCrunch can read, or it is too large.").arg(QFileInfo(path).fileName()));
        return false;
    }
    const QByteArray bytes = file.read(crunch::kMaxClipMediaBytes + 1);
    crunch::ObjectClip fresh;
    if (!pictureClip(bytes, QFileInfo(path).fileName(), 0, 0, QString(), &fresh) || fresh.media.size() != 1 || fresh.width <= 0 || fresh.height <= 0) return false;
    return restyleObject(title, [&](QDomElement &el, grid::ObjectBox *box) {
        if (el.localName() != QLatin1String("pic")) return false;
        // A picture of the old proportions keeps the cut and the size; any other has no cut, keeps its width, and takes the new proportions.
        const Crop now = cropOf(el);
        const double showW = std::max(0.02, 1.0 - double(now.l + now.r) / 100000.0), showH = std::max(0.02, 1.0 - double(now.t + now.b) / 100000.0);
        const double oldRatio = (box->w / showW) / std::max(1.0, box->h / showH), newRatio = fresh.width / std::max(1.0, fresh.height);
        if (std::abs(oldRatio - newRatio) > 0.02 * newRatio) {
            setCrop(el, Crop());
            box->h = std::clamp(std::round(box->w * fresh.height / fresh.width), 4.0, 20000.0);
        }
        return true;
    }, true, -1, [&](crunch::ObjectClip &clip) {
        if (clip.media.size() != 1) return;
        fresh.media[0].token = clip.media[0].token;
        clip.media[0] = fresh.media[0];
    });
}

bool CrunchWindow::rotateObject(int quarterTurns, bool flipH, bool flipV)
{
    return restyleObject(flipH || flipV ? tr("Flip") : tr("Rotate"), [&](QDomElement &el, grid::ObjectBox *) { return rotate(el, quarterTurns, flipH, flipV); });
}

// Crop to a shape of the picture's window (width : height): the sides or the top and bottom are cut, from the middle of what shows now, and the picture is as large as what is left. 0 : 0
// shows all of it again.
bool CrunchWindow::cropObject(int width, int height)
{
    return restyleObject(width > 0 ? tr("Crop") : tr("Reset Crop"), [&](QDomElement &el, grid::ObjectBox *box) {
        const Crop now = cropOf(el);
        const double showW = 1.0 - double(now.l + now.r) / 100000.0, showH = 1.0 - double(now.t + now.b) / 100000.0;
        Crop next;
        if (width <= 0 || height <= 0) {
            box->w = box->w / std::max(0.1, showW);
            box->h = box->h / std::max(0.1, showH);
        } else {
            const CropShares was{double(now.l) / 100000.0, double(now.t) / 100000.0, double(now.r) / 100000.0, double(now.b) / 100000.0};
            const CropPlan plan = planCropAspect(QSizeF(box->w, std::max(1.0, box->h)), was, width, height);
            if (!plan.valid) return false;
            next = cropOfShares(plan.shares);
            box->w = plan.frame.width();
            box->h = plan.frame.height();
        }
        box->w = std::max(4.0, std::round(box->w));
        box->h = std::max(4.0, std::round(box->h));
        return setCrop(el, next);
    }, true);
}

// Fill (the frame stays, the picture covers it) and Fit (the whole picture shows, and the frame takes its shape), worked out from the picture's size in its file.
bool CrunchWindow::fillOrFitObject(bool fill)
{
    QSizeF whole;
    return restyleObject(fill ? tr("Fill") : tr("Fit"), [&](QDomElement &el, grid::ObjectBox *box) {
        if (whole.isEmpty()) return false;
        const QSizeF frame(box->w, std::max(1.0, box->h));
        const CropPlan plan = fill ? planCropFill(frame, whole) : planCropFit(frame, whole, false);
        if (!plan.valid) return false;
        box->w = std::max(4.0, std::round(plan.frame.width()));
        box->h = std::max(4.0, std::round(plan.frame.height()));
        return setCrop(el, cropOfShares(plan.shares));
    }, true, -1, [&](crunch::ObjectClip &clip) {
        if (clip.media.size() != 1) return;
        QByteArray bytes = clip.media[0].bytes;
        QBuffer buffer(&bytes);
        buffer.open(QIODevice::ReadOnly);
        whole = QImageReader(&buffer).size();
    });
}

// Crop to a shape: the picture's own shape geometry (a preset of the gallery), as a picture style sets it.
bool CrunchWindow::cropObjectToShape(const QString &shapeId)
{
    const QString preset = office::presetForShape(shapeId);
    if (preset.isEmpty()) return false;
    return restyleObject(tr("Crop to Shape"), [&](QDomElement &el, grid::ObjectBox *) {
        QDomElement spPr = spPrOf(el);
        if (spPr.isNull() || el.localName() != QLatin1String("pic")) return false;
        const QDomElement now = childNamed(spPr, QStringLiteral("prstGeom"));
        if (!now.isNull() && now.attribute(QStringLiteral("prst")) == preset) return false;
        putElements(spPr, {"prstGeom", "custGeom"}, "<a:prstGeom prst=\"" + preset.toUtf8() + "\"><a:avLst/></a:prstGeom>", {"xfrm"});
        return true;
    }, true);
}

// Compress Pictures: each picture of the sheet (or the picked ones) is made again, as the Crop commands make one, with its file made smaller as the target asks for the size it is shown at
// (and, as asked, with what its crop cuts away taken out of the file and the crop gone). The pictures taken away and the pictures made in their place are one step of Undo, each new one with
// the old one's name, alternative text, place, and order. The bytes saved. (A picture is only on the sheet shown: the sheets are measured when they are shown.)
qint64 CrunchWindow::compressPictures(double ppi, bool all, bool deleteCropped, QString *problems)
{
    if (problems) problems->clear();
    if (!m_session || !m_objects || m_chartSheetShown) return 0;
    if (m_edit.active) commitEdit(CellEditor::Way::Stay);
    if (m_edit.active || !editingAllowed()) return 0;
    const QString stepText = tr("Compress Pictures");
    const std::vector<int> ids = all ? m_objects->idsOf(m_sheet) : pickedObjects();
    std::vector<crunch::ObjectEdit> edits;
    std::vector<int> made;
    int skipped = 0;
    QString skippedWhy;   // (the first reason: the pictures left out for a reason a person can act on)
    std::vector<std::pair<int, int>> replaced;   // (the picture taken away, the one made in its place)
    const int picked = pickedObject();
    qint64 saved = 0;
    for (const int id : ids) {
        if (m_objects->kindOf(m_sheet, id) != ObjectKind::Picture || !m_objects->canDelete(m_sheet, id)) continue;
        crunch::ObjectClip clip;
        if (!m_objects->copyElement(m_sheet, id, &clip, nullptr) || clip.media.size() != 1) continue;
        const QByteArray wrapped = "<w xmlns:xdr=\"" + kXdr.toUtf8() + "\" xmlns:a=\"" + kA.toUtf8() + "\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"" + clip.namespaces
                                   + ">" + clip.elementXml + "</w>";
        QDomDocument doc;
        if (!doc.setContent(wrapped, QDomDocument::ParseOption::UseNamespaceProcessing)) continue;
        QDomElement el = doc.documentElement().firstChildElement();
        const QDomElement blipFill = el.isNull() ? QDomElement() : childNamed(el, QStringLiteral("blipFill"));
        const QDomElement blip = blipFill.isNull() ? QDomElement() : childNamed(blipFill, QStringLiteral("blip"));
        // (A picture with a second picture in it, or one that is only linked, is left as it is.)
        if (blip.isNull() || !childNamed(blip, QStringLiteral("extLst")).isNull() || blip.hasAttributeNS(kR, QStringLiteral("link")) || blip.attributeNS(kR, QStringLiteral("embed")) != clip.media[0].token) continue;
        const QDomElement src = childNamed(blipFill, QStringLiteral("srcRect"));
        auto share = [&](const char *name) { return src.isNull() ? 0.0 : src.attribute(QLatin1String(name), QStringLiteral("0")).toDouble() / 100000.0; };
        const double l = share("l"), t = share("t"), r = share("r"), b = share("b");
        const bool cut = l + t + r + b > 0.000005;
        const bool drop = deleteCropped && cut && l >= 0 && t >= 0 && r >= 0 && b >= 0;
        const double wholeShare = 1.0 - l - r;
        if (wholeShare < 0.02 || clip.width <= 0) continue;
        // (The grid's unit is a pixel at 100%: three quarters of a point.)
        const double shown = clip.width * 0.75 / wholeShare;
        const QRectF keep = drop ? QRectF(l, t, 1.0 - l - r, 1.0 - t - b) : QRectF(0, 0, 1, 1);
        QByteArray bytes, format;
        QString reason;
        if (!compressedPictureFile(clip.media[0].bytes, shown, ppi, keep, &bytes, &format, &reason)) {
            if (!reason.isEmpty()) {
                ++skipped;
                if (skippedWhy.isEmpty()) skippedWhy = reason;
            }
            continue;
        }
        const qint64 gain = clip.media[0].bytes.size() - bytes.size();
        clip.media[0].bytes = bytes;
        clip.media[0].extension = format == "jpg" ? QStringLiteral("jpeg") : QStringLiteral("png");
        clip.media[0].contentType = format == "jpg" ? QStringLiteral("image/jpeg") : QStringLiteral("image/png");
        if (drop) {
            Crop none;
            setCrop(el, none);
        }
        QDomElement props = firstNamed(el, QStringLiteral("cNvPr"));
        if (!props.isNull()) {
            props.setAttribute(QStringLiteral("name"), m_objects->nameOf(m_sheet, id));
            const QString d = m_objects->descriptionOf(m_sheet, id);
            if (d.isEmpty()) props.removeAttribute(QStringLiteral("descr"));
            else props.setAttribute(QStringLiteral("descr"), d);
        }
        QString text;
        {
            QTextStream ts(&text);
            el.save(ts, -1);
        }
        clip.elementXml = text.toUtf8();
        const DrawingAnchor was = m_objects->anchorOf(m_sheet, id);
        QString why;
        const int fresh = m_objects->makeElement(m_sheet, clip, was, &why);
        if (fresh < 0) {
            ++skipped;
            if (skippedWhy.isEmpty()) skippedWhy = why;
            continue;
        }
        made.push_back(fresh);
        replaced.push_back({id, fresh});
        // The old one is taken away (with what writes it back if Undo is used after a save), and the new one is there in its place.
        crunch::ObjectEdit gone;
        gone.sheet = m_sheet;
        gone.id = id;
        gone.before = m_objects->stateOf(m_sheet, id);
        gone.after = gone.before;
        gone.after.deleted = true;
        gone.after.revive = m_objects->captureRevive(m_sheet, id);
        gone.before.revive = gone.after.revive;
        crunch::ObjectEdit put;
        put.sheet = m_sheet;
        put.id = fresh;
        put.before = m_objects->stateOf(m_sheet, fresh);
        put.after = put.before;
        put.after.deleted = false;
        put.after.order = gone.before.order;
        edits.push_back(gone);
        edits.push_back(put);
        saved += gain;
    }
    if (problems && skipped > 0) *problems = skipped == 1 ? tr("1 picture could not be made smaller: %1").arg(skippedWhy) : tr("%1 pictures could not be made smaller: %2").arg(skipped).arg(skippedWhy);
    if (edits.empty()) return 0;
    const crunch::EditSession::Outcome o = m_session->changeObjects(edits, stepText);
    if (!o.ok) {
        for (const int id : made) m_objects->discardMade(m_sheet, id);
        notice(stepText, o.problem);
        return 0;
    }
    // (the picture that was picked is the new one in its place)
    for (const auto &r : replaced)
        if (r.first == picked) m_objects->layer()->select(r.second);
    return saved;
}

void CrunchWindow::compressPicturesDialog()
{
    if (!m_session || !m_objects || m_chartSheetShown) return;
    CompressChoices choices;
    if (!askCompressPictures(this, tr("Apply to all pictures on this sheet"), &choices)) return;
    QString problems;
    const qint64 saved = compressPictures(choices.ppi, choices.all, choices.deleteCropped, &problems);
    if (!problems.isEmpty()) notice(tr("Compress Pictures"), problems);
    tellCompressSaved(this, saved);
}

namespace {

// A text box of a drawing (not another shape that has words): its words are laid out turned, and the page paints them so.
bool isTextBoxElement(const QDomElement &sp)
{
    return sp.localName() == QLatin1String("sp") && childNamed(childNamed(sp, QStringLiteral("nvSpPr")), QStringLiteral("cNvSpPr")).attribute(QStringLiteral("txBox")) == QLatin1String("1");
}

TextDirection directionOfBody(const QDomElement &sp)
{
    const QDomElement props = childNamed(childNamed(sp, QStringLiteral("txBody")), QStringLiteral("bodyPr"));
    return textDirectionFromVert(props.attribute(QStringLiteral("vert"))).value_or(TextDirection::Horizontal);
}

} // namespace

bool CrunchWindow::pickedTextBoxDirection(TextDirection *direction)
{
    const int id = pickedObject();
    if (id < 0 || !m_objects || pickedObjects().size() != 1 || m_objects->kindOf(m_sheet, id) != ObjectKind::Shape) return false;
    crunch::ObjectClip clip;
    if (!m_objects->copyElement(m_sheet, id, &clip, nullptr)) return false;
    const QByteArray wrapped = "<w xmlns:xdr=\"" + kXdr.toUtf8() + "\" xmlns:a=\"" + kA.toUtf8() + "\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"" + clip.namespaces + ">" + clip.elementXml + "</w>";
    QDomDocument doc;
    if (!doc.setContent(wrapped, QDomDocument::ParseOption::UseNamespaceProcessing)) return false;
    const QDomElement sp = doc.documentElement().firstChildElement();
    if (!isTextBoxElement(sp) || childNamed(childNamed(sp, QStringLiteral("txBody")), QStringLiteral("bodyPr")).isNull()) return false;
    if (direction) *direction = directionOfBody(sp);
    return true;
}

bool CrunchWindow::setTextBoxDirection(TextDirection direction)
{
    return restyleObject(tr("Text Direction"), [&](QDomElement &el, grid::ObjectBox *) {
        if (!isTextBoxElement(el)) return false;
        QDomElement props = childNamed(childNamed(el, QStringLiteral("txBody")), QStringLiteral("bodyPr"));
        if (props.isNull() || directionOfBody(el) == direction) return false;   // (a value that is none of ours reads as across, and is kept as it was)
        if (direction == TextDirection::Horizontal) props.removeAttribute(QStringLiteral("vert"));
        else props.setAttribute(QStringLiteral("vert"), QString::fromLatin1(textDirectionVert(direction)));
        return true;
    });
}

bool CrunchWindow::setObjectText(const ShapeTextChoices &choices)
{
    return restyleObject(tr("Edit Text"), [&](QDomElement &el, grid::ObjectBox *) { return setText(el, choices); });
}

bool CrunchWindow::beginShapeTextEdit(int id, const QString &typed)
{
    if (m_shapeEditor || !m_session || !m_objects || !m_view || m_chartSheetShown || m_edit.active || !editingAllowed()) return false;
    if (id < 0 || id != pickedObject() || pickedObjects().size() != 1 || m_objects->kindOf(m_sheet, id) != ObjectKind::Shape || pickedIsTextArt()) return false;
    crunch::ObjectClip clip;
    QString why;
    // (what can't be told is told when the person asked for the edit; a key typed that does not begin one goes on to the cells)
    auto refuse = [&](const QString &text) {
        if (typed.isEmpty()) {
            m_shapeTextRefused = true;
            notice(tr("Edit Text"), text);
        }
        return false;
    };
    if (!m_objects->copyElement(m_sheet, id, &clip, &why)) return refuse(why.isEmpty() ? tr("This shape's text can't be edited here.") : why);
    const QByteArray wrapped = "<w xmlns:xdr=\"" + kXdr.toUtf8() + "\" xmlns:a=\"" + kA.toUtf8() + "\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"" + clip.namespaces
                               + ">" + clip.elementXml + "</w>";
    QDomDocument dom;
    if (!dom.setContent(wrapped, QDomDocument::ParseOption::UseNamespaceProcessing)) return false;
    const QDomElement sp = dom.documentElement().firstChildElement();
    if (sp.localName() != QLatin1String("sp")) return false;
    if (!childNamed(sp, QStringLiteral("txBody")).isNull() && !wordsFitTheEditor(childNamed(sp, QStringLiteral("txBody")))) return refuse(tr("This shape's text can't be edited here: it holds more than the editor can keep (a link, an equation, or very many paragraphs)."));
    const QDomElement geometry = firstNamed(sp, QStringLiteral("prstGeom"));
    static const QStringList linear{QStringLiteral("line"), QStringLiteral("straightConnector1")};
    if (!geometry.isNull() && (linear.contains(geometry.attribute(QStringLiteral("prst"))) || geometry.attribute(QStringLiteral("prst")).contains(QLatin1String("Connector")))) return false;
    const QRect frame = m_objects->layer()->deviceRect(id, grid::Pane::Body);
    if (frame.isEmpty()) return false;
    auto story = std::make_unique<QTextDocument>();
    QFont base = story->defaultFont();
    base.setPointSizeF(11);
    story->setDefaultFont(base);
    const QDomElement body = childNamed(sp, QStringLiteral("txBody"));
    ShapeTextLook look;
    const double zoom = m_view->zoom();
    if (!body.isNull()) {
        jp::office::DrawingContext context;
        context.theme = m_objects->theme();   // (the workbook's own colors: a theme color stays one)
        jp::office::readTextBody(body, story.get(), context);
        const QDomElement props = childNamed(body, QStringLiteral("bodyPr"));
        if (!props.isNull()) {
            const QString anchor = props.attribute(QStringLiteral("anchor"));
            look.anchor = anchor == QLatin1String("ctr") ? Qt::AlignVCenter : anchor == QLatin1String("b") ? Qt::AlignBottom : Qt::AlignTop;
            look.wrap = props.attribute(QStringLiteral("wrap")) != QLatin1String("none");
            look.autofit = !childNamed(props, QStringLiteral("normAutofit")).isNull();
            auto inset = [&](const char *name, double points) {
                const QString v = props.attribute(QString::fromLatin1(name));
                return (v.isEmpty() ? points : std::clamp(v.toDouble() / 12700.0, 0.0, 1000.0)) * zoom * 96.0 / 72.0;
            };
            look.insets = QMarginsF(inset("lIns", 7.2), inset("tIns", 3.6), inset("rIns", 7.2), inset("bIns", 3.6));
            // (Words that run turned are typed turned: the editor lays them out and paints them as the page does.)
            if (isTextBoxElement(sp)) look.quarterTurns = textDirectionQuarterTurns(directionOfBody(sp));
        }
    }
    look.zoom = zoom;
    look.textColor = styleTextColor(sp, m_objects->theme());
    m_shapeEditor = new ShapeTextEdit(m_view->viewport());
    if (jp::spellAsYouType()) m_shapeEditor->setSpellCheck([](QTextDocument *d) { return jp::misspellings(d); });
    connect(m_shapeEditor, &ShapeTextEdit::formatChanged, this, [this] { reflectShapeEditor(); });
    m_shapeEditorId = id;
    connect(m_shapeEditor, &ShapeTextEdit::finished, this, [this] { endShapeTextEdit(true); });
    connect(m_shapeEditor, &ShapeTextEdit::focusLeft, this, [this] {
        // (A click on the cells puts the words in the shape; a box or a button of the ribbon does not, since its command is for the words.)
        QTimer::singleShot(0, this, [this] {
            const QWidget *now = QApplication::focusWidget();
            if (m_shapeEditor && now && (now == m_view || now == m_view->viewport())) endShapeTextEdit(true);
        });
    });
    m_shapeEditor->begin(std::move(story), frame, look, typed);
    reflectShapeEditor();
    return true;
}

void CrunchWindow::endShapeTextEdit(bool keep)
{
    if (!m_shapeEditor) return;
    ShapeTextEdit *editor = m_shapeEditor;
    const bool changed = editor->changed();
    std::unique_ptr<QTextDocument> story = editor->take();
    m_shapeEditor = nullptr;
    const int id = m_shapeEditorId;
    m_shapeEditorId = -1;
    editor->disconnect(this);
    editor->deleteLater();
    // (the words go in the shape they were typed in, whatever is picked by now: the click that ended the edit picked something else, or nothing, and that stays so)
    if (keep && changed && story && id >= 0 && m_objects && !m_objects->isDeleted(m_sheet, id)) {
        const int pickedNow = pickedObject();
        const jp::office::Theme &theme = m_objects->theme();
        restyleObject(tr("Edit Text"), [&](QDomElement &el, grid::ObjectBox *) { return setTextStory(el, story.get(), theme); }, false, id);
        if (pickedNow != id && m_objects->layer()->selectedId() != pickedNow) m_objects->layer()->select(pickedNow);
    }
    m_view->setFocus(Qt::OtherFocusReason);
    m_view->viewport()->update();
}

// The look of the words being typed in a shape, on the ribbon's buttons and boxes (not the look of the cell under the shape).
void CrunchWindow::reflectShapeEditor()
{
    if (!m_shapeEditor) return;
    const QTextCharFormat cf = m_shapeEditor->charFormat();
    auto check = [this](const char *id, bool on) {
        if (QAction *a = act(QString::fromLatin1(id))) a->setChecked(on);
    };
    check("fmt.bold", cf.fontWeight() >= QFont::DemiBold);
    check("fmt.italic", cf.fontItalic());
    check("fmt.underline", cf.underlineStyle() != QTextCharFormat::NoUnderline);
    const Qt::Alignment al = m_shapeEditor->textCursor().block().blockFormat().alignment() & Qt::AlignHorizontal_Mask;
    check("fmt.alignLeft", al == Qt::AlignLeft || al == 0 || al == Qt::AlignLeading);
    check("fmt.alignCenter", al == Qt::AlignHCenter);
    check("fmt.alignRight", al == Qt::AlignRight || al == Qt::AlignTrailing);
    const double size = m_shapeEditor->fontSizePt();
    QString family = cf.fontFamilies().toStringList().value(0);
    if (family.isEmpty()) family = m_shapeEditor->document()->defaultFont().family();
    for (const auto &c : m_fontCombos)
        if (c) c->setCurrentFamily(family);
    for (const auto &c : m_sizeCombos)
        if (c) c->setSize(size > 0 ? size : 11);
}

void CrunchWindow::focusText()
{
    if (m_shapeEditor) m_shapeEditor->setFocus(Qt::OtherFocusReason);
    else m_view->setFocus();
}

// Edit Text: the words in the shape or text box, how they sit, and their size and color.
void CrunchWindow::editObjectText()
{
    const int id = pickedObject();
    if (id < 0 || !m_objects) return;
    const ObjectKind kind = m_objects->kindOf(m_sheet, id);
    if (kind != ObjectKind::Shape) {
        notice(tr("Edit Text"), tr("Text goes in a shape or a text box. Pick one first."));
        return;
    }
    if (editTextArtText()) return;
    if (!m_shapeTextAsker && beginShapeTextEdit(id)) return;   // (the words are typed in place, over the shape; a window with its own way of asking, as a test's, is asked)
    ShapeTextChoices c;
    crunch::ObjectClip clip;
    if (m_objects->copyElement(m_sheet, id, &clip, nullptr)) {
        const QByteArray wrapped = "<w xmlns:xdr=\"" + kXdr.toUtf8() + "\" xmlns:a=\"" + kA.toUtf8() + "\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"" + clip.namespaces
                                   + ">" + clip.elementXml + "</w>";
        QDomDocument doc;
        if (doc.setContent(wrapped, QDomDocument::ParseOption::UseNamespaceProcessing)) {
            const QDomElement sp = doc.documentElement().firstChildElement();
            c.text = textOf(sp);
            const QDomElement body = childNamed(sp, QStringLiteral("txBody"));
            const QDomElement bodyPr = body.isNull() ? QDomElement() : childNamed(body, QStringLiteral("bodyPr"));
            if (!bodyPr.isNull()) {
                const QString anchor = bodyPr.attribute(QStringLiteral("anchor"));
                c.vertical = anchor == QLatin1String("ctr") ? 1 : anchor == QLatin1String("b") ? 2 : 0;
                c.wrap = bodyPr.attribute(QStringLiteral("wrap")) != QLatin1String("none");
            }
            const QDomElement p = body.isNull() ? QDomElement() : childNamed(body, QStringLiteral("p"));
            if (!p.isNull()) {
                const QDomElement pPr = childNamed(p, QStringLiteral("pPr"));
                const QString algn = pPr.isNull() ? QString() : pPr.attribute(QStringLiteral("algn"));
                c.horizontal = algn == QLatin1String("ctr") ? 1 : algn == QLatin1String("r") ? 2 : 0;
                QDomElement rPr = firstNamed(p, QStringLiteral("rPr"));
                if (rPr.isNull()) rPr = firstNamed(p, QStringLiteral("endParaRPr"));
                if (!rPr.isNull()) {
                    const int sz = rPr.attribute(QStringLiteral("sz")).toInt();
                    if (sz >= 100) c.sizePoints = sz / 100;
                    c.bold = rPr.attribute(QStringLiteral("b")) == QLatin1String("1");
                    const QDomElement clr = firstNamed(rPr, QStringLiteral("srgbClr"));
                    if (!clr.isNull()) c.colorHex = clr.attribute(QStringLiteral("val"));
                }
            }
        }
    }
    if (m_shapeTextAsker) {
        if (!m_shapeTextAsker(&c)) return;
    } else {
        QDialog dialog(this);
        dialog.setWindowTitle(tr("Edit Text"));
        auto *layout = new QVBoxLayout(&dialog);
        auto *edit = new QPlainTextEdit(c.text, &dialog);
        edit->setObjectName(QStringLiteral("shapeText"));
        edit->setAccessibleName(tr("Text"));
        edit->setMinimumSize(320, 120);
        layout->addWidget(edit);
        auto *form = new QFormLayout;
        auto *horizontal = new QComboBox(&dialog);
        horizontal->addItems({tr("Left"), tr("Center"), tr("Right")});
        horizontal->setCurrentIndex(c.horizontal);
        horizontal->setAccessibleName(tr("Across"));
        form->addRow(tr("Across"), horizontal);
        auto *vertical = new QComboBox(&dialog);
        vertical->addItems({tr("Top"), tr("Middle"), tr("Bottom")});
        vertical->setCurrentIndex(c.vertical);
        vertical->setAccessibleName(tr("Up and down"));
        form->addRow(tr("Up and down"), vertical);
        auto *size = new QSpinBox(&dialog);
        size->setRange(1, 400);
        size->setValue(c.sizePoints);
        size->setSuffix(tr(" pt"));
        size->setAccessibleName(tr("Size"));
        form->addRow(tr("Size"), size);
        auto *bold = new QComboBox(&dialog);
        bold->addItems({tr("Regular"), tr("Bold")});
        bold->setCurrentIndex(c.bold ? 1 : 0);
        bold->setAccessibleName(tr("Weight"));
        form->addRow(tr("Weight"), bold);
        auto *wrap = new QComboBox(&dialog);
        wrap->addItems({tr("Wrap to the shape"), tr("One line")});
        wrap->setCurrentIndex(c.wrap ? 0 : 1);
        wrap->setAccessibleName(tr("Lines"));
        form->addRow(tr("Lines"), wrap);
        layout->addLayout(form);
        auto *buttons = new jp::ButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
        layout->addWidget(buttons);
        connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
        if (dialog.exec() != QDialog::Accepted) return;
        c.text = edit->toPlainText();
        c.horizontal = horizontal->currentIndex();
        c.vertical = vertical->currentIndex();
        c.sizePoints = size->value();
        c.bold = bold->currentIndex() == 1;
        c.wrap = wrap->currentIndex() == 0;
    }
    setObjectText(c);
}

// The Selection Pane: the sheet's objects by name, the one in front first. Choosing a row picks the object; the buttons move it in front of or behind its neighbor (the shared pane, selectionpane.h;
// a sheet's objects have no hiding or renaming here, so the pane shows neither).
void CrunchWindow::showSelectionPane()
{
    if (!m_objects || !m_session) return;
    if (!m_selectionPane) {
        SelectionPane::Source src;
        src.rows = [this] {
            QVector<SelectionPane::Row> rows;
            if (!m_objects) return rows;
            const std::vector<int> ids = m_objects->idsOf(m_sheet);
            const int picked = pickedObject();
            for (int i = int(ids.size()) - 1; i >= 0; --i) rows.push_back({m_objects->nameOf(m_sheet, ids[size_t(i)]), true, ids[size_t(i)] == picked});
            return rows;
        };
        auto pickRow = [this](int r) {
            if (!m_objects) return;
            const std::vector<int> ids = m_objects->idsOf(m_sheet);
            const int at = int(ids.size()) - 1 - r;
            if (at >= 0 && at < int(ids.size())) m_objects->layer()->select(ids[size_t(at)]);
        };
        src.pick = pickRow;
        src.move = [this, pickRow](int from, int to) {
            pickRow(from);
            for (int step = std::abs(from - to); step > 0; --step) reorderObject(to < from ? 1 : -1);
        };
        auto *pane = new SelectionPane(this, src);
        m_selectionPane = pane;
        connect(m_objects->layer(), &jp::grid::ObjectLayer::selectionChanged, pane, [pane](int) { pane->refresh(); });
    }
    m_selectionPane->refresh();
    m_selectionPane->show();
    m_selectionPane->raise();
}

namespace {

// The ready-made looks of Shape Styles: a fill and an outline each (or none).
struct ShapeStyleKind {
    const char *id;
    const char *text;
    QRgb fill, line;
    bool noFill, noLine;
};
const ShapeStyleKind kShapeStyles[] = {
    {"object.style.blue", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Light Blue"), 0xBDD7EE, 0x2E75B6, false, false},
    {"object.style.green", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Light Green"), 0xC5E0B3, 0x548235, false, false},
    {"object.style.orange", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Light Orange"), 0xF8CBAD, 0xC55A11, false, false},
    {"object.style.gold", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Light Gold"), 0xFFE699, 0xBF9000, false, false},
    {"object.style.gray", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Light Gray"), 0xDBDBDB, 0x7F7F7F, false, false},
    {"object.style.outline", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Outline Only"), 0, 0x2E75B6, true, false},
    {"object.style.plain", QT_TRANSLATE_NOOP("jp::CrunchWindow", "Fill Only"), 0xBDD7EE, 0, false, true},
};

} // namespace

// A ready-made look for the picked shape: its fill and its outline, as one step of Undo.
bool CrunchWindow::applyShapeStyle(int index)
{
    if (index < 0 || index >= int(sizeof(kShapeStyles) / sizeof(kShapeStyles[0])) || pickedObject() < 0 || !m_session) return false;
    const ShapeStyleKind &k = kShapeStyles[index];
    undoStack()->beginMacro(tr("Shape Style"));
    const bool ok = setObjectFill(QColor(k.fill), k.noFill) && setObjectOutline(QColor(k.line), k.noLine);
    undoStack()->endMacro();
    return ok;
}

void CrunchWindow::enableShapeStyles(bool on)
{
    for (const ShapeStyleKind &k : kShapeStyles)
        if (QAction *a = act(QString::fromLatin1(k.id))) a->setEnabled(on);
}

void CrunchWindow::createObjectFormatActions()
{
    for (int i = 0; i < int(sizeof(kShapeStyles) / sizeof(kShapeStyles[0])); ++i)
        mk(QString::fromLatin1(kShapeStyles[i].id), QCoreApplication::translate("jp::CrunchWindow", kShapeStyles[i].text), "paint-bucket", QKeySequence(), [this, i] { applyShapeStyle(i); }, false, false);
    mk("object.selectionPane", tr("Selection Pane"), "layers", QKeySequence(), [this] { showSelectionPane(); });
    m_opsActions << QStringLiteral("object.selectionPane");
    auto mkObject = [this](const char *id, const QString &text, const char *icon, const std::function<void()> &fn) { mk(QString::fromLatin1(id), text, icon, QKeySequence(), fn, false, false); };
    mkObject("object.compress", tr("Compress Pictures…"), "minimize-2", [this] { compressPicturesDialog(); });
    mkObject("object.changePicture", tr("Change Picture…"), "image-up", [this] {
        const QString dir = Settings::get().value(QStringLiteral("dirs/pictures"), QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)).toString();
        const QString path = QFileDialog::getOpenFileName(this, tr("Change Picture"), dir, tr("Pictures (*.png *.jpg *.jpeg *.gif *.bmp *.tif *.tiff *.webp *.svg *.wmf *.emf)") + QStringLiteral(";;") + tr("All Files (*)"));
        if (path.isEmpty()) return;
        Settings::get().setValue(QStringLiteral("dirs/pictures"), QFileInfo(path).absolutePath());
        changePictureFromFile(path);
    });
    mkObject("object.resetPicture", tr("Reset Picture"), "rotate-ccw", [this] { resetPicture(false); });
    mkObject("object.resetPictureSize", tr("Reset Picture & Size"), "rotate-ccw", [this] { resetPicture(true); });
    mkObject("object.editText", tr("Edit Text…"), "text-cursor-input", [this] { editObjectText(); });
    {
        static const std::pair<const char *, TextDirection> directions[] = {{"object.dir.horz", TextDirection::Horizontal}, {"object.dir.r90", TextDirection::Rotate90}, {"object.dir.r270", TextDirection::Rotate270}};
        for (const auto &d : directions) {
            const TextDirection direction = d.second;
            mk(QString::fromLatin1(d.first), textDirectionName(direction), direction == TextDirection::Horizontal ? "text" : "rotate-cw", QKeySequence(), [this, direction] { setTextBoxDirection(direction); }, true, false);
        }
    }
    mkObject("object.rotateRight", tr("Rotate Right 90°"), "rotate-cw", [this] { rotateObject(1, false, false); });
    mkObject("object.rotateLeft", tr("Rotate Left 90°"), "rotate-ccw", [this] { rotateObject(-1, false, false); });
    mkObject("object.flipH", tr("Flip Horizontal"), "flip-horizontal", [this] { rotateObject(0, true, false); });
    mkObject("object.flipV", tr("Flip Vertical"), "flip-vertical", [this] { rotateObject(0, false, true); });
    mkObject("object.cropSquare", tr("Square (1:1)"), "crop", [this] { cropObject(1, 1); });
    mkObject("object.crop43", tr("Landscape 4:3"), "crop", [this] { cropObject(4, 3); });
    mkObject("object.crop34", tr("Portrait 3:4"), "crop", [this] { cropObject(3, 4); });
    mkObject("object.crop169", tr("Wide 16:9"), "crop", [this] { cropObject(16, 9); });
    mkObject("object.crop916", tr("Tall 9:16"), "crop", [this] { cropObject(9, 16); });
    mkObject("object.cropFill", tr("Fill"), "expand", [this] { fillOrFitObject(true); });
    mkObject("object.cropFit", tr("Fit"), "shrink", [this] { fillOrFitObject(false); });
    mkObject("object.cropReset", tr("Show the Whole Picture"), "crop", [this] { cropObject(0, 0); });
    createTextArtActions();
}

void CrunchWindow::objectFormatRibbonParts(RibbonParts &parts)
{
    auto fillButton = [this](const char *name, const QString &tip, const QString &none, bool fill) {
        auto *b = new ColorButton(QStringLiteral("paint-bucket"), tip, true, none);
        b->setObjectName(QString::fromLatin1(name));
        b->setCurrent(ColorRef::rgb(fill ? QColor(0xBD, 0xD7, 0xEE) : QColor(0x2E, 0x75, 0xB6)));
        connect(b, &ColorButton::colorPicked, this, [this, b, fill](const ColorRef &c) {
            b->setCurrent(c);
            const QColor rgb = c.isNone() ? QColor() : c.resolve(workbookColors());
            if (fill) setObjectFill(rgb, c.isNone());
            else setObjectOutline(rgb, c.isNone());
        });
        return b;
    };
    parts.widgets[QStringLiteral("shapeFill")] = [fillButton]() -> QWidget * { return fillButton("object.fillColor", CrunchWindow::tr("Shape Fill"), CrunchWindow::tr("No Fill"), true); };
    parts.widgets[QStringLiteral("cropToShapeButton")] = [this]() -> QWidget * { return cropToShapeButton([this](const QString &id) { cropObjectToShape(id); }, QStringLiteral("cropToShapeButton")); };
    parts.widgets[QStringLiteral("pictureBorder")] = [fillButton]() -> QWidget * { return fillButton("object.pictureBorder", CrunchWindow::tr("Picture Border"), CrunchWindow::tr("No Outline"), false); };
    parts.widgets[QStringLiteral("shapeOutline")] = [fillButton]() -> QWidget * { return fillButton("object.outlineColor", CrunchWindow::tr("Shape Outline"), CrunchWindow::tr("No Outline"), false); };
    parts.widgets[QStringLiteral("pictureStyleGallery")] = [this]() -> QWidget * {
        auto *gal = new Gallery(QSize(60, 46), 6);
        gal->setObjectName(QStringLiteral("pictureStyleGallery"));
        gal->setTitle(tr("Picture Styles"));
        gal->setItemsProvider([this] { return pictureStyleItems(workbookColors()); });
        connect(gal, &Gallery::activated, this, [this](const QString &id) { applyPictureStyle(id.toInt()); });
        return gal;
    };
    parts.menus[QStringLiteral("pictureEffects")] = [this](QWidget *owner) {
        auto *menu = new QMenu(owner);
        menu->setObjectName(QStringLiteral("pictureEffectsMenu"));
        fillPictureEffectsMenu(menu, [this](const QString &, const EffectChange &change) { setPictureEffects(change); });
        return menu;
    };
    parts.menus[QStringLiteral("outlineStyle")] = [this](QWidget *owner) {
        auto *menu = new QMenu(owner);
        menu->setObjectName(QStringLiteral("outlineStyleMenu"));
        QMenu *weights = menu->addMenu(tr("Weight"));
        for (const double w : {0.5, 1.0, 1.5, 2.25, 3.0, 4.5, 6.0}) {
            QAction *a = weights->addAction(tr("%1 pt").arg(QLocale().toString(w)));
            connect(a, &QAction::triggered, this, [this, w] { setObjectOutlineStyle(w, QString()); });
        }
        QMenu *dashes = menu->addMenu(tr("Dashes"));
        const struct {
            const char *name;
            const char *value;
        } kinds[] = {{QT_TR_NOOP("Solid"), "solid"}, {QT_TR_NOOP("Dashed"), "dash"}, {QT_TR_NOOP("Dotted"), "sysDot"}};
        for (const auto &k : kinds) {
            QAction *a = dashes->addAction(tr(k.name));
            const QString value = QString::fromLatin1(k.value);
            connect(a, &QAction::triggered, this, [this, value] { setObjectOutlineStyle(0, value); });
        }
        return menu;
    };
    textArtRibbonParts(parts);
    parts.colors[QStringLiteral("pictureFormat")] = QColor(0x8A, 0x4B, 0x08);
    parts.colors[QStringLiteral("shapeFormat")] = QColor(0x6B, 0x2D, 0x8E);
}

} // namespace jp
