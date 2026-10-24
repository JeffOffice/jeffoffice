#include "sheet/xlsformula.h"

#include "sheet/cellref.h"
#include "sheet/xlsbytes.h"
#include "sheet/xlsfunctions.h"

#include <QCoreApplication>

#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

namespace jp::office::sheet {

namespace {

// A number written as a formula does: the fewest digits (15 to 17) that read back as the same number.
QString numberText(double v)
{
    if (!std::isfinite(v)) return QStringLiteral("#NUM!");
    QString s;
    for (int digits = 15; digits <= 17; ++digits) {
        s = QString::number(v, 'g', digits);
        if (s.toDouble() == v) break;
    }
    return s.replace(QLatin1Char('e'), QLatin1Char('E'));
}

bool looksLikeCell(const QString &name)
{
    // letters then digits ("A1", "XFD1048576"), or the R1C1 forms ("R", "C", "R2", "RC", "R1C1", "R[1]C")
    int i = 0;
    while (i < name.size() && name[i].isLetter() && name[i].unicode() < 128) ++i;
    if (i >= 1 && i <= 3 && i < name.size()) {
        int j = i;
        while (j < name.size() && name[j].isDigit() && name[j].unicode() < 128) ++j;
        if (j == name.size()) return true;
    }
    const QString up = name.toUpper();
    if (up == QLatin1String("R") || up == QLatin1String("C")) return true;
    if (up.startsWith(QLatin1Char('R')) || up.startsWith(QLatin1Char('C'))) {
        bool onlyRc = true;
        for (QChar ch : QStringView(up).mid(1))
            if (!ch.isDigit() && ch != QLatin1Char('R') && ch != QLatin1Char('C') && ch != QLatin1Char('[') && ch != QLatin1Char(']') && ch != QLatin1Char('-')) onlyRc = false;
        if (onlyRc) return true;
    }
    return false;
}

bool needsQuotes(const QString &name)
{
    if (name.isEmpty()) return true;
    if (name[0].isDigit()) return true;
    for (QChar c : name)
        if (!(c.isLetterOrNumber() || c == QLatin1Char('_') || c == QLatin1Char('.'))) return true;
    const QString up = name.toUpper();
    if (up == QLatin1String("TRUE") || up == QLatin1String("FALSE")) return true;
    return looksLikeCell(name);
}

QString quoted(const QString &name)
{
    QString q = name;
    q.replace(QLatin1Char('\''), QLatin1String("''"));
    return QLatin1Char('\'') + q + QLatin1Char('\'');
}

QString maybeQuoted(const QString &name) { return needsQuotes(name) ? quoted(name) : name; }

// What a reference's part is: a place, as the format stores it (with its relative flags).
struct Part {
    quint32 index = 0;
    bool absolute = false;
};

} // namespace

QString xlsErrorText(int code)
{
    switch (code) {
    case 0x00: return QStringLiteral("#NULL!");
    case 0x07: return QStringLiteral("#DIV/0!");
    case 0x0F: return QStringLiteral("#VALUE!");
    case 0x17: return QStringLiteral("#REF!");
    case 0x1D: return QStringLiteral("#NAME?");
    case 0x24: return QStringLiteral("#NUM!");
    case 0x2A: return QStringLiteral("#N/A");
    case 0x2B: return QStringLiteral("#GETTING_DATA");
    }
    return QString();
}

QString xlsSheetPrefix(const QString &name) { return maybeQuoted(name) + QLatin1Char('!'); }

// ---------------------------------------------------------------------------------------------------------------------------------------------------
// the tree

namespace {

enum class NodeKind : quint8 { Lit, Ref, Pre, Post, Paren, Bin, Call };

// One piece of a formula's text. A formula is held as a tree of these and written out in one pass, so that no operator costs the length of what it is put round: the text of a
// chain of a thousand parentheses or minus signs is made in time in proportion to its length. A reference keeps the fields the file stores, and is written for a base cell when
// the text is made, so that a formula shared by many cells is read once.
struct Node {
    NodeKind kind = NodeKind::Lit;
    bool area = false;
    bool r1Rel = false, c1Rel = false, r2Rel = false, c2Rel = false;
    quint32 r1 = 0, c1 = 0, r2 = 0, c2 = 0;
    qint32 a = -1, b = -1;        // the operands (a call: the node that is the name, if it is not a text)
    qint32 first = 0, count = 0;  // a call's arguments, in the tree's list
    QString text;                 // a literal; the operator or the prefix or suffix; a call's name; a reference's sheet
};

} // namespace

struct XlsFormulaTree::Impl {
    std::vector<Node> nodes;
    std::vector<qint32> kids;
    qint32 root = -1;
    bool relative = false, name = false, b8 = true;
    quint32 maxRow = 65535;
    qint64 maxChars = 100000;
    quint32 baseRow = 0, baseCol = 0;
    QString why;

