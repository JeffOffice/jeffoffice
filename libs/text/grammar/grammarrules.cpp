#include "grammar/grammarrules.h"

#include <QCoreApplication>
#include <QSet>
#include <algorithm>

namespace jp::grammar {

namespace {

// ---- tokens: made once for a paragraph, and every rule goes through them once

enum class Tok { Word, Number, Space, Punct };

struct Token {
    int start = 0, end = 0;
    Tok kind = Tok::Punct;
    QString lower;   // words and numbers: in lowercase, with the curly apostrophe made straight
};

struct Ctx {
    const QString &text;
    QVector<Token> tokens;
    QVector<GrammarIssue> issues;
    const char *rule = "";   // the rule that is running
    quint64 steps = 0;
    // The run of tokens with no space in it that each token is in (an address, a file name, a word with its marks): [first, last] token indexes, and how many periods it has
    // before its trailing marks, and whether it has a mark that only an address or a path has.
    QVector<int> chunkFirst, chunkLast;
    QVector<int> chunkDots;
    QVector<char> chunkAddress;
    QVector<char> ifBefore;
    int leaders = -1;   // whether the paragraph has ". . ." in it (found once, when a rule first asks)
    bool hasLeaders()
    {
        if (leaders < 0) {
            leaders = text.contains(QLatin1String(". . .")) ? 1 : 0;
            steps += quint64(text.size());
        }
        return leaders == 1;
    }   // whether a word that opens a clause ("if", "when") came earlier in the same sentence

    int curly = -1;   // whether the paragraph writes its apostrophes curly (found once, when a rule first asks)
    QString apostrophe()
    {
        if (curly < 0) {
            curly = text.contains(QChar(0x2019)) ? 1 : 0;
            steps += quint64(text.size());
        }
        return curly == 1 ? QString(QChar(0x2019)) : QStringLiteral("'");
    }

