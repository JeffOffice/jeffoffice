#include "sheet/oldsavecommon.h"

#include "sheet/drawing.h"
#include "sheet/tables.h"

#include <QCoreApplication>

#include <algorithm>

namespace jp::office::sheet::oldsave {

namespace {

QString validName(QString name, int maxChars)
{
    for (QChar &c : name)
        if (c == QLatin1Char('[') || c == QLatin1Char(']') || c == QLatin1Char(':') || c == QLatin1Char('*') || c == QLatin1Char('?') || c == QLatin1Char('/') || c == QLatin1Char('\\')) c = QLatin1Char('_');
    while (name.startsWith(QLatin1Char('\'')) || name.endsWith(QLatin1Char('\''))) name = name.startsWith(QLatin1Char('\'')) ? name.mid(1) : name.chopped(1);
    if (name.size() > maxChars) name = name.left(maxChars);
    if (name.trimmed().isEmpty()) name = QStringLiteral("Sheet");
    return name;
}

} // namespace

std::vector<Written> sheetsToWrite(const Workbook &book, int limit, int maxName, CopyReport *report)
{
    std::vector<Written> out;
    QSet<QString> used;
    const WorkbookInfo &info = book.info();
    qint64 skippedKinds = 0, chartSheets = 0, cut = 0, renamed = 0;
    for (int place : info.order) {
        if (place < 0 || place >= info.sheets.size()) continue;
        const SheetEntry &e = info.sheets[place];
        if (e.deleted) continue;
        if (e.kind != SheetKind::Worksheet) {
            if (e.kind == SheetKind::Chartsheet) ++chartSheets;
            else ++skippedKinds;
            continue;
        }
        if (int(out.size()) >= limit) {
            ++cut;
            continue;
        }
        Written w;
        w.place = place;
        w.formulaName = e.name;
        w.name = validName(e.name, maxName);
        if (w.name != e.name) ++renamed;
        QString key = w.name.toLower();
        int n = 1;
        while (used.contains(key)) {
            const QString suffix = QStringLiteral(" (%1)").arg(++n);
            w.name = validName(e.name, maxName - int(suffix.size())) + suffix;
            key = w.name.toLower();
        }
        used.insert(key);
        out.push_back(w);
    }
    if (out.empty()) {
        Written w;
        w.place = -1;
        w.name = QStringLiteral("Sheet1");
        out.push_back(w);
    }
    if (report) {
        if (chartSheets > 0) report->notKept << QCoreApplication::translate("Sheet", "chart sheets (%1): this format has no sheet that is a chart, so the chart on each is left out; move it onto a worksheet to keep it").arg(chartSheets);
        if (skippedKinds > 0) report->notKept << QCoreApplication::translate("Sheet", "sheets that are dialogs or macros (%1)").arg(skippedKinds);
        if (cut > 0) report->trimmed << QCoreApplication::translate("Sheet", "%1 sheets past the first %2 that the format holds").arg(cut).arg(limit);
        if (renamed > 0) report->trimmed << QCoreApplication::translate("Sheet", "%1 sheet names changed to names the format allows").arg(renamed);
    }
    return out;
}

std::vector<PlacedCell> sortedCells(const Sheet &sheet, qint64 maxCells, qint64 *notCopied)
{
    std::vector<PlacedCell> cells;
    const qint64 total = sheet.cells.cellCount();
    const qint64 keep = std::max<qint64>(0, std::min(total, maxCells));
    cells.reserve(size_t(std::min<qint64>(keep, 50000000)));
    qint64 seen = 0;
    sheet.cells.forEach([&](quint32 row, quint32 col, const Cell &c) {
        if (seen++ < keep) cells.push_back(PlacedCell{row, col, c});
    });
    if (notCopied) *notCopied = std::max<qint64>(0, seen - qint64(cells.size()));
    const bool sorted = std::is_sorted(cells.begin(), cells.end(), [](const PlacedCell &a, const PlacedCell &b) { return a.row != b.row ? a.row < b.row : a.col < b.col; });
    if (!sorted) std::sort(cells.begin(), cells.end(), [](const PlacedCell &a, const PlacedCell &b) { return a.row != b.row ? a.row < b.row : a.col < b.col; });
    return cells;
}

QString cutText(const QString &text, bool *cut)
{
    if (text.size() <= kMaxCopyTextChars) return text;
    if (cut) *cut = true;
    int n = kMaxCopyTextChars;
    if (text[n - 1].isHighSurrogate()) --n;
    return text.left(n);
}

QString textOf(const Sheet &sheet, const SharedStrings &shared, const Cell &cell)
{
    return sheet.text(cell, shared);
}

QStringList topReasons(const QMap<QString, qint64> &reasons, int most)
{
    QVector<QPair<qint64, QString>> sorted;
    for (auto it = reasons.cbegin(); it != reasons.cend(); ++it) sorted.append({it.value(), it.key()});
    std::sort(sorted.begin(), sorted.end(), [](const QPair<qint64, QString> &a, const QPair<qint64, QString> &b) { return a.first != b.first ? a.first > b.first : a.second < b.second; });
    QStringList out;
    for (int i = 0; i < sorted.size() && i < most; ++i) out << QStringLiteral("%1 x %2").arg(sorted[i].first).arg(sorted[i].second);
    return out;
}

QString sheetNote(const QString &sheet, const QString &what)
{
    return QCoreApplication::translate("Sheet", "%1: %2").arg(sheet, what);
}

Features::Features(const Workbook &book, CopyKind kind) : m_book(book), m_kind(kind) {}

void Features::note(const Sheet &sheet)
{
    auto add = [&](const char *what) { m_found.insert(QCoreApplication::translate("Sheet", what)); };
    if (!sheet.validations.isEmpty()) add(QT_TRANSLATE_NOOP("Sheet", "data validation"));
    if (!sheet.scenarios.isEmpty()) add(QT_TRANSLATE_NOOP("Sheet", "scenarios"));
    if (!sheet.conditionalFormats.isEmpty()) add(QT_TRANSLATE_NOOP("Sheet", "conditional formatting"));
    if (sheet.autoFilter.valid()) add(QT_TRANSLATE_NOOP("Sheet", "filters"));
    if (sheet.isProtected) add(QT_TRANSLATE_NOOP("Sheet", "sheet protection"));
    if (sheet.tabColor.isSet() && m_kind == CopyKind::Xls) add(QT_TRANSLATE_NOOP("Sheet", "tab colors"));
    // (the old form writes the page setup; OpenDocument's page styles are written as far as margins, orientation, scale, and the texts of headers and footers)
    if (!sheet.page.headerFooter.pictures.isEmpty()) add(QT_TRANSLATE_NOOP("Sheet", "pictures in headers and footers"));
    if (m_kind == CopyKind::Ods && (sheet.page.titles.hasRows || sheet.page.titles.hasCols || !sheet.page.colBreaks.isEmpty())) add(QT_TRANSLATE_NOOP("Sheet", "print titles and page breaks between columns"));
    if (!sheet.importedCharts.isEmpty()) add(QT_TRANSLATE_NOOP("Sheet", "charts"));   // (read from the older form, and written only to an .xlsx)
    for (const QString &s : sheet.notKept) m_found.insert(s);
    if (!sheet.richTexts.isEmpty()) add(QT_TRANSLATE_NOOP("Sheet", "formatting inside a text"));
    // formulas that spill into the cells beside them, and LET (the older forms have neither: the copy keeps the values, and the formula as an array formula where it has the functions)
    bool spills = false;
    sheet.cells.forEach([&](quint32, quint32, const Cell &c) {
        if ((c.flags & SpillFormula) && c.hasFormula()) spills = true;
    });
    if (spills) add(QT_TRANSLATE_NOOP("Sheet", "formulas that fill the cells beside them (each is saved with the values it gave)"));
    for (const QString &text : sheet.formulaTexts)
        if (text.contains(QLatin1String("LET("), Qt::CaseInsensitive)) {
            add(QT_TRANSLATE_NOOP("Sheet", "the LET function (the formula keeps the value it gave)"));
            break;
        }
}

QStringList Features::result() const
{
    QSet<QString> found = m_found;
    auto add = [&](const char *what) { found.insert(QCoreApplication::translate("Sheet", what)); };
    const Package &p = m_book.package();
    if (!m_book.isConverted()) {
        bool charts = false, drawings = false, pivots = false, notes = false, macros = false, links = false, slicers = false, controls = false, media = false;
        for (const QString &part : p.partNames()) {
            const QString l = part.toLower();
            if (l.contains(QLatin1String("/charts/chart")) || l.contains(QLatin1String("/chartsheets/"))) charts = true;
            else if (l.contains(QLatin1String("/drawings/drawing")) && !l.contains(QLatin1String("vmldrawing"))) drawings = true;
            else if (l.contains(QLatin1String("/media/"))) media = true;
            else if (l.contains(QLatin1String("pivottable")) || l.contains(QLatin1String("pivotcache"))) pivots = true;
            else if (l.contains(QLatin1String("/comments")) || l.contains(QLatin1String("threadedcomment"))) notes = true;
            else if (l.contains(QLatin1String("vbaproject"))) macros = true;
            else if (l.contains(QLatin1String("externallink"))) links = true;
            else if (l.contains(QLatin1String("slicer")) || l.contains(QLatin1String("timeline"))) slicers = true;
            else if (l.contains(QLatin1String("ctrlprop"))) controls = true;
        }
        // (an .ods copy has each equation as a picture of it: they are named, and the rest of the drawings are what the copy has not)
        int equations = 0, objects = 0;
        if (m_kind == CopyKind::Ods && drawings)
            for (const QString &part : p.partNames()) {
                const QString l = part.toLower();
                if (!l.contains(QLatin1String("/drawings/drawing")) || l.contains(QLatin1String("vmldrawing")) || l.endsWith(QLatin1String(".rels"))) continue;
                int n = 0;
                equations += int(readEquations(p, part, &n).size());
                objects += n;
            }
        if (equations > 0) {
            add(QT_TRANSLATE_NOOP("Sheet", "equations (each is saved as a picture of it)"));
            if (equations >= objects) drawings = media = false;
        }
        if (charts && m_kind == CopyKind::Xls) add(QT_TRANSLATE_NOOP("Sheet", "charts"));   // (an .ods copy writes them: odschart.h)
        if ((drawings || media) && m_kind == CopyKind::Xls) add(QT_TRANSLATE_NOOP("Sheet", "pictures and other drawing objects"));   // (an .ods copy writes them: odsdraw.h)
        if (pivots) add(QT_TRANSLATE_NOOP("Sheet", "pivot tables"));
        if (notes && m_kind == CopyKind::Xls) add(QT_TRANSLATE_NOOP("Sheet", "notes"));
        if (macros) add(QT_TRANSLATE_NOOP("Sheet", "macros"));
        if (links) add(QT_TRANSLATE_NOOP("Sheet", "links to other workbooks (their formulas keep the values they had)"));
        if (slicers) add(QT_TRANSLATE_NOOP("Sheet", "slicers and timelines"));
        if (controls) add(QT_TRANSLATE_NOOP("Sheet", "form controls"));
        if (!m_book.tables().isEmpty()) add(QT_TRANSLATE_NOOP("Sheet", "tables"));
    } else {
        for (const QString &s : m_book.skippedFeatures()) found.insert(s);
    }
    // (a pivot table made in this session has no part in the package yet: the model says)
    if (m_book.pivots().tableCount() > 0) add(QT_TRANSLATE_NOOP("Sheet", "pivot tables"));
    for (const FillSpec &f : m_book.styles().fills)
        if (f.gradient && f.stops.size() >= 2) {
            add(QT_TRANSLATE_NOOP("Sheet", "gradient fills (they are shown as one color)"));
            break;
        }
    if (!m_book.sharedStrings().rich.isEmpty()) add(QT_TRANSLATE_NOOP("Sheet", "formatting inside a text"));
    if (m_book.info().protection.lockStructure) add(QT_TRANSLATE_NOOP("Sheet", "workbook protection"));
    if (!m_book.info().customViews.isEmpty()) add(QT_TRANSLATE_NOOP("Sheet", "custom views"));
    QStringList out(found.cbegin(), found.cend());
    std::sort(out.begin(), out.end());
    return out;
}

} // namespace jp::office::sheet::oldsave

namespace jp::office::sheet {

QStringList featuresNotInCopy(const Workbook &book, CopyKind kind)
{
    oldsave::Features features(book, kind);
    const WorkbookInfo &info = book.info();
    for (int place : info.order) {
        if (place < 0 || place >= info.sheets.size() || info.sheets[place].deleted || info.sheets[place].kind != SheetKind::Worksheet) continue;
        Sheet sheet;
        book.readSheet(place, &sheet);
        features.note(sheet);
    }
    return features.result();
}

} // namespace jp::office::sheet