    // The row and column as a reference is written, from the fields as stored.
    void place(quint32 rowField, quint32 colField, bool rowRel, bool colRel, quint32 base[2], quint32 *row, quint32 *col, bool *outside) const
    {
        if (!relative) {
            *row = rowField;
            *col = colField;
            return;
        }
        qint64 r = rowField, c = colField;
        if (rowRel) {
            const qint64 off = b8 ? qint16(rowField) : qint64(rowField & 0x2000 ? qint64(rowField) - 0x4000 : qint64(rowField));
            r = qint64(base[0]) + off;
            const qint64 rows = name ? qint64(kMaxRows) : qint64(maxRow) + 1;
            if (r < 0 || r >= rows) *outside = true;
            r = ((r % rows) + rows) % rows;
        }
        if (colRel) {
            const qint64 off = qint8(colField & 0xFF);
            c = qint64(base[1]) + off;
            const qint64 cols = name ? qint64(kMaxColumns) : 256;
            if (c < 0 || c >= cols) *outside = true;
            c = ((c % cols) + cols) % cols;
        }
        *row = quint32(r);
        *col = quint32(c);
    }

    static void appendCell(QString *out, quint32 row, quint32 col, bool rowAbs, bool colAbs)
    {
        if (colAbs) out->append(QLatin1Char('$'));
        out->append(columnName(col));
        if (rowAbs) out->append(QLatin1Char('$'));
        out->append(QString::number(qint64(row) + 1));
    }

    void appendRef(QString *out, const Node &n, quint32 base[2], bool *outside) const
    {
        quint32 row1, col1;
        place(n.r1, n.c1, n.r1Rel, n.c1Rel, base, &row1, &col1, outside);
        out->append(n.text);
        if (!n.area) {
            appendCell(out, row1, col1, !n.r1Rel, !n.c1Rel);
            return;
        }
        quint32 row2, col2;
        place(n.r2, n.c2, n.r2Rel, n.c2Rel, base, &row2, &col2, outside);
        const bool allRows = row1 == 0 && row2 >= maxRow;
        const bool allCols = col1 == 0 && col2 >= 255;
        if (allRows) {
            if (!n.c1Rel) out->append(QLatin1Char('$'));
            out->append(columnName(col1));
            out->append(QLatin1Char(':'));
            if (!n.c2Rel) out->append(QLatin1Char('$'));
            out->append(columnName(col2));
        } else if (allCols) {
            if (!n.r1Rel) out->append(QLatin1Char('$'));
            out->append(QString::number(qint64(row1) + 1));
            out->append(QLatin1Char(':'));
            if (!n.r2Rel) out->append(QLatin1Char('$'));
            out->append(QString::number(qint64(row2) + 1));
        } else {
            appendCell(out, row1, col1, !n.r1Rel, !n.c1Rel);
            out->append(QLatin1Char(':'));
            appendCell(out, row2, col2, !n.r2Rel, !n.c2Rel);
        }
    }

