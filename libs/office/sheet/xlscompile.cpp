#include "sheet/xlscompile.h"

#include "sheet/xlsfunctions.h"

#include <QCoreApplication>
#include <QSet>

#include <cmath>
#include <cstring>
#include <vector>

namespace jp::office::sheet {

namespace {

constexpr int kMaxNodes = 4000;
constexpr int kMaxDepth = 80;
constexpr int kMaxArgs = 30;

// The classes of operand a token can be written in: a reference, a value, or an array.
constexpr int kClsRef = 0x20, kClsVal = 0x40, kClsArr = 0x60;

QHash<QString, int> &functionNumbers()
{
    static QHash<QString, int> table = [] {
        QHash<QString, int> t;
        for (int i = 0; i < 512; ++i) {
            const XlsFunction f = xlsFunction(i);
            if (f.name) t.insert(QString::fromLatin1(f.name), i);
        }
        return t;
    }();
    return table;
}

// Functions whose arguments are arrays: an expression inside them is worked out over the whole range, not for the one cell it stands in.
bool takesArrays(const QString &name)
{
    static const QSet<QString> set = {QStringLiteral("SUMPRODUCT"), QStringLiteral("MMULT"), QStringLiteral("TRANSPOSE"), QStringLiteral("MINVERSE"), QStringLiteral("MDETERM"),
                                      QStringLiteral("LINEST"), QStringLiteral("LOGEST"), QStringLiteral("TREND"), QStringLiteral("GROWTH"), QStringLiteral("FREQUENCY"),
                                      QStringLiteral("SUMX2MY2"), QStringLiteral("SUMX2PY2"), QStringLiteral("SUMXMY2"), QStringLiteral("CORREL"), QStringLiteral("COVAR"),
                                      QStringLiteral("PEARSON"), QStringLiteral("RSQ"), QStringLiteral("SLOPE"), QStringLiteral("INTERCEPT"), QStringLiteral("STEYX"),
                                      QStringLiteral("FORECAST"), QStringLiteral("LOOKUP"), QStringLiteral("MATCH"), QStringLiteral("INDEX")};
    return set.contains(name);
}

struct Err {
    const char *text;
    int code;
};
constexpr Err kErrors[] = {{"#NULL!", 0x00}, {"#DIV/0!", 0x07}, {"#VALUE!", 0x0F}, {"#REF!", 0x17}, {"#NAME?", 0x1D}, {"#NUM!", 0x24}, {"#N/A", 0x2A}};

struct Ref {
    bool is3d = false;
    int sheet1 = -1, sheet2 = -1;   // from 0
    quint32 row1 = 0, col1 = 0, row2 = 0, col2 = 0;
    bool row1Abs = false, col1Abs = false, row2Abs = false, col2Abs = false;
    bool area = false;
};

enum class Kind : quint8 { Num, Str, Bool, Error, Missing, Ref, Name, Func, Neg, Plus, Percent, Paren, Binary, Array };

struct Node {
    Kind kind = Kind::Num;
    double number = 0;
    QString text;                  // a string; a function's name; a name
    int code = 0;                  // a bool, an error's code, a binary operator's token
    int index = 0;                 // a name's place; a function's number
    Ref ref;
    std::vector<int> kids;
    std::vector<std::vector<int>> rows;   // an array constant: its cells by row (each a node of the kinds Num, Str, Bool, Error)
};

bool isIdentStart(QChar c) { return c.isLetter() || c == QLatin1Char('_') || c == QLatin1Char('\\'); }
bool isIdentChar(QChar c) { return c.isLetterOrNumber() || c == QLatin1Char('_') || c == QLatin1Char('.') || c == QLatin1Char('\\'); }

class Compiler {
public:
    Compiler(const QString &text, const XlsCompileContext &cx) : m_t(text), m_cx(cx) {}

    bool run(QByteArray *rgce, QByteArray *extra, QString *why)
    {
        if (m_t.size() > 8000) return fail(QCoreApplication::translate("Sheet", "The formula is longer than the old format allows."));
        const int root = parseTop();
        if (root < 0) {
            *why = m_why;
            return false;
        }
        m_out.clear();
        m_extra.clear();
        const int top = m_cx.isName ? kClsRef : m_cx.array ? kClsArr : kClsVal;
        if (!emit(root, top, 0)) {
            *why = m_why;
            return false;
        }
        if (m_out.size() > kXlsMaxFormulaBytes) {
            *why = QCoreApplication::translate("Sheet", "The formula is longer than the old format allows.");
            return false;
        }
        *rgce = m_out;
        *extra = m_extra;
        return true;
    }

private:
    const QString &m_t;
    const XlsCompileContext &m_cx;
    int m_pos = 0;
    int m_depth = 0;
    QString m_why;
    std::vector<Node> m_nodes;
    QByteArray m_out, m_extra;

    bool fail(const QString &why)
    {
        if (m_why.isEmpty()) m_why = why;
        return false;
    }
    int failNode(const QString &why)
    {
        fail(why);
        return -1;
    }
    int add(const Node &n)
    {
        if (int(m_nodes.size()) >= kMaxNodes) return failNode(QCoreApplication::translate("Sheet", "The formula is longer than the old format allows."));
        m_nodes.push_back(n);
        return int(m_nodes.size()) - 1;
    }

