// JeffPub entry point.
//
//   JeffPub [file ...]                  open publications (.jpub, .pub, ...)
//   JeffPub --template <id>             start from a built-in template
//   JeffPub --backstage <page>          open on a File page (new, open, print, ...)
//   JeffPub --screenshot <png> [--size WxH]
//                                         render the window to a PNG and quit
//                                         (works with QT_QPA_PLATFORM=offscreen)
//   JeffPub --export-help-wiki <folder> write Help's topics as GitHub wiki pages
//                                         into a new folder and quit (also
//                                         with QT_QPA_PLATFORM=offscreen)

#include "app/telemetry.h"
#include "app/editor.h"
#include "app/mainwindow.h"
#include "app/pubinfo.h"
#include "app/recovery.h"
#include "app/theme.h"
#include "app/i18n.h"
#include "app/focusring.h"
#include "app/help.h"
#include "app/ribbon.h"
#include "app/updater.h"
#include "app/settings.h"
#include "app/icons.h"
#include "core/fonts.h"
#include "templates/templates.h"

#include <algorithm>

#include <QApplication>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QPalette>
#include <QStyleFactory>
#include <QStyleHints>
#include <QTextStream>
#include <QTimer>

#ifdef Q_OS_WIN
#include <windows.h>
#endif



int main(int argc, char **argv)
{
    Q_INIT_RESOURCE(resources);   // the icons live in the static jpcore library
    jp::setAppInfo(jp::pubAppInfo());   // who the shared libraries say is running
    QApplication::setApplicationName(QStringLiteral("JeffPub"));
    QApplication::setOrganizationName(QStringLiteral("JeffOffice"));
    QApplication::setApplicationVersion(QStringLiteral(JP_VERSION));
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    QApplication app(argc, argv);
    QApplication::setWindowIcon(QIcon(QStringLiteral(":/app.png")));
    jp::moveFromJeffPub79();   // settings and files kept under the old name
    jp::recovery::setCopySuffix(QStringLiteral(".jpub"));   // AutoRecover keeps publications
    // Before the command line and any window build their text. Not for the
    // wiki: its pages are Help in English, whatever the system's language.
    const QStringList args = QCoreApplication::arguments();
    if (std::none_of(args.cbegin(), args.cend(), [](const QString &a) { return a.startsWith(QLatin1String("--export-help-wiki")); }))
        jp::installTranslations();

    QCommandLineParser cli;
    cli.setApplicationDescription(QCoreApplication::translate("Main", "JeffPub desktop publishing"));
    cli.addHelpOption();
    cli.addVersionOption();
    QCommandLineOption shotOpt(QStringLiteral("screenshot"), QCoreApplication::translate("Main", "Save a screenshot of the window and quit."), QStringLiteral("png"));
    QCommandLineOption sizeOpt(QStringLiteral("size"), QCoreApplication::translate("Main", "Window size for --screenshot."), QStringLiteral("WxH"), QStringLiteral("1440x900"));
    QCommandLineOption templOpt(QStringLiteral("template"), QCoreApplication::translate("Main", "Start from a built-in template."), QStringLiteral("id"));
    QCommandLineOption pdfOpt(QStringLiteral("export-pdf"), QCoreApplication::translate("Main", "Export the opened publication to a PDF and quit."), QStringLiteral("pdf"));
    QCommandLineOption pdfaOpt(QStringLiteral("pdfa"), QCoreApplication::translate("Main", "With --export-pdf: write PDF/A-1b for archiving."));
    QCommandLineOption tabOpt(QStringLiteral("tab"), QCoreApplication::translate("Main", "Open on a ribbon tab (for screenshots)."), QStringLiteral("name"));
    QCommandLineOption stageOpt(QStringLiteral("backstage"), QCoreApplication::translate("Main", "Open on a File page."), QStringLiteral("page"));
    QCommandLineOption helpTopicOpt(QStringLiteral("help-topic"), QCoreApplication::translate("Main", "Open Help on a topic (for screenshots)."), QStringLiteral("id"));
    QCommandLineOption wikiOpt(QStringLiteral("export-help-wiki"), QCoreApplication::translate("Main", "Write Help's topics as GitHub wiki pages into a new folder and quit."), QStringLiteral("folder"));
    cli.addOptions({shotOpt, sizeOpt, templOpt, stageOpt, pdfOpt, pdfaOpt, tabOpt, helpTopicOpt, wikiOpt});
    cli.addPositionalArgument(QStringLiteral("files"), QCoreApplication::translate("Main", "Publications to open."), QStringLiteral("[files...]"));
    cli.process(app);

    jp::initCore();
    app.setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    jp::applyUiTheme(jp::Settings::get().value(QStringLiteral("ui/theme"), 0).toInt());
    jp::followSystemTheme();
    jp::installUiPolish();
    jp::installFocusRing(&app);

    auto *w = new jp::MainWindow();
    w->setAttribute(Qt::WA_DeleteOnClose);

    if (cli.isSet(wikiOpt)) {
        QString error;
        const bool ok = jp::help::exportWiki(cli.value(wikiOpt), w, &error);
        QTextStream(ok ? stdout : stderr) << (ok ? QCoreApplication::translate("Main", "Wrote Help's pages to %1.").arg(cli.value(wikiOpt)) : error) << Qt::endl;
        delete w;
        return ok ? 0 : 1;
    }

    jp::installFileOpenHandler(&app);
    const QStringList files = cli.positionalArguments();
    if (!files.isEmpty()) {
        w->openFile(files.first());
        for (int i = 1; i < files.size(); ++i) {
            auto *other = new jp::MainWindow();
            other->setAttribute(Qt::WA_DeleteOnClose);
            other->openFile(files[i]);
            other->show();
        }
    } else if (cli.isSet(templOpt)) {
        if (const jp::TemplateInfo *t = jp::findTemplate(cli.value(templOpt))) {
            auto doc = t->build(jp::TemplateOptions());
            doc->templateId = t->id;
            w->editor()->setDocument(std::move(doc));
        }
    }

    if (cli.isSet(pdfOpt)) {
        w->exportPdf(cli.value(pdfOpt), false, cli.isSet(pdfaOpt));
        delete w;
        return 0;
    }

    if (cli.isSet(shotOpt)) {
        const QStringList wh = cli.value(sizeOpt).split('x');
        w->resize(wh.value(0).toInt() > 0 ? wh.value(0).toInt() : 1440, wh.value(1).toInt() > 0 ? wh.value(1).toInt() : 900);
        w->show();
        if (cli.isSet(stageOpt)) w->showBackstage(cli.value(stageOpt));
        if (cli.isSet(tabOpt))
            if (auto *r = w->findChild<jp::Ribbon *>())
                if (jp::RibbonTab *t = r->tab(cli.value(tabOpt))) r->showTab(t);
        if (cli.isSet(helpTopicOpt)) w->showHelp(cli.value(helpTopicOpt));
        const QString out = cli.value(shotOpt);
        // Let layouts, fonts and the first paint settle before grabbing.
        QTimer::singleShot(600, w, [w, out] {
            w->screenshotTo(out);
            QApplication::exit(0);
        });
        return app.exec();
    }

#ifdef Q_OS_WIN
    // Lets the installer see that JeffPub is running and ask to close it first.
    CreateMutexW(nullptr, FALSE, L"JeffPubRunning");
#endif
    w->show();
    // A normal exit takes this run's AutoRecover copies with it; copies a
    // crashed run left are offered once the window is up.
    QObject::connect(&app, &QCoreApplication::aboutToQuit, [] { jp::recovery::endSession(); });
    QTimer::singleShot(0, w, [w] { w->offerRecovery(); });
    auto *up = new jp::Updater(w);
    jp::setUpdater(up);
    up->checkOnStartup();
    jp::telemetry::start(w);
    if (cli.isSet(stageOpt)) w->showBackstage(cli.value(stageOpt));
    else if (files.isEmpty() && !cli.isSet(templOpt) && jp::Settings::get().value(QStringLiteral("ui/startBackstage"), true).toBool())
        w->showBackstage(QStringLiteral("new"));
    return app.exec();
}
