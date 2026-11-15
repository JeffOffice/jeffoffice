#pragma once
// The OpenType choices of the Font box (its Advanced page): ligatures, number spacing and forms, a stylistic set, and contextual alternates. JeffWrite's Font
// box and the one of the page programs (JeffPub, JeffDeck) both hold it. A box shows what all the text selected has, or "(mixed)" where the text differs; left
// alone it changes nothing.

#include "core/export.h"

#include <QFont>
#include "app/uistyle.h"
#include <QList>
#include <QMap>
#include <QPair>
#include <QTextCharFormat>
#include <QVector>

class QComboBox;

namespace jp {

class JO_COMMON_EXPORT OpenTypeBox : public Section {
    Q_OBJECT
public:
    explicit OpenTypeBox(QWidget *parent = nullptr);
    // Shows what the formats have (the first's where they agree).
    void setFormats(const QVector<QTextCharFormat> &formats);
    // What the person changed: the properties to set (property, value), and those to clear (back to the font's own choice).
    struct Change {
        QList<QPair<int, int>> set;
        QList<int> clear;
        bool isEmpty() const { return set.isEmpty() && clear.isEmpty(); }
    };
    Change change() const;
    static void apply(const Change &change, QTextCharFormat *format);
    // The features the boxes ask for (a box left mixed asks for none), to show a preview with.
    QMap<QFont::Tag, quint32> features() const;

Q_SIGNALS:
    void edited();

private:
    struct Row {
        QComboBox *box = nullptr;
        int property = 0;
        int was = 0;       // the item shown first
        bool unknown = false;
    };
    Row m_ligatures, m_spacing, m_forms, m_set, m_alternates;
    QList<Row *> rows() { return {&m_ligatures, &m_spacing, &m_forms, &m_set, &m_alternates}; }
    QList<const Row *> rows() const { return {&m_ligatures, &m_spacing, &m_forms, &m_set, &m_alternates}; }
    static int valueAt(const Row &row, int index);   // the property value an item stands for (-1: none, the font's own)
    static int itemOf(const Row &row, int value);    // the item for a property value (-1: not one)
    void fill(Row &row, const QVector<QTextCharFormat> &formats);
};

} // namespace jp
