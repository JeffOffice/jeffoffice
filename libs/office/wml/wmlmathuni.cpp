#include "wml/wmlmathuni.h"

#include "wml/wmlmathast.h"

namespace jp::office::wml {

using namespace mathtree;

namespace {

const QChar kRoot(0x221A), kCubeRoot(0x221B), kFourthRoot(0x221C), kOpenFence(0x251C), kCloseFence(0x2524), kOpenGroup(0x3016), kCloseGroup(0x3017), kBlackSquare(0x25A0);

bool isCombining(uint c) { return (c >= 0x300 && c <= 0x36F) || (c >= 0x20D0 && c <= 0x20FF); }
bool isAscii(QChar c) { return (c >= QLatin1Char('a') && c <= QLatin1Char('z')) || (c >= QLatin1Char('A') && c <= QLatin1Char('Z')); }

// A character a fraction or script can take as its own: a letter, a digit, or one of these signs.
bool isOperandCp(uint c)
{
    return QChar::isLetter(c) || QChar::isDigit(c) || c == 0x221E || c == 0x2202 || c == 0x2207 || c == 0x210F || c == 0x2113 || c == 0x2205 || c == 0x2032 || c == 0xB0;
}

QChar closerOf(QChar open)
{
    switch (open.unicode()) {
    case '(': return QLatin1Char(')');
    case '[': return QLatin1Char(']');
    case '{': return QLatin1Char('}');
    case 0x27E8: return QChar(0x27E9);
    case 0x230A: return QChar(0x230B);
    case 0x2308: return QChar(0x2309);
    default: return QChar();
    }
}

// What the operand of a fraction, script, or root is: a bracket group typed as one (and a group that shows no brackets) gives its inside.
Seq operandSeq(Node n)
{
    if (n.kind == Kind::Group) return std::move(n.part[0]);
    if (n.kind == Kind::Delim && n.group && n.part.size() == 1) return std::move(n.part[0]);
    if (n.kind == Kind::Run && n.text.isEmpty() && n.style == 0) return {};
    return Seq{std::move(n)};
}
// ... and as a base or a term: only a group that shows no brackets gives its inside.
Seq baseSeq(Node n)
{
    if (n.kind == Kind::Group) return std::move(n.part[0]);
    if (n.kind == Kind::Run && n.text.isEmpty() && n.style == 0) return {};
    return Seq{std::move(n)};
}

class Reader {
public:
    explicit Reader(const QString &text) : m_t(text) {}
    Seq all()
    {
        Seq s = sequence(QString(), 0);
        return m_failed ? Seq() : s;
    }
    bool failed() const { return m_failed; }

private:
    QString m_t;
    qsizetype m_i = 0;
    bool m_failed = false;

    bool more() const { return m_i < m_t.size(); }
    QChar peek() const { return m_i < m_t.size() ? m_t[m_i] : QChar(); }
    void fail()
    {
        m_failed = true;
        m_i = m_t.size();
    }
    void blanks()
    {
        while (more() && peek().unicode() <= 0x20) ++m_i;
    }
    uint cpAt(qsizetype i, int *len) const
    {
        if (QChar::isHighSurrogate(m_t[i].unicode()) && i + 1 < m_t.size() && QChar::isLowSurrogate(m_t[i + 1].unicode())) {
            *len = 2;
            return QChar::surrogateToUcs4(m_t[i], m_t[i + 1]);
        }
        *len = 1;
        return m_t[i].unicode();
    }
    bool stopAt(const QString &stops) const { return more() && stops.contains(peek()); }
    static void add(Seq &out, Node n)
    {
        if (n.kind == Kind::Group) {
            for (Node &c : n.part[0]) out.push_back(std::move(c));
        } else if (!(n.kind == Kind::Run && n.text.isEmpty() && n.style == 0)) {
            out.push_back(std::move(n));
        }
    }

    // Terms until a stop character (left for the caller) or the end.
    Seq sequence(const QString &stops, int depth)
    {
        Seq out;
        if (depth > kMostEquationDepth) {
            fail();
            return out;
        }
        while (!m_failed) {
            blanks();
            if (!more() || stopAt(stops)) break;
            const qsizetype before = m_i;
            add(out, term(stops, depth));
            if (m_i == before) fail();
        }
        return out;
    }

