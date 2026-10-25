#include "sheet/headerfooter.h"

#include <algorithm>

namespace jp::office::sheet {

namespace {

bool isHex(QChar c) { return (c >= QLatin1Char('0') && c <= QLatin1Char('9')) || (c >= QLatin1Char('a') && c <= QLatin1Char('f')) || (c >= QLatin1Char('A') && c <= QLatin1Char('F')); }
bool isDigit(QChar c) { return c >= QLatin1Char('0') && c <= QLatin1Char('9'); }

// What the parse holds while it goes along: the part and line being filled, the text waiting for its run, and the font the codes have set.
class Parser {
public:
    explicit Parser(HfText *out) : m_out(out), m_part(&out->center) {}

    void run(const QString &code)
    {
        const QString s = code.left(kMaxHeaderFooterChars);
        int codes = 0;
        for (int i = 0; i < s.size();) {
            const QChar c = s[i];
            if (c == QLatin1Char('\r')) {
                ++i;
            } else if (c == QLatin1Char('\n')) {
                flush();
                newLine();
                ++i;
            } else if (c != QLatin1Char('&')) {
                m_pending += c;
                ++i;
            } else {
                if (++codes > kMaxHeaderFooterCodes) break;
                i = code_(s, i);
            }
        }
        flush();
    }

private:
    // Handles the code at s[at] (an &) and gives the place after it.
    int code_(const QString &s, int at)
    {
        if (at + 1 >= s.size()) return s.size();   // (an & at the end)
        const QChar c = s[at + 1];
        switch (c.unicode()) {
        case '&': m_pending += QLatin1Char('&'); return at + 2;
        case 'L': return section(&m_out->left, at + 2);
        case 'C': return section(&m_out->center, at + 2);
        case 'R': return section(&m_out->right, at + 2);
        case 'P': return field(HfField::Page, at + 2);
        case 'N': return field(HfField::Pages, at + 2);
        case 'D': return field(HfField::Date, at + 2);
        case 'T': return field(HfField::Time, at + 2);
        case 'F': return field(HfField::File, at + 2);
        case 'Z': return field(HfField::Path, at + 2);
        case 'A': return field(HfField::Sheet, at + 2);
        case 'G':
            m_out->hasPicture = true;
            return field(HfField::Picture, at + 2);   // (a run of its own, which draws nothing yet)
        case 'B': flush(); m_fmt.bold = !m_fmt.bold; return at + 2;
        case 'I': flush(); m_fmt.italic = !m_fmt.italic; return at + 2;
        case 'U': flush(); m_fmt.underline = m_fmt.underline == HfUnderline::Single ? HfUnderline::None : HfUnderline::Single; return at + 2;
        case 'E': flush(); m_fmt.underline = m_fmt.underline == HfUnderline::Double ? HfUnderline::None : HfUnderline::Double; return at + 2;
        case 'S': flush(); m_fmt.strike = !m_fmt.strike; return at + 2;
        case 'X': flush(); m_fmt.superscript = !m_fmt.superscript; m_fmt.subscript = false; return at + 2;
        case 'Y': flush(); m_fmt.subscript = !m_fmt.subscript; m_fmt.superscript = false; return at + 2;
        case 'O':
        case 'H': return at + 2;   // (outline and shadow: not drawn)
        case '"': return fontCode(s, at + 2);
        case 'K': return colorCode(s, at + 2);
        default: break;
        }
        if (isDigit(c)) {
            flush();
            int i = at + 1, n = 0, digits = 0;
            while (i < s.size() && isDigit(s[i]) && digits < 3) {
                n = n * 10 + (s[i].unicode() - '0');
                ++i;
                ++digits;
            }
            m_fmt.size = std::min(n, 409);
            return i;
        }
        // a letter that is no code is passed over with its &; any other character stays as the text it is
        return c.isLetter() ? at + 2 : at + 1;
    }

