#include "app/equationedit.h"

#include "app/editor.h"
#include "core/document.h"
#include "render/renderer.h"
#include "wml/wmlmathedit.h"

#include <QCoreApplication>

#include <cmath>

namespace jp {

std::shared_ptr<EquationItem> makeEquation(const QString &linear, const QPointF &center, double size)
{
    bool ok = false;
    const QByteArray markup = office::wml::equationMarkup(linear, &ok);
    if (!ok) return nullptr;
    auto w = std::make_shared<EquationItem>();
    w->linear = linear.trimmed();
    w->markup = markup;
    w->size = size;
    QSizeF natural;
    Renderer::equationOutline(*w, &natural);
    if (natural.isEmpty()) return nullptr;
    w->rect = QRectF(QPointF(), natural);
    w->rect.moveCenter(center);
    return w;
}

QString insertEquation(Editor *ed, const QString &linear)
{
    const QSizeF ps = ed->doc()->pageSize();
    auto w = makeEquation(linear, QPointF(ps.width() / 2, ps.height() / 3));
    if (!w) return QString();
    return ed->addItem(w);
}

bool changeEquation(Editor *ed, const QString &id, const QString &linear)
{
    auto *w = dynamic_cast<EquationItem *>(ed->doc()->item(id));
    if (!w || w->linear == linear.trimmed()) return false;
    const QPointF center = w->rect.center();
    auto made = makeEquation(linear, center, w->size);
    if (!made) return false;
    // (The frame the person sized keeps its scale: the new equation is as much larger or smaller than its natural size as the old one was.)
    QSizeF oldNatural;
    Renderer::equationOutline(*w, &oldNatural);
    const double kx = oldNatural.isEmpty() ? 1.0 : w->rect.width() / oldNatural.width(), ky = oldNatural.isEmpty() ? 1.0 : w->rect.height() / oldNatural.height();
    ed->change(QCoreApplication::translate("Editor", "Edit Equation"), [&] {
        w->linear = made->linear;
        w->markup = made->markup;
        w->rect = QRectF(QPointF(), QSizeF(made->rect.width() * kx, made->rect.height() * ky));
        w->rect.moveCenter(center);
    });
    return true;
}

} // namespace jp
