#include "io/pptxtransition.h"

#include "office/xml.h"

#include <QDomDocument>
#include <cmath>

namespace jp::deck {

namespace {

const QLatin1String kP14("http://schemas.microsoft.com/office/powerpoint/2010/main");
// Morph came with the program's 2019 version, in a namespace of its own.
const QLatin1String kP159("http://schemas.microsoft.com/office/powerpoint/2015/09/main");

// What a morph moves, as the file names it.
QString optionOf(const QString &by)
{
    if (by == QLatin1String("words")) return QStringLiteral("byWord");
    if (by == QLatin1String("characters")) return QStringLiteral("byChar");
    return QStringLiteral("byObject");
}

QString unitOf(const QString &option)
{
    if (option == QLatin1String("byWord")) return QStringLiteral("words");
    if (option == QLatin1String("byChar")) return QStringLiteral("characters");
    return QStringLiteral("objects");
}

// The speeds the file names, in seconds (as the program shows them).
double secondsOf(const QString &spd)
{
    if (spd == QLatin1String("slow")) return 1.0;
    if (spd == QLatin1String("fast")) return 0.5;
    return 0.75;
}

QString speedOf(double seconds)
{
    if (seconds <= 0.6) return QStringLiteral("fast");
    if (seconds <= 0.85) return QStringLiteral("med");
    return QStringLiteral("slow");
}

// The p:transition to read: in an mc:AlternateContent, the newer one when its
// effect is one the show knows (Reveal, Morph, or a plain one with a time of
// its own), else the one for older programs.
QDomElement pick(const QDomElement &root)
{
    if (root.localName() == QLatin1String("transition")) return root;
    QDomElement fallback;
    for (QDomElement alt = root.firstChildElement(); !alt.isNull(); alt = alt.nextSiblingElement()) {
        QDomElement found;
        for (QDomElement c = alt.firstChildElement(); !c.isNull(); c = c.nextSiblingElement())
            if (c.localName() == QLatin1String("transition")) found = c;
        if (found.isNull()) continue;
        if (alt.localName() != QLatin1String("Choice")) {
            fallback = found;
            continue;
        }
        // The newer form, when its effect is one the show knows or a plain one with its own time.
        bool known = true;
        for (QDomElement e = found.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.namespaceURI() == kP14 && e.localName() != QLatin1String("reveal")) known = false;
            if (e.namespaceURI() == kP159 && e.localName() != QLatin1String("morph")) known = false;
        }
        if (known) return found;
    }
    return fallback;
}

} // namespace

Transition transitionFromXml(const QString &xml)
{
    Transition t;
    QDomDocument dom;
    if (xml.isEmpty() || !office::parseXml(xml.toUtf8(), &dom)) return t;
    const QDomElement tr = pick(dom.documentElement());
    if (tr.isNull()) return t;
    t.duration = secondsOf(tr.attribute(QStringLiteral("spd"), QStringLiteral("fast")));
    if (tr.hasAttributeNS(kP14, QStringLiteral("dur"))) {
        bool ok = false;
        const double ms = tr.attributeNS(kP14, QStringLiteral("dur")).toDouble(&ok);
        if (ok && std::isfinite(ms)) t.duration = ms / 1000.0;
    }
    const QString click = tr.attribute(QStringLiteral("advClick"));
    t.advanceOnClick = !(click == QLatin1String("0") || click == QLatin1String("false"));
    bool ok = false;
    const double after = tr.attribute(QStringLiteral("advTm")).toDouble(&ok);
    if (ok && std::isfinite(after) && after >= 0) t.advanceAfter = after / 1000.0;
    QDomElement effect;
    for (QDomElement c = tr.firstChildElement(); !c.isNull() && effect.isNull(); c = c.nextSiblingElement())
        if (c.localName() != QLatin1String("sndAc") && c.localName() != QLatin1String("extLst")) effect = c;
    if (effect.isNull()) {
        t.type = QStringLiteral("none");
    } else {
        const QString name = effect.localName(), dir = effect.attribute(QStringLiteral("dir"));
        if (name == QLatin1String("fade")) t.type = QStringLiteral("fade");
        else if (name == QLatin1String("cut")) t.type = QStringLiteral("cut");
        else if (name == QLatin1String("push")) t.type = QStringLiteral("push");
        else if (name == QLatin1String("wipe")) t.type = QStringLiteral("wipe");
        else if (name == QLatin1String("cover")) t.type = QStringLiteral("cover");
        else if (name == QLatin1String("pull")) t.type = QStringLiteral("uncover");
        else if (name == QLatin1String("reveal")) t.type = QStringLiteral("reveal");
        else if (name == QLatin1String("morph") && effect.namespaceURI() == kP159) {
            t.type = QStringLiteral("morph");
            t.by = unitOf(effect.attribute(QStringLiteral("option")));
        }
        else if (name == QLatin1String("split")) t.type = QStringLiteral("split");
        else if (name == QLatin1String("zoom")) t.type = QStringLiteral("zoom");
        else t.type = QStringLiteral("fade");   // one the show has no form for
        if (t.hasDirection()) t.dir = dir.isEmpty() ? QStringLiteral("l") : dir.left(1);
        else if (t.hasInOut()) t.dir = dir.isEmpty() ? QStringLiteral("out") : dir;
    }
    // As the show keeps it: anything out of range takes its default.
    return Transition::fromJson(t.toJson());
}

