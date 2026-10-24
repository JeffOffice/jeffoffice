// The comparison of two readings of one workbook, for the yardstick of the old binary format (docs/jeffcrunch.md, "`.xls`"): what the reader gets from a workbook in the old
// form, and what it gets from the same workbook saved in the XML form by another program.
//   jcrunchtool compare-one <ours.xls> <theirs.xlsx>   every cell of the two, side by side. Lines (tab-separated):
//     `d`  sheet, cell, kind, ours, theirs      a difference: kind V a constant's value, W a formula's value, F a formula's text, N a number format, A a cell only ours has,
//                                               B a cell only theirs has (a few of each kind: the first 60)
//     `s`  the counts: sheets of ours, theirs, whether their names agree, cells with a value or a formula in either, in both, only in ours, only in theirs, constants that
//          have the same value, constants that differ, formula cells in both with the same value, with different values, with the same text, with different texts, cells with
//          a value in both whose number format shows the same, whose format shows otherwise
//   Values are the same when numbers agree to 15 digits, texts are equal, and the booleans and errors are the same; a formula's text is compared after the spaces are gone, the
//   case is folded outside quotes, quotes a sheet name does not need are gone, and the prefixes newer functions carry in the XML form are dropped; two number formats are the same
//   when they show a set of numbers (and a text) alike.

#include "calc/numfmt.h"
#include "sheet/cellref.h"
#include "sheet/sheetreader.h"

#include <QHash>
#include <QRegularExpression>
#include <QSet>
#include <QTextStream>

#include <cmath>

using namespace jp::office::sheet;

