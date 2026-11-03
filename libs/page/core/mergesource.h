#pragma once
// The data of a mail merge: a list of recipients read from a file (a CSV or tab-separated text file, a spreadsheet (.xlsx, .xlsm, .xls, .ods), or a contacts file (.vcf)), and written
// back as a CSV file. The first row of a file of columns holds the column names.

#include "core/document.h"

#include <QString>
#include <QStringList>
#include <QVector>

namespace jp {

// The rows of text of a file of delimited values, with quotes as CSV has them.
QVector<QStringList> parseDelimited(const QString &text, QChar sep);
// The first sheet of a spreadsheet file as rows of text, the cells as they show (a number in its format). False, with the reason, where the file can't be read.
bool readSpreadsheetRows(const QString &path, QVector<QStringList> *rows, QString *error);
bool loadMergeSource(const QString &path, MergeSource *out, QString *error);
bool saveMergeCsv(const MergeSource &m, const QString &path);

} // namespace jp
