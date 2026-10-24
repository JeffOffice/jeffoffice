// JeffCrunch entry point.
//
//   JeffCrunch [file ...]                open workbooks (.xlsx, .xlsm, .xls, .ods, .fods, .csv, .tsv, .txt), or start with an empty window
//   JeffCrunch --screenshot <png> [--size WxH] [--zoom percent] [--sheet N] <file>
//                                        open the workbook, wait until it is read and calculated, render the window to a PNG, and quit
//                                        (works with QT_QPA_PLATFORM=offscreen)
//   JeffCrunch --backstage <page>        open on a File page (open, about)

#include "app/crash.h"
#include "app/crunchinfo.h"
#include "app/crunchwindow.h"
#include "app/focusring.h"
#include "app/i18n.h"
#include "app/icons.h"
#include "app/recovery.h"
#include "app/settings.h"
#include "app/telemetry.h"
#include "app/theme.h"
#include "app/updater.h"
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
// A workbook opened from the Finder (Open With, or dragged onto the app) comes as an event, not on the command line: into a window
// still empty, else a new one.
class FileOpener : public QObject {
public:
    using QObject::QObject;
    bool eventFilter(QObject *o, QEvent *e) override
    {
        if (e->type() != QEvent::FileOpen) return QObject::eventFilter(o, e);
        const QString path = static_cast<QFileOpenEvent *>(e)->file();
        if (path.isEmpty()) return false;
        jp::CrunchWindow *target = nullptr;
        for (QWidget *top : QApplication::topLevelWidgets())
            if (auto *w = qobject_cast<jp::CrunchWindow *>(top))
                if (w->isVisible() && !w->hasWorkbook() && !w->isLoading()) target = w;
        if (!target) {
            target = new jp::CrunchWindow();
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
    Q_INIT_RESOURCE(crunch);      // JeffCrunch's ribbon
    jp::setAppInfo(jp::crunchAppInfo());
    QApplication::setApplicationName(QStringLiteral("JeffCrunch"));
    QApplication::setOrganizationName(QStringLiteral("JeffOffice"));
    QApplication::setApplicationVersion(QStringLiteral(JP_VERSION));
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    QApplication app(argc, argv);
    jp::crash::install(jp::crash::reportPath());   // sent with the statistics on the next start, if they're on
    jp::installTranslations();   // before the command line and any window build their text

    QCommandLineParser cli;
    cli.setApplicationDescription(QCoreApplication::translate("Main", "JeffCrunch workbooks"));
    cli.addHelpOption();
    cli.addVersionOption();
    QCommandLineOption shotOpt(QStringLiteral("screenshot"), QCoreApplication::translate("Main", "Save a screenshot of the window and quit."), QStringLiteral("png"));
    QCommandLineOption sizeOpt(QStringLiteral("size"), QCoreApplication::translate("Main", "Window size for --screenshot."), QStringLiteral("WxH"), QStringLiteral("1440x900"));
    QCommandLineOption zoomOpt(QStringLiteral("zoom"), QCoreApplication::translate("Main", "Start at this zoom, in percent."), QStringLiteral("percent"));
    QCommandLineOption sheetOpt(QStringLiteral("sheet"), QCoreApplication::translate("Main", "Start on this sheet (counting from 1)."), QStringLiteral("number"));
    QCommandLineOption stageOpt(QStringLiteral("backstage"), QCoreApplication::translate("Main", "Open on a File page."), QStringLiteral("page"));
    cli.addOptions({shotOpt, sizeOpt, zoomOpt, sheetOpt, stageOpt});
    cli.addPositionalArgument(QStringLiteral("files"), QCoreApplication::translate("Main", "Workbooks to open."), QStringLiteral("[files...]"));
    cli.process(app);

    jp::initCore();
    app.setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    jp::applyUiTheme(jp::Settings::get().value(QStringLiteral("ui/theme"), 0).toInt());
    jp::followSystemTheme();
    jp::installUiPolish();
    jp::installFocusRing(&app);
    app.installEventFilter(new FileOpener(&app));

    auto *w = new jp::CrunchWindow();
    w->setAttribute(Qt::WA_DeleteOnClose);
    const QStringList files = cli.positionalArguments();
    if (cli.isSet(shotOpt)) {
        const QStringList wh = cli.value(sizeOpt).split('x');
        w->resize(wh.value(0).toInt() > 0 ? wh.value(0).toInt() : 1440, wh.value(1).toInt() > 0 ? wh.value(1).toInt() : 900);
        w->show();
        if (!files.isEmpty()) w->openFile(files.first());
        if (cli.isSet(stageOpt)) w->showFileScreen(cli.value(stageOpt));
        const QString out = cli.value(shotOpt);
        auto shoot = [w, out, &cli, &zoomOpt, &sheetOpt] {
            if (cli.isSet(sheetOpt)) w->setActiveSheet(cli.value(sheetOpt).toInt() - 1);
            if (cli.isSet(zoomOpt) && cli.value(zoomOpt).toDouble() > 0) w->view()->setZoom(cli.value(zoomOpt).toDouble() / 100.0);
            QTimer::singleShot(300, w, [w, out] {
                w->grab().save(out);
                QApplication::exit(0);
            });
        };
        if (files.isEmpty()) {
            QTimer::singleShot(300, w, shoot);
        } else {
            // (once the workbook is read and every formula has its value; a workbook that can't be read is shot as it is)
            QObject::connect(w, &jp::CrunchWindow::calculationFinished, w, shoot);
            QObject::connect(w, &jp::CrunchWindow::loadFinished, w, [shoot](bool ok) {
                if (!ok) shoot();
            });
        }
        return app.exec();
    }
#ifdef Q_OS_WIN
    // Lets the installer see that JeffCrunch is running and ask to close it first.
    CreateMutexW(nullptr, FALSE, L"JeffCrunchRunning");
#endif
    w->show();
    if (!files.isEmpty()) {
        w->openFile(files.first());
        for (int i = 1; i < files.size(); ++i) w->openFile(files[i]);   // (each in a window of its own)
    }
    if (cli.isSet(zoomOpt) && cli.value(zoomOpt).toDouble() > 0) w->view()->setZoom(cli.value(zoomOpt).toDouble() / 100.0);
    if (cli.isSet(stageOpt)) w->showFileScreen(cli.value(stageOpt));
    auto *up = new jp::Updater(w);
    jp::setUpdater(up);
    up->checkOnStartup();
    jp::telemetry::start(w);
    return app.exec();
}