    QChar peek(int ahead = 0) const { return m_pos + ahead < m_t.size() ? m_t[m_pos + ahead] : QChar(); }
    void skipSpaces()
    {
        while (m_pos < m_t.size() && m_t[m_pos] == QLatin1Char(' ')) ++m_pos;
    }
    bool eat(QChar c)
    {
        skipSpaces();
        if (peek() == c) {
            ++m_pos;
            return true;
        }
        return false;
    }

    // ---- the grammar, from the loosest operators to the tightest

    int parseTop()
    {
        skipSpaces();
        int e = parseCompare();
        if (e < 0) return -1;
        skipSpaces();
        // (a defined name may stand for several areas, joined by commas)
        while (m_cx.isName && peek() == QLatin1Char(',')) {
            ++m_pos;
            const int next = parseCompare();
            if (next < 0) return -1;
            e = binary(0x10, e, next);
            skipSpaces();
        }
        if (m_pos != m_t.size()) return failNode(QCoreApplication::translate("Sheet", "The formula has something the old format can't write."));
        return e;
    }

    struct DepthGuard {
        Compiler *c;
        explicit DepthGuard(Compiler *cc) : c(cc) { ++c->m_depth; }
        ~DepthGuard() { --c->m_depth; }
    };

    int binary(int op, int a, int b)
    {
        Node n;
        n.kind = Kind::Binary;
        n.code = op;
        n.kids = {a, b};
        return add(n);
    }

    int parseCompare()
    {
        DepthGuard g(this);
        if (m_depth > kMaxDepth) return failNode(QCoreApplication::translate("Sheet", "The formula is nested more deeply than the old format allows."));
        int left = parseConcat();
        while (left >= 0) {
            skipSpaces();
            int op = 0;
            const QChar c = peek(), d = peek(1);
            int len = 1;
            if (c == QLatin1Char('=')) op = 0x0B;
            else if (c == QLatin1Char('<') && d == QLatin1Char('>')) {
                op = 0x0E;
                len = 2;
            } else if (c == QLatin1Char('<') && d == QLatin1Char('=')) {
                op = 0x0A;
                len = 2;
            } else if (c == QLatin1Char('>') && d == QLatin1Char('=')) {
                op = 0x0C;
                len = 2;
            } else if (c == QLatin1Char('<')) op = 0x09;
            else if (c == QLatin1Char('>')) op = 0x0D;
            if (!op) break;
            m_pos += len;
            const int right = parseConcat();
            if (right < 0) return -1;
            left = binary(op, left, right);
        }
        return left;
    }
    int parseConcat()
    {
        int left = parseAdd();
        while (left >= 0) {
            skipSpaces();
            if (peek() != QLatin1Char('&')) break;
            ++m_pos;
            const int right = parseAdd();
            if (right < 0) return -1;
            left = binary(0x08, left, right);
        }
        return left;
    }
    int parseAdd()
    {
        int left = parseMul();
        while (left >= 0) {
            skipSpaces();
            const QChar c = peek();
            if (c != QLatin1Char('+') && c != QLatin1Char('-')) break;
            ++m_pos;
            const int right = parseMul();
            if (right < 0) return -1;
            left = binary(c == QLatin1Char('+') ? 0x03 : 0x04, left, right);
        }
        return left;
    }
    int parseMul()
    {
        int left = parsePow();
        while (left >= 0) {
            skipSpaces();
            const QChar c = peek();
            if (c != QLatin1Char('*') && c != QLatin1Char('/')) break;
            ++m_pos;
            const int right = parsePow();
            if (right < 0) return -1;
            left = binary(c == QLatin1Char('*') ? 0x05 : 0x06, left, right);
        }
        return left;
    }
    int parsePow()
    {
        int left = parsePercent();
        while (left >= 0) {
            skipSpaces();
            if (peek() != QLatin1Char('^')) break;
            ++m_pos;
            const int right = parsePercent();
            if (right < 0) return -1;
            left = binary(0x07, left, right);
        }
        return left;
    }
    int parsePercent()
    {
        int x = parseUnary();
        while (x >= 0) {
            skipSpaces();
            if (peek() != QLatin1Char('%')) break;
            ++m_pos;
            Node n;
            n.kind = Kind::Percent;
            n.kids = {x};
            x = add(n);
        }
        return x;
    }
    int parseUnary()
    {
        DepthGuard g(this);
        if (m_depth > kMaxDepth) return failNode(QCoreApplication::translate("Sheet", "The formula is nested more deeply than the old format allows."));
        skipSpaces();
        const QChar c = peek();
        if (c == QLatin1Char('-') || c == QLatin1Char('+')) {
            ++m_pos;
            const int inner = parseUnary();
            if (inner < 0) return -1;
            Node n;
            n.kind = c == QLatin1Char('-') ? Kind::Neg : Kind::Plus;
            n.kids = {inner};
            return add(n);
        }
        return parseRefOps();
    }
    // The operators on references: the range operator ':' between two things that are not cells written together, and the intersection (a space between two operands).
    int parseRefOps()
    {
        int left = parsePrimary();
        while (left >= 0) {
            if (peek() == QLatin1Char(':')) {
                ++m_pos;
                const int right = parsePrimary();
                if (right < 0) return -1;
                left = binary(0x11, left, right);
                continue;
            }
            if (peek() == QLatin1Char(' ')) {
                // a space is the intersection only when an operand follows it
                int p = m_pos;
                while (p < m_t.size() && m_t[p] == QLatin1Char(' ')) ++p;
                const QChar n = p < m_t.size() ? m_t[p] : QChar();
                const bool operand = isIdentStart(n) || n == QLatin1Char('$') || n == QLatin1Char('\'') || n == QLatin1Char('(');
                if (operand && isRefLike(left)) {
                    m_pos = p;
                    const int right = parsePrimary();
                    if (right < 0) return -1;
                    left = binary(0x0F, left, right);
                    continue;
                }
            }
            break;
        }
        return left;
    }
    bool isRefLike(int node) const
    {
        const Kind k = m_nodes[size_t(node)].kind;
        return k == Kind::Ref || k == Kind::Name || k == Kind::Paren || (k == Kind::Binary && m_nodes[size_t(node)].code >= 0x0F);
    }

