#include "io/pptxshow.h"

#include "core/showsettings.h"
#include "office/drawingml_color.h"
#include "office/xml.h"

#include <QCoreApplication>
#include <QDomDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QSet>
#include <algorithm>

namespace jp::deck {

namespace {

using office::ns::P;

const QLatin1String kP14("http://schemas.microsoft.com/office/powerpoint/2010/main");
// The extension of p:showPr that holds the laser pointer's color.
const QLatin1String kLaserExt("{EC167BDD-8182-4AB7-AECC-EB403E3ABB37}");
const QString kKeptKey = QStringLiteral("presPropsKept");


QDomElement pChild(const QDomElement &e, const char *name) { return office::child(e, P, QLatin1String(name)); }

// Text for an attribute: markup escaped, and what XML 1.0 can't hold left out.
QString attribute(const QString &text)
{
    QString out;
    out.reserve(text.size());
    for (int i = 0; i < text.size(); ++i) {
        const QChar c = text[i];
        const ushort u = c.unicode();
        if (c.isHighSurrogate() && i + 1 < text.size() && text[i + 1].isLowSurrogate()) {
            out += c;
            out += text[++i];
        } else if (u == '&') out += QLatin1String("&amp;");
        else if (u == '<') out += QLatin1String("&lt;");
        else if (u == '>') out += QLatin1String("&gt;");
        else if (u == '"') out += QLatin1String("&quot;");
        else if (u >= 0x20 && !c.isSurrogate() && u != 0xFFFE && u != 0xFFFF) out += c;
    }
    return out;
}

QString hex(const QColor &c) { return QStringLiteral("%1%2%3").arg(c.red(), 2, 16, QLatin1Char('0')).arg(c.green(), 2, 16, QLatin1Char('0')).arg(c.blue(), 2, 16, QLatin1Char('0')).toUpper(); }

// Whether the element, or one under it, has an attribute that names a relationship: the kept parts are not kept.
bool refersToParts(const QDomElement &e, int depth = 0)
{
    if (depth > 64) return true;
    const QDomNamedNodeMap attrs = e.attributes();
    for (int i = 0; i < attrs.length(); ++i)
        if (office::sameNamespace(attrs.item(i).namespaceURI(), office::ns::R)) return true;
    for (QDomElement c = e.firstChildElement(); !c.isNull(); c = c.nextSiblingElement())
        if (refersToParts(c, depth + 1)) return true;
    return false;
}

bool keepable(const QString &xml)
{
    QDomDocument dom;
    return !xml.isEmpty() && office::parseXml(xml.toUtf8(), &dom) && !refersToParts(dom.documentElement());
}

QString colorXml(const QColor &c, bool preset)
{
    if (preset && c == QColor(255, 0, 0)) return QStringLiteral("<a:prstClr val=\"red\"/>");
    return QStringLiteral("<a:srgbClr val=\"%1\"/>").arg(hex(c));
}

} // namespace

// ---- reading

void readCustomShows(const QDomElement &presentation, const QHash<QString, int> &pageOfRel, Document &doc, QStringList *notes)
{
    const QDomElement list = pChild(presentation, "custShowLst");
    if (list.isNull()) return;
    QVector<CustomShow> shows;
    QSet<qint64> ids;
    int total = 0;
    bool tooMany = false, tooLong = false, unknownSlide = false;
    for (const QDomElement &s : office::children(list, P, QStringLiteral("custShow"))) {
        if (shows.size() >= kMaxCustomShows) {
            tooMany = true;
            break;
        }
        CustomShow show;
        bool ok = false;
        const qint64 id = office::parseInt(office::attr(s, QStringLiteral("id")), -1, &ok);
        // An id that repeats or isn't a number is made new by setCustomShows().
        show.id = ok && id >= 0 && id <= 4294967295LL && !ids.contains(id) ? id : -1;
        if (show.id >= 0) ids.insert(show.id);
        show.name = office::attr(s, QStringLiteral("name")).left(kMaxCustomShowName);
        for (const QDomElement &sld : office::children(pChild(s, "sldLst"), P, QStringLiteral("sld"))) {
            if (show.slides.size() >= kMaxCustomShowSlides || total >= kMaxCustomShowEntries) {
                tooLong = true;
                break;
            }
            const int page = pageOfRel.value(office::attrNs(sld, office::ns::R, QStringLiteral("id")), -1);
            if (page < 0 || page >= doc.pages.size()) {
                unknownSlide = true;
                continue;
            }
            show.slides << doc.pages[page]->id;
            ++total;
        }
        if (show.slides.isEmpty()) continue;
        shows << show;
    }
    // Ids a file left out or repeated: the next free ones.
    qint64 next = 0;
    for (const CustomShow &s : std::as_const(shows)) next = std::max(next, s.id + 1);
    for (CustomShow &s : shows)
        if (s.id < 0) s.id = next++;
    // A show with no name is the next "Custom Show n" no show has: one pass, not a look at every show for each.
    QSet<QString> names;
    for (const CustomShow &s : std::as_const(shows)) names.insert(s.name.toLower());
    int counter = 1;
    for (CustomShow &s : shows) {
        if (!s.name.isEmpty()) continue;
        while (names.contains(customShowDefaultName(counter).toLower())) ++counter;
        s.name = customShowDefaultName(counter);
        names.insert(s.name.toLower());
    }
    setCustomShows(doc, shows);
    if (notes) {
        if (tooMany) *notes << QCoreApplication::translate("PptxImport", "Only the first %1 custom slide shows are read.").arg(kMaxCustomShows);
        if (tooLong) *notes << QCoreApplication::translate("PptxImport", "A custom slide show lists more slides than JeffDeck reads; the rest are left out.");
        if (unknownSlide) *notes << QCoreApplication::translate("PptxImport", "A custom slide show lists a slide that is not in the presentation; it is left out.");
    }
}

void readShowProperties(const QDomElement &presProps, const office::Theme &theme, const std::function<QString(const QDomElement &)> &standalone, Document &doc,
                        QStringList *notes)
{
    QJsonObject kept;
    QJsonArray before, after, showExt;
    qint64 bytes = 0;
    bool tooLarge = false;
    auto keep = [&](QJsonArray &into, const QDomElement &e) {
        if (tooLarge) return;
        const QString xml = standalone(e);
        if (bytes + xml.size() * 2 > kMaxKeptPropsBytes) {
            tooLarge = true;
            return;
        }
        bytes += xml.size() * 2;
        into.append(xml);
    };
    ShowSettings settings;
    const QVector<CustomShow> shows = customShowsOf(doc);
    for (const QDomElement &c : office::children(presProps)) {
        const QString name = c.localName();
        if (name == QLatin1String("htmlPubPr") || name == QLatin1String("webPr") || name == QLatin1String("prnPr")) keep(before, c);
        else if (name == QLatin1String("clrMru") || name == QLatin1String("extLst")) keep(after, c);
        else if (name == QLatin1String("showPr") && office::isElement(c, P, name) && !kept.contains(QStringLiteral("showPr"))) {   // the first only
            kept["showPr"] = true;
            const office::ColorMap map;
            settings.loop = office::parseBool(office::attr(c, QStringLiteral("loop")), false);
            settings.narration = office::parseBool(office::attr(c, QStringLiteral("showNarration")), false);
            settings.animation = office::parseBool(office::attr(c, QStringLiteral("showAnimation")), true);
            settings.useTimings = office::parseBool(office::attr(c, QStringLiteral("useTimings")), true);
            if (!pChild(c, "kiosk").isNull()) {
                settings.type = ShowSettings::Type::Kiosk;
                bool ok = false;
                const qint64 restart = office::parseInt(office::attr(pChild(c, "kiosk"), QStringLiteral("restart")), 300000, &ok);
                if (ok && restart >= 0 && restart <= 4294967295LL) settings.kioskRestart = restart;
            } else if (!pChild(c, "browse").isNull()) {
                settings.type = ShowSettings::Type::Individual;
                settings.scrollbar = office::parseBool(office::attr(pChild(c, "browse"), QStringLiteral("showScrollbar")), true);
            }
            if (const QDomElement range = pChild(c, "sldRg"); !range.isNull()) {
                const qint64 st = office::parseInt(office::attr(range, QStringLiteral("st")), 1), end = office::parseInt(office::attr(range, QStringLiteral("end")), st);
                settings.slides = ShowSettings::Slides::Range;
                settings.from = int(std::clamp<qint64>(st, 1, 100000));
                settings.to = int(std::clamp<qint64>(end, 1, 100000));
            } else if (const QDomElement custom = pChild(c, "custShow"); !custom.isNull()) {
                const qint64 id = office::parseInt(office::attr(custom, QStringLiteral("id")), -1);
                if (customShowById(shows, id)) {
                    settings.slides = ShowSettings::Slides::Custom;
                    settings.customShow = id;
                } else if (notes) {
                    *notes << QCoreApplication::translate("PptxImport", "The slide show was set to play a custom slide show that is not in the file, so it plays every slide.");
                }
            }
            if (const QDomElement pen = pChild(c, "penClr"); !pen.isNull()) {
                const QColor color = office::readColor(pen, theme, map);
                if (color.isValid()) settings.penColor = QColor(color.red(), color.green(), color.blue());
            }
            for (const QDomElement &ext : office::children(pChild(c, "extLst"), P, QStringLiteral("ext"))) {
                if (office::attr(ext, QStringLiteral("uri")).compare(kLaserExt, Qt::CaseInsensitive) == 0) {
                    const QColor color = office::readColor(office::child(ext, kP14, QStringLiteral("laserClr")), theme, map);
                    if (color.isValid()) settings.laserColor = QColor(color.red(), color.green(), color.blue());
                } else {
                    keep(showExt, ext);
                }
            }
        }
    }
    setShowSettings(doc, settings);
    if (!before.isEmpty()) kept["before"] = before;
    if (!after.isEmpty()) kept["after"] = after;
    if (!showExt.isEmpty()) kept["showExt"] = showExt;
    if (kept.isEmpty()) doc.appData.remove(kKeptKey);
    else doc.appData[kKeptKey] = kept;
    if (tooLarge && notes) *notes << QCoreApplication::translate("PptxImport", "The presentation's other slide show settings are too large to keep, so some are left out.");
}

// ---- writing

QString customShowListXml(const Document &doc, const QStringList &slideRels)
{
    const QVector<CustomShow> shows = customShowsOf(doc);
    const QHash<QString, int> pages = pageIndexById(doc);
    QString xml;
    for (const CustomShow &show : shows) {
        QString slides;
        for (const int page : customShowSlides(pages, show))
            if (page < slideRels.size() && !slideRels[page].isEmpty()) slides += QStringLiteral("<p:sld r:id=\"%1\"/>").arg(attribute(slideRels[page]));
        if (slides.isEmpty()) continue;
        xml += QStringLiteral("<p:custShow name=\"%1\" id=\"%2\"><p:sldLst>%3</p:sldLst></p:custShow>").arg(attribute(show.name)).arg(show.id).arg(slides);
    }
    return xml.isEmpty() ? QString() : QStringLiteral("<p:custShowLst>") + xml + QStringLiteral("</p:custShowLst>");
}

QString presentationPropertiesXml(const Document &doc, const QString &namespaces)
{
    const QJsonObject kept = doc.appData.value(kKeptKey).toObject();
    const ShowSettings s = showSettingsOf(doc);
    QString xml = QStringLiteral("<p:presentationPr%1").arg(namespaces);
    QString inner;
    auto keptXml = [&](const char *key) {
        QString out;
        for (const QJsonValue &v : kept.value(QLatin1String(key)).toArray())
            if (keepable(v.toString())) out += v.toString();
        return out;
    };
    inner += keptXml("before");
    const QString showExt = keptXml("showExt");
    if (s != ShowSettings() || kept.value(QStringLiteral("showPr")).toBool() || !showExt.isEmpty()) {
        QString show = QStringLiteral("<p:showPr");
        if (s.loop) show += QStringLiteral(" loop=\"1\"");
        if (s.narration) show += QStringLiteral(" showNarration=\"1\"");
        if (!s.animation) show += QStringLiteral(" showAnimation=\"0\"");
        if (!s.useTimings) show += QStringLiteral(" useTimings=\"0\"");
        show += QLatin1Char('>');
        if (s.type == ShowSettings::Type::Kiosk) show += s.kioskRestart == 300000 ? QStringLiteral("<p:kiosk/>") : QStringLiteral("<p:kiosk restart=\"%1\"/>").arg(s.kioskRestart);
        else if (s.type == ShowSettings::Type::Individual) show += s.scrollbar ? QStringLiteral("<p:browse/>") : QStringLiteral("<p:browse showScrollbar=\"0\"/>");
        else show += QStringLiteral("<p:present/>");
        const QVector<CustomShow> shows = customShowsOf(doc);
        if (s.slides == ShowSettings::Slides::Custom && customShowById(shows, s.customShow)) show += QStringLiteral("<p:custShow id=\"%1\"/>").arg(s.customShow);
        else if (s.slides == ShowSettings::Slides::Range) show += QStringLiteral("<p:sldRg st=\"%1\" end=\"%2\"/>").arg(s.from).arg(std::max(s.from, s.to));
        else show += QStringLiteral("<p:sldAll/>");
        show += QStringLiteral("<p:penClr>") + colorXml(s.penColor, true) + QStringLiteral("</p:penClr>");
        show += QStringLiteral("<p:extLst><p:ext uri=\"%1\"><p14:laserClr xmlns:p14=\"%2\">").arg(kLaserExt, kP14) + colorXml(s.laserColor, false)
                + QStringLiteral("</p14:laserClr></p:ext>") + showExt + QStringLiteral("</p:extLst></p:showPr>");
        inner += show;
    }
    inner += keptXml("after");
    return inner.isEmpty() ? xml + QStringLiteral("/>") : xml + QLatin1Char('>') + inner + QStringLiteral("</p:presentationPr>");
}

} // namespace jp::deck
