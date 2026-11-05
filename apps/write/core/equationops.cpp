#include "core/equationops.h"

#include "core/editops.h"
#include "core/writeprops.h"
#include "text/textprops.h"
#include "wml/wml.h"
#include "wml/wmlmath.h"
#include "wml/xmltree.h"

#include <QHash>
#include <QVector>

namespace jp::write {

using namespace office::wml;

namespace {

const char kMathNs[] = "xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\"";

QByteArray escaped(const QString &text) { return text.toHtmlEscaped().toUtf8(); }

QByteArray run(const QString &text)
{
    if (text.isEmpty()) return {};
    const bool spaces = text.startsWith(QLatin1Char(' ')) || text.endsWith(QLatin1Char(' ')) || text.contains(QLatin1String("  "));
    return QByteArray("<m:r><m:t") + (spaces ? " xml:space=\"preserve\"" : "") + ">" + escaped(text) + "</m:t></m:r>";
}

struct Item {
    QByteArray xml;     // as it stands alone
    QByteArray bare;    // as an operand of a fraction, script, or root: a group without its brackets
    bool operand = false;   // a letter, number, group, or structure that a fraction or script can take as its own
};

class Parser {
public:
    explicit Parser(const QString &text) : m_text(text) {}
    QByteArray all()
    {
        QByteArray out;
        for (const Item &i : sequence(QChar(), 0)) out += i.xml;
        return m_failed ? QByteArray() : out;
    }
    bool failed() const { return m_failed; }

private:
    QChar peek() const { return m_at < m_text.size() ? m_text[m_at] : QChar(); }
    bool more() const { return m_at < m_text.size(); }
    bool fail()
    {
        m_failed = true;
        m_at = m_text.size();
        return false;
    }

    // Items until the closing character (or the end), a closing character left for the caller.
    QVector<Item> sequence(QChar closer, int depth)
    {
        QVector<Item> items;
        if (depth > kMostEquationDepth) {
            fail();
            return items;
        }
        while (more() && !m_failed) {
            if (!closer.isNull() && peek() == closer) break;
            if (++m_pieces > 4000) {
                fail();
                break;
            }
            Item item = primary(depth);
            // Scripts.
            QByteArray sub, sup;
            bool hasSub = false, hasSup = false;
            while ((peek() == QLatin1Char('^') || peek() == QLatin1Char('_')) && !m_failed) {
                const bool up = peek() == QLatin1Char('^');
                ++m_at;
                skipBlanks();
                const Item operand = scriptOperand(depth);
                if (up && !hasSup) {
                    sup = operand.bare.isEmpty() ? operand.xml : operand.bare;
                    hasSup = true;
                } else if (!up && !hasSub) {
                    sub = operand.bare.isEmpty() ? operand.xml : operand.bare;
                    hasSub = true;
                }
            }
            if (hasSub || hasSup) {
                const QByteArray base = item.bare.isEmpty() ? item.xml : item.bare;
                QByteArray x;
                if (hasSub && hasSup) x = "<m:sSubSup><m:e>" + base + "</m:e><m:sub>" + sub + "</m:sub><m:sup>" + sup + "</m:sup></m:sSubSup>";
                else if (hasSup) x = "<m:sSup><m:e>" + base + "</m:e><m:sup>" + sup + "</m:sup></m:sSup>";
                else x = "<m:sSub><m:e>" + base + "</m:e><m:sub>" + sub + "</m:sub></m:sSub>";
                item = Item{x, QByteArray(), true};
            }
            // A fraction: the item before the slash over the item after it.
            if (peek() == QLatin1Char('/') && item.operand) {
                ++m_at;
                skipBlanks();
                Item den = primary(depth);
                if (peek() == QLatin1Char('^') || peek() == QLatin1Char('_')) {
                    // (A script belongs to the denominator's last item.)
                    QByteArray dsub, dsup;
                    bool hasDsub = false, hasDsup = false;
                    while ((peek() == QLatin1Char('^') || peek() == QLatin1Char('_')) && !m_failed) {
                        const bool up = peek() == QLatin1Char('^');
                        ++m_at;
                        const Item operand = scriptOperand(depth);
                        const QByteArray body = operand.bare.isEmpty() ? operand.xml : operand.bare;
                        if (up && !hasDsup) dsup = body, hasDsup = true;
                        else if (!up && !hasDsub) dsub = body, hasDsub = true;
                    }
                    const QByteArray base = den.bare.isEmpty() ? den.xml : den.bare;
                    if (hasDsub && hasDsup) den.xml = "<m:sSubSup><m:e>" + base + "</m:e><m:sub>" + dsub + "</m:sub><m:sup>" + dsup + "</m:sup></m:sSubSup>";
                    else if (hasDsup) den.xml = "<m:sSup><m:e>" + base + "</m:e><m:sup>" + dsup + "</m:sup></m:sSup>";
                    else if (hasDsub) den.xml = "<m:sSub><m:e>" + base + "</m:e><m:sub>" + dsub + "</m:sub></m:sSub>";
                    den.bare.clear();
                }
                const QByteArray num = item.bare.isEmpty() ? item.xml : item.bare, denominator = den.bare.isEmpty() ? den.xml : den.bare;
                item = Item{"<m:f><m:num>" + num + "</m:num><m:den>" + denominator + "</m:den></m:f>", QByteArray(), true};
            }
            items.push_back(item);
        }
        return items;
    }

