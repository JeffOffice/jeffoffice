#include "app/firstwindow.h"
#include <QCoreApplication>
#include <QFileInfo>
#include <QElapsedTimer>
#include <QFileOpenEvent>
#include <QWindow>

namespace jp {

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

} // namespace jp
