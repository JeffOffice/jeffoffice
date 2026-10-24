#include "sheet/xlsformula.h"

#include "sheet/cellref.h"
#include "sheet/xlsbytes.h"
#include "sheet/xlsfunctions.h"

#include <QCoreApplication>

#include <algorithm>
#include <cmath>

namespace jp::office::sheet {

namespace {

QString translate(const char *text) { return QCoreApplication::translate("Sheet", text); }

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

namespace {

class Converter {
public:
    Converter(const XlsContext &cx, const QByteArray &rgce, const QByteArray &extra, const XlsFormulaOptions &opt)
        : m_cx(cx), m_opt(opt), m_in(rgce), m_extra(extra), m_b8(cx.biff >= 8), m_maxRow(cx.biff >= 8 ? 65535u : 16383u)
    {
    }

    XlsFormulaResult run()
    {
        XlsFormulaResult res;
        const QString why = walk();
        if (!why.isEmpty()) {
            res.why = why;
        } else if (m_stack.size() != 1) {
            res.why = translate("A formula's tokens do not make one value.");
        } else {
            res.text = m_stack.last();
        }
        res.outside = m_outside;
        return res;
    }

private:
    const XlsContext &m_cx;
    const XlsFormulaOptions &m_opt;
    Bytes m_in, m_extra;
    bool m_b8;
    quint32 m_maxRow;
    QVector<QString> m_stack;
    qint64 m_chars = 0;
    bool m_outside = false;

    QString bad() const { return translate("A formula has a token this program does not read."); }

    bool push(const QString &s)
    {
        m_chars += s.size();
        if (m_chars > m_opt.maxChars) return false;
        m_stack.append(s);
        return true;
    }
    bool pop(QString *s)
    {
        if (m_stack.isEmpty()) return false;
        *s = m_stack.takeLast();
        m_chars -= s->size();
        return true;
    }

    // The row and column as a reference is written, from the fields as stored.
    bool place(quint32 rowField, quint32 colField, bool rowRel, bool colRel, quint32 *row, quint32 *col)
    {
        if (!m_opt.relative) {
            *row = rowField;
            *col = colField;
            return true;
        }
        qint64 r = rowField, c = colField;
        if (rowRel) {
            const qint64 off = m_b8 ? qint16(rowField) : qint64(rowField & 0x2000 ? qint64(rowField) - 0x4000 : qint64(rowField));
            r = qint64(m_opt.baseRow) + off;
            const qint64 rows = m_opt.name ? qint64(kMaxRows) : qint64(m_maxRow) + 1;
            if (r < 0 || r >= rows) m_outside = true;
            r = ((r % rows) + rows) % rows;
        }
        if (colRel) {
            const qint64 off = qint8(colField & 0xFF);
            c = qint64(m_opt.baseCol) + off;
            const qint64 cols = m_opt.name ? qint64(kMaxColumns) : 256;
            if (c < 0 || c >= cols) m_outside = true;
            c = ((c % cols) + cols) % cols;
        }
        *row = quint32(r);
        *col = quint32(c);
        return true;
    }

    static QString cellText(quint32 row, quint32 col, bool rowAbs, bool colAbs)
    {
        return QString(colAbs ? QStringLiteral("$") : QString()) + columnName(col) + (rowAbs ? QStringLiteral("$") : QString()) + QString::number(qint64(row) + 1);
    }