    void skipBlanks()
    {
        while (peek() == QLatin1Char(' ')) ++m_at;
    }

    // What a script or a limit is: a group, a command's result, or one number or letter.
    Item scriptOperand(int depth)
    {
        if (!more()) return Item{};
        return primary(depth);
    }

    QByteArray join(const QVector<Item> &items) const
    {
        QByteArray out;
        for (const Item &i : items) out += i.xml;
        return out;
    }

    // One group: its brackets "(...)" or "{...}", read to the closing one.
    Item group(QChar close, bool keepBrackets, int depth)
    {
        const QChar open = m_text[m_at];
        ++m_at;
        const QVector<Item> inner = sequence(close, depth + 1);
        if (peek() == close) ++m_at;
        const QByteArray body = join(inner);
        Item item;
        item.bare = body.isEmpty() ? QByteArray("<m:r><m:t></m:t></m:r>") : body;
        item.xml = keepBrackets ? run(QString(open)) + body + run(QString(close)) : body;
        item.operand = true;
        if (item.xml.isEmpty()) item.xml = QByteArray();
        return item;
    }

    QString commandName()
    {
        QString name;
        while (more() && peek().isLetter()) name += m_text[m_at++];
        return name;
    }

    Item radical(int depth)
    {
        QByteArray degree;
        skipBlanks();
        if (peek() == QLatin1Char('[')) {
            ++m_at;
            const QVector<Item> deg = sequence(QLatin1Char(']'), depth + 1);
            if (peek() == QLatin1Char(']')) ++m_at;
            degree = join(deg);
        }
        skipBlanks();
        Item radicand;
        if (peek() == QLatin1Char('(')) radicand = group(QLatin1Char(')'), false, depth);
        else if (peek() == QLatin1Char('{')) radicand = group(QLatin1Char('}'), false, depth);
        else radicand = more() ? primary(depth) : Item{};
        const QByteArray e = radicand.bare.isEmpty() ? radicand.xml : radicand.bare;
        const QByteArray props = degree.isEmpty() ? QByteArray("<m:radPr><m:degHide m:val=\"1\"/></m:radPr><m:deg/>") : QByteArray("<m:deg>") + degree + "</m:deg>";
        return Item{"<m:rad>" + props + "<m:e>" + e + "</m:e></m:rad>", QByteArray(), true};
    }