namespace {

QTextStream &out()
{
    static QTextStream s(stdout);
    return s;
}

struct Info {
    CellType type = CellType::Empty;
    double number = 0;
    QString text;
    bool formula = false;
    QString ftext;
    quint32 style = 0;
};

QString sameDigits(double v)
{
    if (v == 0) return QStringLiteral("0");   // (and minus zero)
    return QString::number(v, 'g', 15);
}

QString valueText(const Info &i)
{
    switch (i.type) {
    case CellType::Empty: return QString();
    case CellType::Number:
    case CellType::Date: return QStringLiteral("n:") + sameDigits(i.number);
    case CellType::Bool: return QStringLiteral("b:") + QString::number(int(i.number));
    case CellType::Error: return QStringLiteral("e:") + errorText(CellError(int(i.number)));
    default: return QStringLiteral("s:") + i.text;
    }
}

Info infoOf(const Workbook &book, const Sheet &sheet, const Cell &c)
{
    Info i;
    i.type = c.type;
    i.number = c.number;
    i.style = c.style;
    if (c.type == CellType::SharedString || c.type == CellType::InlineString || c.type == CellType::FormulaString) i.text = sheet.text(c, book.sharedStrings());
    if (c.type == CellType::SharedString || c.type == CellType::InlineString || c.type == CellType::FormulaString) {
        if (i.text.isEmpty() && !c.hasFormula()) i.type = CellType::Empty;   // (an empty text is no value)
    }
    i.formula = c.hasFormula();
    if (i.formula) i.ftext = sheet.formulaText(c);
    return i;
}

// Whether two numbers differ by no more than two units of the 15th significant digit: the XML form that LibreOffice writes holds only 15 digits, rounded as it rounds them,
// so a number that has more is not the number that was read from the old form in its last digit.
bool nearNumbers(const Info &a, const Info &b)
{
    const bool na = a.type == CellType::Number || a.type == CellType::Date, nb = b.type == CellType::Number || b.type == CellType::Date;
    if (!na || !nb) return false;
    const double big = std::max(std::abs(a.number), std::abs(b.number));
    if (big == 0) return true;
    const double unit = std::pow(10.0, std::floor(std::log10(big)) - 14);
    return std::abs(a.number - b.number) <= 2.01 * unit;
}

bool sameValue(const Info &a, const Info &b)
{
    const bool ea = a.type == CellType::Empty || (a.formula && a.type == CellType::FormulaString && a.text.isEmpty());
    const bool eb = b.type == CellType::Empty || (b.formula && b.type == CellType::FormulaString && b.text.isEmpty());
    if (ea && eb) return true;
    // a formula whose text result is empty and a formula with no value stored are the same here
    return valueText(a) == valueText(b);
}

// A formula's text with what does not matter taken out.
QString normalizeFormula(const QString &f)
{
    QString out;
    bool quoted = false;
    bool sheetQuoted = false;
    for (int i = 0; i < f.size(); ++i) {
        const QChar c = f[i];
        if (quoted) {
            out += c;
            if (c == QLatin1Char('"')) {
                if (i + 1 < f.size() && f[i + 1] == QLatin1Char('"')) {
                    out += f[++i];
                } else {
                    quoted = false;
                }
            }
            continue;
        }
        if (sheetQuoted) {
            out += c.toUpper();   // (the name of a sheet is of no case)
            if (c == QLatin1Char('\'')) {
                if (i + 1 < f.size() && f[i + 1] == QLatin1Char('\'')) out += f[++i];
                else sheetQuoted = false;
            }
            continue;
        }
        if (c == QLatin1Char('"')) {
            quoted = true;
            out += c;
        } else if (c == QLatin1Char('\'')) {
            sheetQuoted = true;
            out += c;
        } else if (!c.isSpace()) {
            out += c.toUpper();
        }
    }
    // quotes round a sheet's name (compared without them: a name that looks like a cell needs them, and the other program leaves some out)
    static const QRegularExpression quotedName(QStringLiteral("'((?:[^']|'')+)'!"));
    out.replace(quotedName, QStringLiteral("\\1!"));
    // a range of one cell is that cell
    static const QRegularExpression oneCell(QStringLiteral("(\\$?[A-Z]{1,3}\\$?[0-9]+):\\1(?![0-9A-Z])"));
    out.replace(oneCell, QStringLiteral("\\1"));
    out.remove(QStringLiteral("_XLFN."));
    out.remove(QStringLiteral("_XLWS."));
    // TRUE() and FALSE() are the constants, and NA() is #N/A, in the XML form that LibreOffice writes
    out.replace(QStringLiteral("TRUE()"), QStringLiteral("TRUE"));
    out.replace(QStringLiteral("FALSE()"), QStringLiteral("FALSE"));
    out.replace(QStringLiteral("NA()"), QStringLiteral("#N/A"));
    // numbers: 1.0 and 1 are one
    static const QRegularExpression number(QStringLiteral("(?<![A-Za-z0-9_$.\\[!'])([0-9]+\\.?[0-9]*(?:E[+-]?[0-9]+)?)(?![A-Za-z0-9_(!'])"));
    QString result;
    int from = 0;
    auto it = number.globalMatch(out);
    while (it.hasNext()) {
        const auto m = it.next();
        // (only inside no quotes: a match inside a string would be changed too, which is the same on both sides)
        result += QStringView(out).mid(from, m.capturedStart() - from);
        bool ok = false;
        const double v = m.captured(1).toDouble(&ok);
        result += ok ? QString::number(v, 'g', 15).toUpper() : m.captured(1);
        from = int(m.capturedEnd());
    }
    result += QStringView(out).mid(from);
    return result;
}

// Whether two number formats show the same: a set of numbers and a text, in each.
bool sameFormat(const QString &a, const QString &b)
{
    if (a == b) return true;
    static const double samples[] = {0, 1, -1, 0.5, -0.25, 12.345, 1234.5678, -98765.4321, 0.000123, 40000.25, 36526, 123456789.123, 1e-7, 0.99999, 2.5, 45366.75};
    const jp::calc::NumberFormat fa(a), fb(b);
    for (double v : samples) {
        for (jp::calc::DateSystem system : {jp::calc::DateSystem::Date1900}) {
            const jp::calc::Formatted ra = fa.format(v, system, 0), rb = fb.format(v, system, 0);
            if (ra.text != rb.text) return false;
        }
    }
    return fa.formatText(QStringLiteral("abc")).text == fb.formatText(QStringLiteral("abc")).text;
}

} // namespace