    QString areaText(quint32 row1, quint32 col1, quint32 row2, quint32 col2, bool r1Abs, bool c1Abs, bool r2Abs, bool c2Abs) const
    {
        const bool allRows = row1 == 0 && row2 >= m_maxRow;
        const bool allCols = col1 == 0 && col2 >= 255;
        if (allRows)
            return QString(c1Abs ? QStringLiteral("$") : QString()) + columnName(col1) + QLatin1Char(':') + (c2Abs ? QStringLiteral("$") : QString()) + columnName(col2);
        if (allCols)
            return QString(r1Abs ? QStringLiteral("$") : QString()) + QString::number(qint64(row1) + 1) + QLatin1Char(':') + (r2Abs ? QStringLiteral("$") : QString())
                + QString::number(qint64(row2) + 1);
        return cellText(row1, col1, r1Abs, c1Abs) + QLatin1Char(':') + cellText(row2, col2, r2Abs, c2Abs);
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
        int ordinal = 1;
        for (int i = 0; i < iw - 1; ++i) {
            const QByteArray &e = m_cx.externSheet5[i];
            if (!e.isEmpty() && quint8(e[0]) != 0x03 && !(e.size() == 2 && quint8(e[0]) == 0x01)) ++ordinal;
        }
        const QString book = QStringLiteral("[") + QString::number(ordinal) + QLatin1Char(']');
        return (needsQuotes(sheet) ? quoted(book + sheet) : book + sheet) + QLatin1Char('!');
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
                case 0x02: return translate("A formula is one that other cells share, and is read where they are.");
                case 0x15: {
                    QString a;
                    if (!pop(&a) || !push(QLatin1Char('(') + a + QLatin1Char(')'))) return bad();
                    continue;
                }
                case 0x16:
                    if (!push(QString())) return bad();
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
                    if (m_in.bad() || !push(QLatin1Char('"') + s + QLatin1Char('"'))) return bad();
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
                        QString a;
                        if (!pop(&a) || !push(QStringLiteral("SUM(") + a + QLatin1Char(')'))) return bad();
                    }
                    continue;
                }
                case 0x1C: {
                    const QString e = xlsErrorText(m_in.u8());
                    if (m_in.bad() || !push(e.isEmpty() ? QStringLiteral("#VALUE!") : e)) return bad();
                    continue;
                }
                case 0x1D: {
                    const quint8 v = m_in.u8();
                    if (m_in.bad() || !push(v ? QStringLiteral("TRUE") : QStringLiteral("FALSE"))) return bad();
                    continue;
                }
                case 0x1E: {
                    const quint16 v = m_in.u16();
                    if (m_in.bad() || !push(QString::number(v))) return bad();
                    continue;
                }
                case 0x1F: {
                    const double v = m_in.f64();
                    if (m_in.bad() || !push(numberText(v))) return bad();
                    continue;
                }
                case 0x12:
                case 0x13: {
                    QString a;
                    if (!pop(&a) || !push((ptg == 0x12 ? QStringLiteral("+") : QStringLiteral("-")) + a)) return bad();
                    continue;
                }
                case 0x14: {
                    QString a;
                    if (!pop(&a) || !push(a + QLatin1Char('%'))) return bad();
                    continue;
                }
                default:
                    if (ptg >= 0x03 && ptg <= 0x11) {
                        QString a, b;
                        if (!pop(&b) || !pop(&a) || !push(a + QLatin1String(binary[ptg - 3]) + b)) return bad();
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
                if (m_in.bad() || a.isEmpty() || !push(a)) return bad();
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
                QStringList args;
                for (int i = 0; i < argc; ++i) {
                    QString a;
                    pop(&a);
                    args.prepend(a);
                }
                QString text;
                if (tab == 255) {
                    if (args.isEmpty()) return bad();
                    const QString callee = args.takeFirst();   // (named first: the order the parts of one sum are worked out in is not set)
                    text = callee + QLatin1Char('(') + args.join(QLatin1Char(',')) + QLatin1Char(')');
                } else {
                    text = QString::fromLatin1(f.name) + QLatin1Char('(') + args.join(QLatin1Char(',')) + QLatin1Char(')');
                }
                if (!push(text)) return bad();
                break;
            }
            case 0x23: {
                const int index = m_in.u16();
                m_in.skip(m_b8 ? 2 : 12);
                const QString n = nameText(index);
                if (m_in.bad() || n.isEmpty() || !push(n)) return bad();
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
                quint32 row, col;
                place(rowField, colField, rowRel, colRel, &row, &col);
                if (m_in.bad() || !push(cellText(row, col, !rowRel, !colRel))) return bad();
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
                quint32 row1, col1, row2, col2;
                place(r1f, c1f, r1Rel, c1Rel, &row1, &col1);
                place(r2f, c2f, r2Rel, c2Rel, &row2, &col2);
                if (m_in.bad() || !push(areaText(row1, col1, row2, col2, !r1Rel, !c1Rel, !r2Rel, !c2Rel))) return bad();
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
                if (m_in.bad() || !push(QStringLiteral("#REF!"))) return bad();
                break;
            }
            case 0x2B: {
                m_in.skip(m_b8 ? 8 : 6);
                if (m_in.bad() || !push(QStringLiteral("#REF!"))) return bad();
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
                    if (m_in.bad() || n.isEmpty() || !push(n)) return bad();
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
                if (!push(text)) return bad();
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
                    if (!push(QStringLiteral("#REF!"))) return bad();
                    break;
                }
                quint32 row1, col1;
                place(r1f, c1f, r1Rel, c1Rel, &row1, &col1);
                QString ref;
                if (area) {
                    quint32 row2, col2;
                    place(r2f, c2f, r2Rel, c2Rel, &row2, &col2);
                    ref = areaText(row1, col1, row2, col2, !r1Rel, !c1Rel, !r2Rel, !c2Rel);
                } else {
                    ref = cellText(row1, col1, !r1Rel, !c1Rel);
                }
                if (!push(sheet + ref)) return bad();
                break;
            }
            case 0x3C:
            case 0x3D: {
                if (m_b8) {
                    m_in.skip(id == 0x3C ? 6 : 10);
                } else {
                    m_in.skip(id == 0x3C ? 17 : 20);
                }
                if (m_in.bad() || !push(QStringLiteral("#REF!"))) return bad();
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

XlsFormulaResult xlsFormulaText(const XlsContext &context, const QByteArray &rgce, const QByteArray &extra, const XlsFormulaOptions &options)
{
    return Converter(context, rgce, extra, options).run();
}

} // namespace jp::office::sheet