    explicit Ctx(const QString &t) : text(t) {}
    int size() const { return int(tokens.size()); }
    void step() { ++steps; }
    QString original(int i) const { return text.mid(tokens[i].start, tokens[i].end - tokens[i].start); }
    bool isWord(int i) const { return i >= 0 && i < size() && tokens[i].kind == Tok::Word; }
    bool isPunct(int i, QChar c) const { return i >= 0 && i < size() && tokens[i].kind == Tok::Punct && text.at(tokens[i].start) == c; }
    bool isSpace(int i) const { return i >= 0 && i < size() && tokens[i].kind == Tok::Space; }
    // Whether a space token is plain spaces only (no tab, no line break, no other space).
    bool plainSpaces(int i) const
    {
        for (int p = tokens[i].start; p < tokens[i].end; ++p)
            if (text.at(p) != QLatin1Char(' ')) return false;
        return true;
    }
    // The word or number after this token with one space between (no mark), and the one before it; -1 when there is none.
    int nextWord(int i) const
    {
        const int j = i + 1;
        if (!isSpace(j) || j + 1 >= size()) return -1;
        const Tok k = tokens[j + 1].kind;
        return (k == Tok::Word || k == Tok::Number) ? j + 1 : -1;
    }
    int prevWord(int i) const
    {
        const int j = i - 1;
        if (!isSpace(j) || j < 1) return -1;
        const Tok k = tokens[j - 1].kind;
        return (k == Tok::Word || k == Tok::Number) ? j - 1 : -1;
    }
    bool firstUpper(int i) const { return text.at(tokens[i].start).isUpper(); }
    void add(int start, int end, GrammarIssue::Kind kind, const QString &message, const QString &replacement)
    {
        GrammarIssue g;
        g.start = start;
        g.end = end;
        g.kind = kind;
        g.message = message;
        g.suggestions << replacement;
        g.rule = QLatin1String(rule);
        issues << g;
    }
};

bool isApostrophe(QChar c) { return c == QLatin1Char('\'') || c == QChar(0x2019); }

void tokenize(Ctx &c)
{
    const QString &t = c.text;
    const int n = int(t.size());
    int i = 0;
    while (i < n) {
        const QChar ch = t.at(i);
        Token tok;
        tok.start = i;
        if (ch.isSpace()) {
            tok.kind = Tok::Space;
            while (i < n && t.at(i).isSpace()) ++i;
        } else if (ch.isLetter()) {
            tok.kind = Tok::Word;
            ++i;
            while (i < n) {
                const QChar d = t.at(i);
                if (d.isLetterOrNumber()) ++i;
                else if ((isApostrophe(d) || d == QLatin1Char('-')) && i + 1 < n && t.at(i + 1).isLetterOrNumber()) i += 2;
                else break;
            }
        } else if (ch.isDigit()) {
            tok.kind = Tok::Number;
            ++i;
            while (i < n) {
                const QChar d = t.at(i);
                if (d.isLetterOrNumber()) ++i;
                else if ((d == QLatin1Char(',') || d == QLatin1Char('.')) && i + 1 < n && t.at(i + 1).isDigit()) i += 2;
                else break;
            }
        } else {
            tok.kind = Tok::Punct;
            ++i;
        }
        tok.end = i;
        if (tok.kind == Tok::Word || tok.kind == Tok::Number) {
            tok.lower = t.mid(tok.start, tok.end - tok.start).toLower();
            tok.lower.replace(QChar(0x2019), QLatin1Char('\''));
        }
        c.tokens << tok;
    }
    c.steps += quint64(n);
    c.ifBefore.resize(c.size());
    {
        bool clause = false;
        for (int k = 0; k < c.size(); ++k) {
            c.ifBefore[k] = clause;
            const Token &tk = c.tokens[k];
            if (tk.kind == Tok::Punct && QLatin1String(".!?;:").contains(t.at(tk.start))) clause = false;
            else if (tk.kind == Tok::Word && (tk.lower == QLatin1String("if") || tk.lower == QLatin1String("when") || tk.lower == QLatin1String("once") || tk.lower == QLatin1String("unless") ||
                                              tk.lower == QLatin1String("whenever") || tk.lower == QLatin1String("until") || tk.lower == QLatin1String("whether")))
                clause = true;
        }
        c.steps += quint64(c.size());
    }
    // The runs without a space.
    const int m = c.size();
    c.chunkFirst.resize(m);
    c.chunkLast.resize(m);
    c.chunkDots.resize(m);
    c.chunkAddress.resize(m);
    int a = 0;
    while (a < m) {
        if (c.tokens[a].kind == Tok::Space) {
            c.chunkFirst[a] = c.chunkLast[a] = a;
            c.chunkDots[a] = 0;
            c.chunkAddress[a] = 0;
            ++a;
            continue;
        }
        int b = a;
        while (b + 1 < m && c.tokens[b + 1].kind != Tok::Space) ++b;
        int end = b;
        while (end > a && c.tokens[end].kind == Tok::Punct) --end;   // (marks at the end are the sentence's)
        int dots = 0;
        char address = 0;
        for (int k = a; k <= b; ++k) {
            if (c.tokens[k].kind != Tok::Punct) continue;
            const QChar p = t.at(c.tokens[k].start);
            if (k <= end && p == QLatin1Char('.')) ++dots;
            if (p == QLatin1Char('/') || p == QLatin1Char('@') || p == QLatin1Char('\\') || p == QLatin1Char('_') || p == QLatin1Char(':') || p == QLatin1Char('#') || p == QLatin1Char('=')) address = 1;
        }
        for (int k = a; k <= b; ++k) {
            c.chunkFirst[k] = a;
            c.chunkLast[k] = b;
            c.chunkDots[k] = dots;
            c.chunkAddress[k] = address;
        }
        c.steps += quint64(b - a + 1);
        a = b + 1;
    }
}

QString matchCase(const QString &sample, const QString &with)
{
    if (!sample.isEmpty() && sample.at(0).isUpper() && !with.isEmpty()) return with.at(0).toUpper() + with.mid(1);
    return with;
}

QSet<QString> setOf(std::initializer_list<const char *> words)
{
    QSet<QString> out;
    for (const char *w : words) out.insert(QLatin1String(w));
    return out;
}
// (made once, by the language's own thread-safe initialization of a static: a check can run on several threads at once)
#define WORDS(...) ([]() -> const QSet<QString> & { static const QSet<QString> s = setOf({__VA_ARGS__}); return s; }())

// Words that end a sentence's last word with a period but are not the end of a sentence (and the endings of addresses).
const QSet<QString> &abbreviations()
{
    return WORDS("etc", "vs", "cf", "approx", "fig", "figs", "eq", "inc", "ltd", "corp", "dept", "est", "misc", "mrs", "prof", "gen", "sgt", "capt", "col", "rev", "hon", "assn", "ave", "blvd",
                 "jan", "feb", "mar", "apr", "jun", "jul", "aug", "sep", "sept", "oct", "nov", "dec", "mon", "tue", "wed", "thu", "fri", "sat", "sun", "nos", "vol", "eds", "sec", "dept", "govt", "univ",
                 "mount", "min", "max", "ref", "tel", "ext", "appt", "apt", "assoc", "bros", "esq", "hrs", "mgmt", "incl", "excl", "esp", "avg", "seq", "ibid", "viz", "resp", "orig", "def",
                 "subm", "cont", "prop", "admin", "educ", "bull", "amer", "meteor", "soc", "var", "ssp", "spp", "subsp", "imp", "exp", "yr", "pp", "dpto", "gov",
                 "dist", "pres", "supt", "cos", "cert", "reg", "regs", "nat", "intl", "int", "mfg", "mfr", "pkg", "qty", "amt", "subj", "temp", "thru", "vol", "wk", "yr");
}
// Words that begin with a small letter after a period without being the start of a sentence.
const QSet<QString> &startsLowerForGood()
{
    return WORDS("pp", "ed", "eds", "vol", "no", "nos", "cf", "viz", "ibid", "et", "al", "vs", "ca", "esp", "incl", "etc", "ssp", "var", "spp", "sp", "ex", "nom", "comb", "auct");
}
const QSet<QString> &joiningWords()
{
    return WORDS("and", "or", "of", "the", "in", "for", "to", "at", "on", "by", "with", "from", "v", "vs", "as", "a", "an");
}
// Words a sentence begins with (the missing space after a period is flagged before these only: "water.Do not", "ended.The").
const QSet<QString> &sentenceStarters()
{
    return WORDS("the", "this", "that", "these", "those", "it", "its", "if", "in", "on", "at", "we", "you", "they", "he", "she", "i", "a", "an", "do", "does", "did", "there", "here", "what", "when", "where", "why",
                 "how", "who", "our", "my", "your", "their", "his", "her", "as", "but", "and", "or", "so", "for", "to", "see", "please", "note", "use", "all", "each", "no", "not", "yes", "now", "then", "also",
                 "however", "some", "many", "most", "one", "two", "go", "let", "after", "before", "with", "without", "once", "while", "because", "since", "thanks", "thank", "hello", "next", "first", "finally");
}
const QSet<QString> &codeWords()
{
    return WORDS("this", "self", "base", "super", "console", "window", "document", "system", "math", "string", "array", "object", "json", "path", "file", "user", "data", "util", "utils", "test", "tests", "node",
                 "view", "model", "config", "error", "logger", "class", "return", "args", "item", "items", "list", "text", "input", "output", "result", "value", "values", "state", "props", "event", "global");
}
const QSet<QString> &addressEndings()
{
    return WORDS("com", "org", "net", "edu", "gov", "io", "js", "py", "ts", "rb", "txt", "pdf", "doc", "docx", "xls", "xlsx", "ppt", "pptx", "html", "htm", "css", "jpg", "jpeg", "png", "gif", "svg", "exe",
                 "zip", "json", "xml", "csv", "md", "cpp", "php", "uk", "us", "ca", "au", "de", "fr", "info", "biz", "co", "me");
}

// ---- the rules: each is one function, and one line of the table at the end

using GK = GrammarIssue::Kind;

const QSet<QString> &functionWords()
{
    return WORDS("the", "a", "an", "of", "to", "in", "on", "at", "and", "or", "it", "for", "with", "that", "this", "is", "was", "be", "by", "as", "we", "you", "they", "he", "she", "i");
}

// "the the": the second of two words alike, with the space before it taken out.
void repeatedWord(Ctx &c)
{
    const QSet<QString> &legitimate = WORDS("had", "that", "is", "bye", "no", "so", "very", "ha", "hah", "blah", "never", "knock", "yada", "choo", "boom", "tsk", "in", "on", "out", "up", "off", "over", "at", "was", "were", "are", "am");   // (put in in an hour; the particles can double)
    for (int i = 0; i < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Word) continue;
        const int j = c.nextWord(i);
        if (j < 0 || c.tokens[j].kind != Tok::Word || c.tokens[i].lower != c.tokens[j].lower) continue;
        const QString between = c.text.mid(c.tokens[i].end, c.tokens[j].start - c.tokens[i].end);
        if (between.contains(QLatin1Char('\n')) || between.contains(QChar(0x2028)) || between.contains(QChar(0x2029))) continue;   // (across a line: left to the person)
        const QString &w = c.tokens[i].lower;
        if (w.size() < 2 || legitimate.contains(w)) continue;
        if (c.firstUpper(i) && c.firstUpper(j)) continue;   // (Bora Bora, Walla Walla: a name said twice)
        if (c.firstUpper(j)) continue;                       // (bank Bank One)
        if (c.firstUpper(i) && !functionWords().contains(w)) continue;   // (Context context: a type and a name; only "The the" and the like are a slip)
        if (c.tokens[j].start - c.tokens[i].end > 2 || c.text.mid(c.tokens[i].end, c.tokens[j].start - c.tokens[i].end).contains(QLatin1Char('\t'))) continue;   // (columns of a table)
        const QChar quoteBefore = c.tokens[i].start > 0 ? c.text.at(c.tokens[i].start - 1) : QChar();
        const QChar quoteAfter = c.tokens[j].end < c.text.size() ? c.text.at(c.tokens[j].end) : QChar();
        if (QStringView(u"\"'\u201C\u2018\u00AB\u201D\u2019\u00BB").contains(quoteBefore) || QStringView(u"\"'\u201C\u2018\u00AB\u201D\u2019\u00BB").contains(quoteAfter)) continue;   // (a word mentioned)
        bool digits = false;
        for (QChar ch : w) digits = digits || ch.isDigit();
        if (digits) continue;
        c.add(c.tokens[i].end, c.tokens[j].end, GK::Repetition, QCoreApplication::translate("Grammar", "Repeated word."), QString());
    }
}

// "a" before a vowel sound and "an" before a consonant sound.
enum class Sound { Vowel, Consonant, Unknown };

Sound letterName(QChar letter)
{
    const char l = char(letter.toLower().unicode());
    if (QLatin1String("aefhilmnorsx").contains(QLatin1Char(l))) return Sound::Vowel;   // the letter's own name: ay, ef, aitch, eye, el, em, en, oh, ar, es, ex
    if (l >= 'a' && l <= 'z') return Sound::Consonant;
    return Sound::Unknown;
}

bool startsWithAny(const QString &w, std::initializer_list<const char *> prefixes)
{
    for (const char *p : prefixes)
        if (w.startsWith(QLatin1String(p))) return true;
    return false;
}

Sound soundOfNumber(const QString &lower)
{
    QString digits;
    bool comma = false;
    int p = 0;
    for (; p < lower.size(); ++p) {
        const QChar ch = lower.at(p);
        if (ch.isDigit()) digits += ch;
        else if (ch == QLatin1Char(',')) comma = true;
        else if (ch == QLatin1Char('.')) continue;   // (8.5: the part before the point says it)
        else break;
    }
    if (digits.isEmpty() || digits.startsWith(QLatin1Char('0'))) return Sound::Unknown;   // (0x88, 03: said in more than one way)
    // What follows the digits: nothing, an ordinal ending (3rd, 18th), or an "s" (1990s); another letter makes a code (4K, 0x88).
    const QString rest = lower.mid(p);
    if (!rest.isEmpty() && rest != QLatin1String("s") && rest != QLatin1String("st") && rest != QLatin1String("nd") && rest != QLatin1String("rd") && rest != QLatin1String("th")) return Sound::Unknown;
    const int dot = int(lower.indexOf(QLatin1Char('.')));
    if (dot > 0) {   // (8.5: eight point five; 11.5 and 18.5 too)
        digits = digits.left(dot - (lower.left(dot).count(QLatin1Char(','))));
    }
    if (digits.size() == 4 && !comma) return Sound::Unknown;   // (a year, or a number: 1800 is read either way)
    if (digits.startsWith(QLatin1Char('8'))) return Sound::Vowel;
    if (digits.size() % 3 == 2 && (digits.startsWith(QLatin1String("11")) || digits.startsWith(QLatin1String("18")))) return Sound::Vowel;
    return Sound::Consonant;
}

// How the word at token j is said at its start. `shouting`: the words near it are in capitals too (a heading, not initials).
Sound soundOfWord(const Ctx &c, int j, bool shouting)
{
    const Token &t = c.tokens[j];
    if (t.kind == Tok::Number) return soundOfNumber(t.lower);
    const QString orig = c.original(j);
    // A single letter and a hyphen ("X-ray", "T-shirt"): the letter's name.
    const int hyphen = int(orig.indexOf(QLatin1Char('-')));
    if (hyphen == 1) return letterName(orig.at(0));
    const int apos = [&] { for (int k = 0; k < orig.size(); ++k) if (isApostrophe(orig.at(k))) return k; return int(orig.size()); }();
    const QString stem = orig.left(hyphen > 0 ? std::min(hyphen, apos) : apos);
    if (stem.isEmpty()) return Sound::Unknown;
    bool allCaps = stem.size() >= 1;
    for (QChar ch : stem) allCaps = allCaps && (ch.isUpper() || ch.isDigit());
    if (allCaps && stem.size() == 1) return letterName(stem.at(0));
    if (!allCaps)
        for (QChar ch : stem)
            if (ch.isDigit()) return Sound::Unknown;   // (u32, x86, h264: said by letter and number)
    const QString w = stem.toLower();
    if (allCaps) {
        // Initials, said by letter, or a word made of them.
        if (shouting) return Sound::Unknown;
        static const QSet<QString> words = {QStringLiteral("nasa"), QStringLiteral("nato"), QStringLiteral("opec"), QStringLiteral("unesco"), QStringLiteral("unicef"), QStringLiteral("laser"),
                                            QStringLiteral("radar"), QStringLiteral("scuba"), QStringLiteral("fifa"), QStringLiteral("aids"), QStringLiteral("nafta"), QStringLiteral("sars"),
                                            QStringLiteral("asap")};
        static const QSet<QString> either = {QStringLiteral("sql"), QStringLiteral("scsi"), QStringLiteral("faq"), QStringLiteral("gif"), QStringLiteral("jpeg")};
        static const QSet<QString> spelled = {QStringLiteral("mri"), QStringLiteral("fbi"), QStringLiteral("fda"), QStringLiteral("fcc"), QStringLiteral("fyi"), QStringLiteral("nfl"), QStringLiteral("nba"),
                                              QStringLiteral("nsa"), QStringLiteral("nhs"), QStringLiteral("mba"), QStringLiteral("mph"), QStringLiteral("mvp"), QStringLiteral("led"), QStringLiteral("lcd"),
                                              QStringLiteral("lgbt"), QStringLiteral("ssn"), QStringLiteral("sms"), QStringLiteral("sos"), QStringLiteral("rsvp"), QStringLiteral("rfid"), QStringLiteral("html"),
                                              QStringLiteral("hdmi"), QStringLiteral("hiv"), QStringLiteral("hr"), QStringLiteral("hq"), QStringLiteral("hd"), QStringLiteral("hvac"), QStringLiteral("fm"),
                                              QStringLiteral("fps"), QStringLiteral("rn"), QStringLiteral("mp3"), QStringLiteral("mpg")};
        if (either.contains(w)) return Sound::Unknown;
        if (!words.contains(w)) {
            const QChar first = w.at(0);
            if (QLatin1String("fhlmnrs").contains(first)) return spelled.contains(w) ? Sound::Vowel : Sound::Unknown;   // (letters whose name starts with a vowel: a word made of them may not)
            if (w.size() > 5) return Sound::Unknown;
            return letterName(first);
        }
    }
    const QChar first = w.at(0);
    if (first < QLatin1Char('a') || first > QLatin1Char('z')) return Sound::Unknown;
    {   // (no vowel, or an "h" and a consonant: Rr, hbook, hm: said by letter)
        bool vowel = false;
        for (QChar ch : w) vowel = vowel || QLatin1String("aeiouy").contains(ch);
        if (!vowel || (first == QLatin1Char('h') && w.size() > 1 && !QLatin1String("aeiouy").contains(w.at(1)))) return Sound::Unknown;
    }
    if (startsWithAny(w, {"histor", "herb", "hotel", "hispan", "yt"}) || (first == QLatin1Char('x'))) return Sound::Unknown;   // (said either way)
    if (first == QLatin1Char('h')) return startsWithAny(w, {"hour", "honest", "honor", "honour", "heir"}) ? Sound::Vowel : Sound::Consonant;
    if (!QLatin1String("aeiou").contains(first)) return Sound::Consonant;
    // A vowel letter, but a consonant sound: one, once, eu-, ewe, uni- (not "unin", "unim"), use, uti-, and the like.
    if (w == QLatin1String("one") || w == QLatin1String("once")) return Sound::Consonant;
    if (startsWithAny(w, {"euler", "eul\u00E9"}) && w.startsWith(QLatin1String("euler"))) return Sound::Vowel;   // (Euler: "oiler")
    if (startsWithAny(w, {"eu", "ewe", "unic", "unif", "unil", "unio", "uniq", "unis", "unit", "univ", "unix", "unanim", "use", "usa", "usu", "ute", "uti", "uto", "uta", "uru", "uga", "uku", "uk", "ufo", "ubiq", "uran", "urin", "uvul"}))
        return Sound::Consonant;
    return Sound::Vowel;
}

// Whether the sentence round token i is all capitals (a heading shouts: "A LOT OF IT" is not initials).
bool shoutingAt(const Ctx &c, int j)
{
    auto caps = [&](int k) {
        if (k < 0 || c.tokens[k].kind != Tok::Word) return false;
        const QString o = c.original(k);
        if (o.size() < 2) return false;
        for (QChar ch : o) if (ch.isLower()) return false;
        return true;
    };
    const int next = c.nextWord(j);
    const int prev = j >= 2 && c.isSpace(j - 1) ? j - 2 : -1;
    const int beforeArticle = prev >= 2 && c.isSpace(prev - 1) ? prev - 2 : -1;
    return caps(next) || caps(beforeArticle);
}

// Whether token i starts a sentence: the first token, or after a sentence mark (and any opening marks and spaces).
bool startsSentence(const Ctx &c, int i)
{
    int k = i - 1;
    while (k >= 0 && (c.tokens[k].kind == Tok::Space || (c.tokens[k].kind == Tok::Punct && QStringView(u"\"'([“‘«").contains(c.text.at(c.tokens[k].start))))) --k;
    return k < 0 || (c.tokens[k].kind == Tok::Punct && QLatin1String(".!?").contains(c.text.at(c.tokens[k].start)));
}

void articles(Ctx &c)
{
    const QSet<QString> &labels = WORDS("letter", "letters", "vowel", "vowels", "symbol", "character", "grade", "type", "vitamin", "plan", "option", "point", "section", "variable", "case", "part", "step", "item", "key",
                                        "class", "category", "group", "column", "row", "label", "word", "words", "sound", "note", "chord", "size", "figure", "table", "appendix", "exhibit", "schedule", "form", "level");
    const QSet<QString> &notNouns = WORDS("is", "are", "am", "and", "or", "as", "at", "in", "on", "if", "of", "it", "its", "i", "a", "an", "be", "to", "us", "up", "any", "all");
    for (int i = 0; i < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Word) continue;
        const QString &l = c.tokens[i].lower;
        if (l != QLatin1String("a") && l != QLatin1String("an")) continue;
        if (i > 0 && c.tokens[i - 1].kind == Tok::Punct && (c.isPunct(i - 1, QLatin1Char('/')) || c.isPunct(i - 1, QLatin1Char('.')))) continue;   // (d/b/a, a.k.a.: letters of a short form)
        const QString orig = c.original(i);
        const bool capital = orig.at(0).isUpper();
        if (orig.size() == 2 && orig.at(1).isUpper() && orig.at(0).isLower()) continue;
        if (capital && !startsSentence(c, i)) continue;   // (Vitamin A, Plan A)
        if (const int p = c.prevWord(i); p >= 0 && labels.contains(c.tokens[p].lower)) continue;   // (the letter a, grade a, type a: a name, not the article)
        const int j = c.nextWord(i);
        if (j < 0 || notNouns.contains(c.tokens[j].lower)) continue;
        const Sound s = soundOfWord(c, j, shoutingAt(c, j));
        if (s == Sound::Unknown) continue;
        if (l == QLatin1String("a") && s == Sound::Vowel)
            c.add(c.tokens[i].start, c.tokens[i].end, GK::Grammar, QCoreApplication::translate("Grammar", "Use \"an\" before a word that starts with a vowel sound."), matchCase(orig, QStringLiteral("an")));
        else if (l == QLatin1String("an") && s == Sound::Consonant)
            c.add(c.tokens[i].start, c.tokens[i].end, GK::Grammar, QCoreApplication::translate("Grammar", "Use \"a\" before a word that starts with a consonant sound."), matchCase(orig, QStringLiteral("a")));
    }
}

