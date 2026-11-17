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
#include "core/items.h"
#include "wml/wmlmathedit.h"

#include <QAction>
#include <QMenu>
#include <QStatusBar>

#include <algorithm>

namespace jp {

namespace {
// The height of the letters of a new equation on a slide, in points (a slide is 540 points tall or so; Pub's page uses 24).
constexpr double kSlideLetters = 40;
} // namespace

void DeckWindow::createEquationActions()
{
    mk("insert.equation", tr("Equation…"), "square-pi", QKeySequence(QStringLiteral("Alt+=")), [this] { insertEquationDialog(); });
    for (int i = 0; i < office::wml::equationGallery().size(); ++i)
        mk(QStringLiteral("insert.equationDesign%1").arg(i), office::wml::equationGallery().at(i).name, "square-pi", QKeySequence(), [this, i] { insertEquation(office::wml::equationGallery().at(i).linear); });
    // A double click on an equation opens the editor on it.
    connect(m_canvas, &Canvas::editEquationWanted, this, [this](const QString &id) { editEquation(id); });
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
    auto *w = dynamic_cast<EquationItem *>(m_ed->doc()->item(id));
    if (!w) return false;
    // An equation a file brought, which no typed line makes, starts from the line it reads as.
    const QString start = w->linear.isEmpty() ? office::wml::equationLine(w->markup) : w->linear;
    QString line = start;
    if (!askEquation(this, tr("Edit Equation"), &line) || line.trimmed() == start.trimmed()) return false;
    if (!changeEquation(m_ed, id, line)) {
        statusBar()->showMessage(tr("This equation can't be changed."), 6000);
        return false;
    }
    return true;
}

void DeckWindow::insertEquationDialog()
{
    // (an equation that is selected by itself is changed)
    if (Item *one = m_ed->single(); one && one->type() == ItemType::Equation) {
        editEquation(one->id);
        return;
    }
    QString line;
    if (!askEquation(this, tr("Insert Equation"), &line)) return;
    insertEquation(line);
}

} // namespace jp