    XlsFormulaResult render(quint32 baseRow, quint32 baseCol) const
    {
        XlsFormulaResult res;
        if (!why.isEmpty() || root < 0) {
            res.why = why.isEmpty() ? QCoreApplication::translate("Sheet", "A formula's tokens do not make one value.") : why;
            return res;
        }
        quint32 base[2] = {baseRow, baseCol};
        QString out;
        // what is still to be written: a node, a character, or the text of a node, from the last
        struct Item {
            qint32 node;
            char16_t ch;
            quint8 what;   // 0 a node, 1 a character, 2 the text of a node
        };
        std::vector<Item> work;
        work.push_back({root, 0, 0});
        auto pushNode = [&](qint32 n) { work.push_back({n, 0, 0}); };
        auto pushChar = [&](char16_t c) { work.push_back({-1, c, 1}); };
        auto pushText = [&](qint32 n) { work.push_back({n, 0, 2}); };
        while (!work.empty()) {
            const Item it = work.back();
            work.pop_back();
            if (it.what == 1) {
                out.append(QChar(it.ch));
            } else if (it.what == 2) {
                out.append(nodes[size_t(it.node)].text);
            } else {
                const Node &n = nodes[size_t(it.node)];
                switch (n.kind) {
                case NodeKind::Lit: out.append(n.text); break;
                case NodeKind::Ref: appendRef(&out, n, base, &res.outside); break;
                case NodeKind::Pre:
                    pushNode(n.a);
                    pushText(it.node);
                    break;
                case NodeKind::Post:
                    pushText(it.node);
                    pushNode(n.a);
                    break;
                case NodeKind::Paren:
                    pushChar(u')');
                    pushNode(n.a);
                    pushChar(u'(');
                    break;
                case NodeKind::Bin:
                    pushNode(n.b);
                    pushText(it.node);
                    pushNode(n.a);
                    break;
                case NodeKind::Call:
                    pushChar(u')');
                    for (qint32 k = n.count - 1; k >= 0; --k) {
                        pushNode(kids[size_t(n.first + k)]);
                        if (k > 0) pushChar(u',');
                    }
                    pushChar(u'(');
                    if (n.a >= 0) pushNode(n.a);
                    else pushText(it.node);
                    break;
                }
            }
            if (out.size() > maxChars) {
                res.why = QCoreApplication::translate("Sheet", "A formula has a token this program does not read.");
                res.outside = false;
                return res;
            }
        }
        res.text = out;
        return res;
    }
};

XlsFormulaTree::XlsFormulaTree() = default;
bool XlsFormulaTree::ok() const { return d && d->why.isEmpty() && d->root >= 0; }
QString XlsFormulaTree::why() const { return d ? d->why : QString(); }
qint64 XlsFormulaTree::size() const { return d ? qint64(d->nodes.size()) : 0; }
XlsFormulaResult XlsFormulaTree::render(quint32 baseRow, quint32 baseCol) const
{
    if (!d) {
        XlsFormulaResult r;
        r.why = QCoreApplication::translate("Sheet", "A formula's tokens do not make one value.");
        return r;
    }
    return d->render(baseRow, baseCol);
}

namespace {

class Converter {
public:
    Converter(const XlsContext &cx, const QByteArray &rgce, const QByteArray &extra, const XlsFormulaOptions &opt)
        : m_cx(cx), m_opt(opt), m_in(rgce), m_extra(extra), m_b8(cx.biff >= 8), m_maxRow(cx.biff >= 8 ? 65535u : 16383u)
    {
    }

    XlsFormulaTree run()
    {
        auto impl = std::make_shared<XlsFormulaTree::Impl>();
        impl->relative = m_opt.relative;
        impl->name = m_opt.name;
        impl->b8 = m_b8;
        impl->maxRow = m_maxRow;
        impl->maxChars = m_opt.maxChars;
        impl->baseRow = m_opt.baseRow;
        impl->baseCol = m_opt.baseCol;
        const QString why = walk();
        if (!why.isEmpty()) impl->why = why;
        else if (m_stack.size() != 1) impl->why = QCoreApplication::translate("Sheet", "A formula's tokens do not make one value.");
        else impl->root = m_stack.last();
        impl->nodes = std::move(m_nodes);
        impl->kids = std::move(m_kids);
        return XlsFormulaTree(impl);
    }

private:
    const XlsContext &m_cx;
    const XlsFormulaOptions &m_opt;
    Bytes m_in, m_extra;
    bool m_b8;
    quint32 m_maxRow;
    std::vector<Node> m_nodes;
    std::vector<qint32> m_kids;
    QVector<qint32> m_stack;
    qint64 m_chars = 0;
    QVector<int> m_ordinals5;   // BIFF5: for each record of the list of external sheets, the number of the other book it names (0 for this workbook's own)

    QString bad() const { return QCoreApplication::translate("Sheet", "A formula has a token this program does not read."); }

    // A node made, with the characters it will write counted against the bound; -1 when past it.
    qint32 make(Node n, qint64 chars)
    {
        m_chars += chars;
        if (m_chars > m_opt.maxChars) return -1;
        m_nodes.push_back(std::move(n));
        return qint32(m_nodes.size() - 1);
    }
    bool push(qint32 node)
    {
        if (node < 0) return false;
        m_stack.append(node);
        return true;
    }
    bool pushLit(const QString &text)
    {
        Node n;
        n.text = text;
        return push(make(std::move(n), text.size()));
    }
    bool pop(qint32 *node)
    {
        if (m_stack.isEmpty()) return false;
        *node = m_stack.takeLast();
        return true;
    }
    bool pushRef(const QString &sheet, bool area, quint32 r1, quint32 c1, bool r1Rel, bool c1Rel, quint32 r2, quint32 c2, bool r2Rel, bool c2Rel)
    {
        Node n;
        n.kind = NodeKind::Ref;
        n.area = area;
        n.text = sheet;
        n.r1 = r1;
        n.c1 = c1;
        n.r1Rel = r1Rel;
        n.c1Rel = c1Rel;
        n.r2 = r2;
        n.c2 = c2;
        n.r2Rel = r2Rel;
        n.c2Rel = c2Rel;
        return push(make(std::move(n), sheet.size() + (area ? 40 : 20)));   // (the most a reference writes, but for the sheet's name, which is in the count)
    }
    bool pushUnary(const QString &text, bool before, qint32 operand)
    {
        Node n;
        n.kind = before ? NodeKind::Pre : NodeKind::Post;
        n.text = text;
        n.a = operand;
        return push(make(std::move(n), text.size()));
    }

