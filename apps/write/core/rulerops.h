#pragma once
// What the ruler changes of a paragraph beyond its indents (editops.h's setIndents): its tab stops. Each is one step that undoes, on every paragraph the selection touches.

#include "core/writedoc.h"

#include <QTextBlockFormat>
#include <QTextCursor>
#include <QTextOption>

namespace jp::write {

// The most tab stops a paragraph keeps, and the farthest a tab stop is put (points from the left edge of the text).
constexpr int kMostTabStops = 64;       // a paragraph is given no more than this many
constexpr int kMostTabStopsShown = 4000; // the ruler draws no more than this many (a paragraph keeps all it has)
constexpr double kMostTabPosition = 1584;
constexpr int kMostBarStops = 64;       // and no more bar stops (lines the layout draws; each file format is written with no more)

struct TabStopEntry {
    double position = 0;
    QTextOption::TabType type = QTextOption::LeftTab;
    QChar leader = QLatin1Char(' ');   // ' ' for none, '.' dots, '-' dashes, '_' a line, U+00B7 middle dots
    bool bar = false;                  // a bar: a vertical line at the place on each line of the paragraph, taking no room (its type and leader mean nothing)
    bool operator==(const TabStopEntry &o) const { return position == o.position && type == o.type && leader == o.leader && bar == o.bar; }
};

// The tab stops of a paragraph's format, in order of position (the leaders of its format matched to them).
QVector<TabStopEntry> tabStopsOf(const QTextBlockFormat &format);
// The same set into a format: kept in order (all of them: a paragraph that had more than kMostTabStops keeps them; adding is what is refused), each from 0 to kMostTabPosition, two at one place made one (the later wins).
void setTabStopsOf(QTextBlockFormat &format, QVector<TabStopEntry> stops);
// Gives every paragraph the selection touches these tab stops.
void setTabStops(WriteDoc &doc, QTextCursor &c, const QVector<TabStopEntry> &stops);
// A tab stop added to the paragraphs, moved, or taken away: `from` and `to` are positions in points (a stop within half a point of `from` is the one meant).
bool addTabStop(WriteDoc &doc, QTextCursor &c, const TabStopEntry &stop);
bool moveTabStop(WriteDoc &doc, QTextCursor &c, double from, double to);
bool removeTabStop(WriteDoc &doc, QTextCursor &c, double at);

} // namespace jp::write