    Node term(const QString &stops, int depth)
    {
        Node n = element(stops, depth);
        while (!m_failed) {
            blanks();
            if (peek() != QLatin1Char('/') || !n.operand) break;
            ++m_i;
            blanks();
            Seq den;
            if (more() && !stopAt(stops)) den = operandSeq(element(stops, depth));
            n = structure(Kind::Frac, {operandSeq(std::move(n)), std::move(den)});
        }
        return n;
    }

    // An atom, with the accents (combining marks) that follow it.
    Node accented(const QString &stops, int depth)
    {
        Node a = atom(stops, depth);
        while (!m_failed && more() && isCombining(peek().unicode())) {
            Node acc = structure(Kind::Accent, {baseSeq(std::move(a))});
            acc.text = QString(peek());
            ++m_i;
            a = std::move(acc);
        }
        return a;
    }

    Node element(const QString &stops, int depth)
    {
        Node a = accented(stops, depth);
        if (m_failed) return a;
        Seq sub, sup;
        bool hasSub = false, hasSup = false;
        scripts(sub, sup, hasSub, hasSup, stops, depth);
        if (!hasSub && !hasSup) return a;
        Node s = structure(Kind::Script, {baseSeq(std::move(a)), std::move(sub), std::move(sup)});
        s.hasA = hasSub;
        s.hasB = hasSup;
        return s;
    }

    // ^ and _ operands after an element, a large operator, or a function name.
    void scripts(Seq &sub, Seq &sup, bool &hasSub, bool &hasSup, const QString &stops, int depth)
    {
        while (!m_failed) {
            blanks();
            const QChar c = peek();
            if (c != QLatin1Char('^') && c != QLatin1Char('_')) break;
            ++m_i;
            blanks();
            Seq operand = scriptOperand(c, stops, depth + 1);   // (an operator or a function name in the operand nests one deeper)
            Seq &slot = c == QLatin1Char('^') ? sup : sub;
            bool &has = c == QLatin1Char('^') ? hasSup : hasSub;
            if (has) {
                Node s = structure(Kind::Script, {std::move(slot), Seq(), Seq()});
                if (c == QLatin1Char('^')) s.hasB = true, s.part[2] = std::move(operand);
                else s.hasA = true, s.part[1] = std::move(operand);
                slot = Seq{std::move(s)};
            } else {
                slot = std::move(operand);
                has = true;
            }
        }
    }

    Seq scriptOperand(QChar kind, const QString &stops, int depth)
    {
        if (depth > kMostEquationDepth) {
            fail();
            return {};
        }
        if (!more() || stopAt(stops)) return {};
        Node a = accented(stops, depth);
        if (m_failed) return {};
        blanks();
        if (peek() == kind) {
            ++m_i;
            blanks();
            Seq inner = scriptOperand(kind, stops, depth + 1);
            Node s = structure(Kind::Script, {baseSeq(std::move(a)), Seq(), Seq()});
            if (kind == QLatin1Char('^')) s.hasB = true, s.part[2] = std::move(inner);
            else s.hasA = true, s.part[1] = std::move(inner);
            return Seq{std::move(s)};
        }
        return operandSeq(std::move(a));
    }

