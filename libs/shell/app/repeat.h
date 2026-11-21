#pragma once
// Repeat (F4, and Ctrl+Y when there is nothing to redo): does the last command again, on whatever is selected now. An app records, after each command that
// can be done again, a name for it and a function that does it on the current selection; this object keeps the latest one, offers it as a command that is
// enabled while there is one, and runs it. It holds nothing of the document: an app clears it when the document or what it can do changes.

#include "core/export.h"
#include <QObject>
#include <QString>
#include <functional>

namespace jp {

class JO_COMMON_EXPORT RepeatCommand : public QObject {
    Q_OBJECT
public:
    explicit RepeatCommand(QObject *parent = nullptr);

    // The command the person gave last: its name as the tip shows it ("Bold"), and what does it again.
    void record(const QString &name, std::function<void()> again);
    void clear();
    bool available() const { return bool(m_again); }
    QString name() const { return m_name; }
    // Does it again; false when there is nothing to do. (The function may record the command again, and that is the same command.)
    bool run();

    // What the command's tip says: "Repeat Bold (F4)", or "Repeat (F4)" when there is nothing to repeat.
    QString tip() const;
    // The keys: F4 for the app's Repeat command, and Ctrl+Y for a second command it gives the window (with no button), which may run only while Redo has no step (Ctrl+Y is
    // Redo while it has one). This says whether that second command is enabled.
    void setRedoAvailable(bool has);
    bool ctrlYEnabled() const { return available() && !m_redoAvailable; }

Q_SIGNALS:
    void changed();

private:
    QString m_name;
    std::function<void()> m_again;
    bool m_running = false;
    bool m_redoAvailable = false;
};

} // namespace jp
