#include "app/firstwindow.h"
#include "core/startuptrace.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QElapsedTimer>
#include <QFileOpenEvent>
#include <QTimer>
#include <QWindow>

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

} // namespace jp
