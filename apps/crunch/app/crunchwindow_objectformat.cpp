// Format of the picture or shape that is picked: fill, outline, rotation, crop, and the text in a shape. The element the drawing holds for the object is copied, changed, and put back as
// the object (the old one taken away and the new one made in its place, with its name, alternative text, place, and order among the objects), as one step of Undo. What a file's object holds
// that these commands do not touch stays in the copy as it was.
#include "app/crunchwindow.h"

#include "app/compresspictures.h"
#include "app/icons.h"
#include "app/plaintext.h"
#include "app/ribbonbuilder.h"
#include "core/objectclip.h"
#include "core/sheetobjects.h"

#include <QAction>
#include <QComboBox>
#include <QDialog>
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

} // namespace

// Changes the element of the picked object with `edit` (which may also read what the object is, and says false when there is nothing to change), and puts the result in its place.
bool CrunchWindow::restyleObject(const QString &stepText, const std::function<bool(QDomElement &, grid::ObjectBox *)> &edit, bool pictureOnly)
{
    const int id = pickedObject();
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
            const double aspect = box->w / std::max(1.0, box->h), target = double(width) / double(height);
            next = now;
            if (aspect > target) {
                const double f = target / aspect;   // the share of the width that stays
                const qint64 cut = qint64(std::llround(showW * (1.0 - f) * 100000.0 / 2.0));
                next.l += cut;
                next.r += cut;
                box->w *= f;
            } else if (aspect < target) {
                const double f = aspect / target;
                const qint64 cut = qint64(std::llround(showH * (1.0 - f) * 100000.0 / 2.0));
                next.t += cut;
                next.b += cut;
                box->h *= f;
            }
        }
        box->w = std::max(4.0, std::round(box->w));
        box->h = std::max(4.0, std::round(box->h));
        return setCrop(el, next);
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
    if (problems && skipped > 0) *problems = tr("%n picture(s) could not be made smaller: %1", nullptr, skipped).arg(skippedWhy);
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

bool CrunchWindow::setObjectText(const ShapeTextChoices &choices)
{
    return restyleObject(tr("Edit Text"), [&](QDomElement &el, grid::ObjectBox *) { return setText(el, choices); });
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
        form->addRow(tr("Across:"), horizontal);
        auto *vertical = new QComboBox(&dialog);
        vertical->addItems({tr("Top"), tr("Middle"), tr("Bottom")});
        vertical->setCurrentIndex(c.vertical);
        vertical->setAccessibleName(tr("Up and down"));
        form->addRow(tr("Up and down:"), vertical);
        auto *size = new QSpinBox(&dialog);
        size->setRange(1, 400);
        size->setValue(c.sizePoints);
        size->setSuffix(tr(" pt"));
        size->setAccessibleName(tr("Size"));
        form->addRow(tr("Size:"), size);
        auto *bold = new QComboBox(&dialog);
        bold->addItems({tr("Regular"), tr("Bold")});
        bold->setCurrentIndex(c.bold ? 1 : 0);
        bold->setAccessibleName(tr("Weight"));
        form->addRow(tr("Weight:"), bold);
        auto *wrap = new QComboBox(&dialog);
        wrap->addItems({tr("Wrap to the shape"), tr("One line")});
        wrap->setCurrentIndex(c.wrap ? 0 : 1);
        wrap->setAccessibleName(tr("Lines"));
        form->addRow(tr("Lines:"), wrap);
        layout->addLayout(form);
        auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
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
    mkObject("object.editText", tr("Edit Text…"), "text-cursor-input", [this] { editObjectText(); });
    mkObject("object.rotateRight", tr("Rotate Right 90°"), "rotate-cw", [this] { rotateObject(1, false, false); });
    mkObject("object.rotateLeft", tr("Rotate Left 90°"), "rotate-ccw", [this] { rotateObject(-1, false, false); });
    mkObject("object.flipH", tr("Flip Horizontal"), "flip-horizontal", [this] { rotateObject(0, true, false); });
    mkObject("object.flipV", tr("Flip Vertical"), "flip-vertical", [this] { rotateObject(0, false, true); });
    mkObject("object.cropSquare", tr("Square (1:1)"), "crop", [this] { cropObject(1, 1); });
    mkObject("object.crop43", tr("Landscape 4:3"), "crop", [this] { cropObject(4, 3); });
    mkObject("object.crop34", tr("Portrait 3:4"), "crop", [this] { cropObject(3, 4); });
    mkObject("object.crop169", tr("Wide 16:9"), "crop", [this] { cropObject(16, 9); });
    mkObject("object.crop916", tr("Tall 9:16"), "crop", [this] { cropObject(9, 16); });
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
    parts.widgets[QStringLiteral("pictureBorder")] = [fillButton]() -> QWidget * { return fillButton("object.pictureBorder", CrunchWindow::tr("Picture Border"), CrunchWindow::tr("No Outline"), false); };
    parts.widgets[QStringLiteral("shapeOutline")] = [fillButton]() -> QWidget * { return fillButton("object.outlineColor", CrunchWindow::tr("Shape Outline"), CrunchWindow::tr("No Outline"), false); };
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
