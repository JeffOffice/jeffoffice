#pragma once
// What themes.cpp and themeapply.cpp share: reading and keeping a slide
// master's theme, and the swaps of colors and fonts that a change of theme
// is made of.

#include "core/themes.h"

#include <QHash>
#include <QTextCharFormat>
#include <functional>

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

// The hyperlink colors (to follow, and followed) that read best for a slide master and its layouts, given the colors `t` has now. A link
// can be typed in the text of any placeholder but the footers, so it should read over what is behind each (4.5 to 1 in small text, 3 to 1 in
// large), and in a text box on the page itself (4.5 to 1). Where no color can do both, the page wins. The colors stay as they were when they
// read; otherwise they move toward light or dark as little as will do.
struct LinkColors {
    QColor link, followed;
};
LinkColors linkColorsFor(const Document &d, const MasterPage &master, const DeckTheme &t);

// Gives each slide master's theme the hyperlink colors that read best on it (linkColorsFor), and the links already set in its pages' text the
// same colors.
void fitLinkColors(Document &d, const MasterPage *only = nullptr);   // only that slide master, if given

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
    ColorSwap plain;   // the same colors swapped without the turn of light and dark (for what stands on a backdrop that doesn't turn)
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

// ---- text on a backdrop ----

// The color a run shows (a plain one or one of the scheme), or invalid when the run sets none.
QColor shownRunColor(const Document &d, const QTextCharFormat &cf);
// Gives a run a plain color.
void setRunColor(QTextCharFormat &cf, const QColor &c);
// Changes the format of each run of a story, and of each paragraph's end, that `fn` changes; `fn` is given the position and whether it is a
// paragraph's end.
void editRunsAt(QTextDocument *sd, const std::function<bool(QTextCharFormat &, int, bool)> &fn);

// Keeps the text of pages readable through a change that moves colors, backgrounds, or layouts. `note` takes down, before the change, what
// each run was set in and whether it read on what stood behind it; `keep`, after, looks at what stands behind it now, and where a run that
// read no longer does it takes the old color (swapped plainly), else the nearest color that reads (4.5 to 1; bullets 3 to 1). Text on a
// picture, whose colors aren't told, keeps its old color. Runs that didn't read before are left alone.
class TextReadable {
public:
    explicit TextReadable(Document &d) : m_d(d) {}
    // A page, and the master page it stands on (a layout or slide master: itself).
    void note(const PageBase &page, const MasterPage *base);
    void keep(PageBase &page, const MasterPage *base, const ColorSwap &plain);

private:
    struct Run {
        int pos = 0;
        QColor was;
        double had = 0;   // the contrast it was fit to read at before (4.5, or 3 for large text and bullets); 0 when it didn't read
    };
    struct Story {
        QVector<Run> runs, ends, bullets;
    };
    Document &m_d;
    QHash<QString, Story> m_stories;   // by page and object (and cell)
};

} // namespace jp::deck::detail
