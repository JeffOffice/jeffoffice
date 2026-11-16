#include "sheet/chartembed.h"

#include "sheet/cellref.h"
#include "sheet/chartdata.h"
#include "sheet/xlsxsave.h"

#include <QCoreApplication>
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
QString notPlain() { return QCoreApplication::translate("jp::ChartData", "Some cells of the workbook kept with this chart hold a formula, a date, or something that is not a number or words, so its numbers can't be changed here."); }

// Opens the workbook with the limits of a chart's workbook: its file, what it comes to inflated, and the cells of a sheet (Crunch's own limit for the cells a chart reads).
bool openWorkbook(const QByteArray &bytes, const QString &sheetName, Opened *o, QString *problem)
{
    if (bytes.isEmpty() || bytes.size() > kMostEmbeddedWorkbookBytes) {
        *problem = bytes.isEmpty() ? QCoreApplication::translate("jp::ChartData", "The numbers of this chart are in a workbook that is kept with it, so they can't be changed here.") : unreadable();
        return false;
    }
    if (!o->package.readLazy(bytes, nullptr, kMostEmbeddedInflatedBytes)) {
        *problem = unreadable();
        return false;
    }
    Limits limits;
    limits.fileBytes = kMostEmbeddedWorkbookBytes;
    limits.cellsPerSheet = kMaxChartCells;
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
        *problem = unreadable();
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
    if (out->series.size() > kMostChartSeries) return QCoreApplication::translate("jp::ChartData", "This chart has more than %1 series or %2 points, or numbers that are not numbers, so its numbers can't be changed here.").arg(kMostChartSeries).arg(kMostChartCategories);
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
            if (v.count() > kMostChartCategories) return QCoreApplication::translate("jp::ChartData", "This chart has more than %1 series or %2 points, or numbers that are not numbers, so its numbers can't be changed here.").arg(kMostChartSeries).arg(kMostChartCategories);
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

// A cell's words (empty for a cell that is not there or holds nothing); false for one that holds more than words or a number.
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
        *text = QString::number(cell->number, 'g', 15);
        *number = true;
        return true;
    default: return false;
    }
}

bool cellNumber(const Opened &o, quint32 row, quint32 col, double *value)
{
    const Cell *cell = o.sheet.cells.find(row, col);
    if (!cell || cell->hasFormula() || cell->type != CellType::Number || !std::isfinite(cell->number)) return false;
    *value = cell->number;
    return true;
}

// The grid the cells make.
QString gridOf(const Plan &p, const Opened &o, ChartData *data, bool *numericCategories)
{
    ChartData out;
    const quint32 rows = o.sheet.extentRows, cols = o.sheet.extentCols;
    auto inside = [&](const Line &l) { return l.range.row2 < rows && l.range.col2 < cols; };
    *numericCategories = false;
    if (p.categories.set) {
        if (!inside(p.categories)) return notFollowed();
        int numbers = 0;
        for (qint64 i = 0; i < p.categories.count(); ++i) {
            const auto [row, col] = p.categories.at(i, p.down);
            QString text;
            bool number = false;
            if (!cellText(o, row, col, &text, &number)) return notPlain();
            numbers += number ? 1 : 0;
            out.categories << text;
        }
        *numericCategories = numbers > 0 && numbers == out.categories.size();
    } else {
        out.categories = p.series.first()->categories;
        if (out.categories.size() > p.points) return notFollowed();
        while (out.categories.size() < p.points) out.categories << QString();
    }
    for (int s = 0; s < p.series.size(); ++s) {
        if (!inside(p.values[s])) return notFollowed();
        QVector<double> values;
        for (qint64 i = 0; i < p.values[s].count(); ++i) {
            const auto [row, col] = p.values[s].at(i, p.down);
            double v = 0;
            if (!cellNumber(o, row, col, &v)) return notPlain();
            values << v;
        }
        out.values << values;
        if (p.names[s].set) {
            if (!inside(p.names[s])) return notFollowed();
            QString text;
            bool number = false;
            if (!cellText(o, p.names[s].range.row1, p.names[s].range.col1, &text, &number)) return notPlain();
            out.seriesNames << text;
        } else {
            out.seriesNames << p.series[s]->name;
        }
    }
    if (!chartDataIsUsable(out)) return QCoreApplication::translate("jp::ChartData", "This chart has more than %1 series or %2 points, or numbers that are not numbers, so its numbers can't be changed here.").arg(kMostChartSeries).arg(kMostChartCategories);
    *data = out;
    return QString();
}

} // namespace

