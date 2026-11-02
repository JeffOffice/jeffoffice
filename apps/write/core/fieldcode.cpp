#include "core/fieldcode.h"

#include <QCoreApplication>
#include <QRegularExpression>
#include <QSet>
#include <algorithm>
#include <cmath>
#include <limits>

namespace jp::write::fields {

namespace {

constexpr qint64 kMostSequence = 2147483647;

bool isSwitchToken(const QString &t) { return t.size() >= 2 && t[0] == QLatin1Char('\\'); }

// Whether the switch of a field takes the word after it.
bool takesArgument(const QString &name, QChar letter)
{
    const QChar l = letter.toLower();
    if (letter == QLatin1Char('*') || letter == QLatin1Char('#') || letter == QLatin1Char('@')) return true;
    if (name == QLatin1String("TOC")) return QStringLiteral("otbcdflspa").contains(l);
    if (name == QLatin1String("SEQ")) return l == QLatin1Char('r') || l == QLatin1Char('s');
    if (name == QLatin1String("REF")) return l == QLatin1Char('d');
    if (name == QLatin1String("HYPERLINK")) return l == QLatin1Char('l') || l == QLatin1Char('o') || l == QLatin1Char('t');
    return false;
}

QString toUpperAscii(const QString &s) { return s.toUpper(); }

const char *const kOnes[] = {"zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine", "ten", "eleven", "twelve", "thirteen", "fourteen", "fifteen", "sixteen", "seventeen", "eighteen", "nineteen"};
const char *const kTens[] = {"", "", "twenty", "thirty", "forty", "fifty", "sixty", "seventy", "eighty", "ninety"};

QString belowThousand(int n)
{
    QStringList parts;
    if (n >= 100) {
        parts << QLatin1String(kOnes[n / 100]) + QLatin1String(" hundred");
        n %= 100;
    }
    if (n >= 20) parts << (n % 10 ? QLatin1String(kTens[n / 10]) + QLatin1Char('-') + QLatin1String(kOnes[n % 10]) : QLatin1String(kTens[n / 10]));
    else if (n > 0) parts << QLatin1String(kOnes[n]);
    return parts.join(QLatin1Char(' '));
}

QString cardinal(qint64 n)
{
    if (n == 0) return QStringLiteral("zero");
    static const char *const scales[] = {"", " thousand", " million", " billion"};
    QStringList parts;
    for (int scale = 0; n > 0; ++scale, n /= 1000)
        if (const int group = int(n % 1000)) parts.prepend(belowThousand(group) + QLatin1String(scales[scale]));
    return parts.join(QLatin1Char(' '));
}

QString ordinalWords(qint64 n)
{
    QString words = cardinal(n);
    // The last word becomes its ordinal.
    const int cut = std::max(words.lastIndexOf(QLatin1Char(' ')), words.lastIndexOf(QLatin1Char('-')));
    const QString head = words.left(cut + 1), last = words.mid(cut + 1);
    static const QHash<QString, QString> irregular = {{QStringLiteral("one"), QStringLiteral("first")}, {QStringLiteral("two"), QStringLiteral("second")}, {QStringLiteral("three"), QStringLiteral("third")},
                                                      {QStringLiteral("five"), QStringLiteral("fifth")}, {QStringLiteral("eight"), QStringLiteral("eighth")}, {QStringLiteral("nine"), QStringLiteral("ninth")},
                                                      {QStringLiteral("twelve"), QStringLiteral("twelfth")}, {QStringLiteral("zero"), QStringLiteral("zeroth")}};
    QString ordinal;
    if (irregular.contains(last)) ordinal = irregular.value(last);
    else if (last.endsWith(QLatin1Char('y'))) ordinal = last.chopped(1) + QStringLiteral("ieth");
    else ordinal = last + QStringLiteral("th");
    return head + ordinal;
}

QString romanOf(qint64 n)
{
    static const struct {
        int value;
        const char *text;
    } table[] = {{1000, "M"}, {900, "CM"}, {500, "D"}, {400, "CD"}, {100, "C"}, {90, "XC"}, {50, "L"}, {40, "XL"}, {10, "X"}, {9, "IX"}, {5, "V"}, {4, "IV"}, {1, "I"}};
    QString out;
    for (const auto &row : table)
        while (n >= row.value) {
            out += QLatin1String(row.text);
            n -= row.value;
        }
    return out;
}

bool isNumberFormat(const QString &g)
{
    static const QStringList names = {QStringLiteral("roman"), QStringLiteral("alphabetic"), QStringLiteral("arabic"), QStringLiteral("ordinal"), QStringLiteral("cardtext"), QStringLiteral("ordtext"), QStringLiteral("hex"), QStringLiteral("arabicdash")};
    return names.contains(g.toLower());
}

// Splits a code into words: a quoted piece is one word (without its quotes), a backslash outside quotes begins a word.
struct Word {
    QString text;
    bool quoted = false;
};

QVector<Word> wordsOf(const QString &text)
{
    QVector<Word> out;
    int i = 0;
    const int n = int(text.size());
    while (i < n && out.size() < kMostCodeWords) {
        if (text[i].isSpace()) {
            ++i;
            continue;
        }
        Word w;
        if (text[i] == QLatin1Char('"') || text[i] == QChar(0x201C)) {
            const QChar close = text[i] == QLatin1Char('"') ? QLatin1Char('"') : QChar(0x201D);
            ++i;
            while (i < n && text[i] != close && text[i] != QLatin1Char('"')) {
                // (A backslash and a quote inside quotes: the quote is a character of the word.)
                if (text[i] == QLatin1Char('\\') && i + 1 < n && (text[i + 1] == QLatin1Char('"'))) ++i;
                w.text += text[i++];
            }
            if (i < n) ++i;
            w.quoted = true;
        } else {
            if (text[i] == QLatin1Char('\\')) {
                w.text += text[i++];
                if (i < n && !text[i].isSpace()) w.text += text[i++];   // (the letter of the switch)
                // (A switch run onto its word, \*MERGEFORMAT.)
                if (w.text.size() == 2 && (w.text[1] == QLatin1Char('*') || w.text[1] == QLatin1Char('#') || w.text[1] == QLatin1Char('@')))
                    while (i < n && !text[i].isSpace() && text[i] != QLatin1Char('\\') && text[i] != QLatin1Char('"')) w.text += text[i++];
            } else {
                while (i < n && !text[i].isSpace() && text[i] != QLatin1Char('\\') && text[i] != QLatin1Char('"')) w.text += text[i++];
            }
        }
        out.push_back(w);
    }
    return out;
}

} // namespace

bool Code::has(QChar letter) const
{
    for (const Switch &s : switches)
        if (s.letter.toLower() == letter.toLower()) return true;
    return false;
}

QString Code::argumentOf(QChar letter) const
{
    for (const Switch &s : switches)
        if (s.letter.toLower() == letter.toLower()) return s.argument;
    return QString();
}

bool Code::keepsFormat() const
{
    for (const QString &g : general)
        if (g.compare(QLatin1String("MERGEFORMAT"), Qt::CaseInsensitive) == 0 || g.compare(QLatin1String("CHARFORMAT"), Qt::CaseInsensitive) == 0) return true;
    return false;
}

Code parseCode(const QString &instruction)
{
    Code code;
    QString text = instruction.left(kMostCodeChars).trimmed();
    if (text.isEmpty()) return code;
    int at = 0;
    if (text[0] == QLatin1Char('=')) {
        code.name = QStringLiteral("=");
        at = 1;
    } else {
        while (at < text.size() && !text[at].isSpace() && text[at] != QLatin1Char('\\') && text[at] != QLatin1Char('"')) ++at;
        code.name = text.left(at).toUpper();
    }
    const QString rest = text.mid(at);
    // The expression: up to the first switch outside quotes.
    {
        bool quoted = false;
        int cut = int(rest.size());
        for (int i = 0; i < rest.size(); ++i) {
            if (rest[i] == QLatin1Char('"')) quoted = !quoted;
            else if (!quoted && rest[i] == QLatin1Char('\\')) {
                cut = i;
                break;
            }
        }
        code.expression = rest.left(cut).trimmed();
    }
    const QVector<Word> words = wordsOf(rest);
    for (int i = 0; i < words.size(); ++i) {
        const Word &w = words[i];
        if (w.quoted || !isSwitchToken(w.text)) {
            code.arguments << w.text;
            continue;
        }
        const QChar letter = w.text[1];
        Switch sw;
        sw.letter = letter;
        if (takesArgument(code.name, letter)) {
            if (w.text.size() > 2) sw.argument = w.text.mid(2);
            else if (i + 1 < words.size() && (words[i + 1].quoted || !isSwitchToken(words[i + 1].text))) sw.argument = words[++i].text;
        } else if (code.name == QLatin1String("TOC") && letter.toLower() == QLatin1Char('n') && i + 1 < words.size() && !words[i + 1].quoted && !isSwitchToken(words[i + 1].text) &&
                   !words[i + 1].text.isEmpty() && words[i + 1].text[0].isDigit()) {
            sw.argument = words[++i].text;   // (\n 2-3: the levels that show no page numbers)
        }
        if (letter == QLatin1Char('*')) code.general << sw.argument;
        else if (letter == QLatin1Char('#')) {
            code.numberPicture = sw.argument.left(kMostPictureChars);
            code.hasNumberPicture = true;
        } else if (letter == QLatin1Char('@')) {
            code.datePicture = sw.argument.left(kMostPictureChars);
            code.hasDatePicture = true;
        }
        if (code.switches.size() < kMostCodeWords) code.switches.push_back(sw);
    }
    return code;
}

// ---- formats

QString formatText(const QString &text, const QStringList &general)
{
    for (const QString &g : general) {
        if (g.compare(QLatin1String("Upper"), Qt::CaseInsensitive) == 0) return text.toUpper();
        if (g.compare(QLatin1String("Lower"), Qt::CaseInsensitive) == 0) return text.toLower();
        if (g.compare(QLatin1String("FirstCap"), Qt::CaseInsensitive) == 0) {
            QString t = text.toLower();
            for (int i = 0; i < t.size(); ++i)
                if (t[i].isLetter()) {
                    t[i] = t[i].toUpper();
                    break;
                }
            return t;
        }
        if (g.compare(QLatin1String("Caps"), Qt::CaseInsensitive) == 0) {
            QString t = text.toLower();
            bool start = true;
            for (int i = 0; i < t.size(); ++i) {
                if (start && t[i].isLetter()) t[i] = t[i].toUpper();
                start = !t[i].isLetterOrNumber() && t[i] != QLatin1Char('\'');
            }
            return t;
        }
    }
    return text;
}

QString formatNumber(qint64 n, const QStringList &general)
{
    QString out = QString::number(n);
    for (const QString &g : general) {
        if (!isNumberFormat(g)) continue;
        const QString l = g.toLower();
        if (l == QLatin1String("roman")) {
            if (n >= 1 && n <= 3999) out = g == QLatin1String("ROMAN") ? romanOf(n) : romanOf(n).toLower();
        } else if (l == QLatin1String("alphabetic")) {
            if (n >= 1 && n <= 26 * 40) {
                const QChar letter(char16_t((g == QLatin1String("ALPHABETIC") ? 'A' : 'a') + (n - 1) % 26));
                out = QString(int((n - 1) / 26 + 1), letter);
            }
        } else if (l == QLatin1String("ordinal")) {
            const qint64 a = n < 0 ? -n : n;
            const int last = int(a % 100) >= 11 && int(a % 100) <= 13 ? 0 : int(a % 10);
            out = QString::number(n) + (last == 1 ? QStringLiteral("st") : last == 2 ? QStringLiteral("nd") : last == 3 ? QStringLiteral("rd") : QStringLiteral("th"));
        } else if (l == QLatin1String("cardtext")) {
            if (n >= 0 && n < 1000000000000LL) out = cardinal(n);
        } else if (l == QLatin1String("ordtext")) {
            if (n >= 0 && n < 1000000000000LL) out = ordinalWords(n);
        } else if (l == QLatin1String("hex")) {
            if (n >= 0) out = QString::number(n, 16).toUpper();
        } else if (l == QLatin1String("arabicdash")) {
            out = QStringLiteral("- %1 -").arg(n);
        }
        break;
    }
    return formatText(out, general);
}

QString plainNumber(double value)
{
    if (!std::isfinite(value)) return QString();
    QString s = QString::number(value, 'f', 6);
    if (s.contains(QLatin1Char('.'))) {
        while (s.endsWith(QLatin1Char('0'))) s.chop(1);
        if (s.endsWith(QLatin1Char('.'))) s.chop(1);
    }
    if (s == QLatin1String("-0")) s = QStringLiteral("0");
    return s;
}

QString numberInPicture(double value, const QString &pictureIn)
{
    if (!std::isfinite(value)) return QStringLiteral("0");
    const QString picture = pictureIn.left(kMostPictureChars);
    // The pictures for positive, negative, and zero (apart at semicolons outside quotes).
    QStringList sections;
    {
        QString cur;
        bool quoted = false;
        for (const QChar c : picture) {
            if (c == QLatin1Char('\'')) quoted = !quoted;
            if (c == QLatin1Char(';') && !quoted) {
                sections << cur;
                cur.clear();
            } else {
                cur += c;
            }
        }
        sections << cur;
    }
    const bool negative = value < 0;
    QString pic = sections[0];
    bool ownSign = false;
    if (negative && sections.size() > 1) {
        pic = sections[1];
        ownSign = true;
    } else if (value == 0 && sections.size() > 2) {
        pic = sections[2];
    }
    // The pieces: literal text, digit places, the point.
    struct Piece {
        enum Kind { Literal, Zero, Hash, Drop, Group, Point, Minus, Plus } kind;
        QString text;
    };
    QVector<Piece> pieces;
    {
        bool quoted = false;
        QString lit;
        auto flush = [&] {
            if (!lit.isEmpty()) pieces.push_back({Piece::Literal, lit});
            lit.clear();
        };
        for (const QChar c : pic) {
            if (c == QLatin1Char('\'')) {
                quoted = !quoted;
                continue;
            }
            if (quoted) {
                lit += c;
                continue;
            }
            Piece::Kind kind = Piece::Literal;
            if (c == QLatin1Char('0')) kind = Piece::Zero;
            else if (c == QLatin1Char('#')) kind = Piece::Hash;
            else if (c == QLatin1Char('x') || c == QLatin1Char('X')) kind = Piece::Drop;
            else if (c == QLatin1Char(',')) kind = Piece::Group;
            else if (c == QLatin1Char('.')) kind = Piece::Point;
            else if (c == QLatin1Char('-')) kind = Piece::Minus;
            else if (c == QLatin1Char('+')) kind = Piece::Plus;
            if (kind == Piece::Literal) {
                lit += c;
                continue;
            }
            flush();
            pieces.push_back({kind, QString(c)});
        }
        flush();
    }
    int point = -1;
    for (int i = 0; i < pieces.size(); ++i)
        if (pieces[i].kind == Piece::Point) {
            point = i;
            break;
        }
    int fraction = 0;
    if (point >= 0)
        for (int i = point + 1; i < pieces.size(); ++i)
            if (pieces[i].kind == Piece::Zero || pieces[i].kind == Piece::Hash || pieces[i].kind == Piece::Drop) ++fraction;
    fraction = std::min(fraction, 15);
    // The digits: rounded to the places of the fraction (half away from zero).
    const double a = std::fabs(value);
    QString digits = QString::number(a, 'f', fraction);
    // (QString::number rounds half to even on the binary value; a value like 2.5 to 0 places is 3 here.)
    {
        const double scale = std::pow(10.0, fraction);
        if (a < 1e15 && fraction >= 0) {
            const double r = std::round(a * scale) / scale;
            digits = QString::number(r, 'f', fraction);
        }
    }
    QString whole = digits.section(QLatin1Char('.'), 0, 0), frac = digits.section(QLatin1Char('.'), 1);
    if (a == 0 && value != 0) whole = QStringLiteral("0");
    const bool zeroResult = digits.toDouble() == 0;
    const bool groups = std::any_of(pieces.begin(), pieces.end(), [](const Piece &p) { return p.kind == Piece::Group; });
    const int limit = point >= 0 ? point : int(pieces.size());
    // The whole part, from the right.
    QString left;
    int next = int(whole.size()) - 1, taken = 0;
    bool dropping = false;
    auto putDigit = [&](QChar d) {
        if (groups && taken > 0 && taken % 3 == 0) left.prepend(QLatin1Char(','));
        left.prepend(d);
        ++taken;
    };
    int firstPlace = -1;
    for (int i = 0; i < limit; ++i)
        if (pieces[i].kind == Piece::Zero || pieces[i].kind == Piece::Hash || pieces[i].kind == Piece::Drop) {
            firstPlace = i;
            break;
        }
    for (int i = limit - 1; i >= 0; --i) {
        const Piece &p = pieces[i];
        switch (p.kind) {
        case Piece::Zero:
        case Piece::Hash:
            if (dropping) break;
            if (next >= 0) {
                putDigit(whole[next--]);
                // (The digits that no place is left for are shown before the first place.)
                if (i == firstPlace)
                    while (next >= 0) putDigit(whole[next--]);
            } else if (p.kind == Piece::Zero) {
                putDigit(QLatin1Char('0'));
            } else if (!groups) {
                left.prepend(QLatin1Char(' '));   // (a place with no digit is a blank, unless the picture groups digits: its leading places then show nothing)
            }
            break;
        case Piece::Drop:
            // (A place of its own that shows its digit, and drops the digits to its left.)
            if (!dropping) {
                putDigit(next >= 0 ? whole[next] : QLatin1Char('0'));
                next = -1;
            }
            dropping = true;
            break;
        case Piece::Minus: left.prepend(negative && !ownSign ? QLatin1Char('-') : QLatin1Char(' ')); break;
        case Piece::Plus: left.prepend(zeroResult ? QLatin1Char(' ') : negative && !ownSign ? QLatin1Char('-') : QLatin1Char('+')); break;
        case Piece::Literal: left.prepend(p.text); break;
        case Piece::Group:
        case Piece::Point: break;
        }
    }
    QString right;
    if (point >= 0) {
        int di = 0;
        bool anyDigit = false, onlyHash = true;
        QString tail;
        for (int i = point + 1; i < pieces.size(); ++i) {
            const Piece &p = pieces[i];
            switch (p.kind) {
            case Piece::Zero:
            case Piece::Hash:
            case Piece::Drop: {
                const QChar d = di < frac.size() ? frac[di] : QLatin1Char('0');
                ++di;
                if (p.kind == Piece::Zero) {
                    tail += d;
                    anyDigit = true;
                    onlyHash = false;
                } else {
                    bool more = false;
                    for (int k = di - 1; k < frac.size(); ++k) more = more || frac[k] != QLatin1Char('0');
                    if (more) {
                        tail += d;
                        anyDigit = true;
                    }
                }
                break;
            }
            case Piece::Minus: tail += negative && !ownSign ? QLatin1Char('-') : QLatin1Char(' '); break;
            case Piece::Plus: tail += zeroResult ? QLatin1Char(' ') : negative && !ownSign ? QLatin1Char('-') : QLatin1Char('+'); break;
            case Piece::Literal: tail += p.text; break;
            default: break;
            }
        }
        (void)onlyHash;
        right = (anyDigit || fraction == 0 ? QStringLiteral(".") : QString()) + tail;
        if (fraction == 0) right = tail;   // (a point with no places after it shows no point)
    }
    QString out = left + right;
    // A negative number whose picture has no sign of its own shows a minus before it.
    const bool hasSign = std::any_of(pieces.begin(), pieces.end(), [](const Piece &p) { return p.kind == Piece::Minus || p.kind == Piece::Plus; });
    if (negative && !ownSign && !hasSign && !zeroResult) out.prepend(QLatin1Char('-'));
    return out.left(2 * kMostPictureChars + 400);
}

QString dateInPicture(const QDateTime &when, const QString &pictureIn)
{
    static const char *const months[] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
    static const char *const days[] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
    const QString picture = pictureIn.left(kMostPictureChars);
    const QDate date = when.date();
    const QTime time = when.time();
    QString out;
    int i = 0;
    const int n = int(picture.size());
    auto run = [&](QChar c) {
        int k = 0;
        while (i + k < n && picture[i + k] == c) ++k;
        return k;
    };
    while (i < n && out.size() < 4 * kMostPictureChars) {
        const QChar c = picture[i];
        if (c == QLatin1Char('\'')) {
            ++i;
            while (i < n && picture[i] != QLatin1Char('\'')) out += picture[i++];
            ++i;
        } else if (c == QLatin1Char('\\') && i + 1 < n) {
            out += picture[i + 1];
            i += 2;
        } else if (c == QLatin1Char('M')) {
            const int k = run(c);
            const int m = date.month();
            out += k == 1 ? QString::number(m) : k == 2 ? QStringLiteral("%1").arg(m, 2, 10, QLatin1Char('0')) : k == 3 ? QLatin1String(months[m - 1]).left(3) : QLatin1String(months[m - 1]);
            i += k;
        } else if (c == QLatin1Char('d')) {
            const int k = run(c);
            const int d = date.day(), w = date.dayOfWeek();
            out += k == 1 ? QString::number(d) : k == 2 ? QStringLiteral("%1").arg(d, 2, 10, QLatin1Char('0')) : k == 3 ? QLatin1String(days[w - 1]).left(3) : QLatin1String(days[w - 1]);
            i += k;
        } else if (c == QLatin1Char('y')) {
            const int k = run(c);
            out += k <= 2 ? QStringLiteral("%1").arg(date.year() % 100, 2, 10, QLatin1Char('0')) : QStringLiteral("%1").arg(date.year(), 4, 10, QLatin1Char('0'));
            i += k;
        } else if (c == QLatin1Char('h') || c == QLatin1Char('H')) {
            const int k = run(c);
            int h = time.hour();
            if (c == QLatin1Char('h')) h = h % 12 == 0 ? 12 : h % 12;
            out += k == 1 ? QString::number(h) : QStringLiteral("%1").arg(h, 2, 10, QLatin1Char('0'));
            i += k;
        } else if (c == QLatin1Char('m')) {
            const int k = run(c);
            out += k == 1 ? QString::number(time.minute()) : QStringLiteral("%1").arg(time.minute(), 2, 10, QLatin1Char('0'));
            i += k;
        } else if (c == QLatin1Char('s')) {
            const int k = run(c);
            out += k == 1 ? QString::number(time.second()) : QStringLiteral("%1").arg(time.second(), 2, 10, QLatin1Char('0'));
            i += k;
        } else if (picture.mid(i, 5).compare(QLatin1String("AM/PM"), Qt::CaseSensitive) == 0 || picture.mid(i, 5).compare(QLatin1String("am/pm"), Qt::CaseSensitive) == 0) {
            const bool pm = time.hour() >= 12;
            out += picture[i] == QLatin1Char('A') ? (pm ? QStringLiteral("PM") : QStringLiteral("AM")) : (pm ? QStringLiteral("pm") : QStringLiteral("am"));
            i += 5;
        } else if (picture.mid(i, 3) == QLatin1String("A/P") || picture.mid(i, 3) == QLatin1String("a/p")) {
            const bool pm = time.hour() >= 12;
            out += picture[i] == QLatin1Char('A') ? (pm ? QStringLiteral("P") : QStringLiteral("A")) : (pm ? QStringLiteral("p") : QStringLiteral("a"));
            i += 3;
        } else {
            out += c;
            ++i;
        }
    }
    return out;
}

// ---- formulas

namespace {

struct Parser {
    QString text;
    int at = 0;
    int depth = 0;
    int steps = 0;
    bool failed = false;
    const std::function<std::optional<double>(const QString &)> *names = nullptr;

