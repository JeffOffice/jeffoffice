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
class FirstPaintTracer : public QObject {
public:
    using QObject::QObject;
    bool eventFilter(QObject *o, QEvent *e) override
    {
        if (e->type() == QEvent::Paint && o == parent()) {
            startupMark("first paint");
            o->removeEventFilter(this);
            deleteLater();
        }
        return false;
    }
};
} // namespace

void traceFirstPaint(QWidget *window)
{
    if (qEnvironmentVariableIsSet("JP_STARTUP_TRACE")) window->installEventFilter(new FirstPaintTracer(window));
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

bool StagedStart::showFirst(QWidget *window, const QString &fileToOpen)
{
    if (!m_staged) return true;
    // The window is up before the fonts are added and the file is read, so it shows at once and says what it is doing.
    if (!fileToOpen.isEmpty())
        if (auto *main = qobject_cast<QMainWindow *>(window)) main->statusBar()->showMessage(openingStatus(fileToOpen));
    m_first = std::make_unique<FirstWindow>(window);
    if (!m_first->open()) return false;   // (closed meanwhile: nothing to open into)
    startupMark("window shown first");
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