    // ---- operands

    int parsePrimary()
    {
        DepthGuard g(this);
        if (m_depth > kMaxDepth) return failNode(QCoreApplication::translate("Sheet", "The formula is nested more deeply than the old format allows."));
        skipSpaces();
        const QChar c = peek();
        if (c.isNull()) return failNode(QCoreApplication::translate("Sheet", "The formula has something the old format can't write."));
        if (c == QLatin1Char('(')) {
            ++m_pos;
            int e = parseCompare();
            if (e < 0) return -1;
            // a comma inside plain parentheses is the union of references
            skipSpaces();
            while (peek() == QLatin1Char(',')) {
                ++m_pos;
                const int next = parseCompare();
                if (next < 0) return -1;
                e = binary(0x10, e, next);
                skipSpaces();
            }
            if (!eat(QLatin1Char(')'))) return failNode(QCoreApplication::translate("Sheet", "The formula has something the old format can't write."));
            Node n;
            n.kind = Kind::Paren;
            n.kids = {e};
            return add(n);
        }
        if (c == QLatin1Char('"')) return parseString();
        if (c == QLatin1Char('#')) return parseError();
        if (c == QLatin1Char('{')) return parseArray();
        if (c.isDigit() || (c == QLatin1Char('.') && peek(1).isDigit())) return parseNumberOrRows();
        if (c == QLatin1Char('[')) return failNode(QCoreApplication::translate("Sheet", "The formula refers to another workbook, which the old format cannot do here."));
        return parseWord();
    }

    int parseString()
    {
        ++m_pos;
        QString s;
        for (;;) {
            if (m_pos >= m_t.size()) return failNode(QCoreApplication::translate("Sheet", "The formula has something the old format can't write."));
            const QChar c = m_t[m_pos++];
            if (c == QLatin1Char('"')) {
                if (peek() == QLatin1Char('"')) {
                    s += c;
                    ++m_pos;
                    continue;
                }
                break;
            }
            s += c;
        }
        if (s.size() > 255) return failNode(QCoreApplication::translate("Sheet", "The formula has a text of more than 255 characters, which the old format can't hold."));
        Node n;
        n.kind = Kind::Str;
        n.text = s;
        return add(n);
    }

    int parseError()
    {
        for (const Err &e : kErrors) {
            const QString t = QString::fromLatin1(e.text);
            if (m_t.mid(m_pos, t.size()).compare(t, Qt::CaseInsensitive) == 0) {
                m_pos += t.size();
                Node n;
                n.kind = Kind::Error;
                n.code = e.code;
                return add(n);
            }
        }
        return failNode(QCoreApplication::translate("Sheet", "The formula has something the old format can't write."));
    }

    // A number, or (when it is a row number followed by a colon and another) a range of whole rows.
    int parseNumberOrRows()
    {
        int p = m_pos;
        while (p < m_t.size() && m_t[p].isDigit()) ++p;
        if (p > m_pos && p < m_t.size() && m_t[p] == QLatin1Char(':') && p + 1 < m_t.size() && m_t[p + 1].isDigit()) {
            int q = p + 1;
            while (q < m_t.size() && m_t[q].isDigit()) ++q;
            if (q >= m_t.size() || !(isIdentStart(m_t[q]) || m_t[q] == QLatin1Char('('))) {
                Ref r;
                bool ok1 = false, ok2 = false;
                const qint64 a = m_t.mid(m_pos, p - m_pos).toLongLong(&ok1), b = m_t.mid(p + 1, q - p - 1).toLongLong(&ok2);
                // (the end of the new form's grid is the end of this one's: a range that runs to the last row is cut at its last)
                const qint64 bb = b == 1048576 ? kXlsMaxRows : b;
                if (!ok1 || !ok2 || a < 1 || bb < 1 || a > kXlsMaxRows || bb > kXlsMaxRows) return failNode(QCoreApplication::translate("Sheet", "The formula has a row past the old format's last one."));
                r.area = true;
                r.row1 = quint32(a - 1);
                r.row2 = quint32(bb - 1);
                r.col1 = 0;
                r.col2 = kXlsMaxCols - 1;
                r.row1Abs = r.row2Abs = false;
                m_pos = q;
                return refNode(r);
            }
        }
        int q = m_pos;
        while (q < m_t.size() && m_t[q].isDigit()) ++q;
        if (q < m_t.size() && m_t[q] == QLatin1Char('.')) {
            ++q;
            while (q < m_t.size() && m_t[q].isDigit()) ++q;
        }
        if (q < m_t.size() && (m_t[q] == QLatin1Char('E') || m_t[q] == QLatin1Char('e'))) {
            int e = q + 1;
            if (e < m_t.size() && (m_t[e] == QLatin1Char('+') || m_t[e] == QLatin1Char('-'))) ++e;
            if (e < m_t.size() && m_t[e].isDigit()) {
                while (e < m_t.size() && m_t[e].isDigit()) ++e;
                q = e;
            }
        }
        bool ok = false;
        const double v = m_t.mid(m_pos, q - m_pos).toDouble(&ok);
        if (!ok || !std::isfinite(v)) return failNode(QCoreApplication::translate("Sheet", "The formula has something the old format can't write."));
        m_pos = q;
        Node n;
        n.kind = Kind::Num;
        n.number = v;
        return add(n);
    }

