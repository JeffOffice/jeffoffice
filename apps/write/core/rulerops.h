#pragma once
// What the ruler changes of a paragraph beyond its indents (editops.h's setIndents): its tab stops. Each is one step that undoes, on every paragraph the selection touches.

#include "core/writedoc.h"

#include <QTextBlockFormat>
#include <QTextCursor>
#include <QTextOption>

namespace jp::write {

// The most tab stops a paragraph keeps, and the farthest a tab stop is put (points from the left edge of the text).
constexpr int kMostTabStops = 64;
constexpr double kMostTabPosition = 1584;

struct TabStopEntry {
    double position = 0;
    QTextOption::TabType type = QTextOption::LeftTab;
    QChar leader = QLatin1Char(' ');   // ' ' for none, '.' dots, '-' dashes, '_' a line, U+00B7 middle dots
    bool operator==(const TabStopEntry &o) const { return position == o.position && type == o.type && leader == o.leader; }
};

// The tab stops of a paragraph's format, in order of position (the leaders of its format matched to them).
QVector<TabStopEntry> tabStopsOf(const QTextBlockFormat &format);
// The same set into a format: kept in order, at most kMostTabStops, each from 0 to kMostTabPosition, two at one place made one (the later wins).
void setTabStopsOf(QTextBlockFormat &format, QVector<TabStopEntry> stops);
// Gives every paragraph the selection touches these tab stops.
void setTabStops(WriteDoc &doc, QTextCursor &c, const QVector<TabStopEntry> &stops);
// A tab stop added to the paragraphs, moved, or taken away: `from` and `to` are positions in points (a stop within half a point of `from` is the one meant).
bool addTabStop(WriteDoc &doc, QTextCursor &c, const TabStopEntry &stop);
bool moveTabStop(WriteDoc &doc, QTextCursor &c, double from, double to);
bool removeTabStop(WriteDoc &doc, QTextCursor &c, double at);

} // namespace jp::write
