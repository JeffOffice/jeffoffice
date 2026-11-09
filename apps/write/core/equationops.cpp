#include "core/equationops.h"

#include "core/editops.h"
#include "core/writeprops.h"
#include "text/textprops.h"
#include "wml/wml.h"
#include "wml/wmlmath.h"
#include "wml/xmltree.h"

#include <QCoreApplication>
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

// The accents a name puts over its operand, and the combining character each is written as.
const QVector<std::pair<const char *, ushort>> &accents()
{
    static const QVector<std::pair<const char *, ushort>> list = {{"hat", 0x302}, {"check", 0x30C}, {"tilde", 0x303}, {"acute", 0x301}, {"grave", 0x300}, {"breve", 0x306},
                                                                  {"dot", 0x307}, {"ddot", 0x308}, {"bar", 0x305}, {"vec", 0x20D7}};
    return list;
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
            if (m_leftDepth > 0 && atRight()) break;
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

    // At \right (and not a longer name that begins so, as \rightarrow).
    bool atRight() const { return m_text.mid(m_at, 6) == QLatin1String("\\right") && (m_at + 6 >= m_text.size() || !m_text[m_at + 6].isLetter()); }

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

    // The operand of an accent, a bar, or a root-like command: a group in braces, or one item.
    Item operandOf(int depth)
    {
        skipBlanks();
        return peek() == QLatin1Char('{') ? group(QLatin1Char('}'), false, depth) : (more() ? primary(depth) : Item{});
    }

    // A matrix: rows of cells in braces, the cells divided by & and the rows by a double backslash.
    Item matrix(const QChar &open, const QChar &close, int depth)
    {
        skipBlanks();
        if (peek() != QLatin1Char('{')) {
            fail();
            return Item{};
        }
        ++m_at;
        QVector<QVector<QByteArray>> rows(1);
        QVector<Item> cell;
        auto endCell = [&] {
            rows.last().push_back(join(cell));
            cell.clear();
        };
        while (more() && !m_failed && peek() != QLatin1Char('}')) {
            if (++m_pieces > 4000) {
                fail();
                break;
            }
            if (peek() == QLatin1Char('&')) {
                ++m_at;
                endCell();
                skipBlanks();
            } else if (m_text.mid(m_at, 2) == QLatin1String("\\\\")) {
                m_at += 2;
                endCell();
                rows.push_back({});
                skipBlanks();
            } else {
                // A piece of the cell: one run of items up to the next divider.
                const qsizetype before = m_at;
                QVector<Item> part = sequenceUntilDivider(depth + 1);
                for (const Item &i : part) cell.push_back(i);
                if (m_at == before) fail();
            }
        }
        if (peek() == QLatin1Char('}')) ++m_at;
        endCell();
        int columns = 0;
        for (const auto &r : rows) columns = std::max(columns, int(r.size()));
        if (rows.size() > 64 || columns > 64 || columns == 0) {
            fail();
            return Item{};
        }
        QByteArray x = "<m:m><m:mPr><m:mcs><m:mc><m:mcPr><m:count m:val=\"" + QByteArray::number(columns) + "\"/><m:mcJc m:val=\"center\"/></m:mcPr></m:mc></m:mcs></m:mPr>";
        for (auto &r : rows) {
            while (r.size() < columns) r.push_back({});
            x += "<m:mr>";
            for (const QByteArray &c : r) x += "<m:e>" + (c.isEmpty() ? QByteArray("<m:r><m:t></m:t></m:r>") : c) + "</m:e>";
            x += "</m:mr>";
        }
        x += "</m:m>";
        if (open.isNull()) return Item{x, QByteArray(), true};
        const QByteArray d = "<m:d><m:dPr><m:begChr m:val=\"" + escaped(QString(open)) + "\"/><m:endChr m:val=\"" + escaped(QString(close)) + "\"/></m:dPr><m:e>" + x + "</m:e></m:d>";
        return Item{d, QByteArray(), true};
    }

    // Items up to a & or a double backslash or the closing brace of a matrix (left for the caller).
    QVector<Item> sequenceUntilDivider(int depth)
    {
        QVector<Item> items;
        const qsizetype start = m_at;
        // (The items are read one at a time by the usual reader on the text up to the divider, so that scripts and fractions work in a cell.)
        qsizetype end = m_at;
        int braces = 0;
        while (end < m_text.size()) {
            const QChar ch = m_text[end];
            if (ch == QLatin1Char('{')) ++braces;
            else if (ch == QLatin1Char('}')) {
                if (braces == 0) break;
                --braces;
            } else if (braces == 0 && ch == QLatin1Char('&')) break;
            else if (braces == 0 && m_text.mid(end, 2) == QLatin1String("\\\\")) break;
            else if (ch == QLatin1Char('\\') && end + 1 < m_text.size()) ++end;
            ++end;
        }
        const QString saved = m_text;
        const qsizetype savedAt = end;
        m_text = saved.left(end);
        m_at = start;
        items = sequence(QChar(), depth);
        m_text = saved;
        m_at = savedAt;
        // (Blanks at the cell's ends are not part of it.)
        while (!items.isEmpty() && items.last().xml == run(QStringLiteral(" "))) items.removeLast();
        while (!items.isEmpty() && items.first().xml == run(QStringLiteral(" "))) items.removeFirst();
        return items;
    }

    // \left( ... \right): brackets that grow with what is between them.
    Item growing(int depth)
    {
        auto delimiter = [&]() -> QString {
            skipBlanks();
            if (!more()) return QString();
            if (peek() == QLatin1Char('.')) {
                ++m_at;
                return QString();
            }
            if (peek() == QLatin1Char('\\') && m_at + 1 < m_text.size()) {
                ++m_at;
                return QString(m_text[m_at++]);
            }
            return QString(m_text[m_at++]);
        };
        const QString open = delimiter();
        ++m_leftDepth;
        const QVector<Item> inner = sequence(QChar(), depth + 1);
        --m_leftDepth;
        QString close;
        if (atRight()) {
            m_at += 6;
            close = delimiter();
        }
        QByteArray body = join(inner);
        if (body.isEmpty()) body = "<m:r><m:t></m:t></m:r>";
        return Item{"<m:d><m:dPr><m:begChr m:val=\"" + escaped(open) + "\"/><m:endChr m:val=\"" + escaped(close) + "\"/></m:dPr><m:e>" + body + "</m:e></m:d>", QByteArray(), true};
    }

    Item accent(const QString &chr, int depth)
    {
        const Item operand = operandOf(depth);
        const QByteArray e = operand.bare.isEmpty() ? operand.xml : operand.bare;
        return Item{"<m:acc><m:accPr><m:chr m:val=\"" + escaped(chr) + "\"/></m:accPr><m:e>" + e + "</m:e></m:acc>", QByteArray(), true};
    }

    Item bar(bool over, int depth)
    {
        const Item operand = operandOf(depth);
        const QByteArray e = operand.bare.isEmpty() ? operand.xml : operand.bare;
        return Item{"<m:bar><m:barPr><m:pos m:val=\"" + QByteArray(over ? "top" : "bot") + "\"/></m:barPr><m:e>" + e + "</m:e></m:bar>", QByteArray(), true};
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
            if (name == QLatin1String("left")) return growing(depth);
            if (name == QLatin1String("overline")) return bar(true, depth);
            if (name == QLatin1String("underline")) return bar(false, depth);
            if (name == QLatin1String("matrix")) return matrix(QChar(), QChar(), depth);
            if (name == QLatin1String("pmatrix")) return matrix(QLatin1Char('('), QLatin1Char(')'), depth);
            if (name == QLatin1String("bmatrix")) return matrix(QLatin1Char('['), QLatin1Char(']'), depth);
            if (name == QLatin1String("vmatrix")) return matrix(QLatin1Char('|'), QLatin1Char('|'), depth);
            for (const auto &acc : accents())
                if (name == QLatin1String(acc.first)) return accent(QString(QChar(acc.second)), depth);
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
    int m_leftDepth = 0;
};

} // namespace

