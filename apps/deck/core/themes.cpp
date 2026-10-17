#include "core/themes.h"

#include "core/color.h"
#include "core/fonts.h"
#include "core/themes_p.h"
#include "office/theme.h"
#include "office/xml.h"
#include "text/textprops.h"

#include <QCoreApplication>
#include <QRegularExpression>
#include <QSet>
#include <algorithm>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QDomDocument>
#include <cmath>

namespace jp::deck {

namespace {

// ---- colors ----

// A color rounded to the 8 bits a file keeps, so it compares equal after a save.
QColor snapped(const QColor &c) { return QColor(c.red(), c.green(), c.blue(), c.alpha()); }

double relativeLuminance(const QColor &c)
{
    auto channel = [](double v) { return v <= 0.03928 ? v / 12.92 : std::pow((v + 0.055) / 1.055, 2.4); };
    return 0.2126 * channel(c.redF()) + 0.7152 * channel(c.greenF()) + 0.0722 * channel(c.blueF());
}

// ---- the themes ----

// A color scheme: its name and the twelve colors as text.
struct Scheme {
    const char *name;
    const char *colors[12];
};

// A built-in theme's colors and fonts. Its four variants are the two schemes on the two backgrounds.
struct ThemeSpec {
    const char *name;
    const char *heading, *body;
    Scheme a, b;
    int style, otherStyle;   // the background style of the theme and the other it is shown on
};

// Colors in the order of a theme: dark 1, light 1, dark 2, light 2, accents 1-6, hyperlink, followed hyperlink.
// Dark 1 and dark 2 read on light 1 and light 2 (and the other way round) at 7 to 1 or better; every hyperlink color
// reads on the theme's own backgrounds, and applying another background makes it read again (keepLinksReadable).
const ThemeSpec kThemes[] = {
    // (Calibri, which is drawn in Carlito: the first theme is the one a new presentation starts from, and it is laid out as the file it is saved to.)
    {"JeffOffice", "Calibri", "Calibri",
     {"JeffOffice", {"#1F2937", "#FFFFFF", "#24364F", "#E8ECF2", "#1D55C4", "#B6420F", "#13773A", "#9E1F63", "#0A6C8C", "#5E2CA5", "#1D55C4", "#5E2CA5"}},
     {"Spice Market", {"#1E1B16", "#FFFFFF", "#3B2F2F", "#F3EDE4", "#C2410C", "#0F766E", "#4D7C0F", "#A21CAF", "#1D4ED8", "#B45309", "#0F5CC0", "#7E22CE"}}, 0, 6},
    {"Harbor", "Libre Baskerville", "Open Sans",
     {"Harbor", {"#1B2A3A", "#FFFFFF", "#102B47", "#EEF3F7", "#0E7C86", "#D9822B", "#2F6DB5", "#7A8B99", "#C0504D", "#5B8C5A", "#1A5FA8", "#6B4E9B"}},
     {"Harbor Sand", {"#2B2620", "#FFFFFF", "#30261D", "#F5EFE6", "#B5651D", "#4F7A5A", "#8C3B2E", "#C9A227", "#5E6B73", "#3F6C8F", "#1F5C99", "#6B4E7A"}}, 0, 7},
    {"Ember", "Oswald", "Lato",
     {"Ember", {"#1A1614", "#FFFFFF", "#2A2320", "#F4EEE8", "#F59E0B", "#EF4444", "#FB923C", "#FACC15", "#2DD4BF", "#A78BFA", "#FBBF24", "#C4B5FD"}},
     {"Ember Ice", {"#14181F", "#FFFFFF", "#1F2A3A", "#E9EEF5", "#38BDF8", "#F472B6", "#34D399", "#FDE047", "#FB923C", "#A78BFA", "#7DD3FC", "#D8B4FE"}}, 10, 0},
    {"Almanac", "Fraunces", "Source Serif 4",
     {"Almanac", {"#26302A", "#FBF7EE", "#1F4D3A", "#EAE3D2", "#A8431F", "#2F6B4F", "#C28A1E", "#5B7A8C", "#8A4B6A", "#6B8E3A", "#1F5C8F", "#7A4B8C"}},
     {"Almanac Plum", {"#2B2A33", "#FBF8F2", "#3B3558", "#E9E4EF", "#7A3E9D", "#C0562B", "#2F7F79", "#B8860B", "#4B6584", "#A23B5A", "#244F9A", "#7A3E9D"}}, 0, 7},
    {"Confetti", "Poppins", "Nunito",
     {"Confetti", {"#231F3A", "#FFFFFF", "#3A2A6B", "#F6F1FF", "#E5336D", "#F2711C", "#F5B800", "#12A594", "#4C6EF5", "#8E44AD", "#3347D6", "#8E44AD"}},
     {"Confetti Lagoon", {"#1C2B2B", "#FFFFFF", "#0F5F63", "#EAF6F4", "#0FA3A3", "#F25F5C", "#FFC857", "#7A5CFA", "#2F80ED", "#E5336D", "#1F5FC4", "#7A3FA0"}}, 0, 9},
    {"Granite", "IBM Plex Sans", "IBM Plex Sans",
     {"Granite", {"#12171D", "#FFFFFF", "#1E2730", "#E6EBF0", "#38BDF8", "#34D399", "#FBBF24", "#F472B6", "#A78BFA", "#FB7185", "#7DD3FC", "#C4B5FD"}},
     {"Granite Copper", {"#161616", "#FFFFFF", "#2B2B30", "#EDEDED", "#F97316", "#22D3EE", "#A3E635", "#E879F9", "#FACC15", "#60A5FA", "#FDBA74", "#E9D5FF"}}, 9, 0},
    {"Orchard", "Quicksand", "Nunito",
     {"Orchard", {"#1F2A1F", "#F6FAF3", "#2E5339", "#E3EFD9", "#3F8F4F", "#E0603A", "#F2B134", "#7A5C99", "#2F8FBF", "#C9477A", "#1E6FB8", "#7A4F9C"}},
     {"Orchard Plum", {"#2A2230", "#FBF7FA", "#5A2D4E", "#F1E5EE", "#B83B6C", "#2F8F83", "#E39B2D", "#5B6BC0", "#7BAA3A", "#8E5BB5", "#2B5FA8", "#7A3F9C"}}, 0, 8},
    {"Marquee", "Playfair Display", "Lato",
     {"Marquee", {"#0E0E10", "#FFFFFF", "#1A1A1F", "#E8E2D0", "#D8B04A", "#B84A62", "#4FA3A5", "#8E7CC3", "#6AA84F", "#E69138", "#E9C46A", "#C9A9E8"}},
     {"Marquee Sapphire", {"#0B1020", "#FFFFFF", "#16213A", "#E6E9F2", "#7FB7FF", "#FF8FA3", "#8AE1C2", "#FFD166", "#B8A1FF", "#FF9F68", "#9CC9FF", "#D4C4FF"}}, 9, 11},
    {"Tideline", "Raleway", "Lato",
     {"Tideline", {"#14293A", "#F4F8FB", "#1B4965", "#DDE9F1", "#1B7F99", "#5FA8D3", "#62B6CB", "#F4A259", "#E76F51", "#2A9D8F", "#145C9E", "#6A4C93"}},
     {"Tideline Dune", {"#2A2A2A", "#FAFAF7", "#3D405B", "#E9E6DD", "#E07A5F", "#81B29A", "#F2CC8F", "#3D5A80", "#98C1D9", "#B56576", "#2F5F9E", "#7A4B8C"}}, 1, 7},
};
constexpr int kThemeCount = int(sizeof(kThemes) / sizeof(kThemes[0]));

ColorSet colorSetOfScheme(const Scheme &s)
{
    ColorSet c;
    c.name = QString::fromLatin1(s.name);
    for (int i = 0; i < 12; ++i) c.colors[i] = QColor(QLatin1String(s.colors[i]));
    return c;
}

DeckTheme deckThemeOf(const ThemeSpec &spec, const Scheme &s)
{
    DeckTheme t;
    t.name = QString::fromLatin1(spec.name);
    for (int i = 0; i < 12; ++i) t.colors[i] = QColor(QLatin1String(s.colors[i]));
    t.headingFont = QString::fromLatin1(spec.heading);
    t.bodyFont = QString::fromLatin1(spec.body);
    return t;
}

// ---- what a theme draws and how it sets its text ----

// How the text of a placeholder is set.
struct Text {
    double size = 0;
    int color = 0;                              // a theme color
    Qt::Alignment align = Qt::AlignLeft;
    VAlign valign = VAlign::Top;
    bool bold = false, caps = false, heading = false;
};

// A shape a slide master or layout draws. Placed in points on a 960 by 540 slide.
struct Shape {
    const char *shape = "rect";
    QRectF r;
    int fill = -1;               // a theme color, or -1 for none
    double opacity = 1;
    int line = -1;               // a theme color for the outline, or -1 for none
    double lineWidth = 0;
    bool flipH = false, flipV = false;
    double corner = -1;          // a rounded rectangle's corner, as a share of its shorter side
};

struct Design {
    // The slide master and most layouts.
    QRectF title, body;
    Text titleText, bodyText;
    // Title Slide, and Section Header.
    QRectF ctrTitle, subTitle, secTitle, secBody;
    Text ctrText, subText, secTitleText, secBodyText;
    // The two layouts with a caption: the panel behind it and the colors of the title and the caption.
    Shape panel;
    int captionTitle = 1, captionText = 3;
    // The color of the bullets of the body text (a theme color).
    int bullet = 4;
    // The footers.
    int footer = 2;
    QRectF dt, ftr, num;         // empty to leave where they are
    // What is drawn.
    QVector<Shape> master, titleSlide, section;
    bool titleHidesMaster = true;
};

Shape sh(const char *shape, const QRectF &r, int fill, double opacity = 1)
{
    Shape s;
    s.shape = shape;
    s.r = r;
    s.fill = fill;
    s.opacity = opacity;
    return s;
}

Shape outline(const char *shape, const QRectF &r, int line, double width)
{
    Shape s;
    s.shape = shape;
    s.r = r;
    s.line = line;
    s.lineWidth = width;
    return s;
}

Shape turned(Shape s, bool flipH, bool flipV)
{
    s.flipH = flipH;
    s.flipV = flipV;
    return s;
}

Shape rounded(Shape s, double corner)
{
    s.corner = corner;
    return s;
}

Text txt(double size, int color, Qt::Alignment align, VAlign valign, bool heading, bool bold = false, bool caps = false)
{
    Text t;
    t.size = size;
    t.color = color;
    t.align = align;
    t.valign = valign;
    t.heading = heading;
    t.bold = bold;
    t.caps = caps;
    return t;
}

constexpr Qt::Alignment L = Qt::AlignLeft, C = Qt::AlignHCenter;

// Each theme's design. Text sits only on the background or on a panel in a dark or light color of the theme, so that it reads when
// the background turns dark or light (those four colors trade places then); the accents are for what carries no text.
Design designOf(int theme)
{
    Design g;
    switch (theme) {
    case 1: {   // Harbor: a navy band across the top, a teal rule under it
        g.title = QRectF(66, 16, 828, 76);
        g.body = QRectF(66, 134, 828, 350);
        g.titleText = txt(32, 1, L, VAlign::Middle, true);
        g.bodyText = txt(26, 0, L, VAlign::Top, false);
        g.ctrTitle = QRectF(66, 96, 828, 200);
        g.subTitle = QRectF(66, 366, 828, 100);
        g.ctrText = txt(50, 1, L, VAlign::Bottom, true);
        g.subText = txt(22, 2, L, VAlign::Top, false);
        g.secTitle = QRectF(66, 70, 828, 200);
        g.secBody = QRectF(66, 330, 828, 110);
        g.secTitleText = txt(44, 1, L, VAlign::Bottom, true);
        g.secBodyText = txt(22, 2, L, VAlign::Top, false);
        g.panel = sh("rect", QRectF(0, 0, 380, 540), 2);
        g.captionTitle = 1;
        g.captionText = 3;
        g.footer = 0;
        g.master = {sh("rect", QRectF(0, 0, 960, 104), 2), sh("rect", QRectF(0, 104, 960, 5), 4), sh("rect", QRectF(0, 104, 150, 5), 5)};
        g.titleSlide = {sh("rect", QRectF(0, 0, 960, 336), 2), sh("rect", QRectF(0, 336, 960, 6), 4), sh("rect", QRectF(66, 336, 120, 6), 5)};
        g.section = {sh("rect", QRectF(0, 0, 960, 300), 2), sh("rect", QRectF(0, 300, 960, 6), 4), sh("rect", QRectF(66, 300, 120, 6), 5)};
        break;
    }
    case 2: {   // Ember: a dark page with a bar of amber down its edge, bold capitals
        g.title = QRectF(66, 30, 828, 84);
        g.body = QRectF(66, 138, 828, 346);
        g.titleText = txt(40, 1, L, VAlign::Middle, true, false, true);
        g.bodyText = txt(26, 1, L, VAlign::Top, false);
        g.ctrTitle = QRectF(66, 120, 470, 240);
        g.subTitle = QRectF(66, 380, 470, 90);
        g.ctrText = txt(54, 1, L, VAlign::Bottom, true, false, true);
        g.subText = txt(22, 3, L, VAlign::Top, false);
        g.secTitle = QRectF(66, 120, 828, 220);
        g.secBody = QRectF(66, 372, 828, 100);
        g.secTitleText = txt(50, 1, L, VAlign::Bottom, true, false, true);
        g.secBodyText = txt(22, 3, L, VAlign::Top, false);
        g.panel = sh("rect", QRectF(0, 0, 380, 540), 2);
        g.captionTitle = 1;
        g.captionText = 3;
        g.footer = 1;
        const Shape bar = sh("rect", QRectF(0, 0, 18, 540), 4);
        g.master = {bar, sh("rect", QRectF(18, 0, 6, 540), 5), sh("rect", QRectF(66, 116, 120, 4), 4), sh("rect", QRectF(66, 494, 828, 1.5), 4, 0.35)};
        g.titleSlide = {sh("rect", QRectF(560, 0, 400, 540), 2), sh("ellipse", QRectF(640, 150, 240, 240), 4), sh("ellipse", QRectF(800, 330, 90, 90), 5), bar, sh("rect", QRectF(18, 0, 6, 540), 5)};
        g.section = {bar, sh("rect", QRectF(18, 0, 6, 540), 5), sh("rect", QRectF(66, 350, 160, 6), 4)};
        break;
    }
    case 3: {   // Almanac: warm paper, a hanging tab, and a double rule
        g.title = QRectF(108, 22, 786, 80);
        g.body = QRectF(66, 132, 828, 352);
        g.titleText = txt(36, 2, L, VAlign::Middle, true);
        g.bodyText = txt(26, 0, L, VAlign::Top, false);
        g.ctrTitle = QRectF(96, 110, 768, 200);
        g.subTitle = QRectF(96, 352, 768, 90);
        g.ctrText = txt(56, 2, C, VAlign::Bottom, true);
        g.subText = txt(24, 0, C, VAlign::Top, false);
        g.secTitle = QRectF(66, 150, 828, 190);
        g.secBody = QRectF(66, 366, 828, 100);
        g.secTitleText = txt(48, 2, L, VAlign::Bottom, true);
        g.secBodyText = txt(24, 0, L, VAlign::Top, false);
        g.panel = sh("rect", QRectF(0, 0, 380, 540), 3);
        g.captionTitle = 2;
        g.captionText = 0;
        g.footer = 0;
        g.master = {sh("rect", QRectF(66, 0, 28, 66), 4), sh("rect", QRectF(66, 106, 828, 2.5), 4), sh("rect", QRectF(66, 112, 828, 0.75), 4)};
        g.titleSlide = {outline("rect", QRectF(30, 26, 900, 462), 4, 1.5), outline("rect", QRectF(38, 34, 884, 446), 6, 0.75), sh("rect", QRectF(430, 330, 100, 2.5), 4)};
        g.section = {sh("rect", QRectF(66, 0, 28, 66), 4), sh("rect", QRectF(66, 350, 160, 2.5), 4)};
        break;
    }
    case 4: {   // Confetti: bright corner triangles and a stripe of color
        g.title = QRectF(66, 36, 828, 80);
        g.body = QRectF(66, 134, 828, 350);
        g.titleText = txt(36, 2, L, VAlign::Middle, true, true);
        g.bodyText = txt(26, 0, L, VAlign::Top, false);
        g.ctrTitle = QRectF(66, 150, 640, 190);
        g.subTitle = QRectF(66, 352, 640, 76);
        g.ctrText = txt(54, 2, L, VAlign::Bottom, true, true);
        g.subText = txt(24, 0, L, VAlign::Top, false);
        g.secTitle = QRectF(66, 130, 700, 200);
        g.secBody = QRectF(66, 352, 700, 76);
        g.secTitleText = txt(46, 2, L, VAlign::Bottom, true, true);
        g.secBodyText = txt(24, 0, L, VAlign::Top, false);
        g.panel = sh("rect", QRectF(0, 0, 380, 540), 3);
        g.captionTitle = 2;
        g.captionText = 0;
        g.footer = 0;
        g.dt = QRectF(190, 500.5, 120, 28.75);   // clear of the triangles in the corners
        g.num = QRectF(660, 500.5, 200, 28.75);
        auto corner = [](double size, int fill, bool right, bool bottom) {
            return turned(sh("rtTriangle", QRectF(right ? 960 - size : 0, bottom ? 540 - size : 0, size, size), fill), right, !bottom);
        };
        const Shape stripe[] = {sh("rect", QRectF(0, 0, 320, 8), 4), sh("rect", QRectF(320, 0, 320, 8), 5), sh("rect", QRectF(640, 0, 320, 8), 6)};
        g.master = {stripe[0], stripe[1], stripe[2], corner(100, 4, true, true), corner(68, 5, true, true), corner(38, 6, true, true)};
        g.titleSlide = {corner(280, 4, true, false), corner(190, 5, true, false), corner(100, 6, true, false), corner(170, 7, false, true), corner(110, 8, false, true), corner(56, 9, false, true)};
        g.section = {corner(170, 7, false, true), corner(110, 8, false, true), corner(56, 9, false, true), corner(240, 4, true, false), corner(160, 5, true, false), corner(84, 6, true, false)};
        break;
    }
    case 5: {   // Granite: a dark page, a slate band for the title, a rail of sky blue
        g.title = QRectF(66, 22, 828, 80);
        g.body = QRectF(66, 134, 828, 350);
        g.titleText = txt(34, 1, L, VAlign::Middle, true, true);
        g.bodyText = txt(24, 1, L, VAlign::Top, false);
        g.ctrTitle = QRectF(66, 120, 470, 220);
        g.subTitle = QRectF(66, 360, 470, 90);
        g.ctrText = txt(46, 1, L, VAlign::Bottom, true, true);
        g.subText = txt(22, 3, L, VAlign::Top, false);
        g.secTitle = QRectF(66, 150, 828, 190);
        g.secBody = QRectF(66, 368, 828, 100);
        g.secTitleText = txt(44, 1, L, VAlign::Bottom, true, true);
        g.secBodyText = txt(22, 3, L, VAlign::Top, false);
        g.panel = sh("rect", QRectF(0, 0, 380, 540), 2);
        g.captionTitle = 1;
        g.captionText = 3;
        g.footer = 1;
        const Shape rail = sh("rect", QRectF(0, 0, 10, 540), 4);
        g.master = {sh("rect", QRectF(0, 0, 960, 106), 2), rail, sh("rect", QRectF(66, 100, 96, 6), 4)};
        g.titleSlide = {sh("rect", QRectF(560, 0, 400, 540), 2), rail, sh("rect", QRectF(560, 0, 6, 540), 4), sh("rect", QRectF(640, 100, 48, 48), 4), sh("rect", QRectF(700, 160, 48, 48), 5),
                        sh("rect", QRectF(760, 220, 48, 48), 6), sh("rect", QRectF(820, 280, 48, 48), 7), sh("rect", QRectF(880, 340, 48, 48), 8)};
        g.section = {rail, sh("rect", QRectF(66, 350, 96, 6), 4)};
        break;
    }
    case 6: {   // Orchard: soft green, a rounded title panel, large soft circles
        g.title = QRectF(84, 34, 792, 72);
        g.body = QRectF(66, 138, 828, 346);
        g.titleText = txt(34, 2, L, VAlign::Middle, true, true);
        g.bodyText = txt(26, 0, L, VAlign::Top, false);
        g.ctrTitle = QRectF(66, 120, 470, 200);
        g.subTitle = QRectF(66, 340, 470, 90);
        g.ctrText = txt(52, 2, L, VAlign::Bottom, true, true);
        g.subText = txt(24, 0, L, VAlign::Top, false);
        g.secTitle = QRectF(84, 128, 792, 226);
        g.secBody = QRectF(66, 384, 828, 100);
        g.secTitleText = txt(44, 2, L, VAlign::Bottom, true, true);
        g.secBodyText = txt(24, 0, L, VAlign::Top, false);
        g.panel = sh("rect", QRectF(0, 0, 380, 540), 3);
        g.captionTitle = 2;
        g.captionText = 0;
        g.footer = 0;
        g.master = {rounded(sh("roundRect", QRectF(48, 26, 864, 88), 3), 0.5), sh("ellipse", QRectF(700, 330, 320, 320), 4, 0.08), sh("ellipse", QRectF(790, 420, 240, 240), 6, 0.10)};
        g.titleSlide = {sh("ellipse", QRectF(560, 40, 420, 420), 4, 0.9), sh("ellipse", QRectF(700, 200, 300, 300), 5, 0.85), sh("ellipse", QRectF(640, 360, 120, 120), 6, 0.9)};
        g.section = {rounded(sh("roundRect", QRectF(48, 116, 864, 258), 3), 0.18), sh("ellipse", QRectF(700, 360, 320, 320), 4, 0.08)};
        break;
    }
    case 7: {   // Marquee: black and gold, a thin double frame
        g.title = QRectF(66, 50, 828, 84);
        g.body = QRectF(66, 150, 828, 310);
        g.titleText = txt(40, 1, C, VAlign::Middle, true);
        g.bodyText = txt(26, 1, L, VAlign::Top, false);
        g.ctrTitle = QRectF(96, 90, 768, 190);
        g.subTitle = QRectF(96, 340, 768, 90);
        g.ctrText = txt(56, 1, C, VAlign::Bottom, true);
        g.subText = txt(22, 3, C, VAlign::Top, false);
        g.secTitle = QRectF(96, 120, 768, 200);
        g.secBody = QRectF(96, 352, 768, 90);
        g.secTitleText = txt(46, 1, C, VAlign::Bottom, true);
        g.secBodyText = txt(22, 3, C, VAlign::Top, false);
        g.panel = sh("rect", QRectF(0, 0, 380, 540), 2);
        g.captionTitle = 1;
        g.captionText = 3;
        g.footer = 1;
        const Shape frame = outline("rect", QRectF(24, 24, 912, 452), 4, 1.5), inner = outline("rect", QRectF(32, 32, 896, 436), 4, 0.5);
        g.master = {frame, inner, sh("rect", QRectF(430, 138, 100, 2), 4)};
        g.titleSlide = {frame, inner, turned(sh("diamond", QRectF(468, 296, 24, 24), 4), false, false), sh("rect", QRectF(330, 307, 120, 1.5), 4), sh("rect", QRectF(510, 307, 120, 1.5), 4)};
        g.section = {frame, inner, sh("rect", QRectF(430, 334, 100, 2), 4)};
        break;
    }
    case 8: {   // Tideline: pale blue-gray, bands of water along the top, and a horizon of bands on the title slide
        g.title = QRectF(66, 34, 828, 80);
        g.body = QRectF(66, 132, 828, 352);
        g.titleText = txt(36, 2, L, VAlign::Middle, true, true);
        g.bodyText = txt(26, 0, L, VAlign::Top, false);
        g.ctrTitle = QRectF(66, 100, 828, 200);
        g.subTitle = QRectF(66, 314, 828, 50);
        g.ctrText = txt(54, 2, L, VAlign::Bottom, true, true);
        g.subText = txt(24, 0, L, VAlign::Top, false);
        g.secTitle = QRectF(96, 150, 798, 190);
        g.secBody = QRectF(96, 356, 798, 100);
        g.secTitleText = txt(46, 2, L, VAlign::Bottom, true, true);
        g.secBodyText = txt(24, 0, L, VAlign::Top, false);
        g.panel = sh("rect", QRectF(0, 0, 380, 540), 3);
        g.captionTitle = 2;
        g.captionText = 0;
        g.footer = 0;
        g.master = {sh("rect", QRectF(0, 0, 960, 10), 4), sh("rect", QRectF(0, 10, 960, 5), 5), sh("rect", QRectF(0, 15, 960, 3), 6), sh("rect", QRectF(66, 120, 828, 1.5), 5, 0.6)};
        g.titleSlide = {sh("rect", QRectF(0, 380, 960, 116), 6, 0.26), sh("rect", QRectF(0, 420, 960, 76), 5, 0.36), sh("rect", QRectF(0, 460, 960, 36), 4, 0.5),
                        sh("rect", QRectF(0, 380, 960, 3), 4), sh("rect", QRectF(0, 420, 960, 2.5), 5), sh("rect", QRectF(0, 460, 960, 2), 6)};
        g.section = {sh("rect", QRectF(0, 0, 28, 540), 4), sh("rect", QRectF(28, 0, 14, 540), 5), sh("rect", QRectF(42, 0, 7, 540), 6)};
        break;
    }
    default: break;
    }
    return g;
}

// ---- building a theme's slide master and layouts out of the standard ones ----

QString plainHex(const QColor &c) { return ColorRef::rgb(c).toString(); }

// Sets the text of a placeholder in the look `x` describes, leaving the words.
void setLook(Document &d, TextItem &t, const Text &x, const DeckTheme &theme)
{
    t.valign = x.valign;
    QTextDocument *sd = d.storyDoc(t.storyId);
    if (!sd) return;
    QTextCharFormat cf;
    cf.setFontFamilies({x.heading ? theme.headingFont : theme.bodyFont});
    cf.setFontPointSize(x.size);
    cf.setFontWeight(x.bold ? QFont::Bold : QFont::Normal);
    cf.setFontCapitalization(x.caps ? QFont::AllUppercase : QFont::MixedCase);
    const QColor color = theme.colors[std::clamp(x.color, 0, 11)];
    cf.setForeground(color);
    cf.setProperty(tp::ColorRefP, plainHex(color));
    QTextBlockFormat bf;
    bf.setAlignment(x.align);
    QTextCursor c(sd);
    c.select(QTextCursor::Document);
    c.mergeCharFormat(cf);
    c.mergeBlockFormat(bf);
    QTextCursor start(sd);
    start.mergeBlockCharFormat(cf);
}

// The five sample paragraphs of a body place become the five levels of a list, as a body of the presentation program has them: each in a list of
// its own with a bullet in the theme color, further in and a little smaller than the one before; `x` is the first level's text.
void bulletize(Document &d, TextItem &t, const Text &x, const DeckTheme &theme, int slot)
{
    QTextDocument *sd = d.storyDoc(t.storyId);
    if (!sd) return;
    static const double smaller[5] = {0, 4, 6, 8, 8};
    static const char16_t marks[5] = {u'\u2022', u'\u2013', u'\u25AA', u'\u2013', u'\u2022'};
    const double base = x.size;
    int level = 0;
    for (QTextBlock b = sd->begin(); b.isValid() && level < 5; b = b.next(), ++level) {
        QTextCursor c(b);
        QTextBlockFormat bf = c.blockFormat();
        bf.setLeftMargin(base * 0.9 + level * base * 1.25);
        bf.setTextIndent(-base * 0.9);
        bf.setTopMargin(level ? base * 0.18 : base * 0.3);
        if (level) bf.setProperty(tp::ListLevel, level);
        else bf.clearProperty(tp::ListLevel);
        c.setBlockFormat(bf);
        QTextCharFormat cf;
        cf.setFontPointSize(std::max(14.0, base - smaller[level]));
        QTextCursor text(b);
        text.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        text.mergeCharFormat(cf);
        c.mergeBlockCharFormat(cf);
        QTextListFormat lf;
        lf.setStyle(QTextListFormat::ListDisc);
        lf.setIndent(0);
        lf.setProperty(tp::BulletChar, QString(QChar(marks[level])));
        lf.setProperty(tp::BulletFont, QStringLiteral("Arial"));
        lf.setProperty(tp::BulletColor, plainHex(theme.colors[std::clamp(slot, 0, 11)]));
        if (level) lf.setProperty(tp::ListLevel, level);
        c.createList(lf);
    }
}

struct Spot {
    QRectF rect;
    Text text;
    bool bullets = false;   // body text: a list of five levels
};

// Where a placeholder goes and how its text is set, by the kind of layout it is on ("master" for the slide master) and its role.
bool spotFor(const Design &g, const QString &kind, const QString &role, Spot *spot)
{
    const QRectF b = g.body;
    const double gap = 18, columnWidth = (b.width() - gap) / 2;
    const QRectF leftColumn(b.x(), b.y(), columnWidth, b.height()), rightColumn(b.x() + columnWidth + gap, b.y(), columnWidth, b.height());
    Text columns = g.bodyText;
    columns.size = std::max(18.0, g.bodyText.size - 2);
    Text head = g.bodyText;
    head.size = std::max(18.0, g.bodyText.size - 2);
    head.bold = true;
    head.valign = VAlign::Bottom;
    Text captionTitle = txt(30, g.captionTitle, L, VAlign::Bottom, true, g.titleText.bold);
    captionTitle.caps = g.titleText.caps;
    Text captionText = txt(18, g.captionText, L, VAlign::Top, false);   // large text: a link in it reads at 3 to 1
    Text captionContent = g.bodyText;
    captionContent.size = std::max(20.0, g.bodyText.size);
    auto set = [&](const QRectF &r, const Text &t, bool bullets = false) {
        spot->rect = r;
        spot->text = t;
        spot->bullets = bullets;
        return true;
    };
    // The date stands on the caption's panel on the two layouts that have one; the footer text keeps clear of it.
    const bool captioned = kind == QLatin1String("objTx") || kind == QLatin1String("picTx");
    if (role == QLatin1String("ph:dt")) return set(g.dt.isEmpty() ? QRectF(66, 500.5, 216, 28.75) : g.dt, txt(12, captioned ? g.captionText : g.footer, L, VAlign::Middle, false));
    if (role == QLatin1String("ph:ftr"))
        return set(captioned ? QRectF(400, 500.5, 260, 28.75) : g.ftr.isEmpty() ? QRectF(318, 500.5, 324, 28.75) : g.ftr, txt(12, g.footer, C, VAlign::Middle, false));
    if (role == QLatin1String("ph:sldNum")) return set(g.num.isEmpty() ? QRectF(678, 500.5, 216, 28.75) : g.num, txt(12, g.footer, Qt::AlignRight, VAlign::Middle, false));
    if (kind == QLatin1String("master")) {
        if (role == QLatin1String("ph:title")) return set(g.title, g.titleText);
        if (role == QLatin1String("ph:body:1")) return set(b, g.bodyText, true);
        return false;
    }
    if (kind == QLatin1String("title")) {
        if (role == QLatin1String("ph:ctrTitle")) return set(g.ctrTitle, g.ctrText);
        if (role == QLatin1String("ph:subTitle:1")) return set(g.subTitle, g.subText);
    } else if (kind == QLatin1String("secHead")) {
        if (role == QLatin1String("ph:title")) return set(g.secTitle, g.secTitleText);
        if (role == QLatin1String("ph:body:1")) return set(g.secBody, g.secBodyText);
    } else if (kind == QLatin1String("obj") || kind == QLatin1String("titleOnly")) {
        if (role == QLatin1String("ph:title")) return set(g.title, g.titleText);
        if (role == QLatin1String("ph:obj:1")) return set(b, g.bodyText, true);
    } else if (kind == QLatin1String("twoObj")) {
        if (role == QLatin1String("ph:title")) return set(g.title, g.titleText);
        if (role == QLatin1String("ph:obj:1")) return set(leftColumn, columns, true);
        if (role == QLatin1String("ph:obj:2")) return set(rightColumn, columns, true);
    } else if (kind == QLatin1String("twoTxTwoObj")) {
        const QRectF headLeft(leftColumn.x(), b.y(), columnWidth, 50), headRight(rightColumn.x(), b.y(), columnWidth, 50);
        const QRectF contentLeft(leftColumn.x(), b.y() + 54, columnWidth, b.height() - 54), contentRight(rightColumn.x(), b.y() + 54, columnWidth, b.height() - 54);
        if (role == QLatin1String("ph:title")) return set(g.title, g.titleText);
        if (role == QLatin1String("ph:body:1")) return set(headLeft, head);
        if (role == QLatin1String("ph:obj:2")) return set(contentLeft, columns, true);
        if (role == QLatin1String("ph:body:3")) return set(headRight, head);
        if (role == QLatin1String("ph:obj:4")) return set(contentRight, columns, true);
    } else if (kind == QLatin1String("objTx") || kind == QLatin1String("picTx")) {
        if (role == QLatin1String("ph:title")) return set(QRectF(66.1, 36, 309.6, 126), captionTitle);
        if (role == QLatin1String("ph:obj:1") || role == QLatin1String("ph:pic:1")) return set(QRectF(408.1, 77.8, 486, 383.8), captionContent, role == QLatin1String("ph:obj:1"));
        if (role == QLatin1String("ph:body:2")) return set(QRectF(66.1, 162, 309.6, 300.1), captionText);
    }
    return false;
}

ItemPtr shapeItem(const Shape &s, const DeckTheme &theme)
{
    auto it = std::make_shared<ShapeItem>();
    it->shape = QString::fromLatin1(s.shape);
    it->rect = s.r;
    it->name = QCoreApplication::translate("Deck", "Decoration");
    it->flipH = s.flipH;
    it->flipV = s.flipV;
    it->fill = s.fill >= 0 ? Fill::solid(ColorRef::rgb(theme.colors[s.fill]), 1 - s.opacity) : Fill();
    it->stroke = s.line >= 0 ? Stroke::line(ColorRef::rgb(theme.colors[s.line]), s.lineWidth) : Stroke::none();
    if (s.corner >= 0) it->adj = {s.corner};
    return it;
}

// Draws the shapes first on a page, under its placeholders.
void drawShapes(MasterPage &page, const QVector<Shape> &shapes, const DeckTheme &theme)
{
    ItemList drawn;
    for (const Shape &s : shapes) drawn.push_back(shapeItem(s, theme));
    page.items.insert(page.items.begin(), drawn.begin(), drawn.end());
}

// Gives the presentation (a new one: a slide master and the nine standard layouts) the design of a built-in theme.
void applyDesign(Document &d, const ThemeSpec &spec, const Design &g)
{
    const DeckTheme theme = deckThemeOf(spec, spec.a);
    setTheme(d, theme);
    d.fonts = {theme.name, theme.headingFont, theme.bodyFont};
    for (auto &m : d.masters) {
        const bool isMaster = isSlideMaster(*m);
        const QString kind = isMaster ? QStringLiteral("master") : layoutType(*m);
        if (isMaster) {
            m->background = backgroundStyle(theme, spec.style);
            m->name = QCoreApplication::translate("Deck", "%1 Slide Master").arg(theme.name);
        }
        for (const auto &it : m->items) {
            Spot spot;
            if (it->type() != ItemType::Text || !spotFor(g, kind, it->role, &spot)) continue;
            it->rect = spot.rect;
            setLook(d, *static_cast<TextItem *>(it.get()), spot.text, theme);
            if (spot.bullets) bulletize(d, *static_cast<TextItem *>(it.get()), spot.text, theme, g.bullet);
        }
        if (isMaster) {
            drawShapes(*m, g.master, theme);
        } else if (kind == QLatin1String("title")) {
            m->showMasterItems = !g.titleHidesMaster;
            drawShapes(*m, g.titleSlide, theme);
        } else if (kind == QLatin1String("secHead")) {
            m->showMasterItems = false;
            drawShapes(*m, g.section, theme);
        } else if (kind == QLatin1String("objTx") || kind == QLatin1String("picTx")) {
            m->showMasterItems = false;
            drawShapes(*m, {g.panel}, theme);
        }
    }
}

// The Title Slide made again from the layout, for a slide master that was restyled.
void remakeTitleSlide(Document &d)
{
    if (!d.pages.isEmpty()) {
        const Page &old = *d.pages[0];
        for (const auto &it : old.items)
            if (it->type() == ItemType::Text) d.removeStory(static_cast<TextItem *>(it.get())->storyId);
        d.removeStory(old.notesStoryId);
        d.pages.clear();
    }
    if (const MasterPage *title = layoutOfType(d, QStringLiteral("title"))) addSlide(d, *title);
}

} // namespace

// ---- contrast ----

double contrastRatio(const QColor &a, const QColor &b)
{
    double la = relativeLuminance(a), lb = relativeLuminance(b);
    if (la < lb) std::swap(la, lb);
    return (la + 0.05) / (lb + 0.05);
}

// ---- backgrounds ----

namespace detail {

QColor plainColor(const ColorRef &c)
{
    switch (c.kind()) {
    case ColorRef::Rgb: return c.rgbValue();
    case ColorRef::Scheme: return c.resolve(builtinColorSchemes().front());
    default: return QColor();
    }
}

QVector<QColor> backgroundColors(const Fill &f)
{
    QVector<QColor> out;
    if (f.type == Fill::Gradient) {
        if (f.stops.isEmpty()) {
            out << plainColor(f.color) << plainColor(f.color2);
        } else {
            for (const GradientStop &s : f.stops) out << plainColor(s.color);
        }
    } else if (f.type == Fill::Solid) {
        out << plainColor(f.color);
    }
    out.erase(std::remove_if(out.begin(), out.end(), [](const QColor &c) { return !c.isValid(); }), out.end());
    return out;
}

void keepLinksReadable(DeckTheme &t, const Fill &background)
{
    QVector<QColor> behind = backgroundColors(background);
    if (behind.isEmpty()) behind << QColor(Qt::white);
    double luminance = 0;
    for (const QColor &c : behind) luminance += relativeLuminance(c) / behind.size();
    const QColor toward = luminance < 0.18 ? QColor(Qt::white) : QColor(Qt::black);
    for (int slot : {10, 11}) {
        QColor c = t.colors[slot];
        auto reads = [&](const QColor &x) {
            for (const QColor &b : behind)
                if (contrastRatio(x, b) < 4.5) return false;
            return true;
        };
        for (int step = 0; step < 20 && !reads(c); ++step) c = snapped(mix(c, toward, 0.08));
        t.colors[slot] = c;
    }
}

} // namespace detail

Fill backgroundStyle(const DeckTheme &theme, int style)
{
    style = std::clamp(style, 0, kBackgroundStyles - 1);
    const QColor bases[4] = {theme.colors[1], theme.colors[3], theme.colors[2], theme.colors[0]};
    const QColor base = bases[style / 3], accent = theme.colors[4];
    const bool dark = backgroundStyleIsDark(style);
    switch (style % 3) {
    case 0: return Fill::solid(ColorRef::rgb(base));
    case 1: return Fill::gradient(ColorRef::rgb(base), ColorRef::rgb(snapped(mix(base, accent, dark ? 0.14 : 0.10))), 90);
    default: return Fill::gradient(ColorRef::rgb(snapped(mix(base, accent, dark ? 0.12 : 0.10))), ColorRef::rgb(snapped(mix(base, accent, dark ? 0.34 : 0.30))), 90);
    }
}

QString backgroundStyleName(int style) { return QCoreApplication::translate("Deck", "Style %1").arg(style + 1); }

bool backgroundStyleIsDark(int style) { return style >= 6; }

int backgroundStyleOf(const Fill &background, const DeckTheme &theme)
{
    const QVector<QColor> got = detail::backgroundColors(background);
    if (got.isEmpty() || (background.type != Fill::Solid && background.type != Fill::Gradient)) return -1;
    for (int s = 0; s < kBackgroundStyles; ++s) {
        const Fill want = backgroundStyle(theme, s);
        if (want.type != background.type) continue;
        const QVector<QColor> colors = detail::backgroundColors(want);
        if (got.first().rgb() == colors.first().rgb() && got.last().rgb() == colors.last().rgb()) return s;
    }
    return -1;
}

bool isDarkBackground(const Fill &background)
{
    const QVector<QColor> colors = detail::backgroundColors(background);
    if (colors.isEmpty()) return false;
    double luminance = 0;
    for (const QColor &c : colors) luminance += relativeLuminance(c) / colors.size();
    return luminance < 0.18;
}

// ---- the built-in themes ----

int builtinThemeCount() { return kThemeCount; }

QString builtinThemeName(int theme) { return theme >= 0 && theme < kThemeCount ? QString::fromLatin1(kThemes[theme].name) : QString(); }

int builtinThemeIndex(const QString &name)
{
    for (int i = 0; i < kThemeCount; ++i)
        if (name == QLatin1String(kThemes[i].name)) return i;
    return -1;
}

FontSet builtinThemeFonts(int theme)
{
    if (theme < 0 || theme >= kThemeCount) return FontSet();
    return {QString::fromLatin1(kThemes[theme].name), QString::fromLatin1(kThemes[theme].heading), QString::fromLatin1(kThemes[theme].body)};
}

QVector<Variant> builtinThemeVariants(int theme)
{
    if (theme < 0 || theme >= kThemeCount) return {};
    const ThemeSpec &t = kThemes[theme];
    return {{colorSetOfScheme(t.a), t.style}, {colorSetOfScheme(t.b), t.style}, {colorSetOfScheme(t.a), t.otherStyle}, {colorSetOfScheme(t.b), t.otherStyle}};
}

QVector<ColorSet> colorSets()
{
    QVector<ColorSet> out;
    for (const ThemeSpec &t : kThemes) out << colorSetOfScheme(t.a) << colorSetOfScheme(t.b);
    return out;
}

QVector<FontSet> fontSets()
{
    QVector<FontSet> out;
    QSet<QString> seen;
    for (int i = 0; i < kThemeCount; ++i) {
        out << builtinThemeFonts(i);
        seen.insert(out.last().heading + QLatin1Char('|') + out.last().body);
    }
    const QStringList available = bundledFamilies();
    for (const FontScheme &s : builtinFontSchemes()) {
        if (!available.contains(s.heading) || !available.contains(s.body)) continue;
        const QString pair = s.heading + QLatin1Char('|') + s.body;
        if (seen.contains(pair)) continue;
        seen.insert(pair);
        out << FontSet{s.name, s.heading, s.body};
    }
    return out;
}

std::unique_ptr<Document> themePresentation(int theme, int variant)
{
    theme = std::clamp(theme, 0, kThemeCount - 1);
    auto d = newPresentation();
    const ThemeSpec &spec = kThemes[theme];
    if (theme > 0) {
        applyDesign(*d, spec, designOf(theme));
        remakeTitleSlide(*d);
        d->appData[QStringLiteral("themeXml")] = QString::fromLatin1(detail::themePartXml(themeOf(*d)).toBase64());
    }
    detail::sayOnTitleSlide(*d, QString::fromLatin1(spec.name));
    detail::syncSchemeColors(*d);
    const QVector<Variant> variants = builtinThemeVariants(theme);
    if (variant > 0 && variant < variants.size()) applyVariant(*d, variants[variant]);
    detail::fitLinkColors(*d);
    return d;
}

// ---- theme parts ----

namespace detail {

// The words on the Title Slide of a theme's presentation.
void sayOnTitleSlide(Document &d, const QString &title)
{
    if (d.pages.isEmpty()) return;
    for (const auto &it : d.pages[0]->items) {
        if (it->type() != ItemType::Text) continue;
        const QString type = placeholderType(*it);
        const QString text = type == QLatin1String("ctrTitle") ? title : type == QLatin1String("subTitle") ? QCoreApplication::translate("Deck", "Subtitle") : QString();
        if (text.isEmpty()) continue;
        QTextCursor c(d.storyDoc(static_cast<TextItem *>(it.get())->storyId));
        c.insertText(text);
    }
}

QByteArray themePartXml(const DeckTheme &t)
{
    office::Theme theme;
    theme.name = theme.colorSchemeName = theme.fontSchemeName = t.name;
    for (int i = 0; i < office::Theme::ColorCount; ++i) theme.colors[i] = t.colors[i];
    theme.majorLatin = t.headingFont;
    theme.minorLatin = t.bodyFont;
    QString xml = QString::fromUtf8(office::writeTheme(theme));
    // The format styles: a flat fill, then two gradients, for shapes and for backgrounds, so the program's background styles differ.
    auto gradient = [](int tint, int shade) {
        return QStringLiteral("<a:gradFill rotWithShape=\"1\"><a:gsLst><a:gs pos=\"0\"><a:schemeClr val=\"phClr\"><a:tint val=\"%1\"/></a:schemeClr></a:gs>"
                              "<a:gs pos=\"100000\"><a:schemeClr val=\"phClr\"><a:shade val=\"%2\"/></a:schemeClr></a:gs></a:gsLst><a:lin ang=\"5400000\" scaled=\"0\"/></a:gradFill>")
            .arg(tint * 1000)
            .arg(shade * 1000);
    };
    const QString flat = QStringLiteral("<a:solidFill><a:schemeClr val=\"phClr\"/></a:solidFill>");
    for (const char *tag : {"fillStyleLst", "bgFillStyleLst"}) {
        const QRegularExpression list(QStringLiteral("<a:%1>.*?</a:%1>").arg(QLatin1String(tag)), QRegularExpression::DotMatchesEverythingOption);
        xml.replace(list, QStringLiteral("<a:%1>%2%3%4</a:%1>").arg(QLatin1String(tag), flat, gradient(94, 90), gradient(88, 74)));
    }
    // Effects: none, a soft shadow, and a deeper one (the shadows the Quick Styles of the second and third kind give).
    const QRegularExpression effects(QStringLiteral("<a:effectStyleLst>.*?</a:effectStyleLst>"), QRegularExpression::DotMatchesEverythingOption);
    auto shadow = [](int blur, int distance, int alpha) {
        return QStringLiteral("<a:effectStyle><a:effectLst><a:outerShdw blurRad=\"%1\" dist=\"%2\" dir=\"5400000\" algn=\"ctr\" rotWithShape=\"0\">"
                              "<a:srgbClr val=\"000000\"><a:alpha val=\"%3\"/></a:srgbClr></a:outerShdw></a:effectLst></a:effectStyle>")
            .arg(blur)
            .arg(distance)
            .arg(alpha * 1000);
    };
    xml.replace(effects, QStringLiteral("<a:effectStyleLst><a:effectStyle><a:effectLst/></a:effectStyle>%1%2</a:effectStyleLst>").arg(shadow(40000, 23000, 35), shadow(57150, 38100, 45)));
    return xml.toUtf8();
}

QByteArray patchThemePart(const QByteArray &xml, const DeckTheme &t)
{
    // A part that says it already is kept as it is, byte for byte.
    office::Theme read;
    if (office::readTheme(xml, &read)) {
        bool same = read.name == t.name && (read.majorLatin.isEmpty() || read.majorLatin == t.headingFont) && (read.minorLatin.isEmpty() || read.minorLatin == t.bodyFont);
        for (int i = 0; i < office::Theme::ColorCount && same; ++i) same = read.colors[i] == t.colors[i];
        if (same) return xml;
    }
    QDomDocument dom;
    if (!office::parseXml(xml, &dom)) return QByteArray();
    QDomElement root = dom.documentElement();
    if (!office::isElement(root, office::ns::A, QStringLiteral("theme"))) return QByteArray();
    QDomElement elements = office::child(root, office::ns::A, QStringLiteral("themeElements"));
    QDomElement colors = office::child(elements, office::ns::A, QStringLiteral("clrScheme"));
    if (colors.isNull()) return QByteArray();
    root.setAttribute(QStringLiteral("name"), t.name);
    colors.setAttribute(QStringLiteral("name"), t.name);
    for (int i = 0; i < office::Theme::ColorCount; ++i) {
        QDomElement slot = office::child(colors, office::ns::A, office::themeColorName(office::Theme::Color(i)));
        if (slot.isNull()) return QByteArray();
        while (!slot.firstChild().isNull()) slot.removeChild(slot.firstChild());
        QDomElement rgb = dom.createElementNS(office::ns::A, QStringLiteral("a:srgbClr"));
        rgb.setAttribute(QStringLiteral("val"), t.colors[i].name(QColor::HexRgb).mid(1).toUpper());
        slot.appendChild(rgb);
    }
    QDomElement fonts = office::child(elements, office::ns::A, QStringLiteral("fontScheme"));
    if (!fonts.isNull()) {
        fonts.setAttribute(QStringLiteral("name"), t.name);
        const struct { const char *kind; const QString &face; } faces[] = {{"majorFont", t.headingFont}, {"minorFont", t.bodyFont}};
        for (const auto &f : faces) {
            QDomElement group = office::child(fonts, office::ns::A, QLatin1String(f.kind));
            QDomElement latin = office::child(group, office::ns::A, QStringLiteral("latin"));
            if (!latin.isNull()) latin.setAttribute(QStringLiteral("typeface"), f.face);
        }
    }
    // The declaration as the package parts have it.
    QByteArray out = dom.toByteArray(-1);
    if (out.startsWith("<?xml")) out.remove(0, out.indexOf("?>") + 2);
    out.prepend("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n");
    return out;
}

} // namespace detail

QByteArray themePart(const DeckTheme &t) { return detail::themePartXml(t); }

QByteArray themePartOf(const Document &d, const MasterPage &master)
{
    const QByteArray kept = detail::masterThemeXml(d, master);
    return kept.isEmpty() ? detail::themePartXml(detail::masterTheme(d, master)) : kept;
}

} // namespace jp::deck