    int parseArray()
    {
        ++m_pos;
        Node n;
        n.kind = Kind::Array;
        std::vector<int> row;
        qsizetype width = -1;
        for (;;) {
            skipSpaces();
            int cell = -1;
            const QChar c = peek();
            if (c == QLatin1Char('"')) cell = parseString();
            else if (c == QLatin1Char('#')) cell = parseError();
            else if (c == QLatin1Char('-') || c == QLatin1Char('+') || c.isDigit() || c == QLatin1Char('.')) {
                bool neg = false;
                if (c == QLatin1Char('-') || c == QLatin1Char('+')) {
                    neg = c == QLatin1Char('-');
                    ++m_pos;
                    skipSpaces();
                }
                if (!(peek().isDigit() || peek() == QLatin1Char('.'))) return failNode(QCoreApplication::translate("Sheet", "The formula has something the old format can't write."));
                cell = parseNumberOrRows();
                if (cell >= 0 && m_nodes[size_t(cell)].kind != Kind::Num) return failNode(QCoreApplication::translate("Sheet", "The formula has something the old format can't write."));
                if (cell >= 0 && neg) m_nodes[size_t(cell)].number = -m_nodes[size_t(cell)].number;
            } else if (isIdentStart(c)) {
                int p = m_pos;
                while (p < m_t.size() && isIdentChar(m_t[p])) ++p;
                const QString w = m_t.mid(m_pos, p - m_pos).toUpper();
                if (w == QLatin1String("TRUE") || w == QLatin1String("FALSE")) {
                    m_pos = p;
                    Node b;
                    b.kind = Kind::Bool;
                    b.code = w == QLatin1String("TRUE");
                    cell = add(b);
                } else {
                    return failNode(QCoreApplication::translate("Sheet", "The formula has something the old format can't write."));
                }
            } else {
                return failNode(QCoreApplication::translate("Sheet", "The formula has something the old format can't write."));
            }
            if (cell < 0) return -1;
            row.push_back(cell);
            skipSpaces();
            const QChar d = peek();
            if (d == QLatin1Char(',')) {
                ++m_pos;
            } else if (d == QLatin1Char(';') || d == QLatin1Char('}')) {
                ++m_pos;
                if (width < 0) width = qsizetype(row.size());
                if (qsizetype(row.size()) != width) return failNode(QCoreApplication::translate("Sheet", "The formula has something the old format can't write."));
                n.rows.push_back(row);
                row.clear();
                if (d == QLatin1Char('}')) break;
            } else {
                return failNode(QCoreApplication::translate("Sheet", "The formula has something the old format can't write."));
            }
            if (n.rows.size() > 256 || width > 256) return failNode(QCoreApplication::translate("Sheet", "The formula has something the old format can't write."));
        }
        return add(n);
    }

    // ---- words: functions, cell references, names, and truth values

    static bool cellAt(const QString &t, int p, int *end, quint32 *row, quint32 *col, bool *rowAbs, bool *colAbs)
    {
        int q = p;
        *colAbs = q < t.size() && t[q] == QLatin1Char('$');
        if (*colAbs) ++q;
        int letters = 0;
        quint32 c = 0;
        while (q < t.size() && ((t[q] >= QLatin1Char('A') && t[q] <= QLatin1Char('Z')) || (t[q] >= QLatin1Char('a') && t[q] <= QLatin1Char('z')))) {
            c = c * 26 + quint32(t[q].toUpper().unicode() - 'A' + 1);
            ++letters;
            ++q;
            if (letters > 3) return false;
        }
        if (letters == 0) return false;
        *rowAbs = q < t.size() && t[q] == QLatin1Char('$');
        if (*rowAbs) ++q;
        int digits = 0;
        qint64 r = 0;
        while (q < t.size() && t[q].isDigit()) {
            r = r * 10 + (t[q].unicode() - '0');
            ++digits;
            ++q;
            if (digits > 8) return false;
        }
        if (digits == 0 || r < 1) return false;
        if (q < t.size() && (isIdentChar(t[q]) || t[q] == QLatin1Char('('))) return false;
        *end = q;
        *row = quint32(r - 1);
        *col = c - 1;
        return true;
    }

    // A column letters alone ("A" or "$A") for a range of whole columns.
    static bool colAt(const QString &t, int p, int *end, quint32 *col, bool *abs)
    {
        int q = p;
        *abs = q < t.size() && t[q] == QLatin1Char('$');
        if (*abs) ++q;
        int letters = 0;
        quint32 c = 0;
        while (q < t.size() && ((t[q] >= QLatin1Char('A') && t[q] <= QLatin1Char('Z')) || (t[q] >= QLatin1Char('a') && t[q] <= QLatin1Char('z')))) {
            c = c * 26 + quint32(t[q].toUpper().unicode() - 'A' + 1);
            ++letters;
            ++q;
            if (letters > 3) return false;
        }
        if (letters == 0) return false;
        if (q < t.size() && (t[q].isDigit() || isIdentChar(t[q]) || t[q] == QLatin1Char('('))) return false;
        *end = q;
        *col = c - 1;
        return true;
    }

