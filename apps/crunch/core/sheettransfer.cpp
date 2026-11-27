// A sheet copied to ANOTHER workbook: the first workbook's session takes a snapshot of the sheet (plain data), and the second's puts it in as a new sheet, in one step of Undo. The two
// stores are never touched together: the snapshot is made with the first held back (and let go of), and the new sheet is made with the second held back. The formulas of the copy that read
// other sheets of the first workbook become links to its file (read by the links' own thread, as any link is); those that read the copied sheet itself stay on it.
#include "core/editsession.h"

#include "core/syslocale.h"
#include "core/formatfind.h"
#include "sheet/externalbooks.h"
#include "sheet/namerefs.h"
#include "sheet/refshift.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QLocale>
#include <QRegularExpression>
#include <QSet>
#include <QUrl>

#include <algorithm>
#include <atomic>
#include <unordered_map>

namespace jp::crunch {

using namespace jp::office::sheet;
using jp::calc::Value;

namespace {

std::atomic<qint64> g_transferCap{0};

qint64 transferCap() { return g_transferCap.load() > 0 ? g_transferCap.load() : kMaxCellsPerEdit; }

quint64 placeOf(quint32 row, quint32 col) { return (quint64(row) << 32) | col; }

bool linkable(const QString &path)
{
    const QString suffix = QFileInfo(path).suffix().toLower();
    return suffix == QLatin1String("xlsx") || suffix == QLatin1String("xlsm") || suffix == QLatin1String("xltx") || suffix == QLatin1String("xltm");
}

} // namespace

void EditSession::setTransferCapForTest(qint64 cells) { g_transferCap.store(cells); }

EditSession::SheetExport EditSession::exportSheet(int sheet, const QString &filePath) const
{
    SheetExport out;
    auto refuse = [&](const QString &why) {
        out.ok = false;
        out.problem = why;
        return out;
    };
    if (!m_book || !m_book->book || !m_book->calc) return refuse(tr("The workbook is still being opened. It can be copied from when it is open."));
    QString why;
    if (!canEditSheet(sheet, &why)) return refuse(why);
    const Workbook &book = *m_book->book;
    const Calculation *calc = m_book->calc.get();
    const Styles &styles = book.styles();
    out.sheetName = book.info().sheets[sheet].name;
    out.bookPath = filePath;
    out.bookName = filePath.isEmpty() ? QString() : QFileInfo(filePath).fileName();
    out.themeKey = themeKeyOf(styles);
    out.date1904 = book.info().dateSystem == DateSystem::Date1904;
    out.bookChanged = isModified();
    for (const Table &t : book.tables())
        if (t.sheet == sheet) return refuse(tr("This sheet has a table. JeffCrunch does not yet copy tables to another workbook."));
    const std::shared_ptr<void> held = holdForReads();   // (the nodes of the cells are read: a result a formula keeps)
    const Sheet &s = calc->sheet(sheet);
    const qint64 cap = transferCap();
    out.usedRows = s.extentRows;
    out.usedCols = s.extentCols;
    if (qint64(s.extentRows) * qint64(s.extentCols) > cap)
        return refuse(tr("This sheet is larger than JeffCrunch copies to another workbook at once (%1 cells). Copy part of it with Copy and Paste.").arg(jp::systemLocale().toString(cap)));
    if (s.merges.size() > cap) return refuse(tr("This sheet has more merged cells than JeffCrunch copies to another workbook at once."));
    std::unordered_map<quint32, qint32> lookOf;
    auto lookFor = [&](quint32 xf) -> qint32 {
        if (xf == 0) return -1;
        const auto found = lookOf.find(xf);
        if (found != lookOf.end()) return found->second;
        FormatChange look = fullFormatChange(styles, xf);
        look.namedStyle.reset();   // (the look is carried, not the style it follows: that one is the other workbook's)
        out.resolvedLooks.push_back(withColorsResolved(styles, look));
        out.looks.push_back(std::move(look));
        return lookOf[xf] = qint32(out.looks.size() - 1);
    };
    QSet<QString> names;
    for (const DefinedName &n : book.info().definedNames) names.insert(n.name.toCaseFolded());
    bool hiddenFormula = false;
    bool followsStyle = false;
    qint64 visited = 0;
    bool tooMany = false;
    if (s.extentRows > 0 && s.extentCols > 0)
        s.cells.forEachInRange(0, 0, s.extentRows - 1, s.extentCols - 1, [&](quint32 row, quint32 col, const Cell &) {
            if (++visited > cap) {
                tooMany = true;
                return false;
            }
            TransferCell tc;
            tc.row = row;
            tc.col = col;
            tc.content = calc->content(sheet, row, col);
            if (!tc.content.exists || tc.content.spilled) return true;
            tc.shown = valueAt(sheet, row, col);
            Calculation::CellContent &c = tc.content;
            const bool arrayPart = !c.editable || c.arrayRange.valid || c.dataTable;
            if (c.isFormula && formulaHidden(sheet, c.style)) {
                hiddenFormula = true;
                tc.freeze = true;
            } else if (arrayPart) {
                tc.freeze = c.isFormula || !c.editable;
            }
            if (c.isFormula && !tc.freeze) {
                // (a name or a table the formula uses is the first workbook's: the cell comes as what it shows)
                bool usesName = c.formula.contains(QLatin1Char('['));
                if (!usesName && !names.isEmpty()) {
                    // (each word of the formula is looked up among the names: the work is the formula's length, not the number of names)
                    static const QRegularExpression word(QStringLiteral("[\\p{L}_\\\\][\\p{L}\\p{N}_.\\\\]*"));
                    for (auto it = word.globalMatch(c.formula); !usesName && it.hasNext();) {
                        const QString w = it.next().captured(0);
                        usesName = names.contains(w.toCaseFolded()) && formulaUsesName(c.formula, w);
                    }
                }
                tc.freeze = usesName;
                if (!tc.freeze) {
                    SheetLinkFacts facts;
                    linkSheetsInFormula(c.formula, out.sheetName, out.sheetName, 1, &facts);
                    if (facts.threeD || facts.readsLinks) {
                        out.ok = false;
                        out.problem = facts.threeD ? tr("A formula of this sheet adds up a block of sheets (such as Sheet1:Sheet3). JeffCrunch does not yet copy such a sheet to another workbook.")
                                                   : tr("A formula of this sheet already reads another workbook. JeffCrunch does not yet copy such a sheet to another workbook.");
                        return false;
                    }
                    tc.freeze = facts.unreadable;
                    out.readsOtherSheets = out.readsOtherSheets || facts.readsOthers;
                }
            }
            if (tc.freeze) {
                c.isFormula = false;
                c.dynamic = false;
                c.formula.clear();
                c.value = tc.shown;
                c.arrayRange = CellRange();
                c.dataTable = false;
                c.hasStored = false;
                ++out.frozen;
            }
            c.hasHeld = false;
            c.copied = false;
            c.sharedString = -1;
            if (c.style != 0 && styles.format(c.style).xfId != 0) followsStyle = true;
            tc.look = lookFor(c.style);
            out.cells.push_back(std::move(tc));
            return true;
        });
    if (!out.ok) return out;
    if (tooMany) return refuse(tr("This sheet has more cells than JeffCrunch copies to another workbook at once (%1). Copy part of it with Copy and Paste.").arg(jp::systemLocale().toString(cap)));
    out.merges = s.merges;
    out.columns = calc->columnRuns(sheet, 0, kMaxColumns - 1);
    out.rows = calc->rowSpecsIn(sheet, 0, kMaxRows - 1);
    if (qint64(out.rows.size()) > cap) return refuse(tr("This sheet has more rows with a size of their own than JeffCrunch copies to another workbook at once."));
    for (const ColumnSpan &c : out.columns) out.columnLook.push_back(lookFor(c.style));
    for (const RowSpec &r : out.rows) out.rowLook.push_back(r.customFormat ? lookFor(r.style) : -1);
    // what the copy will not have
    QStringList left = notCopiedKinds(sheet);
    auto add = [&left](const QString &what) {
        if (!left.contains(what)) left << what;
    };
    if (!s.conditionalFormats.isEmpty()) add(tr("conditional formats"));
    if (!s.validations.isEmpty()) add(tr("rules for what can be typed"));
    if (s.isProtected) add(tr("protection"));
    if (s.autoFilter.ref.valid) add(tr("filter"));
    if (!m_anchors.notes(sheet).empty()) add(tr("notes"));
    if (!m_anchors.links(sheet).empty()) add(tr("links to web pages and places"));
    if (s.tabColor.isSet()) add(tr("color of the tab"));
    if (hiddenFormula) add(tr("formulas hidden by protection (the cells come as values)"));
    if (followsStyle) add(tr("names of the cell styles (the cells look the same)"));
    add(tr("page setup and print settings"));
    out.notCopied = left;
    return out;
}

EditSession::Outcome EditSession::checkImport(const SheetExport &data) const
{
    Outcome out;
    auto refuse = [&](const QString &why) {
        out.ok = false;
        out.problem = why;
        return out;
    };
    if (!data.ok) return refuse(data.problem);
    QString why;
    if (!m_book || !m_book->book || !m_book->calc) return refuse(tr("The workbook is still being opened. It can be changed when it is open."));
    if (!const_cast<EditSession *>(this)->canChangeSheetList(true, &why)) return refuse(why);
    if ((m_book->book->info().dateSystem == DateSystem::Date1904) != data.date1904)
        return refuse(tr("The two workbooks count dates from different years (1900 and 1904), so a sheet can't be copied between them yet."));
    if (data.readsOtherSheets) {
        const QString what = data.bookName.isEmpty() ? tr("the workbook it comes from") : data.bookName;
        if (data.bookPath.isEmpty() || !linkable(data.bookPath))
            return refuse(tr("Formulas on this sheet read other sheets of %1, so the copy has to keep a link to its file. Save %1 as a file of the .xlsx kind first, and then copy the sheet.").arg(what));
        if (data.bookChanged)
            return refuse(tr("Formulas on this sheet read other sheets of %1, so the copy has to keep a link to its file, and %1 has changes that are not saved. Save it first, and then copy the sheet.").arg(what));
    }
    QStringList said;
    if (!data.notCopied.isEmpty()) said << tr("These will not be copied: %1.").arg(jp::systemLocale().locale.createSeparatedList(data.notCopied));
    if (data.readsOtherSheets) said << tr("Formulas that read other sheets of %1 become links to that file.").arg(data.bookName);
    if (!data.looks.empty() && m_book->book->styles().formats.isEmpty()) said << tr("This workbook has no formats to build on, so the copy does not have the fonts, colors, and borders of the sheet.");
    if (data.frozen > 0) said << tr("Cells that use names or tables of the other workbook, or are part of an array, come as the values they show (%1 in all).").arg(data.frozen);
    out.problem = said.join(QLatin1Char(' '));
    return out;
}

EditSession::Outcome EditSession::importSheet(const SheetExport &data, int before, const QString &name, int *made)
{
    Outcome out = checkImport(data);
    if (!out.ok) return out;
    const QString note = out.problem;
    auto refuse = [&](const QString &why) {
        out.ok = false;
        out.problem = why;
        return out;
    };
    QString nm = name;
    if (nm.isEmpty()) {
        nm = data.sheetName;
        for (int n = 2; !validSheetName(nm, -1) && n < 10000; ++n) {
            const QString tail = QStringLiteral(" (%1)").arg(n);
            nm = data.sheetName.left(31 - tail.size()) + tail;
        }
    }
    QString why;
    if (!validSheetName(nm, -1, &why)) return refuse(why);
    // the formats: each of the first workbook's, as this workbook's
    std::vector<quint32> styleOf(data.looks.size(), 0);
    {
        std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold();
        if (!hold) return refuse(tr("The workbook is busy. Try again in a moment."));
        Workbook &book = *m_book->book;
        const bool sameTheme = data.themeKey == themeKeyOf(book.styles());
        book.clearFormatLimit();
        for (size_t i = 0; i < data.looks.size(); ++i) styleOf[i] = book.formatWith(0, sameTheme ? data.looks[i] : data.resolvedLooks[i]);
        if (book.formatLimitHit()) {
            book.clearFormatLimit();
            return refuse(tr("This workbook can't take more formats. Nothing was copied."));
        }
    }
    // the link to the first workbook's file
    int link = 0;
    if (data.readsOtherSheets) {
        ExternalBooks *links = m_book->externals.get();
        if (!links) return refuse(tr("The workbook is still being opened. It can be changed when it is open."));
        const QString clean = QDir::cleanPath(QFileInfo(data.bookPath).absoluteFilePath());
        for (int i = 1; i <= links->count() && link == 0; ++i)
            if (!links->isNetwork(i) && QDir::cleanPath(links->localPath(i)) == clean) link = i;
        if (link == 0) {
            link = links->addLink(QUrl::fromLocalFile(QFileInfo(data.bookPath).absoluteFilePath()).toString(QUrl::FullyEncoded));
            m_newLinks.insert(link);
        }
        links->setLive(link, true);
    }
    const int start = m_undo.index();
    m_undo.beginMacro(tr("Copy Sheet to Another Workbook"));
    auto giveUp = [&](const QString &problem) {
        m_undo.endMacro();
        while (m_undo.index() > start) m_undo.undo();
        return refuse(problem);
    };
    int sheet = -1;
    Outcome inserted = insertSheet(before, nm, &sheet);
    if (!inserted.ok) return giveUp(inserted.problem);
    const quint32 rows = std::max<quint32>(1, data.usedRows), cols = std::max<quint32>(1, data.usedCols);
    std::unordered_map<quint64, const TransferCell *> at;
    for (const TransferCell &c : data.cells) at.emplace(placeOf(c.row, c.col), &c);
    const bool hasShared = !m_book->book->sharedStringsPart().isEmpty();
    PasteMaker make = [&](const PasteSpot &spot, CellChange *ch, Outcome *) {
        const auto found = at.find(placeOf(spot.row, spot.col));
        if (found == at.end()) return true;
        const TransferCell &src = *found->second;
        Calculation::CellContent &after = ch->after;
        after = src.content;
        after.style = src.look >= 0 ? styleOf[size_t(src.look)] : 0;
        if (after.isFormula) {
            after.formula = linkSheetsInFormula(src.content.formula, data.sheetName, nm, link);
            after.hasStored = src.content.hasStored;
            after.stored = src.content.stored;
        } else if (after.value.isText()) {
            ch->textToShare = hasShared;
        }
        after.exists = after.isFormula || !after.value.isEmpty() || after.style != 0;
        return true;
    };
    CellRange target;
    target.valid = true;
    target.row1 = target.col1 = 0;
    target.row2 = rows - 1;
    target.col2 = cols - 1;
    std::vector<CellRange> merges;
    for (const CellRange &m : data.merges) merges.push_back(m);
    Outcome pasted = pasteChanges(sheet, target, rows, cols, make, merges, true, tr("Copy Sheet to Another Workbook"));
    if (!pasted.ok) return giveUp(pasted.problem);
    // the sizes of the columns and rows
    Step sizes;
    if (!data.columns.empty()) {
        SizeEdit e;
        e.sheet = sheet;
        e.columns = true;
        e.first = data.columns.front().first;
        e.last = data.columns.back().last;
        for (size_t i = 0; i < data.columns.size(); ++i) {
            ColumnSpan c = data.columns[i];
            c.style = data.columnLook[i] >= 0 ? styleOf[size_t(data.columnLook[i])] : 0;
            e.columnsAfter.push_back(c);
        }
        sizes.sizes.push_back(std::move(e));
    }
    if (!data.rows.empty()) {
        SizeEdit e;
        e.sheet = sheet;
        e.columns = false;
        e.first = data.rows.front().row;
        e.last = data.rows.back().row;
        for (size_t i = 0; i < data.rows.size(); ++i) {
            RowSpec r = data.rows[i];
            r.style = data.rowLook[i] >= 0 ? styleOf[size_t(data.rowLook[i])] : 0;
            r.customFormat = data.rowLook[i] >= 0;
            r.filtered = false;
            e.rowsAfter.push_back(r);
        }
        sizes.sizes.push_back(std::move(e));
    }
    if (!sizes.sizes.empty()) pushStep(std::move(sizes), tr("Copy Sheet to Another Workbook"));
    m_undo.endMacro();
    if (made) *made = sheet;
    out = Outcome();
    out.problem = note;
    return out;
}

} // namespace jp::crunch
