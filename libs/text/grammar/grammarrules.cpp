#include "grammar/grammarrules.h"

#include <QCoreApplication>
#include <QRegularExpression>
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
    quint64 steps = 0;
    // The run of tokens with no space in it that each token is in (an address, a file name, a word with its marks): [first, last] token indexes, and how many periods it has
    // before its trailing marks, and whether it has a mark that only an address or a path has.
    QVector<int> chunkFirst, chunkLast;
    QVector<int> chunkDots;
    QVector<char> chunkAddress;

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

const QSet<QString> &setOf(std::initializer_list<const char *> words, QSet<QString> &store)
{
    if (store.isEmpty())
        for (const char *w : words) store.insert(QLatin1String(w));
    return store;
}
#define WORDS(...) ([]() -> const QSet<QString> & { static QSet<QString> s; return setOf({__VA_ARGS__}, s); }())

// Words that end a sentence's last word with a period but are not the end of a sentence (and the endings of addresses).
const QSet<QString> &abbreviations()
{
    return WORDS("etc", "vs", "cf", "approx", "fig", "figs", "eq", "inc", "ltd", "corp", "dept", "est", "misc", "mrs", "prof", "gen", "sgt", "capt", "col", "rev", "hon", "assn", "ave", "blvd",
                 "jan", "feb", "mar", "apr", "jun", "jul", "aug", "sep", "sept", "oct", "nov", "dec", "mon", "tue", "wed", "thu", "fri", "sat", "sun", "nos", "vol", "eds", "sec", "dept", "govt", "univ",
                 "mount", "min", "max", "ref", "tel", "ext", "appt", "apt", "assoc", "bros", "esq", "hrs", "mgmt");
}
const QSet<QString> &addressEndings()
{
    return WORDS("com", "org", "net", "edu", "gov", "io", "js", "py", "ts", "rb", "txt", "pdf", "doc", "docx", "xls", "xlsx", "ppt", "pptx", "html", "htm", "css", "jpg", "jpeg", "png", "gif", "svg", "exe",
                 "zip", "json", "xml", "csv", "md", "cpp", "php", "uk", "us", "ca", "au", "de", "fr", "info", "biz", "co", "me");
}

// ---- the rules: each is one function, and one line of the table at the end

using GK = GrammarIssue::Kind;

// "the the": the second of two words alike, with the space before it taken out.
void repeatedWord(Ctx &c)
{
    const QSet<QString> &legitimate = WORDS("had", "that", "is", "bye", "no", "so", "very", "ha", "hah", "blah", "never", "knock", "yada", "choo", "boom", "tsk", "in", "on", "out", "up", "off", "over");   // (put in in an hour; the particles can double)
    for (int i = 0; i < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Word) continue;
        const int j = c.nextWord(i);
        if (j < 0 || c.tokens[j].kind != Tok::Word || c.tokens[i].lower != c.tokens[j].lower) continue;
        const QString between = c.text.mid(c.tokens[i].end, c.tokens[j].start - c.tokens[i].end);
        if (between.contains(QLatin1Char('\n')) || between.contains(QChar(0x2028)) || between.contains(QChar(0x2029))) continue;   // (across a line: left to the person)
        const QString &w = c.tokens[i].lower;
        if (w.size() < 2 || legitimate.contains(w)) continue;
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
    for (QChar ch : lower) {
        if (ch.isDigit()) digits += ch;
        else if (ch == QLatin1Char(',')) comma = true;
        else break;
    }
    if (digits.isEmpty() || digits.startsWith(QLatin1Char('0'))) return Sound::Unknown;   // (0x88, 03: said in more than one way)
    // What follows the digits: nothing, an ordinal ending (3rd, 18th), or an "s" (1990s); another letter makes a code (4K, 0x88).
    const int stop = int(lower.indexOf(QRegularExpression(QStringLiteral("[^0-9,.]"))));
    const QString rest = stop < 0 ? QString() : lower.mid(stop);
    if (!rest.isEmpty() && rest != QLatin1String("s") && rest != QLatin1String("st") && rest != QLatin1String("nd") && rest != QLatin1String("rd") && rest != QLatin1String("th") && rest != QLatin1String("%")) return Sound::Unknown;
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
    if (startsWithAny(w, {"histor", "herb", "hotel", "hispan", "yt"}) || (first == QLatin1Char('x'))) return Sound::Unknown;   // (said either way)
    if (first == QLatin1Char('h')) return startsWithAny(w, {"hour", "honest", "honor", "honour", "heir"}) ? Sound::Vowel : Sound::Consonant;
    if (!QLatin1String("aeiou").contains(first)) return Sound::Consonant;
    // A vowel letter, but a consonant sound: one, once, eu-, ewe, uni- (not "unin", "unim"), use, uti-, and the like.
    if (w == QLatin1String("one") || w == QLatin1String("once")) return Sound::Consonant;
    if (startsWithAny(w, {"eu", "ewe", "unic", "unif", "unil", "unio", "uniq", "unis", "unit", "univ", "unix", "unanim", "use", "usa", "usu", "ute", "uti", "uto", "uku", "uk", "ufo", "ubiq", "uran", "urin", "uvul"}))
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
    while (k >= 0 && (c.tokens[k].kind == Tok::Space || (c.tokens[k].kind == Tok::Punct && QLatin1String("\"'([“‘«").contains(c.text.at(c.tokens[k].start))))) --k;
    return k < 0 || (c.tokens[k].kind == Tok::Punct && QLatin1String(".!?").contains(c.text.at(c.tokens[k].start)));
}

