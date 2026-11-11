// JeffCrunch's Text Art: Insert > Text Art puts words in one of the shared gallery's styles on the sheet, bent into the style's warp. It is a shape of the drawing, as the file format stores
// Text Art: a text box whose body says it was made as Text Art and names the warp (a:prstTxWarp), with the words in letters that have the style's fill and outline. That is what the shared
// reader (libs/office/office/drawingml_shape.cpp) shows as Text Art, so a workbook that has some, made here or elsewhere, is drawn the same. Each change (a new one, other words, another
// style, another warp) is made as every change to a shape is (core/sheetobjects.h: the object is made again, one step of Undo), so the saved file has the shape as it was changed.
#include "app/crunchwindow.h"

#include "app/icons.h"
#include "app/ribbonbuilder.h"
#include "app/textartgallery.h"
#include "app/textdialogs.h"
#include "app/widgets.h"
#include "core/objectclip.h"
#include "core/sheetobjects.h"
#include "office/drawingml_write.h"
#include "render/textart.h"

#include <QAction>
#include <QActionGroup>
#include <QBuffer>
#include <QDomDocument>
#include <QMenu>
#include <QSet>
#include <QXmlStreamWriter>

#include <algorithm>
#include <cmath>

namespace jp {

using namespace jp::office::sheet;

namespace {

const QString kA = QStringLiteral("http://schemas.openxmlformats.org/drawingml/2006/main");
const QString kXdr = QStringLiteral("http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing");

QDomElement childNamed(const QDomElement &e, const QString &local)
{
    for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement())
        if (c.localName() == local) return c;
    return QDomElement();
}

// The element of a shape as XML a drawing holds: written by the shared writer (as for a presentation), with the spreadsheet drawing's prefix for the shape's own parts.
QByteArray textArtXml(const TextArtItem &art)
{
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    QXmlStreamWriter w(&buffer);
    office::DrawingWriteContext ctx;
    office::writeShape(w, art, ctx);
    buffer.close();
    bytes.replace("<p:", "<xdr:").replace("</p:", "</xdr:");
    bytes.replace("<xdr:nvPr/>", "");
    return bytes;
}

// Where a child of a run's properties goes (a:rPr, a:endParaRPr): the order the schema gives, the outline, the fill, the effects, then the fonts. Programs that read these strictly refuse a
// file with them in another order.
int propertyRank(const QString &local)
{
    static const QStringList order = {QStringLiteral("ln"),        QStringLiteral("fill"),      QStringLiteral("effect"),  QStringLiteral("highlight"), QStringLiteral("uLnTx"),
                                      QStringLiteral("uLn"),       QStringLiteral("uFillTx"),   QStringLiteral("uFill"),   QStringLiteral("latin"),     QStringLiteral("ea"),
                                      QStringLiteral("cs"),        QStringLiteral("sym"),       QStringLiteral("hlinkClick"), QStringLiteral("hlinkMouseOver"), QStringLiteral("rtl"),
                                      QStringLiteral("extLst")};
    static const QStringList fills = {QStringLiteral("noFill"), QStringLiteral("solidFill"), QStringLiteral("gradFill"), QStringLiteral("blipFill"), QStringLiteral("pattFill"), QStringLiteral("grpFill")};
    static const QStringList effects = {QStringLiteral("effectLst"), QStringLiteral("effectDag")};
    return int(order.indexOf(fills.contains(local) ? QStringLiteral("fill") : effects.contains(local) ? QStringLiteral("effect") : local));
}

// Puts `child` in the run's properties where the schema has it: after the last child that comes before it, and before the first that comes after.
void insertProperty(QDomElement &look, const QDomElement &child)
{
    const int rank = propertyRank(child.localName());
    for (QDomElement c = look.firstChildElement(); !c.isNull(); c = c.nextSiblingElement())
        if (propertyRank(c.localName()) > rank) {
            look.insertBefore(child, c);
            return;
        }
    look.appendChild(child);
}

bool isTextArt(const QDomElement &sp)
{
    if (sp.localName() != QLatin1String("sp")) return false;
    const QDomElement body = childNamed(sp, QStringLiteral("txBody"));
    const QDomElement props = body.isNull() ? QDomElement() : childNamed(body, QStringLiteral("bodyPr"));
    if (props.isNull()) return false;
    const QString made = props.attribute(QStringLiteral("fromWordArt"));
    const QDomElement warp = childNamed(props, QStringLiteral("prstTxWarp"));
    return (made == QLatin1String("1") || made == QLatin1String("true")) && !warp.isNull() && warp.attribute(QStringLiteral("prst")) != QLatin1String("textNoShape");
}

QList<QDomElement> paragraphsOf(const QDomElement &body)
{
    QList<QDomElement> out;
    for (QDomElement p = body.firstChildElement(); !p.isNull(); p = p.nextSiblingElement())
        if (p.localName() == QLatin1String("p")) out << p;
    return out;
}

// The words, one line a paragraph (a break inside a paragraph is a line of its own).
QString wordsOf(const QDomElement &sp)
{
    QStringList lines;
    for (const QDomElement &p : paragraphsOf(childNamed(sp, QStringLiteral("txBody")))) {
        QString line;
        for (QDomElement r = p.firstChildElement(); !r.isNull(); r = r.nextSiblingElement()) {
            if (r.localName() == QLatin1String("br")) line += QLatin1Char('\n');
            else if (r.localName() == QLatin1String("r") || r.localName() == QLatin1String("fld")) line += childNamed(r, QStringLiteral("t")).text();
        }
        lines << line;
    }
    return lines.join(QLatin1Char('\n'));
}

// The first run's properties (the look of the letters), or the end of the first paragraph's.
QDomElement firstLook(const QDomElement &sp)
{
    const QList<QDomElement> ps = paragraphsOf(childNamed(sp, QStringLiteral("txBody")));
    if (ps.isEmpty()) return QDomElement();
    for (QDomElement r = ps.first().firstChildElement(); !r.isNull(); r = r.nextSiblingElement())
        if (r.localName() == QLatin1String("r")) {
            const QDomElement look = childNamed(r, QStringLiteral("rPr"));
            if (!look.isNull()) return look;
        }
    return childNamed(ps.first(), QStringLiteral("endParaRPr"));
}

// Every run's and paragraph end's properties in the shape.
QList<QDomElement> looksOf(const QDomElement &sp)
{
    QList<QDomElement> out;
    for (const QDomElement &p : paragraphsOf(childNamed(sp, QStringLiteral("txBody"))))
        for (QDomElement c = p.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) {
            if (c.localName() == QLatin1String("endParaRPr")) out << c;
            else if (c.localName() == QLatin1String("r")) {
                const QDomElement look = childNamed(c, QStringLiteral("rPr"));
                if (!look.isNull()) out << look;
            }
        }
    return out;
}

} // namespace