QString embeddedChartProblem(const Chart &chart, const QByteArray &workbook, ChartData *data)
{
    Chart copy = chart;
    Plan p;
    if (const QString why = plan(copy, &p); !why.isEmpty()) return why;
    if (workbook.isEmpty()) return QCoreApplication::translate("jp::ChartData", "The numbers of this chart are in a workbook that is kept with it, so they can't be changed here.");
    auto o = std::make_unique<Opened>();
    QString problem;
    if (!openWorkbook(workbook, p.sheet, o.get(), &problem)) return problem;
    ChartData grid;
    bool numeric = false;
    if (const QString why = gridOf(p, *o, &grid, &numeric); !why.isEmpty()) return why;
    if (data) *data = grid;
    return QString();
}

bool embeddedChartSetData(Chart &chart, const QByteArray &workbook, const ChartData &data, QByteArray *newWorkbook)
{
    Chart work = chart;
    Plan p;
    if (!plan(work, &p).isEmpty() || workbook.isEmpty()) return false;
    auto o = std::make_unique<Opened>();
    QString problem;
    if (!openWorkbook(workbook, p.sheet, o.get(), &problem)) return false;
    ChartData was;
    bool numeric = false;
    if (!gridOf(p, *o, &was, &numeric).isEmpty()) return false;
    if (!chartDataIsUsable(data) || data.values.size() != was.values.size() || data.categories.size() != was.categories.size() || data.seriesNames.size() != was.seriesNames.size()) return false;
    SaveChanges changes;
    changes.activeTab = o->book.info().activeTab;
    changes.sharedStringsBefore = o->book.sharedStringsInFile();
    changes.cellFormatsBefore = o->book.cellFormatsInFile();
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
    auto words = [&](const QString &text, bool numberCells) {
        SavedValue v;
        bool ok = false;
        const double n = text.trimmed().toDouble(&ok);
        if (numberCells && ok && std::isfinite(n)) {
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
    QStringList categories = data.categories;
    for (QString &c : categories) c = cleanChartText(c);
    QStringList names = data.seriesNames;
    for (QString &n : names) n = cleanChartText(n);
    bool categoriesNumeric = numeric;
    for (int r = 0; r < categories.size(); ++r) {
        if (p.categories.set && categories[r] != was.categories[r]) {
            const auto [row, col] = p.categories.at(r, p.down);
            put(row, col, words(categories[r], numeric));
        }
        bool ok = false;
        categories[r].trimmed().toDouble(&ok);
        categoriesNumeric = categoriesNumeric && ok;
    }
    for (int s = 0; s < p.series.size(); ++s) {
        if (p.names[s].set && names[s] != was.seriesNames[s]) put(p.names[s].range.row1, p.names[s].range.col1, words(names[s], false));
        for (int r = 0; r < data.values[s].size(); ++r) {
            if (data.values[s][r] == was.values[s][r]) continue;
            const auto [row, col] = p.values[s].at(r, p.down);
            SavedValue v;
            v.kind = SavedValue::Kind::Number;
            v.number = data.values[s][r];
            put(row, col, v);
        }
    }
    if (edits.empty() && categories == was.categories && names == was.seriesNames) return false;
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
            if (whole || categories[r] != was.categories[r]) {
                if (series.categories.size() != categories.size()) series.categories.resize(categories.size());
                series.categories[r] = categories[r];
            }
        if (series.categoryRefNumeric) {
            series.categoryRefNumeric = categoriesNumeric;
            series.categoryNumbers.clear();
            if (categoriesNumeric)
                for (const QString &c : categories) series.categoryNumbers << c.trimmed().toDouble();
        }
    }
    chart = work;
    return true;
}

} // namespace jp::office::sheet
