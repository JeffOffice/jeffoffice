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

#include <unordered_set>

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

EditSession::SheetExport EditSession::exportSheet(int sheet, const QString &filePath, const QStringList &drawn) const
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
    if (s.merges.size() > cap) return refuse(tr("This sheet has more merged cells than JeffCrunch copies to another workbook at once."));
    std::unordered_map<quint32, qint32> lookOf;
    auto lookFor = [&](quint32 xf) -> qint32 {
        if (xf == 0) return -1;
        const auto found = lookOf.find(xf);
        if (found != lookOf.end()) return found->second;
        FormatChange look = fullFormatChange(styles, xf);
        look.namedStyle.reset();   // (the look is carried, not the style it follows: that one is the other workbook's)
        const BorderSpec &border = styles.border(styles.format(xf).border);
        look.diagonalUp = border.diagonalUp;   // (which way a diagonal line runs is part of the border)
        look.diagonalDown = border.diagonalDown;
        out.resolvedLooks.push_back(withColorsResolved(styles, look));
        out.looks.push_back(std::move(look));
        return lookOf[xf] = qint32(out.looks.size() - 1);
    };
    QSet<QString> names;
    for (const DefinedName &n : book.info().definedNames) names.insert(n.name.toCaseFolded());
    bool followsStyle = false;
    bool kindNames = false, kindTables = false, kindLinks = false, kindArrays = false, kindHidden = false, kindUnreadable = false;
    qint64 visited = 0;
    bool tooMany = false;
    std::unordered_set<quint64> taken;
    std::vector<std::pair<quint32, quint32>> spillers;
    // One cell of the sheet as the snapshot holds it: `member` is a cell a spill fills, which comes as the value it shows.
    auto take = [&](quint32 row, quint32 col, bool member) {
        TransferCell tc;
        tc.row = row;
        tc.col = col;
        tc.content = calc->content(sheet, row, col);
        if (member) tc.content.exists = true;
        if (!tc.content.exists) return true;
        taken.insert(placeOf(row, col));
        tc.shown = valueAt(sheet, row, col);
        Calculation::CellContent &c = tc.content;
        const bool arrayPart = !c.editable || c.arrayRange.valid || c.dataTable;
        if (c.isFormula && c.dynamic) spillers.emplace_back(row, col);
        if (member || c.spilled) {
            tc.freeze = true;
            kindArrays = true;
        } else if (c.isFormula && formulaHidden(sheet, c.style)) {
            kindHidden = true;
            tc.freeze = true;
        } else if (arrayPart || (c.isFormula && c.dynamic)) {
            tc.freeze = c.isFormula || !c.editable;
            kindArrays = kindArrays || tc.freeze;
        }
        if (c.isFormula && !tc.freeze) {
            // (the formula is read, not searched: a "[" in a text in quotes is no table and no other workbook)
            jp::calc::ParseOptions po;
            po.row = row;
            po.col = col;
            const jp::calc::ParseResult parsed = jp::calc::parseFormula(c.formula, po);
            if (!parsed.ok()) {
                tc.freeze = kindUnreadable = true;
            } else if (parsed.program->external) {
                tc.freeze = kindLinks = true;   // (a link to a third workbook: its value comes, the link does not yet)
            } else if (parsed.program->structured) {
                tc.freeze = kindTables = true;
            } else if (!names.isEmpty()) {
                // (each word of the formula is looked up among the names: the work is the formula's length, not the number of names)
                static const QRegularExpression word(QStringLiteral("[\\p{L}_\\\\][\\p{L}\\p{N}_.\\\\]*"));
                for (auto it = word.globalMatch(c.formula); !tc.freeze && it.hasNext();) {
                    const QString w = it.next().captured(0);
                    tc.freeze = names.contains(w.toCaseFolded()) && formulaUsesName(c.formula, w);
                }
                kindNames = kindNames || tc.freeze;
            }
            if (!tc.freeze) {
                SheetLinkFacts facts;
                linkSheetsInFormula(c.formula, out.sheetName, out.sheetName, 1, &facts);
                if (facts.threeD) {
                    out.ok = false;
                    out.problem = tr("A formula of this sheet adds up a block of sheets (such as Sheet1:Sheet3). JeffCrunch does not yet copy such a sheet to another workbook.");
                    return false;
                }
                tc.link = facts.readsOthers;
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
            c.spilled = false;
            c.editable = true;
            ++out.frozen;
        }
        c.hasHeld = false;
        c.copied = false;
        c.sharedString = -1;
        if (c.style != 0 && styles.format(c.style).xfId != 0) followsStyle = true;
        tc.look = lookFor(c.style);
        out.cells.push_back(std::move(tc));
        return true;
    };
    // (the cells that hold something are counted, not the rows and columns they reach: a sheet with one far cell is one cell)
    if (s.extentRows > 0 && s.extentCols > 0)
        s.cells.forEachInRange(0, 0, s.extentRows - 1, s.extentCols - 1, [&](quint32 row, quint32 col, const Cell &) {
            if (++visited > cap) {
                tooMany = true;
                return false;
            }
            return take(row, col, false);
        });
    // the cells a spill fills (held apart from the store when the workbook keeps them as blocks): each comes as the value it shows
    for (size_t i = 0; i < spillers.size() && out.ok && !tooMany; ++i) {
        const CellRange block = calc->spillBlock(sheet, spillers[i].first, spillers[i].second);
        if (!block.valid) continue;
        if (visited + qint64(block.row2 - block.row1 + 1) * qint64(block.col2 - block.col1 + 1) > cap + 1) {
            tooMany = true;
            break;
        }
        for (quint32 r = block.row1; r <= block.row2 && out.ok; ++r)
            for (quint32 c = block.col1; c <= block.col2 && out.ok; ++c) {
                if (taken.count(placeOf(r, c))) continue;
                ++visited;
                take(r, c, true);
            }
    }
    if (!out.ok) return out;
    if (tooMany) return refuse(tr("This sheet has more cells than JeffCrunch copies to another workbook at once (%1). Copy part of it with Copy and Paste.").arg(jp::systemLocale().toString(cap)));
    out.merges = s.merges;
    out.columns = calc->columnRuns(sheet, 0, kMaxColumns - 1);
    out.rows = calc->rowSpecsIn(sheet, 0, kMaxRows - 1);
    if (qint64(out.rows.size()) > cap) return refuse(tr("This sheet has more rows with a size of their own than JeffCrunch copies to another workbook at once."));
    for (const ColumnSpan &c : out.columns) out.columnLook.push_back(lookFor(c.style));
    for (const RowSpec &r : out.rows) out.rowLook.push_back(r.customFormat ? lookFor(r.style) : -1);
    // how the sheet is shown: the panes and the way it reads are the window's (the session keeps them), the rest is the sheet's; the cell the cursor is on and what is scrolled to are not kept
    out.view = s.view;
    if (sheet >= 0 && sheet < m_pages.sheetCount()) {
        const jp::office::sheet::SheetView &pane = m_pages.pane(sheet);
        out.view.frozen = pane.frozen;
        out.view.frozenRows = pane.frozenRows;
        out.view.frozenCols = pane.frozenCols;
        out.view.split = pane.split;
        out.view.splitX = pane.splitX;
        out.view.splitY = pane.splitY;
        out.view.rightToLeft = pane.rightToLeft;
        out.view.showZeros = pane.showZeros;
        out.view.zoom = m_pages.normalZoom(sheet);
    }
    out.view.kind = jp::office::sheet::SheetView::Kind::Normal;
    out.view.normalZoom = out.view.zoom;
    out.view.tabSelected = false;
    out.view.activeRow = out.view.activeCol = 0;
    out.view.topRow = out.view.leftCol = 0;
    out.view.paneRow = out.view.frozen ? out.view.frozenRows : 0;
    out.view.paneCol = out.view.frozen ? out.view.frozenCols : 0;
    out.format = s.format;
    out.format.summaryEdited = !out.format.summaryBelow || !out.format.summaryRight;
    // what the copy will not have
    QStringList left = notCopiedKinds(sheet);
    auto add = [&left](const QString &what) {
        if (!left.contains(what)) left << what;
    };
    for (const QString &d : drawn) add(d);
    if (!s.conditionalFormats.isEmpty()) add(tr("conditional formats"));
    if (!s.validations.isEmpty()) add(tr("rules for what can be typed"));
    if (s.isProtected) add(tr("protection"));
    if (s.autoFilter.ref.valid) add(tr("filter"));
    if (!m_anchors.notes(sheet).empty()) add(tr("notes"));
    if (!m_anchors.links(sheet).empty()) add(tr("links to web pages and places"));
    if (s.tabColor.isSet()) add(tr("color of the tab"));
    if (followsStyle) add(tr("names of the cell styles (the cells look the same)"));
    add(tr("page setup and print settings"));
    out.notCopied = left;
    // the kinds of cell that come as values, from a fixed list in a fixed order
    if (kindNames) out.frozenKinds << tr("use names of the other workbook");
    if (kindTables) out.frozenKinds << tr("use tables or spilled ranges");
    if (kindLinks) out.frozenKinds << tr("read cells of a third workbook");
    if (kindArrays) out.frozenKinds << tr("are part of an array or a spill");
    if (kindHidden) out.frozenKinds << tr("have formulas that protection hides");
    if (kindUnreadable) out.frozenKinds << tr("have formulas JeffCrunch could not read");
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
    if (data.readsOtherSheets && (data.bookPath.isEmpty() || !linkable(data.bookPath))) {
        // (a workbook with changes that are not saved is not refused: its links name its file, and the copy shows what the workbook has now)
        const QString what = data.bookName.isEmpty() ? tr("the workbook it comes from") : data.bookName;
        return refuse(tr("Formulas on this sheet read other sheets of %1, so the copy has to keep a link to its file. Save %1 as a file of the .xlsx kind first, and then copy the sheet.").arg(what));
    }
    // room for the looks is asked for before any is added (the worst case: each look is new)
    if (!data.looks.empty() && !m_book->book->styles().formats.isEmpty() && !m_book->book->formatRoomFor(qint64(data.looks.size())))
        return refuse(tr("The formats of this sheet (fonts, colors, borders, and so on, %1 in all) do not fit in this workbook, which is close to the most it can hold. Nothing was copied.")
                          .arg(jp::systemLocale().toString(qint64(data.looks.size()))));
    QStringList said;
    if (!data.notCopied.isEmpty()) said << tr("These will not be copied: %1.").arg(jp::systemLocale().locale.createSeparatedList(data.notCopied));
    if (data.readsOtherSheets) {
        said << tr("Formulas that read other sheets of %1 become links to its saved file.").arg(data.bookName);
        if (data.bookChanged) said << tr("%1 has changes that are not saved: the copy shows the values it has now, and the links do not see those changes until it is saved.").arg(data.bookName);
    }
    if (!data.looks.empty() && m_book->book->styles().formats.isEmpty()) said << tr("This workbook has no formats to build on, so the copy does not have the fonts, colors, and borders of the sheet.");
    if (data.frozen > 0) {
        const QString kinds = jp::systemLocale().locale.createSeparatedList(data.frozenKinds);
        said << (kinds.isEmpty() ? tr("Some cells come as the values they show, not as formulas (%1 in all).").arg(jp::systemLocale().toString(data.frozen))
                                 : tr("Some cells come as the values they show, not as formulas (%1 in all): those that %2.").arg(jp::systemLocale().toString(data.frozen), kinds));
    }
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
    if (data.merges.size() > jp::office::sheet::kMaxMerges) return refuse(tr("This sheet has more merged cells than JeffCrunch copies to another workbook at once."));
    // (everything that can be refused is asked before anything is changed; the formats a refused or undone copy leaves are bounded by the same room: a format is never made twice)
    const int start = m_undo.index();
    m_undo.beginMacro(tr("Copy Sheet to Another Workbook"));
    auto giveUp = [&](const QString &problem) {
        m_undo.endMacro();
        while (m_undo.index() > start) m_undo.undo();
        return refuse(problem);
    };
    int sheet = -1;
    Outcome inserted = insertSheetOfKind(before, nm, &sheet, false, &data.view, &data.format);
    if (!inserted.ok) return giveUp(inserted.problem);
    // the formats: each of the first workbook's, as this workbook's
    std::vector<quint32> styleOf(data.looks.size(), 0);
    {
        std::unique_ptr<WorkbookLoader::Hold> hold = m_loader->hold();
        if (!hold) return giveUp(tr("The workbook is busy. Try again in a moment."));
        Workbook &book = *m_book->book;
        const bool sameTheme = data.themeKey == themeKeyOf(book.styles());
        book.clearFormatLimit();
        for (size_t i = 0; i < data.looks.size(); ++i) styleOf[i] = book.formatWith(0, sameTheme ? data.looks[i] : data.resolvedLooks[i]);
        if (book.formatLimitHit()) {
            book.clearFormatLimit();
            return giveUp(tr("This workbook can't take more formats. Nothing was copied."));
        }
    }
    // the link to the first workbook's file
    int link = 0;
    if (data.readsOtherSheets) {
        ExternalBooks *links = m_book->externals.get();
        if (!links) return giveUp(tr("The workbook is still being opened. It can be changed when it is open."));
        const QString clean = QDir::cleanPath(QFileInfo(data.bookPath).absoluteFilePath());
        for (int i = 1; i <= links->count() && link == 0; ++i)
            if (!links->isNetwork(i) && QDir::cleanPath(links->localPath(i)) == clean) link = i;
        if (link == 0) {
            link = links->addLink(QUrl::fromLocalFile(QFileInfo(data.bookPath).absoluteFilePath()).toString(QUrl::FullyEncoded));
            m_newLinks.insert(link);
        }
        links->setLive(link, true);
    }
    // the cells that hold something, and the merged cells, as one step (the sheet is new and empty)
    Step step;
    {
        const std::shared_ptr<void> held = holdForReads();
        const Calculation *calc = m_book->calc.get();
        const bool hasShared = !m_book->book->sharedStringsPart().isEmpty();
        for (const TransferCell &src : data.cells) {
            CellChange ch;
            ch.sheet = sheet;
            ch.row = src.row;
            ch.col = quint16(src.col);
            ch.before = calc->content(sheet, src.row, src.col);
            Calculation::CellContent &after = ch.after;
            after = src.content;
            after.style = src.look >= 0 ? styleOf[size_t(src.look)] : 0;
            if (after.isFormula) {
                after.formula = linkSheetsInFormula(src.content.formula, data.sheetName, nm, link);
                if (src.link) {
                    // (a formula that reads the other workbook shows what that workbook shows now: its stored result is that, until the links are updated)
                    after.hasStored = true;
                    after.stored = src.shown;
                } else {
                    after.hasStored = src.content.hasStored;
                    after.stored = src.content.stored;
                }
            } else if (after.value.isText()) {
                ch.textToShare = hasShared;
            }
            after.exists = after.isFormula || !after.value.isEmpty() || after.style != 0;
            if (!after.exists) continue;
            step.cells.push_back(std::move(ch));
        }
        if (!data.merges.isEmpty()) {
            MergeEdit m;
            m.sheet = sheet;
            m.before = calc->mergesOf(sheet);
            m.countBefore = calc->mergeCountOf(sheet);
            m.after = m.before;
            for (const CellRange &r : data.merges) m.after.append(r);
            m.countAfter = m.after.size();
            step.merges.push_back(std::move(m));
        }
    }
    if (!step.cells.empty() || !step.merges.empty()) pushStep(std::move(step), tr("Copy Sheet to Another Workbook"));
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
