// JeffDeck's Insert > Equation: an equation typed in a line (the dialog the other programs have, with the built-in equations to start from) is an object on the
// slide that moves, sizes, undoes, and is edited again by a double click. The model of the line is libs/office/wml/wmlmathedit.h, and the object, its layout, and the
// way it is changed are libs/page/app/equationedit.h.

#include "app/deckwindow.h"

#include "app/deckcanvas.h"
#include "app/editor.h"
#include "app/equationdialog.h"
#include "app/equationedit.h"
#include "app/ribbonbuilder.h"
#include "app/widgets.h"
#include "canvas/canvas.h"
#include "core/deck.h"
#include "core/items.h"
#include "office/drawingml.h"
#include "office/xml.h"
#include "wml/wmlmathedit.h"

#include <QAction>
#include <QColor>
#include <QDomDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QMenu>
#include <QStatusBar>

#include <algorithm>
#include <cmath>

namespace jp {

namespace {
// The height of the letters of a new equation on a slide, in points (a slide is 540 points tall or so; Pub's page uses 24).
constexpr double kSlideLetters = 40;

// What the presentation holds of equations (objects of ItemType::Equation, on slides, masters, and layouts, groups' too, and the bytes of their math): a new one is
// made only within Document::kMaxEquations and kMaxEquationMarkupBytes, the limits a file is read with.
bool equationRoom(const Document &doc, qint64 more)
{
    int count = 0;
    qint64 bytes = more;
    auto count_in = [&](const ItemList &items) {
        walkItems(items, [&](const ItemPtr &it) {
            if (it->type() != ItemType::Equation) return;
            ++count;
            bytes += static_cast<const EquationItem &>(*it).markup.size();
        });
    };
    for (const auto &page : doc.pages) count_in(page->items);
    for (const auto &master : doc.masters) count_in(master->items);
    return count < Document::kMaxEquations && bytes <= Document::kMaxEquationMarkupBytes;
}

// The pair a `.pptx` kept for the picture `id` that stands for an equation (the file's own XML, whole), or an empty object.
QJsonObject keptEquation(const Document &doc, const QString &id, int *page)
{
    const Document::Loc at = doc.find(id);
    if (!at.item || at.item->type() != ItemType::Picture || at.page < 0 || at.page >= int(doc.pages.size())) return QJsonObject();
    for (const QJsonValue &v : doc.pages[size_t(at.page)]->props.value(QStringLiteral("charts")).toArray())
        if (v.toObject().value(QStringLiteral("item")).toString() == id && v.toObject().value(QStringLiteral("kind")).toString() == QLatin1String("equation")) {
            if (page) *page = at.page;
            return v.toObject();
        }
    return QJsonObject();
}

// The math of a kept pair: the markup of the first shape in it whose text is math, with the line it reads as, and the color of its letters when it names one.
bool mathOfKeptPair(const QJsonObject &kept, QByteArray *markup, QString *linear, QColor *color)
{
    QDomDocument dom;
    if (!office::parseXml(kept.value(QStringLiteral("xml")).toString().toUtf8(), &dom)) return false;
    QVector<QDomElement> stack{dom.documentElement()};
    int seen = 0;
    while (!stack.isEmpty() && ++seen < 100000) {
        const QDomElement e = stack.takeLast();
        if (e.localName() == QLatin1String("sp")) {
            *markup = office::shapeMath(e, linear);
            if (!markup->isEmpty()) {
                for (QDomElement n = e.firstChildElement(); !n.isNull(); n = n.nextSiblingElement()) {
                    if (n.localName() != QLatin1String("txBody")) continue;
                    for (QDomElement p = n.firstChildElement(); !p.isNull(); p = p.nextSiblingElement())
                        for (QDomElement r = p.firstChildElement(); !r.isNull(); r = r.nextSiblingElement()) {
                            if (r.localName() != QLatin1String("endParaRPr") && r.localName() != QLatin1String("rPr")) continue;
                            for (QDomElement f = r.firstChildElement(); !f.isNull() && !color->isValid(); f = f.nextSiblingElement())
                                for (QDomElement c = f.firstChildElement(); !c.isNull() && !color->isValid(); c = c.nextSiblingElement())
                                    if (c.localName() == QLatin1String("srgbClr")) *color = QColor(QStringLiteral("#") + c.attribute(QStringLiteral("val")));
                        }
                }
                return true;
            }
            continue;
        }
        for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement()) stack.append(c);
    }
    return false;
}
} // namespace

void DeckWindow::createEquationActions()
{
    mk("insert.equation", tr("Equation…"), "square-pi", QKeySequence(QStringLiteral("Alt+=")), [this] { insertEquationDialog(); });
    for (int i = 0; i < office::wml::equationGallery().size(); ++i)
        mk(QStringLiteral("insert.equationDesign%1").arg(i), office::wml::equationGallery().at(i).name, "square-pi", QKeySequence(), [this, i] { insertEquation(office::wml::equationGallery().at(i).linear); });
    // A double click on an equation opens the editor on it, and so does one on the picture that stands for an equation a file brought.
    connect(m_canvas, &Canvas::editEquationWanted, this, [this](const QString &id) { editEquation(id); });
    connect(m_canvas, &Canvas::pictureTabWanted, this, [this] {
        if (Item *one = m_ed->single(); one && !keptEquation(*m_ed->doc(), one->id, nullptr).isEmpty()) editEquation(one->id);
    });
}

