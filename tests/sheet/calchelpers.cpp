#include "calchelpers.h"

#include "sheet/cellref.h"

namespace calctest {

Grid::Grid() { addSheet(QStringLiteral("Sheet1")); }

int Grid::addSheet(const QString &name)
{
    Sheet s;
    s.name = name;
    m_sheets.push_back(s);
    return int(m_sheets.size()) - 1;
}

Grid &Grid::set(const QString &ref, const Value &v, int sheet)
{
    quint32 row = 0, col = 0;
    if (!jp::office::sheet::parseCellRef(ref, &row, &col)) qFatal("bad reference in a test: %s", qPrintable(ref));
    m_sheets[size_t(sheet)].cells[{row, col}] = v;
    return *this;
}

static Value fromVariant(const QVariant &v)
{
    if (v.typeId() == QMetaType::QString) return v.toString().isEmpty() ? Value() : Value::text(v.toString());
    if (v.typeId() == QMetaType::Bool) return Value::boolean(v.toBool());
    return Value::number(v.toDouble());
}

Grid &Grid::column(const QString &ref, const QVariantList &values, int sheet)
{
    quint32 row = 0, col = 0;
    if (!jp::office::sheet::parseCellRef(ref, &row, &col)) qFatal("bad reference in a test: %s", qPrintable(ref));
    for (const QVariant &v : values) {
        const Value value = fromVariant(v);
        if (!value.isEmpty()) m_sheets[size_t(sheet)].cells[{row, col}] = value;
        ++row;
    }
    return *this;
}

Grid &Grid::row(const QString &ref, const QVariantList &values, int sheet)
{
    quint32 row = 0, col = 0;
    if (!jp::office::sheet::parseCellRef(ref, &row, &col)) qFatal("bad reference in a test: %s", qPrintable(ref));
    for (const QVariant &v : values) {
        const Value value = fromVariant(v);
        if (!value.isEmpty()) m_sheets[size_t(sheet)].cells[{row, col}] = value;
        ++col;
    }
    return *this;
}

Grid &Grid::name(const QString &name, const QString &formula, int sheet)
{
    m_names[QString::number(sheet) + QLatin1Char('|') + name.toUpper()] = formula;
    return *this;
}

Grid &Grid::subtotal(const QString &ref, int sheet)
{
    quint32 row = 0, col = 0;
    if (!jp::office::sheet::parseCellRef(ref, &row, &col)) qFatal("bad reference in a test: %s", qPrintable(ref));
    m_subtotals.insert(QStringLiteral("%1|%2|%3").arg(sheet).arg(row).arg(col));
    return *this;
}

bool Grid::isSubtotal(int sheet, quint32 row, quint32 col) const { return m_subtotals.contains(QStringLiteral("%1|%2|%3").arg(sheet).arg(row).arg(col)); }

int Grid::sheetCount() const { return int(m_sheets.size()); }

int Grid::sheetIndex(const QString &name) const
{
    for (size_t i = 0; i < m_sheets.size(); ++i)
        if (m_sheets[i].name.compare(name, Qt::CaseInsensitive) == 0) return int(i);
    return -1;
}

Value Grid::cell(int sheet, quint32 row, quint32 col) const
{
    if (sheet < 0 || sheet >= int(m_sheets.size())) return Value();
    const auto &cells = m_sheets[size_t(sheet)].cells;
    const auto it = cells.find({row, col});
    return it == cells.end() ? Value() : it->second;
}

void Grid::visit(int sheet, quint32 row1, quint32 col1, quint32 row2, quint32 col2, const std::function<bool(quint32, quint32, const Value &)> &f) const
{
    if (sheet < 0 || sheet >= int(m_sheets.size())) return;
    for (const auto &entry : m_sheets[size_t(sheet)].cells) {
        const quint32 r = entry.first.first, c = entry.first.second;
        if (r < row1 || r > row2 || c < col1 || c > col2) continue;
        if (!f(r, c, entry.second)) return;
    }
}

void Grid::usedExtent(int sheet, quint32 *rows, quint32 *cols) const
{
    *rows = *cols = 0;
    if (sheet < 0 || sheet >= int(m_sheets.size())) return;
    for (const auto &entry : m_sheets[size_t(sheet)].cells) {
        *rows = qMax(*rows, entry.first.first + 1);
        *cols = qMax(*cols, entry.first.second + 1);
    }
}

QString Grid::definedName(int sheet, const QString &name) const
{
    const QString key = name.toUpper();
    if (sheet >= 0) {
        const auto own = m_names.find(QString::number(sheet) + QLatin1Char('|') + key);
        if (own != m_names.end()) return own.value();
    }
    return m_names.value(QStringLiteral("-1|") + key);
}

QString ev(const QString &formula, const Grid &grid, const Settings &settings)
{
    jp::calc::EvalStatus status;
    return evStatus(formula, grid, settings, &status);
}

static jp::calc::ParseResult parseAt(const QString &formula, const Settings &settings, quint32 *row, quint32 *col)
{
    using namespace jp::calc;
    ParseOptions options;
    options.r1c1 = settings.r1c1;
    options.arrayFormula = settings.array;
    *row = *col = 0;
    jp::office::sheet::parseCellRef(settings.at, row, col);
    options.row = *row;
    options.col = *col;
    return parseFormula(formula, options);
}

static jp::calc::EvalContext contextAt(const Grid &grid, const Settings &settings, quint32 row, quint32 col)
{
    jp::calc::EvalContext ctx;
    ctx.source = &grid;
    ctx.sheet = 0;
    ctx.row = row;
    ctx.col = col;
    ctx.shiftRows = settings.shiftRows;
    ctx.shiftCols = settings.shiftCols;
    ctx.dateSystem = settings.system;
    ctx.now = settings.now;
    return ctx;
}

QString evStatus(const QString &formula, const Grid &grid, const Settings &settings, jp::calc::EvalStatus *status, qint64 limit)
{
    using namespace jp::calc;
    quint32 row = 0, col = 0;
    const ParseResult parsed = parseAt(formula, settings, &row, &col);
    if (!parsed.ok()) return QStringLiteral("!parse: ") + parsed.error;
    if (!parsed.program->supported()) return QStringLiteral("!unsupported: ") + parsed.program->unknownFunctions.join(QLatin1Char(','));
    Evaluator evaluator;
    evaluator.seedRandom(1);
    if (limit > 0) evaluator.setWorkLimit(limit);
    const QString result = evaluator.evaluate(*parsed.program, contextAt(grid, settings, row, col)).debug();
    *status = evaluator.status();
    return result;
}

QStringList evInCells(const QString &formula, const Grid &grid, const QStringList &cells, const Settings &settings)
{
    using namespace jp::calc;
    QStringList results;
    Evaluator evaluator;
    evaluator.seedRandom(1);
    for (const QString &cell : cells) {
        Settings here = settings;
        here.at = cell;
        quint32 row = 0, col = 0;
        const ParseResult parsed = parseAt(formula, here, &row, &col);
        if (!parsed.ok()) {
            results << QStringLiteral("!parse: ") + parsed.error;
            continue;
        }
        results << evaluator.evaluate(*parsed.program, contextAt(grid, here, row, col)).debug();
    }
    return results;
}

QString ev(const QString &formula) { return ev(formula, Grid()); }

QString num(double v) { return QStringLiteral("n:") + QString::number(v, 'g', 15); }

} // namespace calctest
