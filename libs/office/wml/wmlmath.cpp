// Equations as a line of text (see wmlmath.h).

#include "wml/wmlmath.h"

#include "wml/wml.h"

#include <QByteArray>
#include <QStringList>
#include <algorithm>
#include <atomic>
#include <utility>

namespace jp::office::wml {

bool styledLetter(uint ch, StyledLetter *out)
{
    if (ch < 0x1D400 || ch > 0x1D7FF) return false;
    // The Latin alphabets (52 letters each): bold, italic, bold italic, script, bold script, fraktur, double-struck, bold fraktur, sans-serif and its bold, italic, bold italic, monospace.
    static const struct { uint from; char style; } latin[] = {{0x1D400, 'b'}, {0x1D434, 'i'}, {0x1D468, 'j'}, {0x1D49C, 'p'}, {0x1D4D0, 'b'}, {0x1D504, 'p'}, {0x1D538, 'd'},
                                                              {0x1D56C, 'b'}, {0x1D5A0, 'p'}, {0x1D5D4, 'b'}, {0x1D608, 'i'}, {0x1D63C, 'j'}, {0x1D670, 'p'}};
    for (const auto &a : latin)
        if (ch >= a.from && ch < a.from + 52) {
            const uint k = ch - a.from;
            *out = {k < 26 ? 'A' + k : 'a' + (k - 26), a.style};
            return true;
        }
    // The Greek alphabets (58 each: 25 capitals, a nabla, 25 small letters, a partial differential, and 6 variants): bold, italic, bold italic, sans-serif bold, sans-serif bold italic.
    static const struct { uint from; char style; } greek[] = {{0x1D6A8, 'b'}, {0x1D6E2, 'i'}, {0x1D71C, 'j'}, {0x1D756, 'b'}, {0x1D790, 'j'}};
    for (const auto &a : greek)
        if (ch >= a.from && ch < a.from + 58) {
            const uint k = ch - a.from;
            static const ushort variants[] = {0x3F5, 0x3D1, 0x3F0, 0x3D5, 0x3F1, 0x3D6};
            const uint base = k < 17 ? 0x391 + k : k == 17 ? 0x3F4 : k < 25 ? 0x3A3 + (k - 18) : k == 25 ? 0x2207 : k < 51 ? 0x3B1 + (k - 26) : k == 51 ? 0x2202 : variants[k - 52];
            *out = {base, a.style};
            return true;
        }
    // The digits (10 each): bold, double-struck, sans-serif, sans-serif bold, monospace.
    if (ch >= 0x1D7CE && ch < 0x1D7CE + 50) {
        static const char styles[] = {'b', 'd', 'p', 'b', 'p'};
        const uint k = ch - 0x1D7CE;
        *out = {'0' + k % 10, styles[k / 10]};
        return true;
    }
    if (ch == 0x1D6A4 || ch == 0x1D6A5) {   // (the dotless i and j)
        *out = {ch == 0x1D6A4 ? 0x131u : 0x237u, 'i'};
        return true;
    }
    if (ch == 0x1D7CA || ch == 0x1D7CB) {   // (the bold digamma)
        *out = {ch == 0x1D7CA ? 0x3DCu : 0x3DDu, 'b'};
        return true;
    }
    return false;
}

namespace {

constexpr int kMostNodes = 50000;
// What a value of the markup that is shown (a bracket, a separator, a sign, an accent) can be of: a few characters, whatever the file puts in it.
constexpr int kMostValueChars = 8;
// (The work of the lines made, for a test: the elements looked at and the characters put together.)
std::atomic<qint64> s_work{0};

bool isRelation(uint c)
{
    switch (c) {
    case '=': case '<': case '>':
    case 0x2260: case 0x2264: case 0x2265: case 0x2248: case 0x2261: case 0x2245: case 0x223C: case 0x221D: case 0x2208: case 0x2209: case 0x220B:
    case 0x2282: case 0x2283: case 0x2286: case 0x2287: case 0x2192: case 0x2190: case 0x2194: case 0x21D2: case 0x21D0: case 0x21D4: case 0x226A: case 0x226B:
    case 0x227A: case 0x227B: case 0x22A5: case 0x2225: case 0x2243: case 0x2254: case 0x21A6:
        return true;
    default:
        return false;
    }
}

bool isBinary(uint c)
{
    switch (c) {
    case '+': case '-':
    case 0x2212: case 0xD7: case 0xF7: case 0xB1: case 0x2213: case 0xB7: case 0x2219: case 0x22C5: case 0x2217: case 0x2218: case 0x2227: case 0x2228: case 0x2229:
    case 0x222A: case 0x2295: case 0x2297: case 0x2216:
        return true;
    default:
        return false;
    }
}

bool isOpening(uint c) { return c == '(' || c == '[' || c == '{' || c == 0x27E8 || c == 0x2308 || c == 0x230A || c == ',' || c == ';' || c == ':'; }

bool isItalicLetter(uint c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= 0x3B1 && c <= 0x3C9); }

// What is more than letters and digits is put in brackets where it is a part of something (a numerator, a limit, a radicand).
QString wrapped(const QString &s)
{
    for (const QChar ch : s)
        if (!ch.isLetterOrNumber() && ch != QLatin1Char('.')) return QLatin1Char('(') + s + QLatin1Char(')');
    return s;
}

// The character an accent over a letter is drawn as in a line (the combining marks are the marks themselves, set beside the letter).
QString accentShown(const QString &chr)
{
    if (chr.size() != 1) return chr;
    switch (chr[0].unicode()) {
    case 0x300: return QStringLiteral("`");
    case 0x301: return QStringLiteral("´");
    case 0x302: return QStringLiteral("^");
    case 0x303: return QStringLiteral("~");
    case 0x304: case 0x305: return QStringLiteral("¯");
    case 0x306: return QStringLiteral("˘");
    case 0x307: return QStringLiteral("˙");
    case 0x308: return QStringLiteral("¨");
    case 0x30C: return QStringLiteral("ˇ");
    case 0x20D7: return QStringLiteral("→");
    default: return chr;
    }
}

class Builder {
public:
    Builder(const XmlTree &tree, int most) : t(tree), maxChars(most) {}
    const XmlTree &t;
    const int maxChars;
    QString out;
    bool operand = false;   // the line so far ends in something that an operator after it works on
    mutable bool cut = false;
    int tight = 0;          // inside a script: no blanks round operators
    int used = 0;
    mutable int nodes = 0;  // the elements looked at, empty ones and those only looked through too (kMostNodes at most)
    int letters = 0, italic = 0, bold = 0;
    int firstRunProps = -1;
    QByteArray runStyle;    // the style of the run being read: "p", "b", "i", "bi", or empty
    bool runNormal = false;

