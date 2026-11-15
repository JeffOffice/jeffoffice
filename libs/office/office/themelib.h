#pragma once
// Themes as the apps share them: the twelve colors of a theme and its heading and body fonts as sets with names, the built-in sets, the sets a person makes (kept
// in the user's settings folder, so every app shows them), a theme part with its colors or fonts changed, and theme files (.thmx, a theme package written here) and the
// themes inside presentations and documents. JeffDeck, JeffWrite, and the rest take their Colors, Fonts, and Themes galleries from here.

#include "core/color.h"
#include "office/theme.h"

#include <QByteArray>
#include <QColor>
#include <QString>
#include <QVector>

namespace jp::office {

// The twelve colors under a name, in the order of Theme::Color.
struct ThemeColors {
    QString name;
    QColor colors[Theme::ColorCount];
    bool operator==(const ThemeColors &o) const;   // the colors only
};
// The fonts of headings and of body text under a name.
struct ThemeFonts {
    QString name, heading, body;
    bool operator==(const ThemeFonts &o) const { return heading == o.heading && body == o.body; }
};
// A whole theme of the Themes gallery: colors and fonts that go together.
struct ThemePack {
    QString name;
    ThemeColors colors;
    ThemeFonts fonts;
};

// What a file or the user's folder may hold (anything more is left out).
constexpr int kMaxCustomThemeSets = 200;
constexpr int kMaxThemeNameLength = 80;
constexpr int kMaxThemeFontLength = 120;
constexpr qint64 kMaxThemeStoreBytes = qint64(1) << 20;
constexpr qint64 kMaxThemePartBytes = qint64(4) << 20;
constexpr qint64 kMaxThemeFileBytes = qint64(16) << 20;      // a .thmx file, and the most any theme parts may inflate to together
constexpr int kMaxThemePartsTried = 8;                       // theme parts looked at in a presentation or document

// The name of one of the twelve slots as people read it ("Dark 1", "Accent 3", "Followed Hyperlink").
QString themeSlotLabel(int slot);

// `text` without the characters a name can't have (control characters and the ones XML forbids), and without the space at its ends.
QString cleanThemeText(const QString &text);

ThemeColors themeColorsOf(const Theme &theme);
ThemeFonts themeFontsOf(const Theme &theme);
// `theme` with these colors (and name) or these fonts (and name). The part as read is changed where it holds them, so its effect styles and the rest stay. When the part has no place for
// them (it is not what it should be), `theme` comes back as it was and `*ok` is false.
Theme withThemeColors(const Theme &theme, const ThemeColors &colors, bool *ok = nullptr);
Theme withThemeFonts(const Theme &theme, const ThemeFonts &fonts, bool *ok = nullptr);
// The same on a theme part: the color scheme or the Latin heading and body typefaces are put in, the rest of the part kept as it is. A part that has no place for them is returned as it
// came, with `*ok` false.
QByteArray themePartWithColors(const QByteArray &part, const ThemeColors &colors, bool *ok = nullptr);
QByteArray themePartWithFonts(const QByteArray &part, const ThemeFonts &fonts, bool *ok = nullptr);

// The built-in sets, of JeffOffice's own naming and design. The fonts are those of JeffPub's font schemes that are installed or bundled.
const QVector<ThemeColors> &builtinThemeColors();
QVector<ThemeFonts> builtinThemeFonts();
QVector<ThemePack> builtinThemePacks();

// JeffPub's color scheme (eight slots) as twelve colors, and back: its main color is Dark 1, its fifth accent Light 2, and the rest are worked out, so the round trip keeps the
// scheme's own eight colors.
ThemeColors themeColorsFromScheme(const jp::ColorScheme &scheme);
jp::ColorScheme schemeFromThemeColors(const ThemeColors &colors);

// The sets a person made, kept in the user's settings folder (the same for every app). Saving a set whose name is there already (any case) replaces it; a set with no name or
// fonts with a name over the limit is refused (false). Names are cut at kMaxThemeNameLength.
QVector<ThemeColors> customThemeColors();
QVector<ThemeFonts> customThemeFonts();
bool saveCustomThemeColors(const ThemeColors &colors);
bool saveCustomThemeFonts(const ThemeFonts &fonts);
bool removeCustomThemeColors(const QString &name);
bool removeCustomThemeFonts(const QString &name);
// When a file of sets in the folder could not be read (too large, or not what it should be) a save keeps it as "<name>.bad" and starts a new one; the notice says so, once.
// Taking the notices clears them.
QStringList takeThemeStoreNotices();
// The folder they are in; tests may give another (empty: the usual one).
QString themeStoreFolder();
void setThemeStoreFolder(const QString &folder);

// A theme file (.thmx): a package of one theme part, with the part's own colors, fonts, and effect styles.
QByteArray writeThemeFile(const Theme &theme);
// The theme of a package (a .thmx, or a presentation, template, or document that holds one): the first theme part of the file's own, as the package's rels name it or, failing
// that, the first of the folders a theme is kept in. False, with a message, if the file isn't a package, holds no theme, or is over the limits: at most kMaxThemePartsTried theme parts are
// looked at, and the parts inflated together stay under kMaxThemeFileBytes. With `themeFileOnly` (a .thmx) the whole package may inflate to no more than that, as its directory says.
bool readThemeFromPackage(const QByteArray &zip, Theme *theme, QString *error = nullptr, bool themeFileOnly = false);
bool readThemeFile(const QString &path, Theme *theme, QString *error = nullptr);

} // namespace jp::office