namespace jp::crunchtool {

int compareMain(const QStringList &args)
{
    if (args.size() != 2) return -1;
    Workbook a, b;
    QString why;
    if (!a.openFile(args[0], &why)) {
        out() << "!ours\t" << why << "\n";
        return 1;
    }
    if (!b.openFile(args[1], &why)) {
        out() << "!theirs\t" << why << "\n";
        return 1;
    }
    const int sheets = int(std::min(a.info().sheets.size(), b.info().sheets.size()));
    bool namesAgree = a.info().sheets.size() == b.info().sheets.size();
    for (int i = 0; i < sheets; ++i)
        if (a.info().sheets[i].name != b.info().sheets[i].name) namesAgree = false;
    qint64 inEither = 0, inBoth = 0, onlyOurs = 0, onlyTheirs = 0, constSame = 0, constNear = 0, constDiff = 0, fvalSame = 0, fvalNear = 0, fvalDiff = 0, ftextSame = 0, ftextDiff = 0, fmtSame = 0, fmtDiff = 0;
    QHash<char, int> shownOf;
    auto diff = [&](int sheet, quint32 row, quint32 col, char kind, const QString &ours, const QString &theirs) {
        if (shownOf[kind]++ >= 60) return;
        auto esc = [](QString s) { return s.replace(QLatin1Char('\\'), QLatin1String("\\\\")).replace(QLatin1Char('\t'), QLatin1String("\\t")).replace(QLatin1Char('\n'), QLatin1String("\\n")).replace(QLatin1Char('\r'), QLatin1String("\\r")); };
        out() << "d\t" << esc(a.info().sheets.value(sheet).name) << '\t' << cellName(row, col) << '\t' << kind << '\t' << esc(ours.left(300)) << '\t' << esc(theirs.left(300)) << "\n";
    };
    for (int i = 0; i < sheets; ++i) {
        if (a.info().sheets[i].kind == SheetKind::Chartsheet || b.info().sheets[i].kind == SheetKind::Chartsheet) continue;
        Sheet sa, sb;
        a.readSheet(i, &sa);
        b.readSheet(i, &sb);
        QHash<quint64, Info> theirs;
        sb.cells.forEach([&](const Cell &c) {
            const Info info = infoOf(b, sb, c);
            if (info.type == CellType::Empty && !info.formula) return;
            theirs.insert((quint64(c.row) << 20) | c.col, info);
        });
        QSet<quint64> seen;
        sa.cells.forEach([&](const Cell &c) {
            const Info mine = infoOf(a, sa, c);
            if (mine.type == CellType::Empty && !mine.formula) return;
            const quint64 key = (quint64(c.row) << 20) | c.col;
            ++inEither;
            const auto it = theirs.constFind(key);
            if (it == theirs.cend()) {
                ++onlyOurs;
                diff(i, c.row, c.col, 'A', valueText(mine) + (mine.formula ? QStringLiteral(" =") + mine.ftext : QString()), QString());
                return;
            }
            seen.insert(key);
            ++inBoth;
            const Info &other = it.value();
            const bool anyFormula = mine.formula || other.formula;
            const bool exact = sameValue(mine, other);
            const bool near = !exact && nearNumbers(mine, other);
            if (anyFormula) {
                if (exact) ++fvalSame;
                else if (near) ++fvalNear;
                else {
                    ++fvalDiff;
                    diff(i, c.row, c.col, 'W', valueText(mine), valueText(other));
                }
                const QString na = mine.formula ? normalizeFormula(mine.ftext) : QString(), nb = other.formula ? normalizeFormula(other.ftext) : QString();
                auto isBoolean = [](const QString &n) { return n == QLatin1String("TRUE") || n == QLatin1String("FALSE"); };
                if (mine.formula && other.formula && (na.startsWith(QLatin1String("TABLE(")) || nb.startsWith(QLatin1String("MULTIPLE.OPERATIONS(")))) {
                    ++ftextSame;   // (a data table: the other program writes it as a function of its own)
                } else if ((!mine.formula && isBoolean(nb)) || (!other.formula && isBoolean(na))) {
                    ++ftextSame;   // (the other program writes a true or false it reads as a formula)
                } else if (mine.formula && other.formula) {
                    if (na == nb) ++ftextSame;
                    else {
                        ++ftextDiff;
                        diff(i, c.row, c.col, 'F', mine.ftext, other.ftext);
                    }
                } else {
                    ++ftextDiff;
                    diff(i, c.row, c.col, 'F', mine.formula ? mine.ftext : QStringLiteral("(a value)"), other.formula ? other.ftext : QStringLiteral("(a value)"));
                }
            } else if (exact) {
                ++constSame;
            } else if (near) {
                ++constNear;
            } else {
                ++constDiff;
                diff(i, c.row, c.col, 'V', valueText(mine), valueText(other));
            }
            if (mine.type == CellType::Number || mine.type == CellType::Date || mine.type == CellType::Empty) {
                const QString ca = a.styles().formatCode(mine.style), cb = b.styles().formatCode(other.style);
                if (mine.type == CellType::Empty) return;
                if (sameFormat(ca, cb)) ++fmtSame;
                else {
                    ++fmtDiff;
                    diff(i, c.row, c.col, 'N', ca, cb);
                }
            }
        });
        for (auto it = theirs.cbegin(); it != theirs.cend(); ++it) {
            if (seen.contains(it.key())) continue;
            ++inEither;
            ++onlyTheirs;
            diff(i, quint32(it.key() >> 20), quint32(it.key() & 0xFFFFF), 'B', QString(), valueText(it.value()) + (it.value().formula ? QStringLiteral(" =") + it.value().ftext : QString()));
        }
    }
    out() << "s\t" << a.info().sheets.size() << '\t' << b.info().sheets.size() << '\t' << (namesAgree ? 1 : 0) << '\t' << inEither << '\t' << inBoth << '\t' << onlyOurs << '\t' << onlyTheirs
          << '\t' << constSame << '\t' << constNear << '\t' << constDiff << '\t' << fvalSame << '\t' << fvalNear << '\t' << fvalDiff << '\t' << ftextSame << '\t' << ftextDiff << '\t' << fmtSame << '\t' << fmtDiff << "\n";
    return 0;
}

} // namespace jp::crunchtool
