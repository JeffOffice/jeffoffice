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

// Where the text in quotes that begins at `i` ends (its closing quote; the text's end when it is not closed). A doubled quote is one inside the text.
int quotedEnd(const QString &text, int i)
{
    const QChar q = text.at(i);
    int j = i + 1;
    while (j < text.size()) {
        if (text.at(j) == q) {
            if (j + 1 < text.size() && text.at(j + 1) == q) {
                j += 2;
                continue;
            }
            return j;
        }
        ++j;
    }
    return int(text.size());
}

// Passes the characters that are not formula to look into: text in quotes, the inside of square brackets (where an apostrophe makes the next character a plain one, as in Table1[['#Items]]),
// and braces. Returns true if `i` was moved past something (to its last character).
bool skipLiteral(const QString &text, int *i, int *square, int *brace)
{
    const QChar c = text.at(*i);
    if (*square > 0) {
        if (c == QLatin1Char('\'')) ++*i;   // (the next character is not a bracket)
        else if (c == QLatin1Char('[')) ++*square;
        else if (c == QLatin1Char(']')) --*square;
        return true;
    }
    if (c == QLatin1Char('"') || c == QLatin1Char('\'')) {
        *i = quotedEnd(text, *i);
        return true;
    }
    if (c == QLatin1Char('[')) {
        ++*square;
        return true;
    }
    if (c == QLatin1Char('{')) {
        ++*brace;
        return true;
    }
    if (c == QLatin1Char('}') && *brace > 0) {
        --*brace;
        return true;
    }
    return *brace > 0;
}

// Every pair of round brackets of the text, outside quotes, in the order of their opening brackets.
std::vector<Pair> pairsOf(const QString &text)
{
    std::vector<Pair> pairs;
    std::vector<int> open;
    int square = 0, brace = 0;
    for (int i = 0; i < text.size(); ++i) {
        if (skipLiteral(text, &i, &square, &brace)) continue;
        const QChar c = text.at(i);
        if (c == QLatin1Char('(')) {
            int s = i;
            while (s > 0 && nameChar(text.at(s - 1))) --s;
            pairs.push_back({i, -1, s < i ? s : -1});
            open.push_back(int(pairs.size()) - 1);
        } else if (c == QLatin1Char(')') && !open.empty()) {
            pairs[size_t(open.back())].close = i;
            open.pop_back();
        }
    }
    return pairs;
}

// The arguments as written (not trimmed). `unclosed` is the quote of a text that is not closed at the end of `inner` (0 for none).
QStringList splitArguments(const QString &inner, QChar separator, QChar *unclosed)
{
    QStringList out;
    int depth = 0, square = 0, brace = 0, from = 0;
    *unclosed = QChar();
    for (int i = 0; i < inner.size(); ++i) {
        const bool quote = square == 0 && (inner.at(i) == QLatin1Char('"') || inner.at(i) == QLatin1Char('\''));
        const int begin = i;
        if (skipLiteral(inner, &i, &square, &brace)) {
            if (quote && i >= inner.size()) *unclosed = inner.at(begin);
            continue;
        }
        const QChar c = inner.at(i);
        if (c == QLatin1Char('(')) ++depth;
        else if (c == QLatin1Char(')') && depth > 0) --depth;
        else if (c == separator && depth == 0) {
            out << inner.mid(from, i - from);
            from = i + 1;
        }
    }
    out << inner.mid(from);
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
    if (!inner.trimmed().isEmpty()) {
        QChar unclosed;
        call.raw = splitArguments(inner, separator, &unclosed);
        // a text that was not closed is closed with its own quote, so that what is written back is a formula that reads
        if (unclosed != QChar()) call.raw.last().append(unclosed);
        for (const QString &a : call.raw) call.arguments << a.trimmed();
    }
    return call;
}

QString callText(const QString &name, const QStringList &arguments, QChar separator, int keep)
{
    QStringList args = arguments;
    while (int(args.size()) > keep && args.last().trimmed().isEmpty()) args.removeLast();
    return name + QLatin1Char('(') + args.join(separator) + QLatin1Char(')');
}

} // namespace jp::crunch
