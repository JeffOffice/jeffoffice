#include "wml/wmlmathlatex.h"

#include "wml/wmlmathast.h"

namespace jp::office::wml {

using namespace mathtree;

namespace {

bool isAscii(QChar c) { return (c >= QLatin1Char('a') && c <= QLatin1Char('z')) || (c >= QLatin1Char('A') && c <= QLatin1Char('Z')); }

bool isOperandCp(uint c)
{
    return QChar::isLetter(c) || QChar::isDigit(c) || c == 0x221E || c == 0x2202 || c == 0x2207 || c == 0x210F || c == 0x2113 || c == 0x2205 || c == 0x2032 || c == 0xB0;
}

Seq baseSeq(Node n)
{
    if (n.kind == Kind::Group) return std::move(n.part[0]);
    if (n.kind == Kind::Run && n.text.isEmpty() && n.style == 0) return {};
    return Seq{std::move(n)};
}

enum Stop { SBrace = 1, SAmp = 2, SRow = 4, SEnd = 8, SRight = 16, SBracket = 32 };

class Reader {
public:
    explicit Reader(const QString &text) : m_t(text) {}
    Seq all()
    {
        Seq s = sequence(0, 0);
        if (more()) fail();   // (a closing brace with no opening one)
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
    // At a command with this name (and not a longer name that begins so).
    bool atCommand(const char *name) const
    {
        const QLatin1String n(name);
        return peek() == QLatin1Char('\\') && m_t.mid(m_i + 1, n.size()) == n && !(m_i + 1 + n.size() < m_t.size() && isAscii(m_t[m_i + 1 + n.size()]));
    }
    bool atStop(int stops) const
    {
        const QChar c = peek();
        if (c == QLatin1Char('}')) return stops & SBrace;
        if (c == QLatin1Char('&')) return stops & SAmp;
        if (c == QLatin1Char(']')) return stops & SBracket;
        if (c == QLatin1Char('\\')) {
            if (m_i + 1 < m_t.size() && m_t[m_i + 1] == QLatin1Char('\\')) return stops & SRow;
            if (stops & SEnd && atCommand("end")) return true;
            if (stops & SRight && (atCommand("right") || atCommand("middle"))) return true;
        }
        return false;
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
    static QString str(uint c) { return QString::fromUcs4(reinterpret_cast<const char32_t *>(&c), 1); }
    static void add(Seq &out, Node n)
    {
        if (n.kind == Kind::Group) {
            for (Node &c : n.part[0]) out.push_back(std::move(c));
        } else if (!(n.kind == Kind::Run && n.text.isEmpty() && n.style == 0)) {
            out.push_back(std::move(n));
        }
    }

    Seq sequence(int stops, int depth)
    {
        Seq out;
        if (depth > kMostEquationDepth) {
            fail();
            return out;
        }
        while (!m_failed) {
            blanks();
            if (!more() || atStop(stops)) break;
            if (peek() == QLatin1Char('}') && !(stops & SBrace)) break;   // (left for the caller to refuse)
            const qsizetype before = m_i;
            add(out, element(stops, depth));
            if (m_i == before) fail();
        }
        return out;
    }

    Node element(int stops, int depth)
    {
        Node a = atom(stops, depth);
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

    void scripts(Seq &sub, Seq &sup, bool &hasSub, bool &hasSup, int stops, int depth)
    {
        while (!m_failed) {
            blanks();
            const QChar c = peek();
            if (c != QLatin1Char('^') && c != QLatin1Char('_')) break;
            ++m_i;
            Seq operand = argument(stops, depth);
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

    // An argument: a group in braces, or one character or command (one digit, as a number is read digit by digit here).
    Seq argument(int stops, int depth)
    {
        if (depth > kMostEquationDepth) {
            fail();
            return {};
        }
        blanks();
        if (!more() || atStop(stops)) return {};
        if (peek() == QLatin1Char('{')) return braces(depth);
        if (peek().isDigit()) return Seq{run(QString(m_t[m_i++]), 0, true)};
        if (peek() == QLatin1Char('^') || peek() == QLatin1Char('_')) return {};
        Node a = atom(stops, depth + 1);
        return baseSeq(std::move(a));
    }

    Seq braces(int depth)
    {
        ++m_i;
        Seq inner = sequence(SBrace, depth + 1);
        if (peek() == QLatin1Char('}')) ++m_i;
        else fail();
        return inner;
    }

    // The text in braces, as it is: a backslash and a character is that character, and braces inside count.
    QString rawText()
    {
        blanks();
        if (peek() != QLatin1Char('{')) {
            fail();
            return QString();
        }
        ++m_i;
        QString s;
        int level = 0;
        while (more()) {
            const QChar c = m_t[m_i];
            if (c == QLatin1Char('\\') && m_i + 1 < m_t.size()) {
                s += m_t[m_i + 1];
                m_i += 2;
                continue;
            }
            if (c == QLatin1Char('{')) ++level;
            if (c == QLatin1Char('}') && level-- == 0) {
                ++m_i;
                return s;
            }
            if (c.unicode() >= 0x20) s += c;
            ++m_i;
        }
        fail();
        return QString();
    }

    Node atom(int stops, int depth)
    {
        if (depth > kMostEquationDepth) {
            fail();
            return Node();
        }
        blanks();
        if (!more() || atStop(stops)) return run(QString());
        const QChar c = peek();
        if (c == QLatin1Char('{')) return structure(Kind::Group, {braces(depth)});
        if (c == QLatin1Char('}')) return run(QString());
        if (c == QLatin1Char('^') || c == QLatin1Char('_')) return run(QString());
        if (c == QLatin1Char('\\')) return command(stops, depth);
        if (c.isDigit()) {
            QString digits;
            while (more() && (peek().isDigit() || (peek() == QLatin1Char('.') && m_i + 1 < m_t.size() && m_t[m_i + 1].isDigit()))) digits += m_t[m_i++];
            return run(digits, 0, true);
        }
        if (c == QLatin1Char('~')) {
            ++m_i;
            return run(QStringLiteral(" "));
        }
        int len = 1;
        const uint cp = cpAt(m_i, &len);
        m_i += len;
        return run(str(cp), 0, isOperandCp(cp));
    }

    Node command(int stops, int depth)
    {
        ++m_i;
        if (!more()) return run(QString());
        if (!isAscii(peek())) {
            int len = 1;
            const uint c = cpAt(m_i, &len);
            m_i += len;
            switch (c) {
            case '\\': case ' ': case ',': case ';': case ':': return run(QStringLiteral(" "));
            case '!': return run(QString());
            case '|': return run(QString(QChar(0x2016)));
            default: return run(str(c), 0, isOperandCp(c));
            }
        }
        QString name;
        while (more() && isAscii(peek())) name += m_t[m_i++];
        if (name == QLatin1String("frac") || name == QLatin1String("dfrac") || name == QLatin1String("tfrac") || name == QLatin1String("cfrac")) {
            Seq num = argument(stops, depth);
            Seq den = argument(stops, depth);
            return structure(Kind::Frac, {std::move(num), std::move(den)});
        }
        if (name == QLatin1String("sqrt")) {
            blanks();
            Seq degree;
            bool hasDegree = false;
            if (peek() == QLatin1Char('[')) {
                ++m_i;
                degree = sequence(SBracket, depth + 1);
                if (peek() == QLatin1Char(']')) ++m_i;
                else fail();
                hasDegree = true;
            }
            Node r = structure(Kind::Rad, {std::move(degree), argument(stops, depth)});
            r.hasA = hasDegree;
            return r;
        }
        if (name == QLatin1String("left")) return delimited(depth);
        if (name == QLatin1String("begin")) return environment(depth);
        if (name == QLatin1String("text") || name == QLatin1String("textrm") || name == QLatin1String("textnormal") || name == QLatin1String("mbox")) return run(rawText(), 'n', true);
        if (name == QLatin1String("mathrm") || name == QLatin1String("rm")) return run(rawText(), 'p', true);
        if (name == QLatin1String("mathbf") || name == QLatin1String("boldsymbol") || name == QLatin1String("bf")) return run(rawText(), 'b', true);
        if (name == QLatin1String("mathit")) return run(rawText(), 0, true);
        if (name == QLatin1String("operatorname")) return function(rawText(), stops, depth);
        if (name == QLatin1String("overline") || name == QLatin1String("underline")) {
            Node bar = structure(Kind::Bar, {argument(stops, depth)});
            bar.hasA = name == QLatin1String("overline");
            return bar;
        }
        if (name == QLatin1String("widehat") || name == QLatin1String("widetilde") || accentMark(name)) {
            Node acc = structure(Kind::Accent, {argument(stops, depth)});
            acc.text = QString(QChar(name == QLatin1String("widehat") ? ushort(0x302) : name == QLatin1String("widetilde") ? ushort(0x303) : accentMark(name)));
            return acc;
        }
        if (const NaryOp *op = naryByName(name)) return nary(QString(QChar(op->chr)), stops, depth);
        if (isFunctionName(name)) return function(name, stops, depth);
        if (name == QLatin1String("limits") || name == QLatin1String("nolimits")) return run(QString());
        if (name == QLatin1String("quad")) return run(QString(QChar(0x2003)));
        if (name == QLatin1String("qquad")) return run(QString(QChar(0x2003)).repeated(2));
        const QString sym = symbolByName(name);
        if (!sym.isEmpty()) return run(sym, 0, true);
        return run(QLatin1Char('\\') + name, 0, true);
    }

    // A delimiter after \left, \middle, or \right.
    QString delimiter()
    {
        blanks();
        if (!more()) {
            fail();
            return QString();
        }
        int len = 1;
        const uint c = cpAt(m_i, &len);
        m_i += len;
        if (c == '.') return QString();
        if (c != '\\') return str(c);
        if (!more()) return QString();
        if (!isAscii(peek())) {
            const uint d = cpAt(m_i, &len);
            m_i += len;
            return d == '|' ? QString(QChar(0x2016)) : str(d);
        }
        QString name;
        while (more() && isAscii(peek())) name += m_t[m_i++];
        const QString sym = symbolByName(name);
        return sym.isEmpty() ? QString() : sym;
    }

    Node delimited(int depth)
    {
        Node d;
        d.kind = Kind::Delim;
        d.operand = true;
        d.text3 = QStringLiteral("|");
        d.text = delimiter();
        while (!m_failed) {
            d.part.push_back(sequence(SRight, depth + 1));
            if (!atCommand("middle")) break;
            m_i += 7;
            d.text3 = delimiter();
        }
        if (atCommand("right")) {
            m_i += 6;
            d.text2 = delimiter();
        } else {
            fail();
        }
        return d;
    }

    Node environment(int depth)
    {
        const QString name = rawText();
        QString open, close;
        if (name == QLatin1String("matrix")) {
        } else if (name == QLatin1String("pmatrix")) {
            open = QStringLiteral("("), close = QStringLiteral(")");
        } else if (name == QLatin1String("bmatrix")) {
            open = QStringLiteral("["), close = QStringLiteral("]");
        } else if (name == QLatin1String("Bmatrix")) {
            open = QStringLiteral("{"), close = QStringLiteral("}");
        } else if (name == QLatin1String("vmatrix")) {
            open = QStringLiteral("|"), close = QStringLiteral("|");
        } else if (name == QLatin1String("Vmatrix")) {
            open = close = QString(QChar(0x2016));
        } else {
            fail();
            return Node();
        }
        std::vector<std::vector<Seq>> grid(1);
        int cells = 0;
        while (!m_failed) {
            grid.back().push_back(sequence(SAmp | SRow | SEnd, depth + 1));
            if (++cells > kMostMatrixCells || int(grid.back().size()) > kMostMatrixSide) {
                fail();
                break;
            }
            if (peek() == QLatin1Char('&')) {
                ++m_i;
            } else if (atStop(SRow) && peek() == QLatin1Char('\\')) {
                m_i += 2;
                if (int(grid.size()) >= kMostMatrixSide) {
                    fail();
                    break;
                }
                grid.emplace_back();
            } else {
                break;
            }
        }
        if (m_failed) return Node();
        if (!atCommand("end")) {
            fail();
            return Node();
        }
        m_i += 4;
        if (rawText() != name) {
            fail();
            return Node();
        }
        if (grid.size() > 1 && grid.back().size() == 1 && grid.back()[0].empty()) grid.pop_back();
        Node m;
        m.kind = Kind::Matrix;
        m.operand = true;
        for (const auto &row : grid) m.cols = std::max(m.cols, int(row.size()));
        for (auto &row : grid) {
            row.resize(size_t(m.cols));
            for (Seq &cell : row) m.part.push_back(std::move(cell));
        }
        if (open.isEmpty()) return m;
        Node d;
        d.kind = Kind::Delim;
        d.operand = true;
        d.text = open;
        d.text2 = close;
        d.text3 = QStringLiteral("|");
        d.part.push_back(Seq{std::move(m)});
        return d;
    }

    Node nary(const QString &chr, int stops, int depth)
    {
        Node n = structure(Kind::Nary, {Seq(), Seq(), Seq()});
        n.text = chr;
        const bool beside = naryBeside(n);
        blanks();
        if (atCommand("limits")) {
            m_i += 7;
            n.limits = beside ? 1 : 0;
        } else if (atCommand("nolimits")) {
            m_i += 9;
            n.limits = beside ? 0 : 2;
        }
        scripts(n.part[0], n.part[1], n.hasA, n.hasB, stops, depth);
        const qsizetype save = m_i;
        blanks();
        if (more() && !atStop(stops) && !m_failed && peek() != QLatin1Char('}')) {
            Node t = element(stops, depth + 1);
            if (t.operand) n.part[2] = baseSeq(std::move(t));
            else m_i = save;
        }
        return n;
    }

    Node function(const QString &name, int stops, int depth)
    {
        Seq nm{run(name, 'p', true)};
        Seq sub, sup;
        bool hasSub = false, hasSup = false;
        blanks();
        if (atCommand("limits") || atCommand("nolimits")) m_i += atCommand("limits") ? 7 : 9;
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
        if (more() && !atStop(stops) && !m_failed && peek() != QLatin1Char('}')) {
            Node t = element(stops, depth + 1);
            if (t.operand) return structure(Kind::Func, {std::move(nm), baseSeq(std::move(t))});
            m_i = save;
        }
        return std::move(nm[0]);
    }
};

class Printer {
public:
    bool ok = true;
    QString all(const Seq &items) { return seq(items); }

private:
    int m_inDegree = 0;

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
    // Whether the node, alone, is read back as the one element that follows a base, a large operator, or a function name.
    static bool elementOk(const Node &n, bool asBase)
    {
        switch (n.kind) {
        case Kind::Run: {
            if (n.style == 'n' || n.style == 'b') return true;
            if (isNumber(n.text)) return true;
            if (n.style == 'p' && isFunctionName(n.text)) return asBase;
            const QList<uint> cps = n.text.toUcs4();
            return cps.size() == 1 && n.text != QLatin1String(" ") && n.text != QString(QChar(0x2003)) && n.text != QLatin1String("'");
        }
        case Kind::Script: case Kind::Nary: case Kind::Func: case Kind::LimLow: return !asBase;
        default: return true;
        }
    }
    QString element(const Seq &items, bool asBase)
    {
        if (items.size() == 1 && elementOk(items[0], asBase)) return item(items[0]);
        return QLatin1Char('{') + seq(items) + QLatin1Char('}');
    }
    QString braced(const Seq &items) { return QLatin1Char('{') + seq(items) + QLatin1Char('}'); }

    static void append(QString &out, const QString &s)
    {
        if (!out.isEmpty() && !s.isEmpty() && isAscii(out.back()) && isAscii(s[0])) {
            qsizetype a = out.size();
            while (a > 0 && isAscii(out[a - 1])) --a;
            if (a > 0 && out[a - 1] == QLatin1Char('\\')) out += QLatin1Char(' ');
        }
        out += s;
    }

    QString seq(const Seq &items)
    {
        QString out;
        for (size_t i = 0; i < items.size() && ok; ++i) {
            append(out, item(items[i]));
            if (out.size() > kMostEquationInput * 4) ok = false;
        }
        return out;
    }

    static QString escapeText(const QString &s)
    {
        QString out;
        for (const QChar c : s) {
            if (QString::fromLatin1("\\{}%$&#_^~").contains(c)) out += QLatin1Char('\\');
            out += c;
        }
        return out;
    }

    QString runText(const Node &n)
    {
        if (n.style == 'n') return QStringLiteral("\\text{") + escapeText(n.text) + QLatin1Char('}');
        if (n.style == 'b') return QStringLiteral("\\mathbf{") + escapeText(n.text) + QLatin1Char('}');
        if (n.style == 'p') {
            if (isFunctionName(n.text)) return QLatin1Char('\\') + n.text;
            return QStringLiteral("\\mathrm{") + escapeText(n.text) + QLatin1Char('}');
        }
        QString out;
        const QList<uint> cps = n.text.toUcs4();
        for (const uint c : cps) {
            const QString name = symbolName(c);
            QString piece;
            if (c == 0x2003) piece = QStringLiteral("\\quad");
            else if (c == ' ') piece = QStringLiteral("\\ ");
            else if (!name.isEmpty()) piece = QLatin1Char('\\') + name;
            else if (c < 0x80 && QString::fromLatin1("\\{}%$&#_^~").contains(QChar(ushort(c)))) piece = QLatin1Char('\\') + QString(QChar(ushort(c)));
            else if (c == ']' && m_inDegree) piece = QStringLiteral("\\]");
            else piece = QString::fromUcs4(reinterpret_cast<const char32_t *>(&c), 1);
            append(out, piece);
        }
        return out;
    }

    // A delimiter after \left, \middle, or \right.
    QString delimiter(const QString &chr)
    {
        if (chr.isEmpty()) return QStringLiteral(".");
        if (chr == QLatin1String(".") || (chr.size() == 1 && (isAscii(chr[0]) || chr[0].isDigit() || chr[0] == QLatin1Char('\\') || chr[0].unicode() <= 0x20))) {   // (no such delimiter)
            ok = false;
            return QString();
        }
        if (chr.size() == 1 && QString::fromLatin1("()[]|/<>").contains(chr[0])) return chr;
        const QList<uint> cps = chr.toUcs4();
        const QString name = cps.size() == 1 ? symbolName(cps[0]) : QString();
        if (chr == QLatin1String("{")) return QStringLiteral("\\{");
        if (chr == QLatin1String("}")) return QStringLiteral("\\}");
        if (chr == QString(QChar(0x2016))) return QStringLiteral("\\|");
        if (!name.isEmpty()) return QLatin1Char('\\') + name;
        return chr;
    }

    // Whether the items are a function's name, as the reader makes it.
    static bool functionName(const Seq &s, QString *name)
    {
        if (s.size() != 1) return false;
        const Node &n = s[0];
        if (n.kind == Kind::Run) {
            if (n.style != 'p' || n.text.isEmpty()) return false;
            if (name) *name = n.text;
            return true;
        }
        if (n.kind == Kind::LimLow) return n.part.size() > 1 && functionName(n.part[0], name) && name && isLimitFunction(*name);
        if (n.kind == Kind::Script) return n.part.size() > 2 && functionName(n.part[0], name);
        return false;
    }

    QString item(const Node &n)
    {
        switch (n.kind) {
        case Kind::Run: return runText(n);
        case Kind::Frac: return QStringLiteral("\\frac") + braced(n.part[0]) + braced(n.part[1]);
        case Kind::Script: {
            QString out = element(n.part[0], true);
            if (n.hasA) out += QStringLiteral("_") + braced(n.part[1]);
            if (n.hasB) out += QStringLiteral("^") + braced(n.part[2]);
            return out;
        }
        case Kind::Rad:
            if (n.hasA) {
                ++m_inDegree;
                const QString deg = seq(n.part[0]);
                --m_inDegree;
                return QStringLiteral("\\sqrt[") + deg + QStringLiteral("]") + braced(n.part[1]);
            }
            return QStringLiteral("\\sqrt") + braced(n.part[1]);
        case Kind::Nary: {
            const NaryOp *op = naryByChar(n.text);
            if (!op) {
                ok = false;
                return QString();
            }
            QString out = QLatin1Char('\\') + QLatin1String(op->latex);
            if (n.limits == 1 && op->beside) out += QStringLiteral("\\limits");
            else if (n.limits == 2 && !op->beside) out += QStringLiteral("\\nolimits");
            if (n.hasA) out += QStringLiteral("_") + braced(n.part[0]);
            if (n.hasB) out += QStringLiteral("^") + braced(n.part[1]);
            if (!n.part[2].empty()) append(out, element(n.part[2], false));
            return out;
        }
        case Kind::Delim: {
            // (A matrix alone in the usual brackets is the one environment that makes both.)
            if (n.part.size() == 1 && n.part[0].size() == 1 && n.part[0][0].kind == Kind::Matrix) {
                const char *env = n.text == QLatin1String("(") && n.text2 == QLatin1String(")") ? "pmatrix" : n.text == QLatin1String("[") && n.text2 == QLatin1String("]") ? "bmatrix"
                                  : n.text == QLatin1String("{") && n.text2 == QLatin1String("}") ? "Bmatrix" : n.text == QLatin1String("|") && n.text2 == QLatin1String("|") ? "vmatrix"
                                  : n.text == QString(QChar(0x2016)) && n.text2 == n.text ? "Vmatrix" : nullptr;
                if (env) return matrix(n.part[0][0], env);
            }
            QString out = QStringLiteral("\\left") + delimiter(n.text) + QLatin1Char(' ');
            for (size_t i = 0; i < n.part.size(); ++i) {
                if (i) out += QStringLiteral(" \\middle") + delimiter(n.text3.isEmpty() ? QStringLiteral("|") : n.text3) + QLatin1Char(' ');
                out += seq(n.part[i]);
            }
            return out + QStringLiteral(" \\right") + delimiter(n.text2);
        }
        case Kind::Accent: {
            const QString name = n.text.size() == 1 ? accentName(n.text[0].unicode()) : QString();
            if (name.isEmpty()) {
                ok = false;
                return QString();
            }
            return QLatin1Char('\\') + name + braced(n.part[0]);
        }
        case Kind::Bar: return QLatin1String(n.hasA ? "\\overline" : "\\underline") + braced(n.part[0]);
        case Kind::Matrix: return matrix(n, "matrix");
        case Kind::Func: {
            QString name;
            QString out = funcName(n.part[0]);
            if (!ok) return QString();
            if (!n.part[1].empty()) append(out, element(n.part[1], false));
            return out;
        }
        case Kind::LimLow: return funcName(Seq{n});
        default: break;
        }
        ok = false;
        return QString();
    }

    // \sin, \lim_{x}, \sin^{2}, or \operatorname{name}.
    QString funcName(const Seq &s)
    {
        QString name;
        if (!functionName(s, &name)) {
            ok = false;
            return QString();
        }
        const Node &n = s[0];
        const Node *base = &n;
        while (base->kind != Kind::Run) base = &base->part[0][0];
        QString out = isFunctionName(name) ? QLatin1Char('\\') + name : QStringLiteral("\\operatorname{") + escapeText(name) + QLatin1Char('}');
        if (n.kind == Kind::LimLow) out += QStringLiteral("_") + braced(n.part[1]);
        if (n.kind == Kind::Script) {
            if (!isFunctionName(name) && n.hasA && !n.hasB && isLimitFunction(name)) {
                ok = false;
                return QString();
            }
            if (n.hasA) out += QStringLiteral("_") + braced(n.part[1]);
            if (n.hasB) out += QStringLiteral("^") + braced(n.part[2]);
        }
        return out;
    }

    QString matrix(const Node &n, const char *env)
    {
        QString out = QStringLiteral("\\begin{") + QLatin1String(env) + QStringLiteral("}");
        const int cols = std::max(1, n.cols);
        for (size_t i = 0; i < n.part.size(); ++i) {
            if (i) out += int(i) % cols == 0 ? QStringLiteral("\\\\") : QStringLiteral("&");
            out += seq(n.part[i]);
        }
        return out + QStringLiteral("\\end{") + QLatin1String(env) + QLatin1Char('}');
    }
};

} // namespace

QByteArray equationFromLatex(const QString &text, bool *ok)
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

QString equationToLatex(const QByteArray &markup)
{
    Seq items;
    if (!read(markup, &items)) return QString();
    Printer p;
    const QString out = p.all(items);
    return p.ok && out.size() <= kMostEquationInput ? out : QString();
}

} // namespace jp::office::wml
