#pragma once
// The property sets of the old binary formats (the streams "\005SummaryInformation" and "\005DocumentSummaryInformation" of a compound file): a property is a number, a text, or a
// time. Reading follows the property set format's sections 2.20 and 2.21 (the stream: one or two sets), 2.15 (typed values), 2.5 (strings in the set's code page, or in 16 bits when
// the page is the Unicode one), and 2.18.2 (the code page property). The stream comes from a stranger: every size and offset is held to the stream, and the numbers of sets, properties,
// and characters are capped.

#include <QByteArray>
#include <QDateTime>
#include <QMap>
#include <QString>
#include <QVector>

namespace jp::office {

struct OleProperty {
    quint16 type = 0;       // the value's type (2 and 3: whole numbers, 0x1E, 0x1F, and 8: texts, 0x40: a time)
    qint64 number = 0;      // a whole number; or, of a time, its 100-nanosecond ticks (an amount of time, not a date, for the property that says how long a document was edited)
    QString text;
    QDateTime time;         // a time of day on the date (UTC); not valid for ticks before 1970 or none
};

struct OlePropertySet {
    QByteArray format;                   // the 16 bytes that name the kind of set
    int codePage = 0;                    // the code page of its texts (0: the set says none)
    QMap<quint32, OleProperty> values;   // by property number
};

constexpr qint64 kMaxOlePropertyStream = 2097152;
constexpr int kMaxOleProperties = 4096;
constexpr int kMaxOlePropertyChars = 65536;

// The sets of a stream (none if it is not one, or is larger than kMaxOlePropertyStream).
QVector<OlePropertySet> readOlePropertySets(const QByteArray &stream);

// The 16 bytes (as a GUID is stored) of the two kinds of set that hold a document's properties.
QByteArray summaryInformationFormat();
QByteArray documentSummaryInformationFormat();

} // namespace jp::office
