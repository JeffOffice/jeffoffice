#pragma once

#include <QObject>
#include <QPointer>
#include <QStringList>

namespace jp {

// The status line for opening a file: "Opening Budget.xlsx…".
QString openingStatus(const QString &path);

// The first window shown before the file given on the command line is read: shows it and waits until it is drawn (taking in no keys or clicks, which would change the
// empty document before the file is put in it). Until this object goes, a file the system asks to open (a Finder event) is kept, and handed to
// the app's own handler after, so the file is opened once, by the first window's own open. `open()` is false if the window was closed meanwhile.
class FirstWindow : public QObject {
public:
    explicit FirstWindow(QWidget *window);
    ~FirstWindow() override;
    bool open() const { return m_window && m_window->isVisible(); }

protected:
    bool eventFilter(QObject *o, QEvent *e) override;

private:
    QPointer<QWidget> m_window;
    QStringList m_held;
};

} // namespace jp
