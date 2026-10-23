#include "sheet/styles.h"

#include "calc/numfmt.h"

#include <algorithm>
#include <cmath>

namespace jp::office::sheet {

// The theme a workbook has when it names none: the colors people expect of a new sheet.
Styles::Styles()
{
    static const quint32 usual[kThemeColors] = {0xFFFFFF, 0x000000, 0xE7E6E6, 0x44546A, 0x4472C4, 0xED7D31, 0xA5A5A5, 0xFFC000, 0x5B9BD5, 0x70AD47, 0x0563C1, 0x954F72};
    for (int i = 0; i < kThemeColors; ++i) theme[i] = usual[i];
}

QString Styles::formatCode(quint32 xf) const
{
    const qint32 id = numFmtId(xf);
    const auto it = numFmts.constFind(id);
    if (it != numFmts.constEnd()) return it.value();
    const QString builtin = jp::calc::builtinFormatCode(id);
    return builtin.isEmpty() ? QStringLiteral("General") : builtin;
}

const CellFormat &Styles::format(quint32 xf) const
{
    static const CellFormat none;
    return xf < quint32(formats.size()) ? formats[int(xf)] : none;
}

const FontSpec &Styles::font(quint32 index) const
{
    static const FontSpec none;
    return index < quint32(fonts.size()) ? fonts[int(index)] : none;
}

const FillSpec &Styles::fill(quint32 index) const
{
    static const FillSpec none;
    return index < quint32(fills.size()) ? fills[int(index)] : none;
}

const BorderSpec &Styles::border(quint32 index) const
{
    static const BorderSpec none;
    return index < quint32(borders.size()) ? borders[int(index)] : none;
}

// A color lightened (tint above 0) or darkened (below 0): its lightness (in the hue, saturation, lightness model) is moved that part of the way to white or to black.
static quint32 tinted(quint32 rgb, double tint)
{
    if (tint == 0) return rgb;
    const double r = double((rgb >> 16) & 0xFF) / 255, g = double((rgb >> 8) & 0xFF) / 255, b = double(rgb & 0xFF) / 255;
    const double high = std::max({r, g, b}), low = std::min({r, g, b});
    double h = 0, s = 0, l = (high + low) / 2;
    if (high != low) {
        const double d = high - low;
        s = l > 0.5 ? d / (2 - high - low) : d / (high + low);
        if (high == r) h = (g - b) / d + (g < b ? 6 : 0);
        else if (high == g) h = (b - r) / d + 2;
        else h = (r - g) / d + 4;
        h /= 6;
    }
    l = tint < 0 ? l * (1 + tint) : l * (1 - tint) + tint;
    auto channel = [&](double t, double p, double q) {
        if (t < 0) t += 1;
        if (t > 1) t -= 1;
        if (t < 1.0 / 6) return p + (q - p) * 6 * t;
        if (t < 0.5) return q;
        if (t < 2.0 / 3) return p + (q - p) * (2.0 / 3 - t) * 6;
        return p;
    };
    double out[3] = {l, l, l};
    if (s != 0) {
        const double q = l < 0.5 ? l * (1 + s) : l + s - l * s, p = 2 * l - q;
        out[0] = channel(h + 1.0 / 3, p, q);
        out[1] = channel(h, p, q);
        out[2] = channel(h - 1.0 / 3, p, q);
    }
    auto byte = [](double v) { return quint32(std::lround(std::min(1.0, std::max(0.0, v)) * 255)); };
    return byte(out[0]) << 16 | byte(out[1]) << 8 | byte(out[2]);
}

quint32 Styles::resolve(const ColorSpec &color, quint32 automatic) const
{
    switch (color.kind) {
    case ColorSpec::Kind::Rgb: return color.rgb;
    case ColorSpec::Kind::Indexed:
        // 64 and 65 stand for the system's colors, which a cell takes from where it is shown
        if (color.index < 0 || color.index >= 64) return automatic;
        return color.index < indexed.size() ? indexed[color.index] : defaultIndexed(color.index);
    case ColorSpec::Kind::Theme:
        if (color.index < 0 || color.index >= kThemeColors) return automatic;
        return tinted(theme[color.index], color.tint);
    case ColorSpec::Kind::None:
    case ColorSpec::Kind::Auto: break;
    }
    return automatic;
}

quint32 Styles::defaultIndexed(int index)
{
    if (index < 0 || index >= 64) return 0;
    // the first eight are the basic colors, and the places from 8 are the palette that number formats name ([Color1] and on)
    static const quint32 basic[8] = {0x000000, 0xFFFFFF, 0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00, 0xFF00FF, 0x00FFFF};
    return index < 8 ? basic[index] : jp::calc::formatColorRgb(index - 7);
}

} // namespace jp::office::sheet
