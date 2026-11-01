// The check of the formulas an .xls copy is made of (docs/jeffcrunch.md):
//   jcrunchtool token-check <file.xls> [show]   reads the formulas of an old binary workbook with this program's reader, writes each one again with the compiler of the copy writer, and compares the tokens
//                                               with the file's own, byte for byte. One line of tab-separated fields: `token-check`, the path, the formulas of the file, those compared, those
//                                               whose tokens are the same, and (with `show`, or for the first) the first that differ, as the text, the file's tokens, and ours.
//   Not compared: a formula that is a pointer to a shared formula or an array, a relative reference of a shared formula, a name, a reference through the list of external sheets, and a formula the
//   compiler refuses. The file's marks for spaces are taken out of its tokens (ours keeps none), and its one-argument SUM written as the sum mark is read as the call of SUM.

#include "io/cfb.h"
#include "sheet/sheetreader.h"
#include "sheet/xlscompile.h"

#include <QFile>
#include <QTextStream>

#include <cstring>

using namespace jp::office;
using namespace jp::office::sheet;

namespace jp::crunchtool {

namespace {

struct Walked {
    bool ok = false;
    bool comparable = true;
    QByteArray tokens;   // the tokens with the marks for spaces taken out
};

// The tokens as they are to be compared; comparable is false for a token that this check does not compare.
Walked normalize(const QByteArray &rg)
{
    Walked w;
    int i = 0;
    const int n = int(rg.size());
    auto u8 = [&](int at) { return quint8(rg[at]); };
    while (i < n) {
        const int p = u8(i);
        int len = 0;
        const int base = p >= 0x20 ? ((p & 0x1F) | 0x20) : p;
        if (p == 0x01 || p == 0x02) { w.comparable = false; len = 5; }
        else if (p >= 0x03 && p <= 0x16) len = 1;
        else if (p == 0x17) {
            if (i + 3 > n) return w;
            len = 3 + u8(i + 1) * ((u8(i + 2) & 1) ? 2 : 1);
        } else if (p == 0x19) {
            if (i + 2 > n) return w;
            const int t = u8(i + 1);
            len = 4;
            if (t == 0x04) {
                if (i + 4 > n) return w;
                len = 4 + 2 * (u8(i + 2) + (u8(i + 3) << 8) + 1);
            }
            if (t == 0x40 || t == 0x41) {   // a mark for spaces: ours has none
                i += len;
                continue;
            }
            if (t == 0x10 && i + len <= n) {   // the sum mark stands for the call of SUM with one argument
                w.tokens.append(char(0x42));
                w.tokens.append(char(1));
                w.tokens.append(char(4));
                w.tokens.append(char(0));
                i += len;
                continue;
            }
        } else if (p == 0x1C || p == 0x1D) len = 2;
        else if (p == 0x1E) len = 3;
        else if (p == 0x1F) len = 9;
        else if (base == 0x20) len = 8;
        else if (base == 0x21) len = 3;
        else if (base == 0x22) len = 4;
        else if (base == 0x24) len = 5;
        else if (base == 0x25) len = 9;
        else if (base == 0x2A) len = 5;
        else if (base == 0x2B) len = 9;
        else {   // names, external references, memory tokens, relative references of shared formulas
            w.comparable = false;
            return w;
        }
        if (i + len > n) return w;
        w.tokens.append(rg.constData() + i, len);
        i += len;
    }
    w.ok = true;
    return w;
}

} // namespace

int tokenCheckMain(const QStringList &args)
{
    if (args.isEmpty() || args.size() > 2) return -1;
    QTextStream out(stdout);
    const bool show = args.size() == 2;
    Workbook book;
    QString why;
    if (!book.openFile(args[0], &why) || !book.isBinary()) {
        out << "token-check\t" << args[0] << "\tnot opened: " << why << "\n";
        return 1;
    }
    jp::cfb::File file;
    QFile f(args[0]);
    if (!f.open(QIODevice::ReadOnly) || !jp::cfb::read(f.readAll(), &file, &why)) {
        out << "token-check\t" << args[0] << "\tnot a compound file\n";
        return 1;
    }
    const QByteArray d = file.stream(QStringLiteral("Workbook"));
    if (d.isEmpty()) {
        out << "token-check\t" << args[0] << "\tno Workbook stream\n";
        return 1;
    }
    // the sheets in the order of the workbook (each worksheet substream follows its BOF, in the order of the BOUNDSHEET records)
    std::vector<int> worksheets;
    for (int i = 0; i < book.info().sheets.size(); ++i)
        if (book.info().sheets[i].kind == SheetKind::Worksheet) worksheets.push_back(i);
    XlsCompileContext cx;
    for (const SheetEntry &e : book.info().sheets) cx.sheetNames << e.name;
    cx.xti = [](int, int) { return 0; };
    qint64 formulas = 0, compared = 0, same = 0;
    QStringList firsts;
    int sheetNo = -1;
    Sheet sheet;
    qsizetype at = 0;
    int depth = 0;
    bool inSheet = false;
    while (at + 4 <= d.size()) {
        const int id = quint8(d[at]) | (quint8(d[at + 1]) << 8), len = quint8(d[at + 2]) | (quint8(d[at + 3]) << 8);
        if (at + 4 + len > d.size()) break;
        const char *b = d.constData() + at + 4;
        if (id == 0x0809) {
            const int type = len >= 4 ? (quint8(b[2]) | (quint8(b[3]) << 8)) : 0;
            ++depth;
            if (depth == 1 && type == 0x0010) {
                ++sheetNo;
                inSheet = sheetNo < int(worksheets.size()) && book.readSheet(worksheets[size_t(sheetNo)], &sheet);
            } else if (depth == 1) {
                inSheet = false;
                if (type != 0x0005) ++sheetNo;
            }
        } else if (id == 0x000A) {
            --depth;
        } else if (id == 0x0006 && inSheet && depth == 1 && len >= 22) {
            ++formulas;
            const int row = quint8(b[0]) | (quint8(b[1]) << 8), col = quint8(b[2]) | (quint8(b[3]) << 8);
            const int cce = quint8(b[20]) | (quint8(b[21]) << 8);
            if (22 + cce <= len) {
                const Walked w = normalize(QByteArray(b + 22, cce));
                const Cell *cell = sheet.cells.find(quint32(row), quint32(col));
                if (w.ok && w.comparable && cell && cell->hasFormula()) {
                    const QString text = sheet.formulaText(*cell);
                    QByteArray rgce, extra;
                    XlsCompileContext c = cx;
                    c.sheet = worksheets[size_t(sheetNo)];
                    QString problem;
                    if (!text.isEmpty() && xlsCompileFormula(text, c, &rgce, &extra, &problem) && extra.isEmpty()) {
                        ++compared;
                        if (rgce == w.tokens) ++same;
                        else if (firsts.size() < (show ? 20 : 1))
                            firsts << QStringLiteral("%1!%2%3: %4 | file %5 | ours %6").arg(book.info().sheets[worksheets[size_t(sheetNo)]].name).arg(columnName(quint32(col))).arg(row + 1).arg(text,
                                                                                                                                                  QString::fromLatin1(w.tokens.toHex(' ')), QString::fromLatin1(rgce.toHex(' ')));
                    }
                }
            }
        }
        at += 4 + len;
    }
    out << "token-check\t" << args[0] << '\t' << formulas << '\t' << compared << '\t' << same;
    for (const QString &s : firsts) out << '\t' << s;
    out << "\n";
    return 0;
}

} // namespace jp::crunchtool
