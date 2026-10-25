#include "core/nametable.h"

namespace jp::crunch {

void NameTable::load(const QVector<jp::office::sheet::DefinedName> &file)
{
    m_entries.clear();
    m_byKey.clear();
    m_base.clear();
    m_alive = 0;
    for (const jp::office::sheet::DefinedName &n : file) {
        if (n.name.startsWith(QLatin1String("_xlnm."), Qt::CaseInsensitive)) continue;
        const QString key = keyOf(n.localSheet, n.name);
        if (m_byKey.contains(key)) continue;
        NameEntry e;
        e.name = n.name;
        e.scope = n.localSheet;
        e.comment = n.comment;
        e.hidden = n.hidden;
        e.base = int(m_base.size());
        m_base.push_back({n.name, n.localSheet, n.ref, n.comment, n.hidden});
        m_byKey.insert(key, int(m_entries.size()));
        m_entries.push_back(e);
        ++m_alive;
    }
    ++m_generation;
}

int NameTable::find(int scope, const QString &name) const
{
    const auto at = m_byKey.constFind(keyOf(scope, name));
    return at == m_byKey.constEnd() ? -1 : at.value();
}

int NameTable::add(const NameEntry &entry)
{
    m_entries.push_back(entry);
    const int id = int(m_entries.size()) - 1;
    if (entry.alive) {
        m_byKey.insert(keyOf(entry.scope, entry.name), id);
        ++m_alive;
    }
    ++m_generation;
    return id;
}

void NameTable::put(int id, const NameEntry &entry)
{
    NameEntry &e = m_entries[size_t(id)];
    if (e.alive) {
        m_byKey.remove(keyOf(e.scope, e.name));
        --m_alive;
    }
    const int base = e.base;   // (what the file has of a name is the slot's, not the step's: the steps are made before saves and undone after them)
    e = entry;
    e.base = base;
    if (e.alive) {
        m_byKey.insert(keyOf(e.scope, e.name), id);
        ++m_alive;
    }
    ++m_generation;
}

} // namespace jp::crunch
