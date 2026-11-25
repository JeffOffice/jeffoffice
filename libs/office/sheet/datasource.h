#pragma once
// Rows of values read from a JSON or XML file, for Data > Get Data. A file is local, is read once up to a stated size, and is never trusted: every count below is checked before
// the thing it counts is made. JSON: an array of objects is a list; each key is a column, an object inside is flattened as key.subkey down to maxDepth, and a list of plain values is
// joined into one cell or spread over columns. XML: an element that repeats under the same parent is a list; its attributes and its child elements that hold only text are the columns.
// An XML file with a document type or an entity is refused (nothing is looked up, nothing is expanded).

#include <QString>
#include <QStringList>
#include <QVector>

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

struct DataTable {
    QStringList header;
    QVector<QVector<DataCell>> rows;
    QStringList notes;   // what was left out or cut, in words
};

// The lists a file offers: JSON paths ("$" for a top-level array, "$.items" for a key), or XML paths ("/feed/entry"). Empty with *why set when the file can't be read.
QStringList dataJsonLists(const QByteArray &bytes, const DataLimits &limits, QString *why);
QStringList dataXmlLists(const QByteArray &bytes, const DataLimits &limits, QString *why);

// Reads the list at path; maxRows <= 0 reads all (up to limits.maxRows), a preview passes the few rows it shows. False with *why set when nothing can be loaded.
bool dataReadJson(const QByteArray &bytes, const QString &path, ArrayMode mode, int maxRows, const DataLimits &limits, DataTable *out, QString *why);
bool dataReadXml(const QByteArray &bytes, const QString &path, int maxRows, const DataLimits &limits, DataTable *out, QString *why);

}   // namespace jp::office::sheet