// A sentence that starts with a small letter, after a period (not after "!" or "?": "Yahoo! is", "Who? what?"), and not at the start of a paragraph, which may be a list item.
void lowercaseSentenceStartWith(Ctx &c, const QSet<QString> *shortened)
{
    auto lowercaseWord = [&](int k) {
        if (!c.isWord(k)) return false;
        const QString o = c.original(k);
        if (o.size() < 2 || !o.at(0).isLower()) return false;
        for (int p = 1; p < o.size(); ++p)
            if (o.at(p).isUpper()) return false;   // (iPhone, eBay)
        const int after = k + 1;
        if (after < c.size() && c.tokens[after].kind == Tok::Punct) {
            const QChar m = c.text.at(c.tokens[after].start);
            if (m == QLatin1Char('@') || m == QLatin1Char('/') || m == QLatin1Char(':') || m == QLatin1Char('\\')) return false;   // (an address)
            if (m == QLatin1Char('.') && after + 1 < c.size() && c.tokens[after + 1].kind != Tok::Space && c.tokens[after + 1].kind != Tok::Punct) return false;   // (a file name, www.)
            if (m == QLatin1Char('.') && after + 1 < c.size() && c.tokens[after + 1].kind == Tok::Punct) return false;   // ("e.g.", "i.e.")
        }
        return true;
    };
    for (int i = 1; i + 2 < c.size(); ++i) {
        c.step();
        if (!c.isPunct(i, QLatin1Char('.')) || c.tokens[i + 1].kind != Tok::Space) continue;
        const int w = i + 2;
        if (!lowercaseWord(w)) continue;
        const int before = i - 1;
        if (!c.isWord(before)) continue;
        const QString &b = c.tokens[before].lower;
        if (b.size() < 3 || abbreviations().contains(b) || (shortened && shortened->contains(b))) continue;
        // A shortened word ("org. code", "alt. conf. id", "Dougl. ex Torr.", "Camp. lari"): a short lowercase word, a name with a capital inside ("PhDs."), a word that is not
        // plain Latin letters, a capitalized word that stops in a cluster of consonants, or a paragraph with column leaders or a colon after the word.
        {
            const QString shown0 = c.original(before);
            bool latin = true, innerUpper = false;
            for (int q = 0; q < shown0.size(); ++q) {
                if (shown0.at(q).unicode() > 0x24F || !shown0.at(q).isLetter()) latin = false;
                if (q > 0 && shown0.at(q).isUpper()) innerUpper = true;
            }
            const QChar l1 = b.at(b.size() - 1), l2 = b.at(b.size() - 2);
            const bool cluster = b.size() >= 4 && !QLatin1String("aeiouy").contains(l1) && !QLatin1String("aeiouy").contains(l2) && shown0.at(0).isUpper();
            const bool shortLower = b.size() <= 3 || addressEndings().contains(c.tokens[w].lower);   // (a short word, or the end of an address with a space in it)
            const int after1 = w + 1;
            const bool leader = (after1 + 1 < c.size() && c.tokens[after1].kind == Tok::Space && c.tokens[after1 + 1].kind == Tok::Punct && c.text.at(c.tokens[after1 + 1].start) == QLatin1Char(':')) || c.hasLeaders();   // (a colon after it, or a table of contents with its leaders)
            if (!latin || innerUpper || cluster || shortLower || leader) continue;
        }
        if (before >= 1 && c.isPunct(before - 1, QLatin1Char('.'))) continue;
        if (b.contains(QLatin1Char('\'')) || b.contains(QChar(0x2019))) continue;   // (addt'l., int'l.: shortened words)
        const QString shown = c.original(before);
        if (shown.at(shown.size() - 1).isUpper() && (shown.size() == 1 || !shown.at(shown.size() - 2).isLetter())) continue;   // (post-E. coli: an initial)
        if (startsLowerForGood().contains(c.tokens[w].lower) || joiningWords().contains(c.tokens[w].lower)) continue;   // ("Admin. and Information": the sentence goes on)   // (pp. 72-107, et al.)
        const int s0 = c.tokens[w].start;
        c.add(s0, s0 + 1, GK::Capitalization, QCoreApplication::translate("Grammar", "Start a sentence with a capital letter."), QString(c.text.at(s0).toUpper()));
    }
}

// Calls f(i, j) for each word token i whose lowercase is in `firsts`, when the word after it (one space between) is in `nexts`.
template <class F>
void eachPair(Ctx &c, const QSet<QString> &firsts, const QSet<QString> &nexts, F f)
{
    for (int i = 0; i < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Word || !firsts.contains(c.tokens[i].lower)) continue;
        const int j = c.nextWord(i);
        if (j >= 0 && c.tokens[j].kind == Tok::Word && nexts.contains(c.tokens[j].lower)) f(i, j);
    }
}

void replaceToken(Ctx &c, int i, const QString &with, GK kind, const QString &message)
{
    c.add(c.tokens[i].start, c.tokens[i].end, kind, message, matchCase(c.original(i), with));
}

// its and it's.
void itsAndItIs(Ctx &c)
{
    eachPair(c, WORDS("its"), WORDS("a", "an", "the", "not", "been"), [&](int i, int) {
        replaceToken(c, i, QStringLiteral("it's"), GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"it's\" (it is) here, not \"its\"."));
    });
    eachPair(c, WORDS("it's"), WORDS("own"), [&](int i, int) {
        replaceToken(c, i, QStringLiteral("its"), GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"its\" (belonging to it) here, not \"it's\"."));
    });
}

// their, there, and they're.
void theirThereTheyAre(Ctx &c)
{
    eachPair(c, WORDS("their"), WORDS("is", "are", "was", "were"), [&](int i, int) {
        replaceToken(c, i, QStringLiteral("there"), GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"there\" here, not \"their\"."));
    });
    eachPair(c, WORDS("there", "they're"), WORDS("own"), [&](int i, int) {
        replaceToken(c, i, QStringLiteral("their"), GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"their\" (belonging to them) here."));
    });
}

// your and you're.
void yourYouAre(Ctx &c)
{
    for (int i = 0; i < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Word || c.tokens[i].lower != QLatin1String("your")) continue;
        const int j = c.nextWord(i);
        static const QSet<QString> nexts = {QStringLiteral("a"), QStringLiteral("an"), QStringLiteral("the"), QStringLiteral("not"), QStringLiteral("always"), QStringLiteral("never")};
        if (j < 0 || !nexts.contains(c.tokens[j].lower) || c.original(j) != c.tokens[j].lower) continue;   // (a capital letter: "Your A grade")
        if (c.tokens[j].lower == QLatin1String("a")) {   // (your a cappella, your a priori: Latin and Italian)
            const int k = c.nextWord(j);
            if (k >= 0 && (c.tokens[k].lower == QLatin1String("cappella") || c.tokens[k].lower == QLatin1String("priori") || c.tokens[k].lower == QLatin1String("posteriori") ||
                           c.tokens[k].lower == QLatin1String("fortiori") || c.tokens[k].lower == QLatin1String("la")))
                continue;
        }
        replaceToken(c, i, QStringLiteral("you're"), GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"you're\" (you are) here, not \"your\"."));
    }
    eachPair(c, WORDS("you're"), WORDS("own", "name"), [&](int i, int) {
        replaceToken(c, i, QStringLiteral("your"), GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"your\" (belonging to you) here, not \"you're\"."));
    });
}

// then and than.
void thenAndThan(Ctx &c)
{
    eachPair(c, WORDS("more", "less", "better", "worse", "rather", "other", "fewer", "greater", "smaller", "larger", "bigger", "higher", "lower", "faster", "slower", "older", "younger"), WORDS("then"),
             [&](int i, int j) {
                 if (c.ifBefore[i]) return;   // ("If you feel better then call me": then, the next thing)
                 replaceToken(c, j, QStringLiteral("than"), GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"than\" to compare."));
             });
    eachPair(c, WORDS("and"), WORDS("than"), [&](int, int j) { replaceToken(c, j, QStringLiteral("then"), GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"then\" (next) here, not \"than\".")); });
}

// lose and loose.
void loseAndLoose(Ctx &c)
{
    const QSet<QString> &before = WORDS("to", "will", "would", "could", "can", "should", "might", "may", "must", "don't", "doesn't", "didn't", "won't", "can't", "couldn't", "shouldn't", "wouldn't", "never", "not");
    const QSet<QString> &objects = WORDS("weight", "money", "track", "sight", "hope", "control", "faith", "touch", "patience", "interest", "focus", "sleep", "momentum", "heart", "hope");
    for (int i = 0; i < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Word || c.tokens[i].lower != QLatin1String("loose")) continue;
        const int p = c.prevWord(i), n = c.nextWord(i);
        if (p >= 0 && n >= 0 && before.contains(c.tokens[p].lower) && objects.contains(c.tokens[n].lower))
            replaceToken(c, i, QStringLiteral("lose"), GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"lose\" (not win, or let go of) here, not \"loose\"."));
    }
    eachPair(c, WORDS("too", "very"), WORDS("lose"), [&](int, int j) {
        replaceToken(c, j, QStringLiteral("loose"), GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"loose\" (not tight) here, not \"lose\"."));
    });
}

// "could of" and the like, for "could have".
void couldOf(Ctx &c)
{
    const QSet<QString> &verbs = WORDS("could", "should", "would", "must", "might");
    const QSet<QString> &notHave = WORDS("course", "the", "a", "an", "which", "whom", "whose", "these", "those", "them", "us", "you", "him", "her", "it", "all", "one", "any", "each", "both", "my", "your", "his",
                                         "our", "their", "this", "that", "what", "many", "some", "no", "his");
    const QSet<QString> &articles = WORDS("a", "the", "this", "that", "my", "your", "our", "their", "his", "her", "its");
    eachPair(c, verbs, WORDS("of"), [&](int i, int j) {
        const int k = c.nextWord(j);
        if (k >= 0 && notHave.contains(c.tokens[k].lower)) return;
        const int p = c.prevWord(i);
        if (p >= 0 && articles.contains(c.tokens[p].lower)) return;   // ("a must of the trade")
        c.add(c.tokens[j].start, c.tokens[j].end, GK::Grammar, QCoreApplication::translate("Grammar", "Use \"have\", not \"of\", after this word."), QStringLiteral("have"));
    });
}

void lowercaseSentenceStart(Ctx &c) { lowercaseSentenceStartWith(c, nullptr); }

// Two or more spaces between words.
void doubleSpace(Ctx &c)
{
    // A paragraph with a tab or a run of three spaces is set in columns (a table cell, a pasted table), and its double spaces are alignment.
    bool columns = c.text.contains(QLatin1Char('=')) || c.text.contains(QLatin1String("\",\""));   // (a formula: "P(U1  u1) = ...")
    int doubles = 0, words = 0;
    for (int i = 0; i < c.size() && !columns; ++i) {
        c.step();
        if (c.tokens[i].kind == Tok::Word) ++words;
        if (c.tokens[i].kind != Tok::Space) continue;
        const int n = c.tokens[i].end - c.tokens[i].start;
        if (n >= 3 || c.text.mid(c.tokens[i].start, n).contains(QLatin1Char('\t'))) columns = true;
        if (n == 2) ++doubles;
    }
    if (columns || (doubles >= 5 && doubles * 8 > words)) return;   // (a justified paragraph spreads its spaces: five and one in eight words is not typing)
    auto numeric = [&](int k) {   // (figures, marks, and one-letter symbols are a table or a formula: "800 -  859", "X  xi")
        const Token &t = c.tokens[k];
        return t.kind == Tok::Number || (t.kind == Tok::Punct && !QStringView(u"\"'([\u201C\u2018,;").contains(c.text.at(t.start)) && (c.text.at(t.start) != QLatin1Char('-') || c.isPunct(k - 1, QLatin1Char('-')) || c.isPunct(k + 1, QLatin1Char('-')))) || (t.kind == Tok::Word && t.end - t.start == 1 && c.text.at(t.start) != QLatin1Char('a') && c.text.at(t.start) != QLatin1Char('I') && c.text.at(t.start) != QLatin1Char('A'));
    };
    for (int i = 1; i + 1 < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Space || c.tokens[i].end - c.tokens[i].start != 2 || !c.plainSpaces(i)) continue;   // (three or more are an alignment)
        if (numeric(i - 1) || numeric(i + 1)) continue;
        // Two spaces after the end of a sentence are a way of typing, not a mistake.
        int k = i - 1;
        while (k > 0 && c.tokens[k].kind == Tok::Punct && QStringView(u"\"')]\u201D\u2019").contains(c.text.at(c.tokens[k].start))) --k;
        if (c.tokens[k].kind == Tok::Punct && QLatin1String(".!?:").contains(c.text.at(c.tokens[k].start))) continue;
        c.add(c.tokens[i].start, c.tokens[i].end, GK::Punctuation, QCoreApplication::translate("Grammar", "Use one space here."), QStringLiteral(" "));
    }
}

// A space before a comma, a period, or another mark that goes with the word before it.
void spaceBeforeMark(Ctx &c)
{
    for (int i = 1; i + 1 < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Space || !c.plainSpaces(i) || c.tokens[i].end - c.tokens[i].start != 1) continue;   // (a longer run is an alignment)
        const int m = i + 1;
        if (c.tokens[m].kind != Tok::Punct) continue;
        const QChar mark = c.text.at(c.tokens[m].start);
        if (!QLatin1String(",.;!?").contains(mark)) continue;   // (not the colon: "Name : John", "f : X -> Y" are spaced on purpose)
        const Tok before = c.tokens[i - 1].kind;
        if (before != Tok::Word && before != Tok::Number && !(before == Tok::Punct && (c.isPunct(i - 1, QLatin1Char(')')) || c.isPunct(i - 1, QLatin1Char(']'))))) continue;
        const int after = m + 1;
        if (after < c.size() && c.tokens[after].kind == Tok::Punct && !QStringView(u")]}\"'\u201D\u2019,.;:!?\u2026").contains(c.text.at(c.tokens[after].start))) continue;   // (?*frame*: code)
        if (mark == QLatin1Char('.') && after + 1 < c.size() && c.tokens[after].kind == Tok::Space && c.isPunct(after + 1, QLatin1Char('.'))) continue;   // (a spaced ellipsis, ". . .")
        if (after < c.size()) {
            const Token &a = c.tokens[after];
            if (a.kind == Tok::Word || a.kind == Tok::Number) continue;   // (.txt, 3 .5, :D)
            if (a.kind == Tok::Punct) {
                const QChar q = c.text.at(a.start);
                if (QLatin1String(",.;:!?").contains(q) || q == QChar(0x2026)) continue;   // (a run of marks)
                if (mark == QLatin1Char(':') || mark == QLatin1Char(';') || mark == QLatin1Char('.')) continue;   // (:) ;) and the like)
            }
        }
        if ((mark == QLatin1Char('?') || mark == QLatin1Char('!')) && before == Tok::Number) continue;   // (a value that is not known: "56.89 ?")
        if (mark == QLatin1Char(':') && before == Tok::Number && after + 1 < c.size() && c.tokens[after].kind == Tok::Space && c.tokens[after + 1].kind == Tok::Number) continue;   // (3 : 1)
        c.add(c.tokens[i].start, c.tokens[i].end, GK::Punctuation, QCoreApplication::translate("Grammar", "Take out the space before this mark."), QString());
    }
}