    // The sheet part of a reference ("Sheet1!", "'My Sheet'!", "[1]Sheet1!", "Sheet1:Sheet3!"); `deleted` when it is a sheet that is gone.
    QString sheetPart(int ixti, bool *deleted)
    {
        *deleted = false;
        if (ixti < 0 || ixti >= m_cx.xti.size()) {
            *deleted = true;
            return QString();
        }
        const XlsXti &x = m_cx.xti[ixti];
        if (x.supbook < 0 || x.supbook >= m_cx.supbooks.size()) {
            *deleted = true;
            return QString();
        }
        const XlsSupBook &book = m_cx.supbooks[x.supbook];
        if (x.first == -1 || x.last == -1) {
            *deleted = true;
            return QString();
        }
        auto nameAt = [&](int i) {
            if (book.kind == XlsSupBook::Kind::Internal) return i >= 0 && i < m_cx.sheetNames.size() ? m_cx.sheetNames[i] : QString();
            return i >= 0 && i < book.sheets.size() ? book.sheets[i] : QStringLiteral("Sheet") + QString::number(i + 1);
        };
        const QString prefix = book.kind == XlsSupBook::Kind::External ? QStringLiteral("[") + QString::number(book.ordinal) + QLatin1Char(']') : QString();
        if (x.first == -2) return prefix.isEmpty() ? QString() : prefix + QLatin1Char('!');
        QString name = nameAt(x.first);
        if (book.kind == XlsSupBook::Kind::Internal && name.isEmpty()) {
            *deleted = true;
            return QString();
        }
        if (x.last != x.first && x.last >= 0) name += QLatin1Char(':') + nameAt(x.last);
        return (needsQuotes(name) ? quoted(prefix + name) : prefix + name) + QLatin1Char('!');
    }

    // The same for the form BIFF5 uses: the number of an external-sheet record (negative: sheets of this workbook), and the first and last sheet.
    QString sheetPart5(int iw, int first, int last, bool *deleted)
    {
        *deleted = false;
        if (iw < 0 || (iw > 0 && iw <= m_cx.externSheet5.size() && !m_cx.externSheet5[iw - 1].isEmpty() && quint8(m_cx.externSheet5[iw - 1][0]) == 0x03)
            || (iw > 0 && iw <= m_cx.externSheet5.size() && m_cx.externSheet5[iw - 1].size() == 2 && quint8(m_cx.externSheet5[iw - 1][0]) == 0x01)) {
            QString name;
            if (iw > 0 && quint8(m_cx.externSheet5[iw - 1][0]) == 0x03 && m_cx.externSheet5[iw - 1].size() > 1 && m_cx.bytesToText) {
                // a sheet of this workbook, by its name
                name = m_cx.bytesToText(m_cx.externSheet5[iw - 1].constData() + 1, int(m_cx.externSheet5[iw - 1].size() - 1));
            } else if (first >= 0 && first < m_cx.sheetNames.size()) {
                name = m_cx.sheetNames[first];
                if (last != first && last >= 0 && last < m_cx.sheetNames.size()) name += QLatin1Char(':') + m_cx.sheetNames[last];
            }
            if (name.isEmpty()) {
                *deleted = true;
                return QString();
            }
            return maybeQuoted(name) + QLatin1Char('!');
        }
        if (iw == 0 || iw > m_cx.externSheet5.size()) {
            *deleted = true;
            return QString();
        }
        // another workbook: the name has the file, then the sheet after a 0x03
        const QByteArray &enc = m_cx.externSheet5[iw - 1];
        int cut = int(enc.lastIndexOf('\x03'));
        QString sheet = m_cx.bytesToText ? m_cx.bytesToText(enc.constData() + cut + 1, int(enc.size() - cut - 1)) : QString();
        sheet.remove(QChar(0x01)).remove(QChar(0x02));
        const int ordinal = ordinalOf5(iw);
        const QString book = QStringLiteral("[") + QString::number(ordinal) + QLatin1Char(']');
        return (needsQuotes(sheet) ? quoted(book + sheet) : book + sheet) + QLatin1Char('!');
    }

