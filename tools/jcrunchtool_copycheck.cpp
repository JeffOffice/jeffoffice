// The check of a copy of a workbook in another file format (docs/jeffcrunch.md):
//   jcrunchtool copy-check <file> <xls|ods|xlsx>   opens the workbook, writes a copy in that format, reads the copy back with this program's own readers, and compares what the copy holds
//                                                  with what the workbook held, cell by cell: the values (numbers, texts, truth values, errors), the formulas (kept, and written the same),
//                                                  the number formats, and the look of each cell (font, fill, borders, alignment), the merged ranges, and the names. What the format cannot
//                                                  hold (cells past its last row or column) is counted as lost by its limit, not as a difference. One line of tab-separated fields:
//                                                  `copy-check`, the path, the format, the result (ok, or what failed), the bytes of the copy, the sheets, the cells compared, the values
//                                                  that differ, the formulas the workbook had, the formulas the copy has, the formulas whose text is the same, the formulas written as values,
//                                                  the cells whose format was compared, those whose number format is the same, those whose look is the same, the cells lost by a limit, the
//                                                  merged ranges of the workbook and of the copy, the milliseconds the write and the read took, what the copy does not have, and the first
//                                                  difference. (tools/crunch_copy_check.py runs it over a list of workbooks and sums it up.)

#include "office/package.h"
#include "sheet/oldsave.h"
#include "sheet/oldsavecommon.h"
#include "sheet/sheetreader.h"
#include "sheet/xlsxsave.h"

#include <QElapsedTimer>
#include <QFile>
#include <QRegularExpression>
#include <QSet>
#include <QTemporaryDir>
#include <QTextStream>

#include <algorithm>
#include <cmath>

using namespace jp::office;
using namespace jp::office::sheet;