    int sheetIndex(const QString &name)
    {
        for (int i = 0; i < m_cx.sheetNames.size(); ++i)
            if (m_cx.sheetNames[i].compare(name, Qt::CaseInsensitive) == 0) return i;
        return -1;
    }

    // A sheet name written before "!" (quoted with apostrophes, or plain), or two of them with a colon between; true if there is one, and the position after the "!".
    bool sheetPrefix(int *afterBang, int *first, int *last, bool *unknown)
    {
        int p = m_pos;
        auto readName = [&](int *at, QString *name) {
            if (*at >= m_t.size()) return false;
            if (m_t[*at] == QLatin1Char('\'')) {
                int q = *at + 1;
                QString s;
                for (;;) {
                    if (q >= m_t.size()) return false;
                    if (m_t[q] == QLatin1Char('\'')) {
                        if (q + 1 < m_t.size() && m_t[q + 1] == QLatin1Char('\'')) {
                            s += QLatin1Char('\'');
                            q += 2;
                            continue;
                        }
                        ++q;
                        break;
                    }
                    s += m_t[q++];
                }
                *name = s;
                *at = q;
                return true;
            }
            int q = *at;
            while (q < m_t.size() && (isIdentChar(m_t[q]) || m_t[q].unicode() > 127)) ++q;
            if (q == *at) return false;
            *name = m_t.mid(*at, q - *at);
            *at = q;
            return true;
        };
        QString a, b;
        if (!readName(&p, &a)) return false;
        bool two = false;
        if (p < m_t.size() && m_t[p] == QLatin1Char(':')) {
            int q = p + 1;
            if (readName(&q, &b) && q < m_t.size() && m_t[q] == QLatin1Char('!')) {
                p = q;
                two = true;
            }
        }
        if (p >= m_t.size() || m_t[p] != QLatin1Char('!')) return false;
        *afterBang = p + 1;
        *first = sheetIndex(a);
        *last = two ? sheetIndex(b) : *first;
        *unknown = *first < 0 || *last < 0;
        return true;
    }

    int refNode(const Ref &r)
    {
        Node n;
        n.kind = Kind::Ref;
        n.ref = r;
        return add(n);
    }