void articles(Ctx &c)
{
    const QSet<QString> &notNouns = WORDS("is", "are", "am", "and", "or", "as", "at", "in", "on", "if", "of", "it", "its", "i", "a", "an", "be", "to", "us", "up", "any", "all");
    for (int i = 0; i < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Word) continue;
        const QString &l = c.tokens[i].lower;
        if (l != QLatin1String("a") && l != QLatin1String("an")) continue;
        const QString orig = c.original(i);
        const bool capital = orig.at(0).isUpper();
        if (orig.size() == 2 && orig.at(1).isUpper() && orig.at(0).isLower()) continue;
        if (capital && !startsSentence(c, i)) continue;   // (Vitamin A, Plan A)
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

// A sentence that starts with a small letter.
void lowercaseSentenceStart(Ctx &c)
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
    auto fix = [&](int k) {
        const int s = c.tokens[k].start;
        c.add(s, s + 1, GK::Capitalization, QCoreApplication::translate("Grammar", "Start a sentence with a capital letter."), QString(c.text.at(s).toUpper()));
    };
    // The paragraph's first word, when the paragraph is a sentence or more (three words or more, ending with a sentence mark).
    int first = 0;
    while (first < c.size() && c.tokens[first].kind == Tok::Space) ++first;
    if (lowercaseWord(first)) {
        int words = 0, last = c.size() - 1;
        while (last > 0 && (c.tokens[last].kind == Tok::Space || (c.tokens[last].kind == Tok::Punct && QLatin1String("\"')]”’").contains(c.text.at(c.tokens[last].start))))) --last;
        for (int k = 0; k < c.size(); ++k) {
            c.step();
            if (c.tokens[k].kind == Tok::Word) ++words;
        }
        if (words >= 3 && last > first && c.tokens[last].kind == Tok::Punct && QLatin1String(".!?").contains(c.text.at(c.tokens[last].start))) fix(first);
    }
    // After a sentence mark and a space.
    for (int i = 1; i + 2 < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Punct || !QLatin1String(".!?").contains(c.text.at(c.tokens[i].start))) continue;
        if (c.tokens[i + 1].kind != Tok::Space) continue;
        const int w = i + 2;
        if (!lowercaseWord(w) && !(c.isWord(w) && c.tokens[w].end - c.tokens[w].start == 1 && c.text.at(c.tokens[w].start) == QLatin1Char('a') && false)) continue;
        const int before = i - 1;
        if (!c.isWord(before)) continue;
        if (c.text.at(c.tokens[i].start) == QLatin1Char('.')) {
            const QString &b = c.tokens[before].lower;
            if (b.size() < 3 || abbreviations().contains(b)) continue;
            if (before >= 1 && c.isPunct(before - 1, QLatin1Char('.'))) continue;
        }
        fix(w);
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
    eachPair(c, WORDS("its"), WORDS("a", "an", "the", "not", "been", "going"), [&](int i, int) {
        replaceToken(c, i, QStringLiteral("it's"), GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"it's\" (it is) here, not \"its\"."));
    });
    eachPair(c, WORDS("it's", "it’s"), WORDS("own"), [&](int i, int) {
        replaceToken(c, i, QStringLiteral("its"), GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"its\" (belonging to it) here, not \"it's\"."));
    });
}