// Insert > Text > Text Art: the words "Your Text Here" in the style, in a box a little wider than four times its height, put where the next object goes, as one step.
bool CrunchWindow::insertTextArt(const QString &styleId)
{
    if (!m_session || !m_objects || m_chartSheetShown) return false;
    auto art = newTextArt(styleId);
    art->text = tr("Your Text Here");
    art->size = 40;
    art->rect = QRectF(0, 0, 240, 60);   // (points: 320 by 80 on the grid)
    // a name no other object of the sheet has
    QSet<QString> used;
    for (const crunch::SheetObjects::ObjectInfo &info : m_objects->describe(m_sheet)) used.insert(info.name);
    int n = 1;
    while (used.contains(tr("Text Art %1").arg(n))) ++n;
    art->name = tr("Text Art %1").arg(n);
    crunch::ObjectClip clip;
    clip.kind = QStringLiteral("shape");
    clip.width = 320;
    clip.height = 80;
    clip.elementXml = textArtXml(*art);
    if (!placeObjectClip(clip, tr("Insert Text Art"))) return false;
    setStatus(tr("Text Art inserted"));
    return true;
}

// Whether the picked object is Text Art (of a drawing: one a file has, or one put here).
bool CrunchWindow::pickedIsTextArt()
{
    const int id = pickedObject();
    if (id < 0 || !m_objects || m_objects->kindOf(m_sheet, id) != ObjectKind::Shape) return false;
    crunch::ObjectClip clip;
    if (!m_objects->copyElement(m_sheet, id, &clip, nullptr)) return false;
    const QByteArray wrapped = "<w xmlns:xdr=\"" + kXdr.toUtf8() + "\" xmlns:a=\"" + kA.toUtf8() + "\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"" + clip.namespaces + ">"
                               + clip.elementXml + "</w>";
    QDomDocument doc;
    if (!doc.setContent(wrapped, QDomDocument::ParseOption::UseNamespaceProcessing)) return false;
    return isTextArt(doc.documentElement().firstChildElement());
}

