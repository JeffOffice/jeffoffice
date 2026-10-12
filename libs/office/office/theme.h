#pragma once
// The theme part (a:theme): the twelve colors and the fonts that a
// presentation, document, or workbook's drawings refer to by name
// ("accent1", the heading font) instead of by value, so one change to the
// theme restyles all of it.

#include <QByteArray>
#include <QColor>
#include <QString>

namespace jp::office {

struct Theme {
    // In the order of the file's a:clrScheme.
    enum Color { Dark1, Light1, Dark2, Light2, Accent1, Accent2, Accent3, Accent4, Accent5, Accent6, Hyperlink, FollowedHyperlink, ColorCount };
    QString name, colorSchemeName, fontSchemeName;
    QColor colors[ColorCount];
    // The Latin, East Asian, and complex-script typefaces for headings
    // (major) and body text (minor); empty for none.
    QString majorLatin, minorLatin, majorEastAsian, minorEastAsian, majorComplex, minorComplex;
    // The part as read (empty for a theme made here). writeTheme gives it
    // back as it was, so what this struct doesn't hold (the format scheme,
    // per-script fonts, extensions) survives a save; clear it after changing
    // a field above, or the change is not written.
    QByteArray xml;
};

// A color's name in the XML: "dk1", "lt1", "dk2", "lt2", "accent1" to
// "accent6", "hlink", "folHlink".
QString themeColorName(Theme::Color slot);
// The slot a name stands for; false if it is none of those twelve.
bool themeColorFromName(const QString &name, Theme::Color *slot);

// Reads a theme part. False, with a message, if it isn't XML or isn't a
// theme with a color scheme. A color the part lacks or can't read takes the
// one from defaultTheme(); fonts it lacks are empty.
bool readTheme(const QByteArray &xml, Theme *theme, QString *error = nullptr);
// `xml` if the theme has it; else a complete, valid part made from the
// fields (the format scheme in it plain: three fills, three lines, three
// effect styles, and three background fills). A color left unset is written
// as defaultTheme()'s.
QByteArray writeTheme(const Theme &theme);
// JeffOffice's own: Carlito for headings and body, and colors that are
// JeffOffice's, not any other suite's.
Theme defaultTheme();

} // namespace jp::office