    Node atom(const QString &stops, int depth)
    {
        if (depth > kMostEquationDepth) {
            fail();
            return Node();
        }
        blanks();
        if (!more() || stopAt(stops)) return run(QString());
        int len = 1;
        const uint c = cpAt(m_i, &len);
        if (c == '"') return normalText();
        if (c == '\\') return command(stops, depth);
        if (c == kRoot.unicode() || c == kCubeRoot.unicode() || c == kFourthRoot.unicode()) return radical(stops, depth);
        if (c == kOpenFence.unicode()) return fenced(depth);
        if (c == kOpenGroup.unicode()) {
            ++m_i;
            Seq inner = sequence(QString(kCloseGroup), depth + 1);
            if (peek() == kCloseGroup) ++m_i;
            Node g = structure(Kind::Group, {std::move(inner)});
            return g;
        }
        if (c == kBlackSquare.unicode()) {
            ++m_i;
            return matrix(depth);
        }
        if (const QChar close = closerOf(QChar(ushort(c))); !close.isNull() && len == 1) return bracketed(QChar(ushort(c)), close, depth);
        if (len == 1) {
            if (const NaryOp *op = naryByChar(QString(QChar(ushort(c))))) {
                ++m_i;
                return nary(QString(QChar(op->chr)), stops, depth);
            }
        }
        if (len == 1 && QChar::isDigit(c) && QChar(ushort(c)).isDigit()) {
            QString digits;
            while (more() && (peek().isDigit() || (peek() == QLatin1Char('.') && m_i + 1 < m_t.size() && m_t[m_i + 1].isDigit()))) digits += m_t[m_i++];
            return run(digits, 0, true);
        }
        if (len == 1 && isAscii(QChar(ushort(c))) && !(m_i > 0 && isAscii(m_t[m_i - 1]))) {   // (a function name begins a word)
            qsizetype end = m_i;
            while (end < m_t.size() && isAscii(m_t[end])) ++end;
            const QString word = m_t.mid(m_i, end - m_i);
            if (isFunctionName(word)) {
                m_i = end;
                return function(word, stops, depth);
            }
        }
        m_i += len;
        return run(QString::fromUcs4(reinterpret_cast<const char32_t *>(&c), 1), 0, isOperandCp(c));
    }

    Node normalText()
    {
        ++m_i;
        QString s;
        while (more()) {
            const QChar c = m_t[m_i];
            if (c == QLatin1Char('\\') && m_i + 1 < m_t.size()) {
                s += m_t[m_i + 1];
                m_i += 2;
            } else if (c == QLatin1Char('"')) {
                ++m_i;
                break;
            } else {
                s += c;
                ++m_i;
            }
        }
        return run(s, 'n', true);
    }

    Node command(const QString &stops, int depth)
    {
        ++m_i;
        if (!more()) return run(QString());
        if (!isAscii(peek())) {
            int len = 1;
            const uint c = cpAt(m_i, &len);
            m_i += len;
            return run(QString::fromUcs4(reinterpret_cast<const char32_t *>(&c), 1), 0, isOperandCp(c));
        }
        QString name;
        while (more() && isAscii(peek())) name += m_t[m_i++];
        if (name == QLatin1String("matrix")) return matrix(depth);
        if (name == QLatin1String("sqrt")) {
            m_i -= 5;   // (back to the backslash: the root reads its own sign or name)
            return radical(stops, depth);
        }
        if (name == QLatin1String("overline") || name == QLatin1String("underline")) {
            Node bar = structure(Kind::Bar, {operandSeq(accented(stops, depth + 1))});
            bar.hasA = name == QLatin1String("overline");
            return bar;
        }
        if (const ushort mark = accentMark(name)) {
            Node acc = structure(Kind::Accent, {operandSeq(accented(stops, depth + 1))});
            acc.text = QString(QChar(mark));
            return acc;
        }
        if (const NaryOp *op = naryByName(name)) return nary(QString(QChar(op->chr)), stops, depth);
        const QString sym = symbolByName(name);
        if (!sym.isEmpty()) return run(sym, 0, true);
        return run(QLatin1Char('\\') + name, 0, true);
    }

    // The root sign (or \sqrt, which is at m_i), a bracket group or an atom after it.
    Node radical(const QString &stops, int depth)
    {
        Seq degree;
        bool hasDegree = false;
        if (peek() == kCubeRoot || peek() == kFourthRoot) {
            degree = Seq{run(peek() == kCubeRoot ? QStringLiteral("3") : QStringLiteral("4"), 0, true)};
            hasDegree = true;
        }
        m_i += peek() == QLatin1Char('\\') ? 5 : 1;
        blanks();
        Seq radicand;
        if (peek() == QLatin1Char('(')) {
            ++m_i;
            Seq first = sequence(QStringLiteral("&)"), depth + 1);
            if (peek() == QLatin1Char('&')) {
                ++m_i;
                degree = std::move(first);
                hasDegree = true;
                radicand = sequence(QStringLiteral(")"), depth + 1);
            } else {
                radicand = std::move(first);
            }
            if (peek() == QLatin1Char(')')) ++m_i;
        } else {
            radicand = operandSeq(accented(stops, depth + 1));
        }
        Node r = structure(Kind::Rad, {std::move(degree), std::move(radicand)});
        r.hasA = hasDegree;
        return r;
    }