// No space between the period (or ? or !) that ends a sentence and the capital that begins the next.
void missingSpaceAfterPeriod(Ctx &c)
{
    // A word that has a period joined to a word after it three times or more in a paragraph is a name with parts (jess.Fact, jess.Value, jess.Rete), not slips of the keyboard.
    QHash<QString, int> joined;
    for (int i = 1; i + 1 < c.size(); ++i) {
        c.step();
        if (c.isPunct(i, QLatin1Char('.')) && c.isWord(i - 1) && c.isWord(i + 1)) ++joined[c.tokens[i - 1].lower];
    }
    for (int i = 1; i + 1 < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind == Tok::Punct && c.text.at(c.tokens[i].start) == QLatin1Char('.') && c.isWord(i - 1) && joined.value(c.tokens[i - 1].lower) >= 3) continue;
        if (c.tokens[i].kind != Tok::Punct) continue;
        const QChar mark = c.text.at(c.tokens[i].start);
        if (!QLatin1String(".?!").contains(mark)) continue;
        if (!c.isWord(i - 1) || !c.isWord(i + 1)) continue;
        const QString before = c.original(i - 1), after = c.original(i + 1);
        if (before.size() < 4 || !before.at(before.size() - 1).isLower() || before.contains(QLatin1Char('-'))) continue;   // (app.Run, obj.Method: code)
        if (after.size() < 2 || !after.at(0).isUpper() || after.mid(1) != after.mid(1).toLower()) continue;   // (Data!MyName, Sheet1!A1)
        if (mark == QLatin1Char('.') && (abbreviations().contains(c.tokens[i - 1].lower) || addressEndings().contains(c.tokens[i + 1].lower))) continue;
        if (!sentenceStarters().contains(c.tokens[i + 1].lower)) continue;   // (jess.Rete, Rete.Context: names; before a word that begins a sentence only)
        if (c.isPunct(i + 2, QLatin1Char('(')) || codeWords().contains(c.tokens[i - 1].lower)) continue;   // (Console.Log(x), this.State)
        if (c.chunkAddress[i] || (mark == QLatin1Char('.') && c.chunkDots[i] > 1)) continue;   // (an address, a file name, initials)
        c.add(c.tokens[i].start, c.tokens[i].end, GK::Punctuation, QCoreApplication::translate("Grammar", "Put a space after the end of a sentence."), QString(mark) + QLatin1Char(' '));
    }
}

// "alot".
void alot(Ctx &c)
{
    for (int i = 0; i < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind == Tok::Word && c.tokens[i].lower == QLatin1String("alot"))
            replaceToken(c, i, QStringLiteral("a lot"), GK::WordChoice, QCoreApplication::translate("Grammar", "\"A lot\" is two words."));
    }
}

// "everyday" for "every day" where the words round it make that clear ("almost everyday", "Everyday I walk"). The other way round is left alone: "my every day" is fine.
void everyday(Ctx &c)
{
    const QSet<QString> &adverbs = WORDS("almost", "nearly", "practically", "virtually");
    const QSet<QString> &pronouns = WORDS("i", "we", "you", "they", "he", "she");
    for (int i = 0; i < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Word) continue;
        const QString &l = c.tokens[i].lower;
        if (l == QLatin1String("everyday")) {
            const int p = c.prevWord(i), n = c.nextWord(i);
            if ((p >= 0 && adverbs.contains(c.tokens[p].lower)) || (n >= 0 && pronouns.contains(c.tokens[n].lower)))
                replaceToken(c, i, QStringLiteral("every day"), GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"every day\" (each day) here."));
        }
    }
}

// ---- more rules: agreement, word choice, grammar, punctuation, capitalization, and the style rules (which are off unless asked for)

// Whether a word is in capitals only ("NASA", "WAS": a heading shouts, and its words are not the ones the rules know).
bool allCaps(const Ctx &c, int i)
{
    const QString o = c.original(i);
    if (o.size() < 2) return false;
    for (QChar ch : o)
        if (ch.isLower()) return false;
    return true;
}

// Whether the words from token `first` to token `last` are in quotation marks or right against them (a word mentioned, or a character who talks that way).
bool quotedAround(const Ctx &c, int first, int last)
{
    const QChar b = c.tokens[first].start > 0 ? c.text.at(c.tokens[first].start - 1) : QChar();
    const QChar a = c.tokens[last].end < c.text.size() ? c.text.at(c.tokens[last].end) : QChar();
    return QStringView(u"\"“”‘«»").contains(b) || QStringView(u"\"“”‘«»").contains(a);
}

// The index of the last word of `words` (lowercase, one space between each) if they are the words from token i on, else -1.
int phraseAt(const Ctx &c, int i, const QStringList &words)
{
    int t = i;
    for (int k = 0; k < words.size(); ++k) {
        if (t < 0 || c.tokens[t].kind != Tok::Word || c.tokens[t].lower != words[k]) return -1;
        if (k + 1 < words.size()) t = c.nextWord(t);
    }
    return t;
}

// Phrases to look for, and what to put in place of each, found by the word they begin with.
class PhraseTable {
public:
    PhraseTable(std::initializer_list<std::pair<const char *, const char *>> items)
    {
        for (const auto &it : items) {
            Entry e;
            e.words = QString::fromUtf8(it.first).split(QLatin1Char(' '));
            e.with = QString::fromUtf8(it.second);
            m_by[e.words.first()].append(e);
        }
    }
    // Calls f(first, last, with) for each phrase in the text.
    template <class F>
    void each(Ctx &c, F f) const
    {
        for (int i = 0; i < c.size(); ++i) {
            c.step();
            if (c.tokens[i].kind != Tok::Word) continue;
            const auto it = m_by.constFind(c.tokens[i].lower);
            if (it == m_by.constEnd()) continue;
            for (const Entry &e : *it)
                if (const int last = phraseAt(c, i, e.words); last >= 0) f(i, last, e.with);
        }
    }

private:
    struct Entry {
        QStringList words;
        QString with;
    };
    QHash<QString, QVector<Entry>> m_by;
};

// Puts `with` (in the first letter's case of the words) in place of the words from token i to token j.
void replacePhrase(Ctx &c, int i, int j, const QString &with, GK kind, const QString &message)
{
    if (allCaps(c, i) || quotedAround(c, i, j)) return;
    c.add(c.tokens[i].start, c.tokens[j].end, kind, message, matchCase(c.original(i), with));
}

// An issue with words to say and nothing to put in place.
void note(Ctx &c, int start, int end, GK kind, const QString &message)
{
    GrammarIssue g;
    g.start = start;
    g.end = end;
    g.kind = kind;
    g.message = message;
    g.rule = QLatin1String(c.rule);
    c.issues << g;
}

// The sentences: [first, last] token indexes and the number of words. A period ends one only before a space and a capital, and not after an abbreviation or an initial.
struct Sentence {
    int first = 0, last = 0, words = 0;
};
QVector<Sentence> sentencesOf(Ctx &c)
{
    QVector<Sentence> out;
    Sentence s;
    bool open = false;
    for (int i = 0; i < c.size(); ++i) {
        c.step();
        const Token &t = c.tokens[i];
        if (t.kind == Tok::Space) continue;
        if (!open) {
            s = Sentence();
            s.first = i;
            open = true;
        }
        s.last = i;
        if (t.kind == Tok::Word || t.kind == Tok::Number) ++s.words;
        if (t.kind != Tok::Punct || !QLatin1String(".!?").contains(c.text.at(t.start))) continue;
        const bool period = c.text.at(t.start) == QLatin1Char('.');
        int n = i + 1;
        if (n < c.size() && c.tokens[n].kind == Tok::Punct && QStringView(u"\"')”’").contains(c.text.at(c.tokens[n].start))) ++n;
        if (n < c.size() && c.tokens[n].kind != Tok::Space) continue;
        if (n + 1 < c.size() && !c.firstUpper(n + 1) && c.tokens[n + 1].kind == Tok::Word) continue;
        if (period && i > 0 && c.tokens[i - 1].kind == Tok::Word && (abbreviations().contains(c.tokens[i - 1].lower) || c.tokens[i - 1].end - c.tokens[i - 1].start == 1)) continue;
        out << s;
        open = false;
    }
    if (open) out << s;
    return out;
}

// he don't, she don't, it don't.
void heDont(Ctx &c)
{
    eachPair(c, WORDS("he", "she", "it"), WORDS("don't"), [&](int, int j) {
        replacePhrase(c, j, j, QStringLiteral("doesn't"), GK::Grammar, QCoreApplication::translate("Grammar", "Use \"doesn't\" after \"he\", \"she\", and \"it\"."));
    });
}

// they was, we was. ("you" is left out of all of these: "What I like about you was" has "you" as the object.)
void theyWas(Ctx &c)
{
    eachPair(c, WORDS("they", "we"), WORDS("was"), [&](int, int j) {
        replacePhrase(c, j, j, QStringLiteral("were"), GK::Grammar, QCoreApplication::translate("Grammar", "Use \"were\" after \"they\" and \"we\"."));
    });
    eachPair(c, WORDS("they", "we"), WORDS("wasn't"), [&](int, int j) {
        replacePhrase(c, j, j, QStringLiteral("weren't"), GK::Grammar, QCoreApplication::translate("Grammar", "Use \"weren't\" after \"they\" and \"we\"."));
    });
}

// he have, she have, it have: where the word before shows that "have" is the verb of its sentence (a conjunction, or a new sentence), not an answer to "does" or "that" (he have).
void sheHave(Ctx &c)
{
    const QSet<QString> &conjunctions = WORDS("and", "but", "so", "because", "when", "while", "although", "though", "since", "once", "then", "where", "which", "whereas", "or", "yet", "after", "before", "until");
    eachPair(c, WORDS("he", "she", "it"), WORDS("have"), [&](int i, int j) {
        const int p = c.prevWord(i);
        const bool open = (p < 0 && (i == 0 || c.isPunct(i - 1, QLatin1Char(',')) || startsSentence(c, i))) || (p >= 0 && conjunctions.contains(c.tokens[p].lower));
        if (!open) return;
        replacePhrase(c, j, j, QStringLiteral("has"), GK::Grammar, QCoreApplication::translate("Grammar", "Use \"has\" after \"he\", \"she\", and \"it\"."));
    });
}

// they has, we has.
void pluralHas(Ctx &c)
{
    eachPair(c, WORDS("they", "we"), WORDS("has"), [&](int i, int j) {
        const int p = c.prevWord(i);
        if (p >= 0 && WORDS("like", "unlike", "than", "as").contains(c.tokens[p].lower)) return;
        replacePhrase(c, j, j, QStringLiteral("have"), GK::Grammar, QCoreApplication::translate("Grammar", "Use \"have\" after \"they\" and \"we\"."));
    });
}

// they does, we does, and "doesn't".
void pluralDoes(Ctx &c)
{
    auto fix = [&](int i, int j, const QString &with) {
        const int p = c.prevWord(i);
        if (p >= 0 && WORDS("like", "unlike", "than", "as").contains(c.tokens[p].lower)) return;
        replacePhrase(c, j, j, with, GK::Grammar, QCoreApplication::translate("Grammar", "Use \"do\" after \"they\" and \"we\"."));
    };
    eachPair(c, WORDS("they", "we"), WORDS("does"), [&](int i, int j) { fix(i, j, QStringLiteral("do")); });
    eachPair(c, WORDS("they", "we"), WORDS("doesn't"), [&](int i, int j) { fix(i, j, QStringLiteral("don't")); });
}

// they is, we is.
void theyIs(Ctx &c)
{
    auto fix = [&](int i, int j, const QString &with) {
        const int p = c.prevWord(i);
        if (p >= 0 && WORDS("like", "unlike", "than", "as").contains(c.tokens[p].lower)) return;
        replacePhrase(c, j, j, with, GK::Grammar, QCoreApplication::translate("Grammar", "Use \"are\" after \"they\" and \"we\"."));
    };
    eachPair(c, WORDS("they", "we"), WORDS("is"), [&](int i, int j) { fix(i, j, QStringLiteral("are")); });
    eachPair(c, WORDS("they", "we"), WORDS("isn't"), [&](int i, int j) { fix(i, j, QStringLiteral("aren't")); });
}

// "there is many", "there was several": a plural that needs "are" or "were" ("there is many a man" is right).
void thereIsMany(Ctx &c)
{
    for (int i = 0; i < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Word || c.tokens[i].lower != QLatin1String("there")) continue;
        const int j = c.nextWord(i);
        if (j < 0 || (c.tokens[j].lower != QLatin1String("is") && c.tokens[j].lower != QLatin1String("was"))) continue;
        const int k = c.nextWord(j);
        if (k < 0 || !WORDS("many", "several", "numerous", "multiple", "countless").contains(c.tokens[k].lower)) continue;
        const int m = c.nextWord(k);
        if (m >= 0 && (c.tokens[m].lower == QLatin1String("a") || c.tokens[m].lower == QLatin1String("an"))) continue;
        replacePhrase(c, j, j, c.tokens[j].lower == QLatin1String("is") ? QStringLiteral("are") : QStringLiteral("were"), GK::Grammar,
                      QCoreApplication::translate("Grammar", "Use \"are\" or \"were\" before a plural."));
    }
}

// "everyone are", "somebody don't": a word for one person takes a verb for one.
void indefiniteAre(Ctx &c)
{
    const QSet<QString> &ones = WORDS("everyone", "everybody", "someone", "somebody", "anyone", "anybody", "nobody");
    eachPair(c, ones, WORDS("are"), [&](int, int j) {
        replacePhrase(c, j, j, QStringLiteral("is"), GK::Grammar, QCoreApplication::translate("Grammar", "Use \"is\" after a word for one person (\"everyone\", \"somebody\")."));
    });
    eachPair(c, ones, WORDS("don't"), [&](int, int j) {
        replacePhrase(c, j, j, QStringLiteral("doesn't"), GK::Grammar, QCoreApplication::translate("Grammar", "Use \"doesn't\" after a word for one person (\"everyone\", \"somebody\")."));
    });
}

