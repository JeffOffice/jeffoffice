// An equation of a slide as a formula object of the presentation: a frame with the MathML document of the equation (a part of its own, Object N) and a picture of it beside it for the programs
// that don't read formulas.
#include "io/odpexport_p.h"

#include "core/items.h"
#include "office/mathobject.h"
#include "wml/wmlmathpath.h"

#include <QCoreApplication>

#include <algorithm>
#include <cmath>

namespace jp::deck::odpw {

bool Exporter::writeEquation(X &x, Scope &sc, const EquationItem &eq)
{
    if (m_formulaCount >= office::kMostMathObjects) return false;
    const QByteArray plain = office::mathObjectFromOmml(eq.markup);
    const QByteArray content = plain.isEmpty() || eq.fill.isNone() ? plain : office::mathObjectWithColor(plain, eq.fill.color.resolve(m_doc.colors));
    if (content.isEmpty() || m_formulaBytes + content.size() > office::kMostMathObjectBytes) return false;
    const double w = eq.rect.width(), h = eq.rect.height();
    if (!(w > 0 && h > 0)) return false;
    const double px = std::min({3.0, 4000.0 / std::max(1.0, w), 4000.0 / std::max(1.0, h)});
    const QColor color = eq.fill.isNone() ? QColor(Qt::black) : eq.fill.color.resolve(m_doc.colors);
    const QByteArray png = office::wml::equationPng(eq.markup, std::max(1, int(std::lround(w * px))), std::max(1, int(std::lround(h * px))), color);
    if (png.isEmpty()) return false;
    const QString dir = QStringLiteral("Object %1").arg(++m_formulaCount);
    m_formulaBytes += content.size();
    addFile(dir + QStringLiteral("/"), QByteArray(), QStringLiteral("application/vnd.oasis.opendocument.formula"), false);
    addFile(dir + QStringLiteral("/content.xml"), content, QStringLiteral("text/xml"), true);
    const QString picture = QStringLiteral("ObjectReplacements/%1.png").arg(dir);
    addFile(picture, png, QStringLiteral("image/png"), false);
    const QString style = sc.styles->style(QStringLiteral("graphic"), sc.master ? QStringLiteral("Mgr") : QStringLiteral("gr"), QString(),
                                           QStringLiteral("<style:graphic-properties draw:fill=\"none\" draw:stroke=\"none\" draw:shadow=\"hidden\"/>"));
    x.open("draw:frame").attr("draw:style-name", style);
    writeCommon(x, eq);
    x.attr("draw:layer", sc.layer);
    writePlace(x, eq.rect, eq.rotation);
    x.open("draw:object").attr("xlink:href", "./" + dir).attr("xlink:type", "simple").attr("xlink:show", "embed").attr("xlink:actuate", "onLoad").close();
    x.open("draw:image").attr("xlink:href", picture).attr("xlink:type", "simple").attr("xlink:show", "embed").attr("xlink:actuate", "onLoad").attr("loext:mime-type", "image/png").close();
    writeInside(x, eq);
    x.close();
    return true;
}

} // namespace jp::deck::odpw