QString equationSymbol(const QString &name)
{
    static const QHash<QString, char32_t> symbols = {
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

const QVector<EquationDesign> &equationGallery()
{
    static const QVector<EquationDesign> designs = {
        {QCoreApplication::translate("jp::write::Equations", "Quadratic Formula"), QStringLiteral("x=(-b\\pm\\sqrt(b^2-4ac))/(2a)")},
        {QCoreApplication::translate("jp::write::Equations", "Pythagorean Theorem"), QStringLiteral("a^2+b^2=c^2")},
        {QCoreApplication::translate("jp::write::Equations", "Area of a Circle"), QStringLiteral("A=\\pi r^2")},
        {QCoreApplication::translate("jp::write::Equations", "Euler's Identity"), QStringLiteral("e^(i\\pi)+1=0")},
        {QCoreApplication::translate("jp::write::Equations", "Geometric Series"), QStringLiteral("\\sum_(k=0)^n x^k=(1-x^(n+1))/(1-x)")},
        {QCoreApplication::translate("jp::write::Equations", "Taylor Expansion"), QStringLiteral("e^x=1+x+x^2/2+x^3/6+\\cdots")},
        {QCoreApplication::translate("jp::write::Equations", "Normal Distribution"), QStringLiteral("f(x)=1/(\\sigma\\sqrt(2\\pi)) e^(-(x-\\mu)^2/(2\\sigma^2))")},
        {QCoreApplication::translate("jp::write::Equations", "Definition of a Derivative"), QStringLiteral("f'(x)=lim_(h\\to 0) (f(x+h)-f(x))/h")},
        {QCoreApplication::translate("jp::write::Equations", "Integral of a Power"), QStringLiteral("\\int_0^1 x^n dx=1/(n+1)")},
        {QCoreApplication::translate("jp::write::Equations", "Identity Matrix"), QStringLiteral("I=\\pmatrix{1 & 0 \\\\ 0 & 1}")},
        {QCoreApplication::translate("jp::write::Equations", "Vector Length"), QStringLiteral("|\\vec{v}|=\\sqrt(x^2+y^2)")},
        {QCoreApplication::translate("jp::write::Equations", "Mean"), QStringLiteral("\\bar{x}=(\\sum_(i=1)^n x_i)/n")},
    };
    return designs;
}

namespace {

// The line for one node of the markup: what the reader makes the same node of. `ok` goes false for anything it can't say.
class Lineup {
public:
    explicit Lineup(const XmlTree &t) : m_t(t) {}
    bool ok() const { return m_ok; }
    QString of(int node, int depth)
    {
        QString out;
        for (int c = m_t.firstChild(node); c >= 0; c = m_t.next(c)) out += one(c, depth + 1);
        return out;
    }

private:
    QString braces(const QString &s) const { return QLatin1Char('{') + s + QLatin1Char('}'); }
    static QString plain(const QString &text)
    {
        QString out;
        for (const QChar ch : text) {
            if (QString::fromLatin1("\\(){}^_/[]&").contains(ch)) out += QLatin1Char('\\');
            out += ch;
        }
        return out;
    }
    int kid(int node, const char *name) const
    {
        for (int c = m_t.firstChild(node); c >= 0; c = m_t.next(c))
            if (m_t.localName(c) == name) return c;
        return -1;
    }
    QString valueOf(int node, const char *name, const QString &fallback) const
    {
        const int c = node >= 0 ? kid(node, name) : -1;
        return c >= 0 && m_t.has(c, "val", m_t.ns(c)) ? m_t.attr(c, "val", m_t.ns(c)) : fallback;
    }
    QString text(int node) const
    {
        QString out;
        for (int c = m_t.firstChild(node); c >= 0; c = m_t.next(c))
            if (m_t.localName(c) == "t") out += m_t.text(c);
        return out;
    }
    // A bracket character for \left or \right.
    static QString delimiterText(const QString &chr)
    {
        if (chr.isEmpty()) return QStringLiteral(".");
        return chr == QLatin1String(".") || chr == QLatin1String("\\") ? QLatin1Char('\\') + chr : chr;
    }

    QString one(int node, int depth)
    {
        if (depth > kMostEquationDepth + 4) {
            m_ok = false;
            return QString();
        }
        const QByteArray name = m_t.localName(node);
        if (name == "r") {
            const QString t = text(node);
            const int pr = kid(node, "rPr");
            const bool styled = pr >= 0;
            if (styled) {
                // (Only an upright run of a function's name is one the reader makes.)
                if (valueOf(pr, "sty", QString()) != QLatin1String("p") || t.isEmpty()) {
                    m_ok = false;
                    return QString();
                }
                for (const QChar ch : t)
                    if (!ch.isLetter()) m_ok = false;
                return t;
            }
            return plain(t);
        }
        if (name == "f") return braces(of(kid(node, "num"), depth)) + QLatin1Char('/') + braces(of(kid(node, "den"), depth));
        if (name == "sSup") return braces(of(kid(node, "e"), depth)) + QLatin1String("^") + braces(of(kid(node, "sup"), depth));
        if (name == "sSub") return braces(of(kid(node, "e"), depth)) + QLatin1String("_") + braces(of(kid(node, "sub"), depth));
        if (name == "sSubSup") return braces(of(kid(node, "e"), depth)) + QLatin1String("_") + braces(of(kid(node, "sub"), depth)) + QLatin1String("^") + braces(of(kid(node, "sup"), depth));
        if (name == "rad") {
            const int pr = kid(node, "radPr");
            const bool hidden = pr >= 0 && valueOf(pr, "degHide", QString()) == QLatin1String("1");
            const QString degree = hidden ? QString() : of(kid(node, "deg"), depth);
            return QLatin1String("\\sqrt") + (degree.isEmpty() ? QString() : QLatin1Char('[') + degree + QLatin1Char(']')) + braces(of(kid(node, "e"), depth));
        }
        if (name == "nary") {
            const int pr = kid(node, "naryPr");
            const QString chr = valueOf(pr, "chr", QString());
            const char *command = chr == QString(QChar(0x2211)) ? "\\sum" : chr == QString(QChar(0x220F)) ? "\\prod" : chr == QString(QChar(0x222B)) ? "\\int" : chr == QString(QChar(0x222E)) ? "\\oint" : nullptr;
            if (!command) {
                m_ok = false;
                return QString();
            }
            const bool subHidden = valueOf(pr, "subHide", QString()) == QLatin1String("1"), supHidden = valueOf(pr, "supHide", QString()) == QLatin1String("1");
            QString out = QLatin1String(command);
            if (!subHidden) out += QLatin1String("_") + braces(of(kid(node, "sub"), depth));
            if (!supHidden) out += QLatin1String("^") + braces(of(kid(node, "sup"), depth));
            return out + braces(of(kid(node, "e"), depth));
        }
        if (name == "acc") {
            const int pr = kid(node, "accPr");
            const QString chr = valueOf(pr, "chr", QString());
            for (const auto &a : accents())
                if (chr == QString(QChar(a.second))) return QLatin1Char('\\') + QLatin1String(a.first) + braces(of(kid(node, "e"), depth));
            m_ok = false;
            return QString();
        }
        if (name == "bar") {
            const int pr = kid(node, "barPr");
            return QLatin1String(valueOf(pr, "pos", QStringLiteral("bot")) == QLatin1String("top") ? "\\overline" : "\\underline") + braces(of(kid(node, "e"), depth));
        }
        if (name == "m") {
            QString rows;
            for (int r = m_t.firstChild(node); r >= 0; r = m_t.next(r)) {
                if (m_t.localName(r) != "mr") continue;
                QString row;
                for (int e = m_t.firstChild(r); e >= 0; e = m_t.next(e))
                    if (m_t.localName(e) == "e") row += (row.isEmpty() ? QString() : QStringLiteral(" & ")) + of(e, depth);
                rows += (rows.isEmpty() ? QString() : QStringLiteral(" \\\\ ")) + row;
            }
            return QLatin1String("\\matrix") + braces(rows);
        }
        if (name == "d") {
            const int pr = kid(node, "dPr");
            const QString open = valueOf(pr, "begChr", QStringLiteral("(")), close = valueOf(pr, "endChr", QStringLiteral(")"));
            int es = 0, e = -1;
            for (int c = m_t.firstChild(node); c >= 0; c = m_t.next(c))
                if (m_t.localName(c) == "e") ++es, e = c;
            if (es != 1 || (pr >= 0 && (kid(pr, "sepChr") >= 0 || kid(pr, "grow") >= 0 || kid(pr, "shp") >= 0)) || open.size() > 1 || close.size() > 1) {
                m_ok = false;
                return QString();
            }
            // (A matrix alone in brackets is the one command that makes both.)
            const int only = m_t.firstChild(e);
            if (only >= 0 && m_t.next(only) < 0 && m_t.localName(only) == "m") {
                const QString matrix = one(only, depth + 1);
                const char *kind = open == QLatin1String("(") && close == QLatin1String(")") ? "\\pmatrix" : open == QLatin1String("[") && close == QLatin1String("]") ? "\\bmatrix"
                                   : open == QLatin1String("|") && close == QLatin1String("|") ? "\\vmatrix" : nullptr;
                if (kind) return QLatin1String(kind) + matrix.mid(7);
            }
            return QLatin1String("\\left") + delimiterText(open) + of(e, depth) + QLatin1String("\\right") + delimiterText(close);
        }
        m_ok = false;
        return QString();
    }

    const XmlTree &m_t;
    bool m_ok = true;
};

} // namespace

QString equationLinear(const QByteArray &markup)
{
    XmlTree t;
    if (!t.parse("<x>" + markup + "</x>", nullptr)) return QString();
    const int n = t.firstChild(t.root());
    if (n < 0 || t.localName(n) != "oMath") return QString();
    Lineup lineup(t);
    const QString out = lineup.of(n, 0);
    return lineup.ok() ? out : QString();
}

namespace {

// The equation the character just after `position` holds: its markup and its atom; false for any other character.
bool equationAtomAt(const QTextDocument *story, int position, Inline *atom)
{
    if (!story || position < 0 || position + 1 > story->characterCount() - 1) return false;
    for (QTextBlock::iterator it = story->findBlock(position).begin(); !it.atEnd(); ++it) {
        const QTextFragment f = it.fragment();
        if (position < f.position() || position >= f.position() + f.length()) continue;
        const QTextCharFormat cf = f.charFormat();
        if (!cf.hasProperty(prop::Atom)) return false;
        const QByteArray bytes = storedBytes(cf, prop::Atom);
        if (!bytes.contains("oMath")) return false;
        *atom = unpackAtom(bytes);
        return atom->tag == QLatin1String("oMath");
    }
    return false;
}

} // namespace

QString equationAt(const WriteDoc &doc, int position)
{
    Inline atom;
    if (!equationAtomAt(doc.story(), position, &atom)) return QString();
    return equationLinear(atom.raw);
}

bool replaceEquation(WriteDoc &doc, int position, const QString &linear)
{
    Inline atom;
    if (!equationAtomAt(doc.story(), position, &atom) || isLocked(doc.story()->findBlock(position))) return false;
    bool ok = false;
    const QByteArray markup = equationMarkup(linear, &ok);
    if (!ok || markup == atom.raw) return false;
    QTextCursor c(doc.story());
    c.setPosition(position);
    c.setPosition(position + 1, QTextCursor::KeepAnchor);
    return insertEquation(doc, c, linear);
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
