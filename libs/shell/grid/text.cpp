#include "grid/text.h"

#include <algorithm>
#include <cmath>
#include <cstring>

namespace jp::grid {

namespace {

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
        // (what a cell shows is cut first: a text of millions of characters costs no more than a short one, in its number format's text section too)
        QString text = shownLine(v.text(), look.wrap);
        if (look.hasFormat) {
            const jp::calc::Formatted f = look.format.formatText(text);
            if (!f.failed) text = shownLine(f.text, look.wrap);
        }
        s.text = text;
        s.width = meter.width(s.text, look);
        break;
    }
    }
    s.align = look.horizontal != HAlign::General ? look.horizontal : s.kind == Kind::Number ? HAlign::Right : s.kind == Kind::Text ? HAlign::Left : HAlign::Center;
    return s;
}

QString displayFormula(const QString &formula)
{
    if (!formula.contains(QLatin1String("_xl"), Qt::CaseInsensitive)) return formula;
    static const char *const kPrefixes[] = {"_xlfn.", "_xlws.", "_xludf.", "_xlpm."};
    auto identChar = [](QChar c) { return c.isLetterOrNumber() || c == QLatin1Char('_') || c == QLatin1Char('.'); };
    QString out;
    out.reserve(formula.size());
    QChar quote;   // the quote of a text ("...") or a sheet's name ('...') that the formula is in
    const int n = formula.size();
    for (int i = 0; i < n;) {
        const QChar c = formula.at(i);
        if (!quote.isNull()) {
            out += c;
            if (c == quote) {
                if (i + 1 < n && formula.at(i + 1) == quote) out += formula.at(++i);   // (a doubled quote is one inside)
                else quote = QChar();
            }
            ++i;
            continue;
        }
        if (c == QLatin1Char('"') || c == QLatin1Char('\'')) {
            quote = c;
            out += c;
            ++i;
            continue;
        }
        if (c == QLatin1Char('_') && (i == 0 || !identChar(formula.at(i - 1)))) {
            // (one or more: a function only a sheet has is _xlfn._xlws.FILTER)
            int skipped = 0;
            for (bool found = true; found;) {
                found = false;
                for (const char *prefix : kPrefixes) {
                    const int len = int(strlen(prefix));
                    if (formula.mid(i + skipped, len).compare(QLatin1String(prefix), Qt::CaseInsensitive) == 0) {
                        skipped += len;
                        found = true;
                        break;
                    }
                }
            }
            if (skipped) {
                i += skipped;
                continue;
            }
        }
        out += c;
        ++i;
    }
    return out;
}

QString shownLine(const QString &given, bool wrap, qint64 *scanned)
{
    const qsizetype n = given.size();
    if (wrap) {
        // (line breaks stay in a text that wraps; only the cut)
        if (scanned) *scanned = std::min<qint64>(n, kMaxShownChars);
        return n > kMaxShownChars ? given.left(kMaxShownChars) : given;
    }
    // the cut is made as the text is cleaned, so that a text of millions of characters costs what is shown of it (a return left out does not count toward what is shown)
    QString out;
    out.reserve(int(std::min<qsizetype>(n, kMaxShownChars)));
    qint64 looked = 0;
    // (and no more than kMaxScannedChars are looked at to find them: a text of millions of carriage returns, which show as nothing, is not read to its end)
    const qsizetype scan = std::min<qsizetype>(n, kMaxScannedChars);
    for (qsizetype i = 0; i < scan && out.size() < kMaxShownChars; ++i) {
        ++looked;
        const QChar c = given.at(i);
        if (c == QLatin1Char('\r')) continue;
        out.append(c == QLatin1Char('\n') ? QLatin1Char(' ') : c);
    }
    if (scanned) *scanned = looked;
    return out;
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
