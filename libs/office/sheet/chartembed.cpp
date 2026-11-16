#include "sheet/chartembed.h"

#include "sheet/cellref.h"
#include "sheet/chartdata.h"
#include "sheet/xlsxsave.h"
#include "calc/numfmt.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QVector>
#include <QSet>

#include <cmath>
#include <memory>

namespace jp::office::sheet {

namespace {

// "Sheet1!$B$2:$B$13" or "'My Sheet'!$B$2": the sheet and the block; false for anything else (no sheet, a list of blocks, a name, a whole column).
bool splitRef(const QString &ref, QString *sheet, CellRange *range)
{
    QString text = ref.trimmed();
    if (text.startsWith(QLatin1Char('('))) return false;
    int bang = -1;
    if (text.startsWith(QLatin1Char('\''))) {
        QString name;
        int i = 1;
        for (; i < text.size(); ++i) {
            if (text[i] == QLatin1Char('\'')) {
                if (i + 1 < text.size() && text[i + 1] == QLatin1Char('\'')) {
                    name += QLatin1Char('\'');
                    ++i;
                    continue;
                }
                break;
            }
            name += text[i];
        }
        if (i + 1 >= text.size() || text[i + 1] != QLatin1Char('!')) return false;
        *sheet = name;
        bang = i + 1;
    } else {
        bang = text.indexOf(QLatin1Char('!'));
        if (bang <= 0) return false;
        *sheet = text.left(bang);
    }
    const QString block = text.mid(bang + 1);
    if (block.contains(QLatin1Char(',')) || block.contains(QLatin1Char('!'))) return false;
    return parseRangeRef(block, range) && range->valid;
}

// One column or one row of cells (or one cell) on a sheet.
struct Line {
    QString sheet;
    CellRange range;
    bool set = false;
    qint64 count() const { return range.cellCount(); }
    bool column() const { return range.col1 == range.col2; }   // (one cell is a column)
    QPair<quint32, quint32> at(qint64 i, bool down) const { return down ? QPair<quint32, quint32>(range.row1 + quint32(i), range.col1) : QPair<quint32, quint32>(range.row1, range.col1 + quint32(i)); }
    bool operator==(const Line &o) const { return sheet.compare(o.sheet, Qt::CaseInsensitive) == 0 && range == o.range; }
};

// The workbook, opened for reading and for saving.
struct Opened {
    Package package;
    Workbook book;
    Sheet sheet;
    int index = -1;
};

QString unreadable() { return QCoreApplication::translate("jp::ChartData", "The workbook kept with this chart can't be read, so its numbers can't be changed here."); }
QString notFollowed() { return QCoreApplication::translate("jp::ChartData", "This chart reads cells of the workbook kept with it that can't be followed (another sheet or file, a block that is not one column or one row, or cells past the sheet's), so its numbers can't be changed here."); }
QString notPlain() { return QCoreApplication::translate("jp::ChartData", "Some cells of the workbook kept with this chart hold a formula, a date, a formatted number used as a name, or something that is not a number or words, so its numbers can't be changed here."); }
QString tooLarge() { return QCoreApplication::translate("jp::ChartData", "The workbook kept with this chart is too large to edit here, so its numbers can't be changed here."); }
QString tooBig() { return QCoreApplication::translate("jp::ChartData", "Some numbers of this chart are larger than 1,000,000,000,000,000, so its numbers can't be changed here."); }
QString tooMany() { return QCoreApplication::translate("jp::ChartData", "This chart has more than %1 series or %2 points, or numbers that are not numbers, so its numbers can't be changed here.").arg(kMostChartSeries).arg(kMostChartCategories); }

thread_local qint64 g_opened = 0;

// Opens the workbook with the limits of a chart's workbook: its file, what it comes to inflated, and the cells of a sheet (see chartembed.h).
bool openWorkbook(const QByteArray &bytes, const QString &sheetName, Opened *o, QString *problem)
{
    if (bytes.isEmpty()) {
        *problem = QCoreApplication::translate("jp::ChartData", "The numbers of this chart are in a workbook that is kept with it, so they can't be changed here.");
        return false;
    }
    if (bytes.size() > kMostEmbeddedWorkbookBytes) {
        *problem = tooLarge();
        return false;
    }
    ++g_opened;
    if (!o->package.readLazy(bytes, nullptr, kMostEmbeddedInflatedBytes)) {
        // (one that opens with the usual allowance is only too large)
        Package probe;
        *problem = probe.readLazy(bytes) ? tooLarge() : unreadable();
        return false;
    }
    Limits limits;
    limits.fileBytes = kMostEmbeddedWorkbookBytes;
    limits.cellsPerSheet = kMostEmbeddedSheetCells;
    limits.sheetBytes = kMostEmbeddedInflatedBytes;
    limits.sheets = 64;
    if (!o->book.open(o->package, nullptr, limits)) {
        *problem = unreadable();
        return false;
    }
    const QVector<SheetEntry> &sheets = o->book.info().sheets;
    for (int i = 0; i < sheets.size(); ++i)
        if (!sheets[i].deleted && sheets[i].kind == SheetKind::Worksheet && sheets[i].name.compare(sheetName, Qt::CaseInsensitive) == 0) o->index = i;
    if (o->index < 0) {
        *problem = notFollowed();
        return false;
    }
    if (!o->book.readSheet(o->index, &o->sheet) || !o->sheet.error.isEmpty() || o->sheet.partial) {
        // (past a limit of the reader, or damaged)
        const QString &why = o->sheet.error;
        *problem = why == QCoreApplication::translate("Sheet", "The sheet has more cells than any real file has.") || why == QCoreApplication::translate("Sheet", "The sheet would take more memory to hold than any real file's does.") ? tooLarge() : unreadable();
        return false;
    }
    return true;
}

// What the references of the chart say, checked.
struct Plan {
    QVector<ChartSeries *> series;
    QVector<Line> values, names;   // for each series (a name that is not a cell: not `set`)
    Line categories;               // shared by every series (not `set`: the words are the chart's own)
    QString sheet;
    bool down = true;              // the points run down a column (else along a row)
    int points = 0;
};

QVector<ChartSeries *> allSeries(Chart &chart)
{
    QVector<ChartSeries *> out;
    for (ChartPlot &p : chart.plots)
        for (ChartSeries &s : p.series) out << &s;
    return out;
}

// The plan, or a reason it can't be had.
QString plan(Chart &chart, Plan *out)
{
    out->series = allSeries(chart);
    if (out->series.isEmpty()) return QCoreApplication::translate("jp::ChartData", "This chart has no series.");
    if (out->series.size() > kMostChartSeries) return tooMany();
    bool first = true;
    for (ChartSeries *s : std::as_const(out->series)) {
        if (!s->xRef.isEmpty() || s->valueRef.isEmpty()) return notFollowed();
        Line v;
        if (!splitRef(s->valueRef, &v.sheet, &v.range)) return notFollowed();
        v.set = true;
        if (v.range.row1 != v.range.row2 && v.range.col1 != v.range.col2) return notFollowed();
        if (first) {
            out->sheet = v.sheet;
            out->down = v.column();
            out->points = int(std::min<qint64>(v.count(), kMostChartCategories + 1));
            if (v.count() > kMostChartCategories) return tooMany();
        }
        // (one point reads as a column or a row: it takes the direction of the rest)
        if (v.sheet.compare(out->sheet, Qt::CaseInsensitive) != 0 || v.count() != out->points || (v.count() > 1 && v.column() != out->down)) return notFollowed();
        out->values << v;
        Line n;
        if (!s->nameRef.isEmpty()) {
            if (!splitRef(s->nameRef, &n.sheet, &n.range) || n.count() != 1 || n.sheet.compare(out->sheet, Qt::CaseInsensitive) != 0) return notFollowed();
            n.set = true;
        }
        out->names << n;
        Line c;
        if (!s->categoryRef.isEmpty()) {
            if (!splitRef(s->categoryRef, &c.sheet, &c.range) || c.sheet.compare(out->sheet, Qt::CaseInsensitive) != 0 || c.count() != out->points || (c.count() > 1 && c.column() != out->down)) return notFollowed();
            c.set = true;
        }
        if (first) out->categories = c;
        else if (c.set != out->categories.set || (c.set && !(c == out->categories))) return notFollowed();
        first = false;
    }
    // A cell is read for one thing only (a change to it would change two).
    QSet<quint64> cells;
    auto key = [](quint32 row, quint32 col) { return (quint64(row) << 20) | col; };
    for (const Line &l : std::as_const(out->values))
        for (qint64 i = 0; i < l.count(); ++i) {
            const auto [row, col] = l.at(i, out->down);
            if (cells.contains(key(row, col))) return notFollowed();
            cells.insert(key(row, col));
        }
    if (out->categories.set)
        for (qint64 i = 0; i < out->categories.count(); ++i) {
            const auto [row, col] = out->categories.at(i, out->down);
            if (cells.contains(key(row, col))) return notFollowed();
            cells.insert(key(row, col));
        }
    QSet<quint64> named;
    for (const Line &l : std::as_const(out->names)) {
        if (!l.set) continue;
        const quint64 k = key(l.range.row1, l.range.col1);
        if (cells.contains(k) || named.contains(k)) return notFollowed();
        named.insert(k);
    }
    return QString();
}

// Whether the cell's number format shows it as a date or time (a number in a date's format is not a plain number: the chart shows it as a date, which this can't type), and whether it is
// General (a number as a name or a word is shown and typed as it is only then).
calc::FormatKind formatKindOf(const Opened &o, const Cell &cell) { return calc::NumberFormat(o.book.styles().formatCode(cell.style)).kind(); }

// A cell's words (empty for a cell that is not there or holds nothing); false for one that holds more than words or a plain number (a formula, a date, a number with a format of its own,
// an error, a true or false).
bool cellText(const Opened &o, quint32 row, quint32 col, QString *text, bool *number)
{
    *text = QString();
    *number = false;
    const Cell *cell = o.sheet.cells.find(row, col);
    if (!cell) return true;
    if (cell->hasFormula()) return false;
    switch (cell->type) {
    case CellType::Empty: return true;
    case CellType::SharedString:
    case CellType::InlineString:
        *text = o.sheet.text(*cell, o.book.sharedStrings());
        return true;
    case CellType::Number:
        if (!std::isfinite(cell->number) || formatKindOf(o, *cell) != calc::FormatKind::General) return false;
        *text = chartNumberText(cell->number);
        *number = true;
        return true;
    default: return false;
    }
}

// A number of the series: a number cell that is not a date (any other number format is fine: the chart stores the number, and its format code stays).
bool cellNumber(const Opened &o, quint32 row, quint32 col, double *value)
{
    const Cell *cell = o.sheet.cells.find(row, col);
    if (!cell || cell->hasFormula() || cell->type != CellType::Number || !std::isfinite(cell->number) || formatKindOf(o, *cell) == calc::FormatKind::Date) return false;
    *value = cell->number;
    return true;
}

// The grid the cells make, and which of its words are numbers in their cells.
struct Grid {
    ChartData data;
    QVector<bool> categoryNumber, nameNumber;
};

QString gridOf(const Plan &p, const Opened &o, Grid *grid)
{
    Grid out;
    ChartData &data = out.data;
    const quint32 rows = o.sheet.extentRows, cols = o.sheet.extentCols;
    auto inside = [&](const Line &l) { return l.range.row2 < rows && l.range.col2 < cols; };
    if (p.categories.set) {
        if (!inside(p.categories)) return notFollowed();
        for (qint64 i = 0; i < p.categories.count(); ++i) {
            const auto [row, col] = p.categories.at(i, p.down);
            QString text;
            bool number = false;
            if (!cellText(o, row, col, &text, &number)) return notPlain();
            out.categoryNumber << number;
            data.categories << text;
        }
    } else {
        data.categories = p.series.first()->categories;
        if (data.categories.size() > p.points) return notFollowed();
        while (data.categories.size() < p.points) data.categories << QString();
        out.categoryNumber.fill(false, data.categories.size());
    }
    for (int s = 0; s < p.series.size(); ++s) {
        if (!inside(p.values[s])) return notFollowed();
        QVector<double> values;
        for (qint64 i = 0; i < p.values[s].count(); ++i) {
            const auto [row, col] = p.values[s].at(i, p.down);
            double v = 0;
            if (!cellNumber(o, row, col, &v)) return notPlain();
            if (std::abs(v) > 1e15) return tooBig();
            values << v;
        }
        data.values << values;
        if (p.names[s].set) {
            if (!inside(p.names[s])) return notFollowed();
            QString text;
            bool number = false;
            if (!cellText(o, p.names[s].range.row1, p.names[s].range.col1, &text, &number)) return notPlain();
            data.seriesNames << text;
            out.nameNumber << number;
        } else {
            data.seriesNames << p.series[s]->name;
            out.nameNumber << false;
        }
    }
    if (!chartDataIsUsable(data)) return tooMany();
    *grid = out;
    return QString();
}

// What a chart's workbook was said to hold, by the workbook (its digest) and what the chart reads: asked again and again (each refresh of the window's commands), the workbook is opened once.
struct Said {
    QByteArray digest;
    QString signature;
    QString problem;
    ChartData data;
};
thread_local QVector<Said> g_said;

// The digest of a workbook's bytes: found again by the buffer itself (the same bytes, shared) so that asking again costs no pass over them.
struct Digested {
    QByteArray bytes, digest;
};
thread_local Digested g_digested[8];
thread_local int g_digestedNext = 0;

QByteArray digestOf(const QByteArray &book)
{
    for (const Digested &d : g_digested)
        if (!d.bytes.isNull() && d.bytes.constData() == book.constData() && d.bytes.size() == book.size()) return d.digest;
    Digested made{book, QCryptographicHash::hash(book, QCryptographicHash::Sha1)};
    g_digested[g_digestedNext] = made;
    g_digestedNext = (g_digestedNext + 1) % 8;
    return made.digest;
}

// What the grid depends on besides the workbook: where the chart reads, and the words it keeps itself.
QString signatureOf(Chart &chart)
{
    QString sig = chart.externalDataId;
    for (const ChartSeries *s : allSeries(chart)) sig += QLatin1Char('\x1f') + s->valueRef + QLatin1Char('\x1e') + s->nameRef + QLatin1Char('\x1e') + s->categoryRef + QLatin1Char('\x1e') + s->xRef + QLatin1Char('\x1e') + s->name + QLatin1Char('\x1e') + s->categories.join(QLatin1Char('\x1d'));
    return sig;
}

} // namespace

qint64 embeddedWorkbooksOpenedForTest() { return g_opened; }

QString embeddedChartProblem(const Chart &chart, const QByteArray &workbook, ChartData *data)
{
    Chart copy = chart;
    Plan p;
    if (const QString why = plan(copy, &p); !why.isEmpty()) return why;
    if (workbook.isEmpty()) return QCoreApplication::translate("jp::ChartData", "The numbers of this chart are in a workbook that is kept with it, so they can't be changed here.");
    const QByteArray digest = digestOf(workbook);
    const QString signature = signatureOf(copy);
    for (const Said &said : std::as_const(g_said))
        if (said.digest == digest && said.signature == signature) {
            if (said.problem.isEmpty() && data) *data = said.data;
            return said.problem;
        }
    Said made{digest, signature, QString(), ChartData()};
    auto o = std::make_unique<Opened>();
    Grid grid;
    if (openWorkbook(workbook, p.sheet, o.get(), &made.problem)) made.problem = gridOf(p, *o, &grid);
    made.data = grid.data;
    if (g_said.size() >= 16) g_said.clear();
    g_said.append(made);
    if (made.problem.isEmpty() && data) *data = made.data;
    return made.problem;
}

bool embeddedChartSetData(Chart &chart, const QByteArray &workbook, const ChartData &data, QByteArray *newWorkbook)
{
    Chart work = chart;
    Plan p;
    if (!plan(work, &p).isEmpty() || workbook.isEmpty()) return false;
    auto o = std::make_unique<Opened>();
    QString problem;
    if (!openWorkbook(workbook, p.sheet, o.get(), &problem)) return false;
    Grid grid;
    if (!gridOf(p, *o, &grid).isEmpty()) return false;
    const ChartData &was = grid.data;
    if (!chartDataIsUsable(data) || data.values.size() != was.values.size() || data.categories.size() != was.categories.size() || data.seriesNames.size() != was.seriesNames.size()) return false;
    SaveChanges changes;
    changes.activeTab = o->book.info().activeTab;
    changes.sharedStringsBefore = o->book.sharedStringsInFile();
    changes.cellFormatsBefore = o->book.cellFormatsInFile();
    // (a formula of the workbook that reads the cells is worked out again when the workbook is opened, so none shows the old numbers)
    changes.fullCalcOnLoad = true;
    SheetChanges &sheet = changes.sheets[o->index];
    std::map<std::pair<quint32, quint32>, SavedCell> edits;
    auto put = [&](quint32 row, quint32 col, const SavedValue &value) {
        SavedCell cell;
        cell.row = row;
        cell.col = quint16(col);
        if (const Cell *old = o->sheet.cells.find(row, col)) cell.style = old->style;
        cell.value = value;
        edits[{row, col}] = cell;
    };
    // Words typed into a cell: a number stays a number where the cell held one, else words (nothing: an empty cell).
    auto words = [&](const QString &raw, bool wasNumber) {
        const QString text = cleanChartText(raw);
        SavedValue v;
        bool ok = false;
        const double n = text.toDouble(&ok);
        if (wasNumber && ok && std::isfinite(n)) {
            v.kind = SavedValue::Kind::Number;
            v.number = n;
        } else if (text.isEmpty()) {
            v.kind = SavedValue::Kind::Empty;
        } else {
            v.kind = SavedValue::Kind::Text;
            v.text = text;
        }
        return v;
    };
    // Only what the person changed is compared and written, each as it was typed against each as it was read: not the text a change would be cleaned to.
    bool categoriesChanged = false, namesChanged = false;
    bool categoriesNumeric = !grid.categoryNumber.contains(false);
    QStringList categories(data.categories);
    for (int r = 0; r < categories.size(); ++r) {
        const bool changed = data.categories[r] != was.categories[r];
        categoriesChanged = categoriesChanged || changed;
        categories[r] = changed ? cleanChartText(data.categories[r]) : was.categories[r];
        if (changed && p.categories.set) {
            const auto [row, col] = p.categories.at(r, p.down);
            put(row, col, words(data.categories[r], grid.categoryNumber[r]));
        }
        bool ok = false;
        categories[r].toDouble(&ok);
        categoriesNumeric = categoriesNumeric && ok;
    }
    QStringList names(data.seriesNames);
    for (int s = 0; s < p.series.size(); ++s) {
        const bool changed = data.seriesNames[s] != was.seriesNames[s];
        namesChanged = namesChanged || changed;
        names[s] = changed ? cleanChartText(data.seriesNames[s]) : was.seriesNames[s];
        if (changed && p.names[s].set) put(p.names[s].range.row1, p.names[s].range.col1, words(data.seriesNames[s], grid.nameNumber[s]));
        for (int r = 0; r < data.values[s].size(); ++r) {
            if (data.values[s][r] == was.values[s][r]) continue;
            const auto [row, col] = p.values[s].at(r, p.down);
            SavedValue v;
            v.kind = SavedValue::Kind::Number;
            v.number = data.values[s][r];
            put(row, col, v);
        }
    }
    if (edits.empty() && !categoriesChanged && !namesChanged) return false;
    if (!edits.empty()) {
        for (const auto &e : edits) sheet.cells.push_back(e.second);
        QString why;
        QByteArray saved;
        if (!saveXlsx(o->book, changes, &saved, &why) || saved.isEmpty() || saved.size() > kMostEmbeddedWorkbookBytes) return false;
        if (newWorkbook) *newWorkbook = saved;
    } else if (newWorkbook) {
        *newWorkbook = workbook;
    }
    // The values the chart stores of the cells.
    for (int s = 0; s < p.series.size(); ++s) {
        ChartSeries &series = *p.series[s];
        series.values = data.values[s];
        series.name = names[s];
        const bool whole = series.categories.size() != categories.size();
        for (int r = 0; r < categories.size(); ++r)
            if (whole || data.categories[r] != was.categories[r]) {
                if (series.categories.size() != categories.size()) series.categories.resize(categories.size());
                series.categories[r] = categories[r];
            }
        if (series.categoryRefNumeric) {
            series.categoryRefNumeric = categoriesNumeric;
            series.categoryNumbers.clear();
            if (categoriesNumeric)
                for (const QString &c : categories) series.categoryNumbers << c.toDouble();
        }
    }
    chart = work;
    return true;
}

} // namespace jp::office::sheet
