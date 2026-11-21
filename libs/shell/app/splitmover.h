#pragma once
// Move Split: the arrow keys move the divider between two panes of a window (a way to size panes without a mouse). The first divider is
// chosen at the start, Tab goes on to the next one, and Enter, Esc, or a click ends it. Built once for every app's window (View > Window).

#include "core/export.h"

#include <QList>
#include <QObject>
#include <QPointer>
#include <QWidget>

class QSplitter;

namespace jp {

class JO_COMMON_EXPORT SplitMover : public QObject {
    Q_OBJECT
public:
    explicit SplitMover(QObject *parent = nullptr);
    ~SplitMover() override;
    // Starts with the dividers of `splitters`, each one between two panes, in order. False when there is none.
    bool start(const QList<QSplitter *> &splitters);
    void stop();
    bool active() const { return m_active; }
    // How far an arrow moves a divider (a Shift arrow moves four times as far), in pixels.
    static constexpr int kStep = 10;

    struct Handle {
        QPointer<QSplitter> splitter;
        int index;    // the pane after the divider; its handle is the divider
        int before;   // the shown pane before it (hidden panes between them have no size)
    };

Q_SIGNALS:
    // What to tell the person (the keys to use), and that it has ended.
    void prompt(const QString &text);
    void finished();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void select(int which);
    void move(int pixels);
    QList<Handle> m_handles;
    int m_current = 0;
    bool m_active = false;
    QPointer<QWidget> m_window;   // the window it was started in: the only one whose keys it reads
};

} // namespace jp
