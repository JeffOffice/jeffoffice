#pragma once
// The ready-made data bars, color scales, and icon sets of the Conditional Formatting menu, and the sentence that says what a rule does (the lists of Manage Rules).

#include "sheet/sheetmodel.h"

#include <QPixmap>
#include <QString>
#include <QStringList>
#include <QVector>

namespace jp::condpresets {

// The data bar colors the menu offers, each as a gradient and as a solid fill.
struct BarColor {
    const char *name;
    quint32 rgb;
};
const QVector<BarColor> &barColors();
// The color scales the menu offers: two or three colors, lowest first, and the name ("Green - Yellow - Red": highest first).
struct ScalePreset {
    const char *name;
    QVector<quint32> colors;
};
const QVector<ScalePreset> &scalePresets();
// The icon sets by group (the names the file has), and the words each is listed with.
struct IconGroup {
    const char *name;
    QStringList sets;
};
const QVector<IconGroup> &iconGroups();
QString iconSetLabel(const QString &set);

// The rules the menu makes for `range`.
jp::office::sheet::CondRule dataBarRule(const jp::office::sheet::CellRange &range, int color, bool gradient);
jp::office::sheet::CondRule colorScaleRule(const jp::office::sheet::CellRange &range, int preset);
// The icons change at even steps of the percent of the range (33 and 67 for three icons, and so on).
jp::office::sheet::CondRule iconSetRule(const jp::office::sheet::CellRange &range, const QString &set);

// The dates a rule can name, as the file writes them, and as they are listed.
struct Period {
    const char *name;
    const char *text;
};
const QVector<Period> &periods();

// What a rule does, in a sentence for a list ("Cell value > 5", "Data bar", "Icon set 3 Arrows").
QString describe(const jp::office::sheet::CondRule &rule);

// Small pictures for the menus: a data bar, a color scale, and a strip of the icons of a set.
QPixmap barPixmap(quint32 rgb, bool gradient, int size);
QPixmap scalePixmap(const QVector<quint32> &colors, int size);
QPixmap iconSetStrip(const QString &set, int size);

} // namespace jp::condpresets

namespace jp::condpresets {

// The ready-made looks of a rule that changes how cells look: a font color and a fill.
int lookCount();
QString lookName(int preset);
jp::office::sheet::DifferentialFormat lookOf(int preset);

} // namespace jp::condpresets
