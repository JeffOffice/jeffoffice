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
#include <functional>
#include <QString>
#include <QVector>

namespace jp::deck {

// The gray of a new presentation's footers (date, footer text, slide number): its dark 1 this many percent toward white; the gray of the text of a
// Section Header is `kSectionTint`. Files made before footers were darkened to read (4.5 to 1 on white) have them at 45 percent.
constexpr int kFooterTint = 30, kSectionTint = 40, kOldFooterTint = 45;

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
// The layouts, in the order the New Slide gallery lists them: of the slide
// master `masterId`, or of every master.
QVector<MasterPage *> layouts(const Document &d, const QString &masterId = QString());
MasterPage *layoutOfType(const Document &d, const QString &type, const QString &masterId = QString());
// How the layout galleries name a layout: its type, or, when an earlier layout of its slide master has the same type (custom layouts all
// have), "id:" and its id. And the layout a key names, among the layouts of `masterId` (or of every master).
QString layoutKey(const Document &d, const MasterPage &layout);
MasterPage *layoutByKey(const Document &d, const QString &key, const QString &masterId = QString());
// The slide master under slide `slide` (its layout's base), or "".
QString slideMasterOf(const Document &d, int slide);

// A new presentation: 16:9 slides, the default theme, a slide master with
// the nine standard layouts, and one Title Slide.
std::unique_ptr<Document> newPresentation();
// A new slide master with the nine standard layouts, in the presentation's theme and for its slide size, after the slide master
// `after` and its layouts (a layout names its slide master); the end when `after` names none. Their names are free ones.
MasterPage *addSlideMaster(Document &d, const QString &after = QString());
// A new layout, "Custom Layout", of the slide master `masterId` after the layout `after` (the master's last when none): a title and the
// date, footer, and slide number in the places the slide master has for them. Null if `masterId` is no slide master.
MasterPage *addLayout(Document &d, const QString &masterId, const QString &after = QString());
// A master's placeholder of that type ("title", "body", "dt", "ftr", or "sldNum") as a new presentation has it, for the slide size: null
// for any other type.
ItemPtr standardPlaceholder(Document &d, const QString &type);
// A copy of a layout's or a slide master's placeholder; with `empty`, its text is gone and its formatting kept for what is typed.
ItemPtr copyPlaceholder(Document &d, const Item &place, bool empty);

// A presentation saved with no slides (a template can be) gets one, as a
// new presentation starts: a Title Slide, or the first layout or master it
// has. False if it has neither.
bool ensureASlide(Document &d);

// Adds a slide with `layout` at `at` (-1: at the end): copies of the
// layout's title, body, and other content placeholders, empty, with their
// formatting and prompts (the date, footer, and slide number only when the
// slide shows them), and an empty notes page. Returns its index.
int addSlide(Document &d, const MasterPage &layout, int at = -1);
// A copy of slide `index` right after it, notes included: its objects keep
// their shape numbers, so its animations and charts stay theirs. Returns its index.
int duplicateSlide(Document &d, int index);
// Gives the slide another layout: its placeholders take the new layout's
// places and formatting by type (title to title, body to body...), the text
// staying; placeholders the new layout has no place for stay where they were.
void applyLayout(Document &d, Page &slide, const MasterPage &layout);
// What followLook set from the other look.
struct Following {
    bool color = false, family = false;
};
// Text that looked as one placeholder's text said looks as another's says: of the font, size, weight, slant, capitals, and color of `cf`, each
// that was as `was` has it (or as it has none) becomes as `now` has it (or none). `cf` says which of the colors and fonts it set.
Following followLook(QTextCharFormat &cf, const QTextCharFormat &was, const QTextCharFormat &now);

// The text of a slide's placeholder `t` takes the look of the layout `to` has for it where it had that of the layout `from` had (either may be
// null: then nothing is followed): paragraph by paragraph, by their levels, each one's list (bullet, font, color, size), indent, spacing, and
// alignment where they were the old paragraph's, and its text's font, size, weight, slant, capitals, and color that were. After each run
// (and each paragraph's end) has what it followed, `rest` is called with the run's format and what was followed, to change what wasn't; it says
// whether it changed anything.
void followLayout(Document &d, TextItem &t, const Item *from, const Item *to, const std::function<bool(QTextCharFormat &, const Following &)> &rest);
// A slide's placeholders that have nothing in them take the list of their layout's paragraph for their level, as a new slide's do (a file
// leaves an empty paragraph out of its list), so that what is typed in them has its bullets. Nothing else about them changes.
void inheritLayoutLists(Document &d);
// Whether the layout of the slide that `placeholder` is on has a list in the place it has for it: the levels of a slide's text then have bullets
// of their own to take (changeListLevel).
bool layoutHasList(const Document &d, const Item &placeholder);
// The paragraphs `first` to `last` (counted from 0) of the text of `placeholder`, a placeholder on a slide, go `dir` levels in (or out, when
// negative): each takes the layout's paragraph of its new level (the deepest the layout has, indented as far again for each level past it): its
// list (bullet character, font, color, and size), indent, and spacing, and its text that looked as the old level's does the same. Nothing
// changes, and it says so, when the layout has no list for the placeholder.
bool changeListLevel(Document &d, const Item &placeholder, int first, int last, int dir);
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
// The same for one slide, layout, or slide master that was made for slides of
// size `from` and now stands in a presentation of size `to` (a copy brought
// from another one): `slide` for a slide, whose other objects keep their
// shape, else everything stretches to the new shape. With `keepSquares`, what
// is a square on a master (a circle, a square, a corner triangle) is not
// stretched: it scales by the smaller ratio about the middle of where
// stretching puts it, and keeps to the edge of the slide it was on.
void fitPageToSize(Document &d, PageBase &page, const QSizeF &from, const QSizeF &to, bool slide, bool keepSquares = false);

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
