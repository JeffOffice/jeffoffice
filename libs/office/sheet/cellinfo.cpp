#include "sheet/cellinfo.h"

#include <QStringList>

namespace jp::office::sheet {

namespace {

// The sections of a code (split at ";" outside quotes and brackets).
QStringList sectionsOf(const QString &code)
{
    QStringList out;
    QString cur;
    bool quote = false, bracket = false;
    for (int i = 0; i < code.size(); ++i) {
        const QChar ch = code[i];
        if (ch == QLatin1Char('\\') && i + 1 < code.size() && !quote && !bracket) {
            cur += ch;
            cur += code[++i];
            continue;
        }
        if (ch == QLatin1Char('"') && !bracket) quote = !quote;
        else if (!quote && ch == QLatin1Char('[')) bracket = true;
        else if (!quote && ch == QLatin1Char(']')) bracket = false;
        if (ch == QLatin1Char(';') && !quote && !bracket) {
            out << cur;
            cur.clear();
        } else {
            cur += ch;
        }
    }
    out << cur;
    return out;
}

// A section without its quoted text, its escaped characters, its pads and fills, and its brackets (a currency sign in a bracket stays as "$"; a color or a condition goes).
// `color` is told whether a bracket named a color.
QString bare(const QString &section, bool *color = nullptr, bool *currency = nullptr)
{
    static const QStringList colors{QStringLiteral("black"), QStringLiteral("white"), QStringLiteral("red"), QStringLiteral("green"), QStringLiteral("blue"), QStringLiteral("yellow"),
                                    QStringLiteral("magenta"), QStringLiteral("cyan")};
    QString out;
    for (int i = 0; i < section.size(); ++i) {
        const QChar ch = section[i];
        if (ch == QLatin1Char('"')) {
            const int end = section.indexOf(QLatin1Char('"'), i + 1);
            const QString quoted = end < 0 ? section.mid(i + 1) : section.mid(i + 1, end - i - 1);
            if (currency && quoted.contains(QLatin1Char('$'))) *currency = true;
            i = end < 0 ? int(section.size()) : end;
        } else if (ch == QLatin1Char('\\') || ch == QLatin1Char('_') || ch == QLatin1Char('*')) {
            if (ch == QLatin1Char('\\') && i + 1 < section.size() && section[i + 1] == QLatin1Char('$') && currency) *currency = true;
            ++i;
        } else if (ch == QLatin1Char('[')) {
            const int end = section.indexOf(QLatin1Char(']'), i + 1);
            const QString inner = (end < 0 ? section.mid(i + 1) : section.mid(i + 1, end - i - 1)).toLower();
            if (inner.startsWith(QLatin1Char('$'))) {
                if (currency) *currency = true;
            } else if (color && (colors.contains(inner) || inner.startsWith(QLatin1String("color")))) {
                *color = true;
            } else if (inner == QLatin1String("h") || inner == QLatin1String("hh") || inner == QLatin1String("m") || inner == QLatin1String("mm") || inner == QLatin1String("s") || inner == QLatin1String("ss")) {
                out += inner[0];   // (an elapsed time)
            }
            i = end < 0 ? int(section.size()) : end;
        } else if (ch == QLatin1Char('$')) {
            if (currency) *currency = true;
        } else {
            out += ch;
        }
    }
    return out;
}

int decimalsOf(const QString &b)
{
    const int dot = b.indexOf(QLatin1Char('.'));
    if (dot < 0) return 0;
    int n = 0;
    for (int i = dot + 1; i < b.size() && (b[i] == QLatin1Char('0') || b[i] == QLatin1Char('#') || b[i] == QLatin1Char('?')); ++i) ++n;
    return n;
}

QString dateCode(const QString &lower)
{
    const bool hasH = lower.contains(QLatin1Char('h')), hasS = lower.contains(QLatin1Char('s')), ampm = lower.contains(QLatin1String("am/pm")) || lower.contains(QLatin1String("a/p"));
    const bool hasY = lower.contains(QLatin1Char('y')), hasD = lower.contains(QLatin1Char('d'));
    if (hasH) {
        if (hasS) return ampm ? QStringLiteral("D6") : QStringLiteral("D8");
        return ampm ? QStringLiteral("D7") : QStringLiteral("D9");
    }
    const bool hasM = lower.contains(QLatin1Char('m')), monthName = lower.contains(QLatin1String("mmm"));
    if (hasY && hasD) return monthName ? QStringLiteral("D1") : QStringLiteral("D4");
    if (hasY && hasM) return QStringLiteral("D3");
    if (hasD && hasM) return monthName ? QStringLiteral("D2") : QStringLiteral("D5");
    if (hasY) return QStringLiteral("D4");
    return QStringLiteral("D1");
}

} // namespace

FormatSummary summarizeFormat(const QString &code)
{
    FormatSummary out;
    const QStringList sections = sectionsOf(code);
    bool positiveColor = false, currency = false;
    const QString positive = bare(sections[0], &positiveColor, &currency);
    if (sections.size() > 1) bare(sections[1], &out.negativeColor);
    out.parentheses = positive.contains(QLatin1Char('('));
    const QString lower = positive.toLower();
    QString kind;
    if (code.trimmed().isEmpty() || lower.trimmed() == QLatin1String("general")) {
        kind = QStringLiteral("G");
    } else if (lower.contains(QLatin1String("e+")) || lower.contains(QLatin1String("e-"))) {
        kind = QStringLiteral("S") + QString::number(decimalsOf(positive));
    } else if (lower.contains(QLatin1Char('d')) || lower.contains(QLatin1Char('y')) || lower.contains(QLatin1Char('h')) || lower.contains(QLatin1Char('s')) || lower.contains(QLatin1Char('m'))) {
        kind = dateCode(lower);
    } else if (positive.contains(QLatin1Char('%'))) {
        kind = QStringLiteral("P") + QString::number(decimalsOf(positive));
    } else if (!positive.contains(QLatin1Char('0')) && !positive.contains(QLatin1Char('#')) && !positive.contains(QLatin1Char('?'))) {
        kind = QStringLiteral("G");
    } else if (positive.contains(QLatin1Char('/'))) {
        kind = QStringLiteral("G");   // a fraction
    } else if (currency) {
        kind = QStringLiteral("C") + QString::number(decimalsOf(positive));
    } else {
        // a comma between digit places groups them; the commas after the last place divide by a thousand
        bool group = false;
        for (int i = 0; i + 1 < positive.size(); ++i)
            if (positive[i] == QLatin1Char(',') && (positive[i + 1] == QLatin1Char('0') || positive[i + 1] == QLatin1Char('#') || positive[i + 1] == QLatin1Char('?'))) group = true;
        kind = (group ? QStringLiteral(",") : QStringLiteral("F")) + QString::number(decimalsOf(positive));
    }
    out.code = kind;
    if (kind != QLatin1String("G")) {
        if (out.negativeColor) out.code += QLatin1Char('-');
        if (out.parentheses) out.code += QLatin1String("()");
    }
    return out;
}

} // namespace jp::office::sheet
