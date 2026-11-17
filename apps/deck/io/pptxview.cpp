#include "io/pptxview.h"

#include "core/gridview.h"
#include "app/guidesdialog.h"
#include "office/xml.h"

#include <QLocale>
#include <algorithm>
#include <cmath>

namespace jp::deck {

namespace {

using office::ns::P;

// A guide's position is in eighths of a point; a point is 12700 EMU.
constexpr double kGuideUnit = 8;
constexpr double kEmuPerPoint = 12700;
// The elements of the guide list that are looked at: more are ignored.
constexpr int kMaxGuideElements = 4096;

QDomElement pChild(const QDomElement &e, const char *name) { return office::child(e, P, QLatin1String(name)); }

bool boolAttr(const QDomElement &e, const char *name, bool *out)
{
    const QString v = e.attribute(QLatin1String(name));
    if (v == QLatin1String("1") || v == QLatin1String("true")) *out = true;
    else if (v == QLatin1String("0") || v == QLatin1String("false")) *out = false;
    else return false;
    return true;
}

} // namespace

void readViewProperties(const QDomElement &viewPr, Document &doc)
{
    const QDomElement c = pChild(pChild(viewPr, "slideViewPr"), "cSldViewPr");
    GridView g;
    bool b = false;
    if (boolAttr(c, "snapToGrid", &b)) g.snapGrid = b;
    if (boolAttr(c, "snapToObjects", &b)) g.snapObjects = b;
    if (boolAttr(c, "showGuides", &b)) g.showGuides = b;
    const QDomElement spacing = pChild(viewPr, "gridSpacing");
    bool ok = false;
    const double cx = spacing.attribute(QStringLiteral("cx")).toDouble(&ok);
    // (a file with no gridSpacing leaves the spacing to the program; one that has it, whatever the number, sets it)
    if (ok && std::isfinite(cx) && cx > 0) g.spacing = cx / kEmuPerPoint;
    setGridView(doc, g);

    RulerGuides guides;
    int seen = 0;
    for (QDomElement e = pChild(c, "guideLst").firstChildElement(); !e.isNull() && seen < kMaxGuideElements; e = e.nextSiblingElement(), ++seen) {
        if (e.localName() != QLatin1String("guide") || e.namespaceURI() != P) continue;
        bool numeric = true;
        const QString posText = e.attribute(QStringLiteral("pos"), QStringLiteral("0"));
        const double pos = QLocale::c().toDouble(posText, &numeric);
        if (!numeric || !std::isfinite(pos)) continue;
        // (the direction left out is vertical, as the schema says)
        (e.attribute(QStringLiteral("orient")) == QLatin1String("horz") ? guides.h : guides.v) << pos / kGuideUnit;
    }
    fitGuides(guides, doc.pageSize());
    if (guides.h.size() > kMaxGuidesPerAxis) guides.h.resize(kMaxGuidesPerAxis);
    if (guides.v.size() > kMaxGuidesPerAxis) guides.v.resize(kMaxGuidesPerAxis);
    doc.viewGuides = guides;
}

QString viewPropertiesXml(const Document &doc, const QString &namespaces)
{
    const GridView g = gridViewOf(doc);
    QString attrs;
    auto flag = [&](const char *name, const std::optional<bool> &v) {
        if (v) attrs += QStringLiteral(" %1=\"%2\"").arg(QLatin1String(name), *v ? QStringLiteral("1") : QStringLiteral("0"));
    };
    flag("showGuides", g.showGuides);
    flag("snapToGrid", g.snapGrid);
    flag("snapToObjects", g.snapObjects);
    const RulerGuides &guides = doc.viewGuides;
    QString list;
    auto add = [&](const QVector<double> &v, const char *orient) {
        for (double pos : v) {
            if (!std::isfinite(pos)) continue;
            list += QStringLiteral("<p:guide%1 pos=\"%2\"/>").arg(orient ? QStringLiteral(" orient=\"%1\"").arg(QLatin1String(orient)) : QString()).arg(qint64(std::llround(pos * kGuideUnit)));
        }
    };
    add(guides.h, "horz");
    add(guides.v, nullptr);
    // (no spacing set: no gridSpacing element, which the schema allows; a set one is always written, the twelfth of an inch too)
    const QString spacing = g.spacing ? QStringLiteral("<p:gridSpacing cx=\"%1\" cy=\"%1\"/>").arg(std::max<qint64>(1, std::llround(*g.spacing * kEmuPerPoint))) : QString();
    return QStringLiteral("<p:viewPr%1><p:normalViewPr><p:restoredLeft sz=\"15620\"/><p:restoredTop sz=\"94660\"/></p:normalViewPr><p:slideViewPr><p:cSldViewPr%2><p:cViewPr varScale=\"1\">"
                          "<p:scale><a:sx n=\"100\" d=\"100\"/><a:sy n=\"100\" d=\"100\"/></p:scale><p:origin x=\"0\" y=\"0\"/></p:cViewPr>%3</p:cSldViewPr></p:slideViewPr>"
                          "<p:notesTextViewPr><p:cViewPr><p:scale><a:sx n=\"1\" d=\"1\"/><a:sy n=\"1\" d=\"1\"/></p:scale><p:origin x=\"0\" y=\"0\"/></p:cViewPr></p:notesTextViewPr>"
                          "%4</p:viewPr>")
        .arg(namespaces, attrs, list.isEmpty() ? QStringLiteral("<p:guideLst/>") : QStringLiteral("<p:guideLst>%1</p:guideLst>").arg(list)).arg(spacing);
}

} // namespace jp::deck
