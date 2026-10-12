#pragma once
// A presentation's sections: named runs of slides that the slide thumbnails
// and the slide sorter group under headings. They are the document's appData
// "sections", an array of {"id": "{GUID}", "name": "...", "count": n} in slide
// order, the counts adding up to the number of slides (a section may have
// none). A presentation without the key has no sections.
//
// Every change to the slide list goes through here too, so the counts stay
// right: call slideInserted(), slideRemoved(), or slidesReordered() right
// after the slides change. Whatever a caller misses is made good on reading:
// slides the counts don't reach join the last section, and counts past the
// last slide are cut off.

#include "core/document.h"

#include <QList>
#include <QString>
#include <QVector>

namespace jp::deck {

struct Section {
    QString id;      // "{GUID}"
    QString name;
    int count = 0;   // how many slides
};

// The sections, in slide order, their counts adding up to the slides; empty
// when the presentation has none.
QVector<Section> sectionsOf(const Document &d);
// Gives the presentation these sections (an empty list: none). The counts are
// made to add up to the slides.
void setSections(Document &d, const QVector<Section> &sections);
bool hasSections(const Document &d);
// "{3F2504E0-4F89-11D3-9A0C-0305E82C3301}", a new one each time.
QString newSectionId();
// What a section is called until it is renamed, and what the slides before the first section made are.
QString untitledSectionName();
QString defaultSectionName();

// The section that holds slide `slide`; -1 for none (or no sections).
int sectionOfSlide(const Document &d, int slide);
// The first slide of section `section`, and how many slides it has (0 for a
// section that has none; the first slide is then where it would start).
int firstSlideOf(const Document &d, int section);
int slideCountOf(const Document &d, int section);

// A new section starting at slide `before`, named "Untitled Section" when no
// name is given: it takes that slide and the ones after it up to the end of
// the section it was in. With no sections yet, the slides before it go in a
// "Default Section". Returns its index. A `before` past the last slide makes
// an empty section at the end.
int addSection(Document &d, int before, const QString &name = QString());
void renameSection(Document &d, int section, const QString &name);
// Takes the section away; its slides join the section before, or, for the
// first, the next. The only section's removal leaves none.
void removeSection(Document &d, int section);
void removeAllSections(Document &d);
// Takes the section and its slides away. False, and nothing changes, when
// that would leave the presentation without a slide.
bool removeSectionAndSlides(Document &d, int section);
// Moves the section, with its slides, up (-1) or down (1) past its neighbor.
// Returns its new index, or -1 when it can't move that way.
int moveSection(Document &d, int section, int by);

// The slide list changed: slide `at` was just inserted (it joins the section
// of the slide before it, the first section if there is none), slide `at`
// was just taken away (from the section that held it), or the slides were put
// in another order, as `order` says (the old index of each slide now, as a
// list that holds every slide once). Slides listed in `moved` (old indices)
// went to a new place and join the section there: `into`, when it is a
// section's index, or else the section of the slide before them (the first
// section for the first place). Where `into` can't be (not between the
// sections of the slides each side of the new place), the nearest that can.
void slideInserted(Document &d, int at);
void slideRemoved(Document &d, int at);
void slidesReordered(Document &d, const QList<int> &order, const QList<int> &moved, int into = -1);
// The order that moving the slides `moved` (indices, any order) to just
// before slide `before` (the slide count for the end) makes: for
// slidesReordered().
QList<int> orderAfterMoving(int count, const QList<int> &moved, int before);

} // namespace jp::deck
