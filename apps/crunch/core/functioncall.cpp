#include "core/functioncall.h"

#include "core/functioninfo.h"

#include <vector>

namespace jp::crunch {

namespace {

bool nameChar(QChar c) { return c.isLetterOrNumber() || c == QLatin1Char('_') || c == QLatin1Char('.'); }

struct Pair {
    int open, close;   // close is -1 for a bracket that is not closed
    int nameStart;     // -1 for a bracket with no name before it
};

// Every pair of round brackets of the text, outside quotes, in the order of their opening brackets.
std::vector<Pair> pairsOf(const QString &text)
{
    std::vector<Pair> pairs;
    std::vector<int> open;
    int square = 0, brace = 0;
    for (int i = 0; i < text.size(); ++i) {
        const QChar c = text.at(i);
        if (c == QLatin1Char('"') || c == QLatin1Char('\'')) {
            // (a doubled quote is one inside the text)
            int j = i + 1;
            while (j < text.size()) {
                if (text.at(j) == c) {
                    if (j + 1 < text.size() && text.at(j + 1) == c) {
                        j += 2;
                        continue;
                    }
                    break;
                }
                ++j;
            }
            i = j;
        } else if (c == QLatin1Char('[')) {
            ++square;
        } else if (c == QLatin1Char(']') && square > 0) {
            --square;
        } else if (c == QLatin1Char('{')) {
            ++brace;
        } else if (c == QLatin1Char('}') && brace > 0) {
            --brace;
        } else if (c == QLatin1Char('(') && square == 0 && brace == 0) {
            int s = i;
            while (s > 0 && nameChar(text.at(s - 1))) --s;
            pairs.push_back({i, -1, s < i ? s : -1});
            open.push_back(int(pairs.size()) - 1);
        } else if (c == QLatin1Char(')') && square == 0 && brace == 0 && !open.empty()) {
            pairs[size_t(open.back())].close = i;
            open.pop_back();
        }
    }
    return pairs;
}

QStringList splitArguments(const QString &inner, QChar separator)
{
    QStringList out;
    int depth = 0, square = 0, brace = 0, from = 0;
    for (int i = 0; i < inner.size(); ++i) {
        const QChar c = inner.at(i);
        if (c == QLatin1Char('"') || c == QLatin1Char('\'')) {
            int j = i + 1;
            while (j < inner.size()) {
                if (inner.at(j) == c) {
                    if (j + 1 < inner.size() && inner.at(j + 1) == c) {
                        j += 2;
                        continue;
                    }
                    break;
                }
                ++j;
            }
            i = j;
        } else if (c == QLatin1Char('[')) {
            ++square;
        } else if (c == QLatin1Char(']') && square > 0) {
            --square;
        } else if (c == QLatin1Char('{')) {
            ++brace;
        } else if (c == QLatin1Char('}') && brace > 0) {
            --brace;
        } else if (square == 0 && brace == 0) {
            if (c == QLatin1Char('(')) ++depth;
            else if (c == QLatin1Char(')') && depth > 0) --depth;
            else if (c == separator && depth == 0) {
                out << inner.mid(from, i - from).trimmed();
                from = i + 1;
            }
        }
    }
    out << inner.mid(from).trimmed();
    return out;
}

} // namespace

FunctionCall functionCallAt(const QString &text, int cursor, QChar separator)
{
    FunctionCall call;
    if (!text.startsWith(QLatin1Char('='))) return call;
    const std::vector<Pair> pairs = pairsOf(text);
    int best = -1;
    for (size_t i = 0; i < pairs.size(); ++i) {
        const Pair &p = pairs[i];
        const int close = p.close < 0 ? int(text.size()) : p.close;
        if (cursor <= p.open || cursor > close) continue;
        if (p.nameStart < 0 || !docOf(text.mid(p.nameStart, p.open - p.nameStart))) continue;
        best = int(i);   // (a later pair that holds the cursor is inside the earlier ones)
    }
    if (best < 0) return call;
    const Pair &p = pairs[size_t(best)];
    call.found = true;
    call.name = text.mid(p.nameStart, p.open - p.nameStart);
    call.start = p.nameStart;
    call.closed = p.close >= 0;
    call.end = call.closed ? p.close + 1 : int(text.size());
    const QString inner = text.mid(p.open + 1, (call.closed ? p.close : int(text.size())) - p.open - 1);
    if (!inner.trimmed().isEmpty()) call.arguments = splitArguments(inner, separator);
    return call;
}

QString callText(const QString &name, const QStringList &arguments, QChar separator)
{
    QStringList args = arguments;
    while (!args.isEmpty() && args.last().trimmed().isEmpty()) args.removeLast();
    return name + QLatin1Char('(') + args.join(separator) + QLatin1Char(')');
}

} // namespace jp::crunch
