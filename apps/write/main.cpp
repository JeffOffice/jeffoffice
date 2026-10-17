// JeffWrite entry point.
//
//   JeffWrite [file ...]                 open documents (.docx, .docm, .dotx, .dotm, .doc, .dot), or start a new one
//   JeffWrite --screenshot <png> [--size WxH] [--zoom percent|width|page|pages] [--page N]
//                                         render the window to a PNG and quit
//                                         (works with QT_QPA_PLATFORM=offscreen)
//   JeffWrite --backstage <page>         open on a File page (new, open, print, ...)

#include "app/crash.h"
#include "app/focusring.h"
#include "app/i18n.h"
#include "app/icons.h"
#include "app/pageview.h"
#include "app/recovery.h"
#include "app/settings.h"
#include "app/telemetry.h"
#include "app/theme.h"
#include "app/updater.h"
#include "app/writeinfo.h"
#include "app/writewindow.h"
#include "core/fonts.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QFileOpenEvent>
#include <QStyleFactory>
#include <QTimer>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace {
// A document opened from the Finder (Open With, or dragged onto the app) comes as an event, not on the command line: into a window
// still empty, else a new one.
class FileOpener : public QObject {
public:
    using QObject::QObject;
    bool eventFilter(QObject *o, QEvent *e) override
    {
        if (e->type() != QEvent::FileOpen) return QObject::eventFilter(o, e);
        const QString path = static_cast<QFileOpenEvent *>(e)->file();
        if (path.isEmpty()) return false;
        jp::WriteWindow *target = nullptr;
        for (QWidget *top : QApplication::topLevelWidgets())
            if (auto *w = qobject_cast<jp::WriteWindow *>(top))
                if (w->isVisible() && w->filePath().isEmpty() && !w->isModified()) target = w;
        if (!target) {
            target = new jp::WriteWindow();
            target->setAttribute(Qt::WA_DeleteOnClose);
            target->show();
        }
        target->openFile(path);
        target->raise();
        target->activateWindow();
        return true;
    }
};
} // namespace

int main(int argc, char **argv)
{
    Q_INIT_RESOURCE(resources);   // icons and font measurements, in the shared libraries
    Q_INIT_RESOURCE(write);       // JeffWrite's ribbon
    jp::setAppInfo(jp::writeAppInfo());
    QApplication::setApplicationName(QStringLiteral("JeffWrite"));
    QApplication::setOrganizationName(QStringLiteral("JeffOffice"));
    QApplication::setApplicationVersion(QStringLiteral(JP_VERSION));
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    QApplication app(argc, argv);
    jp::crash::install(jp::crash::reportPath());   // sent with the statistics on the next start, if they're on
    jp::recovery::setCopySuffix(QStringLiteral(".docx"));   // AutoRecover keeps documents
    jp::installTranslations();   // before the command line and any window build their text

    QCommandLineParser cli;
    cli.setApplicationDescription(QCoreApplication::translate("Main", "JeffWrite documents"));
    cli.addHelpOption();
    cli.addVersionOption();
    QCommandLineOption shotOpt(QStringLiteral("screenshot"), QCoreApplication::translate("Main", "Save a screenshot of the window and quit."), QStringLiteral("png"));
    QCommandLineOption sizeOpt(QStringLiteral("size"), QCoreApplication::translate("Main", "Window size for --screenshot."), QStringLiteral("WxH"), QStringLiteral("1440x900"));
    QCommandLineOption zoomOpt(QStringLiteral("zoom"), QCoreApplication::translate("Main", "Start at this zoom: a percent, or width, page, or pages."), QStringLiteral("zoom"));
    QCommandLineOption pageOpt(QStringLiteral("page"), QCoreApplication::translate("Main", "Start at this page."), QStringLiteral("number"));
    QCommandLineOption stageOpt(QStringLiteral("backstage"), QCoreApplication::translate("Main", "Open on a File page."), QStringLiteral("page"));
    cli.addOptions({shotOpt, sizeOpt, zoomOpt, pageOpt, stageOpt});
    cli.addPositionalArgument(QStringLiteral("files"), QCoreApplication::translate("Main", "Documents to open."), QStringLiteral("[files...]"));
    cli.process(app);

    jp::initCore();
    app.setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    jp::applyUiTheme(jp::Settings::get().value(QStringLiteral("ui/theme"), 0).toInt());
    jp::followSystemTheme();
    jp::installUiPolish();
    jp::installFocusRing(&app);
    app.installEventFilter(new FileOpener(&app));

    auto *w = new jp::WriteWindow();
    w->setAttribute(Qt::WA_DeleteOnClose);
    const QStringList files = cli.positionalArguments();
    if (!files.isEmpty()) {
        w->openFile(files.first());
        for (int i = 1; i < files.size(); ++i) {
            auto *other = new jp::WriteWindow();
            other->setAttribute(Qt::WA_DeleteOnClose);
            other->openFile(files[i]);
            other->show();
        }
    }
    auto applyView = [w, &cli, &zoomOpt, &pageOpt] {
        using Mode = jp::write::PageView::ZoomMode;
        if (cli.isSet(zoomOpt)) {
            const QString z = cli.value(zoomOpt);
            if (z == QLatin1String("width")) w->view()->setZoomMode(Mode::PageWidth);
            else if (z == QLatin1String("page")) w->view()->setZoomMode(Mode::OnePage);
            else if (z == QLatin1String("pages")) w->view()->setZoomMode(Mode::ManyPages, 3, 1);
            else if (z.toDouble() > 0) w->view()->setZoom(z.toDouble() / 100.0);
        }
        if (cli.isSet(pageOpt)) w->view()->goToPage(cli.value(pageOpt).toInt() - 1);
    };
    if (cli.isSet(shotOpt)) {
        const QStringList wh = cli.value(sizeOpt).split('x');
        w->resize(wh.value(0).toInt() > 0 ? wh.value(0).toInt() : 1440, wh.value(1).toInt() > 0 ? wh.value(1).toInt() : 900);
        w->show();
        if (cli.isSet(stageOpt)) w->showFileScreen(cli.value(stageOpt));
        const QString out = cli.value(shotOpt);
        QTimer::singleShot(300, w, [w, out, applyView] {
            w->view()->paginateAll();
            applyView();
            w->view()->setCaretWithoutFocus(true);
            QTimer::singleShot(300, w, [w, out] {
                w->grab().save(out);
                QApplication::exit(0);
            });
        });
        return app.exec();
    }
#ifdef Q_OS_WIN
    // Lets the installer see that JeffWrite is running and ask to close it first.
    CreateMutexW(nullptr, FALSE, L"JeffWriteRunning");
#endif
    w->show();
    applyView();
    if (cli.isSet(stageOpt)) w->showFileScreen(cli.value(stageOpt));
    // A normal exit takes this run's AutoRecover copies with it; copies a crashed run left are offered once the window is up.
    QObject::connect(&app, &QCoreApplication::aboutToQuit, [] { jp::recovery::endSession(); });
    QTimer::singleShot(0, w, [w] { w->offerRecovery(); });
    auto *up = new jp::Updater(w);
    jp::setUpdater(up);
    up->checkOnStartup();
    jp::telemetry::start(w);
    return app.exec();
}
