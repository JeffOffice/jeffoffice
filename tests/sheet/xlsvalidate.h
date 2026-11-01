// A check of the structure of an .xls file that a writer made, written from the rules of the format and from the files a real program writes, and not from this program's own reader (which
// reads what the writer writes, and so cannot see a record the writer makes wrongly): the sizes of the records, the order they come in, the offsets and counts and indexes that point at
// other records, and the tokens of every formula and name.
#pragma once

#include <QByteArray>
#include <QString>

namespace xlstest {

// Empty if the file passes; else the first thing found wrong, in words.
QString validateXls(const QByteArray &file);

// The tokens of one formula, walked: false, with `why`, if a token runs past the end, is not one the format has, or leaves the wrong number of operands (`externalSheets` and `names` bound the
// indexes of references through the list of external sheets and of names; negative: not checked).
bool validateTokens(const QByteArray &rgce, int externalSheets, int names, QString *why);

} // namespace xlstest
