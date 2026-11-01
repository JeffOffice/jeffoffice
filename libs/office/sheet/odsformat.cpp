#include "sheet/odsformat.h"

#include <QStringList>
#include <QVector>

namespace jp::office::sheet {

namespace {

QString esc(const QString &s)
{
    QString out;
    for (const QChar c : s) {
        if (c == QLatin1Char('&')) out += QLatin1String("&amp;");
        else if (c == QLatin1Char('<')) out += QLatin1String("&lt;");
        else if (c == QLatin1Char('>')) out += QLatin1String("&gt;");
        else if (c == QLatin1Char('"')) out += QLatin1String("&quot;");
        else out += c;
    }
    return out;
}

struct Section {
    QString color;       // "#RRGGBB" or empty
    QString condition;   // "value()>100" or empty
    QString body;        // the elements
    bool percent = false, date = false, time = false, text = false, elapsed = false;
};

bool colorOfName(const QString &name, QString *rgb)
{
    static const struct { const char *name; const char *rgb; } colors[] = {{"black", "#000000"}, {"blue", "#0000FF"}, {"cyan", "#00FFFF"}, {"green", "#00FF00"},
                                                                         {"magenta", "#FF00FF"}, {"red", "#FF0000"}, {"white", "#FFFFFF"}, {"yellow", "#FFFF00"}};
    for (const auto &c : colors)
        if (name.compare(QLatin1String(c.name), Qt::CaseInsensitive) == 0) {
            *rgb = QLatin1String(c.rgb);
            return true;
        }
    return false;
}

class SectionParser {
public:
    explicit SectionParser(const QString &s) : m_s(s) {}