    int section(HfPart *part, int next)
    {
        flush();
        m_part = part;
        m_fmt = HfRun();
        m_line = -1;
        // (a part that is named again goes on at its last line)
        if (!part->lines.isEmpty()) m_line = int(part->lines.size()) - 1;
        return next;
    }

    int field(HfField f, int next)
    {
        flush();
        HfRun r = m_fmt;
        r.field = f;
        add(r);
        return next;
    }

    // &"Arial,Bold Italic": the family, and the style (which says bold and italic both: a style without them is neither).
    int fontCode(const QString &s, int from)
    {
        flush();
        const int end = s.indexOf(QLatin1Char('"'), from);
        const QString spec = end < 0 ? s.mid(from) : s.mid(from, end - from);
        const int comma = spec.indexOf(QLatin1Char(','));
        const QString family = (comma < 0 ? spec : spec.left(comma)).trimmed();
        const QString style = comma < 0 ? QString() : spec.mid(comma + 1);
        m_fmt.font = family == QLatin1String("-") ? QString() : family.left(64);
        if (comma >= 0) {
            m_fmt.bold = style.contains(QLatin1String("bold"), Qt::CaseInsensitive);
            m_fmt.italic = style.contains(QLatin1String("italic"), Qt::CaseInsensitive);
        }
        return end < 0 ? s.size() : end + 1;
    }

