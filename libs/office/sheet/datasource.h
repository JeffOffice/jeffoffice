#pragma once
// Rows of values read from a JSON or XML file, for Data > Get Data. A file is local, is read once up to a stated size, and is never trusted: every count below is checked before
// the thing it counts is made. JSON: an array of objects is a list; each key is a column, an object inside is flattened as key.subkey down to maxDepth, and a list of plain values is
// joined into one cell or spread over columns. XML: an element that repeats under the same parent is a list; its attributes and its child elements that hold only text are the columns.
// An XML file with a document type or an entity is refused (nothing is looked up, nothing is expanded).

#include "sheet/datastepmodel.h"

#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QVector>

#include <functional>

namespace jp::office::sheet {

struct DataLimits {
    qint64 maxBytes = qint64(64) << 20;   // the size of the file
    int maxRows = 200000;
    int maxCols = 256;
    int maxDepth = 4;                     // objects inside objects that are flattened; deeper ones are kept as text
    int maxXmlDepth = 64;                 // elements inside elements
    int maxStringChars = 32767;           // what a cell holds
    int maxLists = 64;                    // lists offered to pick from
    int arrayColumns = 16;                // columns a list of plain values is spread over
};

enum class ArrayMode { Join, Columns };

// One cell as read. kind: 0 empty, 'n' number, 'b' true or false, 's' text. XML values are all text (the person's choice of types is made later, from the text).
struct DataCell {
    QString text;
    char kind = 0;
};

struct DataRows {
    QStringList header;
    QVector<QVector<DataCell>> rows;
    QStringList notes;   // what was left out or cut, in words
};

// The lists a file offers: JSON paths ("$" for a top-level array, "$.items" for a key), or XML paths ("/feed/entry"). Empty with *why set when the file can't be read.
QStringList dataJsonLists(const QByteArray &bytes, const DataLimits &limits, QString *why);
QStringList dataXmlLists(const QByteArray &bytes, const DataLimits &limits, QString *why);

// Reads the list at path; maxRows <= 0 reads all (up to limits.maxRows), a preview passes the few rows it shows. False with *why set when nothing can be loaded.
bool dataReadJson(const QByteArray &bytes, const QString &path, ArrayMode mode, int maxRows, const DataLimits &limits, DataRows *out, QString *why);
bool dataReadXml(const QByteArray &bytes, const QString &path, int maxRows, const DataLimits &limits, DataRows *out, QString *why);

// What a table made by Get Data remembers of where its rows came from, as the text kept in the table's part and read back. kind is "json", "xml", or "workbook"; list is the JSON or XML path, or for
// a workbook "sheet:Name", "table:Name", or "name:Name"; asText is the choice to load every column as text.
struct DataConnection {
    QString kind, path, list;
    ArrayMode mode = ArrayMode::Join;
    bool asText = false;
    QString name;          // what the person calls the query (empty: the name of its table)
    qint64 ranAt = 0;      // when it last ran (seconds since 1970, UTC); 0 when not known
    qint64 rowCount = -1;  // the rows it gave then; -1 when not known
    QVector<DataStep> steps;   // what is done to the rows the file gives, in order (datasteps.h)
    QString stepsText;     // the steps as they were saved (JSON); kept as they were when they can't be read
    QString stepsProblem;  // why the steps can't be run, in words; empty when they can
    QStringList columns;   // the names the file's columns had in the table when it was made, so that Refresh knows which columns are the file's and which the person added (left out when too many to keep)
    QString text() const;
    static bool parse(const QString &text, DataConnection *out);
};

// A column's kind from what it holds: 't' text, 'n' number, 'b' true or false, 'd' date (the year-month-day form, with a time or not). Everything is 't' when asText. A column of numbers with a leading
// zero or more than 15 digits stays text (an identifier is not a quantity).
QVector<char> dataColumnKinds(const DataRows &table, bool asText);

// The bytes of a local file, read in pieces so that `cancelled` is asked as it goes. A web address, a path on another computer, a folder, and a file larger than the limit are refused, with *why in words.
bool dataReadFile(const QString &path, const DataLimits &limits, const std::function<bool()> &cancelled, QByteArray *bytes, QString *why);

// The place a path names, with links and ".." worked out when the file is there (else the cleaned full path): what a person is shown and says yes to, and what is then opened.
QString dataReadablePath(const QString &path);

// Whether the text names a place that is not a file on this computer's own disks (a web address, a network path).
bool dataPathIsRemote(const QString &path);

}   // namespace jp::office::sheet