    int parseWord()
    {
        const int start = m_pos;
        int afterBang = 0, s1 = -1, s2 = -1;
        bool unknownSheet = false;
        const bool prefixed = sheetPrefix(&afterBang, &s1, &s2, &unknownSheet);
        if (prefixed) {
            if (unknownSheet) {
                if (m_t.mid(start, 2) == QLatin1String("'["))
                    return failNode(QCoreApplication::translate("Sheet", "The formula refers to another workbook, which the old format cannot do here."));
                return failNode(QCoreApplication::translate("Sheet", "The formula names a sheet that is not in the workbook."));
            }
            m_pos = afterBang;
        }
        const int bodyStart = m_pos;
        // a reference to a sheet that is gone ("Sheet2!#REF!") is the error
        if (prefixed && peek() == QLatin1Char('#')) return parseError();
        // a function: a name directly followed by "("
        if (!prefixed) {
            int p = m_pos;
            while (p < m_t.size() && isIdentChar(m_t[p])) ++p;
            if (p > m_pos && p < m_t.size() && m_t[p] == QLatin1Char('(')) return parseFunction(p);
        }
        // a range of whole rows ("$1:$1048576", "Sheet2!2:5")
        {
            int q = bodyStart;
            const bool abs1 = q < m_t.size() && m_t[q] == QLatin1Char('$');
            if (abs1) ++q;
            int d1 = q;
            while (q < m_t.size() && m_t[q].isDigit()) ++q;
            if (q > d1 && q < m_t.size() && m_t[q] == QLatin1Char(':')) {
                int q2 = q + 1;
                const bool abs2 = q2 < m_t.size() && m_t[q2] == QLatin1Char('$');
                if (abs2) ++q2;
                const int d2 = q2;
                while (q2 < m_t.size() && m_t[q2].isDigit()) ++q2;
                if (q2 > d2 && (q2 >= m_t.size() || !(isIdentChar(m_t[q2]) || m_t[q2] == QLatin1Char('(')))) {
                    bool ok1 = false, ok2 = false;
                    const qint64 a = m_t.mid(d1, q - d1).toLongLong(&ok1), b0 = m_t.mid(d2, q2 - d2).toLongLong(&ok2);
                    const qint64 b = b0 == 1048576 ? kXlsMaxRows : b0;   // (a range to the last row is cut at the old form's last)
                    if (!ok1 || !ok2 || a < 1 || b < 1 || a > kXlsMaxRows || b > kXlsMaxRows) return failNode(QCoreApplication::translate("Sheet", "The formula has a row past the old format's last one."));
                    Ref r;
                    r.is3d = prefixed;
                    r.sheet1 = s1;
                    r.sheet2 = s2;
                    r.area = true;
                    r.row1 = quint32(a - 1);
                    r.row2 = quint32(b - 1);
                    r.row1Abs = abs1;
                    r.row2Abs = abs2;
                    r.col1 = 0;
                    r.col2 = kXlsMaxCols - 1;
                    m_pos = q2;
                    return refNode(r);
                }
            }
        }
        // a range of whole columns
        {
            int e1 = 0;
            quint32 c1 = 0;
            bool a1 = false;
            if (colAt(m_t, bodyStart, &e1, &c1, &a1) && e1 < m_t.size() && m_t[e1] == QLatin1Char(':')) {
                int e2 = 0;
                quint32 c2 = 0;
                bool a2 = false;
                if (colAt(m_t, e1 + 1, &e2, &c2, &a2)) {
                    if (c2 == 16383) c2 = kXlsMaxCols - 1;   // (a range to the last column is cut at the old form's last)
                    if (c1 >= quint32(kXlsMaxCols) || c2 >= quint32(kXlsMaxCols)) return failNode(QCoreApplication::translate("Sheet", "The formula has a column past the old format's last one."));
                    Ref r;
                    r.is3d = prefixed;
                    r.sheet1 = s1;
                    r.sheet2 = s2;
                    r.area = true;
                    r.col1 = c1;
                    r.col2 = c2;
                    r.col1Abs = a1;
                    r.col2Abs = a2;
                    r.row1 = 0;
                    r.row2 = kXlsMaxRows - 1;
                    m_pos = e2;
                    return refNode(r);
                }
            }
        }
        // a cell, or two cells joined by a colon
        {
            int e1 = 0;
            quint32 r1 = 0, c1 = 0;
            bool ra1 = false, ca1 = false;
            if (cellAt(m_t, bodyStart, &e1, &r1, &c1, &ra1, &ca1)) {
                if (r1 >= quint32(kXlsMaxRows) || c1 >= quint32(kXlsMaxCols)) {
                    return failNode(QCoreApplication::translate("Sheet", "The formula has a cell past the old format's last row or column."));
                }
                Ref r;
                r.is3d = prefixed;
                r.sheet1 = s1;
                r.sheet2 = s2;
                r.row1 = r.row2 = r1;
                r.col1 = r.col2 = c1;
                r.row1Abs = r.row2Abs = ra1;
                r.col1Abs = r.col2Abs = ca1;
                m_pos = e1;
                if (e1 < m_t.size() && m_t[e1] == QLatin1Char(':')) {
                    int e2 = 0;
                    quint32 r2 = 0, c2 = 0;
                    bool ra2 = false, ca2 = false;
                    if (cellAt(m_t, e1 + 1, &e2, &r2, &c2, &ra2, &ca2)) {
                        if (r2 == 1048575) r2 = kXlsMaxRows - 1;   // (a range to the last row or column is cut at the old form's last)
                        if (c2 == 16383) c2 = kXlsMaxCols - 1;
                        if (r2 >= quint32(kXlsMaxRows) || c2 >= quint32(kXlsMaxCols)) {
                            return failNode(QCoreApplication::translate("Sheet", "The formula has a cell past the old format's last row or column."));
                        }
                        r.area = true;
                        r.row2 = r2;
                        r.col2 = c2;
                        r.row2Abs = ra2;
                        r.col2Abs = ca2;
                        m_pos = e2;
                    }
                }
                return refNode(r);
            }
        }
        // a name, or a truth value
        int p = bodyStart;
        while (p < m_t.size() && (isIdentChar(m_t[p]) || m_t[p].unicode() > 127)) ++p;
        if (p == bodyStart) {
            m_pos = start;
            return failNode(QCoreApplication::translate("Sheet", "The formula has something the old format can't write."));
        }
        const QString word = m_t.mid(bodyStart, p - bodyStart);
        if (!prefixed) {
            const QString up = word.toUpper();
            if (up == QLatin1String("TRUE") || up == QLatin1String("FALSE")) {
                m_pos = p;
                Node n;
                n.kind = Kind::Bool;
                n.code = up == QLatin1String("TRUE");
                return add(n);
            }
        }
        if (p < m_t.size() && m_t[p] == QLatin1Char('[')) return failNode(QCoreApplication::translate("Sheet", "The formula names a table's column, which the old format can't hold."));
        const QString key = word.toLower();
        int found = 0;
        if (prefixed) found = m_cx.names.value(QString::number(s1) + QLatin1Char('|') + key, 0);
        else {
            found = m_cx.sheet >= 0 ? m_cx.names.value(QString::number(m_cx.sheet) + QLatin1Char('|') + key, 0) : 0;
            if (!found) found = m_cx.names.value(QStringLiteral("-1|") + key, 0);
        }
        if (!found) return failNode(QCoreApplication::translate("Sheet", "The formula uses a name the old format doesn't have."));
        m_pos = p;
        Node n;
        n.kind = Kind::Name;
        n.index = found;
        n.text = word;
        return add(n);
    }