// Edit Text for Text Art: its words, font, size, bold, and italic in the shared box. The letters keep their fill and outline.
bool CrunchWindow::editTextArtText()
{
    if (!pickedIsTextArt()) return false;
    const int id = pickedObject();
    crunch::ObjectClip clip;
    if (!m_objects->copyElement(m_sheet, id, &clip, nullptr)) return false;
    const QByteArray wrapped = "<w xmlns:xdr=\"" + kXdr.toUtf8() + "\" xmlns:a=\"" + kA.toUtf8() + "\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"" + clip.namespaces + ">"
                               + clip.elementXml + "</w>";
    QDomDocument doc;
    if (!doc.setContent(wrapped, QDomDocument::ParseOption::UseNamespaceProcessing)) return false;
    const QDomElement sp = doc.documentElement().firstChildElement();
    TextArtItem art;
    art.text = wordsOf(sp);
    const QDomElement look = firstLook(sp);
    art.size = 36;
    if (!look.isNull()) {
        const int sz = look.attribute(QStringLiteral("sz")).toInt();
        if (sz >= 100 && sz <= 400000) art.size = sz / 100.0;   // (a size that is none of the sizes there are is not one to scale by)
        art.bold = look.attribute(QStringLiteral("b")) == QLatin1String("1");
        art.italic = look.attribute(QStringLiteral("i")) == QLatin1String("1");
        art.font = childNamed(look, QStringLiteral("latin")).attribute(QStringLiteral("typeface"));
    }
    const double was = art.size;
    if (m_textArtAsker) {
        if (!m_textArtAsker(&art)) return true;
    } else if (!askTextArtText(this, &art)) {
        return true;
    }
    restyleObject(tr("Edit Text Art Text"), [&](QDomElement &el, grid::ObjectBox *box) {
        if (!isTextArt(el)) return false;
        QDomDocument d = el.ownerDocument();
        QDomElement body = childNamed(el, QStringLiteral("txBody"));
        const QList<QDomElement> old = paragraphsOf(body);
        if (old.isEmpty()) return false;
        const QDomElement paragraph = old.first().cloneNode(true).toElement();
        for (const QDomElement &p : old) body.removeChild(p);
        for (const QString &line : art.text.split(QLatin1Char('\n'))) {
            QDomElement p = paragraph.cloneNode(true).toElement();
            QDomElement keep;
            QList<QDomElement> drop;
            for (QDomElement c = p.firstChildElement(); !c.isNull(); c = c.nextSiblingElement())
                if (c.localName() == QLatin1String("r") && keep.isNull() && !line.isEmpty()) keep = c;
                else if (c.localName() == QLatin1String("r") || c.localName() == QLatin1String("br") || c.localName() == QLatin1String("fld")) drop << c;
            for (const QDomElement &c : drop) p.removeChild(c);
            if (keep.isNull() && !line.isEmpty()) {
                // (a paragraph that had no run: one is made from the paragraph end's properties)
                keep = d.createElementNS(kA, QStringLiteral("a:r"));
                const QDomElement end = childNamed(p, QStringLiteral("endParaRPr"));
                if (!end.isNull()) {
                    QDomElement rPr = end.cloneNode(true).toElement();
                    rPr.setTagName(QStringLiteral("a:rPr"));
                    keep.appendChild(rPr);
                }
                const QDomElement end2 = childNamed(p, QStringLiteral("endParaRPr"));
                if (end2.isNull()) p.appendChild(keep);
                else p.insertBefore(keep, end2);
            }
            if (!keep.isNull()) {
                QDomElement t = childNamed(keep, QStringLiteral("t"));
                if (t.isNull()) {
                    t = d.createElementNS(kA, QStringLiteral("a:t"));
                    keep.appendChild(t);
                }
                while (!t.firstChild().isNull()) t.removeChild(t.firstChild());
                t.appendChild(d.createTextNode(line));
            }
            body.appendChild(p);
        }
        for (QDomElement look : looksOf(el)) {
            look.setAttribute(QStringLiteral("sz"), qint64(std::llround(std::clamp(art.size, 4.0, 400.0) * 100)));
            look.setAttribute(QStringLiteral("b"), art.bold ? QStringLiteral("1") : QStringLiteral("0"));
            look.setAttribute(QStringLiteral("i"), art.italic ? QStringLiteral("1") : QStringLiteral("0"));
            if (!art.font.isEmpty()) {
                QDomElement latin = childNamed(look, QStringLiteral("latin"));
                if (latin.isNull()) {
                    latin = d.createElementNS(kA, QStringLiteral("a:latin"));
                    insertProperty(look, latin);
                }
                latin.setAttribute(QStringLiteral("typeface"), art.font);
            }
        }
        // A new size scales the box, about its middle.
        if (std::abs(was - art.size) > 0.1 && was > 0) {
            const double k = std::clamp(art.size / was, 0.1, 10.0);
            const double w = std::clamp(std::round(box->w * k), 4.0, 20000.0), h = std::clamp(std::round(box->h * k), 4.0, 20000.0);
            box->x -= (w - box->w) / 2;
            box->y -= (h - box->h) / 2;
            box->w = w;
            box->h = h;
        }
        return true;
    });
    return true;
}