// who's and whose.
void whosAndWhose(Ctx &c)
{
    eachPair(c, WORDS("who's"), WORDS("car", "house", "book", "name", "idea", "fault", "turn", "job", "mother", "father", "friend", "phone", "dog", "cat", "money", "opinion", "wife", "husband", "son",
                                      "daughter", "team", "company", "office", "shoes", "bag", "keys", "coat", "address", "birthday", "side", "responsibility", "decision", "business"),
             [&](int i, int) { replaceToken(c, i, QStringLiteral("whose"), GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"whose\" (belonging to whom) here, not \"who's\".")); });
    eachPair(c, WORDS("whose"), WORDS("going", "coming", "been", "being", "doing", "gonna", "ready", "here", "there", "not", "the", "a", "an", "got"), [&](int i, int) {
        replaceToken(c, i, QStringLiteral("who's"), GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"who's\" (who is) here, not \"whose\"."));
    });
}

// affect and effect, where the word after (or the word before) says which is meant.
void affectAndEffect(Ctx &c)
{
    const QSet<QString> &determiners = WORDS("an", "the", "no", "any", "its", "their", "this", "that", "negative", "positive", "adverse", "significant", "major", "big", "great", "little", "huge", "direct", "real",
                                             "lasting", "profound", "minimal", "overall");
    for (int i = 0; i < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Word) continue;
        const QString &l = c.tokens[i].lower;
        const int p = c.prevWord(i), n = c.nextWord(i);
        if (l == QLatin1String("affect") && p >= 0 && n >= 0 && determiners.contains(c.tokens[p].lower) && c.tokens[n].lower == QLatin1String("on"))
            replaceToken(c, i, QStringLiteral("effect"), GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"effect\" (a result) here, not \"affect\"."));
        else if (l == QLatin1String("effect") && p >= 0 && n >= 0) {
            const QString &before = c.tokens[p].lower, &after = c.tokens[n].lower;
            const bool modal = WORDS("will", "would", "could", "can", "may", "might", "won't", "can't", "doesn't", "didn't").contains(before);
            if (modal && WORDS("me", "us", "you", "him", "them").contains(after))
                replaceToken(c, i, QStringLiteral("affect"), GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"affect\" (to change) here, not \"effect\"."));
        }
        if (l == QLatin1String("effect") && p >= 0 && c.prevWord(p) >= 0 && WORDS("will", "would", "could", "can", "may", "might").contains(c.tokens[c.prevWord(p)].lower) &&
            WORDS("negatively", "positively", "adversely").contains(c.tokens[p].lower))
            replaceToken(c, i, QStringLiteral("affect"), GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"affect\" (to change) here, not \"effect\"."));
    }
}

const QSet<QString> &countNouns()
{
    return WORDS("people", "items", "cars", "students", "children", "errors", "mistakes", "problems", "hours", "minutes", "dollars", "calories", "customers", "employees", "words", "pages", "times", "days",
                 "years", "ways", "things", "options", "choices", "books", "accidents", "injuries", "deaths", "jobs", "houses", "users", "visitors", "patients", "cases", "files", "emails", "requests", "calls",
                 "votes", "questions", "reasons", "examples", "friends", "kids", "places", "ideas");
}

// "less people" for "fewer people".
void lessAndFewer(Ctx &c)
{
    for (int i = 0; i < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Word || c.tokens[i].lower != QLatin1String("less")) continue;
        const int n = c.nextWord(i);
        if (n < 0 || !countNouns().contains(c.tokens[n].lower)) continue;
        replaceToken(c, i, QStringLiteral("fewer"), GK::Grammar, QCoreApplication::translate("Grammar", "Use \"fewer\" for things you can count."));
    }
}

// "amount of people" for "number of people" (the old rule: "amount" is for what is measured, "number" for what is counted).
void amountAndNumber(Ctx &c)
{
    eachPair(c, WORDS("amount"), WORDS("of"), [&](int i, int j) {
        const int k = c.nextWord(j);
        if (k < 0 || !countNouns().contains(c.tokens[k].lower)) return;
        replaceToken(c, i, QStringLiteral("number"), GK::Style, QCoreApplication::translate("Grammar", "Use \"number\" for things you can count."));
    });
}

// "much people" for "many people".
void muchAndMany(Ctx &c)
{
    for (int i = 0; i < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Word || c.tokens[i].lower != QLatin1String("much")) continue;
        const int n = c.nextWord(i);
        if (n < 0 || !countNouns().contains(c.tokens[n].lower)) continue;
        replaceToken(c, i, QStringLiteral("many"), GK::Grammar, QCoreApplication::translate("Grammar", "Use \"many\" for things you can count."));
    }
}

// "irregardless".
void irregardless(Ctx &c)
{
    for (int i = 0; i < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind == Tok::Word && c.tokens[i].lower == QLatin1String("irregardless"))
            replaceToken(c, i, QStringLiteral("regardless"), GK::WordChoice, QCoreApplication::translate("Grammar", "\"Regardless\" is the word; \"irregardless\" is not standard."));
    }
}

// "weather" for "whether": "know weather to", "weather or not".
void whetherAndWeather(Ctx &c)
{
    const QSet<QString> &verbs = WORDS("know", "ask", "asked", "asking", "wonder", "wondering", "decide", "decided", "determine", "unsure", "sure", "regardless", "irrespective", "matter", "say", "tell", "check");
    for (int i = 0; i < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Word || c.tokens[i].lower != QLatin1String("weather")) continue;
        const int p = c.prevWord(i), n = c.nextWord(i);
        if (n < 0) continue;
        const QString &after = c.tokens[n].lower;
        bool whether = false;
        if (after == QLatin1String("or")) {
            const int m = c.nextWord(n);
            whether = m >= 0 && c.tokens[m].lower == QLatin1String("not") && ((p >= 0 && (verbs.contains(c.tokens[p].lower) || WORDS("of", "on", "about", "if").contains(c.tokens[p].lower))) || (p < 0 && startsSentence(c, i)));
        } else if (p >= 0 && verbs.contains(c.tokens[p].lower) && WORDS("to", "we", "you", "they", "he", "she", "it", "i", "this", "that", "there", "your", "my", "our", "his", "her", "their").contains(after))
            whether = true;
        if (whether) replaceToken(c, i, QStringLiteral("whether"), GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"whether\" (if) here, not \"weather\"."));
    }
}

// "quiet a few" for "quite a few".
void quiteAndQuiet(Ctx &c)
{
    for (int i = 0; i < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Word || c.tokens[i].lower != QLatin1String("quiet")) continue;
        const int a = c.nextWord(i);
        if (a < 0 || c.tokens[a].lower != QLatin1String("a")) continue;
        const int b = c.nextWord(a);
        if (b < 0 || (c.tokens[b].lower != QLatin1String("few") && c.tokens[b].lower != QLatin1String("lot"))) continue;
        const int p = c.prevWord(i);
        if (p >= 0 && WORDS("keep", "kept", "be", "stay", "stayed", "remain", "remained", "keeps", "stays", "stand", "stood").contains(c.tokens[p].lower)) continue;
        replaceToken(c, i, QStringLiteral("quite"), GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"quite\" (very) here, not \"quiet\"."));
    }
}

// "will chose" for "will choose".
void choseAndChoose(Ctx &c)
{
    eachPair(c, WORDS("will", "would", "can", "could", "should", "may", "might", "must", "to", "please", "can't", "won't", "cannot", "didn't", "don't", "doesn't", "wouldn't", "couldn't", "shouldn't"),
             WORDS("chose"), [&](int, int j) { replaceToken(c, j, QStringLiteral("choose"), GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"choose\" here; \"chose\" is the past.")); });
}

// advice (the noun) and advise (the verb).
void adviceAndAdvise(Ctx &c)
{
    eachPair(c, WORDS("the", "some", "any", "your", "my", "his", "our", "their", "good", "bad", "great", "best", "free", "legal", "medical", "professional", "expert", "sound", "wise"), WORDS("advise"),
             [&](int, int j) { replaceToken(c, j, QStringLiteral("advice"), GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"advice\" (the noun) here.")); });
    for (int i = 0; i < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Word || c.tokens[i].lower != QLatin1String("advice")) continue;
        const int p = c.prevWord(i), n = c.nextWord(i);
        if (p < 0 || n < 0 || !WORDS("i", "we", "they", "to", "will", "would", "can", "could", "should", "may", "might").contains(c.tokens[p].lower)) continue;
        if (!WORDS("you", "him", "them", "us", "me", "that", "against").contains(c.tokens[n].lower)) continue;
        replaceToken(c, i, QStringLiteral("advise"), GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"advise\" (the verb) here."));
    }
}

// "it is to much" for "it is too much": after a word that says what something is, and before the end of the clause ("amount to much" and "to much of" are right).
void toMuch(Ctx &c)
{
    eachPair(c, WORDS("to"), WORDS("much"), [&](int i, int j) {
        const int p = c.prevWord(i);
        if (p < 0 || !WORDS("is", "are", "was", "were", "be", "it's", "that's", "there's", "seems", "seem", "feels", "feel", "felt", "looks", "been", "being").contains(c.tokens[p].lower)) return;
        const int after = j + 1;
        bool end = after >= c.size() || (c.tokens[after].kind == Tok::Punct && QLatin1String(".,!?;").contains(c.text.at(c.tokens[after].start)));
        if (!end) {
            const int m = c.nextWord(j);
            end = m >= 0 && WORDS("for", "to", "and", "but").contains(c.tokens[m].lower);
        }
        if (end) replaceToken(c, i, QStringLiteral("too"), GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"too\" (more than enough) here, not \"to\"."));
    });
}

// "want too go" for "want to go".
void tooAndTo(Ctx &c)
{
    eachPair(c, WORDS("want", "wants", "wanted", "need", "needs", "needed", "going", "try", "tried", "trying", "able", "hope", "hoped", "plan", "plans", "decided", "like", "love"), WORDS("too"), [&](int, int j) {
        const int k = c.nextWord(j);
        if (k < 0 || !WORDS("be", "do", "go", "see", "get", "make", "have", "take", "find", "help", "know", "use", "say", "come", "give", "read", "write", "buy", "learn", "meet", "talk").contains(c.tokens[k].lower)) return;
        replaceToken(c, j, QStringLiteral("to"), GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"to\" here, not \"too\"."));
    });
}

// "can't breath" for "can't breathe".
void breathAndBreathe(Ctx &c)
{
    const QSet<QString> &before = WORDS("can't", "couldn't", "cannot", "to", "will", "would", "could", "can", "should", "must", "might", "may", "didn't", "don't", "won't");
    for (int i = 0; i < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Word || c.tokens[i].lower != QLatin1String("breath")) continue;
        const int p = c.prevWord(i), n = c.nextWord(i);
        if (p < 0 || !before.contains(c.tokens[p].lower)) continue;
        const bool follows = n >= 0 && WORDS("in", "out", "the", "again", "easy", "easier", "deeply", "normally", "properly", "fresh", "air", "slowly").contains(c.tokens[n].lower);
        const bool end = n < 0 && i + 1 < c.size() && c.tokens[i + 1].kind == Tok::Punct && QLatin1String(".!?").contains(c.text.at(c.tokens[i + 1].start)) && c.tokens[p].lower != QLatin1String("to");
        if (follows || end) replaceToken(c, i, QStringLiteral("breathe"), GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"breathe\" (the verb) here."));
    }
}

// Words run together that are two: "infact", "eachother", "atleast".
void runTogether(Ctx &c)
{
    static const QHash<QString, QString> table = {{QStringLiteral("infact"), QStringLiteral("in fact")},       {QStringLiteral("eachother"), QStringLiteral("each other")},
                                                  {QStringLiteral("atleast"), QStringLiteral("at least")},     {QStringLiteral("aswell"), QStringLiteral("as well")},
                                                  {QStringLiteral("inspite"), QStringLiteral("in spite")},     {QStringLiteral("noone"), QStringLiteral("no one")},
                                                  {QStringLiteral("everytime"), QStringLiteral("every time")}, {QStringLiteral("alittle"), QStringLiteral("a little")},
                                                  {QStringLiteral("thankyou"), QStringLiteral("thank you")},   {QStringLiteral("abit"), QStringLiteral("a bit")}};
    for (int i = 0; i < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Word) continue;
        const auto it = table.constFind(c.tokens[i].lower);
        if (it == table.constEnd() || c.chunkAddress[i] || c.chunkDots[i] > 0) continue;
        replacePhrase(c, i, i, *it, GK::WordChoice, QCoreApplication::translate("Grammar", "These are two words."));
    }
}

// Sayings with a word wrong: "for all intensive purposes", "one in the same".
void idioms(Ctx &c)
{
    static const PhraseTable table({{"for all intensive purposes", "for all intents and purposes"}, {"one in the same", "one and the same"}, {"case and point", "case in point"},
                                    {"nip it in the butt", "nip it in the bud"}, {"deep seeded", "deep-seated"}, {"wet your appetite", "whet your appetite"}, {"free reign", "free rein"},
                                    {"tow the line", "toe the line"}, {"pass mustard", "pass muster"}, {"mute point", "moot point"}, {"head over heals", "head over heels"},
                                    {"sneak peak", "sneak peek"}, {"all of the sudden", "all of a sudden"}, {"peaked my interest", "piqued my interest"},
                                    {"peaked your interest", "piqued your interest"}, {"peaked his interest", "piqued his interest"}, {"peaked her interest", "piqued her interest"},
                                    {"peaked their interest", "piqued their interest"}});
    table.each(c, [&](int i, int j, const QString &with) {
        replacePhrase(c, i, j, with, GK::WordChoice, QCoreApplication::translate("Grammar", "This saying has a different word."));
    });
}

