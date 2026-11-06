#pragma once

#include <QObject>
#include <QPointer>
#include <QSplashScreen>
#include <QStringList>

namespace jp {

// The card an app shows while it starts: its color, its tile, its name, the suite's, the version, what it is doing,
// and how far along it is. It shows at once and goes when the first window is drawn.
class StartupSplash : public QSplashScreen {
public:
    StartupSplash();
    // What is being done, and how far along the start is (0 to 1); drawn at once, since the work that follows keeps
    // the event loop from running.
    void step(const QString &what, double done);
    QString status() const { return m_status; }
    double progress() const { return m_done; }

protected:
    void drawContents(QPainter *p) override;
    void mousePressEvent(QMouseEvent *) override {}   // (a click doesn't close it: the window is coming)

private:
    QString m_status;
    double m_done = 0;
    bool m_round = true;
};

// Shows the splash unless there is no screen to show it on (offscreen, as in tests and --screenshot); null then.
StartupSplash *showStartupSplash();
// The splash's line for opening a file: "Opening Budget.xlsx…".
QString openingStatus(const QString &path);

// The first window shown before the file given on the command line is read: shows it, waits until it is drawn (taking in no keys or clicks, which would change the
// empty document before the file is put in it), and closes the splash. Until this object goes, a file the system asks to open (a Finder event) is kept, and handed to
// the app's own handler after, so the file is opened once, by the first window's own open. `open()` is false if the window was closed meanwhile.
class FirstWindow : public QObject {
public:
    FirstWindow(QWidget *window, StartupSplash *splash);
    ~FirstWindow() override;
    bool open() const { return m_window && m_window->isVisible(); }

protected:
    bool eventFilter(QObject *o, QEvent *e) override;

private:
    QPointer<QWidget> m_window;
    QStringList m_held;
};

} // namespace jp
