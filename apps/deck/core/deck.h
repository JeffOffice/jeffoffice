#pragma once
// Presentations on the shared page model (libs/page). Each slide is a Page,
// each slide layout a MasterPage whose base is its slide master, and each
// placeholder a text box whose role is "ph:<type>" or "ph:<type>:<index>"
// with the type named as presentation files name it ("title", "ctrTitle",
// "subTitle", "body", "obj", "pic", "dt", "ftr", "sldNum"). A layout's and a
// slide master's placeholders are prompts that new slides copy; they never
// show on the slides themselves. The masters say which they are in their
// props ("kind": "master" or "layout", a layout's "type" as files name it:
// "title", "obj", "secHead", "twoObj", "twoTxTwoObj", "titleOnly", "blank",
// "objTx", "picTx"). The theme is the document's appData "theme".

#include "core/document.h"

#include <QColor>
#include <QString>
#include <QVector>

namespace jp::deck {

// 16:9 slides, 13.333 by 7.5 inches.
constexpr double kSlideWidth = 960, kSlideHeight = 540;
// 4:3 slides, 10 by 7.5 inches.
constexpr double kStandardWidth = 720, kStandardHeight = 540;

// The colors and fonts a presentation's slides draw with.
struct DeckTheme {
    QString name;
    // dark 1, light 1, dark 2, light 2, accents 1-6, hyperlink, followed
    QColor colors[12];
    QString headingFont, bodyFont;
    QJsonObject toJson() const;
    static DeckTheme fromJson(const QJsonObject &o);
};
DeckTheme defaultTheme();
DeckTheme themeOf(const Document &d);
void setTheme(Document &d, const DeckTheme &t);

// "ph:title" -> "title"; "" when the item isn't a placeholder.
QString placeholderType(const Item &it);
QString placeholderRole(const QString &type, int index = 0);
// What an empty placeholder of that type says on screen.
QString promptFor(const QString &type);

bool isSlideMaster(const MasterPage &m);
bool isLayout(const MasterPage &m);
QString layoutType(const MasterPage &m);
// The layouts, in the order the New Slide gallery lists them.
QVector<MasterPage *> layouts(const Document &d);
MasterPage *layoutOfType(const Document &d, const QString &type);

// A new presentation: 16:9 slides, the default theme, a slide master with
// the nine standard layouts, and one Title Slide.
std::unique_ptr<Document> newPresentation();

// Adds a slide with `layout` at `at` (-1: at the end): copies of the
// layout's title, body, and other content placeholders, empty, with their
// formatting and prompts (the date, footer, and slide number only when the
// slide shows them), and an empty notes page. Returns its index.
int addSlide(Document &d, const MasterPage &layout, int at = -1);
// A copy of slide `index` right after it, notes included. Returns its index.
int duplicateSlide(Document &d, int index);
// Gives the slide another layout: its placeholders take the new layout's
// places and formatting by type (title to title, body to body...), the text
// staying; placeholders the new layout has no place for stay where they were.
void applyLayout(Document &d, Page &slide, const MasterPage &layout);
// Puts the slide's placeholders back where its layout has them.
void resetSlide(Document &d, Page &slide);

QString notesText(const Document &d, const Page &slide);

// The name a slide size goes by in setup.sizeName: "Standard" (4:3),
// "Widescreen" (16:9), or "Custom".
QString slideSizeName(const QSizeF &size);
// Gives every slide, layout, and the slide master another size, as Ensure
// Fit does: text sizes and outline weights scale by the smaller of the
// width's and the height's ratio; the layouts, the master,
// and the placeholders on slides stretch to the new shape; the other objects
// on slides keep their shape, scaled by that ratio and centered. Ruler guides
// keep their share of the slide. Notes keep their text sizes.
void setSlideSize(Document &d, const QSizeF &size);

// What Header & Footer shows on slides: the date (as the field that keeps
// it current), the slide number, and a line of footer text, each in the
// placeholder its layout has for it.
struct FooterOptions {
    bool date = false, slideNumber = false, footer = false;
    QString footerText;
    bool skipTitleSlide = false;   // not on slides whose layout is Title Slide
    bool any() const { return date || slideNumber || footer; }
    // Whether a slide with this layout type shows the placeholder of that type ("dt", "ftr", "sldNum").
    bool shows(const QString &placeholderType, const QString &layoutType) const;
    QJsonObject toJson() const;
    static FooterOptions fromJson(const QJsonObject &o);
};
// What the presentation says: its appData "footerOptions", or, when only
// appData "footers" is on, all three.
FooterOptions footerOptions(const Document &d);
// Gives `only`, or every slide (and new ones, through appData), the
// placeholders the options want, with the date, number, and text in them,
// and takes away the ones they don't.
void applyFooters(Document &d, const FooterOptions &o, Page *only = nullptr);

// A solid background for one slide; for all of them, the slide master's,
// with the slides' and layouts' own cleared so they follow it.
void setSlideBackground(Page &slide, const QColor &color);
void setAllBackgrounds(Document &d, const QColor &color);

} // namespace jp::deck