    Item nary(const QString &symbol, bool integral, int depth)
    {
        QByteArray sub, sup;
        bool hasSub = false, hasSup = false;
        skipBlanks();
        while ((peek() == QLatin1Char('^') || peek() == QLatin1Char('_')) && !m_failed) {
            const bool up = peek() == QLatin1Char('^');
            ++m_at;
            const Item operand = scriptOperand(depth);
            const QByteArray body = operand.bare.isEmpty() ? operand.xml : operand.bare;
            if (up && !hasSup) sup = body, hasSup = true;
            else if (!up && !hasSub) sub = body, hasSub = true;
        }
        skipBlanks();
        Item operand = more() && peek() != QLatin1Char(')') && peek() != QLatin1Char('}') ? primary(depth) : Item{};
        // (What it works on may carry scripts of its own.)
        QByteArray e = operand.bare.isEmpty() ? operand.xml : operand.bare;
        if ((peek() == QLatin1Char('^') || peek() == QLatin1Char('_')) && !m_failed) {
            QByteArray esub, esup;
            bool hasEsub = false, hasEsup = false;
            while ((peek() == QLatin1Char('^') || peek() == QLatin1Char('_')) && !m_failed) {
                const bool up = peek() == QLatin1Char('^');
                ++m_at;
                const Item o = scriptOperand(depth);
                const QByteArray body = o.bare.isEmpty() ? o.xml : o.bare;
                if (up && !hasEsup) esup = body, hasEsup = true;
                else if (!up && !hasEsub) esub = body, hasEsub = true;
            }
            if (hasEsub && hasEsup) e = "<m:sSubSup><m:e>" + e + "</m:e><m:sub>" + esub + "</m:sub><m:sup>" + esup + "</m:sup></m:sSubSup>";
            else if (hasEsup) e = "<m:sSup><m:e>" + e + "</m:e><m:sup>" + esup + "</m:sup></m:sSup>";
            else if (hasEsub) e = "<m:sSub><m:e>" + e + "</m:e><m:sub>" + esub + "</m:sub></m:sSub>";
        }
        QByteArray props = "<m:naryPr><m:chr m:val=\"" + escaped(symbol) + "\"/><m:limLoc m:val=\"" + (integral ? "subSup" : "undOvr") + "\"/>";
        if (!hasSub) props += "<m:subHide m:val=\"1\"/>";
        if (!hasSup) props += "<m:supHide m:val=\"1\"/>";
        props += "</m:naryPr>";
        return Item{"<m:nary>" + props + "<m:sub>" + sub + "</m:sub><m:sup>" + sup + "</m:sup><m:e>" + e + "</m:e></m:nary>", QByteArray(), true};
    }

    Item primary(int depth)
    {
        if (!more()) return Item{};
        const QChar ch = peek();
        if (ch == QLatin1Char('(')) return group(QLatin1Char(')'), true, depth);
        if (ch == QLatin1Char('{')) return group(QLatin1Char('}'), false, depth);
        if (ch == QLatin1Char('\\')) {
            ++m_at;
            const QString name = commandName();
            if (name.isEmpty()) {
                // (A backslash and a character that is no letter: that character, as it is.)
                if (!more()) return Item{};
                const QString one(m_text[m_at++]);
                return Item{run(one), QByteArray(), false};
            }
            if (name == QLatin1String("frac")) {
                skipBlanks();
                Item num = peek() == QLatin1Char('{') ? group(QLatin1Char('}'), false, depth) : primary(depth);
                skipBlanks();
                Item den = peek() == QLatin1Char('{') ? group(QLatin1Char('}'), false, depth) : primary(depth);
                const QByteArray n = num.bare.isEmpty() ? num.xml : num.bare, d = den.bare.isEmpty() ? den.xml : den.bare;
                return Item{"<m:f><m:num>" + n + "</m:num><m:den>" + d + "</m:den></m:f>", QByteArray(), true};
            }
            if (name == QLatin1String("sqrt")) return radical(depth);
            if (name == QLatin1String("sum")) return nary(QString(QChar(0x2211)), false, depth);
            if (name == QLatin1String("prod")) return nary(QString(QChar(0x220F)), false, depth);
            if (name == QLatin1String("int")) return nary(QString(QChar(0x222B)), true, depth);
            if (name == QLatin1String("oint")) return nary(QString(QChar(0x222E)), true, depth);
            const QString symbol = equationSymbol(name);
            if (!symbol.isEmpty()) return Item{run(symbol), QByteArray(), true};
            return Item{run(QLatin1Char('\\') + name), QByteArray(), true};   // (a name that is none stays as typed)
        }
        if (ch.isDigit()) {
            QString digits;
            while (more() && (peek().isDigit() || peek() == QLatin1Char('.')) && !(peek() == QLatin1Char('.') && (m_at + 1 >= m_text.size() || !m_text[m_at + 1].isDigit()))) digits += m_text[m_at++];
            return Item{run(digits), QByteArray(), true};
        }
        if (ch.isLetter()) {
            // A word that names a root or an operator with limits.
            QString word;
            qsizetype end = m_at;
            while (end < m_text.size() && m_text[end].isLetter()) word += m_text[end++];
            if (word == QLatin1String("sqrt") && end < m_text.size() && (m_text[end] == QLatin1Char('(') || m_text[end] == QLatin1Char('['))) {
                m_at = end;
                return radical(depth);
            }
            static const QHash<QString, QPair<QString, bool>> operators = {{QStringLiteral("sum"), {QString(QChar(0x2211)), false}}, {QStringLiteral("prod"), {QString(QChar(0x220F)), false}},
                                                                        {QStringLiteral("int"), {QString(QChar(0x222B)), true}}};
            if (operators.contains(word) && end < m_text.size() && (m_text[end] == QLatin1Char('_') || m_text[end] == QLatin1Char('^'))) {
                m_at = end;
                return nary(operators[word].first, operators[word].second, depth);
            }
            // (Functions keep their names whole: sin, cos, log.)
            static const QStringList functions = {QStringLiteral("sin"), QStringLiteral("cos"), QStringLiteral("tan"), QStringLiteral("log"), QStringLiteral("ln"), QStringLiteral("exp"), QStringLiteral("lim"),
                                                  QStringLiteral("min"), QStringLiteral("max")};
            if (functions.contains(word)) {
                m_at = end;
                return Item{"<m:r><m:rPr><m:sty m:val=\"p\"/></m:rPr><m:t>" + escaped(word) + "</m:t></m:r>", QByteArray(), true};
            }
            ++m_at;
            return Item{run(QString(ch)), QByteArray(), true};
        }
        // An operator, a space, or another sign: as it is, and nothing a fraction takes.
        ++m_at;
        QString text(ch);
        return Item{run(text), QByteArray(), false};
    }

