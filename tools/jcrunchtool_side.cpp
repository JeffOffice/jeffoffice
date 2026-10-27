// The check of saving what a workbook keeps beside its cells: its notes, its links, and its protection (docs/jeffcrunch.md):
//   jcrunchtool side-roundtrip <file.xlsx> [out.xlsx]   opens the workbook, writes it with every note put again, every link taken out and put in again, and the protection of every
//                                              sheet (and of the workbook) made again from what was read, reads the file written, and compares it with the workbook: the notes (their
//                                              places, words, authors, and whether they are shown), the links (cells, address or place, screen tip, shown text), the protection
//                                              settings with the hash of the password as the file has it, and every cell (type, format, text or number, formula). One line of tab-separated
//                                              fields: `side-roundtrip`, the path, the result (ok, or the first differences), the notes, the links, the sheets that are protected,
//                                              whether the workbook is, and the cells. (With a second name, the file written is kept there.)

#include "office/package.h"
#include "sheet/notes.h"
#include "sheet/sheetreader.h"
#include "sheet/xlsxsave.h"

#include <QFile>
#include <QTextStream>

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

QStringList linkLines(const Sheet &s)
{
    QStringList lines;
    for (const LinkSpec &l : s.links) lines << rangeName(l.range) + QLatin1Char('|') + l.target + QLatin1Char('|') + l.location + QLatin1Char('|') + l.tip + QLatin1Char('|') + l.display;
    lines.sort();
    return lines;
}

} // namespace

int sideRoundtripMain(const QStringList &args)
{
    if (args.size() != 1 && args.size() != 2) return -1;
    const QString path = args[0];
    int notes = 0, links = 0, protectedSheets = 0;
    bool bookProtected = false;
    qint64 cells = 0;
    auto line = [&](const QString &result) {
        out() << "side-roundtrip\t" << path << '\t' << result << '\t' << notes << '\t' << links << '\t' << protectedSheets << '\t' << (bookProtected ? 1 : 0) << '\t' << cells << "\n";
        out().flush();
    };
    Workbook book;
    QString why;
    if (!book.openFile(path, &why) || book.isConverted()) {
        line(QStringLiteral("not opened: ") + why);
        return 1;
    }
    SaveChanges changes;
    const int count = int(book.info().sheets.size());
    std::vector<Sheet> before(static_cast<size_t>(count));
    std::vector<std::map<quint64, NoteInfo>> notesBefore(static_cast<size_t>(count));
    for (int i = 0; i < count; ++i) {
        if (book.info().sheets[i].kind != SheetKind::Worksheet || !book.readSheet(i, &before[size_t(i)])) continue;
        const Sheet &s = before[size_t(i)];
        cells += s.cells.cellCount();
        notesBefore[size_t(i)] = readSheetNoteInfos(book, i);
        SheetChanges &sc = changes.sheets[i];
        for (const auto &n : notesBefore[size_t(i)]) {
            ++notes;
            if (n.second.threaded) continue;   // (kept by the file as it is)
            NoteOp op;
            op.row = quint32(n.first >> 32);
            op.col = quint32(n.first & 0xFFFFFFFFu);
            op.author = n.second.author;
            op.text = n.second.text;
            op.shown = n.second.shown;
            sc.noteOps.push_back(op);
        }
        for (const LinkSpec &l : s.links) {
            ++links;
            sc.unlinks.push_back(l.range);
            sc.links.push_back(l);
        }
        if (s.protection.enabled) {
            ++protectedSheets;
            sc.setProtection = true;
            sc.protection = s.protection;
        }
        if (sc.empty()) changes.sheets.erase(i);
    }
    if (book.info().protection.any()) {
        bookProtected = true;
        changes.setWorkbookProtection = true;
        changes.workbookProtection = book.info().protection;
    }
    if (notes == 0 && links == 0 && protectedSheets == 0 && !bookProtected) {
        line(QStringLiteral("none"));
        return 0;
    }
    QByteArray bytes;
    if (!saveXlsx(book, changes, &bytes, &why)) {
        line(QStringLiteral("save failed: ") + why);
        return 3;
    }
    if (args.size() == 2) {
        QFile file(args[1]);
        if (file.open(QIODevice::WriteOnly)) file.write(bytes);
    }
    Package package;
    Workbook back;
    if (!package.readLazy(bytes, &why) || !back.open(package, &why)) {
        line(QStringLiteral("the file written does not open: ") + why);
        return 3;
    }
    QStringList diffs;
    auto add = [&](const QString &what) {
        if (diffs.size() < 4 && !diffs.contains(what)) diffs << what;
    };
    if (back.info().sheets.size() != book.info().sheets.size()) add(QStringLiteral("sheet count"));
    if (!(back.info().protection == book.info().protection)) add(QStringLiteral("workbook protection"));
    for (int i = 0; i < std::min<int>(count, back.info().sheets.size()); ++i) {
        if (book.info().sheets[i].kind != SheetKind::Worksheet) continue;
        const Sheet &x = before[size_t(i)];
        Sheet y;
        if (!back.readSheet(i, &y)) {
            add(QStringLiteral("a sheet does not read"));
            continue;
        }
        if (!(x.protection == y.protection)) add(QStringLiteral("sheet protection"));
        if (linkLines(x) != linkLines(y)) add(QStringLiteral("links"));
        const std::map<quint64, NoteInfo> notesAfter = readSheetNoteInfos(back, i);
        if (notesAfter.size() != notesBefore[size_t(i)].size()) add(QStringLiteral("note count"));
        for (const auto &n : notesBefore[size_t(i)]) {
            const auto it = notesAfter.find(n.first);
            if (it == notesAfter.end()) add(QStringLiteral("a note is missing"));
            else if (!(it->second == n.second)) add(QStringLiteral("a note's words, author, or shown flag"));
        }
        if (x.cells.cellCount() != y.cells.cellCount()) add(QStringLiteral("cell count"));
        x.cells.forEach([&](quint32 cellRow, quint32 cellCol, const Cell &c) {
            const Cell *e = y.cells.find(cellRow, cellCol);
            if (!e) {
                add(QStringLiteral("a cell is missing"));
                return;
            }
            if (c.type != e->type || c.style != e->style) add(QStringLiteral("cell type or format"));
            else if (c.type == CellType::SharedString || c.type == CellType::InlineString || c.type == CellType::FormulaString) {
                if (x.text(c, book.sharedStrings()) != y.text(*e, back.sharedStrings())) add(QStringLiteral("cell text"));
            } else if (!(c.number == e->number || (std::isnan(c.number) && std::isnan(e->number)))) {
                add(QStringLiteral("cell number"));
            }
            if (c.hasFormula() != e->hasFormula() || (c.hasFormula() && x.formulaText(c) != y.formulaText(*e))) add(QStringLiteral("formula"));
        });
    }
    line(diffs.isEmpty() ? QStringLiteral("ok") : diffs.join(QStringLiteral("; ")));
    return diffs.isEmpty() ? 0 : 2;
}

} // namespace jp::crunchtool