    int child(int n, const char *name) const
    {
        if (n < 0) return -1;
        for (int c = t.firstChild(n); c >= 0; c = t.next(c)) {
            if (++nodes > kMostNodes) {
                cut = true;
                return -1;
            }
            if (t.ns(c) != NsW && t.localName(c) == name) return c;
        }
        return -1;
    }
    // The value of an element (m:val), as much of it as is ever shown.
    QString valOf(int n) const
    {
        if (n < 0) return QString();
        QString v = t.attr(n, "val", t.ns(n));
        if (v.isEmpty()) v = t.attrAny(n, "val");
        if (v.size() > kMostValueChars) {
            v.truncate(kMostValueChars);
            if (v.back().isHighSurrogate()) v.chop(1);
        }
        return v;
    }
    // A property that is on unless it says off (a missing one is `otherwise`).
    bool onOff(int n, bool otherwise) const
    {
        if (n < 0) return otherwise;
        const QString v = valOf(n).toLower();
        return !(v == QLatin1String("0") || v == QLatin1String("false") || v == QLatin1String("off"));
    }

    void add(uint ch)
    {
        out += QChar::fromUcs4(char32_t(ch));
        used += ch > 0xFFFF ? 2 : 1;
    }
    void space()
    {
        if (!out.isEmpty() && !out.endsWith(QLatin1Char(' ')) && !out.endsWith(QChar(QChar::LineSeparator))) add(' ');
    }
    void raw(const QString &s)
    {
        s_work += s.size();
        // (No more than the line may hold is put on it.)
        const int room = std::max(0, maxChars - used);
        if (s.size() > room) {
            out += s.left(room);
            used += room;
            cut = true;
            return;
        }
        out += s;
        used += int(s.size());
    }
    // The strings joined with `between` in the places, as far as the line has room (nothing past it is put together).
    QString joined(const QStringList &list, const QString &between)
    {
        QString got;
        for (int i = 0; i < list.size(); ++i) {
            if (used + got.size() >= maxChars) {
                cut = true;
                break;
            }
            if (i) got += between;
            got += list[i];
        }
        return got;
    }

