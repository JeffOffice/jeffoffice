#pragma once
// How a cell looks, worked out once per cell format from the styles part: its font, text color, fill, borders, how its text sits, and the number format that turns its value into
// text, with every color resolved to RGB (a theme color with its tint, an indexed one through the palette). Cells name their format by place, so a sheet of a million cells with
// ten looks has ten of these, made as they are first asked for and let go of when too many are held.

#include "calc/numfmt.h"
#include "sheet/styles.h"

#include <QColor>
#include <QString>

#include <memory>
#include <string>
#include <unordered_map>

namespace jp::grid {

using jp::office::sheet::BorderStyle;
using jp::office::sheet::HAlign;
using jp::office::sheet::PatternKind;
using jp::office::sheet::Underline;
using jp::office::sheet::VAlign;

struct LineLook {
    BorderStyle style = BorderStyle::None;
    QRgb color = 0xFF000000;
};

struct CellLook {
    // the text
    QString family;
    double pointSize = 11;
    bool bold = false, italic = false, strike = false;
    Underline underline = Underline::None;
    quint8 vertAlign = 0;     // 0 baseline, 1 superscript, 2 subscript
    QRgb textColor = 0xFF000000;
    quint32 fontId = 0;       // a number for this font (family, size, and style), the same for every look that has it: what the drawing keys its fonts by
    // the fill: none, a solid color (fg), or a pattern of fg over bg
    PatternKind pattern = PatternKind::None;
    QRgb fg = 0, bg = 0;
    // the borders
    LineLook left, right, top, bottom, diagonal;
    bool diagonalUp = false, diagonalDown = false;
    // how the text sits
    HAlign horizontal = HAlign::General;
    VAlign vertical = VAlign::Bottom;
    bool wrap = false, shrink = false;
    int indent = 0;
    int rotation = 0;         // 0 to 90 up, 91 to 180 down (that less 90), 255 upright, one letter under another
    // the value's number format
    jp::calc::NumberFormat format;
    bool hasFormat = false;   // some format other than General
    bool hasFill() const { return pattern != PatternKind::None; }
    bool hasBorder() const
    {
        return left.style != BorderStyle::None || right.style != BorderStyle::None || top.style != BorderStyle::None || bottom.style != BorderStyle::None ||
               (diagonal.style != BorderStyle::None && (diagonalUp || diagonalDown));
    }
};

class LookTable {
public:
    // The most looks held when a screen begins; more than that are let go of then (a file may have a million cell formats, and a screen shows far fewer).
    static constexpr size_t kMaxHeld = 4096;

    explicit LookTable(const jp::office::sheet::Styles *styles = nullptr);
    void setStyles(const jp::office::sheet::Styles *styles);
    // The look of a cell format (the plain look for one the file doesn't have), made when first asked for. The reference holds until trim().
    const CellLook &look(quint32 xf) const;
    // Lets every look go if more than kMaxHeld are held; called where no reference is held (the start of drawing a screen).
    void trim() const;
    size_t held() const { return m_looks.size(); }
    // The width in pixels (at 100%) of the widest digit of the workbook's default font: what a column's width in characters is counted in.
    int maxDigitWidth() const;
    // Looks made so far (a test counts them).
    qint64 made() const { return m_made; }

private:
    CellLook make(quint32 xf) const;
    quint32 fontIdOf(const CellLook &look) const;
    const jp::office::sheet::Styles *m_styles = nullptr;
    mutable std::unordered_map<quint32, std::unique_ptr<CellLook>> m_looks;
    mutable qint64 m_made = 0;
    mutable std::unordered_map<std::string, quint32> m_fontIds;   // by the font's signature
};

} // namespace jp::grid