void DeckWindow::addEquationParts(RibbonParts &parts)
{
    parts.menus[QStringLiteral("equationGallery")] = [this](QWidget *owner) {
        auto *m = new QMenu(owner);
        for (int i = 0; i < office::wml::equationGallery().size(); ++i) m->addAction(act(QStringLiteral("insert.equationDesign%1").arg(i)));
        return m;
    };
}

bool DeckWindow::insertEquation(const QString &linear)
{
    const QSizeF ps = m_ed->doc()->pageSize();
    auto eq = makeEquation(linear, QPointF(ps.width() / 2, ps.height() / 2), kSlideLetters);
    if (eq && !equationRoom(*m_ed->doc(), eq->markup.size())) {
        statusBar()->showMessage(tr("This presentation has as many equations as it can hold."), 6000);
        return false;
    }
    if (!eq) {
        statusBar()->showMessage(tr("An equation can't be put here."), 6000);
        return false;
    }
    // One that is wider or taller than most of the slide is made smaller to fit, keeping its proportions.
    const double k = std::min({1.0, ps.width() * 0.9 / eq->rect.width(), ps.height() * 0.9 / eq->rect.height()});
    if (k < 1.0) {
        eq->rect = QRectF(QPointF(), eq->rect.size() * k);
        eq->rect.moveCenter(QPointF(ps.width() / 2, ps.height() / 2));
    }
    m_ed->endTextEdit();
    m_ed->addItem(eq);
    return true;
}

bool DeckWindow::editEquation(const QString &id)
{
    // The picture that stands for an equation a file brought is drawn and saved as the file has it until the equation is changed here: then it is made an equation,
    // from the line it reads as and the line typed, in the same place, as one step that undoes.
    int slide = -1;
    if (const QJsonObject kept = keptEquation(*m_ed->doc(), id, &slide); !kept.isEmpty()) {
        QByteArray markup;
        QString linear;
        QColor color;
        if (!mathOfKeptPair(kept, &markup, &linear, &color)) {
            statusBar()->showMessage(tr("This equation can't be changed here."), 6000);
            return false;
        }
        const QString start = linear.isEmpty() ? office::wml::equationLine(markup) : linear;
        QString line = start;
        if (!askEquation(this, tr("Edit Equation"), &line) || line.trimmed() == start.trimmed()) return false;
        const Document::Loc at = m_ed->doc()->find(id);
        if (!at.item || !at.list || !equationRoom(*m_ed->doc(), 0)) {
            statusBar()->showMessage(tr("This presentation has as many equations as it can hold."), 6000);
            return false;
        }
        const Item &old = *at.item;
        auto made = makeEquation(line, old.rect.center(), kSlideLetters);
        if (!made || !equationRoom(*m_ed->doc(), made->markup.size())) {
            statusBar()->showMessage(tr("This equation can't be changed."), 6000);
            return false;
        }
        // (the new equation fits the frame the old one had, as large as it goes, keeping its proportions)
        const double k = std::min(old.rect.width() / made->rect.width(), old.rect.height() / made->rect.height());
        if (k > 0 && std::isfinite(k)) {
            made->rect = QRectF(QPointF(), made->rect.size() * k);
            made->rect.moveCenter(old.rect.center());
        }
        made->id = old.id;
        made->name = old.name;
        made->altText = old.altText;
        made->altTitle = old.altTitle;
        made->sourceId = old.sourceId;
        made->rotation = old.rotation;
        made->flipH = old.flipH;
        made->flipV = old.flipV;
        made->hidden = old.hidden;
        made->hyperlink = old.hyperlink;
        made->size = kSlideLetters;
        if (color.isValid()) made->fill = Fill::solid(ColorRef::rgb(color));
        const int index = at.index;
        ItemList *list = at.list;
        m_ed->endTextEdit();
        m_ed->change(tr("Edit Equation"), [&] {
            (*list)[size_t(index)] = made;
            QJsonObject props = m_ed->doc()->pages[size_t(slide)]->props;
            QJsonArray rest;
            for (const QJsonValue &v : props.value(QStringLiteral("charts")).toArray())
                if (v.toObject().value(QStringLiteral("item")).toString() != id) rest.append(v);
            props[QStringLiteral("charts")] = rest;
            m_ed->doc()->pages[size_t(slide)]->props = props;
        });
        m_ed->select(id);
        return true;
    }
    auto *w = dynamic_cast<EquationItem *>(m_ed->doc()->item(id));
    if (!w) return false;
    // An equation a file brought, which no typed line makes, starts from the line it reads as.
    const QString start = w->linear.isEmpty() ? office::wml::equationLine(w->markup) : w->linear;
    QString line = start;
    if (!askEquation(this, tr("Edit Equation"), &line) || line.trimmed() == start.trimmed()) return false;
    // (The new math counts toward the presentation's total of math, as a new equation's does, less what the old held.)
    if (const auto made = makeEquation(line, QPointF(), w->size); made && !equationRoom(*m_ed->doc(), qint64(made->markup.size()) - w->markup.size())) {
        statusBar()->showMessage(tr("This presentation has as many equations as it can hold."), 6000);
        return false;
    }
    if (!changeEquation(m_ed, id, line)) {
        statusBar()->showMessage(tr("This equation can't be changed."), 6000);
        return false;
    }
    return true;
}

void DeckWindow::insertEquationDialog()
{
    // (an equation that is selected by itself is changed)
    if (Item *one = m_ed->single(); one && (one->type() == ItemType::Equation || !keptEquation(*m_ed->doc(), one->id, nullptr).isEmpty())) {
        editEquation(one->id);
        return;
    }
    QString line;
    if (!askEquation(this, tr("Insert Equation"), &line)) return;
    insertEquation(line);
}

} // namespace jp