// "this things" for "these things", "these thing" for "this thing": only where the noun ends the phrase, so that "this years program" (a missing apostrophe) and "these day trips" are left alone.
void demonstrativeNumber(Ctx &c)
{
    static const QHash<QString, QString> singular = {{QStringLiteral("things"), QStringLiteral("thing")},       {QStringLiteral("ideas"), QStringLiteral("idea")},     {QStringLiteral("items"), QStringLiteral("item")},
                                                     {QStringLiteral("students"), QStringLiteral("student")},  {QStringLiteral("cars"), QStringLiteral("car")},       {QStringLiteral("books"), QStringLiteral("book")},
                                                     {QStringLiteral("days"), QStringLiteral("day")},          {QStringLiteral("years"), QStringLiteral("year")},     {QStringLiteral("ways"), QStringLiteral("way")},
                                                     {QStringLiteral("problems"), QStringLiteral("problem")},  {QStringLiteral("questions"), QStringLiteral("question")}, {QStringLiteral("reasons"), QStringLiteral("reason")},
                                                     {QStringLiteral("examples"), QStringLiteral("example")},  {QStringLiteral("people"), QStringLiteral("person")}, {QStringLiteral("children"), QStringLiteral("child")}};
    QSet<QString> singulars;
    for (auto it = singular.cbegin(); it != singular.cend(); ++it) singulars.insert(it.value());
    const QSet<QString> &endsPhrase = WORDS("is", "was", "are", "were", "can", "will", "would", "should", "could", "must", "may", "might", "and", "or", "of", "in", "on", "for", "to", "with", "that", "which", "who", "have",
                                            "had", "do", "did", "make", "get", "go", "come", "seem", "need", "mean", "show", "look", "work", "happen", "matter");
    for (int i = 0; i < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Word) continue;
        const QString &l = c.tokens[i].lower;
        const bool thisL = l == QLatin1String("this"), theseL = l == QLatin1String("these"), thoseL = l == QLatin1String("those");
        if (!thisL && !theseL && !thoseL) continue;
        const int n = c.nextWord(i);
        if (n < 0 || c.tokens[n].kind != Tok::Word) continue;
        const int after = c.nextWord(n);
        if (after >= 0 && !endsPhrase.contains(c.tokens[after].lower)) continue;
        if (thisL && singular.contains(c.tokens[n].lower))
            replaceToken(c, i, QStringLiteral("these"), GK::Grammar, QCoreApplication::translate("Grammar", "Use \"these\" before a plural."));
        else if ((theseL || thoseL) && singulars.contains(c.tokens[n].lower))
            replaceToken(c, i, theseL ? QStringLiteral("this") : QStringLiteral("that"), GK::Grammar, QCoreApplication::translate("Grammar", "Use \"this\" or \"that\" before one."));
    }
}

// "between you and I" for "between you and me".
void betweenYouAndI(Ctx &c)
{
    for (int i = 0; i < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Word || !WORDS("between", "for", "with", "to", "from", "about", "of", "beside", "near", "behind", "around", "without").contains(c.tokens[i].lower)) continue;
        const int a = c.nextWord(i);
        if (a < 0 || !WORDS("you", "him", "her", "them").contains(c.tokens[a].lower)) continue;
        const int b = c.nextWord(a);
        if (b < 0 || c.tokens[b].lower != QLatin1String("and")) continue;
        const int d = c.nextWord(b);
        if (d < 0 || c.original(d) != QLatin1String("I")) continue;
        const int e = c.nextWord(d);
        if (e >= 0 && WORDS("am", "was", "will", "would", "can", "could", "have", "had", "are", "were", "do", "did", "should", "must", "may", "might", "shall").contains(c.tokens[e].lower)) continue;
        c.add(c.tokens[d].start, c.tokens[d].end, GK::Grammar, QCoreApplication::translate("Grammar", "Use \"me\" after a word like \"between\" or \"for\"."), QStringLiteral("me"));
    }
}

// "Me and him went" for "He and I went".
void meAndHim(Ctx &c)
{
    const QSet<QString> &verbs = WORDS("went", "are", "were", "was", "have", "had", "will", "did", "do", "can", "could", "would", "should", "got", "always", "never", "both", "love", "like", "want", "need", "go",
                                       "ran", "took", "made", "saw", "came", "played", "decided", "thought", "think", "know", "felt", "walked", "talked", "stayed");
    static const QHash<QString, QString> subject = {{QStringLiteral("him"), QStringLiteral("he")}, {QStringLiteral("her"), QStringLiteral("she")}, {QStringLiteral("them"), QStringLiteral("they")},
                                                    {QStringLiteral("you"), QStringLiteral("you")}};
    for (int i = 0; i < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Word || !startsSentence(c, i)) continue;
        const QString &l = c.tokens[i].lower;
        const int a = c.nextWord(i);
        if (a < 0 || c.tokens[a].lower != QLatin1String("and")) continue;
        const int b = c.nextWord(a);
        if (b < 0) continue;
        QString who;
        int last = b;
        if (l == QLatin1String("me") && subject.contains(c.tokens[b].lower)) who = subject.value(c.tokens[b].lower);
        else if (subject.contains(l) && l != QLatin1String("you") && c.tokens[b].lower == QLatin1String("me")) who = subject.value(l);
        else continue;
        const int v = c.nextWord(last);
        if (v < 0 || !verbs.contains(c.tokens[v].lower) || quotedAround(c, i, last)) continue;
        c.add(c.tokens[i].start, c.tokens[last].end, GK::Grammar, QCoreApplication::translate("Grammar", "Use \"%1 and I\" for the ones who do it.").arg(who), matchCase(c.original(i), who + QStringLiteral(" and I")));
    }
}

// "more better", "most best".
void doubleComparative(Ctx &c)
{
    const QSet<QString> &more = WORDS("better", "worse", "faster", "slower", "easier", "harder", "bigger", "larger", "smaller", "higher", "lower", "cheaper", "stronger", "weaker", "happier", "older", "younger",
                                      "taller", "shorter", "longer", "simpler", "safer", "nicer", "cleaner", "quicker", "wiser", "smarter", "brighter", "darker", "richer", "poorer", "heavier", "lighter", "closer", "newer");
    const QSet<QString> &most = WORDS("best", "worst", "easiest", "hardest", "biggest", "largest", "smallest", "highest", "lowest", "cheapest", "strongest", "fastest", "slowest");
    auto fix = [&](int i, int j) { replacePhrase(c, i, j, c.tokens[j].lower, GK::Grammar, QCoreApplication::translate("Grammar", "Use one comparison, not two (\"better\", not \"more better\").")); };
    eachPair(c, WORDS("more"), more, [&](int i, int j) { fix(i, j); });
    eachPair(c, WORDS("most"), most, [&](int i, int j) { fix(i, j); });
}

const QHash<QString, QString> &pastToBase()
{
    static const QHash<QString, QString> t = {{QStringLiteral("went"), QStringLiteral("go")},       {QStringLiteral("came"), QStringLiteral("come")},     {QStringLiteral("took"), QStringLiteral("take")},
                                              {QStringLiteral("gave"), QStringLiteral("give")},     {QStringLiteral("wrote"), QStringLiteral("write")},   {QStringLiteral("knew"), QStringLiteral("know")},
                                              {QStringLiteral("began"), QStringLiteral("begin")},   {QStringLiteral("spoke"), QStringLiteral("speak")},   {QStringLiteral("drove"), QStringLiteral("drive")},
                                              {QStringLiteral("flew"), QStringLiteral("fly")},      {QStringLiteral("swam"), QStringLiteral("swim")},     {QStringLiteral("sang"), QStringLiteral("sing")},
                                              {QStringLiteral("broke"), QStringLiteral("break")},   {QStringLiteral("forgot"), QStringLiteral("forget")}, {QStringLiteral("ran"), QStringLiteral("run")},
                                              {QStringLiteral("ate"), QStringLiteral("eat")}};
    return t;
}

// "did went" for "did go", "couldn't came" for "couldn't come".
void modalPast(Ctx &c)
{
    const QSet<QString> &before = WORDS("did", "didn't", "does", "doesn't", "do", "don't", "could", "should", "would", "couldn't", "wouldn't", "shouldn't", "won't", "must", "might");
    for (int i = 0; i < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Word || !before.contains(c.tokens[i].lower)) continue;
        const int n = c.nextWord(i);
        if (n < 0 || c.tokens[n].kind != Tok::Word || !pastToBase().contains(c.tokens[n].lower)) continue;
        replacePhrase(c, n, n, pastToBase().value(c.tokens[n].lower), GK::Grammar, QCoreApplication::translate("Grammar", "Use the plain form of the verb here (\"did go\", not \"did went\")."));
    }
}

// "has went" for "has gone", "have took" for "have taken".
void havePast(Ctx &c)
{
    static const QHash<QString, QString> participle = {{QStringLiteral("went"), QStringLiteral("gone")},   {QStringLiteral("wrote"), QStringLiteral("written")}, {QStringLiteral("took"), QStringLiteral("taken")},
                                                       {QStringLiteral("gave"), QStringLiteral("given")},   {QStringLiteral("ate"), QStringLiteral("eaten")},      {QStringLiteral("came"), QStringLiteral("come")},
                                                       {QStringLiteral("knew"), QStringLiteral("known")},   {QStringLiteral("began"), QStringLiteral("begun")},    {QStringLiteral("spoke"), QStringLiteral("spoken")},
                                                       {QStringLiteral("drove"), QStringLiteral("driven")}, {QStringLiteral("broke"), QStringLiteral("broken")},   {QStringLiteral("chose"), QStringLiteral("chosen")},
                                                       {QStringLiteral("forgot"), QStringLiteral("forgotten")}, {QStringLiteral("flew"), QStringLiteral("flown")}, {QStringLiteral("sang"), QStringLiteral("sung")},
                                                       {QStringLiteral("ran"), QStringLiteral("run")},      {QStringLiteral("drank"), QStringLiteral("drunk")},   {QStringLiteral("stole"), QStringLiteral("stolen")},
                                                       {QStringLiteral("wore"), QStringLiteral("worn")},    {QStringLiteral("tore"), QStringLiteral("torn")},     {QStringLiteral("fell"), QStringLiteral("fallen")},
                                                       {QStringLiteral("grew"), QStringLiteral("grown")},   {QStringLiteral("threw"), QStringLiteral("thrown")}};
    const QSet<QString> &have = WORDS("have", "has", "had", "haven't", "hasn't", "hadn't");
    for (int i = 0; i < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Word) continue;
        const QString &l = c.tokens[i].lower;
        if (!have.contains(l) && !l.endsWith(QLatin1String("'ve"))) continue;
        const int n = c.nextWord(i);
        if (n < 0 || c.tokens[n].kind != Tok::Word || !participle.contains(c.tokens[n].lower)) continue;
        replacePhrase(c, n, n, participle.value(c.tokens[n].lower), GK::Grammar, QCoreApplication::translate("Grammar", "Use the form of the verb that goes with \"have\" (\"has gone\", not \"has went\")."));
    }
}

// "can't hardly" for "can hardly", and "don't have no money" for "don't have any money".
void doubleNegative(Ctx &c)
{
    static const QHash<QString, QString> positive = {{QStringLiteral("can't"), QStringLiteral("can")},       {QStringLiteral("couldn't"), QStringLiteral("could")}, {QStringLiteral("don't"), QStringLiteral("do")},
                                                     {QStringLiteral("didn't"), QStringLiteral("did")},       {QStringLiteral("doesn't"), QStringLiteral("does")},   {QStringLiteral("wouldn't"), QStringLiteral("would")},
                                                     {QStringLiteral("won't"), QStringLiteral("will")},       {QStringLiteral("haven't"), QStringLiteral("have")},   {QStringLiteral("hasn't"), QStringLiteral("has")},
                                                     {QStringLiteral("hadn't"), QStringLiteral("had")},       {QStringLiteral("isn't"), QStringLiteral("is")},       {QStringLiteral("wasn't"), QStringLiteral("was")},
                                                     {QStringLiteral("shouldn't"), QStringLiteral("should")}, {QStringLiteral("aren't"), QStringLiteral("are")},     {QStringLiteral("weren't"), QStringLiteral("were")}};
    const QSet<QString> &verbs = WORDS("have", "has", "need", "see", "want", "know", "get", "got", "find", "take", "make", "give", "hear", "like", "mind");
    for (int i = 0; i < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Word || !positive.contains(c.tokens[i].lower)) continue;
        const int n = c.nextWord(i);
        if (n < 0) continue;
        const QString &l = c.tokens[n].lower;
        if (l == QLatin1String("hardly") || l == QLatin1String("barely") || l == QLatin1String("scarcely")) {
            if (!quotedAround(c, i, n))
                c.add(c.tokens[i].start, c.tokens[n].end, GK::Grammar, QCoreApplication::translate("Grammar", "This is a double negative: \"%1\" already says \"not\".").arg(l),
                      matchCase(c.original(i), positive.value(c.tokens[i].lower) + QLatin1Char(' ') + l));
        } else if (verbs.contains(l)) {
            const int m = c.nextWord(n);
            if (m < 0 || c.tokens[m].lower != QLatin1String("no") || c.original(m) != QLatin1String("no")) continue;
            const int w = c.nextWord(m);
            if (w < 0 || c.tokens[w].kind != Tok::Word || WORDS("one", "body", "thing", "where", "longer", "more", "matter", "doubt", "way", "other", "less", "doubts", "such").contains(c.tokens[w].lower)) continue;
            replaceToken(c, m, QStringLiteral("any"), GK::Grammar, QCoreApplication::translate("Grammar", "This is a double negative: use \"any\" after \"not\"."));
        }
    }
}

