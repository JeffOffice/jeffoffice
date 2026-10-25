#pragma once
// Helping a person write a formula: where the cursor is in the text being typed (the word it is at the end of, the function whose brackets it is in, and which of its arguments), the
// functions and names that word may be the beginning of, and the argument list of a function with the argument the cursor is at marked. No GUI.

#include "core/functioninfo.h"

#include <QChar>
#include <QLocale>
#include <QString>
#include <QVector>

#include <vector>

namespace jp::crunch {

class NameTable;

struct FormulaSpot {
    bool formula = false;      // the entry is a formula (it begins with "=")
    bool inText = false;       // the cursor is in a text in quotes or a sheet's name in quotes, or between the brackets of a table's column: nothing is completed there
    int wordStart = 0;         // where the word the cursor is at the end of begins (the cursor's place when there is none)
    QString word;              // that word: letters, digits, "_", "." (and "\")
    bool canComplete = false;  // the word is where a function or a name may be written: after "=", an operator, an opening bracket, or a separator
    // The cursor is in the brackets of a table's column, as in Sales[Am| or Sales[[#Headers],[Am|: `table` is the name before the brackets (empty when there is none, as in [@Am|: the
    // table the formula's cell is in), `columnWord` what is typed of the column's name, and `columnStart` where it begins. `inText` is set too: no function or name is completed there.
    bool inColumns = false;
    QString table, columnWord;
    int columnStart = 0;
    // The cursor is where a reference may be written (after "=", an operator, an opening bracket, or a separator, with no word begun): a click on a cell is put there.
    bool canPoint = false;
    QString function;          // the function whose brackets the cursor is in (the nearest; empty when it is in none)
    int argument = 0;          // and which of its arguments, from 0
};

// `separator` is what stands between arguments: "," as a file writes it, or ";" in a locale that writes a comma for the decimal point.
FormulaSpot spotAt(const QString &text, int cursor, QChar separator);
// The separator a person's locale writes between arguments.
QChar argumentSeparator(const QLocale &locale);

// What the list offers: a function, a name, or a table.
struct Completion {
    enum class Kind : quint8 { Function, Name, Table, Column, SpecialItem } kind = Kind::Function;
    QString text;      // what is put in the formula
    int id = -1;       // a name: its place in the name table
};
// The most that are offered at once, and the most names that are looked at to offer them.
constexpr int kMaxOffered = 12;
constexpr int kMaxNamesIndexed = 20000;

// Every function and name an entry may use, kept so that the ones that begin with a word are found without looking at the rest.
class CompletionIndex {
public:
    // Made from the table (the names of the workbook, and of `sheet`; at most kMaxNamesIndexed of them) and the names of tables.
    void build(const NameTable &names, int sheet, const QStringList &tables);
    // The ones that begin with `word` (any case), in the order of their names, at most `cap`.
    std::vector<Completion> matches(const QString &word, int cap = kMaxOffered) const;
    int nameCount() const { return int(m_names.size()); }
    // The columns of the tables, for what is typed inside their brackets: by the table's name (any case); at most kMaxTablesIndexed tables and kMaxColumnsIndexed columns of each.
    static constexpr int kMaxTablesIndexed = 2000;
    static constexpr int kMaxColumnsIndexed = 1000;
    void setTableColumns(const std::vector<std::pair<QString, QStringList>> &tables);
    // The table a name-less reference ([@Amount]) is of: the table the formula's cell is in (empty for none).
    void setCellTable(const QString &table) { m_cellTable = table.toUpper(); }
    // The columns of `table` (the cell's, when empty) that begin with the word, then the special items (#All, #Data, #Headers, #Totals, #This Row) that do, at most `cap`.
    std::vector<Completion> columnMatches(const QString &table, const QString &word, int cap = kMaxOffered) const;

private:
    struct Entry {
        QString upper, shown;
        int id;
        bool table;
    };
    std::vector<Entry> m_names;   // sorted by `upper`
    std::vector<std::pair<QString, QStringList>> m_tableColumns;   // by the table's name in capitals, sorted
    QString m_cellTable;
};

// "SUM(number1, [number2], ...)" with the argument the cursor is at in bold (html, texts escaped); the separator is the one the person types. The last argument is the one for every
// argument after it when the list ends in "...".
QString hintHtml(const FunctionDoc &doc, int argument, QChar separator);
// The same without the marks, and the number of the argument that is marked (-1 for none).
QString hintPlain(const FunctionDoc &doc, int argument, QChar separator, int *marked = nullptr);

} // namespace jp::crunch