namespace jp::crunchtool {

namespace {

QTextStream &out()
{
    static QTextStream s(stdout);
    return s;
}

enum class Class { None, Number, Text, Bool, Error };

Class classOf(const Cell &c)
{
    switch (c.type) {
    case CellType::Number:
    case CellType::Date: return Class::Number;
    case CellType::SharedString:
    case CellType::InlineString:
    case CellType::FormulaString: return Class::Text;
    case CellType::Bool: return Class::Bool;
    case CellType::Error: return Class::Error;
    case CellType::Empty: return Class::None;
    }
    return Class::None;
}

// An error the old format has no number for is written as #N/A.
int errorKey(int e, bool old)
{
    if (!old) return e;
    switch (CellError(e)) {
    case CellError::Null: case CellError::Div0: case CellError::Value: case CellError::Ref: case CellError::Name: case CellError::Num: case CellError::NA: return e;
    default: return int(CellError::NA);
    }
}

// A number format code with the marks that only quote its text taken out (a backslash, quotes, and the "_" that stands for the space of a character's width): two codes that show the same
// are the same.
QString shownCode(QString code)
{
    // a currency mark in brackets ("[$$-409]") shows as the mark
    code.replace(QRegularExpression(QStringLiteral("\\[\\$([^\\]-]*)(?:-[^\\]]*)?\\]")), QStringLiteral("\\1"));
    QString out;
    for (int i = 0; i < code.size(); ++i) {
        const QChar c = code[i];
        if (c == QLatin1Char('\\') || c == QLatin1Char('"')) continue;
        if (c == QLatin1Char('_') && i + 1 < code.size()) {
            out += QLatin1Char(' ');
            ++i;
            continue;
        }
        out += c;
    }
    out = out.trimmed();
    if (out.endsWith(QLatin1String(";@"))) out.chop(2);   // (a section for texts that shows them as they are)
    return out;
}

// A formula without the spaces between its parts, in capitals outside its texts, and TRUE() as TRUE: two formulas that work the same are the same.
QString shownFormula(const QString &f)
{
    QString out;
    bool quote = false;
    for (const QChar c : f) {
        if (c == QLatin1Char('"')) quote = !quote;
        if (quote || c != QLatin1Char(' ')) out += quote ? c : c.toUpper();
    }
    out.replace(QLatin1String("TRUE()"), QLatin1String("TRUE")).replace(QLatin1String("FALSE()"), QLatin1String("FALSE")).replace(QLatin1String("_XLFN."), QString());
    // (a sheet's name is quoted or not as the form likes)
    QString bare;
    bool inText = false;
    for (const QChar c : out) {
        if (c == QLatin1Char('"')) inText = !inText;
        if (inText || c != QLatin1Char('\'')) bare += c;
    }
    return bare;
}

QString lookKey(const Styles &st, quint32 xf)
{
    const CellFormat &f = st.format(xf);
    const FontSpec &font = st.font(f.font);
    const FillSpec &fill = st.fill(f.fill);
    const BorderSpec &b = st.border(f.border);
    auto rgb = [&](const ColorSpec &c) { return st.resolve(c, 0xFFFFFFFFu); };
    // (the automatic color of a border is black)
    auto side = [&](const BorderSide &s) { return s.style == BorderStyle::None ? QStringLiteral("-") : QString::number(int(s.style)) + QLatin1Char('/') + QString::number(st.resolve(s.color, 0), 16); };
    // (a border style the other format has no name for comes back as the nearest; the key keeps the heavy/light and dashed/solid)
    // (the alignments a format has no name for come back as the nearest it has: centered across, filled, distributed, and justified at the top)
    const int horizontal = f.horizontal == HAlign::CenterContinuous ? int(HAlign::Center) : f.horizontal == HAlign::Fill ? int(HAlign::Left) : f.horizontal == HAlign::Distributed ? int(HAlign::Justify) : int(f.horizontal);
    const int vertical = f.vertical == VAlign::Justify || f.vertical == VAlign::Distributed ? int(VAlign::Top) : int(f.vertical);
    // (a fill whose color is the automatic one is no fill)
    const bool noFill = fill.pattern == PatternKind::None || rgb(fill.fg) == 0xFFFFFFFFu;
    return QStringLiteral("%1|%2|%3%4%5%6|%7|%8|%9|%10|%11|%12|%13|%14|%15|%16|%17")
        .arg(st.fontNameOf(font).section(QLatin1Char(','), 0, 0).toLower()).arg(font.size).arg(int(font.bold)).arg(int(font.italic)).arg(int(font.strike)).arg(int(font.underline != Underline::None))
        .arg(rgb(font.color), 0, 16).arg(int(font.vertAlign)).arg(noFill ? QStringLiteral("-") : QString::number(rgb(fill.fg), 16)).arg(side(b.left)).arg(side(b.right)).arg(side(b.top))
        .arg(side(b.bottom)).arg(horizontal).arg(vertical).arg(int(f.wrap)).arg(int(f.locked));
}

bool openBack(const QByteArray &bytes, const QString &format, Workbook *back, QString *why, QTemporaryDir *dir)
{
    if (format == QLatin1String("xls")) return back->openBinary(bytes, why);
    if (format == QLatin1String("xlsx")) {
        Package p;
        if (!p.readLazy(bytes, why)) return false;
        return back->open(p, why);
    }
    const QString path = dir->filePath(QStringLiteral("copy.ods"));
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly)) return false;
    f.write(bytes);
    f.close();
    return back->openFile(path, why);
}

} // namespace