    bool parse(Section *out)
    {
        QString literal;
        bool literalQuoted = false;   // (the text in quotes is kept apart from the text without: the code comes back as it was written)
        auto addLiteral = [&](const QString &text, bool quoted) {
            if (!literal.isEmpty() && quoted != literalQuoted) {
                m_parts.append(QStringLiteral("<number:text>") + esc(literal) + QStringLiteral("</number:text>"));
                literal.clear();
            }
            literalQuoted = quoted;
            literal += text;
        };
        auto flushLiteral = [&]() {
            if (!literal.isEmpty()) {
                m_parts.append(QStringLiteral("<number:text>") + esc(literal) + QStringLiteral("</number:text>"));
                literal.clear();
            }
        };
        int i = 0;
        const int n = m_s.size();
        while (i < n) {
            const QChar c = m_s[i];
            if (c == QLatin1Char('"')) {
                const int end = int(m_s.indexOf(QLatin1Char('"'), i + 1));
                if (end < 0) return false;
                addLiteral(m_s.mid(i + 1, end - i - 1), true);
                i = end + 1;
            } else if (c == QLatin1Char('\\')) {
                if (i + 1 < n) addLiteral(QString(m_s[i + 1]), true);
                i += 2;
            } else if (c == QLatin1Char('_')) {
                // the space as wide as the next character (the other program's extension names the character)
                flushLiteral();
                if (i + 1 < n) m_parts.append(QStringLiteral("<number:text loext:blank-width-char=\"%1\"> </number:text>").arg(esc(QString(m_s[i + 1]))));
                i += 2;
            } else if (c == QLatin1Char('*')) {
                // the character that fills the rest of the width
                flushLiteral();
                if (i + 1 < n) m_parts.append(QStringLiteral("<number:fill-character>%1</number:fill-character>").arg(esc(QString(m_s[i + 1]))));
                i += 2;
            } else if (c == QLatin1Char('[')) {
                const int end = int(m_s.indexOf(QLatin1Char(']'), i));
                if (end < 0) return false;
                const QString inner = m_s.mid(i + 1, end - i - 1);
                i = end + 1;
                QString rgb;
                if (colorOfName(inner, &rgb)) {
                    out->color = rgb;
                } else if (inner.startsWith(QLatin1Char('$'))) {
                    // a currency: the symbol, before the dash that gives the language
                    const int dash = int(inner.indexOf(QLatin1Char('-')));
                    addLiteral(inner.mid(1, dash < 0 ? -1 : dash - 1), true);
                } else if (inner.startsWith(QLatin1Char('>')) || inner.startsWith(QLatin1Char('<')) || inner.startsWith(QLatin1Char('='))) {
                    QString op = inner.left(inner.size() > 1 && (inner[1] == QLatin1Char('=') || inner[1] == QLatin1Char('>')) ? 2 : 1);
                    QString num = inner.mid(op.size());
                    bool ok = false;
                    num.toDouble(&ok);
                    if (!ok) return false;
                    if (op == QLatin1String("<>")) op = QStringLiteral("!=");
                    out->condition = QStringLiteral("value()") + op + num;
                } else if (inner.compare(QLatin1String("h"), Qt::CaseInsensitive) == 0 || inner.compare(QLatin1String("hh"), Qt::CaseInsensitive) == 0
                           || inner.compare(QLatin1String("m"), Qt::CaseInsensitive) == 0 || inner.compare(QLatin1String("mm"), Qt::CaseInsensitive) == 0
                           || inner.compare(QLatin1String("s"), Qt::CaseInsensitive) == 0 || inner.compare(QLatin1String("ss"), Qt::CaseInsensitive) == 0) {
                    flushLiteral();
                    out->elapsed = true;
                    out->time = true;
                    const QChar u = inner[0].toLower();
                    const bool lng = inner.size() == 2;
                    m_dates.append({u == QLatin1Char('h') ? 'h' : u == QLatin1Char('m') ? 'M' : 's', lng, true});
                    m_parts.append(QString(QChar(ushort(0xE000 + m_dates.size() - 1))));
                } else if (inner.startsWith(QLatin1String("Color"), Qt::CaseInsensitive)) {
                    // (a color of the palette: no data style has it)
                } else {
                    return false;
                }
            } else if (c == QLatin1Char('0') || c == QLatin1Char('#') || c == QLatin1Char('?') || (c == QLatin1Char('.') && i + 1 < n && (m_s[i + 1] == QLatin1Char('0') || m_s[i + 1] == QLatin1Char('#') || m_s[i + 1] == QLatin1Char('?')))) {
                flushLiteral();
                int j = i;
                while (j < n && (m_s[j] == QLatin1Char('0') || m_s[j] == QLatin1Char('#') || m_s[j] == QLatin1Char('?') || m_s[j] == QLatin1Char(',') || m_s[j] == QLatin1Char('.'))) ++j;
                const QString picture = m_s.mid(i, j - i);
                // a fraction: digits, a slash, digits (with the whole number before it, after a space)
                if (j < n && m_s[j] == QLatin1Char('/')) {
                    int k = j + 1;
                    while (k < n && (m_s[k].isDigit() || m_s[k] == QLatin1Char('?') || m_s[k] == QLatin1Char('#') || m_s[k] == QLatin1Char('0'))) ++k;
                    if (k == j + 1) return false;
                    const QString denominator = m_s.mid(j + 1, k - j - 1);
                    bool fixed = false;
                    const int fixedValue = denominator.toInt(&fixed);
                    QString whole;
                    // the whole part was put before as a picture followed by a space
                    int minInteger = 0;
                    if (!m_parts.isEmpty() && m_pendingWhole) {
                        minInteger = m_pendingWholeMin;
                        m_parts.removeLast();
                        m_pendingWhole = false;
                    }
                    QString frac = QStringLiteral("<number:fraction number:min-integer-digits=\"%1\" number:min-numerator-digits=\"%2\"").arg(minInteger).arg(std::max<qsizetype>(1, picture.size()));
                    if (fixed) frac += QStringLiteral(" number:denominator-value=\"%1\"").arg(fixedValue);
                    else frac += QStringLiteral(" number:min-denominator-digits=\"%1\"").arg(denominator.size());
                    frac += QStringLiteral("/>");
                    m_parts.append(frac);
                    i = k;
                    continue;
                }
                if (j < n && (m_s[j] == QLatin1Char('E') || m_s[j] == QLatin1Char('e')) && j + 2 < n && (m_s[j + 1] == QLatin1Char('+') || m_s[j + 1] == QLatin1Char('-')) && m_s[j + 2] == QLatin1Char('0')) {
                    int k = j + 2;
                    while (k < n && m_s[k] == QLatin1Char('0')) ++k;
                    m_parts.append(scientific(picture, k - j - 2, m_s[j + 1] == QLatin1Char('+')));
                    i = k;
                    continue;
                }
                // the whole part of a mixed fraction ("# ?/?"): a bare "#" or "0" followed by a space
                if (picture.size() >= 1 && j + 1 < n && m_s[j] == QLatin1Char(' ') && !picture.contains(QLatin1Char('.')) && m_s.indexOf(QLatin1Char('/'), j) > 0) {
                    const int slash = int(m_s.indexOf(QLatin1Char('/'), j));
                    bool onlyPlaceholders = true;
                    for (int q = j + 1; q < slash; ++q)
                        if (m_s[q] != QLatin1Char('?') && m_s[q] != QLatin1Char('#') && m_s[q] != QLatin1Char('0')) onlyPlaceholders = false;
                    if (onlyPlaceholders) {
                        m_pendingWhole = true;
                        m_pendingWholeMin = 1;
                        m_parts.append(QString());
                        i = j + 1;
                        continue;
                    }
                }
                m_parts.append(numberPicture(picture));
                i = j;
            } else if (c == QLatin1Char('%')) {
                flushLiteral();
                out->percent = true;
                m_parts.append(QStringLiteral("<number:text>%</number:text>"));
                ++i;
            } else if (c == QLatin1Char('@')) {
                flushLiteral();
                out->text = true;
                m_parts.append(QStringLiteral("<number:text-content/>"));
                ++i;
            } else if (c.isLetter()) {
                // General, a date or time code, or text
                int j = i;
                while (j < n && m_s[j].toLower() == c.toLower()) ++j;
                const int run = j - i;
                const QChar u = c.toLower();
                if (m_s.mid(i, 7).compare(QLatin1String("General"), Qt::CaseInsensitive) == 0) {
                    flushLiteral();
                    m_parts.append(QStringLiteral("<number:number number:min-integer-digits=\"1\"/>"));
                    i += 7;
                } else if ((u == QLatin1Char('a') && (m_s.mid(i, 5).compare(QLatin1String("AM/PM"), Qt::CaseInsensitive) == 0 || m_s.mid(i, 3).compare(QLatin1String("A/P"), Qt::CaseInsensitive) == 0))) {
                    flushLiteral();
                    out->time = true;
                    m_parts.append(QStringLiteral("<number:am-pm/>"));
                    i += m_s.mid(i, 5).compare(QLatin1String("AM/PM"), Qt::CaseInsensitive) == 0 ? 5 : 3;
                } else if (u == QLatin1Char('y') || u == QLatin1Char('d') || u == QLatin1Char('h') || u == QLatin1Char('s') || u == QLatin1Char('m')) {
                    flushLiteral();
                    m_dates.append({u == QLatin1Char('m') ? 'M' : u.toLatin1(), run >= 2, false, run});
                    m_parts.append(QString(QChar(ushort(0xE000 + m_dates.size() - 1))));
                    if (u == QLatin1Char('y') || u == QLatin1Char('d')) out->date = true;
                    else out->time = true;
                    i = j;
                } else {
                    addLiteral(QString(c), true);
                    ++i;
                }
            } else {
                addLiteral(QString(c), false);
                ++i;
            }
        }
        flushLiteral();
        // the date tokens: which "m" is a month and which a minute
        for (int p = 0; p < m_parts.size(); ++p) {
            if (m_parts[p].size() != 1 || m_parts[p][0].unicode() < 0xE000) continue;
            DateTok &t = m_dates[m_parts[p][0].unicode() - 0xE000];
            if (t.kind == 'M') {
                bool minute = false;
                // the nearest date token before and after
                for (int q = p - 1; q >= 0; --q)
                    if (m_parts[q].size() == 1 && m_parts[q][0].unicode() >= 0xE000) {
                        minute = m_dates[m_parts[q][0].unicode() - 0xE000].kind == 'h';
                        break;
                    }
                if (!minute)
                    for (int q = p + 1; q < m_parts.size(); ++q)
                        if (m_parts[q].size() == 1 && m_parts[q][0].unicode() >= 0xE000) {
                            minute = m_dates[m_parts[q][0].unicode() - 0xE000].kind == 's';
                            break;
                        }
                t.kind = minute ? 'm' : 'M';
                if (!minute) out->date = true;
                else out->time = true;
            }
        }
        QString body;
        for (int p = 0; p < m_parts.size(); ++p) {
            const QString &part = m_parts[p];
            if (part.size() == 1 && part[0].unicode() >= 0xE000) {
                const DateTok &t = m_dates[part[0].unicode() - 0xE000];
                const QString style = t.longStyle ? QStringLiteral(" number:style=\"long\"") : QString();
                switch (t.kind) {
                case 'y': body += QStringLiteral("<number:year%1/>").arg(t.run >= 3 ? QStringLiteral(" number:style=\"long\"") : QString()); break;
                case 'M': body += t.run >= 3 ? QStringLiteral("<number:month number:textual=\"true\"%1/>").arg(t.run >= 4 ? QStringLiteral(" number:style=\"long\"") : QString())
                                             : QStringLiteral("<number:month%1/>").arg(style);
                    break;
                case 'd': body += t.run >= 3 ? QStringLiteral("<number:day-of-week%1/>").arg(t.run >= 4 ? QStringLiteral(" number:style=\"long\"") : QString()) : QStringLiteral("<number:day%1/>").arg(style); break;
                case 'h': body += QStringLiteral("<number:hours%1/>").arg(style); break;
                case 'm': body += QStringLiteral("<number:minutes%1/>").arg(style); break;
                case 's': {
                    // seconds with a fraction: ".0", ".00" right after
                    int decimals = 0;
                    if (p + 2 < m_parts.size() + 1 && p + 1 < m_parts.size() && m_parts[p + 1].startsWith(QLatin1String("<number:text>."))) {
                        const QString lit = m_parts[p + 1];
                        const int end = int(lit.indexOf(QLatin1String("</number:text>")));
                        const QString text = lit.mid(14, end - 14);
                        bool allZero = text.size() >= 2 && text[0] == QLatin1Char('.');
                        for (int q = 1; q < text.size(); ++q)
                            if (text[q] != QLatin1Char('0')) allZero = false;
                        if (allZero && text.size() <= 4) {
                            decimals = int(text.size()) - 1;
                            m_parts[p + 1].clear();
                        }
                    }
                    body += decimals > 0 ? QStringLiteral("<number:seconds%1 number:decimal-places=\"%2\"/>").arg(style).arg(decimals) : QStringLiteral("<number:seconds%1/>").arg(style);
                    break;
                }
                default: break;
                }
            } else {
                body += part;
            }
        }
        out->body = body;
        return true;
    }

private:
    struct DateTok {
        char kind = 'y';
        bool longStyle = false;
        bool elapsed = false;
        int run = 1;
    };
    const QString &m_s;
    QVector<QString> m_parts;
    QVector<DateTok> m_dates;
    bool m_pendingWhole = false;
    int m_pendingWholeMin = 0;

