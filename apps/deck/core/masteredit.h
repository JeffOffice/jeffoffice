#pragma once
// Editing slide masters and layouts (Slide Master view): adding and taking away masters and layouts, naming them, the placeholders a
// layout and a master have, and carrying an edit of a master or layout to what stands on it. A slide's placeholder holds its own copy of
// what it takes from its layout and master (a file's slides have it explicit), so an edit reaches the slides the way a change of theme
// does: text that looked as the layout said it did looks as the layout says now, and a placeholder that stood where the layout had it
// stands where the layout has it now. What a slide changed by hand stays.

#include "core/deck.h"

#include <QString>

namespace jp::deck {

// What Insert Placeholder puts on a layout, and the type each is as files name it.
enum class PlaceholderKind { Content, Text, Picture, Chart, Table, Media, OnlineImage };
QString placeholderTypeOf(PlaceholderKind kind);
// What a placeholder of that type is called on a layout that shows it with nothing in it ("Chart"), or empty for the ones that show their
// own sample text.
QString placeholderLabel(const QString &type);

// The slides that stand on a layout, or, for a slide master, on its layouts (or on it).
int slidesUsing(const Document &d, const MasterPage &page);
// Why the master page can't be deleted, or empty when it can: a master or layout that slides stand on stays, the last slide master
// stays, and so does the last layout of a slide master.
QString whyCantDelete(const Document &d, const MasterPage &page);
// Takes the slide master or layout away (a slide master with its layouts), with the text it holds. False, with the reason in `why`, if it can't be
// (whyCantDelete).
bool deleteMasterPage(Document &d, const QString &id, QString *why = nullptr);

// The name a slide master or layout goes by (empty names are not kept).
void renameMasterPage(Document &d, const QString &id, const QString &name);

// Preserve: a slide master or layout that stays in the file whether or not a slide stands on it. A page made or read as preserved is; a slide
// master a change of theme made for some slides is not, until it is.
bool isPreserved(const MasterPage &page);
void setPreserved(MasterPage &page, bool on);

// A new placeholder of that kind on the layout, below the title: its text styles are the body's of the slide master. It has the next number
// the layout's placeholders have. Returns it, or null if `layout` is no layout.
Item *insertPlaceholder(Document &d, MasterPage &layout, PlaceholderKind kind);

// The Title and Footers boxes of a layout: whether it has a title, and the date, footer, and slide number; and giving it or taking them.
// Given back, each is as the slide master has it.
bool layoutHasTitle(const MasterPage &layout);
bool layoutHasFooters(const MasterPage &layout);
void setLayoutTitle(Document &d, MasterPage &layout, bool on);
void setLayoutFooters(Document &d, MasterPage &layout, bool on);

// Master Layout: the places a slide master has.
struct MasterParts {
    bool title = false, text = false, date = false, number = false, footer = false;
    bool operator==(const MasterParts &o) const { return title == o.title && text == o.text && date == o.date && number == o.number && footer == o.footer; }
};
MasterParts masterParts(const MasterPage &master);
// Gives the slide master the places that are asked for, as a new presentation has them, and takes away the others. The layouts keep theirs.
void setMasterParts(Document &d, MasterPage &master, const MasterParts &parts);

// Carries what changed in the slide masters and layouts of `now` since `before` (the same presentation, earlier) to what stands on them,
// in `now`: a slide master's change to its layouts, and a layout's change (with the slide master's) to its slides. Changes of anything but
// the masters and layouts are not looked at. A master edited this way is kept (a change of theme won't take it away).
void carryMasterEdits(Document &now, const Document &before);

} // namespace jp::deck
