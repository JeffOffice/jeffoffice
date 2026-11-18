#include "app/equationedit.h"

#include "app/editor.h"
#include "core/document.h"
#include "render/renderer.h"
#include "wml/wmlmathedit.h"

#include <QCoreApplication>

#include <cmath>

namespace jp {

std::shared_ptr<EquationItem> makeEquationFromMarkup(const QByteArray &markup, const QString &linear, bool linearForm, const QPointF &center, double size)
{
    if (markup.isEmpty() || markup.size() > (1 << 20)) return nullptr;
    auto w = std::make_shared<EquationItem>();
    w->linear = linear.trimmed();
    w->markup = markup;
    w->linearForm = linearForm;
    w->size = size;
    QSizeF natural;
    Renderer::equationOutline(*w, &natural);
    if (natural.isEmpty()) return nullptr;
    w->rect = QRectF(QPointF(), natural);
    w->rect.moveCenter(center);
    return w;
}

std::shared_ptr<EquationItem> makeEquation(const QString &linear, const QPointF &center, double size)
{
    bool ok = false;
    const QByteArray markup = office::wml::equationMarkup(linear, &ok);
    if (!ok) return nullptr;
    return makeEquationFromMarkup(markup, linear, false, center, size);
}

QString insertEquationMarkup(Editor *ed, const QByteArray &markup, const QString &linear, bool linearForm)
{
    const QSizeF ps = ed->doc()->pageSize();
    auto w = makeEquationFromMarkup(markup, linear, linearForm, QPointF(ps.width() / 2, ps.height() / 3));
    if (!w) return QString();
    return ed->addItem(w);
}

QString insertEquation(Editor *ed, const QString &linear)
{
    const QSizeF ps = ed->doc()->pageSize();
    auto w = makeEquation(linear, QPointF(ps.width() / 2, ps.height() / 3));
    if (!w) return QString();
    return ed->addItem(w);
}

bool changeEquationMarkup(Editor *ed, QString id, const QByteArray &markup, const QString &linear, bool linearForm)
{
    auto *w = dynamic_cast<EquationItem *>(ed->doc()->item(id));
    if (!w || (w->markup == markup && w->linear == linear.trimmed() && w->linearForm == linearForm)) return false;
    const QPointF center = w->rect.center();
    auto made = makeEquationFromMarkup(markup, linear, linearForm, center, w->size);
    if (!made) return false;
    // (The frame the person sized keeps its scale: the new equation is as much larger or smaller than its natural size as the old one was.)
    QSizeF oldNatural;
    Renderer::equationOutline(*w, &oldNatural);
    const double kx = oldNatural.isEmpty() ? 1.0 : w->rect.width() / oldNatural.width(), ky = oldNatural.isEmpty() ? 1.0 : w->rect.height() / oldNatural.height();
    ed->change(QCoreApplication::translate("Editor", "Edit Equation"), [&] {
        w->linear = made->linear;
        w->markup = made->markup;
        w->linearForm = made->linearForm;
        w->rect = QRectF(QPointF(), QSizeF(made->rect.width() * kx, made->rect.height() * ky));
        w->rect.moveCenter(center);
    });
    return true;
}

bool changeEquation(Editor *ed, QString id, const QString &linear)
{
    auto *w = dynamic_cast<EquationItem *>(ed->doc()->item(id));
    if (!w || w->linear == linear.trimmed()) return false;
    bool ok = false;
    const QByteArray markup = office::wml::equationMarkup(linear, &ok);
    return ok && changeEquationMarkup(ed, id, markup, linear, w->linearForm);
}

QString equationReadableText(const EquationItem &eq)
{
    QString text = eq.linear.trimmed();
    if (text.isEmpty()) text = office::wml::equationToUnicodeMath(eq.markup).trimmed();
    if (text.isEmpty()) text = office::wml::equationLine(eq.markup).trimmed();
    return text;
}

std::shared_ptr<EquationItem> builtUpEquation(const EquationItem &eq)
{
    auto out = std::static_pointer_cast<EquationItem>(eq.clone());
    if (!eq.linearForm) return out;
    QSizeF linearNatural, builtNatural;
    Renderer::equationOutline(eq, &linearNatural);
    out->linearForm = false;
    Renderer::equationOutline(*out, &builtNatural);
    if (linearNatural.isEmpty() || builtNatural.isEmpty()) return out;
    out->rect = QRectF(eq.rect.topLeft(), QSizeF(builtNatural.width() * eq.rect.width() / linearNatural.width(), builtNatural.height() * eq.rect.height() / linearNatural.height()));
    return out;
}

bool setEquationLinearForm(Editor *ed, QString id, bool linearForm)
{
    auto *w = dynamic_cast<EquationItem *>(ed->doc()->item(id));
    if (!w || w->linearForm == linearForm) return false;
    return changeEquationMarkup(ed, id, w->markup, w->linear, linearForm);
}

} // namespace jp