    static QString numberPicture(const QString &picture)
    {
        const int dot = int(picture.indexOf(QLatin1Char('.')));
        const QString intPart = dot < 0 ? picture : picture.left(dot), decPart = dot < 0 ? QString() : picture.mid(dot + 1);
        int minInteger = 0, blank = 0;
        bool grouping = false;
        int scale = 0;
        // thousands: commas after the last digit scale by 1000 each; a comma between digits groups
        int lastDigit = -1;
        for (int i = 0; i < intPart.size(); ++i)
            if (intPart[i] != QLatin1Char(',')) lastDigit = i;
        for (int i = 0; i < intPart.size(); ++i) {
            if (intPart[i] == QLatin1Char(',')) {
                if (i < lastDigit) grouping = true;
                else ++scale;
            } else if (intPart[i] == QLatin1Char('0')) {
                ++minInteger;
            } else if (intPart[i] == QLatin1Char('?')) {
                ++minInteger;
                ++blank;
            }
        }
        int decimals = 0, minDecimals = 0;
        for (const QChar c : decPart) {
            if (c == QLatin1Char(',')) {
                ++scale;
                continue;
            }
            ++decimals;
            if (c == QLatin1Char('0')) minDecimals = decimals;
        }
        QString x = QStringLiteral("<number:number");
        if (dot >= 0) x += QStringLiteral(" number:decimal-places=\"%1\" number:min-decimal-places=\"%2\"").arg(decimals).arg(minDecimals);
        else x += QStringLiteral(" number:decimal-places=\"0\"");
        x += QStringLiteral(" number:min-integer-digits=\"%1\"").arg(minInteger);
        if (blank > 0) x += QStringLiteral(" loext:max-blank-integer-digits=\"%1\"").arg(blank);
        if (grouping) x += QStringLiteral(" number:grouping=\"true\"");
        if (scale > 0) {
            qint64 f = 1;
            for (int i = 0; i < scale; ++i) f *= 1000;
            x += QStringLiteral(" number:display-factor=\"%1\"").arg(f);
        }
        return x + QStringLiteral("/>");
    }

