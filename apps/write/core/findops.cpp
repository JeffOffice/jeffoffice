#include "core/findops.h"

#include "text/textprops.h"

#include <QCoreApplication>
#include <QHash>
#include <QVector>
#include <algorithm>
#include <functional>

namespace jp::write {

namespace {

QString problem(const char *text) { return QCoreApplication::translate("jp::write::Find", text); }

}

FindPattern wildcardPattern(const QString &pattern, bool matchCase)
{
    FindPattern out;
    if (pattern.isEmpty()) {
        out.error = problem(QT_TRANSLATE_NOOP("jp::write::Find", "Type what to find."));
        return out;
    }
    if (pattern.size() > kMostPatternLength) {
        out.error = problem(QT_TRANSLATE_NOOP("jp::write::Find", "The pattern is too long."));
        return out;
    }
    QString re;
    int groups = 0;
    for (int i = 0; i < pattern.size(); ++i) {
        const QChar c = pattern[i];
        const QChar next = i + 1 < pattern.size() ? pattern[i + 1] : QChar();
        if (c == QLatin1Char('\\') && !next.isNull()) {
            re += QRegularExpression::escape(QString(next));
            ++i;
        } else if (c == QLatin1Char('^') && !next.isNull()) {
            ++i;
            switch (next.unicode()) {
            case 'p': re += QStringLiteral("\n"); break;
            case 't': re += QStringLiteral("\t"); break;
            case '#': re += QStringLiteral("\\d"); break;
            case '$': re += QStringLiteral("[A-Za-z]"); break;
            case 'w': re += QStringLiteral("[ \\t]+"); break;
            case 'b': re += QChar(kFindSectionBreak); break;
            case 'd': re += QChar(kFindField); break;
            case 'e': re += QChar(kFindEndnoteMark); break;
            case 'f': re += QChar(kFindFootnoteMark); break;
            case '^': re += QStringLiteral("\\^"); break;
            default: re += QRegularExpression::escape(QString(c) + next); break;
            }
        } else if (c == QLatin1Char('?')) {
            re += QStringLiteral("[^\n]");
        } else if (c == QLatin1Char('*')) {
            re += QStringLiteral("[^\n]*?");
        } else if (c == QLatin1Char('<')) {
            re += QStringLiteral("\\b(?=\\w)");
        } else if (c == QLatin1Char('>')) {
            re += QStringLiteral("(?<=\\w)\\b");
        } else if (c == QLatin1Char('@')) {
            re += QLatin1Char('+');
        } else if (c == QLatin1Char('[')) {
            const int close = int(pattern.indexOf(QLatin1Char(']'), i + 2));
            if (close < 0) {
                out.error = problem(QT_TRANSLATE_NOOP("jp::write::Find", "A [ has no ] to close it."));
                return out;
            }
            QString set = pattern.mid(i + 1, close - i - 1);
            const bool negate = set.startsWith(QLatin1Char('!'));
            if (negate) set.remove(0, 1);
            set.replace(QLatin1Char('\\'), QStringLiteral("\\\\"));
            set.replace(QLatin1Char('^'), QStringLiteral("\\^"));
            set.replace(QLatin1Char('['), QStringLiteral("\\["));
            re += QLatin1Char('[') + (negate ? QStringLiteral("^") : QString()) + set + QLatin1Char(']');
            i = close;
        } else if (c == QLatin1Char('{')) {
            const int close = int(pattern.indexOf(QLatin1Char('}'), i + 1));
            static const QRegularExpression counts(QStringLiteral("^\\d{1,3}(,\\d{0,3})?$"));
            if (close < 0 || !counts.match(pattern.mid(i + 1, close - i - 1)).hasMatch()) {
                out.error = problem(QT_TRANSLATE_NOOP("jp::write::Find", "A { needs a count and a } to close it, such as {2} or {1,3}."));
                return out;
            }
            re += pattern.mid(i, close - i + 1);
            i = close;
        } else if (c == QLatin1Char('(')) {
            if (++groups > 9) {
                out.error = problem(QT_TRANSLATE_NOOP("jp::write::Find", "A pattern can have at most nine groups."));
                return out;
            }
            re += c;
        } else if (c == QLatin1Char(')')) {
            if (--groups < 0) break;
            re += c;
        } else {
            re += QRegularExpression::escape(QString(c));
        }
    }
    if (groups != 0) {
        out.error = problem(QT_TRANSLATE_NOOP("jp::write::Find", "The ( and ) in the pattern do not match."));
        return out;
    }
    out.regex = QRegularExpression(re, matchCase ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption);
    if (!out.regex.isValid()) {
        out.error = problem(QT_TRANSLATE_NOOP("jp::write::Find", "That is not a pattern JeffWrite can use."));
        out.regex = QRegularExpression();
    }
    return out;
}

QString expandReplacement(const QString &with, const QRegularExpressionMatch &match)
{
    QString out;
    for (int i = 0; i < with.size(); ++i) {
        const QChar c = with[i];
        const QChar next = i + 1 < with.size() ? with[i + 1] : QChar();
        if (c == QLatin1Char('\\') && next.isDigit() && next != QLatin1Char('0')) {
            out += match.captured(next.digitValue());
            ++i;
        } else if (c == QLatin1Char('\\') && next == QLatin1Char('\\')) {
            out += QLatin1Char('\\');
            ++i;
        } else if (c == QLatin1Char('^') && next == QLatin1Char('&')) {
            out += match.captured(0);
            ++i;
        } else if (c == QLatin1Char('^') && next == QLatin1Char('t')) {
            out += QLatin1Char('\t');
            ++i;
        } else if (c == QLatin1Char('^') && next == QLatin1Char('^')) {
            out += QLatin1Char('^');
            ++i;
        } else {
            out += c;
        }
    }
    return out;
}

} // namespace jp::write