    // One character of a run's text.
    void put(uint ch)
    {
        if (used >= maxChars) {
            cut = true;
            return;
        }
        StyledLetter sl;
        bool counted = false;
        if (styledLetter(ch, &sl)) {
            ch = sl.base;
            if (QChar::isLetter(ch)) {
                ++letters;
                italic += sl.style == 'i' || sl.style == 'j';
                bold += sl.style == 'b' || sl.style == 'j';
            }
            counted = true;
        }
        if (!counted && QChar::isLetter(ch)) {
            ++letters;
            const bool plain = runNormal || runStyle == "p" || runStyle == "b";
            italic += !plain && isItalicLetter(ch);
            bold += runStyle == "b" || runStyle == "bi";
        }
        if (ch == ' ') {
            space();
            return;
        }
        const bool relation = isRelation(ch), binary = isBinary(ch);
        if (tight == 0 && (relation || (binary && operand))) {
            space();
            add(ch);
            space();
            operand = false;
            return;
        }
        add(ch);
        operand = !(relation || binary || isOpening(ch));
    }
    void putText(const QString &s)
    {
        for (qsizetype i = 0; i < s.size(); ++i) {
            uint ch = s[i].unicode();
            if (QChar::isHighSurrogate(ch) && i + 1 < s.size() && QChar::isLowSurrogate(s[i + 1].unicode())) ch = QChar::surrogateToUcs4(ch, s[i + 1].unicode()), ++i;
            put(ch);
        }
    }

    // The text of what is inside an element, apart from the line so far.
    QString captured(int n, int depth, bool script = false)
    {
        ++s_work;
        if (++nodes > kMostNodes) {
            cut = true;
            return QString();
        }
        QString saved;
        std::swap(saved, out);
        const bool before = operand;
        operand = false;
        if (script) ++tight;
        children(n, depth);
        if (script) --tight;
        QString got = out.trimmed();
        out = saved;
        operand = before;
        return got;
    }
    QString part(int n, const char *name, int depth, bool script = false)
    {
        const int c = child(n, name);
        return c < 0 ? QString() : captured(c, depth, script);
    }
    QStringList parts(int n, const char *name, int depth)
    {
        QStringList got;
        for (int c = t.firstChild(n); c >= 0 && used < maxChars; c = t.next(c)) {
            if (nodes > kMostNodes) break;
            if (t.ns(c) != NsW && t.localName(c) == name) got << captured(c, depth);
            else ++nodes;
        }
        return got;
    }

    void run(int r)
    {
        runStyle.clear();
        runNormal = false;
        if (const int pr = child(r, "rPr"); pr >= 0) {
            runStyle = valOf(child(pr, "sty")).toLatin1();
            runNormal = child(pr, "nor") >= 0 && onOff(child(pr, "nor"), true);
        }
        for (int c = t.firstChild(r); c >= 0 && used < maxChars; c = t.next(c)) {
            const QByteArray name = t.localName(c);
            if (name == "rPr" && t.ns(c) == NsW) {
                if (firstRunProps < 0) firstRunProps = c;
            } else if (name == "t") {
                QString s = t.text(c);
                if (t.attrAny(c, "space") != QLatin1String("preserve")) s = s.trimmed();
                putText(s);
            } else if (name == "tab" || name == "br" || name == "cr") {
                space();
            }
        }
        runStyle.clear();
        runNormal = false;
    }

    void children(int n, int depth)
    {
        for (int c = t.firstChild(n); c >= 0 && used < maxChars; c = t.next(c)) element(c, depth + 1);
    }

