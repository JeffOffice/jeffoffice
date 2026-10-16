#pragma once
// The table styles a presentation names by id (a:tableStyleId): the program has them built in, so a file
// need not define the ones it uses. These are this suite's own drawings of each family (Light, Medium,
// and Dark, plain or in one of the theme's six accent colors), written the way a file's tableStyles part
// writes a style (ECMA-376 part 1, 20.1.4.2.27), so that they are read by the same code and the colors
// come from the theme in use.

#include <QByteArray>
#include <QString>
#include <QVector>

namespace jp::office {

// A style in the Table Styles gallery.
struct TableStyleInfo {
    QString id;     // {GUID}, as a:tableStyleId has it
    QString name;   // in English: "Medium Style 2 - Accent 1"
    QString group;  // "Light", "Medium", or "Dark": the gallery's rows
    QString accent; // "", or "accent1" to "accent6" (the first, for a style of two)
};

// The gallery's styles in the order it lists them, then the two with no look at all ("No Style, No Grid" and
// "No Style, Table Grid", whose group is empty).
const QVector<TableStyleInfo> &tableStyleList();

// The style with that id as an a:tblStyleLst holding it: its regions (whole table, bands, first and last
// row and column, corners) with the fills, lines, and text it gives each. Empty for an id that is not one
// of the list's.
QByteArray tableStyleXml(const QString &id);

// The style a new table gets: "Medium Style 2 - Accent 1".
QString defaultTableStyleId();

} // namespace jp::office
