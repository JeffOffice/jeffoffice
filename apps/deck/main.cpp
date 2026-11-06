// JeffDeck entry point.
//
//   JeffDeck [file ...]                 open presentations (.pptx, .potx, .ppsx, .ppt, .pps, .pot, .odp, .otp), or start a new one
//   JeffDeck --screenshot <png> [--size WxH]
//                                         render the window to a PNG and quit
//                                         (works with QT_QPA_PLATFORM=offscreen)
//   JeffDeck --view normal|outline|sorter|master  start in that view
//   JeffDeck --backstage <page>         open on a File page (new, open, print, ...)

#include "app/crash.h"
#include "app/deckinfo.h"
#include "app/deckwindow.h"
#include "app/editor.h"
#include "app/focusring.h"
#include "app/i18n.h"
#include "app/icons.h"
#include "app/recovery.h"
#include "app/settings.h"
#include "app/firstwindow.h"
#include "app/telemetry.h"
#include "app/theme.h"
#include "app/updater.h"
#include "core/fonts.h"
#include "core/startuptrace.h"

#include <QApplication>
#include <QPointer>
#include <QStatusBar>
#include <QTextStream>
#include <memory>
#include <QCommandLineParser>
#include <QFileOpenEvent>
#include <QStyleFactory>
#include <QTimer>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace {
// A presentation opened from the Finder (Open With, or dragged onto the app)
// comes as an event, not on the command line: into a window still empty,
// else a new one.
class FileOpener : public QObject {
public:
    using QObject::QObject;
    bool eventFilter(QObject *o, QEvent *e) override
    {
        if (e->type() != QEvent::FileOpen) return QObject::eventFilter(o, e);
        const QString path = static_cast<QFileOpenEvent *>(e)->file();
        if (path.isEmpty()) return false;
        jp::DeckWindow *target = nullptr;
        for (QWidget *top : QApplication::topLevelWidgets())
            if (auto *w = qobject_cast<jp::DeckWindow *>(top))
                if (w->isVisible() && w->editor()->filePath().isEmpty() && !w->editor()->isModified()) target = w;
        if (!target) {
            target = new jp::DeckWindow();
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
    jp::startupMark("main entered");
    Q_INIT_RESOURCE(resources);   // icons and font measurements, in the shared libraries
    Q_INIT_RESOURCE(deck);        // JeffDeck's ribbon
    jp::setAppInfo(jp::deckAppInfo());
    QApplication::setApplicationName(QStringLiteral("JeffDeck"));
    QApplication::setOrganizationName(QStringLiteral("JeffOffice"));
    QApplication::setApplicationVersion(QStringLiteral(JP_VERSION));
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    QApplication app(argc, argv);
    jp::startupMark("QApplication built");
    jp::crash::install(jp::crash::reportPath());   // sent with the statistics on the next start, if they're on
    jp::recovery::setCopySuffix(QStringLiteral(".pptx"));   // automatic recovery keeps presentations
    jp::installTranslations();   // before the command line and any window build their text

    QCommandLineParser cli;
    cli.setApplicationDescription(QCoreApplication::translate("Main", "JeffDeck presentations"));
    cli.addHelpOption();
    cli.addVersionOption();
    QCommandLineOption shotOpt(QStringLiteral("screenshot"), QCoreApplication::translate("Main", "Save a screenshot of the window and quit."), QStringLiteral("png"));
    QCommandLineOption sizeOpt(QStringLiteral("size"), QCoreApplication::translate("Main", "Window size for --screenshot."), QStringLiteral("WxH"), QStringLiteral("1440x900"));
    QCommandLineOption viewOpt(QStringLiteral("view"), QCoreApplication::translate("Main", "Start in this view: normal, outline, sorter, or master."), QStringLiteral("view"));
    QCommandLineOption stageOpt(QStringLiteral("backstage"), QCoreApplication::translate("Main", "Open on a File page."), QStringLiteral("page"));
    cli.addOptions({shotOpt, sizeOpt, viewOpt, stageOpt});
    cli.addPositionalArgument(QStringLiteral("files"), QCoreApplication::translate("Main", "Presentations to open."), QStringLiteral("[files...]"));
    cli.process(app);
    const bool plain = !cli.isSet(shotOpt);   // (not a job that makes a file and quits)
    jp::StagedStart start(plain);
    app.setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    jp::applyUiTheme(jp::Settings::get().value(QStringLiteral("ui/theme"), 0).toInt());
    jp::followSystemTheme();
    jp::installUiPolish();
    jp::installFocusRing(&app);
    app.installEventFilter(new FileOpener(&app));
    auto *w = new jp::DeckWindow();
    jp::startupMark("window built");
    jp::traceFirstPaint(w);
    w->setAttribute(Qt::WA_DeleteOnClose);
    const QStringList files = cli.positionalArguments();
#ifdef Q_OS_WIN
    // Lets the installer see that JeffDeck is running and ask to close it first (before the window shows).
    if (plain) CreateMutexW(nullptr, FALSE, L"JeffDeckRunning");
#endif
    if (!start.showFirst(w, files.value(0))) return 0;
    if (!files.isEmpty()) {
        QPointer<jp::DeckWindow> guard(w);
        // (A screenshot has nobody to click a box: a file that can't be opened is said on stderr and the run ends.)
        if (cli.isSet(shotOpt))
            w->setNotifier([](const QString &, const QString &text) { QTextStream(stderr) << text << "\n"; });
        if (!w->openFile(files.first()) && cli.isSet(shotOpt)) return 1;
        jp::startupMark("file opened");
        jp::traceDrawn(w, "file drawn");
        if (guard && w->statusBar()->currentMessage() == jp::openingStatus(files.first())) w->statusBar()->clearMessage();
        for (int i = 1; i < files.size(); ++i) {
            auto *other = new jp::DeckWindow();
            other->setAttribute(Qt::WA_DeleteOnClose);
            other->openFile(files[i]);
            other->show();
        }
    }
    start.finish();   // (the first window has opened its file: file-open events go to the handler again)
    if (cli.isSet(viewOpt)) {
        const QString view = cli.value(viewOpt);
        QTimer::singleShot(0, w, [w, view] { w->setView(view == QLatin1String("sorter") ? jp::DeckWindow::View::Sorter : view == QLatin1String("outline") ? jp::DeckWindow::View::Outline : view == QLatin1String("master") ? jp::DeckWindow::View::Master : jp::DeckWindow::View::Normal); });
    }
    if (cli.isSet(shotOpt)) {
        const QStringList wh = cli.value(sizeOpt).split('x');
        w->resize(wh.value(0).toInt() > 0 ? wh.value(0).toInt() : 1440, wh.value(1).toInt() > 0 ? wh.value(1).toInt() : 900);
        w->show();
        if (cli.isSet(stageOpt)) w->showFileScreen(cli.value(stageOpt));
        const QString out = cli.value(shotOpt);
        QTimer::singleShot(600, w, [w, out] {
            w->grab().save(out);
            QApplication::exit(0);
        });
        return app.exec();
    }
    w->show();
    jp::startupMark("window shown");
    if (cli.isSet(stageOpt)) w->showFileScreen(cli.value(stageOpt));
    // A normal exit takes this run's automatic recovery copies with it; copies a
    // crashed run left are offered once the window is up.
    QObject::connect(&app, &QCoreApplication::aboutToQuit, [] { jp::recovery::endSession(); });
    QTimer::singleShot(0, w, [w] { w->offerRecovery(); });
    auto *up = new jp::Updater(w);
    jp::setUpdater(up);
    up->checkOnStartup();
    jp::telemetry::start(w);
    jp::grabAfterStart(w);
    return app.exec();
}