namespace jp::write {

namespace {

// A letter's Soundex digit ('0' for the letters that code nothing, and the vowels).
char soundexDigit(QChar c)
{
    switch (c.toLower().unicode()) {
    case 'b': case 'f': case 'p': case 'v': return '1';
    case 'c': case 'g': case 'j': case 'k': case 'q': case 's': case 'x': case 'z': return '2';
    case 'd': case 't': return '3';
    case 'l': return '4';
    case 'm': case 'n': return '5';
    case 'r': return '6';
    case 'h': case 'w': return 'h';   // (they do not part two letters of one code)
    default: return '0';
    }
}

bool isAscii(QChar c) { return c.unicode() < 128; }

struct Token {
    int start = 0, end = 0;
    QString key;
};

constexpr int kMostFindWords = 200000;   // a longer text is searched as far as this many words

bool isWordChar(QChar c) { return c.isLetterOrNumber() || c == QLatin1Char('\''); }

// The words of the text from `from` on that may begin a match, each with the key that makes two words alike.
QVector<Token> wordsOf(const QString &text, int from, const std::function<QString(const QString &)> &key)
{
    QVector<Token> out;
    int i = std::max(0, from);
    // (A word that began before `from` is not one that begins a match.)
    while (i > 0 && i < text.size() && isWordChar(text[i - 1]) && isWordChar(text[i])) ++i;
    const int n = int(text.size());
    while (i < n && out.size() < kMostFindWords) {
        if (!isWordChar(text[i])) {
            ++i;
            continue;
        }
        int j = i;
        while (j < n && isWordChar(text[j])) ++j;
        out.push_back({i, j, key(text.mid(i, j - i))});
        i = j;
    }
    return out;
}

// Words in a row, whose keys are the query's, with nothing but spaces and punctuation between them.
QPair<int, int> wordRun(const QString &text, int from, const AdvancedFind &o, const std::function<QString(const QString &)> &key)
{
    QStringList want;
    for (const Token &t : wordsOf(o.query, 0, key))
        if (!t.key.isEmpty()) want << t.key;
    if (want.isEmpty()) return {-1, -1};
    const QVector<Token> tokens = wordsOf(text, from, key);
    for (int i = 0; i + want.size() <= tokens.size(); ++i) {
        bool same = true;
        for (int k = 0; k < want.size() && same; ++k) {
            same = tokens[i + k].key == want[k];
            // (Words in a phrase are parted by spaces alone: a comma or a full stop ends the phrase.)
            if (same && k > 0) {
                const QStringView gap = QStringView(text).mid(tokens[i + k - 1].end, tokens[i + k].start - tokens[i + k - 1].end);
                same = std::all_of(gap.begin(), gap.end(), [](QChar c) { return c.isSpace(); });
            }
        }
        if (same) return {tokens[i].start, tokens[i + int(want.size()) - 1].end};
    }
    return {-1, -1};
}

} // namespace

QString soundexOf(const QString &word)
{
    QString code;
    char last = '0';
    for (const QChar c : word) {
        if (!isAscii(c) || !c.isLetter()) continue;
        const char d = soundexDigit(c);
        if (code.isEmpty()) {
            code += c.toUpper();
            last = d;
            continue;
        }
        if (d == 'h') continue;
        if (d != '0' && d != last) code += QLatin1Char(d);
        last = d;
        if (code.size() == 4) break;
    }
    if (code.isEmpty()) return QString();
    while (code.size() < 4) code += QLatin1Char('0');
    return code;
}

QString wordStem(const QString &word)
{
    static const QHash<QString, QString> irregular = {
        {QStringLiteral("am"), QStringLiteral("be")}, {QStringLiteral("is"), QStringLiteral("be")}, {QStringLiteral("are"), QStringLiteral("be")}, {QStringLiteral("was"), QStringLiteral("be")},
        {QStringLiteral("were"), QStringLiteral("be")}, {QStringLiteral("been"), QStringLiteral("be")}, {QStringLiteral("being"), QStringLiteral("be")},
        {QStringLiteral("ran"), QStringLiteral("run")}, {QStringLiteral("went"), QStringLiteral("go")}, {QStringLiteral("gone"), QStringLiteral("go")}, {QStringLiteral("goes"), QStringLiteral("go")},
        {QStringLiteral("has"), QStringLiteral("have")}, {QStringLiteral("had"), QStringLiteral("have")}, {QStringLiteral("does"), QStringLiteral("do")}, {QStringLiteral("did"), QStringLiteral("do")},
        {QStringLiteral("done"), QStringLiteral("do")}, {QStringLiteral("made"), QStringLiteral("make")}, {QStringLiteral("took"), QStringLiteral("take")}, {QStringLiteral("taken"), QStringLiteral("take")},
        {QStringLiteral("saw"), QStringLiteral("see")}, {QStringLiteral("seen"), QStringLiteral("see")}, {QStringLiteral("came"), QStringLiteral("come")}, {QStringLiteral("gave"), QStringLiteral("give")},
        {QStringLiteral("given"), QStringLiteral("give")}, {QStringLiteral("wrote"), QStringLiteral("write")}, {QStringLiteral("written"), QStringLiteral("write")}, {QStringLiteral("men"), QStringLiteral("man")},
        {QStringLiteral("women"), QStringLiteral("woman")}, {QStringLiteral("children"), QStringLiteral("child")}, {QStringLiteral("feet"), QStringLiteral("foot")}, {QStringLiteral("teeth"), QStringLiteral("tooth")},
        {QStringLiteral("mice"), QStringLiteral("mouse")}, {QStringLiteral("better"), QStringLiteral("good")}, {QStringLiteral("best"), QStringLiteral("good")}, {QStringLiteral("said"), QStringLiteral("say")},
        {QStringLiteral("knew"), QStringLiteral("know")}, {QStringLiteral("known"), QStringLiteral("know")}, {QStringLiteral("thought"), QStringLiteral("think")}, {QStringLiteral("bought"), QStringLiteral("buy")},
        {QStringLiteral("brought"), QStringLiteral("bring")}, {QStringLiteral("found"), QStringLiteral("find")}, {QStringLiteral("told"), QStringLiteral("tell")}, {QStringLiteral("sold"), QStringLiteral("sell")}};
    QString w = word.toLower();
    w.remove(QLatin1Char('\''));
    if (irregular.contains(w)) w = irregular.value(w);
    auto vowel = [](QChar c) { return QStringLiteral("aeiouy").contains(c); };
    auto hasVowel = [&](const QString &s) { return std::any_of(s.begin(), s.end(), vowel); };
    auto strip = [&](const QString &suffix) {
        if (w.size() > suffix.size() + 1 && w.endsWith(suffix) && hasVowel(w.left(w.size() - suffix.size()))) {
            w.chop(suffix.size());
            return true;
        }
        return false;
    };
    // (One ending, the longest that fits.)
    if (!strip(QStringLiteral("ing")) && !strip(QStringLiteral("est")) && !strip(QStringLiteral("ed")) && !strip(QStringLiteral("er"))) {
        const QString base = w.left(w.size() - 2);
        if (w.endsWith(QStringLiteral("es")) && w.size() > 3) w = (base.endsWith(QLatin1Char('s')) || base.endsWith(QLatin1Char('x')) || base.endsWith(QLatin1Char('z')) || base.endsWith(QStringLiteral("ch")) || base.endsWith(QStringLiteral("sh"))) ? base : w.left(w.size() - 1);
        else if (w.endsWith(QLatin1Char('s')) && !w.endsWith(QStringLiteral("ss")) && !w.endsWith(QStringLiteral("us")) && w.size() > 3) w.chop(1);
    }
    if (w.endsWith(QLatin1Char('e')) && w.size() > 2) w.chop(1);
    if (w.endsWith(QLatin1Char('y')) && w.size() > 2) w[w.size() - 1] = QLatin1Char('i');
    if (w.size() > 3 && w[w.size() - 1] == w[w.size() - 2] && !vowel(w[w.size() - 1]) && w[w.size() - 1] != QLatin1Char('s') && w[w.size() - 1] != QLatin1Char('l')) w.chop(1);
    return w;
}

QPair<int, int> advancedMatch(const QString &text, int from, const AdvancedFind &o)
{
    if (o.query.isEmpty()) return {-1, -1};
    if (o.soundsLike) return wordRun(text, from, o, soundexOf);
    if (o.wordForms) return wordRun(text, from, o, wordStem);
    // Letters with punctuation or spaces allowed between them (the characters the choices leave out of the query are not looked for).
    const QString between = QStringLiteral("%1%2").arg(o.ignorePunctuation ? QStringLiteral("\\p{P}\\p{S}") : QString(), o.ignoreSpaces ? QStringLiteral("\\s") : QString());
    QStringList parts;
    for (const QChar c : o.query) {
        if (o.ignorePunctuation && (c.isPunct() || c.isSymbol())) continue;
        if (o.ignoreSpaces && c.isSpace()) continue;
        parts << QRegularExpression::escape(QString(c));
    }
    if (parts.isEmpty()) return {-1, -1};
    QString pattern = parts.join(between.isEmpty() ? QString() : QStringLiteral("[%1]*").arg(between));
    if (o.wholeWords) pattern = QStringLiteral("\\b%1\\b").arg(pattern);
    QRegularExpression re(pattern, o.matchCase ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch m = re.match(text, std::max(0, from));
    return m.hasMatch() ? QPair<int, int>{int(m.capturedStart()), int(m.capturedEnd())} : QPair<int, int>{-1, -1};
}

bool FormatFilter::matchesChar(const QTextCharFormat &f) const
{
    auto is = [](int want, bool have) { return want < 0 || (want != 0) == have; };
    if (!is(bold, f.fontWeight() >= QFont::Bold)) return false;
    if (!is(italic, f.fontItalic())) return false;
    if (!is(underline, f.fontUnderline() || f.underlineStyle() != QTextCharFormat::NoUnderline)) return false;
    if (!is(strike, f.fontStrikeOut())) return false;
    if (!is(highlighted, f.hasProperty(tp::HighlightRefP) && !f.stringProperty(tp::HighlightRefP).isEmpty())) return false;
    if (!font.isEmpty()) {
        const QStringList families = f.fontFamilies().toStringList();
        const bool same = std::any_of(families.begin(), families.end(), [&](const QString &fam) { return fam.compare(font, Qt::CaseInsensitive) == 0; });
        if (!same) return false;
    }
    return true;
}

} // namespace jp::write