// A comma after the year of a date that begins a clause: "On June 5, 2020 we".
void commaAfterYear(Ctx &c)
{
    const QSet<QString> &months = WORDS("january", "february", "march", "april", "may", "june", "july", "august", "september", "october", "november", "december");
    const QSet<QString> &next = WORDS("the", "we", "he", "she", "they", "it", "i", "a", "an", "our", "my", "his", "her", "there", "this", "these", "those", "you");
    for (int i = 0; i + 6 < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Word || !months.contains(c.tokens[i].lower) || !c.firstUpper(i)) continue;
        const int day = c.nextWord(i);
        if (day < 0 || c.tokens[day].kind != Tok::Number || c.tokens[day].lower.size() > 2) continue;
        const int comma = day + 1;
        if (!c.isPunct(comma, QLatin1Char(',')) || !c.isSpace(comma + 1)) continue;
        const int year = comma + 2;
        if (year >= c.size() || c.tokens[year].kind != Tok::Number || c.tokens[year].lower.size() != 4) continue;
        const int n = c.nextWord(year);
        if (n < 0 || c.tokens[n].kind != Tok::Word || !next.contains(c.tokens[n].lower)) continue;
        c.add(c.tokens[year].start, c.tokens[year].end, GK::Punctuation, QCoreApplication::translate("Grammar", "Put a comma after the year."), c.original(year) + QLatin1Char(','));
    }
}

// A comma after the state of a city and state: "in Austin, Texas is".
void commaAfterState(Ctx &c)
{
    static const PhraseTable states({{"alabama", ""}, {"alaska", ""}, {"arizona", ""}, {"arkansas", ""}, {"california", ""}, {"colorado", ""}, {"connecticut", ""}, {"delaware", ""}, {"florida", ""},
                                     {"georgia", ""}, {"hawaii", ""}, {"idaho", ""}, {"illinois", ""}, {"indiana", ""}, {"iowa", ""}, {"kansas", ""}, {"kentucky", ""}, {"louisiana", ""}, {"maine", ""},
                                     {"maryland", ""}, {"massachusetts", ""}, {"michigan", ""}, {"minnesota", ""}, {"mississippi", ""}, {"missouri", ""}, {"montana", ""}, {"nebraska", ""}, {"nevada", ""},
                                     {"new hampshire", ""}, {"new jersey", ""}, {"new mexico", ""}, {"new york", ""}, {"north carolina", ""}, {"north dakota", ""}, {"ohio", ""}, {"oklahoma", ""},
                                     {"oregon", ""}, {"pennsylvania", ""}, {"rhode island", ""}, {"south carolina", ""}, {"south dakota", ""}, {"tennessee", ""}, {"texas", ""}, {"utah", ""}, {"vermont", ""},
                                     {"virginia", ""}, {"washington", ""}, {"west virginia", ""}, {"wisconsin", ""}, {"wyoming", ""}});
    const QSet<QString> &prepositions = WORDS("in", "from", "at", "near", "to", "of", "outside", "around", "visit", "visited", "born");
    const QSet<QString> &next = WORDS("is", "was", "are", "were", "has", "have", "had", "will", "would", "can", "could", "which", "where", "who", "the", "we", "it", "they", "he", "she", "there");
    states.each(c, [&](int first, int last, const QString &) {
        // the comma before, and the city before it (one or two words that begin with capitals, after a preposition)
        const int comma = first - 2;
        if (comma < 1 || !c.isPunct(comma, QLatin1Char(',')) || !c.isSpace(comma + 1) || !c.isWord(comma - 1) || !c.firstUpper(comma - 1) || !c.firstUpper(first)) return;
        int city = comma - 1;
        static const QSet<QString> stateNames = {QStringLiteral("new york"), QStringLiteral("new jersey"), QStringLiteral("new mexico"), QStringLiteral("new hampshire"), QStringLiteral("north carolina"),
                                                 QStringLiteral("south carolina"), QStringLiteral("north dakota"), QStringLiteral("south dakota"), QStringLiteral("west virginia"), QStringLiteral("rhode island")};
        // (a state before a state is not a city: "In Texas, Florida has", and a month or a day is the start of a clause: "In March, Texas is dry")
        if (WORDS("alabama", "alaska", "arizona", "arkansas", "california", "colorado", "connecticut", "delaware", "florida", "georgia", "hawaii", "idaho", "illinois", "indiana", "iowa", "kansas", "kentucky",
                  "louisiana", "maine", "maryland", "massachusetts", "michigan", "minnesota", "mississippi", "missouri", "montana", "nebraska", "nevada", "ohio", "oklahoma", "oregon", "pennsylvania", "tennessee",
                  "texas", "utah", "vermont", "virginia", "washington", "wisconsin", "wyoming", "january", "february", "march", "april", "may", "june", "july", "august", "september", "october", "november",
                  "december", "monday", "tuesday", "wednesday", "thursday", "friday", "saturday", "sunday").contains(c.tokens[city].lower)) return;
        if (const int before = c.prevWord(city); before >= 0 && stateNames.contains(c.tokens[before].lower + QLatin1Char(' ') + c.tokens[city].lower)) return;
        int p = c.prevWord(city);
        if (p >= 0 && c.firstUpper(p) && !prepositions.contains(c.tokens[p].lower)) p = c.prevWord(p);
        if (p < 0 || !prepositions.contains(c.tokens[p].lower)) return;
        const int n = c.nextWord(last);
        if (n < 0 || !next.contains(c.tokens[n].lower)) return;
        c.add(c.tokens[last].start, c.tokens[last].end, GK::Punctuation, QCoreApplication::translate("Grammar", "Put a comma after the state."), c.original(last) + QLatin1Char(','));
    });
}

// A compound that comes before a noun takes a hyphen: "a well known author".
void compoundHyphen(Ctx &c)
{
    const QSet<QString> &anyDeterminer = WORDS("a", "an", "the", "this", "that", "his", "her", "their", "our", "my", "your", "its", "some", "any", "no", "every", "such", "another", "most");
    const QSet<QString> &fewDeterminers = WORDS("a", "an", "his", "her", "their", "our", "my", "your", "its");
    const QSet<QString> &notNouns = WORDS("for", "as", "by", "to", "in", "of", "and", "or", "but", "that", "is", "was", "are", "were", "at", "on", "with", "from", "which", "who", "if", "when", "than", "because",
                                          "it", "they", "he", "she", "we", "you", "i", "there", "this", "these", "those", "what", "how", "where", "can", "will", "would", "could", "should", "must", "may", "might",
                                          "has", "have", "had", "do", "does", "did", "be", "been", "being", "not", "also", "only", "just", "still", "already", "then", "so", "too", "very", "about", "into", "over",
                                          "after", "before", "during", "while", "until", "under", "between", "through", "without", "within", "per", "up", "down", "out", "off", "all", "both", "each", "more",
                                          "most", "much", "many", "other", "same", "such", "even", "ever", "never", "always", "often", "now", "here");
    static const PhraseTable pairs({{"well known", ""}, {"well written", ""}, {"well established", ""}, {"well documented", ""}, {"well defined", ""}, {"well designed", ""}, {"well made", ""},
                                    {"well received", ""}, {"well respected", ""}, {"well informed", ""}, {"well behaved", ""}, {"well trained", ""}, {"well organized", ""}, {"well maintained", ""},
                                    {"well paid", ""}, {"long term", ""}, {"short term", ""}, {"full time", ""}, {"part time", ""}, {"high quality", ""}, {"low cost", ""}, {"low income", ""},
                                    {"high speed", ""}, {"hard working", ""}, {"user friendly", ""}, {"family owned", ""}, {"family friendly", ""}, {"wide ranging", ""}, {"far reaching", ""},
                                    {"long lasting", ""}, {"fast growing", ""}, {"best known", ""}, {"high profile", ""}, {"high level", ""}, {"low level", ""}, {"large scale", ""}, {"small scale", ""},
                                    {"open ended", ""}});
    pairs.each(c, [&](int first, int last, const QString &) {
        const int p = c.prevWord(first), n = c.nextWord(last);
        if (p < 0 || n < 0 || c.tokens[n].kind != Tok::Word || notNouns.contains(c.tokens[n].lower)) return;
        const bool well = c.tokens[first].lower == QLatin1String("well") || c.tokens[first].lower == QLatin1String("best");
        if (!(well ? anyDeterminer : fewDeterminers).contains(c.tokens[p].lower)) return;
        if (c.original(first).at(0).isUpper() && !startsSentence(c, first)) return;
        if (allCaps(c, first) || allCaps(c, last)) return;
        if (c.text.mid(c.tokens[first].end, c.tokens[last].start - c.tokens[first].end) != QLatin1String(" ")) return;
        c.add(c.tokens[first].start, c.tokens[last].end, GK::Punctuation, QCoreApplication::translate("Grammar", "Join these with a hyphen when they come before a noun."),
              c.original(first) + QLatin1Char('-') + c.original(last));
    });
}

// A comma with no space after it between two words: "yes,but".
void commaSpace(Ctx &c)
{
    // (a paragraph that has three of these or more is a list or a table of values, not a slip of the keyboard)
    int slips = 0;
    for (int i = 1; i + 1 < c.size(); ++i) {
        c.step();
        if (c.isPunct(i, QLatin1Char(',')) && c.isWord(i - 1) && c.isWord(i + 1)) ++slips;
    }
    if (slips > 2) return;
    QVector<int> chunkCommas(c.size(), 0);   // by the first token of a run without spaces
    for (int i = 0; i < c.size(); ++i) {
        c.step();
        if (c.isPunct(i, QLatin1Char(','))) ++chunkCommas[c.chunkFirst[i]];
    }
    for (int i = 1; i + 1 < c.size(); ++i) {
        c.step();
        if (!c.isPunct(i, QLatin1Char(',')) || !c.isWord(i - 1) || !c.isWord(i + 1)) continue;
        if (c.chunkAddress[i] || c.chunkDots[i] > 0) continue;
        if (c.tokens[i - 1].end - c.tokens[i - 1].start < 3 || c.tokens[i + 1].end - c.tokens[i + 1].start < 3) continue;
        if (chunkCommas[c.chunkFirst[i]] > 1) continue;   // (a list of names or values: a,b,c)
        if (c.original(i - 1).at(0).isDigit() || c.original(i + 1).at(0).isDigit()) continue;
        c.add(c.tokens[i].start, c.tokens[i].end, GK::Punctuation, QCoreApplication::translate("Grammar", "Put a space after the comma."), QStringLiteral(", "));
    }
}

// Two commas, or two periods after a word.
void doubledMark(Ctx &c)
{
    for (int i = 1; i + 1 < c.size(); ++i) {
        c.step();
        if (!c.isWord(i - 1)) continue;
        if (c.isPunct(i, QLatin1Char(',')) && c.isPunct(i + 1, QLatin1Char(',')) && c.isSpace(i + 2) && c.tokens[i + 2].end - c.tokens[i + 2].start == 1 && c.isWord(i + 3) && !c.chunkAddress[i])   // (a table of values has runs of spaces and empty fields)
            c.add(c.tokens[i].start, c.tokens[i + 1].end, GK::Punctuation, QCoreApplication::translate("Grammar", "Two commas in a row."), QStringLiteral(","));
        else if (c.isPunct(i, QLatin1Char('.')) && c.isPunct(i + 1, QLatin1Char('.')) && !c.isPunct(i + 2, QLatin1Char('.')) && (i + 2 >= c.size() || c.isSpace(i + 2)) && !c.isPunct(i - 1, QLatin1Char('.'))) {
            if (abbreviations().contains(c.tokens[i - 1].lower) || (c.tokens[i - 1].end - c.tokens[i - 1].start <= 3 && c.firstUpper(i - 1))) continue;   // (Mr.. is an abbreviation and a slip)
            GrammarIssue g;
            g.start = c.tokens[i].start;
            g.end = c.tokens[i + 1].end;
            g.kind = GK::Punctuation;
            g.message = QCoreApplication::translate("Grammar", "Two periods: use one, or three for an ellipsis.");
            g.suggestions << QStringLiteral(".") << QStringLiteral("...");
            g.rule = QLatin1String(c.rule);
            c.issues << g;
        }
    }
}

// "Your welcome." for "You're welcome.", where it begins a sentence and ends it.
void yourWelcome(Ctx &c)
{
    eachPair(c, WORDS("your"), WORDS("welcome"), [&](int i, int j) {
        if (!startsSentence(c, i) || j + 1 >= c.size() || c.tokens[j + 1].kind != Tok::Punct || !QLatin1String(".!").contains(c.text.at(c.tokens[j + 1].start))) return;
        replaceToken(c, i, QStringLiteral("you're"), GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"you're\" (you are) here, not \"your\"."));
    });
}

