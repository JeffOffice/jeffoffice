#pragma once
// The defined names of an open workbook as a person sees them: each name with the sheet it belongs to (or the whole workbook), what it is for, and whether it is hidden. What a name stands for
// (the text of its formula) is the calculation's (Calculation::nameText), which follows the cells as they are moved; the table keeps the rest, and what the file has of each name, so that saving
// can tell what a person made, changed, or took away. A name is never taken out of the table: one that is taken away is `alive` false, and one that is made is put at the end, so that the
// steps of Undo can name a name by its place. No GUI.

#include "sheet/sheetreader.h"

#include <QHash>
#include <QString>
#include <QVector>

#include <vector>

namespace jp::crunch {

struct NameEntry {
    QString name;          // as it is written
    int scope = -1;        // the place of the sheet it belongs to; -1 for the workbook
    QString comment;
    bool hidden = false;
    bool alive = true;
    int base = -1;         // its place among the names the file has (see NameTable::base), -1 for one made since
    bool operator==(const NameEntry &o) const { return name == o.name && scope == o.scope && comment == o.comment && hidden == o.hidden && alive == o.alive && base == o.base; }
};

// A name as the file has it.
struct NameBase {
    QString name;
    int scope = -1;
    QString text, comment;
    bool hidden = false;
};

class NameTable {
public:
    // The names of a file (those the program keeps for itself, such as the print areas, are not in the table; of two with the same scope and name the first stands).
    void load(const QVector<jp::office::sheet::DefinedName> &file);
    // The place of the name that is alive with this scope and name (any case); -1 for none.
    int find(int scope, const QString &name) const;
    int slots() const { return int(m_entries.size()); }
    const NameEntry &at(int id) const { return m_entries[size_t(id)]; }
    int aliveCount() const { return m_alive; }
    // Makes a name (at the end); its place.
    int add(const NameEntry &entry);
    // Puts the entry in the place (it may be another name, or alive or not).
    void put(int id, const NameEntry &entry);
    // What the file has: the names in the order it has them (each table entry that came from it says which). `rebase` makes the table's names what the file has (after a save); `textOf`
    // says what a name stands for now.
    const std::vector<NameBase> &base() const { return m_base; }
    template <typename F>
    void rebase(F textOf)
    {
        m_base.clear();
        for (NameEntry &e : m_entries) {
            if (!e.alive) {
                e.base = -1;
                continue;
            }
            e.base = int(m_base.size());
            m_base.push_back({e.name, e.scope, textOf(e), e.comment, e.hidden});
        }
    }
    // Changes whenever the table does (what is kept from it, a list of names to offer, is made again).
    qint64 generation() const { return m_generation; }
    // The key of the name in the calculation.
    static QString keyOf(int scope, const QString &name) { return QString::number(scope) + QLatin1Char('|') + name.toUpper(); }

private:
    std::vector<NameEntry> m_entries;
    QHash<QString, int> m_byKey;   // the names that are alive
    std::vector<NameBase> m_base;
    int m_alive = 0;
    qint64 m_generation = 0;
};

} // namespace jp::crunch
