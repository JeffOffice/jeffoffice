#pragma once
// The slide show's settings and the custom slide shows in a presentation file
// (core/showsettings.h): the settings are p:showPr in ppt/presProps.xml
// (ECMA-376 Part 1, 19.2.1.22), the custom shows p:custShowLst in
// ppt/presentation.xml (19.2.1.5).
//
//   p:showPr    loop, showNarration, showAnimation, useTimings (booleans);
//               p:present, p:browse (showScrollbar), or p:kiosk (restart);
//               p:sldAll, p:sldRg (st and end, counting from 1), or
//               p:custShow (id); p:penClr; and an extension list, where the
//               laser pointer's color is (p14:laserClr).
//   p:custShow  name and id, and p:sldLst of p:sld, each with r:id, the
//               relationship the presentation has to the slide.
//
// What else the file's presentation properties hold (the HTML and print
// settings, the recent colors, and extensions JeffDeck has no form for) is
// kept in appData "presPropsKept" and written back as it was, around the
// show properties.

#include "core/document.h"
#include "office/theme.h"

#include <QDomElement>
#include <QHash>
#include <QString>
#include <QStringList>
#include <functional>

namespace jp::deck {

// A file may list this many custom shows, so many slides in each, and so many
// in all; more are left out. Real presentations have a few shows of a few
// dozen slides; these are 20 times as many shows and 100 times as many slides
// in all (a show can list the same slide again, so the slides in one are
// limited by the file's, not by the slides there are).
constexpr int kMaxCustomShows = 1000, kMaxCustomShowSlides = 10000, kMaxCustomShowEntries = 100000;
// The XML of the presentation properties kept to write back: a few hundred
// bytes in real files; this is 100 times that.
constexpr int kMaxKeptPropsBytes = 64 << 10;

// The custom shows of the presentation (p:presentation), into the document.
// `pageOfRel` says which page the slide a relationship id names became.
// Notes say what was left out.
void readCustomShows(const QDomElement &presentation, const QHash<QString, int> &pageOfRel, Document &doc, QStringList *notes);
// The presentation properties (p:presentationPr): the show settings go into
// the document, and the rest is kept. `standalone` makes an element into XML
// that reads by itself. Call after readCustomShows().
void readShowProperties(const QDomElement &presProps, const office::Theme &theme, const std::function<QString(const QDomElement &)> &standalone, Document &doc,
                        QStringList *notes);

// p:custShowLst, with the shows' slides named by the relationship id of each
// page's slide (`slideRels`); empty when there are no shows.
QString customShowListXml(const Document &doc, const QStringList &slideRels);
// The whole of presProps.xml's root element; `namespaces` are the xmlns attributes it carries.
QString presentationPropertiesXml(const Document &doc, const QString &namespaces);

// Counted since the program began on this thread, for tests of cost: the show properties read in full (the first only), and the slides that custom shows kept.
struct ShowImportWork {
    qint64 showPropertiesRead = 0, customShowEntriesKept = 0;
};
ShowImportWork showImportWorkForTest();

} // namespace jp::deck