// Text Art Styles: the picked Text Art's letters take the style's fill, outline, shadow, and font; its words, size, and warp stay.
bool CrunchWindow::setTextArtStyle(const QString &styleId)
{
    bool known = false;
    for (const TextArtStyle &st : textArtStyles()) known = known || st.id == styleId;
    if (!known) return false;
    bool art = true;
    // The letters of a new Text Art of the style, as the writer makes them: the look to give the picked one's runs.
    auto styled = newTextArt(styleId);
    styled->text = QStringLiteral("A");
    styled->rect = QRectF(0, 0, 10, 10);
    const QByteArray wrapped = "<w xmlns:xdr=\"" + kXdr.toUtf8() + "\" xmlns:a=\"" + kA.toUtf8() + "\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\">" + textArtXml(*styled) + "</w>";
    QDomDocument model;
    if (!model.setContent(wrapped, QDomDocument::ParseOption::UseNamespaceProcessing)) return false;
    const QDomElement want = firstLook(model.documentElement().firstChildElement());
    if (want.isNull()) return false;
    const bool done = restyleObject(tr("Text Art Styles"), [&](QDomElement &el, grid::ObjectBox *) {
        art = isTextArt(el);
        if (!art) return false;
        static const QStringList looks = {QStringLiteral("ln"),       QStringLiteral("noFill"),    QStringLiteral("solidFill"), QStringLiteral("gradFill"), QStringLiteral("blipFill"),
                                          QStringLiteral("pattFill"), QStringLiteral("grpFill"),   QStringLiteral("effectLst"), QStringLiteral("effectDag"), QStringLiteral("latin")};
        QDomDocument d = el.ownerDocument();
        for (QDomElement look : looksOf(el)) {
            for (QDomElement c = look.firstChildElement(); !c.isNull();) {
                const QDomElement next = c.nextSiblingElement();
                if (looks.contains(c.localName())) look.removeChild(c);
                c = next;
            }
            look.setAttribute(QStringLiteral("b"), want.attribute(QStringLiteral("b"), QStringLiteral("0")));
            look.setAttribute(QStringLiteral("i"), want.attribute(QStringLiteral("i"), QStringLiteral("0")));
            for (QDomElement c = want.firstChildElement(); !c.isNull(); c = c.nextSiblingElement())
                if (looks.contains(c.localName())) {
                    QDomElement made = d.importNode(c, true).toElement();
                    insertProperty(look, made);
                }
        }
        return true;
    });
    if (!art) notice(tr("Text Art Styles"), tr("Text Art styles are for Text Art. Pick one first."));
    return done;
}