    QString m_text;
    qsizetype m_at = 0;
    int m_pieces = 0;
    bool m_failed = false;
};

} // namespace

QString equationSymbol(const QString &name)
{
    static const QHash<QString, uint> symbols = {
        {QStringLiteral("alpha"), 0x3B1}, {QStringLiteral("beta"), 0x3B2}, {QStringLiteral("gamma"), 0x3B3}, {QStringLiteral("delta"), 0x3B4}, {QStringLiteral("epsilon"), 0x3B5},
        {QStringLiteral("zeta"), 0x3B6}, {QStringLiteral("eta"), 0x3B7}, {QStringLiteral("theta"), 0x3B8}, {QStringLiteral("iota"), 0x3B9}, {QStringLiteral("kappa"), 0x3BA},
        {QStringLiteral("lambda"), 0x3BB}, {QStringLiteral("mu"), 0x3BC}, {QStringLiteral("nu"), 0x3BD}, {QStringLiteral("xi"), 0x3BE}, {QStringLiteral("pi"), 0x3C0},
        {QStringLiteral("rho"), 0x3C1}, {QStringLiteral("sigma"), 0x3C3}, {QStringLiteral("tau"), 0x3C4}, {QStringLiteral("upsilon"), 0x3C5}, {QStringLiteral("phi"), 0x3C6},
        {QStringLiteral("chi"), 0x3C7}, {QStringLiteral("psi"), 0x3C8}, {QStringLiteral("omega"), 0x3C9}, {QStringLiteral("Gamma"), 0x393}, {QStringLiteral("Delta"), 0x394},
        {QStringLiteral("Theta"), 0x398}, {QStringLiteral("Lambda"), 0x39B}, {QStringLiteral("Xi"), 0x39E}, {QStringLiteral("Pi"), 0x3A0}, {QStringLiteral("Sigma"), 0x3A3},
        {QStringLiteral("Phi"), 0x3A6}, {QStringLiteral("Psi"), 0x3A8}, {QStringLiteral("Omega"), 0x3A9}, {QStringLiteral("infty"), 0x221E}, {QStringLiteral("pm"), 0xB1},
        {QStringLiteral("mp"), 0x2213}, {QStringLiteral("times"), 0xD7}, {QStringLiteral("div"), 0xF7}, {QStringLiteral("cdot"), 0x22C5}, {QStringLiteral("leq"), 0x2264},
        {QStringLiteral("le"), 0x2264}, {QStringLiteral("geq"), 0x2265}, {QStringLiteral("ge"), 0x2265}, {QStringLiteral("neq"), 0x2260}, {QStringLiteral("ne"), 0x2260},
        {QStringLiteral("approx"), 0x2248}, {QStringLiteral("equiv"), 0x2261}, {QStringLiteral("partial"), 0x2202}, {QStringLiteral("nabla"), 0x2207}, {QStringLiteral("to"), 0x2192},
        {QStringLiteral("rightarrow"), 0x2192}, {QStringLiteral("leftarrow"), 0x2190}, {QStringLiteral("leftrightarrow"), 0x2194}, {QStringLiteral("in"), 0x2208},
        {QStringLiteral("notin"), 0x2209}, {QStringLiteral("subset"), 0x2282}, {QStringLiteral("cup"), 0x222A}, {QStringLiteral("cap"), 0x2229}, {QStringLiteral("forall"), 0x2200},
        {QStringLiteral("exists"), 0x2203}, {QStringLiteral("ldots"), 0x2026}, {QStringLiteral("cdots"), 0x22EF}, {QStringLiteral("degree"), 0xB0}, {QStringLiteral("angle"), 0x2220},
        {QStringLiteral("sim"), 0x223C}, {QStringLiteral("propto"), 0x221D}};
    const auto found = symbols.constFind(name);
    return found == symbols.constEnd() ? QString() : QString::fromUcs4(&*found, 1);
}

QByteArray equationMarkup(const QString &linear, bool *ok)
{
    if (ok) *ok = false;
    const QString text = linear.trimmed();
    if (text.isEmpty() || text.size() > kMostEquationInput) return {};
    Parser parser(text);
    const QByteArray body = parser.all();
    if (parser.failed() || body.isEmpty()) return {};
    if (ok) *ok = true;
    return QByteArray("<m:oMath ") + kMathNs + ">" + body + "</m:oMath>";
}

QString equationLine(const QByteArray &markup)
{
    XmlTree t;
    if (!t.parse("<x>" + markup + "</x>", nullptr)) return QString();
    const int n = t.firstChild(t.root());
    if (n < 0) return QString();
    return equationText(t, n, 4000).text;
}

bool insertEquation(WriteDoc &doc, QTextCursor &c, const QString &linear)
{
    bool ok = false;
    const QByteArray markup = equationMarkup(linear, &ok);
    if (!ok) return false;
    XmlTree t;
    if (!t.parse("<x>" + markup + "</x>", nullptr)) return false;
    const int n = t.firstChild(t.root());
    if (n < 0) return false;
    const Equation e = equationText(t, n, 4000);
    if (e.text.isEmpty()) return false;
    Inline atom;
    atom.kind = Inline::ParaAtom;
    atom.role = Inline::Single;
    atom.tag = QStringLiteral("oMath");
    atom.raw = markup;
    atom.display = e.text;
    atom.key = e.style;
    // The character is put in with the line the pages show as its field text; the letters' look is set with it, in the same step.
    QTextCursor outer = c;
    outer.beginEditBlock();
    Inline field = atom;
    field.key.clear();
    const int before = std::min(c.selectionStart(), c.position());
    const bool put = insertFieldAtom(doc, c, field);
    if (put) {
        QTextCursor look(c.document());
        look.setPosition(before);
        look.setPosition(c.position(), QTextCursor::KeepAnchor);
        QTextCharFormat f;
        if (atom.key == QLatin1String("d")) f.setProperty(tp::DisplayFamily, QStringLiteral("DejaVu Sans"));
        else if (!atom.key.isEmpty()) f.setProperty(tp::DisplayStyle, atom.key);
        // (The atom itself keeps the key, as a file's equation does.)
        look.mergeCharFormat(f);
    }
    outer.endEditBlock();
    return put;
}

} // namespace jp::write