int copyCheckMain(const QStringList &args)
{
    if (args.size() != 2) return -1;
    const QString path = args[0], format = args[1];
    if (format != QLatin1String("xls") && format != QLatin1String("ods") && format != QLatin1String("xlsx")) return -1;
    QStringList fields;
    auto line = [&](const QString &result, const QStringList &rest = QStringList()) {
        out() << "copy-check\t" << path << '\t' << format << '\t' << result;
        for (const QString &r : rest) out() << '\t' << r;
        out() << "\n";
        out().flush();
    };
    Workbook book;
    QString why;
    if (!book.openFile(path, &why)) {
        line(QStringLiteral("not opened: ") + why);
        return 1;
    }
    QElapsedTimer timer;
    timer.start();
    QByteArray bytes;
    CopyReport report;
    bool ok;
    if (format == QLatin1String("xls")) ok = saveXls(book, &bytes, &report, &why);
    else if (format == QLatin1String("ods")) ok = saveOds(book, &bytes, &report, &why);
    else ok = saveXlsx(book, SaveChanges(), &bytes, &why);
    const qint64 writeMs = timer.elapsed();
    if (!ok) {
        line(QStringLiteral("save failed: ") + why);
        return 3;
    }
    if (const QString keepAs = qEnvironmentVariable("COPY_CHECK_KEEP"); !keepAs.isEmpty()) {   // (the copy, for a look at it)
        QFile kept(keepAs);
        if (kept.open(QIODevice::WriteOnly)) kept.write(bytes);
    }
    timer.restart();
    QTemporaryDir dir;
    Workbook back;
    if (!openBack(bytes, format, &back, &why, &dir)) {
        line(QStringLiteral("the copy does not open: ") + why, {QString::number(bytes.size())});
        return 3;
    }
    // the sheets that were written, in order
    CopyReport scratch;
    std::vector<oldsave::Written> written;
    if (format == QLatin1String("xlsx")) {
        for (int place : book.info().order)
            if (place >= 0 && place < book.info().sheets.size() && !book.info().sheets[place].deleted && book.info().sheets[place].kind == SheetKind::Worksheet) {
                oldsave::Written w;
                w.place = place;
                w.name = book.info().sheets[place].name;
                written.push_back(w);
            }
    } else {
        written = oldsave::sheetsToWrite(book, format == QLatin1String("xls") ? kMaxXlsSheets : kMaxSheets, &scratch);
    }
    // the sheets of the copy that are worksheets, in order
    std::vector<int> backPlaces;
    for (int place : back.info().order)
        if (place >= 0 && place < back.info().sheets.size() && !back.info().sheets[place].deleted && back.info().sheets[place].kind == SheetKind::Worksheet) backPlaces.push_back(place);
    QString firstDiff;
    auto diff = [&](const QString &what) {
        if (firstDiff.isEmpty()) firstDiff = what;
        firstDiff.replace(QLatin1Char('\n'), QLatin1Char(' ')).replace(QLatin1Char('\r'), QLatin1Char(' ')).replace(QLatin1Char('\t'), QLatin1Char(' '));   // (one line, whatever the text of a cell holds)
    };
    const bool old = format == QLatin1String("xls");
    const quint32 maxRow = old ? quint32(65536) : format == QLatin1String("ods") ? quint32(1048576) : quint32(1048576);
    const quint32 maxCol = old ? quint32(256) : quint32(16384);
    qint64 sheets = 0, compared = 0, valueDiffs = 0, formulas = 0, formulasBack = 0, formulasSame = 0, fmt = 0, fmtNum = 0, fmtLook = 0, lost = 0, mergesA = 0, mergesB = 0;
    if (written.size() != backPlaces.size() && !(written.size() == 1 && written[0].place < 0 && backPlaces.size() == 1)) diff(QStringLiteral("sheet count %1 vs %2").arg(written.size()).arg(backPlaces.size()));
    for (size_t i = 0; i < written.size() && i < backPlaces.size(); ++i) {
        if (written[i].place < 0) continue;
        ++sheets;
        Sheet a, b;
        book.readSheet(written[i].place, &a);
        back.readSheet(backPlaces[i], &b);
        // the cells covered by a merged range other than its first are not kept by the formats
        QVector<CellRange> merges;
        for (const CellRange &m : a.merges)
            if (m.valid && m.cellCount() >= 2 && m.row2 < maxRow && m.col2 < maxCol && m.cellCount() <= 1000000) merges.append(m);
        mergesA += merges.size();
        for (const CellRange &m : b.merges)
            if (m.valid && m.cellCount() >= 2) ++mergesB;
        auto coveredByMerge = [&](quint32 row, quint32 col) {
            for (const CellRange &m : merges)
                if (m.contains(row, col) && !(m.row1 == row && m.col1 == col)) return true;
            return false;
        };
        QSet<quint64> arrayCells;
        a.cells.forEach([&](quint32 row, quint32 col, const Cell &c) {
            if (c.flags & (ArrayFormula | DataTable)) {
                const CellRange r = a.formulaRange(c);
                if (r.valid && r.cellCount() <= 100000)
                    for (quint32 rr = r.row1; rr <= r.row2; ++rr)
                        for (quint32 cc = r.col1; cc <= r.col2; ++cc) arrayCells.insert((quint64(rr) << 20) | cc);
            }
            (void)row;
            (void)col;
        });
        a.cells.forEach([&](quint32 row, quint32 col, const Cell &c) {
            const Class ca = classOf(c);
            if (ca == Class::None && !c.hasFormula()) return;
            if (row >= maxRow || col >= maxCol) {
                ++lost;
                return;
            }
            if (coveredByMerge(row, col)) return;
            const Cell *e = b.cells.find(row, col);
            const Class cb = e ? classOf(*e) : Class::None;
            // (an empty text is no value in a document of OpenDocument's kind)
            bool emptyText = false;
            if (ca == Class::Text && format == QLatin1String("ods")) {
                const QString t = a.text(c, book.sharedStrings());
                emptyText = std::all_of(t.cbegin(), t.cend(), [](QChar ch) { return ch.unicode() < 0x20 && ch != QLatin1Char('\t') && ch != QLatin1Char('\n') && ch != QLatin1Char('\r'); });
            }
            if (ca != Class::None && !emptyText) {
                ++compared;
                bool same = false;
                if (ca == cb) {
                    switch (ca) {
                    case Class::Number: same = c.number == e->number || (std::isnan(c.number) && std::isnan(e->number)); break;
                    case Class::Text: {
                        QString x = a.text(c, book.sharedStrings());
                        if (x.size() > kMaxCopyTextChars) x = x.left(kMaxCopyTextChars);
                        if (format == QLatin1String("ods")) {
                            x.replace(QLatin1String("\r\n"), QLatin1String("\n")).replace(QLatin1Char('\r'), QLatin1Char('\n'));   // (a line break is one character there)
                            QString kept;
                            for (const QChar ch : x)
                                if (ch.unicode() >= 0x20 || ch == QLatin1Char('\t') || ch == QLatin1Char('\n')) kept += ch;   // (a document holds no other control character)
                            x = kept;
                        }
                        same = x == b.text(*e, back.sharedStrings());
                        break;
                    }
                    case Class::Bool: same = (c.number != 0) == (e->number != 0); break;
                    case Class::Error: same = errorKey(int(c.number), old) == errorKey(int(e->number), old); break;
                    case Class::None: break;
                    }
                } else if (c.type == CellType::Number && !std::isfinite(c.number) && cb == Class::Error) {
                    same = true;
                }
                if (!same) {
                    ++valueDiffs;
                    auto repr = [](const Sheet &sh, const SharedStrings &ss, const Cell *cell) {
                        if (!cell) return QStringLiteral("none");
                        const Class k = classOf(*cell);
                        return k == Class::Text ? QStringLiteral("\"") + sh.text(*cell, ss).left(40) + QLatin1Char('"') : QString::number(cell->number, 'g', 17);
                    };
                    diff(QStringLiteral("value at %1!%2 (%3 vs %4: %5 vs %6)").arg(written[i].name, cellName(row, col)).arg(int(ca)).arg(int(cb), 0, 10, QLatin1Char(' ')).arg(repr(a, book.sharedStrings(), &c), repr(b, back.sharedStrings(), e)));
                }
                // the formats
                if (e) {
                    ++fmt;
                    if (shownCode(book.styles().formatCode(c.style, QLocale::c())) == shownCode(back.styles().formatCode(e->style, QLocale::c()))) ++fmtNum;
                    else diff(QStringLiteral("number format at %1!%2 (%3 vs %4)").arg(written[i].name, cellName(row, col), book.styles().formatCode(c.style, QLocale::c()), back.styles().formatCode(e->style, QLocale::c())));
                    if (lookKey(book.styles(), c.style) == lookKey(back.styles(), e->style)) ++fmtLook;
                    else diff(QStringLiteral("look at %1!%2 (%3 vs %4)").arg(written[i].name, cellName(row, col), lookKey(book.styles(), c.style), lookKey(back.styles(), e->style)));
                }
            }
            if (c.hasFormula() || arrayCells.contains((quint64(row) << 20) | col)) {
                if (c.hasFormula() && !(c.flags & DataTable)) {
                    ++formulas;
                    if (e && e->hasFormula()) {
                        ++formulasBack;
                        if (shownFormula(a.formulaText(c)) == shownFormula(b.formulaText(*e))) ++formulasSame;
                        else diff(QStringLiteral("formula at %1!%2 (%3 vs %4)").arg(written[i].name, cellName(row, col), a.formulaText(c), b.formulaText(*e)));
                    }
                }
            }
        });
    }
    if (book.info().definedNames.size() != back.info().definedNames.size() && format != QLatin1String("ods")) diff(QStringLiteral("names %1 vs %2").arg(book.info().definedNames.size()).arg(back.info().definedNames.size()));
    const qint64 readMs = timer.elapsed();
    line(firstDiff.isEmpty() ? QStringLiteral("ok") : QStringLiteral("differs"),
         {QString::number(bytes.size()), QString::number(sheets), QString::number(compared), QString::number(valueDiffs), QString::number(formulas), QString::number(formulasBack), QString::number(formulasSame),
          QString::number(report.formulasAsValues), QString::number(fmt), QString::number(fmtNum), QString::number(fmtLook), QString::number(lost), QString::number(mergesA), QString::number(mergesB),
          QString::number(writeMs), QString::number(readMs), report.notKept.join(QStringLiteral("; ")), firstDiff.left(300), report.formulaReasons.join(QStringLiteral("; "))});
    return 0;
}

} // namespace jp::crunchtool
