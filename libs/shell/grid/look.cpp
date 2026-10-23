#include "grid/look.h"

#include <algorithm>
#include <cmath>

namespace jp::grid {

using jp::office::sheet::ColorSpec;
using jp::office::sheet::Styles;

namespace {

QRgb opaque(quint32 rgb) { return 0xFF000000u | (rgb & 0xFFFFFF); }

LineLook lineOf(const Styles &s, const jp::office::sheet::BorderSide &side)
{
    LineLook l;
    l.style = side.style;
    l.color = opaque(s.resolve(side.color, 0x000000));
    return l;
}

// How wide the widest digit of a family is, as a part of the em (what a column's width in characters is counted in).
double digitFactor(const QString &family)
{
    const QString n = family.toLower();
    auto has = [&](const char *part) { return n.contains(QLatin1String(part)); };
    if (has("calibri") || has("carlito")) return 0.507;
    if (has("verdana")) return 0.636;
    if (has("tahoma")) return 0.546;
    if (has("courier") || has("mono")) return 0.6;
    if (has("consolas")) return 0.55;
    if (has("arial") || has("helvetica") || has("liberation sans") || has("arimo")) return 0.556;
    if (has("times") || has("liberation serif") || has("tinos")) return 0.5;
    return 0.55;
}

} // namespace

LookTable::LookTable(const Styles *styles) : m_styles(styles) {}

void LookTable::setStyles(const Styles *styles)
{
    m_styles = styles;
    m_looks.clear();
    m_fontIdOfFont.clear();
}

void LookTable::trim() const
{
    if (m_looks.size() > kMaxHeld) m_looks.clear();
}

int LookTable::maxDigitWidth() const
{
    if (!m_styles || m_styles->fonts.isEmpty()) return 7;
    const jp::office::sheet::FontSpec &f = m_styles->fonts[0];
    const double em = f.size * 96.0 / 72.0;
    return int(std::min(64L, std::max(3L, std::lround(em * digitFactor(f.name)))));
}

const CellLook &LookTable::look(quint32 xf) const
{
    auto it = m_looks.find(xf);
    if (it == m_looks.end()) {
        ++m_made;
        it = m_looks.emplace(xf, std::make_unique<CellLook>(make(xf))).first;
    }
    return *it->second;
}

quint32 LookTable::fontIdOf(quint32 fontIndex, const CellLook &l) const
{
    // (a font is looked up by its place in the styles: its signature, which holds its name, is built once for each font and not for each look)
    if (fontIndex < m_fontIdOfFont.size() && m_fontIdOfFont[fontIndex] >= 0) return quint32(m_fontIdOfFont[fontIndex]);
    ++m_signaturesMade;
    const std::string signature = l.family.toStdString() + '|' + std::to_string(l.pointSize) + '|' + char('0' + l.bold) + char('0' + l.italic) + char('0' + l.strike) +
                                  char('0' + int(l.underline)) + char('0' + l.vertAlign);
    quint32 id;
    const auto it = m_fontIds.find(signature);
    if (it != m_fontIds.end()) {
        id = it->second;
    } else {
        id = quint32(m_fontIds.size());
        m_fontIds.emplace(signature, id);
    }
    if (fontIndex < 100000000u) {
        if (fontIndex >= m_fontIdOfFont.size()) m_fontIdOfFont.resize(size_t(fontIndex) + 1, -1);
        m_fontIdOfFont[fontIndex] = id;
    }
    return id;
}

CellLook LookTable::make(quint32 xf) const
{
    CellLook l;
    if (!m_styles) return l;
    const Styles &s = *m_styles;
    const jp::office::sheet::CellFormat &f = s.format(xf);
    const jp::office::sheet::FontSpec &font = s.font(f.font);
    l.family = font.name;
    l.pointSize = font.size;
    l.bold = font.bold;
    l.italic = font.italic;
    l.strike = font.strike;
    l.underline = font.underline;
    l.vertAlign = font.vertAlign;
    l.textColor = opaque(s.resolve(font.color, 0x000000));
    l.fontId = fontIdOf(f.font, l);
    const jp::office::sheet::FillSpec &fill = s.fill(f.fill);
    if (fill.pattern == PatternKind::Solid && !fill.fg.isSet()) {
        l.pattern = PatternKind::None;   // (a solid fill that names no color is not drawn)
    } else {
        l.pattern = fill.pattern;
        l.fg = opaque(s.resolve(fill.fg, 0x000000));
        l.bg = opaque(s.resolve(fill.bg, 0xFFFFFF));
    }
    const jp::office::sheet::BorderSpec &b = s.border(f.border);
    l.left = lineOf(s, b.left);
    l.right = lineOf(s, b.right);
    l.top = lineOf(s, b.top);
    l.bottom = lineOf(s, b.bottom);
    l.diagonal = lineOf(s, b.diagonal);
    l.diagonalUp = b.diagonalUp;
    l.diagonalDown = b.diagonalDown;
    l.horizontal = f.horizontal;
    l.vertical = f.vertical;
    l.wrap = f.wrap;
    l.shrink = f.shrink;
    l.indent = f.indent;
    l.rotation = f.rotation;
    l.format = jp::calc::NumberFormat(s.formatCode(xf));
    l.hasFormat = !l.format.isGeneral();
    return l;
}

} // namespace jp::grid