// their, there, and they're.
void theirThereTheyAre(Ctx &c)
{
    eachPair(c, WORDS("their"), WORDS("is", "are", "was", "were"), [&](int i, int) {
        replaceToken(c, i, QStringLiteral("there"), GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"there\" here, not \"their\"."));
    });
    eachPair(c, WORDS("there", "they're", "they’re"), WORDS("own"), [&](int i, int) {
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
        static const QSet<QString> nexts = {QStringLiteral("a"), QStringLiteral("an"), QStringLiteral("the"), QStringLiteral("not"), QStringLiteral("going"), QStringLiteral("being"), QStringLiteral("always"), QStringLiteral("never")};
        if (j < 0 || !nexts.contains(c.tokens[j].lower) || c.original(j) != c.tokens[j].lower) continue;   // (a capital letter: "Your A grade")
        replaceToken(c, i, QStringLiteral("you're"), GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"you're\" (you are) here, not \"your\"."));
    }
    eachPair(c, WORDS("you're", "you’re"), WORDS("own", "name"), [&](int i, int) {
        replaceToken(c, i, QStringLiteral("your"), GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"your\" (belonging to you) here, not \"you're\"."));
    });
}

// then and than.
void thenAndThan(Ctx &c)
{
    eachPair(c, WORDS("more", "less", "better", "worse", "rather", "other", "fewer", "greater", "smaller", "larger", "bigger", "higher", "lower", "faster", "slower", "older", "younger"), WORDS("then"),
             [&](int, int j) { replaceToken(c, j, QStringLiteral("than"), GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"than\" to compare.")); });
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

// Two or more spaces between words.
void doubleSpace(Ctx &c)
{
    for (int i = 1; i + 1 < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Space || c.tokens[i].end - c.tokens[i].start < 2 || !c.plainSpaces(i)) continue;
        c.add(c.tokens[i].start, c.tokens[i].end, GK::Punctuation, QCoreApplication::translate("Grammar", "Use one space here."), QStringLiteral(" "));
    }
}

// A space before a comma, a period, or another mark that goes with the word before it.
void spaceBeforeMark(Ctx &c)
{
    for (int i = 1; i + 1 < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Space || !c.plainSpaces(i)) continue;
        const int m = i + 1;
        if (c.tokens[m].kind != Tok::Punct) continue;
        const QChar mark = c.text.at(c.tokens[m].start);
        if (!QLatin1String(",.;:!?").contains(mark)) continue;
        const Tok before = c.tokens[i - 1].kind;
        if (before != Tok::Word && before != Tok::Number && !(before == Tok::Punct && (c.isPunct(i - 1, QLatin1Char(')')) || c.isPunct(i - 1, QLatin1Char(']'))))) continue;
        const int after = m + 1;
        if (after < c.size()) {
            const Token &a = c.tokens[after];
            if (a.kind == Tok::Word || a.kind == Tok::Number) continue;   // (.txt, 3 .5, :D)
            if (a.kind == Tok::Punct) {
                const QChar q = c.text.at(a.start);
                if (QLatin1String(",.;:!?").contains(q) || q == QChar(0x2026)) continue;   // (a run of marks)
                if (mark == QLatin1Char(':') || mark == QLatin1Char(';') || mark == QLatin1Char('.')) continue;   // (:) ;) and the like)
            }
        }
        if (mark == QLatin1Char(':') && before == Tok::Number && after + 1 < c.size() && c.tokens[after].kind == Tok::Space && c.tokens[after + 1].kind == Tok::Number) continue;   // (3 : 1)
        c.add(c.tokens[i].start, c.tokens[i].end, GK::Punctuation, QCoreApplication::translate("Grammar", "Take out the space before this mark."), QString());
    }
}

