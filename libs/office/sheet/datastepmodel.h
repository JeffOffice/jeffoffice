#pragma once
// A step of a query made by Data > Get Data (see datasteps.h): one operation of a short, closed list, with typed arguments. This part is the model and its saved form (JSON), kept apart so that the
// connection text (datasource.h) can hold the steps.

#include <QString>
#include <QStringList>
#include <QVector>

namespace jp::office::sheet {

struct DataStep {
    enum class Op : quint8 { Keep, Remove, Rename, Filter, Sort, Kind };
    Op op = Op::Keep;
    QStringList cols;       // Keep: the columns to keep, in this order; Remove: the columns to take out
    QString col;            // Rename: the column; Filter, Sort, Kind: the column
    QString to;             // Rename: the new name
    QString cmp;            // Filter: eq, ne, contains, notcontains, gt, ge, lt, le, blank, notblank
    QString value;          // Filter: what to compare with
    bool descending = false;   // Sort
    QString kind;           // Kind: text, number, or date
    QString order;          // Kind date: MDY, DMY, or YMD
    QString decimal, group; // Kind number: the marks the values are written with ("." and "," if empty)
    QString label;          // what the person calls the step (empty: the step's own description)
    bool operator==(const DataStep &o) const;
};

// The most characters of a query's text (datasource.h) kept in a table's part: the file, the choices, the column names, and the steps. A longer one is refused when it is made, never cut.
constexpr int kMaxDataSourceChars = 400000;
constexpr int kMaxDataSteps = 100;
constexpr int kMaxDataStepChars = 1000;      // a name or a value in a step
constexpr int kMaxDataStepsJsonChars = 100000;
// The most cell visits the steps of one run may make (rows times steps, counted before each step runs).
constexpr qint64 kMaxDataStepWork = 200000000;

// The steps as JSON text, and back. parseDataSteps says in words (in *why) what is wrong with text that is not a list of known steps; a step from a newer version (an op not in the list) is such a case.
QString dataStepsToJson(const QVector<DataStep> &steps);
bool parseDataSteps(const QString &json, QVector<DataStep> *out, QString *why, bool *newer = nullptr);   // *newer: the step is of a kind this version does not have

// One line in words for a step: "Choose columns: a, b" and the like (its label, if it has one).
QString dataStepTitle(const DataStep &step);

}   // namespace jp::office::sheet
