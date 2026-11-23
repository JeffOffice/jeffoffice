#pragma once
// The colors a text highlight is picked from, with the names a file uses (every app that marks text with a highlighter offers these).

#include <QColor>
#include <QString>
#include <QStringList>

namespace jp::tp {

struct HighlightColor {
    const char *name;
    unsigned rgb;
};

inline const HighlightColor *highlightColors(int *count)
{
    static const HighlightColor all[] = {{"yellow", 0xFFFF00},     {"green", 0x00FF00},    {"cyan", 0x00FFFF},       {"magenta", 0xFF00FF},   {"blue", 0x0000FF},
                                         {"red", 0xFF0000},        {"darkBlue", 0x000080}, {"darkCyan", 0x008080},   {"darkGreen", 0x008000}, {"darkMagenta", 0x800080},
                                         {"darkRed", 0x800000},    {"darkYellow", 0x808000}, {"darkGray", 0x808080}, {"lightGray", 0xC0C0C0}, {"black", 0x000000},
                                         {"white", 0xFFFFFF}};
    *count = int(sizeof all / sizeof *all);
    return all;
}

}   // namespace jp::tp