    int colorCode(const QString &s, int from)
    {
        flush();
        if (from + 6 <= s.size()) {
            bool hex = true;
            for (int k = 0; k < 6; ++k) hex = hex && isHex(s[from + k]);
            if (hex) {
                bool ok = false;
                const uint rgb = s.mid(from, 6).toUInt(&ok, 16);
                m_fmt.hasColor = ok;
                m_fmt.color = QColor((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
                return from + 6;
            }
            // a color of the theme, "01+034" (its number, a sign, and its tint): the page takes the text's own color
            if (isDigit(s[from]) && isDigit(s[from + 1]) && (s[from + 2] == QLatin1Char('+') || s[from + 2] == QLatin1Char('-')) && isDigit(s[from + 3]) && isDigit(s[from + 4]) && isDigit(s[from + 5])) {
                m_fmt.hasColor = false;
                return from + 6;
            }
        }
        return from;
    }

    void flush()
    {
        if (m_pending.isEmpty()) return;
        HfRun r = m_fmt;
        r.text = m_pending;
        m_pending.clear();
        add(r);
    }

    void newLine()
    {
        if (m_part->lines.size() >= kMaxHeaderFooterLines) {
            m_full = true;
            return;
        }
        // (a part that has no line yet and is given a line break first has an empty first line: "\nB" is B on the second line)
        if (m_line < 0 && m_part->lines.isEmpty()) m_part->lines.append(HfLine());
        m_part->lines.append(HfLine());
        m_line = int(m_part->lines.size()) - 1;
        m_full = false;
    }

    void add(const HfRun &run)
    {
        if (m_full) return;
        if (m_line < 0 || m_line >= m_part->lines.size()) {
            if (m_part->lines.size() >= kMaxHeaderFooterLines) return;
            m_part->lines.append(HfLine());
            m_line = int(m_part->lines.size()) - 1;
        }
        // (the runs of a part are bounded together)
        qsizetype runs = 0;
        for (const HfLine &l : std::as_const(m_part->lines)) runs += l.size();
        if (runs >= kMaxHeaderFooterRuns) return;
        m_part->lines[m_line].append(run);
    }

    HfText *m_out;
    HfPart *m_part;
    int m_line = -1;
    bool m_full = false;
    QString m_pending;
    HfRun m_fmt;
};

} // namespace

HfText parseHeaderFooter(const QString &code)
{
    HfText out;
    Parser(&out).run(code);
    return out;
}

namespace {

// The date in the computer's short form, with the year in full (the short form of many locales has two digits; a printed date has four).
QString shortDateText(const QLocale &locale, const QDate &date)
{
    QString format = locale.dateFormat(QLocale::ShortFormat);
    if (!format.contains(QLatin1String("yyyy"))) format.replace(QLatin1String("yy"), QLatin1String("yyyy"));
    return locale.toString(date, format);
}

} // namespace

QString hfFieldText(HfField field, const HfContext &c)
{
    switch (field) {
    case HfField::Page: return QString::number(c.page);
    case HfField::Pages: return QString::number(c.pages);
    case HfField::Date: return shortDateText(c.locale, c.now.date());
    case HfField::Time: return c.locale.toString(c.now.time(), QLocale::ShortFormat);
    case HfField::File: return c.file;
    case HfField::Path: return c.path;
    case HfField::Sheet: return c.sheet;
    default: return QString();
    }
}

QString escapeHeaderFooterText(const QString &plain)
{
    QString out = plain;
    out.replace(QLatin1Char('&'), QLatin1String("&&"));
    return out;
}

QString composeHeaderFooter(const QString &left, const QString &center, const QString &right)
{
    QString out;
    if (!left.isEmpty()) out += QLatin1String("&L") + left;
    if (!center.isEmpty()) out += QLatin1String("&C") + center;
    if (!right.isEmpty()) out += QLatin1String("&R") + right;
    return out;
}

void splitHeaderFooter(const QString &code, QString *left, QString *center, QString *right)
{
    left->clear();
    center->clear();
    right->clear();
    QString *into = center;
    for (int i = 0; i < code.size(); ++i) {
        const QChar c = code[i];
        if (c == QLatin1Char('&') && i + 1 < code.size()) {
            const QChar n = code[i + 1];
            if (n == QLatin1Char('L') || n == QLatin1Char('C') || n == QLatin1Char('R')) {
                into = n == QLatin1Char('L') ? left : n == QLatin1Char('C') ? center : right;
                ++i;
                continue;
            }
            // (a code of another kind, or && for an ampersand, stays as it is in the part)
            *into += c;
            *into += n;
            ++i;
            continue;
        }
        *into += c;
    }
}

bool headerFooterHasPicture(const QString &code, QChar part)
{
    const HfText t = parseHeaderFooter(code);
    const HfPart &p = part == QLatin1Char('L') ? t.left : part == QLatin1Char('C') ? t.center : t.right;
    for (const HfLine &line : p.lines)
        for (const HfRun &run : line)
            if (run.field == HfField::Picture) return true;
    return false;
}

void dropUnusedHeaderPictures(HeaderFooter *hf)
{
    for (qsizetype i = hf->pictures.size(); i-- > 0;) {
        const QString place = canonicalHeaderPlace(hf->pictures[i].place);
        if (place.size() < 2) continue;
        const QChar part = place[0], kind = place[1];
        if ((part != QLatin1Char('L') && part != QLatin1Char('C') && part != QLatin1Char('R')) || (kind != QLatin1Char('H') && kind != QLatin1Char('F'))) continue;
        const QString form = place.mid(2);
        if (!form.isEmpty() && form != QLatin1String("EVEN") && form != QLatin1String("FIRST")) continue;
        const bool header = kind == QLatin1Char('H');
        const QString &text = form == QLatin1String("FIRST") ? (header ? hf->firstHeader : hf->firstFooter) : form == QLatin1String("EVEN") ? (header ? hf->evenHeader : hf->evenFooter) : (header ? hf->oddHeader : hf->oddFooter);
        if (!headerFooterHasPicture(text, part)) hf->pictures.removeAt(i);
    }
}

QString headerFooterPreview(const QString &code, const HfContext &context)
{
    const HfText t = parseHeaderFooter(code);
    QStringList parts;
    for (const HfPart *p : {&t.left, &t.center, &t.right}) {
        QStringList lines;
        for (const HfLine &line : p->lines) {
            QString text;
            for (const HfRun &r : line) text += r.field == HfField::None ? r.text : hfFieldText(r.field, context);
            lines << text;
        }
        const QString joined = lines.join(QLatin1Char(' '));
        if (!joined.isEmpty()) parts << joined;
    }
    return parts.join(QLatin1String(" | "));
}

} // namespace jp::office::sheet