    int parseFunction(int nameEnd)
    {
        const QString name = m_t.mid(m_pos, nameEnd - m_pos).toUpper();
        int number = xlsFunctionNumber(name);
        bool addIn = false;
        if (number < 0 && xlsIsAddInFunction(name) && m_cx.addIn) {
            number = 255;
            addIn = true;
        }
        if (number < 0) return failNode(QCoreApplication::translate("Sheet", "The formula uses the function %1, which the old format doesn't have.").arg(name));
        m_pos = nameEnd + 1;
        Node n;
        n.kind = Kind::Func;
        n.text = name;
        n.index = number;
        n.code = addIn ? 1 : 0;
        skipSpaces();
        if (peek() == QLatin1Char(')')) {
            ++m_pos;
            return add(n);
        }
        for (;;) {
            skipSpaces();
            int arg;
            if (peek() == QLatin1Char(',') || peek() == QLatin1Char(')')) {
                Node miss;
                miss.kind = Kind::Missing;
                arg = add(miss);
            } else {
                arg = parseCompare();
            }
            if (arg < 0) return -1;
            n.kids.push_back(arg);
            if (int(n.kids.size()) > kMaxArgs - (addIn ? 1 : 0)) return failNode(QCoreApplication::translate("Sheet", "The formula calls a function with more than 30 arguments, which the old format can't hold."));
            skipSpaces();
            if (peek() == QLatin1Char(',')) {
                ++m_pos;
                continue;
            }
            if (peek() == QLatin1Char(')')) {
                ++m_pos;
                break;
            }
            return failNode(QCoreApplication::translate("Sheet", "The formula has something the old format can't write."));
        }
        return add(n);
    }

    // ---- writing the tokens

    void u8(int v) { m_out.append(char(v)); }
    void u16(int v)
    {
        m_out.append(char(v & 0xFF));
        m_out.append(char((v >> 8) & 0xFF));
    }
    static void put16(QByteArray *a, int v)
    {
        a->append(char(v & 0xFF));
        a->append(char((v >> 8) & 0xFF));
    }
    static void putDouble(QByteArray *a, double v)
    {
        quint64 bits;
        std::memcpy(&bits, &v, 8);
        for (int i = 0; i < 8; ++i) a->append(char((bits >> (8 * i)) & 0xFF));
    }

    bool emitString(QByteArray *a, const QString &s, int countBytes)
    {
        bool wide = false;
        for (const QChar c : s)
            if (c.unicode() > 0xFF) wide = true;
        if (countBytes == 1) a->append(char(s.size()));
        else put16(a, int(s.size()));
        a->append(char(wide ? 1 : 0));
        if (wide) {
            for (const QChar c : s) put16(a, c.unicode());
        } else {
            for (const QChar c : s) a->append(char(c.unicode()));
        }
        return true;
    }

    // The class an operand is written in, given the class its place asks for.
    bool emit(int idx, int cls, int depth)
    {
        if (depth > kMaxDepth * 2) return fail(QCoreApplication::translate("Sheet", "The formula is nested more deeply than the old format allows."));
        const Node &n = m_nodes[size_t(idx)];
        const int valueCls = m_cx.array ? kClsArr : kClsVal;
        switch (n.kind) {
        case Kind::Num: {
            const double v = n.number;
            if (v >= 0 && v <= 65535 && v == std::floor(v)) {
                u8(0x1E);
                u16(int(v));
            } else {
                u8(0x1F);
                putDouble(&m_out, v);
            }
            return true;
        }
        case Kind::Str:
            u8(0x17);
            return emitString(&m_out, n.text, 1);
        case Kind::Bool:
            u8(0x1D);
            u8(n.code);
            return true;
        case Kind::Error:
            u8(0x1C);
            u8(n.code);
            return true;
        case Kind::Missing:
            u8(0x16);
            return true;
        case Kind::Name:
            u8(0x03 + cls);
            u16(n.index);
            u16(0);
            return true;
        case Kind::Ref: return emitRef(n.ref, cls);
        case Kind::Paren:
            if (!emit(n.kids[0], cls, depth + 1)) return false;
            u8(0x15);
            return true;
        case Kind::Neg:
        case Kind::Plus:
        case Kind::Percent:
            if (!emit(n.kids[0], valueCls, depth + 1)) return false;
            u8(n.kind == Kind::Neg ? 0x13 : n.kind == Kind::Plus ? 0x12 : 0x14);
            return true;
        case Kind::Binary: {
            const int operand = n.code >= 0x0F ? kClsRef : valueCls;
            if (!emit(n.kids[0], operand, depth + 1)) return false;
            if (!emit(n.kids[1], operand, depth + 1)) return false;
            u8(n.code);
            return true;
        }
        case Kind::Array: return emitArray(n, cls);
        case Kind::Func: return emitFunc(n, cls, depth);
        }
        return false;
    }

    bool emitRef(const Ref &r, int cls)
    {
        const int colFlag1 = (r.col1Abs ? 0 : 0x4000) | (r.row1Abs ? 0 : 0x8000);
        const int colFlag2 = (r.col2Abs ? 0 : 0x4000) | (r.row2Abs ? 0 : 0x8000);
        if (r.is3d) {
            const int first = r.sheet1, last = r.sheet2 < 0 ? r.sheet1 : r.sheet2;
            const int x = m_cx.xti ? m_cx.xti(std::min(first, last), std::max(first, last)) : 0;
            if (x < 0) return fail(QCoreApplication::translate("Sheet", "The formula names more sheets than the old format can."));
            if (r.area) {
                u8(0x1B + cls);
                u16(x);
                u16(int(r.row1));
                u16(int(r.row2));
                u16(int(r.col1) | colFlag1);
                u16(int(r.col2) | colFlag2);
            } else {
                u8(0x1A + cls);
                u16(x);
                u16(int(r.row1));
                u16(int(r.col1) | colFlag1);
            }
            return true;
        }
        if (r.area) {
            u8(0x05 + cls);
            u16(int(r.row1));
            u16(int(r.row2));
            u16(int(r.col1) | colFlag1);
            u16(int(r.col2) | colFlag2);
        } else {
            u8(0x04 + cls);
            u16(int(r.row1));
            u16(int(r.col1) | colFlag1);
        }
        return true;
    }