    Node bracketed(QChar open, QChar close, int depth)
    {
        ++m_i;
        Node d;
        d.kind = Kind::Delim;
        d.operand = true;
        d.group = true;
        d.text = QString(open);
        d.text2 = QString(close);
        d.text3 = QStringLiteral("|");
        const QString stops = QString(close) + QLatin1Char('|');
        while (!m_failed) {
            d.part.push_back(sequence(stops, depth + 1));
            if (peek() != QLatin1Char('|')) break;
            ++m_i;
        }
        if (peek() == close) ++m_i;
        else d.text2.clear();
        return d;
    }

    // The open sign, a character, the text, the close sign, and a character.
    Node fenced(int depth)
    {
        ++m_i;
        Node d;
        d.kind = Kind::Delim;
        d.operand = true;
        d.text3 = QStringLiteral("|");
        auto character = [&] {
            if (!more()) return QString();
            int len = 1;
            const uint c = cpAt(m_i, &len);
            m_i += len;
            return c == ' ' ? QString() : QString::fromUcs4(reinterpret_cast<const char32_t *>(&c), 1);
        };
        d.text = character();
        while (!m_failed) {
            d.part.push_back(sequence(QString(kCloseFence) + QLatin1Char('|'), depth + 1));
            if (peek() != QLatin1Char('|')) break;
            ++m_i;
        }
        if (peek() == kCloseFence) {
            ++m_i;
            d.text2 = character();
        }
        return d;
    }

    // \matrix( ... ): cells divided by &, rows by @ or #.
    Node matrix(int depth)
    {
        blanks();
        if (peek() != QLatin1Char('(')) {
            fail();
            return Node();
        }
        ++m_i;
        std::vector<std::vector<Seq>> grid(1);
        int cells = 0;
        while (!m_failed) {
            grid.back().push_back(sequence(QStringLiteral("&@#)"), depth + 1));
            if (++cells > kMostMatrixCells || int(grid.back().size()) > kMostMatrixSide) {
                fail();
                break;
            }
            const QChar c = peek();
            if (c == QLatin1Char('&')) {
                ++m_i;
            } else if (c == QLatin1Char('@') || c == QLatin1Char('#')) {
                ++m_i;
                if (int(grid.size()) >= kMostMatrixSide) {
                    fail();
                    break;
                }
                grid.emplace_back();
            } else {
                if (c == QLatin1Char(')')) ++m_i;
                break;
            }
        }
        if (m_failed) return Node();
        if (grid.size() > 1 && grid.back().size() == 1 && grid.back()[0].empty()) grid.pop_back();
        Node m;
        m.kind = Kind::Matrix;
        m.operand = true;
        for (const auto &row : grid) m.cols = std::max(m.cols, int(row.size()));
        for (auto &row : grid) {
            row.resize(size_t(m.cols));
            for (Seq &cell : row) m.part.push_back(std::move(cell));
        }
        return m;
    }

    Node nary(const QString &chr, const QString &stops, int depth)
    {
        Node n = structure(Kind::Nary, {Seq(), Seq(), Seq()});
        n.text = chr;
        bool beside = naryBeside(n);
        blanks();
        if (m_t.mid(m_i, 7) == QLatin1String("\\limits") && !(m_i + 7 < m_t.size() && isAscii(m_t[m_i + 7]))) {
            m_i += 7;
            n.limits = beside ? 1 : 0;
        } else if (m_t.mid(m_i, 9) == QLatin1String("\\nolimits") && !(m_i + 9 < m_t.size() && isAscii(m_t[m_i + 9]))) {
            m_i += 9;
            n.limits = beside ? 0 : 2;
        }
        scripts(n.part[0], n.part[1], n.hasA, n.hasB, stops, depth);
        const qsizetype save = m_i;
        blanks();
        if (more() && !stopAt(stops) && !m_failed) {
            Node t = term(stops, depth + 1);
            if (t.operand) n.part[2] = baseSeq(std::move(t));
            else m_i = save;
        }
        return n;
    }