    void element(int n, int depth)
    {
        if (depth > kMaxEquationDepth) {
            cut = true;
            return;
        }
        ++s_work;
        if (++nodes > kMostNodes || used >= maxChars) {
            cut = true;
            return;
        }
        const QByteArray name = t.localName(n);
        if (t.ns(n) == NsW) {
            // (A tracked change's insertion is shown, its deletion is not.)
            if (name == "ins" || name == "moveTo" || name == "smartTag" || name == "customXml") children(n, depth);
            return;
        }
        if (name.endsWith("Pr")) return;   // (properties: oMathParaPr, rPr, fPr, ctrlPr...)
        if (name == "r") {
            run(n);
        } else if (name == "oMathPara") {
            bool first = true;
            for (int c = t.firstChild(n); c >= 0 && used < maxChars; c = t.next(c)) {
                if (t.ns(c) == NsW || t.localName(c) != "oMath") continue;
                if (!first) {
                    out += QChar(QChar::LineSeparator);
                    ++used;
                }
                first = false;
                operand = false;
                element(c, depth + 1);
            }
        } else if (name == "f") {
            const QString num = part(n, "num", depth), den = part(n, "den", depth);
            const bool noBar = valOf(child(child(n, "fPr"), "type")) == QLatin1String("noBar");
            raw(wrapped(num) + (noBar ? QStringLiteral(" ") : QStringLiteral("/")) + wrapped(den));
            operand = true;
        } else if (name == "sSup" || name == "sSub" || name == "sSubSup") {
            const QString base = part(n, "e", depth), sub = part(n, "sub", depth, true), sup = part(n, "sup", depth, true);
            raw(base + sub + sup);
            operand = true;
        } else if (name == "sPre") {
            const QString sub = part(n, "sub", depth, true), sup = part(n, "sup", depth, true), base = part(n, "e", depth);
            raw(sub + sup + base);
            operand = true;
        } else if (name == "rad") {
            const bool noDegree = onOff(child(child(n, "radPr"), "degHide"), false);
            const QString degree = noDegree ? QString() : part(n, "deg", depth), root = part(n, "e", depth);
            raw(wrapped(degree) + QChar(0x221A) + wrapped(root));
            operand = true;
        } else if (name == "nary") {
            const int pr = child(n, "naryPr"), chr = child(pr, "chr");
            const QString sign = chr >= 0 ? valOf(chr) : QString(QChar(0x222B));
            const QString sub = onOff(child(pr, "subHide"), false) ? QString() : part(n, "sub", depth);
            const QString sup = onOff(child(pr, "supHide"), false) ? QString() : part(n, "sup", depth);
            const QString body = part(n, "e", depth);
            QStringList pieces;
            if (!sign.isEmpty()) pieces << sign;
            if (!sub.isEmpty()) pieces << wrapped(sub);
            if (!sup.isEmpty()) pieces << wrapped(sup);
            if (!body.isEmpty()) pieces << body;
            raw(pieces.join(QLatin1Char(' ')));
            operand = true;
        } else if (name == "d") {
            const int pr = child(n, "dPr");
            auto chr = [&](const char *which, const QString &otherwise) {
                const int c = child(pr, which);
                return c >= 0 ? valOf(c) : otherwise;
            };
            raw(chr("begChr", QStringLiteral("(")) + joined(parts(n, "e", depth), chr("sepChr", QStringLiteral("|"))) + chr("endChr", QStringLiteral(")")));
            operand = true;
        } else if (name == "func") {
            const QString function = part(n, "fName", depth), arg = part(n, "e", depth);
            raw(function + (function.isEmpty() || arg.isEmpty() ? QString() : QStringLiteral(" ")) + arg);
            operand = true;
        } else if (name == "limLow" || name == "limUpp") {
            const QString base = part(n, "e", depth), limit = part(n, "lim", depth);
            raw(base + (base.isEmpty() || limit.isEmpty() ? QString() : QStringLiteral(" ")) + limit);
            operand = true;
        } else if (name == "m") {
            QStringList rows;
            for (int c = t.firstChild(n); c >= 0 && used < maxChars && nodes <= kMostNodes; c = t.next(c)) {
                if (t.ns(c) != NsW && t.localName(c) == "mr") rows << joined(parts(c, "e", depth), QStringLiteral(" "));
                else ++nodes;
            }
            raw(joined(rows, QStringLiteral("; ")));
            operand = true;
        } else if (name == "eqArr") {
            raw(joined(parts(n, "e", depth), QStringLiteral(" ")));
            operand = true;
        } else if (name == "acc") {
            const int chr = child(child(n, "accPr"), "chr");
            raw(part(n, "e", depth) + accentShown(chr >= 0 ? valOf(chr) : QString(QChar(0x302))));
            operand = true;
        } else if (name == "groupChr") {
            const int chr = child(child(n, "groupChrPr"), "chr");
            raw(part(n, "e", depth) + (chr >= 0 ? valOf(chr) : QString(QChar(0x23DF))));
            operand = true;
        } else if (name == "phant") {
            if (onOff(child(child(n, "phantPr"), "show"), true)) {
                raw(part(n, "e", depth));
                operand = true;
            }
        } else {
            children(n, depth);   // (e, num, box, bar, borderBox, and any element this does not know: what is in it)
        }
    }
};

} // namespace

qint64 equationWorkForTest() { return s_work.load(); }
void resetEquationWorkForTest() { s_work = 0; }

Equation equationText(const XmlTree &t, int n, int maxChars)
{
    Equation eq;
    if (maxChars <= 0) return eq;
    Builder b(t, maxChars);
    b.element(n, 0);
    eq.text = b.out.trimmed();
    if (eq.text.size() > maxChars) {
        eq.text.truncate(maxChars);
        if (eq.text.size() && eq.text.back().isHighSurrogate()) eq.text.chop(1);
        b.cut = true;
    }
    eq.cut = b.cut;
    eq.runProps = b.firstRunProps;
    eq.style = QString(b.letters > 0 && b.italic * 2 > b.letters ? QStringLiteral("i") : QString()) + (b.letters > 0 && b.bold * 2 > b.letters ? QStringLiteral("b") : QString());
    return eq;
}

} // namespace jp::office::wml