// A lowercase "i" for the word "I": before a verb that says it is the word, or "i'm", "i'll", "i've", "i'd".
void lowercaseI(Ctx &c)
{
    const QSet<QString> &verbs = WORDS("am", "was", "will", "have", "had", "think", "can", "do", "would", "could", "should", "know", "need", "want", "see", "like", "feel", "believe", "hope", "guess", "love",
                                       "just", "also", "really", "never", "always", "don't", "didn't", "can't", "won't", "got", "went", "thought", "mean", "wish", "hate", "said", "wanted", "did");
    for (int i = 0; i < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Word || c.chunkAddress[i] || c.chunkDots[i] > 0) continue;
        const QString o = c.original(i);
        const QString &l = c.tokens[i].lower;
        const bool contraction = l == QLatin1String("i'm") || l == QLatin1String("i'll") || l == QLatin1String("i've") || l == QLatin1String("i'd");
        if (contraction ? o.at(0) != QLatin1Char('i') : o != QLatin1String("i")) continue;
        if (!contraction) {
            const int n = c.nextWord(i);
            if (n < 0 || !verbs.contains(c.tokens[n].lower)) continue;
            if (c.isPunct(i - 1, QLatin1Char('(')) || c.isPunct(i - 1, QLatin1Char('.')) || c.isPunct(i - 1, QLatin1Char('=')) || c.isPunct(i - 1, QLatin1Char('<'))) continue;
        }
        c.add(c.tokens[i].start, c.tokens[i].start + 1, GK::Capitalization, QCoreApplication::translate("Grammar", "Write \"I\" with a capital letter."), QStringLiteral("I"));
    }
}

// The days and months written in small letters.
void dayAndMonthNames(Ctx &c)
{
    const QSet<QString> &names = WORDS("sunday", "monday", "tuesday", "wednesday", "thursday", "friday", "saturday", "january", "february", "april", "june", "july", "september", "october", "november", "december");
    for (int i = 0; i < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Word || !names.contains(c.tokens[i].lower) || c.firstUpper(i) || c.chunkAddress[i] || c.chunkDots[i] > 0) continue;
        if (c.original(i) != c.tokens[i].lower) continue;
        c.add(c.tokens[i].start, c.tokens[i].start + 1, GK::Capitalization, QCoreApplication::translate("Grammar", "Begin the name of a day or month with a capital letter."), QString(c.text.at(c.tokens[i].start).toUpper()));
    }
}

// ---- style rules (off unless asked for)

// "could care less" for "couldn't care less".
void couldCareLess(Ctx &c)
{
    static const PhraseTable table({{"could care less", "couldn't care less"}});
    table.each(c, [&](int i, int j, const QString &with) {
        replacePhrase(c, i, j, with, GK::Style, QCoreApplication::translate("Grammar", "The saying is \"couldn't care less\" (it is not possible to care less)."));
    });
}

// "try and" for "try to".
void tryAnd(Ctx &c)
{
    const QSet<QString> &verbs = WORDS("find", "get", "make", "do", "see", "keep", "stop", "help", "come", "be", "give", "take", "understand", "figure", "remember", "think", "learn", "use", "fix", "finish", "avoid",
                                       "stay", "break", "answer", "change", "save", "reach", "read", "write", "build", "win", "solve", "convince", "explain", "catch");
    eachPair(c, WORDS("try"), WORDS("and"), [&](int i, int j) {
        const int v = c.nextWord(j);
        if (v < 0 || !verbs.contains(c.tokens[v].lower) || c.firstUpper(v)) return;
        replacePhrase(c, i, j, QStringLiteral("try to"), GK::Style, QCoreApplication::translate("Grammar", "Use \"try to\" in careful writing."));
    });
}

// "in order to" for "to".
void inOrderTo(Ctx &c)
{
    static const PhraseTable table({{"in order to", "to"}, {"so as to", "to"}});
    table.each(c, [&](int i, int j, const QString &with) { replacePhrase(c, i, j, with, GK::Style, QCoreApplication::translate("Grammar", "\"To\" says the same in fewer words.")); });
}

// Long phrases with a short one that says the same.
void wordyPhrases(Ctx &c)
{
    static const PhraseTable table({{"due to the fact that", "because"}, {"owing to the fact that", "because"}, {"in spite of the fact that", "although"}, {"despite the fact that", "although"},
                                    {"at this point in time", "now"}, {"at the present time", "now"}, {"in the event that", "if"}, {"for the purpose of", "for"}, {"a large number of", "many"},
                                    {"with regard to", "about"}, {"with regards to", "about"}, {"in regard to", "about"}, {"has the ability to", "can"}, {"have the ability to", "can"},
                                    {"each and every", "every"}, {"in the near future", "soon"}, {"on a daily basis", "daily"}, {"on a regular basis", "regularly"}, {"in close proximity to", "near"},
                                    {"prior to", "before"}, {"subsequent to", "after"}, {"the majority of", "most"}, {"a majority of", "most"}, {"in the absence of", "without"},
                                    {"for the reason that", "because"}, {"in light of the fact that", "because"}, {"until such time as", "until"}, {"during the course of", "during"},
                                    {"inside of", "inside"}, {"outside of", "outside"}, {"off of", "off"}});
    table.each(c, [&](int i, int j, const QString &with) { replacePhrase(c, i, j, with, GK::Style, QCoreApplication::translate("Grammar", "A shorter way to say this.")); });
}

// Words that say what the word next to them says.
void redundantPhrases(Ctx &c)
{
    static const PhraseTable table({{"free gift", "gift"}, {"end result", "result"}, {"future plans", "plans"}, {"added bonus", "bonus"}, {"advance planning", "planning"}, {"basic fundamentals", "fundamentals"},
                                    {"close proximity", "proximity"}, {"completely finished", "finished"}, {"final outcome", "outcome"}, {"unexpected surprise", "surprise"}, {"revert back", "revert"},
                                    {"return back", "return"}, {"repeat again", "repeat"}, {"join together", "join"}, {"merge together", "merge"}, {"very unique", "unique"}, {"totally unique", "unique"},
                                    {"absolutely essential", "essential"}, {"exact same", "same"}, {"true fact", "fact"}, {"usual custom", "custom"}});
    table.each(c, [&](int i, int j, const QString &with) { replacePhrase(c, i, j, with, GK::Style, QCoreApplication::translate("Grammar", "One of these words says what the other does.")); });
}

// "was written by": a sentence that tells what was done before who did it.
void passiveVoice(Ctx &c)
{
    const QSet<QString> &be = WORDS("is", "are", "was", "were", "be", "been", "being");
    const QSet<QString> &irregular = WORDS("made", "done", "given", "taken", "written", "known", "seen", "shown", "built", "held", "kept", "sent", "told", "found", "chosen", "caused", "begun", "led", "paid", "brought", "bought",
                                           "thought", "taught", "driven", "broken", "stolen", "worn", "won", "lost", "left", "set", "run", "read", "put", "cut", "hit", "heard", "met");
    for (int i = 0; i < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Word || !be.contains(c.tokens[i].lower)) continue;
        const int v = c.nextWord(i);
        if (v < 0 || c.tokens[v].kind != Tok::Word) continue;
        const QString &w = c.tokens[v].lower;
        if (!(irregular.contains(w) || (w.size() > 4 && w.endsWith(QLatin1String("ed"))))) continue;
        const int by = c.nextWord(v);
        if (by < 0 || c.tokens[by].lower != QLatin1String("by") || allCaps(c, i)) continue;
        note(c, c.tokens[i].start, c.tokens[v].end, GK::Style, QCoreApplication::translate("Grammar", "This is in the passive voice. Say who does it first to make it direct."));
    }
}

// A sentence of more than 40 words.
void longSentence(Ctx &c)
{
    for (const Sentence &s : sentencesOf(c))
        if (s.words > 40) note(c, c.tokens[s.first].start, c.tokens[s.first].end, GK::Style, QCoreApplication::translate("Grammar", "This sentence has %1 words. Consider splitting it.").arg(s.words));
}

// Three sentences in a row that begin with the same word.
void repeatedSentenceStart(Ctx &c)
{
    const QVector<Sentence> all = sentencesOf(c);
    for (int k = 2; k < all.size(); ++k) {
        const Token &a = c.tokens[all[k - 2].first], &b = c.tokens[all[k - 1].first], &d = c.tokens[all[k].first];
        if (a.kind != Tok::Word || b.kind != Tok::Word || d.kind != Tok::Word || a.lower != b.lower || b.lower != d.lower || a.lower.size() < 2) continue;
        note(c, d.start, d.end, GK::Style, QCoreApplication::translate("Grammar", "Three sentences in a row begin with \"%1\". Vary the start.").arg(c.text.mid(d.start, d.end - d.start)));
    }
}

// The rules: to add one, write its function above and put it here.
struct Rule {
    const char *name;
    void (*run)(Ctx &);
    GK kind;
};

// The rules of the other languages (each file has its functions and the table of its rules).
#include "grammar/grammar_common.inc"
#include "grammar/grammar_fr.inc"

const Rule kRules[] = {
    {"repeated-word", repeatedWord, GK::Repetition},
    {"a-an", articles, GK::Grammar},
    {"sentence-start", lowercaseSentenceStart, GK::Capitalization},
    {"its", itsAndItIs, GK::WordChoice},
    {"their-there", theirThereTheyAre, GK::WordChoice},
    {"your-youre", yourYouAre, GK::WordChoice},
    {"then-than", thenAndThan, GK::WordChoice},
    {"lose-loose", loseAndLoose, GK::WordChoice},
    {"could-of", couldOf, GK::Grammar},
    {"double-space", doubleSpace, GK::Punctuation},
    {"space-before-mark", spaceBeforeMark, GK::Punctuation},
    {"missing-space", missingSpaceAfterPeriod, GK::Punctuation},
    {"alot", alot, GK::WordChoice},
    {"everyday", everyday, GK::WordChoice},
    {"he-dont", heDont, GK::Grammar},
    {"they-was", theyWas, GK::Grammar},
    {"she-have", sheHave, GK::Grammar},
    {"plural-has", pluralHas, GK::Grammar},
    {"plural-does", pluralDoes, GK::Grammar},
    {"they-is", theyIs, GK::Grammar},
    {"there-is-many", thereIsMany, GK::Grammar},
    {"indefinite-are", indefiniteAre, GK::Grammar},
    {"whos-whose", whosAndWhose, GK::WordChoice},
    {"affect-effect", affectAndEffect, GK::WordChoice},
    {"less-fewer", lessAndFewer, GK::Grammar},
    {"much-many", muchAndMany, GK::Grammar},
    {"irregardless", irregardless, GK::WordChoice},
    {"whether-weather", whetherAndWeather, GK::WordChoice},
    {"quite-quiet", quiteAndQuiet, GK::WordChoice},
    {"chose-choose", choseAndChoose, GK::WordChoice},
    {"advice-advise", adviceAndAdvise, GK::WordChoice},
    {"to-much", toMuch, GK::WordChoice},
    {"too-to", tooAndTo, GK::WordChoice},
    {"breath-breathe", breathAndBreathe, GK::WordChoice},
    {"run-together", runTogether, GK::WordChoice},
    {"idioms", idioms, GK::WordChoice},
    {"demonstrative-number", demonstrativeNumber, GK::Grammar},
    {"between-you-and-i", betweenYouAndI, GK::Grammar},
    {"me-and-him", meAndHim, GK::Grammar},
    {"double-comparative", doubleComparative, GK::Grammar},
    {"modal-past", modalPast, GK::Grammar},
    {"have-past", havePast, GK::Grammar},
    {"double-negative", doubleNegative, GK::Grammar},
    {"comma-after-year", commaAfterYear, GK::Punctuation},
    {"comma-after-state", commaAfterState, GK::Punctuation},
    {"compound-hyphen", compoundHyphen, GK::Punctuation},
    {"comma-space", commaSpace, GK::Punctuation},
    {"doubled-mark", doubledMark, GK::Punctuation},
    {"your-welcome", yourWelcome, GK::WordChoice},
    {"lowercase-i", lowercaseI, GK::Capitalization},
    {"day-month-names", dayAndMonthNames, GK::Capitalization},
    {"amount-number", amountAndNumber, GK::Style},
    {"could-care-less", couldCareLess, GK::Style},
    {"try-and", tryAnd, GK::Style},
    {"in-order-to", inOrderTo, GK::Style},
    {"wordy-phrases", wordyPhrases, GK::Style},
    {"redundant-phrases", redundantPhrases, GK::Style},
    {"passive-voice", passiveVoice, GK::Style},
    {"long-sentence", longSentence, GK::Style},
    {"repeated-sentence-start", repeatedSentenceStart, GK::Style},
};

// The table of the rules of a language.
struct RuleList {
    const Rule *first;
    size_t n;
};
RuleList rulesOf(Language language)
{
    switch (language) {
    case Language::French: return {kFrenchRules, sizeof(kFrenchRules) / sizeof(kFrenchRules[0])};
    case Language::German:
    case Language::Spanish:
    case Language::English: break;
    }
    return {kRules, sizeof(kRules) / sizeof(kRules[0])};
}

} // namespace

QVector<RuleInfo> rules(Language language)
{
    QVector<RuleInfo> out;
    const RuleList list = rulesOf(language);
    for (size_t i = 0; i < list.n; ++i) out << RuleInfo{QLatin1String(list.first[i].name), list.first[i].kind};
    return out;
}

QVector<GrammarIssue> check(const QString &text, quint64 *work, quint32 kinds, Language language)
{
    Ctx c(text);
    tokenize(c);
    const RuleList list = rulesOf(language);
    for (size_t i = 0; i < list.n; ++i) {
        const Rule &r = list.first[i];
        if (!(kinds & kindBit(r.kind))) continue;
        c.rule = r.name;
        r.run(c);
    }
    // In order of position; of two that overlap, the one that starts first stays.
    std::stable_sort(c.issues.begin(), c.issues.end(), [](const GrammarIssue &a, const GrammarIssue &b) { return a.start < b.start; });
    QVector<GrammarIssue> out;
    int last = 0;
    for (const GrammarIssue &g : c.issues) {
        if (g.start < last || !(kinds & kindBit(g.kind))) continue;
        out << g;
        last = g.end;
    }
    if (work) *work = c.steps;
    return out;
}

} // namespace jp::grammar