    // The number of the other book that record `iw` of the list (counted from 1) names, counting the records that name one.
    int ordinalOf5(int iw)
    {
        if (m_ordinals5.isEmpty()) {
            int ordinal = 0;
            m_ordinals5.reserve(int(m_cx.externSheet5.size()) + 1);
            m_ordinals5.append(1);   // (before the first record: the first other book is 1)
            for (const QByteArray &e : m_cx.externSheet5) {
                if (!e.isEmpty() && quint8(e[0]) != 0x03 && !(e.size() == 2 && quint8(e[0]) == 0x01)) ++ordinal;
                m_ordinals5.append(1 + ordinal);
            }
        }
        return iw >= 1 && iw - 1 < m_ordinals5.size() ? m_ordinals5[iw - 1] : 1;
    }

    QString nameText(int index)
    {
        if (index < 1 || index > m_cx.names.size()) return QString();
        const XlsNameInfo &n = m_cx.names[index - 1];
        if (n.localSheet >= 0 && n.localSheet != m_opt.sheet && n.localSheet < m_cx.sheetNames.size()) return xlsSheetPrefix(m_cx.sheetNames[n.localSheet]) + n.text;
        return n.text;
    }

    QString arrayConstant()
    {
        qint64 cols, rows;
        if (m_b8) {
            cols = qint64(m_extra.u8()) + 1;
            rows = qint64(m_extra.u16()) + 1;
        } else {
            cols = m_extra.u8();
            if (cols == 0) cols = 256;
            rows = m_extra.u16();
        }
        // (a value takes at least a byte: a count the bytes can't hold is damage)
        if (m_extra.bad() || rows * cols > m_extra.left() || rows * cols > 1000000) return QString();
        QString out = QStringLiteral("{");
        for (qint64 r = 0; r < rows; ++r) {
            if (r) out += QLatin1Char(';');
            for (qint64 c = 0; c < cols; ++c) {
                if (c) out += QLatin1Char(',');
                const quint8 type = m_extra.u8();
                switch (type) {
                case 0x00:
                    m_extra.skip(8);
                    out += QStringLiteral("\"\"");
                    break;
                case 0x01: out += numberText(m_extra.f64()); break;
                case 0x02: {
                    QString s;
                    if (m_b8) {
                        const int n = m_extra.u16();
                        const quint8 flags = m_extra.u8();
                        if (flags & 1) {
                            const char *p = m_extra.take(qint64(n) * 2);
                            if (p) s = QString::fromUtf16(reinterpret_cast<const char16_t *>(p), n);
                        } else {
                            const char *p = m_extra.take(n);
                            if (p) s = QString::fromLatin1(p, n);
                        }
                    } else {
                        const int n = m_extra.u8();
                        const char *p = m_extra.take(n);
                        if (p) s = m_cx.bytesToText ? m_cx.bytesToText(p, n) : QString::fromLatin1(p, n);
                    }
                    s.replace(QLatin1Char('"'), QLatin1String("\"\""));
                    out += QLatin1Char('"') + s + QLatin1Char('"');
                    break;
                }
                case 0x04: {
                    const quint8 v = m_extra.u8();
                    m_extra.skip(7);
                    out += v ? QStringLiteral("TRUE") : QStringLiteral("FALSE");
                    break;
                }
                case 0x10: {
                    const quint8 code = m_extra.u8();
                    m_extra.skip(7);
                    const QString e = xlsErrorText(code);
                    out += e.isEmpty() ? QStringLiteral("#VALUE!") : e;
                    break;
                }
                default: return QString();
                }
                if (m_extra.bad()) return QString();
                if (out.size() > m_opt.maxChars) return QString();
            }
        }
        return out + QLatin1Char('}');
    }

