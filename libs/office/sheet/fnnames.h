#pragma once
// The names of functions, the error values, and TRUE and FALSE in the languages the apps are shown in (resources/functionnames.txt: one row for a name, a column for a language).
// A file always holds the English names; these are what a person types and reads in a formula when the program is shown in their language. A name with no local form keeps
// its English one. No GUI.

#include <QString>
#include <QStringList>

namespace jp::office::sheet {

// The column of the table for a display language ("de", "pt_PT", "es_ES" is "es"); empty for English and for a language the table has none for.
QString formulaLanguageOf(const QString &uiLanguage);
// The columns, in the table's order.
QStringList formulaNameLanguages();

// The local name for an English one (a function, an error such as "#N/A", "TRUE"), or the English one when the language has none.
QString localFormulaName(const QString &english, const QString &language);
// The English name for a local one (any case), or empty when it is not a local name of that language.
QString englishFormulaName(const QString &local, const QString &language);

// The other local names a function is read by (a language has two traditions for some names; the first, shown one is localFormulaName).
QStringList formulaNameAlternatives(const QString &english, const QString &language);
// The names that mean two things in a table (the shipped one, or the text of one in the same format): empty when it is sound.
QStringList formulaNameProblems();
QStringList formulaNameProblems(const QString &tableText);

// For the tests and the lists: the English names the table has a row for, and how many of them have a local name in the language.
QStringList formulaNameRows();
int localFormulaNameCount(const QString &language);
// Whether the table says the English name is kept in every language.
bool formulaNameKeptEnglish(const QString &english);
// The longest a name may be for it to be looked up (a longer word in a formula is left as it is).
constexpr int kMaxFormulaNameChars = 255;

} // namespace jp::office::sheet