// Text Warp: the picked Text Art's words are bent into the shape ("plain" for none); its look stays.
bool CrunchWindow::setTextArtWarp(const QString &transform)
{
    bool known = false;
    for (const TextArtTransform &t : textArtTransforms()) known = known || t.id == transform;
    if (!known) return false;
    bool art = true;
    const QString prst = office::textWarpPreset(transform);
    const bool done = restyleObject(tr("Text Warp"), [&](QDomElement &el, grid::ObjectBox *) {
        art = isTextArt(el);
        if (!art) return false;
        QDomElement warp = childNamed(childNamed(childNamed(el, QStringLiteral("txBody")), QStringLiteral("bodyPr")), QStringLiteral("prstTxWarp"));
        if (warp.attribute(QStringLiteral("prst")) == prst) return false;
        warp.setAttribute(QStringLiteral("prst"), prst);
        return true;
    });
    if (!art) notice(tr("Text Warp"), tr("Text Warp is for Text Art. Pick one first."));
    return done;
}

void CrunchWindow::createTextArtActions()
{
    for (const TextArtTransform &t : textArtTransforms()) {
        const QString id = t.id;
        mk(QStringLiteral("object.textArt.warp.") + id, t.name, "waves", QKeySequence(), [this, id] { setTextArtWarp(id); }, false, false);
    }
}

void CrunchWindow::textArtRibbonParts(RibbonParts &parts)
{
    parts.widgets[QStringLiteral("textArtButton")] = [this]() -> QWidget * {
        auto *b = new GalleryButton(icon("case-sensitive"), tr("Text Art"), QSize(30, 30), 6, true);
        b->setToolTip(tr("Text Art"));
        b->setItemsProvider([this] { return textArtGalleryItems(workbookColors()); });
        connect(b, &GalleryButton::activated, this, [this](const QString &styleId) { insertTextArt(styleId); });
        return b;
    };
    parts.widgets[QStringLiteral("textArtStyles")] = [this]() -> QWidget * {
        auto *b = new GalleryButton(icon("case-sensitive"), tr("Text Art Styles"), QSize(30, 30), 6, true);
        b->setToolTip(tr("Text Art Styles"));
        b->setItemsProvider([this] { return textArtGalleryItems(workbookColors()); });
        connect(b, &GalleryButton::activated, this, [this](const QString &styleId) { setTextArtStyle(styleId); });
        return b;
    };
    parts.menus[QStringLiteral("textArtWarp")] = [this](QWidget *owner) {
        auto *m = new QMenu(owner);
        auto *group = new QActionGroup(m);
        for (const TextArtTransform &t : textArtTransforms()) {
            QAction *a = act(QStringLiteral("object.textArt.warp.") + t.id);
            group->addAction(a);
            m->addAction(a);
        }
        return m;
    };
}

} // namespace jp
