#pragma once
// JeffDeck's themes. A theme is the colors and fonts of a presentation, its
// slide master with the standard layouts and what they draw (bands, lines,
// corner shapes), and its backgrounds. JeffDeck ships nine (the first is the
// one a new presentation has), each with four variants: another color scheme,
// and where it fits another background.
//
// A theme is applied from a presentation that holds it (one of the built-in
// themes' or a file's): its slide master and layouts are copied in, every
// slide goes to the layout of its own kind, and its text takes the new fonts
// and colors where it had the old theme's, not where it was set by hand.
//
// The colors that carry a theme are plain colors on the objects (saving finds
// the theme's colors in them again), so a change of colors is a swap of one
// color for another wherever a color of the old scheme is found.

#include "core/deck.h"

#include <QList>
#include <QString>
#include <QVector>
#include <memory>

namespace jp::deck {

// A color scheme: the twelve colors a DeckTheme holds, under a name.
struct ColorSet {
    QString name;
    QColor colors[12];
};
// A font scheme: the fonts of headings and of body text, under a name.
struct FontSet {
    QString name, heading, body;
};
// A variant of a theme: a color scheme, and one of the twelve background styles.
struct Variant {
    ColorSet colors;
    int background = 0;
};

// The twelve background styles of a theme: its Light 1, Light 2, Dark 2, and
// Dark 1 colors (style / 3, in that order), each flat, as a soft gradient, and
// as a stronger one (style % 3). The gradients run from the color toward the
// theme's first accent.
constexpr int kBackgroundStyles = 12;
Fill backgroundStyle(const DeckTheme &theme, int style);
QString backgroundStyleName(int style);
// Whether the style is a dark one (Dark 2 and Dark 1): text on it is light.
bool backgroundStyleIsDark(int style);
// Which of the twelve a background is for `theme`, or -1.
int backgroundStyleOf(const Fill &background, const DeckTheme &theme);
// Whether text must be light on this background.
bool isDarkBackground(const Fill &background);

// The built-in themes. The first is the one a new presentation has.
int builtinThemeCount();
QString builtinThemeName(int theme);
// The theme a name is, or -1.
int builtinThemeIndex(const QString &name);
FontSet builtinThemeFonts(int theme);
// The four variants; the first is the theme as it is first applied.
QVector<Variant> builtinThemeVariants(int theme);
// The color schemes and font schemes the Variants menu lists: those of the
// built-in themes' variants, and for fonts JeffPub's font schemes after them.
QVector<ColorSet> colorSets();
QVector<FontSet> fontSets();

// A presentation in the theme: its slide master, the nine standard layouts, and
// a Title Slide that says the theme's name; in variant `variant` of it. 16:9.
std::unique_ptr<Document> themePresentation(int theme, int variant = 0);

// A one-slide presentation in the theme `d` has (its first slide master and layouts, and its colors), whatever the theme is: what the
// galleries show. And the same in variant `variant` (of variantsOf).
std::unique_ptr<Document> themeSample(const Document &d);
std::unique_ptr<Document> variantSample(const Document &d, int variant);

// The four variants of the theme `d` has: the built-in theme's own when it is
// one, else its colors on four backgrounds. And the one `d` shows now, or -1.
QVector<Variant> variantsOf(const Document &d);
int currentVariant(const Document &d);
// The first slide master's theme as a color set and font set.
ColorSet colorSetOf(const Document &d);
FontSet fontSetOf(const Document &d);

// Whether a presentation can give its theme: it needs a slide master, and that master needs layouts (a theme with none would leave the
// presentation's slides with nothing to stand on).
enum class ThemeSource { Fine, NoSlideMaster, NoLayouts };
ThemeSource themeSourceProblem(const Document &source);

// ---- changing a presentation (each is one step for the caller to record) ----

// Gives every slide the theme of `source`: its first slide master and that
// master's layouts replace the presentation's own, and each slide goes to the
// layout of its kind (title to title, two content to two content...), its text
// kept, and set in the new fonts and colors where it used the theme's. False,
// with nothing changed, if `source` can't give a theme (themeSourceProblem).
bool applyThemeToAll(Document &d, const Document &source);
// The same for the slides `slides` (indexes) only: they get a copy of the
// source's slide master of their own (the other slides keep theirs, and the
// presentation's theme stays the first master's).
bool applyThemeToSlides(Document &d, const Document &source, const QList<int> &slides);

// Gives a slide a layout, as applyLayout does and, when the layout stands on another slide master than the slide's own did, that slide master's
// theme too: its text that had the old theme's fonts and colors has the new one's, as a change of theme from the Design tab sets it.
void moveToLayout(Document &d, Page &slide, const MasterPage &layout);

// Each of these changes the presentation: every slide master's theme, and the
// slides, layouts, and masters standing on them.
// Colors: the old scheme's colors become the new scheme's wherever they are
// used, the background styles follow, and hyperlinks stay readable.
void applyColors(Document &d, const ColorSet &colors);
// Fonts: text set in the old heading and body fonts takes the new ones.
void applyFonts(Document &d, const FontSet &fonts);
// A background style for every slide (the slides' and layouts' own backgrounds
// give way); text turns light on a dark one and dark on a light one.
void applyBackgroundStyle(Document &d, int style);
// A variant's colors and its background.
void applyVariant(Document &d, const Variant &v);

// A presentation whose first slide master has a dark background has light text where none was chosen (new text boxes): the color it takes is
// the theme's light 1. Applying a theme does this itself; a presentation read from a file has the usual black until this is asked.
void keepTextReadable(Document &d);

// Makes the slide master the user's own: one that a change of theme made for some slides stays when no slide stands on it any more, and
// is no longer the theme it was made from (choosing that theme again for other slides makes a new one).
void keepSlideMaster(MasterPage &master);
// Whether a change of theme made the slide master for some slides, and nothing has made it the user's own since.
bool isThemeMade(const MasterPage &master);

// The theme part a slide master follows, as it is saved: the one kept from a file while its colors and fonts are still the master's, else one
// made from them (the format styles of the theme part made here are the ones the Quick Styles are drawn from).
QByteArray themePartOf(const Document &d, const MasterPage &master);
// The theme part a theme's colors and fonts make.
QByteArray themePart(const DeckTheme &t);

// WCAG 2 contrast ratio of two colors (1 to 21).
double contrastRatio(const QColor &a, const QColor &b);

} // namespace jp::deck
