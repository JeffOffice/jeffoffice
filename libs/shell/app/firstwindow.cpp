#include "app/firstwindow.h"
#include "core/startuptrace.h"

#include "core/fonts.h"

#include <QApplication>
#include <QCoreApplication>
#include <QFontInfo>
#include <QMainWindow>
#include <QStatusBar>
#include <QFileInfo>
#include <QElapsedTimer>
#include <QFileOpenEvent>
#include <QTimer>
#include <QWindow>

#include <cstdio>

namespace jp {

namespace {
// The first paint of the window or of anything in it (a start screen covering the whole window is what paints, then, and not the window itself).
class FirstPaintTracer : public QObject {
public:
    using QObject::QObject;
    bool eventFilter(QObject *o, QEvent *e) override
    {
        auto *window = static_cast<QWidget *>(parent());
        if (e->type() == QEvent::Paint && (o == window || window->isAncestorOf(qobject_cast<QWidget *>(o)))) {
            startupMark("first paint");
            QCoreApplication::instance()->removeEventFilter(this);
            deleteLater();
        }
        return false;
    }
};
} // namespace

void traceFirstPaint(QWidget *window)
{
    if (qEnvironmentVariableIsSet("JP_STARTUP_TRACE")) QCoreApplication::instance()->installEventFilter(new FirstPaintTracer(window));
}

QString openingStatus(const QString &path)
{
    return QCoreApplication::translate("Splash", "Opening %1…").arg(QFileInfo(path).fileName());
}

FirstWindow::FirstWindow(QWidget *window) : m_window(window)
{
    QCoreApplication::instance()->installEventFilter(this);   // (before the app's own handler, which is asked last)
    window->show();
    QElapsedTimer waited;
    waited.start();
    // (a splash screen's finish() would wait with the keys and clicks let in)
    while (m_window && waited.elapsed() < 3000 && !(m_window->windowHandle() && m_window->windowHandle()->isExposed()))
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents, 50);
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents, 50);
}

FirstWindow::~FirstWindow()
{
    QCoreApplication::instance()->removeEventFilter(this);
    for (const QString &path : m_held) QCoreApplication::postEvent(QCoreApplication::instance(), new QFileOpenEvent(path));
}

bool FirstWindow::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::FileOpen && o == QCoreApplication::instance()) {
        const QString path = static_cast<QFileOpenEvent *>(e)->file();
        if (!path.isEmpty()) m_held << path;
        return true;
    }
    return QObject::eventFilter(o, e);
}

void traceDrawn(QWidget *window, const char *what)
{
    if (!qEnvironmentVariableIsSet("JP_STARTUP_TRACE")) return;
    // (a paint is posted after the call, and the zero timer comes after it)
    QTimer::singleShot(0, window, [what] { startupMark(what); });
}

StagedStart::StagedStart(bool plain)
    : m_staged(plain && !qEnvironmentVariableIsSet("JP_SERIAL_START") && !familyMayBeBundled(QFontInfo(QApplication::font()).family()))
{
    if (!m_staged) initCore();
}

bool StagedStart::showFirst(QWidget *window, const QString &fileToOpen, const std::function<void()> &whenDrawn)
{
    if (!m_staged) return true;
    // The window is up before the fonts are added and the file is read, so it shows at once and says what it is doing.
    if (!fileToOpen.isEmpty())
        if (auto *main = qobject_cast<QMainWindow *>(window)) main->statusBar()->showMessage(openingStatus(fileToOpen));
    m_first = std::make_unique<FirstWindow>(window);
    if (!m_first->open()) return false;   // (closed meanwhile: nothing to open into)
    startupMark("window shown first");
    if (whenDrawn) {
        whenDrawn();
        // (what it made is drawn: its layout done, and the window painted at once, with no keys or clicks taken in)
        for (int round = 0; round < 8; ++round) {   // (until a round has nothing left to do, or for a while)
            QElapsedTimer handled;
            handled.start();
            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents, 50);
            if (handled.elapsed() < 5) break;
        }
        window->repaint();
        QGuiApplication::sync();   // (to the screen now: the fonts take a while, and the events are not handled then)
        startupMark("after the first picture");
    }
    initCore();
    startupMark("initCore done");
    if (fileToOpen.isEmpty()) {
        traceDrawn(window, "window usable");
        m_first.reset();
    }
    return true;
}

void grabAfterStart(QWidget *window)
{
    if (!qEnvironmentVariableIsSet("JP_GRAB") || !qEnvironmentVariableIsSet("JP_STARTUP_TRACE")) return;
    const int ms = qEnvironmentVariableIntValue("JP_GRAB_MS");
    QTimer::singleShot(ms > 0 ? ms : 1500, window, [window] {
        const QString path = qEnvironmentVariable("JP_GRAB");
        const bool ok = window->grab().save(path);
        if (!ok) std::fprintf(stderr, "JP_GRAB: can't save %s\n", qPrintable(path));
        QCoreApplication::exit(ok ? 0 : 1);
    });
}

} // namespace jp