// No space between the period (or ? or !) that ends a sentence and the capital that begins the next.
void missingSpaceAfterPeriod(Ctx &c)
{
    for (int i = 1; i + 1 < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Punct) continue;
        const QChar mark = c.text.at(c.tokens[i].start);
        if (!QLatin1String(".?!").contains(mark)) continue;
        if (!c.isWord(i - 1) || !c.isWord(i + 1)) continue;
        const QString before = c.original(i - 1), after = c.original(i + 1);
        if (before.size() < 3 || !before.at(before.size() - 1).isLower() || before.contains(QLatin1Char('-'))) continue;
        if (after.size() < 2 || !after.at(0).isUpper() || after.mid(1) != after.mid(1).toLower()) continue;   // (Data!MyName, Sheet1!A1)
        if (mark == QLatin1Char('.') && (abbreviations().contains(c.tokens[i - 1].lower) || addressEndings().contains(c.tokens[i + 1].lower))) continue;
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

// "everyday" (ordinary) and "every day" (each day), where the words round them make it clear which is meant.
void everyday(Ctx &c)
{
    const QSet<QString> &adverbs = WORDS("almost", "nearly", "practically", "virtually");
    const QSet<QString> &pronouns = WORDS("i", "we", "you", "they", "he", "she");
    const QSet<QString> &possessives = WORDS("the", "an", "our", "my", "your", "their", "his", "her");
    const QSet<QString> &notNoun = WORDS("i", "we", "you", "they", "he", "she", "it", "and", "or", "but", "is", "are", "was", "were", "a", "the", "to", "of", "in", "on", "at", "for", "with", "when", "as", "that");
    for (int i = 0; i < c.size(); ++i) {
        c.step();
        if (c.tokens[i].kind != Tok::Word) continue;
        const QString &l = c.tokens[i].lower;
        if (l == QLatin1String("everyday")) {
            const int p = c.prevWord(i), n = c.nextWord(i);
            if ((p >= 0 && adverbs.contains(c.tokens[p].lower)) || (n >= 0 && pronouns.contains(c.tokens[n].lower)))
                replaceToken(c, i, QStringLiteral("every day"), GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"every day\" (each day) here."));
        } else if (l == QLatin1String("every")) {
            const int j = c.nextWord(i);
            if (j < 0 || c.tokens[j].lower != QLatin1String("day")) continue;
            const int p = c.prevWord(i), n = c.nextWord(j);
            if (p >= 0 && possessives.contains(c.tokens[p].lower) && n >= 0 && c.tokens[n].kind == Tok::Word && !notNoun.contains(c.tokens[n].lower))
                c.add(c.tokens[i].start, c.tokens[j].end, GK::WordChoice, QCoreApplication::translate("Grammar", "Use \"everyday\" (ordinary) here."), matchCase(c.original(i), QStringLiteral("everyday")));
        }
    }
}

// The rules: to add one, write its function above and put it here.
struct Rule {
    const char *name;
    void (*run)(Ctx &);
};
const Rule kRules[] = {
    {"repeated-word", repeatedWord},
    {"a-an", articles},
    {"sentence-start", lowercaseSentenceStart},
    {"its", itsAndItIs},
    {"their-there", theirThereTheyAre},
    {"your-youre", yourYouAre},
    {"then-than", thenAndThan},
    {"lose-loose", loseAndLoose},
    {"could-of", couldOf},
    {"double-space", doubleSpace},
    {"space-before-mark", spaceBeforeMark},
    {"missing-space", missingSpaceAfterPeriod},
    {"alot", alot},
    {"everyday", everyday},
};

} // namespace

QVector<GrammarIssue> check(const QString &text, quint64 *work)
{
    Ctx c(text);
    tokenize(c);
    for (const Rule &r : kRules) r.run(c);
    // In order of position; of two that overlap, the one that starts first stays.
    std::stable_sort(c.issues.begin(), c.issues.end(), [](const GrammarIssue &a, const GrammarIssue &b) { return a.start < b.start; });
    QVector<GrammarIssue> out;
    int last = 0;
    for (const GrammarIssue &g : c.issues) {
        if (g.start < last) continue;
        out << g;
        last = g.end;
    }
    if (work) *work = c.steps;
    return out;
}

} // namespace jp::grammar
