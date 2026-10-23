// Tests of the formula language (libs/base/calc): a grid of cells a formula can read, and a call that works a formula out and says the value in one line.
#pragma once

#include "calc/eval.h"
#include "calc/formula.h"
#include "calc/functions.h"
#include "calc/value.h"

#include <QHash>
#include <QMap>
#include <QString>
#include <QtTest>

#include <map>
#include <vector>

#include "../testrunner.h"

namespace calctest {

using jp::calc::ErrorKind;
using jp::calc::Value;

// A few sheets of cells in memory ("Sheet1" first), and defined names. The cell the formula is in is J10 unless a test says another.
class Grid : public jp::calc::CellSource {
public:
    Grid();
    int addSheet(const QString &name);
    // Puts a value in a cell ("B3"); `sheet` counts from 0.
    Grid &set(const QString &ref, const Value &v, int sheet = 0);
    Grid &n(const QString &ref, double v, int sheet = 0) { return set(ref, Value::number(v), sheet); }
    Grid &s(const QString &ref, const QString &v, int sheet = 0) { return set(ref, Value::text(v), sheet); }
    Grid &b(const QString &ref, bool v, int sheet = 0) { return set(ref, Value::boolean(v), sheet); }
    Grid &e(const QString &ref, ErrorKind v, int sheet = 0) { return set(ref, Value::error(v), sheet); }
    // Numbers down a column from `ref` (a number, or text for a string, or "" for an empty cell).
    Grid &column(const QString &ref, const QVariantList &values, int sheet = 0);
    Grid &row(const QString &ref, const QVariantList &values, int sheet = 0);
    // A defined name; `sheet` -1 for the workbook's.
    Grid &name(const QString &name, const QString &formula, int sheet = -1);

    int sheetCount() const override;
    int sheetIndex(const QString &name) const override;
    Value cell(int sheet, quint32 row, quint32 col) const override;
    void visit(int sheet, quint32 row1, quint32 col1, quint32 row2, quint32 col2, const std::function<bool(quint32, quint32, const Value &)> &f) const override;
    void usedExtent(int sheet, quint32 *rows, quint32 *cols) const override;
    QString definedName(int sheet, const QString &name) const override;

private:
    struct Sheet {
        QString name;
        std::map<std::pair<quint32, quint32>, Value> cells;
    };
    std::vector<Sheet> m_sheets;
    QMap<QString, QString> m_names;   // "<sheet>|<NAME>" -> formula
};

struct Settings {
    QString at = QStringLiteral("J10");      // the cell the formula is in
    bool array = false;                      // an array formula
    bool r1c1 = false;
    jp::calc::DateSystem system = jp::calc::DateSystem::Date1900;
    double now = 45366.5;                    // March 15, 2024, noon
    int shiftRows = 0, shiftCols = 0;
};

// The value of a formula, as Value::debug() says it; "!parse: ..." for text that is no formula, "!unsupported: NAME" for a function the registry lacks.
QString ev(const QString &formula, const Grid &grid, const Settings &settings = Settings());
QString ev(const QString &formula);
// The same, and how the evaluation ended; `limit` is the most steps it may take (0: the engine's own).
QString evStatus(const QString &formula, const Grid &grid, const Settings &settings, jp::calc::EvalStatus *status, qint64 limit = 0);
// The same for a number, to 15 digits: "n:" and the digits.
QString num(double v);

} // namespace calctest

using namespace calctest;
