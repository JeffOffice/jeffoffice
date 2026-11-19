#include "app/linkprogress.h"
#include "core/workthread.h"

#include <QEventLoop>
#include <QObject>
#include <QProgressDialog>
#include <QTimer>

#include <atomic>

namespace jp {

bool runLinkReads(QWidget *parent, jp::office::sheet::ExternalBooks *links, const QString &text, const std::function<void()> &work)
{
    if (!links) {
        work();
        return true;
    }
    links->resetCancel();
    std::atomic<bool> done{false};
    jp::WorkThread worker([&] {
        work();
        done.store(true);
    });
    QProgressDialog dialog(text, QObject::tr("Cancel"), 0, 0, parent);
    dialog.setObjectName(QStringLiteral("linkProgress"));
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setMinimumDuration(400);
    bool cancelled = false;
    QObject::connect(&dialog, &QProgressDialog::canceled, &dialog, [&] {
        cancelled = true;
        links->cancelReads();
    });
    QEventLoop loop;
    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, &loop, [&] {
        if (done.load()) loop.quit();
    });
    timer.start(10);
    loop.exec();
    worker.join();
    return !cancelled;
}

} // namespace jp
