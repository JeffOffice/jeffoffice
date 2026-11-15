#include "office/themelib.h"

#include "core/fonts.h"
#include "office/package.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QFileInfo>
#include <QSaveFile>
#include <QSettings>
#include <algorithm>

namespace jp::office {

namespace {

struct ColorRow {
    const char *name;
    const char *c[Theme::ColorCount];
};

// dark 1, light 1, dark 2, light 2, accents 1-6, hyperlink, followed hyperlink.
const ColorRow kColorRows[] = {
    {"Lagoon", {"#14303A", "#FFFFFF", "#0B4F6C", "#E3F1F4", "#0B8A9E", "#F2994A", "#2E86AB", "#7FB069", "#C44569", "#5C6BC0", "#0A6E85", "#6A4C93"}},
    {"Paprika", {"#2B1B17", "#FFFFFF", "#5B2A1E", "#F7EBE3", "#D1451F", "#E9A23B", "#8C4A2F", "#3F7D6B", "#B83A59", "#6E5A8F", "#B33A18", "#7A4A8C"}},
    {"Fern", {"#1D2B22", "#FFFFFF", "#24472F", "#EAF2E4", "#3E7C4B", "#A3B745", "#1F6F78", "#D18F2D", "#7B5E8A", "#B5523B", "#2F6B3F", "#74517E"}},
    {"Graphite", {"#1C1F24", "#FFFFFF", "#2F3640", "#EDEFF2", "#4B5563", "#0F766E", "#B45309", "#6D28D9", "#BE123C", "#1D4ED8", "#1E4DB7", "#6D28D9"}},
    {"Plum Wine", {"#2A1630", "#FFFFFF", "#4A1D5A", "#F5ECF7", "#8E2F8B", "#D6628A", "#F2A65A", "#3E7C9C", "#5F8F5B", "#A4392F", "#7A2276", "#59427A"}},
    {"Citrine", {"#2A2411", "#FFFFFF", "#4A3F12", "#FBF5DA", "#C79A00", "#6B8E23", "#D2691E", "#2E7D8C", "#A33F5C", "#5B5F97", "#8A6A00", "#6B4C8F"}},
    {"Nightfall", {"#11182B", "#FFFFFF", "#1C2B4A", "#E6EAF4", "#4C6FD6", "#9B5DE5", "#17B8A6", "#F5B83D", "#E4577A", "#7C8DB5", "#3550C8", "#7440B8"}},
    {"Coral Reef", {"#18323A", "#FFFFFF", "#0F4C5C", "#E8F4F3", "#E36F5C", "#1AA6B7", "#F2B84B", "#4C9F70", "#8E6CC4", "#C9485B", "#0E7C86", "#8E4F8F"}},
    {"Birch", {"#2B2926", "#FFFFFF", "#4A4640", "#F3EFE8", "#8C7A5B", "#5F8A6E", "#C9A66B", "#6F8FA8", "#A3604A", "#7D6B91", "#3B6EA5", "#7D5A8C"}},
    {"Glacier", {"#12283A", "#FFFFFF", "#1B4965", "#E9F3F8", "#3A8FB7", "#62B6CB", "#1B6CA8", "#A9D6E5", "#F28F3B", "#5E60CE", "#18649A", "#6247AA"}},
    {"Clay", {"#2E211C", "#FFFFFF", "#5C3D2E", "#F6ECE4", "#B5651D", "#8F5E4B", "#D9A05B", "#6B7F5E", "#9C4F5B", "#4F6D7A", "#9A4F12", "#6B4C7A"}},
    {"Quince", {"#20261F", "#FFFFFF", "#3A4F2B", "#F2F5E6", "#C2412D", "#6A994E", "#E9A23B", "#386641", "#8C4A6E", "#3D6B8C", "#9C2F1F", "#5C4B8A"}},
};

// The font scheme (JeffPub's names) each built-in theme pairs with its colors.
const char *const kPackFonts[] = {"Modern", "Slab", "Literary", "Corporate", "Elegant", "Civic", "Headline", "Friendly", "Traditional", "Clean", "Editorial", "Classic Sans"};
static_assert(sizeof(kPackFonts) / sizeof(kPackFonts[0]) == sizeof(kColorRows) / sizeof(kColorRows[0]), "one font scheme for each color set");

QString cut(const QString &s, int most) { return cleanThemeText(s.left(most)); }

QString hexOf(const QColor &c) { return c.name(QColor::HexRgb).mid(1).toUpper(); }

// ---- the user's folder ----
QString &folderOverride()
{
    static QString folder;
    return folder;
}

QString storeFile(const char *kind) { return themeStoreFolder() + QLatin1Char('/') + QLatin1String(kind); }

QStringList &noticesHeld()
{
    static QStringList notices;
    return notices;
}

// The file's bytes; `*bad` is set when it is there but can't be used (over the limit, or not readable).
QByteArray readStore(const char *kind, bool *bad)
{
    *bad = false;
    QFile f(storeFile(kind));
    if (!f.exists()) return QByteArray();
    if (!f.open(QIODevice::ReadOnly) || f.size() > kMaxThemeStoreBytes) {
        *bad = true;
        return QByteArray();
    }
    return f.read(kMaxThemeStoreBytes);
}

bool writeStore(const char *kind, const QJsonArray &list)
{
    if (!QDir().mkpath(themeStoreFolder())) return false;
    QSaveFile f(storeFile(kind));
    if (!f.open(QIODevice::WriteOnly)) return false;
    f.write(QJsonDocument(QJsonObject{{QStringLiteral("sets"), list}}).toJson(QJsonDocument::Compact));
    return f.commit();
}

QJsonArray setsOf(const char *kind, bool *bad = nullptr)
{
    bool unusable = false;
    const QByteArray bytes = readStore(kind, &unusable);
    QJsonArray out;
    if (!unusable && !bytes.isEmpty()) {
        QJsonParseError err;
        const QJsonDocument doc = QJsonDocument::fromJson(bytes, &err);
        if (err.error != QJsonParseError::NoError || !doc.isObject() || !doc.object().value(QStringLiteral("sets")).isArray()) unusable = true;
        else out = doc.object().value(QStringLiteral("sets")).toArray();
    }
    if (bad) *bad = unusable;
    return out;
}

// A file of sets that can't be read is never written over: it is kept as "<name>.bad" (an earlier one of that name goes), and the person is told.
void keepUnusableStore(const char *kind)
{
    bool bad = false;
    setsOf(kind, &bad);
    if (!bad) return;
    QFile::remove(storeFile(kind) + QStringLiteral(".bad"));
    if (QFile::rename(storeFile(kind), storeFile(kind) + QStringLiteral(".bad")))
        noticesHeld() << QCoreApplication::translate("ThemeLibrary", "The file of your own theme sets could not be read. It was kept as %1.bad, and a new file was started.").arg(QLatin1String(kind));
}

QJsonObject toJson(const ThemeColors &c)
{
    QJsonArray list;
    for (const QColor &col : c.colors) list << (QLatin1Char('#') + hexOf(col));
    return {{QStringLiteral("name"), c.name}, {QStringLiteral("colors"), list}};
}

QJsonObject toJson(const ThemeFonts &f)
{
    return {{QStringLiteral("name"), f.name}, {QStringLiteral("heading"), f.heading}, {QStringLiteral("body"), f.body}};
}

bool fromJson(const QJsonObject &o, ThemeColors *out)
{
    const QString name = cut(o.value(QStringLiteral("name")).toString(), kMaxThemeNameLength);
    const QJsonArray list = o.value(QStringLiteral("colors")).toArray();
    if (name.isEmpty() || list.size() != Theme::ColorCount) return false;
    ThemeColors c;
    c.name = name;
    for (int i = 0; i < Theme::ColorCount; ++i) {
        const QString s = list.at(i).toString();
        c.colors[i] = QColor(s);
        if (s.size() != 7 || !c.colors[i].isValid()) return false;
    }
    *out = c;
    return true;
}

bool fromJson(const QJsonObject &o, ThemeFonts *out)
{
    ThemeFonts f;
    f.name = cut(o.value(QStringLiteral("name")).toString(), kMaxThemeNameLength);
    f.heading = cut(o.value(QStringLiteral("heading")).toString(), kMaxThemeFontLength);
    f.body = cut(o.value(QStringLiteral("body")).toString(), kMaxThemeFontLength);
    if (f.name.isEmpty() || f.heading.isEmpty() || f.body.isEmpty()) return false;
    *out = f;
    return true;
}

template <class T> QVector<T> readSets(const char *kind)
{
    QVector<T> out;
    for (const QJsonValue &v : setsOf(kind)) {
        if (out.size() >= kMaxCustomThemeSets) break;
        T t;
        if (v.isObject() && fromJson(v.toObject(), &t)) out << t;
    }
    return out;
}

template <class T> bool saveSet(const char *kind, const T &set)
{
    keepUnusableStore(kind);
    QVector<T> all = readSets<T>(kind);
    bool replaced = false;
    for (T &t : all)
        if (t.name.compare(set.name, Qt::CaseInsensitive) == 0) {
            t = set;
            replaced = true;
        }
    if (!replaced) {
        if (all.size() >= kMaxCustomThemeSets) return false;
        all << set;
    }
    QJsonArray list;
    for (const T &t : all) list << toJson(t);
    return writeStore(kind, list);
}

template <class T> bool removeSet(const char *kind, const QString &name)
{
    bool bad = false;
    setsOf(kind, &bad);
    if (bad) return false;
    QVector<T> all = readSets<T>(kind);
    const qsizetype before = all.size();
    all.erase(std::remove_if(all.begin(), all.end(), [&](const T &t) { return t.name.compare(name, Qt::CaseInsensitive) == 0; }), all.end());
    if (all.size() == before) return false;
    QJsonArray list;
    for (const T &t : all) list << toJson(t);
    return writeStore(kind, list);
}

QString xmlEscaped(const QString &s) { return cleanThemeText(s).toHtmlEscaped(); }

} // namespace

bool ThemeColors::operator==(const ThemeColors &o) const
{
    for (int i = 0; i < Theme::ColorCount; ++i)
        if (colors[i] != o.colors[i]) return false;
    return true;
}

QString cleanThemeText(const QString &text)
{
    QString out;
    out.reserve(text.size());
    for (int i = 0; i < text.size(); ++i) {
        const QChar c = text.at(i);
        const ushort u = c.unicode();
        if (c.isHighSurrogate() && i + 1 < text.size() && text.at(i + 1).isLowSurrogate()) {
            out += c;
            out += text.at(++i);
            continue;
        }
        if (u < 0x20 || (u >= 0x7F && u <= 0x9F) || c.isSurrogate() || u == 0xFFFE || u == 0xFFFF) continue;
        out += c;
    }
    return out.trimmed();
}

QStringList takeThemeStoreNotices()
{
    const QStringList out = noticesHeld();
    noticesHeld().clear();
    return out;
}

QString themeSlotLabel(int slot)
{
    switch (slot) {
    case Theme::Dark1: return QCoreApplication::translate("ThemeLibrary", "Dark 1");
    case Theme::Light1: return QCoreApplication::translate("ThemeLibrary", "Light 1");
    case Theme::Dark2: return QCoreApplication::translate("ThemeLibrary", "Dark 2");
    case Theme::Light2: return QCoreApplication::translate("ThemeLibrary", "Light 2");
    case Theme::Accent1: return QCoreApplication::translate("ThemeLibrary", "Accent 1");
    case Theme::Accent2: return QCoreApplication::translate("ThemeLibrary", "Accent 2");
    case Theme::Accent3: return QCoreApplication::translate("ThemeLibrary", "Accent 3");
    case Theme::Accent4: return QCoreApplication::translate("ThemeLibrary", "Accent 4");
    case Theme::Accent5: return QCoreApplication::translate("ThemeLibrary", "Accent 5");
    case Theme::Accent6: return QCoreApplication::translate("ThemeLibrary", "Accent 6");
    case Theme::Hyperlink: return QCoreApplication::translate("ThemeLibrary", "Hyperlink");
    case Theme::FollowedHyperlink: return QCoreApplication::translate("ThemeLibrary", "Followed Hyperlink");
    }
    return QString();
}

ThemeColors themeColorsOf(const Theme &theme)
{
    ThemeColors c;
    c.name = theme.colorSchemeName;
    for (int i = 0; i < Theme::ColorCount; ++i) c.colors[i] = theme.colors[i];
    return c;
}

ThemeFonts themeFontsOf(const Theme &theme)
{
    return {theme.fontSchemeName, theme.majorLatin, theme.minorLatin};
}

Theme withThemeColors(const Theme &theme, const ThemeColors &colors, bool *ok)
{
    if (ok) *ok = true;
    Theme t = theme;
    if (!t.xml.isEmpty()) {
        bool patched = true;
        const QByteArray part = themePartWithColors(t.xml, colors, &patched);
        if (!patched) {
            if (ok) *ok = false;
            return theme;
        }
        t.xml = part;
    }
    for (int i = 0; i < Theme::ColorCount; ++i) t.colors[i] = colors.colors[i];
    if (!colors.name.isEmpty()) t.colorSchemeName = cleanThemeText(colors.name);
    return t;
}

Theme withThemeFonts(const Theme &theme, const ThemeFonts &fonts, bool *ok)
{
    if (ok) *ok = true;
    Theme t = theme;
    if (!t.xml.isEmpty()) {
        bool patched = true;
        const QByteArray part = themePartWithFonts(t.xml, fonts, &patched);
        if (!patched) {
            if (ok) *ok = false;
            return theme;
        }
        t.xml = part;
    }
    t.majorLatin = cleanThemeText(fonts.heading);
    t.minorLatin = cleanThemeText(fonts.body);
    if (!fonts.name.isEmpty()) t.fontSchemeName = cleanThemeText(fonts.name);
    return t;
}

QByteArray themePartWithColors(const QByteArray &part, const ThemeColors &colors, bool *ok)
{
    if (ok) *ok = false;
    static const QRegularExpression scheme(QStringLiteral("<((?:\\w+:)?)clrScheme\\b[^>]*?(?:/>|>.*?</\\1clrScheme>)"), QRegularExpression::DotMatchesEverythingOption);
    const QString xml = QString::fromUtf8(part);
    const QRegularExpressionMatch m = scheme.match(xml);
    if (!m.hasMatch()) return part;
    if (ok) *ok = true;
    const QString p = m.captured(1);
    QString made = QLatin1Char('<') + p + QStringLiteral("clrScheme name=\"") + xmlEscaped(colors.name) + QStringLiteral("\">");
    for (int i = 0; i < Theme::ColorCount; ++i) {
        const QString slot = themeColorName(Theme::Color(i));
        made += QLatin1Char('<') + p + slot + QStringLiteral("><") + p + QStringLiteral("srgbClr val=\"") + hexOf(colors.colors[i]) + QStringLiteral("\"/></") + p + slot + QLatin1Char('>');
    }
    made += QStringLiteral("</") + p + QStringLiteral("clrScheme>");
    QString out = xml;
    out.replace(m.capturedStart(), m.capturedLength(), made);
    return out.toUtf8();
}

QByteArray themePartWithFonts(const QByteArray &part, const ThemeFonts &fonts, bool *ok)
{
    if (ok) *ok = false;
    QString xml = QString::fromUtf8(part);
    int found = 0;
    for (const char *kind : {"majorFont", "minorFont"}) {
        const QString face = xmlEscaped(kind[1] == 'a' ? fonts.heading : fonts.body);
        const QRegularExpression list(QStringLiteral("(<((?:\\w+:)?)%1\\b[^>]*>\\s*)<(?:\\w+:)?latin\\b[^>]*?/>").arg(QLatin1String(kind)), QRegularExpression::DotMatchesEverythingOption);
        const QRegularExpressionMatch m = list.match(xml);
        if (!m.hasMatch()) continue;
        ++found;
        const QString latin = m.captured(1) + QLatin1Char('<') + m.captured(2) + QStringLiteral("latin typeface=\"") + face + QStringLiteral("\"/>");
        xml.replace(m.capturedStart(), m.capturedLength(), latin);
    }
    if (found < 2) return part;
    if (ok) *ok = true;
    static const QRegularExpression named(QStringLiteral("(<(?:\\w+:)?fontScheme\\b[^>]*?\\bname=\")[^\"]*(\")"));
    if (!fonts.name.isEmpty()) {
        const QRegularExpressionMatch m = named.match(xml);
        if (m.hasMatch()) xml.replace(m.capturedStart(), m.capturedLength(), m.captured(1) + xmlEscaped(fonts.name) + m.captured(2));
    }
    return xml.toUtf8();
}

const QVector<ThemeColors> &builtinThemeColors()
{
    static const QVector<ThemeColors> list = [] {
        QVector<ThemeColors> out;
        for (const ColorRow &row : kColorRows) {
            ThemeColors c;
            c.name = QString::fromLatin1(row.name);
            for (int i = 0; i < Theme::ColorCount; ++i) c.colors[i] = QColor(QLatin1String(row.c[i]));
            out << c;
        }
        return out;
    }();
    return list;
}

QVector<ThemeFonts> builtinThemeFonts()
{
    // (kept once the fonts are loaded; before that the answer is only the stand-in, which is not kept)
    static QVector<ThemeFonts> kept;
    if (!kept.isEmpty()) return kept;
    QVector<ThemeFonts> out;
    const QStringList available = bundledFamilies();
    for (const jp::FontScheme &s : builtinFontSchemes())
        if (available.contains(s.heading) && available.contains(s.body)) out << ThemeFonts{s.name, s.heading, s.body};
    if (out.isEmpty()) return {ThemeFonts{QStringLiteral("JeffOffice"), defaultTheme().majorLatin, defaultTheme().minorLatin}};
    kept = out;
    return out;
}

QVector<ThemePack> builtinThemePacks()
{
    static QVector<ThemePack> kept;
    if (!kept.isEmpty()) return kept;
    const QVector<ThemeFonts> fonts = builtinThemeFonts();
    QVector<ThemePack> out;
    const auto &colors = builtinThemeColors();
    for (int i = 0; i < colors.size(); ++i) {
        ThemeFonts f = fonts.first();
        for (const ThemeFonts &candidate : fonts)
            if (candidate.name == QLatin1String(kPackFonts[i])) f = candidate;
        out << ThemePack{colors[i].name, colors[i], f};
    }
    if (fonts.size() > 1) kept = out;
    return out;
}

ThemeColors themeColorsFromScheme(const jp::ColorScheme &s)
{
    ThemeColors c;
    c.name = s.name;
    c.colors[Theme::Dark1] = s.c[jp::Main];
    c.colors[Theme::Light1] = QColor(Qt::white);
    c.colors[Theme::Dark2] = mix(s.c[jp::Main], s.c[jp::Accent1], 0.3);
    c.colors[Theme::Light2] = s.c[jp::Accent5];
    c.colors[Theme::Accent1] = s.c[jp::Accent1];
    c.colors[Theme::Accent2] = s.c[jp::Accent2];
    c.colors[Theme::Accent3] = s.c[jp::Accent3];
    c.colors[Theme::Accent4] = s.c[jp::Accent4];
    c.colors[Theme::Accent5] = mix(s.c[jp::Accent1], s.c[jp::Accent3], 0.5);
    c.colors[Theme::Accent6] = mix(s.c[jp::Accent2], s.c[jp::Accent4], 0.5);
    c.colors[Theme::Hyperlink] = s.c[jp::Hyperlink];
    c.colors[Theme::FollowedHyperlink] = s.c[jp::Followed];
    return c;
}

jp::ColorScheme schemeFromThemeColors(const ThemeColors &c)
{
    jp::ColorScheme s;
    s.name = c.name;
    s.c[jp::Main] = c.colors[Theme::Dark1];
    s.c[jp::Accent1] = c.colors[Theme::Accent1];
    s.c[jp::Accent2] = c.colors[Theme::Accent2];
    s.c[jp::Accent3] = c.colors[Theme::Accent3];
    s.c[jp::Accent4] = c.colors[Theme::Accent4];
    s.c[jp::Accent5] = c.colors[Theme::Light2];
    s.c[jp::Hyperlink] = c.colors[Theme::Hyperlink];
    s.c[jp::Followed] = c.colors[Theme::FollowedHyperlink];
    return s;
}

QString themeStoreFolder()
{
    if (!folderOverride().isEmpty()) return folderOverride();
    // (next to the suite's settings file, so it is the place the settings go to wherever they are moved: a test program's run has its own)
    static const QString name = QStringLiteral("JeffOffice");
    return QFileInfo(QSettings(QSettings::IniFormat, QSettings::UserScope, name, name).fileName()).absolutePath() + QStringLiteral("/themes");
}

void setThemeStoreFolder(const QString &folder) { folderOverride() = folder; }

QVector<ThemeColors> customThemeColors() { return readSets<ThemeColors>("colors.json"); }
QVector<ThemeFonts> customThemeFonts() { return readSets<ThemeFonts>("fonts.json"); }

bool saveCustomThemeColors(const ThemeColors &colors)
{
    ThemeColors c = colors;
    c.name = cut(c.name, kMaxThemeNameLength);
    if (c.name.isEmpty()) return false;
    for (const QColor &col : c.colors)
        if (!col.isValid()) return false;
    return saveSet("colors.json", c);
}

bool saveCustomThemeFonts(const ThemeFonts &fonts)
{
    ThemeFonts f;
    f.name = cut(fonts.name, kMaxThemeNameLength);
    f.heading = cut(fonts.heading, kMaxThemeFontLength);
    f.body = cut(fonts.body, kMaxThemeFontLength);
    if (f.name.isEmpty() || f.heading.isEmpty() || f.body.isEmpty()) return false;
    return saveSet("fonts.json", f);
}

bool removeCustomThemeColors(const QString &name) { return removeSet<ThemeColors>("colors.json", name); }
bool removeCustomThemeFonts(const QString &name) { return removeSet<ThemeFonts>("fonts.json", name); }

QByteArray writeThemeFile(const Theme &theme)
{
    PackageWriter w;
    const QString type = QStringLiteral("application/vnd.openxmlformats-officedocument.theme+xml");
    const QString manager = QStringLiteral("/theme/theme/themeManager.xml");
    w.addPart(manager, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<a:themeManager xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\"/>",
              QStringLiteral("application/vnd.openxmlformats-officedocument.themeManager+xml"));
    w.addPart(QStringLiteral("/theme/theme/theme1.xml"), writeTheme(theme), type);
    w.addRelationship(QStringLiteral("/"), QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/themeManager"), manager);
    w.addRelationship(manager, QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/theme"), QStringLiteral("/theme/theme/theme1.xml"));
    return w.finish();
}

bool readThemeFromPackage(const QByteArray &zip, Theme *theme, QString *error, bool themeFileOnly)
{
    auto fail = [&](const QString &why) {
        if (error) *error = why;
        return false;
    };
    Package pkg;
    QString why;
    // (a theme file holds one small part: its directory may not claim more; any other package may hold pictures, and only the theme parts asked for are inflated)
    if (!pkg.readLazy(zip, &why, themeFileOnly ? kMaxThemeFileBytes : Package::kLazyMaxInflated))
        return fail(why.isEmpty() ? QCoreApplication::translate("ThemeLibrary", "The file is not a package.") : why);
    int tried = 0;
    qint64 inflated = 0;
    bool over = false;
    auto tryPart = [&](const QString &name) {
        if (over || tried >= kMaxThemePartsTried || !pkg.hasPart(name)) return false;
        ++tried;
        const qint64 size = pkg.partSize(name);
        if (size < 0 || size > kMaxThemePartBytes) return false;
        inflated += size;
        if (inflated > kMaxThemeFileBytes) {
            over = true;
            return false;
        }
        Theme t;
        if (!readTheme(pkg.part(name), &t, nullptr)) return false;
        *theme = t;
        return true;
    };
    // The theme the main part names (a presentation or document has one), then any theme part in the folders themes are kept in.
    const QString main = pkg.mainPart();
    if (!main.isEmpty())
        for (const Relationship &r : pkg.relationshipsOfType(main, QStringLiteral("/theme")))
            if (!r.external && tryPart(r.target)) return true;
    static const QRegularExpression named(QStringLiteral("^/(?:ppt|word|xl|theme)/theme/theme\\d*\\.xml$"), QRegularExpression::CaseInsensitiveOption);
    for (const QString &name : pkg.partNames()) {
        if (tried >= kMaxThemePartsTried || over) break;
        if (named.match(name).hasMatch() && tryPart(name)) return true;
    }
    if (over) return fail(QCoreApplication::translate("ThemeLibrary", "The theme parts of the file are too large."));
    return fail(QCoreApplication::translate("ThemeLibrary", "The file holds no theme."));
}

bool readThemeFile(const QString &path, Theme *theme, QString *error)
{
    const bool themeFile = path.endsWith(QLatin1String(".thmx"), Qt::CaseInsensitive);
    QByteArray bytes;
    QString why;
    if (!readWholeFile(path, &bytes, &why, themeFile ? kMaxThemeFileBytes : qint64(1) << 30)) {
        if (error) *error = why;
        return false;
    }
    return readThemeFromPackage(bytes, theme, error, themeFile);
}

} // namespace jp::office
