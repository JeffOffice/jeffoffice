#pragma once
// What a workbook's styles part says about how cells look: the number formats (the custom codes by id, and the format each cell format shows its value with),
// the fonts, fills, and borders, the cell formats that join them with an alignment, the palette of indexed colors, and the colors of the workbook's theme (a
// color is often named, not given: "theme 4, a little lighter", "indexed 10"). Reading never looks at a cell: a cell's `style` is the place of its cell format.

#include <QHash>
#include <QLocale>
#include <QString>
#include <QVector>

namespace jp::office::sheet {

// A color as the file names it. `rgb` is 0xRRGGBB (the alpha a file writes in front is dropped).
struct ColorSpec {
    enum class Kind : quint8 { None, Rgb, Theme, Indexed, Auto };
    Kind kind = Kind::None;
    quint32 rgb = 0;       // Rgb
    qint32 index = 0;      // Theme: 0 to 11 (light 1, dark 1, light 2, dark 2, the six accents, the two hyperlinks); Indexed: the palette's place
    double tint = 0;       // -1 (darker) to 1 (lighter), for a theme color
    bool isSet() const { return kind != Kind::None; }
};

enum class Underline : quint8 { None, Single, Double, SingleAccounting, DoubleAccounting };

struct FontSpec {
    QString name;
    double size = 11;                // points
    bool bold = false, italic = false, strike = false;
    Underline underline = Underline::None;
    quint8 vertAlign = 0;            // 0 baseline, 1 superscript, 2 subscript
    ColorSpec color;
};

enum class PatternKind : quint8 {
    None, Solid, MediumGray, DarkGray, LightGray, DarkHorizontal, DarkVertical, DarkDown, DarkUp, DarkGrid, DarkTrellis,
    LightHorizontal, LightVertical, LightDown, LightUp, LightGrid, LightTrellis, Gray125, Gray0625
};

struct FillSpec {
    PatternKind pattern = PatternKind::None;
    ColorSpec fg, bg;                // a solid fill shows fg; a pattern draws fg over bg
    bool gradient = false;           // a gradient is shown as a solid fill of its first stop (fg)
};

enum class BorderStyle : quint8 { None, Thin, Medium, Dashed, Dotted, Thick, Double, Hair, MediumDashed, DashDot, MediumDashDot, DashDotDot, MediumDashDotDot, SlantDashDot };

struct BorderSide {
    BorderStyle style = BorderStyle::None;
    ColorSpec color;
};

struct BorderSpec {
    BorderSide left, right, top, bottom, diagonal;
    bool diagonalUp = false, diagonalDown = false;
};

enum class HAlign : quint8 { General, Left, Center, Right, Fill, Justify, CenterContinuous, Distributed };
enum class VAlign : quint8 { Bottom, Center, Top, Justify, Distributed };

// A cell format (one `xf` of the cell formats): its number format, its font, fill, and border by place, and how the text sits in the cell.
struct CellFormat {
    qint32 numFmtId = 0;
    quint32 font = 0, fill = 0, border = 0;
    HAlign horizontal = HAlign::General;
    VAlign vertical = VAlign::Bottom;
    bool wrap = false, shrink = false;
    quint8 indent = 0;
    quint8 rotation = 0;             // 0 to 90 turns the text up, 91 to 180 down (that many less 90), 255 sets it upright one letter under another
};

// The colors of the theme in the order the styles name them by number: light 1, dark 1, light 2, dark 2, accent 1 to 6, hyperlink, followed hyperlink.
constexpr int kThemeColors = 12;

struct Styles {
    QHash<int, QString> numFmts;    // the formats the file defines, by id
    QVector<qint32> cellXfs;        // the format id of each cell format, in order
    qint64 chars = 0;               // the characters of all the format codes
    QVector<FontSpec> fonts;
    QVector<FillSpec> fills;
    QVector<BorderSpec> borders;
    QVector<CellFormat> formats;    // each cell format whole, in order (as cellXfs)
    QVector<quint32> indexed;       // the palette the file defines for indexed colors (0xRRGGBB); empty for the usual one
    quint32 theme[kThemeColors];    // the workbook's theme colors (0xRRGGBB), set when the workbook is opened; the usual ones until then

    Styles();

    // The id of the number format of a cell format (0, General, for one the file doesn't have).
    qint32 numFmtId(quint32 xf) const { return xf < quint32(cellXfs.size()) ? cellXfs[int(xf)] : 0; }
    // Its code: the file's own for an id it defines, else the built-in one of that id (calc/numfmt.h), else "General". The built-in dates 14 and 22 are the system's short date: they
    // are in the order of the locale given (the person's, by default).
    QString formatCode(quint32 xf, const QLocale &locale = QLocale()) const;

    // What is there for a cell format, a font, a fill, or a border by its place: the first of each (the default look) for a place the file doesn't have.
    const CellFormat &format(quint32 xf) const;
    const FontSpec &font(quint32 index) const;
    const FillSpec &fill(quint32 index) const;
    const BorderSpec &border(quint32 index) const;

    // The color as 0xRRGGBB: a theme color with its tint, an indexed color through the palette, an RGB as it is. `automatic` is what the "automatic" color (and
    // an index that stands for the system's) is, and what a color that is not set is: the caller's own (text is dark, a border dark, a fill none).
    quint32 resolve(const ColorSpec &color, quint32 automatic) const;
    // The usual palette's color of an indexed place (0 to 63); 0 for any other.
    static quint32 defaultIndexed(int index);
};

} // namespace jp::office::sheet
