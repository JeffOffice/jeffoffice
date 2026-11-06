#pragma once

#include <QObject>
#include <QPointer>
#include <QStringList>

#include <memory>

namespace jp {

// With JP_STARTUP_TRACE set, notes (as startupMark does) when the window is first painted. Nothing happens without it.
void traceFirstPaint(QWidget *window);
// With JP_STARTUP_TRACE set, notes when the event loop first has nothing to paint or answer after this call (the window and what was opened in it are drawn).
void traceDrawn(QWidget *window, const char *what);

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

// How the four programs start, in one place. A plain start (`plain`: not a job that makes a file and quits) shows the window before the bundled fonts are added and the
// file is read; the other starts, and a program whose own font is one of the bundled families, add the fonts first (JP_SERIAL_START=1 does too, for comparing).
class StagedStart {
public:
    explicit StagedStart(bool plain);
    bool staged() const { return m_staged; }
    // With the window built: shows it, waits until it is drawn, and adds the fonts (a no-op in the other order). False if the window was closed meanwhile.
    bool showFirst(QWidget *window, const QString &fileToOpen = QString());
    // The file is open (or there is none): file-open events go to the program's handler again.
    void finish() { m_first.reset(); }

private:
    bool m_staged = false;
    std::unique_ptr<FirstWindow> m_first;
};

// A hook for comparing starts, only with JP_STARTUP_TRACE set: JP_GRAB=<png> saves the window JP_GRAB_MS (1500) milliseconds after this call and quits (status 1,
// and a line on stderr, if the picture can't be saved).
void grabAfterStart(QWidget *window);

} // namespace jp