    bool emitArray(const Node &n, int cls)
    {
        u8(0x00 + cls);
        m_out.append(QByteArray(7, '\0'));
        const int cols = n.rows.empty() ? 1 : int(n.rows[0].size());
        m_extra.append(char(cols - 1));
        put16(&m_extra, int(n.rows.size()) - 1);
        for (const std::vector<int> &row : n.rows) {
            for (int cell : row) {
                const Node &c = m_nodes[size_t(cell)];
                if (c.kind == Kind::Num) {
                    m_extra.append(char(0x01));
                    putDouble(&m_extra, c.number);
                } else if (c.kind == Kind::Str) {
                    m_extra.append(char(0x02));
                    emitString(&m_extra, c.text, 2);
                } else if (c.kind == Kind::Bool) {
                    m_extra.append(char(0x04));
                    m_extra.append(char(c.code));
                    m_extra.append(QByteArray(7, '\0'));
                } else {
                    m_extra.append(char(0x10));
                    m_extra.append(char(c.code));
                    m_extra.append(QByteArray(7, '\0'));
                }
            }
        }
        return true;
    }

    bool emitFunc(const Node &n, int cls, int depth)
    {
        const bool arrays = takesArrays(n.text);
        const int valueCls = m_cx.array ? kClsArr : kClsVal;
        if (n.code == 1) {
            // a function of the add-in set: the name of it comes first, then the arguments, and the call is function 255 with one more argument
            int ixti = 0, index = 0;
            if (!m_cx.addIn(n.text, &ixti, &index)) return fail(QCoreApplication::translate("Sheet", "The formula names more functions than the old format can."));
            u8(0x39);
            u16(ixti);
            u16(index);
            u16(0);
        }
        for (int kid : n.kids) {
            const Node &k = m_nodes[size_t(kid)];
            int argCls = arrays ? kClsArr : valueCls;
            if (!arrays && (k.kind == Kind::Ref || k.kind == Kind::Name)) argCls = kClsRef;
            if (!arrays && k.kind == Kind::Paren) {
                const Node &inner = m_nodes[size_t(k.kids[0])];
                if (inner.kind == Kind::Ref || inner.kind == Kind::Name) argCls = kClsRef;
            }
            if (!emit(kid, argCls, depth + 1)) return false;
        }
        const XlsFunction f = xlsFunction(n.index);
        const int argc = int(n.kids.size());
        const int own = cls == kClsRef ? kClsRef : valueCls;
        if (n.code == 1) {
            u8(0x02 + own);
            u8(argc + 1);
            u16(255);
        } else if (f.fixedArgs >= 0 && f.fixedArgs == argc) {
            u8(0x01 + own);
            u16(n.index);
        } else {
            u8(0x02 + own);
            u8(argc);
            u16(n.index);
        }
        return true;
    }
};

} // namespace

bool xlsIsAddInFunction(const QString &name)
{
    static const QSet<QString> set = [] {
        static const char *const names[] = {"ACCRINT", "ACCRINTM", "AMORDEGRC", "AMORLINC", "BESSELI", "BESSELJ", "BESSELK", "BESSELY", "BIN2DEC", "BIN2HEX", "BIN2OCT", "COMPLEX", "CONVERT",
            "COUPDAYBS", "COUPDAYS", "COUPDAYSNC", "COUPNCD", "COUPNUM", "COUPPCD", "CUMIPMT", "CUMPRINC", "DEC2BIN", "DEC2HEX", "DEC2OCT", "DELTA", "DISC", "DOLLARDE", "DOLLARFR", "DURATION", "EDATE",
            "EFFECT", "EOMONTH", "ERF", "ERFC", "FACTDOUBLE", "FVSCHEDULE", "GCD", "GESTEP", "HEX2BIN", "HEX2DEC", "HEX2OCT", "IMABS", "IMAGINARY", "IMARGUMENT", "IMCONJUGATE", "IMCOS", "IMDIV", "IMEXP",
            "IMLN", "IMLOG10", "IMLOG2", "IMPOWER", "IMPRODUCT", "IMREAL", "IMSIN", "IMSQRT", "IMSUB", "IMSUM", "INTRATE", "ISEVEN", "ISODD", "LCM", "MDURATION", "MROUND", "MULTINOMIAL", "NETWORKDAYS",
            "NOMINAL", "OCT2BIN", "OCT2DEC", "OCT2HEX", "ODDFPRICE", "ODDFYIELD", "ODDLPRICE", "ODDLYIELD", "PRICE", "PRICEDISC", "PRICEMAT", "QUOTIENT", "RANDBETWEEN", "RECEIVED", "SERIESSUM", "SQRTPI",
            "TBILLEQ", "TBILLPRICE", "TBILLYIELD", "WEEKNUM", "WORKDAY", "XIRR", "XNPV", "YEARFRAC", "YIELD", "YIELDDISC", "YIELDMAT"};
        QSet<QString> out;
        for (const char *n : names) out.insert(QString::fromLatin1(n));
        return out;
    }();
    return set.contains(name);
}

int xlsFunctionNumber(const QString &name)
{
    return functionNumbers().value(name, -1);
}

bool xlsCompileFormula(const QString &text, const XlsCompileContext &context, QByteArray *rgce, QByteArray *extra, QString *why)
{
    Compiler c(text, context);
    return c.run(rgce, extra, why);
}

} // namespace jp::office::sheet