QString transitionXml(const Transition &t)
{
    if (t == Transition()) return QString();
    QString attrs = QStringLiteral(" spd=\"%1\"").arg(speedOf(t.duration));
    if (!t.advanceOnClick) attrs += QStringLiteral(" advClick=\"0\"");
    if (t.advanceAfter) attrs += QStringLiteral(" advTm=\"%1\"").arg(std::llround(*t.advanceAfter * 1000));
    const QString dir = t.dir;
    QString effect;
    if (t.type == QLatin1String("fade")) effect = QStringLiteral("<p:fade/>");
    else if (t.type == QLatin1String("cut")) effect = QStringLiteral("<p:cut thruBlk=\"1\"/>");
    else if (t.type == QLatin1String("push")) effect = QStringLiteral("<p:push dir=\"%1\"/>").arg(dir);
    else if (t.type == QLatin1String("wipe")) effect = QStringLiteral("<p:wipe dir=\"%1\"/>").arg(dir);
    else if (t.type == QLatin1String("cover")) effect = QStringLiteral("<p:cover dir=\"%1\"/>").arg(dir);
    else if (t.type == QLatin1String("uncover")) effect = QStringLiteral("<p:pull dir=\"%1\"/>").arg(dir);
    else if (t.type == QLatin1String("split")) effect = QStringLiteral("<p:split orient=\"vert\" dir=\"%1\"/>").arg(dir);
    else if (t.type == QLatin1String("zoom")) effect = QStringLiteral("<p:zoom dir=\"%1\"/>").arg(dir);
    // Reveal came with the program's 2010 version, and so did a time of the
    // transition's own (p14:dur) beside the three speeds: programs before it
    // read the fallback, a fade at the nearest speed. Morph came with 2019's,
    // and its fallback is a fade too.
    const bool reveal = t.type == QLatin1String("reveal"), morph = t.isMorph();
    if (!reveal && !morph && std::abs(t.duration - secondsOf(speedOf(t.duration))) < 0.0005)
        return effect.isEmpty() ? QStringLiteral("<p:transition%1/>").arg(attrs) : QStringLiteral("<p:transition%1>%2</p:transition>").arg(attrs, effect);
    QString newer = effect, older = effect, requires = QStringLiteral("p14"), declared;
    if (reveal) {
        newer = QStringLiteral("<p14:reveal dir=\"%1\"/>").arg(dir == QLatin1String("r") ? QStringLiteral("r") : QStringLiteral("l"));
        older = QStringLiteral("<p:fade/>");
    } else if (morph) {
        newer = QStringLiteral("<p159:morph option=\"%1\"/>").arg(optionOf(t.by));
        older = QStringLiteral("<p:fade/>");
        requires = QStringLiteral("p159");
        declared = QStringLiteral(" xmlns:p159=\"%1\"").arg(kP159);
    }
    return QStringLiteral("<mc:AlternateContent xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\">"
                          "<mc:Choice%5 xmlns:p14=\"http://schemas.microsoft.com/office/powerpoint/2010/main\" Requires=\"%6\">"
                          "<p:transition%1 p14:dur=\"%2\">%3</p:transition></mc:Choice>"
                          "<mc:Fallback><p:transition%1>%4</p:transition></mc:Fallback></mc:AlternateContent>")
        .arg(attrs).arg(std::llround(t.duration * 1000)).arg(newer, older, declared, requires);
}

} // namespace jp::deck