    Node function(const QString &name, const QString &stops, int depth)
    {
        Seq nm{run(name, 'p', true)};
        Seq sub, sup;
        bool hasSub = false, hasSup = false;
        scripts(sub, sup, hasSub, hasSup, stops, depth);
        if (hasSub && !hasSup && isLimitFunction(name)) {
            nm = Seq{structure(Kind::LimLow, {std::move(nm), std::move(sub)})};
        } else if (hasSub || hasSup) {
            Node s = structure(Kind::Script, {std::move(nm), std::move(sub), std::move(sup)});
            s.hasA = hasSub;
            s.hasB = hasSup;
            nm = Seq{std::move(s)};
        }
        const qsizetype save = m_i;
        blanks();
        if (more() && !stopAt(stops) && !m_failed) {
            Node t = term(stops, depth + 1);
            if (t.operand) return structure(Kind::Func, {std::move(nm), baseSeq(std::move(t))});
            m_i = save;
        }
        return std::move(nm[0]);
    }
};

class Printer {
public:
    bool ok = true;
    QString all(const Seq &items) { return seq(items, false); }

private:
    enum Pos { Num, ScriptOp, RadOp, Base, Keep };

    static bool isNumber(const QString &s)
    {
        if (s.isEmpty() || !s[0].isDigit() || !s.back().isDigit()) return false;
        bool dot = false;
        for (const QChar c : s) {
            if (c == QLatin1Char('.')) {
                if (dot) return false;
                dot = true;
            } else if (!c.isDigit()) {
                return false;
            }
        }
        return true;
    }
    static bool atomRun(const Node &n)
    {
        if (n.style == 'n') return true;
        if (n.style == 'b') return false;
        if (isNumber(n.text)) return true;
        const QList<uint> cps = n.text.toUcs4();
        return cps.size() == 1 && isOperandCp(cps[0]) && !isCombining(cps[0]);
    }
    static bool bareOk(const Node &n, Pos pos)
    {
        switch (n.kind) {
        case Kind::Run: return atomRun(n) || (pos == Base && n.style == 'p' && isFunctionName(n.text));
        case Kind::Rad: case Kind::Matrix: case Kind::Accent: case Kind::Bar: return true;
        case Kind::Script: return pos == Num || pos == Keep;
        case Kind::Delim: return pos == Base || pos == Keep;
        case Kind::Frac: case Kind::Func: case Kind::Nary: case Kind::LimLow: return pos == Keep;
        default: return false;
        }
    }
    // An operand written so that the reader takes it back as the same items.
    QString operand(const Seq &items, Pos pos)
    {
        if (items.size() == 1 && bareOk(items[0], pos)) return item(items[0], false);
        const bool strip = pos != Base && pos != Keep;
        const QString inner = seq(items, strip);   // (a | inside brackets divides arguments)
        return strip ? QLatin1Char('(') + inner + QLatin1Char(')') : QString(kOpenGroup) + inner + QString(kCloseGroup);
    }
    static void append(QString &out, const QString &s)
    {
        if (!out.isEmpty() && !s.isEmpty() && isAscii(out.back()) && isAscii(s[0])) {
            qsizetype a = out.size();
            while (a > 0 && isAscii(out[a - 1])) --a;
            qsizetype b = 0;
            while (b < s.size() && isAscii(s[b])) ++b;
            const QString trailing = out.mid(a);
            if (isFunctionName(trailing) || isFunctionName(trailing + s.left(b)) || isFunctionName(s.left(b))) out += QLatin1Char(' ');   // (a function name begins a word)
        }
        out += s;
    }