    static QString scientific(const QString &picture, int exponentDigits, bool forcedSign)
    {
        const int dot = int(picture.indexOf(QLatin1Char('.')));
        const QString intPart = dot < 0 ? picture : picture.left(dot), decPart = dot < 0 ? QString() : picture.mid(dot + 1);
        return QStringLiteral("<number:scientific-number number:decimal-places=\"%1\" number:min-integer-digits=\"%2\" number:min-exponent-digits=\"%3\"%4/>")
            .arg(decPart.size()).arg(intPart.count(QLatin1Char('0'))).arg(exponentDigits).arg(forcedSign ? QStringLiteral(" number:forced-exponent-sign=\"true\"") : QString());
    }
};

QString element(const Section &s, const QString &name, bool main, const QString &maps)
{
    QString kind = s.text && !s.percent ? QStringLiteral("text-style") : s.percent ? QStringLiteral("percentage-style") : (s.date || s.time) ? (s.date ? QStringLiteral("date-style") : QStringLiteral("time-style")) : QStringLiteral("number-style");
    QString x = QStringLiteral("<number:%1 style:name=\"%2\" style:family=\"data-style\"").arg(kind, name);
    if (s.elapsed) x += QStringLiteral(" number:truncate-on-overflow=\"false\"");
    x += QStringLiteral(">");
    if (!s.color.isEmpty()) x += QStringLiteral("<style:text-properties fo:color=\"%1\"/>").arg(s.color);
    x += s.body;
    if (main) x += maps;
    x += QStringLiteral("</number:%1>").arg(kind);
    return x;
}

} // namespace

bool odfDataStyles(const QString &code, const QString &baseName, OdfDataStyles *out)
{
    if (code.isEmpty() || code.compare(QLatin1String("General"), Qt::CaseInsensitive) == 0) {
        out->xml.clear();
        out->mainName.clear();
        return true;
    }
    if (code.size() > 1000) return false;
    // the sections, split at semicolons outside quotes and brackets
    QStringList raw;
    {
        QString cur;
        bool quote = false, bracket = false;
        for (int i = 0; i < code.size(); ++i) {
            const QChar c = code[i];
            if (c == QLatin1Char('\\') && i + 1 < code.size() && !quote) {
                cur += c;
                cur += code[++i];
                continue;
            }
            if (c == QLatin1Char('"')) quote = !quote;
            else if (!quote && c == QLatin1Char('[')) bracket = true;
            else if (!quote && c == QLatin1Char(']')) bracket = false;
            if (c == QLatin1Char(';') && !quote && !bracket) {
                raw.append(cur);
                cur.clear();
            } else {
                cur += c;
            }
        }
        raw.append(cur);
    }
    if (raw.size() > 4) return false;
    // a last section that is only "@" shows a text as it is: a data style for numbers has no place for it
    if (raw.size() >= 2 && raw.last().trimmed() == QLatin1String("@")) raw.removeLast();
    QVector<Section> sections;
    for (const QString &r : std::as_const(raw)) {
        Section s;
        SectionParser p(r);
        if (!p.parse(&s)) return false;
        if (s.body.isEmpty() && r.trimmed().isEmpty() && raw.size() > 1) {
            // (a section left empty shows nothing)
        } else if (s.body.isEmpty()) {
            return false;
        }
        sections.append(s);
    }
    QString xml;
    if (sections.size() == 1) {
        out->xml = element(sections[0], baseName, true, QString());
        out->mainName = baseName;
        return true;
    }
    // the sections other than the last are styles the last names with maps
    QString maps;
    const int n = int(sections.size());
    const bool textLast = n == 4;
    const int main = n == 2 ? 1 : n == 3 ? 2 : 3;
    int numbered = 0;
    auto addMap = [&](const QString &condition, int section) {
        const QString name = baseName + QStringLiteral("P%1").arg(numbered++);
        xml += element(sections[section], name, false, QString());
        maps += QStringLiteral("<style:map style:condition=\"%1\" style:apply-style-name=\"%2\"/>").arg(condition.toHtmlEscaped(), name);
    };
    const bool explicitConditions = !sections[0].condition.isEmpty() || (n > 1 && !sections[1].condition.isEmpty());
    if (explicitConditions) {
        for (int i = 0; i < n - 1; ++i) {
            if (sections[i].condition.isEmpty()) {
                // a section with no condition after one that has: it takes what is left
                return false;
            }
            addMap(sections[i].condition, i);
        }
    } else if (n == 2) {
        addMap(QStringLiteral("value()>=0"), 0);
    } else {
        addMap(QStringLiteral("value()>0"), 0);
        addMap(QStringLiteral("value()<0"), 1);
        if (textLast) addMap(QStringLiteral("value()=0"), 2);
    }
    xml += element(sections[main], baseName, true, maps);
    out->xml = xml;
    out->mainName = baseName;
    return true;
}

} // namespace jp::office::sheet