    QString walk()
    {
        static const char *const binary[] = {"+", "-", "*", "/", "^", "&", "<", "<=", "=", ">=", ">", "<>", " ", ",", ":"};   // 0x03 to 0x11
        int steps = 0;
        while (m_in.left() > 0) {
            if (++steps > 1000000) return bad();
            const quint8 ptg = m_in.u8();
            if (ptg < 0x20) {
                switch (ptg) {
                case 0x01:
                case 0x02: return QCoreApplication::translate("Sheet", "A formula is one that other cells share, and is read where they are.");
                case 0x15: {
                    qint32 a;
                    Node n;
                    n.kind = NodeKind::Paren;
                    if (!pop(&a)) return bad();
                    n.a = a;
                    if (!push(make(std::move(n), 2))) return bad();
                    continue;
                }
                case 0x16:
                    if (!pushLit(QString())) return bad();
                    continue;
                case 0x17: {
                    QString s;
                    if (m_b8) {
                        const int n = m_in.u8();
                        const quint8 flags = m_in.u8();
                        if (flags & 1) {
                            const char *p = m_in.take(qint64(n) * 2);
                            if (p) s = QString::fromUtf16(reinterpret_cast<const char16_t *>(p), n);
                        } else {
                            const char *p = m_in.take(n);
                            if (p) s = QString::fromLatin1(p, n);
                        }
                    } else {
                        const int n = m_in.u8();
                        const char *p = m_in.take(n);
                        if (p) s = m_cx.bytesToText ? m_cx.bytesToText(p, n) : QString::fromLatin1(p, n);
                    }
                    s.replace(QLatin1Char('"'), QLatin1String("\"\""));
                    if (m_in.bad() || !pushLit(QLatin1Char('"') + s + QLatin1Char('"'))) return bad();
                    continue;
                }
                case 0x19: {
                    const quint8 grbit = m_in.u8();
                    if (grbit & 0x04) {
                        const int count = m_in.u16();
                        m_in.skip((qint64(count) + 1) * 2);
                    } else {
                        m_in.skip(2);
                    }
                    if (m_in.bad()) return bad();
                    if ((grbit & 0x10) && !(grbit & 0x04)) {
                        qint32 a;
                        if (!pop(&a)) return bad();
                        Node n;
                        n.kind = NodeKind::Call;
                        n.text = QStringLiteral("SUM");
                        n.first = qint32(m_kids.size());
                        n.count = 1;
                        m_kids.push_back(a);
                        if (!push(make(std::move(n), 5))) return bad();
                    }
                    continue;
                }
                case 0x1C: {
                    const QString e = xlsErrorText(m_in.u8());
                    if (m_in.bad() || !pushLit(e.isEmpty() ? QStringLiteral("#VALUE!") : e)) return bad();
                    continue;
                }
                case 0x1D: {
                    const quint8 v = m_in.u8();
                    if (m_in.bad() || !pushLit(v ? QStringLiteral("TRUE") : QStringLiteral("FALSE"))) return bad();
                    continue;
                }
                case 0x1E: {
                    const quint16 v = m_in.u16();
                    if (m_in.bad() || !pushLit(QString::number(v))) return bad();
                    continue;
                }
                case 0x1F: {
                    const double v = m_in.f64();
                    if (m_in.bad() || !pushLit(numberText(v))) return bad();
                    continue;
                }
                case 0x12:
                case 0x13: {
                    qint32 a;
                    if (!pop(&a) || !pushUnary(ptg == 0x12 ? QStringLiteral("+") : QStringLiteral("-"), true, a)) return bad();
                    continue;
                }
                case 0x14: {
                    qint32 a;
                    if (!pop(&a) || !pushUnary(QStringLiteral("%"), false, a)) return bad();
                    continue;
                }
                default:
                    if (ptg >= 0x03 && ptg <= 0x11) {
                        qint32 a, b;
                        if (!pop(&b) || !pop(&a)) return bad();
                        Node n;
                        n.kind = NodeKind::Bin;
                        n.text = QString::fromLatin1(binary[ptg - 3]);
                        n.a = a;
                        n.b = b;
                        if (!push(make(std::move(n), n.text.size()))) return bad();
                        continue;
                    }
                    return bad();
                }
            }
            if (ptg >= 0x80) return bad();
            const quint8 id = quint8((ptg & 0x1F) | 0x20);
            switch (id) {
            case 0x20: {   // an array constant: the token holds nothing but room; the values follow the formula
                m_in.skip(7);
                const QString a = arrayConstant();
                if (m_in.bad() || a.isEmpty() || !pushLit(a)) return bad();
                break;
            }
            case 0x21:
            case 0x22: {
                int argc;
                int tab;
                if (id == 0x21) {
                    tab = m_in.u16();
                    const XlsFunction f = xlsFunction(tab);
                    if (!f.name || f.fixedArgs < 0) return bad();
                    argc = f.fixedArgs;
                } else {
                    argc = m_in.u8() & 0x7F;
                    tab = m_in.u16();
                    if (tab & 0x8000) return bad();
                }
                if (m_in.bad()) return bad();
                const XlsFunction f = xlsFunction(tab);
                if (!f.name) return bad();
                if (argc > m_stack.size()) return bad();
                Node n;
                n.kind = NodeKind::Call;
                // (the arguments come off the stack last first)
                QVector<qint32> args(argc);
                for (int i = argc - 1; i >= 0; --i) pop(&args[i]);
                int from = 0;
                if (tab == 255) {
                    if (args.isEmpty()) return bad();
                    n.a = args[0];   // the name of the function is the first of the arguments
                    from = 1;
                } else {
                    n.text = QString::fromLatin1(f.name);
                }
                n.first = qint32(m_kids.size());
                n.count = qint32(argc - from);
                for (int i = from; i < argc; ++i) m_kids.push_back(args[i]);
                if (!push(make(std::move(n), 2 + (argc > 1 ? argc - 1 : 0) + (tab == 255 ? 0 : 20)))) return bad();
                break;
            }
            case 0x23: {
                const int index = m_in.u16();
                m_in.skip(m_b8 ? 2 : m_cx.biff >= 5 ? 12 : 5);
                const QString n = nameText(index);
                if (m_in.bad() || n.isEmpty() || !pushLit(n)) return bad();
                break;
            }
            case 0x24:
            case 0x2C: {   // a cell
                quint32 rowField, colField;
                bool rowRel, colRel;
                if (m_b8) {
                    rowField = m_in.u16();
                    const quint16 c = m_in.u16();
                    colField = c & 0x3FFF;
                    colRel = c & 0x4000;
                    rowRel = c & 0x8000;
                    if (m_opt.relative && colRel) colField = c & 0xFF;
                } else {
                    const quint16 r = m_in.u16();
                    colField = m_in.u8();
                    rowField = r & 0x3FFF;
                    colRel = r & 0x4000;
                    rowRel = r & 0x8000;
                }
                if (m_in.bad() || !pushRef(QString(), false, rowField, colField, rowRel, colRel, 0, 0, false, false)) return bad();
                break;
            }
            case 0x25:
            case 0x2D: {   // an area
                quint32 r1f, r2f, c1f, c2f;
                bool r1Rel, r2Rel, c1Rel, c2Rel;
                if (m_b8) {
                    r1f = m_in.u16();
                    r2f = m_in.u16();
                    const quint16 c1 = m_in.u16(), c2 = m_in.u16();
                    c1f = c1 & 0x3FFF;
                    c2f = c2 & 0x3FFF;
                    c1Rel = c1 & 0x4000;
                    r1Rel = c1 & 0x8000;
                    c2Rel = c2 & 0x4000;
                    r2Rel = c2 & 0x8000;
                    if (m_opt.relative) {
                        if (c1Rel) c1f = c1 & 0xFF;
                        if (c2Rel) c2f = c2 & 0xFF;
                    }
                } else {
                    const quint16 r1 = m_in.u16(), r2 = m_in.u16();
                    c1f = m_in.u8();
                    c2f = m_in.u8();
                    r1f = r1 & 0x3FFF;
                    r2f = r2 & 0x3FFF;
                    c1Rel = r1 & 0x4000;
                    r1Rel = r1 & 0x8000;
                    c2Rel = r2 & 0x4000;
                    r2Rel = r2 & 0x8000;
                }
                if (m_in.bad() || !pushRef(QString(), true, r1f, c1f, r1Rel, c1Rel, r2f, c2f, r2Rel, c2Rel)) return bad();
                break;
            }
            case 0x26:
            case 0x27:
            case 0x28:
                // (memory tokens remember the cells of the expression that follows; the expression itself is in the tokens)
                m_in.skip(6);
                if (id == 0x26) skipRemembered();
                if (m_in.bad()) return bad();
                break;
            case 0x29:
            case 0x2E:
            case 0x2F:
                m_in.skip(2);
                if (m_in.bad()) return bad();
                break;
            case 0x2A: {   // a cell that is gone
                m_in.skip(m_b8 ? 4 : 3);
                if (m_in.bad() || !pushLit(QStringLiteral("#REF!"))) return bad();
                break;
            }
            case 0x2B: {
                m_in.skip(m_b8 ? 8 : 6);
                if (m_in.bad() || !pushLit(QStringLiteral("#REF!"))) return bad();
                break;
            }
            case 0x39: {   // a name in another sheet or book, or an add-in function
                if (!m_b8) {
                    // (a negative number is a name of this workbook; else an external name, counted in the order of their records)
                    const int iw = qint16(m_in.u16());
                    m_in.skip(8);
                    const int index = m_in.u16();
                    m_in.skip(12);
                    const QString n = iw < 0 ? nameText(index) : index >= 1 && index <= m_cx.externNames5.size() ? m_cx.externNames5[index - 1] : QString();
                    if (m_in.bad() || n.isEmpty() || !pushLit(n)) return bad();
                    break;
                }
                const int ixti = m_in.u16();
                const int index = m_in.u16();
                m_in.skip(2);
                if (m_in.bad() || ixti >= m_cx.xti.size() || m_cx.xti[ixti].supbook >= m_cx.supbooks.size()) return bad();
                const XlsXti &x = m_cx.xti[ixti];
                const XlsSupBook &book = m_cx.supbooks[x.supbook];
                QString text;
                if (book.kind == XlsSupBook::Kind::AddIn) {
                    if (index < 1 || index > book.names.size()) return bad();
                    text = book.names[index - 1];
                } else if (book.kind == XlsSupBook::Kind::External) {
                    if (index < 1 || index > book.names.size()) return bad();
                    bool deleted;
                    text = sheetPart(ixti, &deleted);
                    if (deleted) return bad();
                    if (text.isEmpty()) text = QStringLiteral("[") + QString::number(book.ordinal) + QStringLiteral("]!");
                    text += book.names[index - 1];
                } else {
                    text = nameText(index);
                    if (text.isEmpty()) return bad();
                }
                if (!pushLit(text)) return bad();
                break;
            }
            case 0x3A:
            case 0x3B: {   // a cell or an area on a sheet
                const bool area = id == 0x3B;
                QString sheet;
                bool deleted = false;
                quint32 r1f, r2f = 0, c1f, c2f = 0;
                bool r1Rel, r2Rel = false, c1Rel, c2Rel = false;
                if (m_b8) {
                    const int ixti = m_in.u16();
                    sheet = sheetPart(ixti, &deleted);
                    r1f = m_in.u16();
                    if (area) r2f = m_in.u16();
                    const quint16 c1 = m_in.u16();
                    quint16 c2 = 0;
                    if (area) c2 = m_in.u16();
                    c1f = c1 & 0x3FFF;
                    c2f = c2 & 0x3FFF;
                    c1Rel = c1 & 0x4000;
                    r1Rel = c1 & 0x8000;
                    c2Rel = c2 & 0x4000;
                    r2Rel = c2 & 0x8000;
                    if (m_opt.relative) {
                        if (c1Rel) c1f = c1 & 0xFF;
                        if (c2Rel) c2f = c2 & 0xFF;
                    }
                } else {
                    const int iw = qint16(m_in.u16());
                    m_in.skip(8);
                    const int first = m_in.u16(), last = m_in.u16();
                    sheet = sheetPart5(iw, first, last, &deleted);
                    const quint16 r1 = m_in.u16();
                    quint16 r2 = 0;
                    if (area) r2 = m_in.u16();
                    c1f = m_in.u8();
                    if (area) c2f = m_in.u8();
                    r1f = r1 & 0x3FFF;
                    r2f = r2 & 0x3FFF;
                    c1Rel = r1 & 0x4000;
                    r1Rel = r1 & 0x8000;
                    c2Rel = r2 & 0x4000;
                    r2Rel = r2 & 0x8000;
                }
                if (m_in.bad()) return bad();
                if (deleted) {
                    if (!pushLit(QStringLiteral("#REF!"))) return bad();
                    break;
                }
                if (!pushRef(sheet, area, r1f, c1f, r1Rel, c1Rel, r2f, c2f, r2Rel, c2Rel)) return bad();
                break;
            }
            case 0x3C:
            case 0x3D: {
                if (m_b8) {
                    m_in.skip(id == 0x3C ? 6 : 10);
                } else {
                    m_in.skip(id == 0x3C ? 17 : 20);
                }
                if (m_in.bad() || !pushLit(QStringLiteral("#REF!"))) return bad();
                break;
            }
            default: return bad();
            }
        }
        if (m_in.bad()) return bad();
        return QString();
    }

    // The cells a constant-reference memory token remembers follow the formula: a count and that many areas of eight bytes.
    void skipRemembered()
    {
        if (!m_b8) return;
        const int count = m_extra.u16();
        m_extra.skip(qint64(count) * 8);
    }
};

} // namespace

XlsFormulaTree xlsParseFormula(const XlsContext &context, const QByteArray &rgce, const QByteArray &extra, const XlsFormulaOptions &options)
{
    return Converter(context, rgce, extra, options).run();
}

XlsFormulaResult xlsFormulaText(const XlsContext &context, const QByteArray &rgce, const QByteArray &extra, const XlsFormulaOptions &options)
{
    const XlsFormulaTree tree = xlsParseFormula(context, rgce, extra, options);
    if (!tree.ok()) {
        XlsFormulaResult r;
        r.why = tree.why().isEmpty() ? QCoreApplication::translate("Sheet", "A formula's tokens do not make one value.") : tree.why();
        return r;
    }
    return tree.render(options.baseRow, options.baseCol);
}

} // namespace jp::office::sheet