    QString seq(const Seq &items, bool inDelim)
    {
        // (A bracket in a run, with a bracket in the same row that closes it, is written as itself and read back as brackets: counted character by character, so that the text
        // of one run and of several runs print the same.)
        std::vector<std::vector<char>> bare(items.size());
        std::vector<std::pair<size_t, qsizetype>> open;
        for (size_t i = 0; i < items.size(); ++i) {
            const Node &n = items[i];
            if (n.kind != Kind::Run || n.style == 'n' || n.style == 'b' || (n.style == 'p' && isFunctionName(n.text))) continue;
            bare[i].assign(size_t(n.text.size()), 0);
            for (qsizetype j = 0; j < n.text.size(); ++j) {
                const QChar c = n.text[j];
                if (!closerOf(c).isNull()) {
                    open.push_back({i, j});
                } else if (!open.empty() && closerOf(items[open.back().first].text[open.back().second]) == c) {
                    bare[open.back().first][size_t(open.back().second)] = bare[i][size_t(j)] = 1;
                    open.pop_back();
                }
            }
        }
        QString out;
        int level = 0;
        for (size_t i = 0; i < items.size() && ok; ++i) {
            const QString piece = items[i].kind == Kind::Run ? runText(items[i], inDelim, bare[i].empty() ? nullptr : &bare[i], &level) : item(items[i], inDelim || level > 0);
            // (A number after a structure that ends in an operand would be read as more of that operand.)
            if (i > 0 && items[i - 1].kind != Kind::Run && !out.isEmpty() && !piece.isEmpty() && out.back().isDigit() && (piece[0].isDigit() || piece[0] == QLatin1Char('.'))) out += QLatin1Char(' ');
            append(out, piece);
            if (out.size() > kMostEquationInput * 4) ok = false;
        }
        return out;
    }

    QString runText(const Node &n, bool inDelim, const std::vector<char> *bare, int *level)
    {
        if (n.style == 'b') {
            ok = false;
            return QString();
        }
        QString out;
        if (n.style == 'n') {
            out = QStringLiteral("\"");
            for (const QChar c : n.text) {
                if (c == QLatin1Char('"') || c == QLatin1Char('\\')) out += QLatin1Char('\\');
                out += c;
            }
            return out + QLatin1Char('"');
        }
        if (n.style == 'p' && isFunctionName(n.text)) return n.text;
        static const QString specials = QStringLiteral("\\^_/()[]{}&@#\"") + QString(kRoot) + QString(kCubeRoot) + QString(kFourthRoot) + QString(kOpenFence) + QString(kCloseFence) + QString(kOpenGroup)
                                        + QString(kCloseGroup) + QString(kBlackSquare) + QString(QChar(0x27E8)) + QString(QChar(0x27E9)) + QString(QChar(0x230A)) + QString(QChar(0x230B))
                                        + QString(QChar(0x2308)) + QString(QChar(0x2309));
        for (qsizetype i = 0; i < n.text.size(); ++i) {
            const QChar c = n.text[i];
            const bool nary = naryByChar(QString(c)) != nullptr;
            const bool plainBracket = bare && (*bare)[size_t(i)];
            if (plainBracket && !closerOf(c).isNull() && level) ++*level;
            QString piece(c);
            const bool within = inDelim || (level && *level > 0);
            if (c == QLatin1Char(' ') || isCombining(c.unicode()) || nary || (c == QLatin1Char('|') && within) || (specials.contains(c) && !plainBracket)) piece.prepend(QLatin1Char('\\'));
            append(out, piece);
            if (plainBracket && closerOf(c).isNull() && level) --*level;
        }
        return out;
    }

