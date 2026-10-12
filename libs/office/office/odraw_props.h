#pragma once
// A shape's properties in a binary drawing ([MS-ODRAW] 2.3): the tables of its
// property records, each with the complex data that follows its table.

#include "office/odraw.h"

#include <QHash>
#include <QString>

namespace jp::office::odraw {

class Props {
public:
    // Adds the properties of one property record (0xF00B, 0xF121, or 0xF122);
    // a property in a later record replaces one of the same number.
    void add(const QByteArray &data, const OdrawRecord &record);
    bool has(quint16 id) const { return m_props.contains(id); }
    // The property's value (for a complex one, the length of its data).
    quint32 value(quint16 id, quint32 fallback = 0) const;
    qint32 integer(quint16 id, qint32 fallback = 0) const { return qint32(value(id, quint32(fallback))); }
    // A 16.16 fixed-point number.
    double fixed(quint16 id, double fallback = 0) const;
    // One flag of a boolean property: the value is `bit` of the low 16 bits,
    // and counts only when the use flag 16 bits above it is set.
    bool flag(quint16 id, quint32 bit, bool fallback) const;
    QByteArray complex(quint16 id) const { return m_props.value(id).complex; }
    // A complex property that holds UTF-16 text.
    QString text(quint16 id) const;

private:
    struct Prop {
        quint32 value = 0;
        QByteArray complex;
    };
    QHash<quint16, Prop> m_props;
};

// The elements of an array property's complex data (IMsoArray: a header of
// the number of elements, the number allocated, and the size of each, then the
// elements); the count is cut to what the data holds.
struct ArrayView {
    int count = 0;
    int size = 0;   // bytes in an element
    qint64 offset(int i) const { return 6 + qint64(i) * size; }
};
ArrayView arrayOf(const QByteArray &complex);

QString utf16Text(const QByteArray &bytes, qint64 offset = 0);

} // namespace jp::office::odraw
