#include "grid/text.h"

#include <algorithm>
#include <cmath>

namespace jp::grid {

namespace {

constexpr int kMaxShownChars = 1024;
constexpr double kIndentWidth = 9;

double indentPixels(const CellLook &look) { return look.indent * kIndentWidth; }

} // namespace

TextMeter::~TextMeter() = default;

double textRoom(double cellWidth, const CellLook &look) { return cellWidth - 2 * kCellPadding - indentPixels(look); }

int charsFor(double cellWidth, int maxDigitWidth)
{
    if (maxDigitWidth <= 0) return 1;
    return std::max(1, int((cellWidth - 5) / maxDigitWidth));
}

Shown showValue(const jp::calc::Value &given, const CellLook &look, jp::calc::DateSystem system, double cellWidth, int maxDigitWidth, const TextMeter &meter)
{
    using jp::calc::Value;
    Shown s;
    const Value &v = given.isArray() ? given.at(0, 0) : given;
    switch (v.kind()) {
    case Value::Kind::Empty:
    case Value::Kind::Array:
        s.kind = Kind::Empty;
        return s;
    case Value::Kind::Number: {
        s.kind = Kind::Number;
        const jp::calc::Formatted f = look.format.format(v.number(), system, charsFor(cellWidth, maxDigitWidth));
        s.text = f.text;
        if (f.color > 0) s.color = 0xFF000000u | jp::calc::formatColorRgb(f.color);
        s.fillAt = f.fillAt;
        s.fillChar = f.fillChar;
        const double room = textRoom(cellWidth, look);
        s.width = f.failed ? room + 1 : meter.width(s.text, look);
        if (f.failed || s.width > room) {
            // too wide for the cell, or a value the format cannot show: the cell is filled with "#"
            const double one = std::max(1.0, meter.width(QStringLiteral("#"), look));
            const int count = std::max(1, int(room / one));
            s.text = QString(count, QLatin1Char('#'));
            s.hashes = true;
            s.fillAt = -1;
            s.width = meter.width(s.text, look);
        }
        break;
    }
    case Value::Kind::Bool:
        s.kind = Kind::Bool;
        s.text = v.boolean() ? QStringLiteral("TRUE") : QStringLiteral("FALSE");
        s.width = meter.width(s.text, look);
        break;
    case Value::Kind::Error:
        s.kind = Kind::Error;
        s.text = jp::calc::errorKindText(v.errorKind());
        s.width = meter.width(s.text, look);
        break;
    case Value::Kind::Text: {
        s.kind = Kind::Text;
        QString text = v.text();
        if (look.hasFormat) {
            const jp::calc::Formatted f = look.format.formatText(text);
            if (!f.failed) text = f.text;
        }
        // (a line break shows as a space in a text that is not wrapped; a cell shows at most this many characters)
        if (!look.wrap) text.replace(QLatin1Char('\n'), QLatin1Char(' ')).remove(QLatin1Char('\r'));
        if (text.size() > kMaxShownChars) text.truncate(kMaxShownChars);
        s.text = text;
        s.width = meter.width(s.text, look);
        break;
    }
    }
    s.align = look.horizontal != HAlign::General ? look.horizontal : s.kind == Kind::Number ? HAlign::Right : s.kind == Kind::Text ? HAlign::Left : HAlign::Center;
    return s;
}

Spill spillOf(const Shown &shown, const CellLook &look, double cellWidth, const std::function<double(int step)> &neighbor)
{
    Spill sp;
    if (shown.kind != Kind::Text || shown.hashes || look.wrap || look.shrink || look.rotation != 0) return sp;
    const double need = shown.width + 2 * kCellPadding + indentPixels(look) - cellWidth;
    if (need <= 0) return sp;
    const bool both = shown.align == HAlign::Center || shown.align == HAlign::CenterContinuous;
    const bool toLeft = shown.align == HAlign::Right;
    auto go = [&](int direction, double target, int *columns) {
        double gained = 0;
        for (int step = 1; step <= kMaxSpill && gained < target; ++step) {
            const double w = neighbor(direction * step);
            if (w < 0) break;
            gained += w;
            *columns = step;
        }
        return gained;
    };
    if (both) {
        sp.extra = go(-1, need / 2, &sp.left) + go(1, need / 2, &sp.right);
    } else if (toLeft) {
        sp.extra = go(-1, need, &sp.left);
    } else {
        sp.extra = go(1, need, &sp.right);
    }
    return sp;
}

} // namespace jp::grid