    QString item(const Node &n, bool inDelim)
    {
        switch (n.kind) {
        case Kind::Run: return runText(n, inDelim, nullptr, nullptr);
        case Kind::Frac: return operand(n.part[0], Num) + QLatin1Char('/') + operand(n.part[1], Num);
        case Kind::Script: {
            QString out = operand(n.part[0], Base);
            if (n.hasA) out += QLatin1Char('_') + operand(n.part[1], ScriptOp);
            if (n.hasB) out += QLatin1Char('^') + operand(n.part[2], ScriptOp);
            return out;
        }
        case Kind::Rad:
            if (n.hasA) return QString(kRoot) + QLatin1Char('(') + seq(n.part[0], false) + QLatin1Char('&') + seq(n.part[1], false) + QLatin1Char(')');
            return QString(kRoot) + operand(n.part[1], RadOp);
        case Kind::Nary: {
            QString out = n.text;
            const bool beside = naryBeside(n);
            if (n.limits == 1 && beside) out += QStringLiteral("\\limits");
            else if (n.limits == 2 && !beside) out += QStringLiteral("\\nolimits");
            if (n.hasA) out += QLatin1Char('_') + operand(n.part[0], ScriptOp);
            if (n.hasB) out += QLatin1Char('^') + operand(n.part[1], ScriptOp);
            if (!n.part[2].empty()) {
                const QString e = operand(n.part[2], Keep);
                if (!e.isEmpty() && ((isAscii(out.back()) && isAscii(e[0])) || (out.back().isDigit() && e[0].isDigit()))) out += QLatin1Char(' ');
                out += e;
            }
            return out;
        }
        case Kind::Delim: {
            bool standard = false;
            if (n.text.size() == 1 && n.text2.size() == 1) standard = closerOf(n.text[0]) == n.text2[0];
            if (n.part.size() > 1 && n.text3 != QLatin1String("|") && !n.text3.isEmpty()) {
                ok = false;
                return QString();
            }
            QString body;
            for (size_t i = 0; i < n.part.size(); ++i) body += (i ? QStringLiteral("|") : QString()) + seq(n.part[i], true);
            if (standard) return n.text + body + n.text2;
            return QString(kOpenFence) + (n.text.isEmpty() ? QStringLiteral(" ") : n.text) + body + QString(kCloseFence) + (n.text2.isEmpty() ? QStringLiteral(" ") : n.text2);
        }
        case Kind::Accent: {
            const Node *only = n.part[0].size() == 1 ? &n.part[0][0] : nullptr;
            const QList<uint> mark = n.text.toUcs4();
            if (only && only->kind == Kind::Run && only->style != 'b' && only->style != 'n' && only->text.toUcs4().size() == 1 && mark.size() == 1 && isCombining(mark[0]))
                return runText(*only, inDelim, nullptr, nullptr) + n.text;
            const QString name = n.text.size() == 1 ? accentName(n.text[0].unicode()) : QString();
            if (name.isEmpty()) {
                ok = false;
                return QString();
            }
            return QLatin1Char('\\') + name + QLatin1Char('(') + seq(n.part[0], true) + QLatin1Char(')');
        }
        case Kind::Bar: return QLatin1String(n.hasA ? "\\overline(" : "\\underline(") + seq(n.part[0], true) + QLatin1Char(')');
        case Kind::Matrix: {
            QString out = QStringLiteral("\\matrix(");
            const int cols = std::max(1, n.cols);
            for (size_t i = 0; i < n.part.size(); ++i) {
                if (i) out += int(i) % cols == 0 ? QLatin1Char('@') : QLatin1Char('&');
                out += seq(n.part[i], false);
            }
            return out + QLatin1Char(')');
        }
        case Kind::Func: {
            if (!functionName(n.part[0])) {
                ok = false;
                return QString();
            }
            QString out = seq(n.part[0], false);
            if (!n.part[1].empty()) {
                const QString e = operand(n.part[1], Keep);
                if (!e.isEmpty() && isAscii(out.back()) && isAscii(e[0])) out += QLatin1Char(' ');
                out += e;
            }
            return out;
        }
        case Kind::LimLow:
            if (n.part.size() < 2 || n.part[0].size() != 1 || n.part[0][0].kind != Kind::Run || !functionName(n.part[0]) || !isLimitFunction(n.part[0][0].text)) {
                ok = false;
                return QString();
            }
            return seq(n.part[0], false) + QLatin1Char('_') + operand(n.part[1], ScriptOp);
        default: break;
        }
        ok = false;
        return QString();
    }

    // Whether the items are a function's name (alone, or with a limit under it, or with scripts), as the reader makes them.
    static bool functionName(const Seq &s)
    {
        if (s.size() != 1) return false;
        const Node &n = s[0];
        if (n.kind == Kind::Run) return n.style == 'p' && isFunctionName(n.text);
        if (n.kind == Kind::LimLow || n.kind == Kind::Script) return n.part.size() > 1 && functionName(n.part[0]);
        return false;
    }
};

} // namespace

QByteArray equationFromUnicodeMath(const QString &text, bool *ok)
{
    if (ok) *ok = false;
    const QString t = trimmedText(text);
    if (t.isEmpty() || t.size() > kMostEquationInput) return {};
    Reader reader(t);
    const Seq items = reader.all();
    if (reader.failed() || items.empty()) return {};
    const QByteArray markup = write(items);
    if (ok) *ok = !markup.isEmpty();
    return markup;
}

QString equationToUnicodeMath(const QByteArray &markup)
{
    Seq items;
    if (!read(markup, &items)) return QString();
    Printer p;
    const QString out = p.all(items);
    return p.ok && out.size() <= kMostEquationInput ? out : QString();
}

} // namespace jp::office::wml
