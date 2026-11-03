#pragma once
// The data of a mail merge: a list of recipients read from a file (a CSV or tab-separated text file, a spreadsheet (.xlsx, .xlsm, .xls, .ods), or a contacts file (.vcf)), and written
// back as a CSV file. The first row of a file of columns holds the column names.

#include "core/document.h"

#include <QString>
#include <QStringList>
#include <QVector>

namespace jp {

// What a list may hold: the most columns, and the most cells in all (rows times columns, so a list of a hundred thousand recipients with a dozen columns fits, and a hostile file's wide
// header or far-away cell cannot make memory grow past about a few hundred megabytes). What is past them is left out, and the list says so (MergeSource::truncated).
constexpr int kMostMergeColumns = 1000;
constexpr qint64 kMostMergeCells = 4000000;
inline int mostMergeColumns() { return kMostMergeColumns; }
inline qint64 mostMergeCells() { return kMostMergeCells; }

// The rows of text of a file of delimited values, with quotes as CSV has them. A row keeps its first kMostMergeColumns cells, and the rows stop where the cells come to kMostMergeCells (`truncated` says so).
QVector<QStringList> parseDelimited(const QString &text, QChar sep, bool *truncated = nullptr);
// The first sheet of a spreadsheet file as rows of text, the cells as they show (a number in its format). False, with the reason, where the file can't be read.
bool readSpreadsheetRows(const QString &path, QVector<QStringList> *rows, QString *error, bool *truncated = nullptr);
bool loadMergeSource(const QString &path, MergeSource *out, QString *error);
bool saveMergeCsv(const MergeSource &m, const QString &path);

} // namespace jp
