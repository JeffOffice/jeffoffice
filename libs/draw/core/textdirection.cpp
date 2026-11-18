#include "core/textdirection.h"

#include <QCoreApplication>

#include <algorithm>
#include <cmath>

namespace jp {

const char *textDirectionVert(TextDirection direction)
{
    return direction == TextDirection::Rotate90 ? "vert" : direction == TextDirection::Rotate270 ? "vert270" : "horz";
}

std::optional<TextDirection> textDirectionFromVert(const QString &vert)
{
    if (vert == QLatin1String("horz")) return TextDirection::Horizontal;
    if (vert == QLatin1String("vert") || vert == QLatin1String("eaVert")) return TextDirection::Rotate90;
    if (vert == QLatin1String("vert270")) return TextDirection::Rotate270;
    return std::nullopt;
}

int textDirectionQuarterTurns(TextDirection direction)
{
    return direction == TextDirection::Rotate90 ? 1 : direction == TextDirection::Rotate270 ? 3 : 0;
}

TextDirection textDirectionFromQuarterTurns(int quarterTurns)
{
    return quarterTurns == 1 ? TextDirection::Rotate90 : quarterTurns == 3 ? TextDirection::Rotate270 : TextDirection::Horizontal;
}

QString textDirectionName(TextDirection direction)
{
    switch (direction) {
    case TextDirection::Rotate90: return QCoreApplication::translate("jp::TextDirection", "Rotate all text 90°");
    case TextDirection::Rotate270: return QCoreApplication::translate("jp::TextDirection", "Rotate all text 270°");
    default: return QCoreApplication::translate("jp::TextDirection", "Horizontal");
    }
}

QString textDirectionOdfAttributes(TextDirection direction)
{
    if (direction == TextDirection::Rotate90) return QStringLiteral(" style:writing-mode=\"tb-rl\"");
    if (direction == TextDirection::Rotate270) return QStringLiteral(" style:writing-mode=\"tb-rl\" loext:writing-mode=\"bt-lr\"");
    return QString();
}

TextDirection textDirectionFromOdf(const QString &styleMode, const QString &extensionMode, bool turnedBack)
{
    if (extensionMode == QLatin1String("bt-lr") || styleMode == QLatin1String("bt-lr")) return TextDirection::Rotate270;
    if (extensionMode.startsWith(QLatin1String("tb")) || styleMode.startsWith(QLatin1String("tb"))) return turnedBack ? TextDirection::Rotate270 : TextDirection::Rotate90;
    return TextDirection::Horizontal;
}

QSizeF usualTextBoxSize() { return QSizeF(144, 72); }

std::pair<QPointF, QPointF> textBoxDragEnds(const QPointF &from, const QPointF &to, bool keepProportions, const QSizeF &usual)
{
    if (!std::isfinite(from.x()) || !std::isfinite(from.y()) || !std::isfinite(to.x()) || !std::isfinite(to.y())) return {from, from};
    QPointF d = to - from;
    if (std::abs(d.x()) < 4 && std::abs(d.y()) < 4) {
        // (A click: the usual size, from the point.)
        return {from, from + QPointF(usual.width(), usual.height())};
    }
    if (keepProportions) {
        const double side = std::max(std::abs(d.x()), std::abs(d.y()));
        d = QPointF(d.x() < 0 ? -side : side, d.y() < 0 ? -side : side);
    }
    // (A box that can hold a line: at least 18 points each way.)
    auto atLeast = [](double v) { return std::abs(v) < 18 ? (v < 0 ? -18.0 : 18.0) : v; };
    d = QPointF(atLeast(d.x()), atLeast(d.y()));
    const QPointF end = from + d;
    return {QPointF(std::min(from.x(), end.x()), std::min(from.y(), end.y())), QPointF(std::max(from.x(), end.x()), std::max(from.y(), end.y()))};
}

} // namespace jp
