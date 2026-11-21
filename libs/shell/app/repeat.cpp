#include "app/repeat.h"

#include <QCoreApplication>

namespace jp {

RepeatCommand::RepeatCommand(QObject *parent) : QObject(parent) {}

void RepeatCommand::record(const QString &name, std::function<void()> again, quint64 document)
{
    if (m_running) return;
    if (document != 0 && document != m_document) return;   // (given in a document that is gone)   // (a command done again is the one already kept)
    m_name = name;
    m_again = std::move(again);
    Q_EMIT changed();
}

void RepeatCommand::setDocument(quint64 token)
{
    if (token == m_document) return;
    m_document = token;
    clear();
}

void RepeatCommand::clear()
{
    if (!m_again && m_name.isEmpty()) return;
    m_name.clear();
    m_again = nullptr;
    Q_EMIT changed();
}

bool RepeatCommand::run()
{
    if (!m_again || m_running) return false;
    m_running = true;
    const std::function<void()> again = m_again;   // (a copy: the function may be replaced while it runs)
    again();
    m_running = false;
    return true;
}

void RepeatCommand::setRedoAvailable(bool has)
{
    if (m_redoAvailable == has) return;
    m_redoAvailable = has;
    Q_EMIT changed();
}

QString RepeatCommand::tip() const
{
    return m_again ? QCoreApplication::translate("Repeat", "Repeat %1 (F4)").arg(m_name) : QCoreApplication::translate("Repeat", "Repeat (F4)");
}

} // namespace jp