    void skip()
    {
        while (at < text.size() && text[at].isSpace()) ++at;
    }
    bool take(QChar c)
    {
        skip();
        if (at < text.size() && text[at] == c) {
            ++at;
            return true;
        }
        return false;
    }
    bool takeText(const QString &s)
    {
        skip();
        if (text.mid(at, s.size()) == s) {
            at += int(s.size());
            return true;
        }
        return false;
    }
    bool tick() { return ++steps > 20000 ? (failed = true, false) : true; }

    double comparison()
    {
        const double left = sum();
        if (failed) return 0;
        skip();
        auto cmp = [&](const char *op, auto test) {
            if (!takeText(QLatin1String(op))) return false;
            const double right = sum();
            lastCompare = test(left, right) ? 1.0 : 0.0;
            return true;
        };
        if (cmp("<>", [](double a, double b) { return a != b; }) || cmp("<=", [](double a, double b) { return a <= b; }) || cmp(">=", [](double a, double b) { return a >= b; }) ||
            cmp("<", [](double a, double b) { return a < b; }) || cmp(">", [](double a, double b) { return a > b; }) || cmp("=", [](double a, double b) { return a == b; }))
            return lastCompare;
        return left;
    }
    double lastCompare = 0;

    double expression()
    {
        if (++depth > kMostFormulaDepth) {
            failed = true;
            --depth;
            return 0;
        }
        const double v = comparison();
        --depth;
        return v;
    }
    double sum()
    {
        double v = product();
        while (!failed) {
            if (!tick()) return 0;
            if (take(QLatin1Char('+'))) v += product();
            else if (take(QLatin1Char('-'))) v -= product();
            else break;
        }
        return v;
    }
    double product()
    {
        double v = unary();
        while (!failed) {
            if (!tick()) return 0;
            if (take(QLatin1Char('*'))) v *= unary();
            else if (take(QLatin1Char('/'))) {
                const double d = unary();
                if (d == 0) {
                    failed = true;
                    return 0;
                }
                v /= d;
            } else break;
        }
        return v;
    }
    double unary()
    {
        if (++depth > kMostFormulaDepth) {
            failed = true;
            --depth;
            return 0;
        }
        double v;
        if (take(QLatin1Char('-'))) v = -unary();
        else if (take(QLatin1Char('+'))) v = unary();
        else {
            v = postfix();
            if (!failed && take(QLatin1Char('^'))) {
                const double e = unary();
                v = std::pow(v, e);
            }
        }
        --depth;
        return v;
    }
    double postfix()
    {
        double v = primary();
        while (!failed && take(QLatin1Char('%'))) v /= 100.0;
        return v;
    }
    QVector<double> arguments()
    {
        QVector<double> out;
        skip();
        if (take(QLatin1Char(')'))) return out;
        while (!failed) {
            out.push_back(expression());
            if (out.size() > 1000) failed = true;
            if (take(QLatin1Char(',')) || take(QLatin1Char(';'))) continue;
            if (!take(QLatin1Char(')'))) failed = true;
            break;
        }
        return out;
    }
    double primary()
    {
        skip();
        if (!tick() || at >= text.size()) {
            failed = true;
            return 0;
        }
        const QChar c = text[at];
        if (c == QLatin1Char('(')) {
            ++at;
            const double v = expression();
            if (!take(QLatin1Char(')'))) failed = true;
            return v;
        }
        if (c.isDigit() || c == QLatin1Char('.')) {
            int end = at;
            while (end < text.size() && (text[end].isDigit() || text[end] == QLatin1Char('.'))) ++end;
            bool ok = false;
            const double v = text.mid(at, end - at).toDouble(&ok);
            at = end;
            if (!ok) failed = true;
            return v;
        }
        if (c.isLetter() || c == QLatin1Char('_')) {
            int end = at;
            while (end < text.size() && (text[end].isLetterOrNumber() || text[end] == QLatin1Char('_') || text[end] == QLatin1Char('.'))) ++end;
            const QString name = text.mid(at, end - at);
            at = end;
            skip();
            const QString f = name.toUpper();
            if (at < text.size() && text[at] == QLatin1Char('(')) {
                ++at;
                const QVector<double> a = arguments();
                if (failed) return 0;
                return call(f, a);
            }
            if (f == QLatin1String("TRUE")) return 1;
            if (f == QLatin1String("FALSE")) return 0;
            if (names && *names)
                if (const std::optional<double> v = (*names)(name)) return *v;
            failed = true;
            return 0;
        }
        failed = true;
        return 0;
    }
    double call(const QString &f, const QVector<double> &a)
    {
        auto need = [&](int count) {
            if (a.size() != count) failed = true;
            return !failed;
        };
        if (f == QLatin1String("SUM")) {
            double s = 0;
            for (double v : a) s += v;
            return s;
        }
        if (f == QLatin1String("PRODUCT")) {
            double s = 1;
            for (double v : a) s *= v;
            return s;
        }
        if (f == QLatin1String("COUNT")) return double(a.size());
        if (f == QLatin1String("AVERAGE")) {
            if (a.isEmpty()) return failed = true, 0;
            double s = 0;
            for (double v : a) s += v;
            return s / a.size();
        }
        if (f == QLatin1String("MAX") || f == QLatin1String("MIN")) {
            if (a.isEmpty()) return failed = true, 0;
            return f == QLatin1String("MAX") ? *std::max_element(a.begin(), a.end()) : *std::min_element(a.begin(), a.end());
        }
        if (f == QLatin1String("ABS")) return need(1) ? std::fabs(a[0]) : 0;
        if (f == QLatin1String("INT")) return need(1) ? std::floor(a[0]) : 0;
        if (f == QLatin1String("SIGN")) return need(1) ? (a[0] > 0 ? 1.0 : a[0] < 0 ? -1.0 : 0.0) : 0;
        if (f == QLatin1String("NOT")) return need(1) ? (a[0] == 0 ? 1.0 : 0.0) : 0;
        if (f == QLatin1String("MOD")) {
            if (!need(2)) return 0;
            if (a[1] == 0) return failed = true, 0;
            return a[0] - a[1] * std::floor(a[0] / a[1]);
        }
        if (f == QLatin1String("ROUND")) {
            if (!need(2)) return 0;
            const double digits = std::max(-15.0, std::min(15.0, a[1]));
            const double scale = std::pow(10.0, std::trunc(digits));
            return std::round(a[0] * scale) / scale;
        }
        if (f == QLatin1String("AND")) {
            for (double v : a)
                if (v == 0) return 0;
            return a.isEmpty() ? 0 : 1;
        }
        if (f == QLatin1String("OR")) {
            for (double v : a)
                if (v != 0) return 1;
            return 0;
        }
        if (f == QLatin1String("IF")) return need(3) ? (a[0] != 0 ? a[1] : a[2]) : 0;
        if (f == QLatin1String("DEFINED")) return 0;
        failed = true;
        return 0;
    }
};

} // namespace

std::optional<double> evaluateFormula(const QString &expression, const std::function<std::optional<double>(const QString &)> &name)
{
    if (expression.size() > kMostCodeChars) return std::nullopt;
    Parser p;
    p.text = expression;
    p.names = &name;
    p.skip();
    if (p.at >= p.text.size()) return std::nullopt;
    const double v = p.expression();
    p.skip();
    if (p.failed || p.at < p.text.size() || !std::isfinite(v)) return std::nullopt;
    return v;
}

// ---- results

namespace {

QString errorText(const char *text) { return QCoreApplication::translate("jp::write::Fields", text); }

// A number as the field's switches show it: in the picture, in a number format, or as it is.
QString showNumber(const Code &code, double value)
{
    if (code.hasNumberPicture) return numberInPicture(value, code.numberPicture);
    bool formatted = false;
    for (const QString &g : code.general) formatted = formatted || isNumberFormat(g);
    const double rounded = std::round(value);
    if (formatted && std::fabs(rounded) < 9e15) return formatNumber(qint64(rounded), code.general);
    if (std::fabs(value - rounded) < 1e-9 && std::fabs(rounded) < 9e15) return formatText(QString::number(qint64(rounded)), code.general);
    return formatText(plainNumber(value), code.general);
}

QString showText(const Code &code, const QString &text)
{
    if (code.hasNumberPicture) {
        bool ok = false;
        const double v = text.trimmed().toDouble(&ok);
        if (ok) return numberInPicture(v, code.numberPicture);
    }
    return formatText(text, code.general);
}

QString showDate(const Code &code, const QDateTime &moment, const QString &fallback)
{
    return formatText(dateInPicture(moment, code.hasDatePicture ? code.datePicture : fallback), code.general);
}

// The numbers a condition compares: text that reads as a number, else text.
bool numberOf(const QString &s, double *v)
{
    bool ok = false;
    *v = s.trimmed().toDouble(&ok);
    return ok;
}

bool wildcardMatch(const QString &pattern, const QString &text)
{
    QString re = QRegularExpression::escape(pattern);
    re.replace(QStringLiteral("\\*"), QStringLiteral(".*")).replace(QStringLiteral("\\?"), QStringLiteral("."));
    QRegularExpression rx(QLatin1Char('^') + re + QLatin1Char('$'), QRegularExpression::CaseInsensitiveOption | QRegularExpression::DotMatchesEverythingOption);
    return text.size() < 2000 && rx.match(text).hasMatch();
}

std::optional<QString> propertyValue(const Properties &p, const QString &nameIn, const Code &code)
{
    const QString name = nameIn.toLower().simplified();
    auto text = [&](const QString &v) { return std::optional<QString>(showText(code, v)); };
    auto date = [&](const QDateTime &d) -> std::optional<QString> {
        if (!d.isValid()) return std::nullopt;
        return showDate(code, d, QStringLiteral("M/d/yyyy h:mm:ss AM/PM"));
    };
    auto count = [&](int v) -> std::optional<QString> { return p.statistics ? std::optional<QString>(showNumber(code, v)) : std::nullopt; };
    if (name == QLatin1String("author")) return text(p.author);
    if (name == QLatin1String("title")) return text(p.title);
    if (name == QLatin1String("subject")) return text(p.subject);
    if (name == QLatin1String("keywords")) return text(p.keywords);
    if (name == QLatin1String("comments")) return text(p.comments);
    if (name == QLatin1String("lastsavedby")) return text(p.lastSavedBy);
    if (name == QLatin1String("template")) return text(p.template_);
    if (name == QLatin1String("company")) return text(p.company);
    if (name == QLatin1String("manager")) return text(p.manager);
    if (name == QLatin1String("category")) return text(p.category);
    if (name == QLatin1String("revision number") || name == QLatin1String("revnum")) return text(p.revision);
    if (name == QLatin1String("creation date") || name == QLatin1String("createtime") || name == QLatin1String("created")) return date(p.created);
    if (name == QLatin1String("last save time") || name == QLatin1String("savetime") || name == QLatin1String("saved")) return date(p.saved);
    if (name == QLatin1String("last print date") || name == QLatin1String("printtime") || name == QLatin1String("printed")) return date(p.printed);
    if (name == QLatin1String("total editing time") || name == QLatin1String("edittime")) return std::optional<QString>(showNumber(code, double(p.totalMinutes)));
    if (name == QLatin1String("number of pages") || name == QLatin1String("pages")) return count(p.pages);
    if (name == QLatin1String("number of words") || name == QLatin1String("words")) return count(p.words);
    if (name == QLatin1String("number of characters") || name == QLatin1String("characters")) return count(p.characters);
    if (name == QLatin1String("number of characters (with spaces)")) return count(p.charactersWithSpaces);
    if (name == QLatin1String("number of lines") || name == QLatin1String("lines")) return count(p.lines);
    if (name == QLatin1String("number of paragraphs") || name == QLatin1String("paragraphs")) return count(p.paragraphs);
    return std::nullopt;
}

std::optional<QString> referenceText(const Code &code, Env &env)
{
    const QString name = code.arguments.value(0).trimmed();
    if (!env.bookmark) return std::nullopt;
    if (name.isEmpty()) return referenceSourceNotFound();
    const std::optional<BookmarkInfo> b = env.bookmark(name);
    if (!b) return referenceSourceNotFound();
    if (code.name == QLatin1String("PAGEREF")) {
        if (code.has(QLatin1Char('p'))) return b->position < env.position ? QCoreApplication::translate("jp::write::Fields", "above") : QCoreApplication::translate("jp::write::Fields", "below");
        if (b->page.isEmpty()) return std::nullopt;
        bool ok = false;
        const int n = b->pageIndex > 0 ? b->pageIndex : b->page.toInt(&ok);
        for (const QString &g : code.general)
            if (isNumberFormat(g) && (b->pageIndex > 0 || ok)) return formatNumber(n, code.general);
        return formatText(b->page, code.general);
    }
    // REF
    if (code.has(QLatin1Char('p'))) return b->position < env.position ? QCoreApplication::translate("jp::write::Fields", "above") : QCoreApplication::translate("jp::write::Fields", "below");
    if (code.has(QLatin1Char('w'))) return showText(code, b->fullNumber.isEmpty() ? QStringLiteral("0") : b->fullNumber);
    if (code.has(QLatin1Char('r'))) return showText(code, b->relativeNumber.isEmpty() ? QStringLiteral("0") : b->relativeNumber);
    if (code.has(QLatin1Char('n'))) return showText(code, b->number.isEmpty() ? QStringLiteral("0") : b->number);
    return formatText(b->text, code.general);
}

std::optional<QString> sequenceText(const Code &code, Env &env)
{
    if (!env.sequences) return std::nullopt;
    const QString key = code.arguments.value(0).trimmed().toUpper();
    qint64 &counter = (*env.sequences)[key];
    if (code.has(QLatin1Char('r'))) {
        bool ok = false;
        const qint64 v = code.argumentOf(QLatin1Char('r')).toLongLong(&ok);
        counter = ok ? std::max<qint64>(0, std::min(v, kMostSequence)) : counter;
    } else if (code.has(QLatin1Char('c'))) {
        if (counter == 0) counter = 1;
    } else {
        counter = std::min(counter + 1, kMostSequence);
    }
    if (code.has(QLatin1Char('h'))) return QString();
    return formatNumber(counter, code.general);
}

std::optional<QString> condition(const Code &code)
{
    QStringList w = code.arguments;
    if (w.size() != 5) return std::nullopt;
    const QString &op = w[1];
    double a = 0, b = 0;
    const bool numeric = numberOf(w[0], &a) && numberOf(w[2], &b);
    bool result;
    if (op == QLatin1String("=")) result = numeric ? a == b : wildcardMatch(w[2], w[0]);
    else if (op == QLatin1String("<>")) result = numeric ? a != b : !wildcardMatch(w[2], w[0]);
    else if (op == QLatin1String("<")) result = numeric ? a < b : w[0].compare(w[2], Qt::CaseInsensitive) < 0;
    else if (op == QLatin1String("<=")) result = numeric ? a <= b : w[0].compare(w[2], Qt::CaseInsensitive) <= 0;
    else if (op == QLatin1String(">")) result = numeric ? a > b : w[0].compare(w[2], Qt::CaseInsensitive) > 0;
    else if (op == QLatin1String(">=")) result = numeric ? a >= b : w[0].compare(w[2], Qt::CaseInsensitive) >= 0;
    else return std::nullopt;
    return formatText(result ? w[3] : w[4], code.general);
}

} // namespace

QString referenceSourceNotFound() { return errorText("Error! Reference source not found."); }
QString bookmarkNotDefined() { return errorText("Error! Bookmark not defined."); }

bool dependsOnTheMachine(const QString &name)
{
    static const QStringList names = {QStringLiteral("DATE"), QStringLiteral("TIME"), QStringLiteral("SAVEDATE"), QStringLiteral("PRINTDATE"), QStringLiteral("FILENAME"), QStringLiteral("FILESIZE"),
                                      QStringLiteral("AUTHOR"), QStringLiteral("LASTSAVEDBY"), QStringLiteral("EDITTIME"), QStringLiteral("USERNAME"), QStringLiteral("USERINITIALS"), QStringLiteral("USERADDRESS")};
    return names.contains(name.toUpper());
}

bool isKnown(const QString &nameIn)
{
    static const QSet<QString> names = {QStringLiteral("PAGE"), QStringLiteral("NUMPAGES"), QStringLiteral("SECTIONPAGES"), QStringLiteral("SECTION"), QStringLiteral("PAGEREF"), QStringLiteral("REF"), QStringLiteral("NOTEREF"),
                                        QStringLiteral("STYLEREF"), QStringLiteral("SEQ"), QStringLiteral("DATE"), QStringLiteral("TIME"), QStringLiteral("CREATEDATE"), QStringLiteral("SAVEDATE"), QStringLiteral("PRINTDATE"),
                                        QStringLiteral("AUTHOR"), QStringLiteral("TITLE"), QStringLiteral("SUBJECT"), QStringLiteral("KEYWORDS"), QStringLiteral("COMMENTS"), QStringLiteral("LASTSAVEDBY"), QStringLiteral("TEMPLATE"),
                                        QStringLiteral("FILENAME"), QStringLiteral("FILESIZE"), QStringLiteral("DOCPROPERTY"), QStringLiteral("NUMWORDS"), QStringLiteral("NUMCHARS"), QStringLiteral("IF"), QStringLiteral("="),
                                        QStringLiteral("QUOTE"), QStringLiteral("REVNUM"), QStringLiteral("EDITTIME")};
    return names.contains(nameIn.toUpper());
}

std::optional<QString> compute(const Code &code, Env &env)
{
    const QString &n = code.name;
    if (n == QLatin1String("PAGE")) {
        if (env.page <= 0) return std::nullopt;
        bool formatted = false;
        for (const QString &g : code.general) formatted = formatted || isNumberFormat(g);
        if (formatted || env.pageText.isEmpty()) return formatNumber(env.page, code.general);
        return formatText(env.pageText, code.general);
    }
    if (n == QLatin1String("NUMPAGES")) return env.pages > 0 ? std::optional<QString>(showNumber(code, env.pages)) : std::nullopt;
    if (n == QLatin1String("SECTIONPAGES")) return env.sectionPages > 0 ? std::optional<QString>(showNumber(code, env.sectionPages)) : std::nullopt;
    if (n == QLatin1String("SECTION")) return env.section > 0 ? std::optional<QString>(showNumber(code, env.section)) : std::nullopt;
    if (n == QLatin1String("PAGEREF") || n == QLatin1String("REF")) return referenceText(code, env);
    if (n == QLatin1String("NOTEREF")) {
        if (!env.noteNumber) return std::nullopt;
        const std::optional<QString> v = env.noteNumber(code.arguments.value(0).trimmed(), code);
        return v ? std::optional<QString>(formatText(*v, code.general)) : std::nullopt;
    }
    if (n == QLatin1String("STYLEREF")) {
        if (!env.styleRef) return std::nullopt;
        const std::optional<QString> v = env.styleRef(code.arguments.value(0).trimmed(), code);
        return v ? std::optional<QString>(formatText(*v, code.general)) : std::nullopt;
    }
    if (n == QLatin1String("SEQ")) return sequenceText(code, env);
    if (n == QLatin1String("DATE") || n == QLatin1String("TIME")) {
        if (!env.now.isValid()) return std::nullopt;
        return showDate(code, env.now, n == QLatin1String("DATE") ? QStringLiteral("M/d/yyyy") : QStringLiteral("h:mm AM/PM"));
    }
    if (n == QLatin1String("CREATEDATE") || n == QLatin1String("SAVEDATE") || n == QLatin1String("PRINTDATE")) {
        const QDateTime d = n == QLatin1String("CREATEDATE") ? env.properties.created : n == QLatin1String("SAVEDATE") ? env.properties.saved : env.properties.printed;
        if (!d.isValid()) return std::nullopt;
        return showDate(code, d, QStringLiteral("M/d/yyyy h:mm:ss AM/PM"));
    }
    if (n == QLatin1String("AUTHOR") || n == QLatin1String("TITLE") || n == QLatin1String("SUBJECT") || n == QLatin1String("KEYWORDS") || n == QLatin1String("COMMENTS") || n == QLatin1String("LASTSAVEDBY") ||
        n == QLatin1String("TEMPLATE") || n == QLatin1String("REVNUM")) {
        if (!code.arguments.isEmpty()) return formatText(code.arguments.join(QLatin1Char(' ')), code.general);   // (a new value, shown)
        return propertyValue(env.properties, n == QLatin1String("REVNUM") ? QStringLiteral("revnum") : n, code);
    }
    if (n == QLatin1String("EDITTIME")) return showNumber(code, double(env.properties.totalMinutes));
    if (n == QLatin1String("DOCPROPERTY")) return propertyValue(env.properties, code.arguments.value(0), code);
    if (n == QLatin1String("NUMWORDS")) return env.properties.statistics ? std::optional<QString>(showNumber(code, env.properties.words)) : std::nullopt;
    if (n == QLatin1String("NUMCHARS")) return env.properties.statistics ? std::optional<QString>(showNumber(code, env.properties.characters)) : std::nullopt;
    if (n == QLatin1String("FILENAME")) {
        const QString v = code.has(QLatin1Char('p')) ? env.properties.filePath : env.properties.fileName;
        return v.isEmpty() ? std::nullopt : std::optional<QString>(formatText(v, code.general));
    }
    if (n == QLatin1String("FILESIZE")) {
        if (env.properties.fileSize < 0) return std::nullopt;
        double size = double(env.properties.fileSize);
        if (code.has(QLatin1Char('k'))) size = std::round(size / 1024.0);
        else if (code.has(QLatin1Char('m'))) size = std::round(size / 1048576.0);
        return showNumber(code, size);
    }
    if (n == QLatin1String("QUOTE")) return formatText(code.arguments.join(QLatin1Char(' ')), code.general);
    if (n == QLatin1String("IF")) return condition(code);
    if (n == QLatin1String("=")) {
        const std::optional<double> v = evaluateFormula(code.expression, env.number);
        return v ? std::optional<QString>(showNumber(code, *v)) : std::nullopt;
    }
    return std::nullopt;
}

} // namespace jp::write::fields
