#pragma once
// What themes.cpp and themeapply.cpp share: reading and keeping a slide
// master's theme, and the swaps of colors and fonts that a change of theme
// is made of.

#include "core/themes.h"

#include <QHash>

namespace jp::deck::detail {

// The presentation's first slide master (the one whose theme is the presentation's), or null.
MasterPage *firstSlideMaster(const Document &d);

// The theme a slide master has: the presentation's for the first, else the one it keeps in its
// properties (its theme part), else the presentation's.
DeckTheme masterTheme(const Document &d, const MasterPage &m);
// The theme part kept for a slide master (empty for none).
QByteArray masterThemeXml(const Document &d, const MasterPage &m);
// Keeps `t` as the theme of the slide master: for the first the presentation's, with its fonts and the
// colors new objects take; the theme part kept follows the colors and fonts.
void storeMasterTheme(Document &d, MasterPage &m, const DeckTheme &t);
// Keeps `t` with the theme part `xml` (changed to say t's colors and fonts) for the slide master.
void storeMasterTheme(Document &d, MasterPage &m, const DeckTheme &t, const QByteArray &xml);

// A complete theme part for `t`: its colors and fonts, and format styles that give the program's
// background styles a gradient to show.
QByteArray themePartXml(const DeckTheme &t);
// `xml` with the colors and fonts of `t` in it (the rest as it was); empty if `xml` is no theme part.
QByteArray patchThemePart(const QByteArray &xml, const DeckTheme &t);

// Says `title` and a subtitle on the first slide (a Title Slide), in the look the layout gave it.
void sayOnTitleSlide(Document &d, const QString &title);

// The first slide master's theme as the colors and fonts new objects and the pickers take.
void syncSchemeColors(Document &d);

// The color (a plain one) a fill or stroke has; invalid when it has none or it isn't a plain color.
QColor plainColor(const ColorRef &c);
// The colors a background runs through, first to last.
QVector<QColor> backgroundColors(const Fill &f);

// `t` with its hyperlink colors made readable (4.5 to 1) on `background`, if they weren't.
void keepLinksReadable(DeckTheme &t, const Fill &background);

// The swap of one scheme's colors for another's. With `flip`, dark 1 and light 1 trade places, as do
// dark 2 and light 2 (text turns light on a dark background and dark on a light one).
struct ColorSwap {
    QHash<QRgb, QColor> to;
    QColor operator()(const QColor &c) const;
    bool isEmpty() const { return to.isEmpty(); }
};
ColorSwap colorSwap(const DeckTheme &from, const DeckTheme &to, bool flip);

// The swap of one scheme's fonts for another's. Where the old heading and body fonts are the same, a
// title takes the new heading font and anything else the new body font.
struct FontSwap {
    QString fromHeading, fromBody, toHeading, toBody;
    QString operator()(const QString &family, bool title) const;
    bool isEmpty() const { return fromHeading == toHeading && fromBody == toBody; }
};

struct Swap {
    ColorSwap colors;
    FontSwap fonts;
    bool isEmpty() const { return colors.isEmpty() && fonts.isEmpty(); }
};

// Swaps the colors and fonts on a page: fills, outlines, and the text of every object (groups and tables too).
void swapOnPage(Document &d, PageBase &page, const Swap &swap);

// A slide master's pages: the slide master, its layouts, and the slides standing on them.
struct Family {
    MasterPage *master = nullptr;
    QVector<MasterPage *> layouts;
    QVector<Page *> slides;
};
// Every slide master's, by its id, found in one pass over the pages.
QHash<QString, Family> familiesOf(Document &d);

} // namespace jp::deck::detail
