// JeffDeck entry point.
//
//   JeffDeck [file ...]                 open presentations (.pptx, .potx, .ppsx), or start a new one
//   JeffDeck --screenshot <png> [--size WxH]
//                                         render the window to a PNG and quit
//                                         (works with QT_QPA_PLATFORM=offscreen)
//   JeffDeck --view normal|outline|sorter  start in that view

#include "app/crash.h"
#include "app/deckinfo.h"
#include "app/deckwindow.h"
#include "app/focusring.h"
#include "app/icons.h"
#include "app/recovery.h"
#include "app/settings.h"
#include "app/theme.h"
#include "core/fonts.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QStyleFactory>
#include <QTimer>

int main(int argc, char **argv)
{
    Q_INIT_RESOURCE(resources);   // icons and font measurements, in the shared libraries
    Q_INIT_RESOURCE(deck);        // JeffDeck's ribbon
    jp::setAppInfo(jp::deckAppInfo());
    QApplication::setApplicationName(QStringLiteral("JeffDeck"));
    QApplication::setOrganizationName(QStringLiteral("JeffOffice"));
    QApplication::setApplicationVersion(QStringLiteral(JP_VERSION));
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    QApplication app(argc, argv);
    jp::crash::install(jp::crash::reportPath());
    jp::recovery::setCopySuffix(QStringLiteral(".pptx"));   // AutoRecover keeps presentations

    QCommandLineParser cli;
    cli.setApplicationDescription(QCoreApplication::translate("Main", "JeffDeck presentations"));
    cli.addHelpOption();
    cli.addVersionOption();
    QCommandLineOption shotOpt(QStringLiteral("screenshot"), QCoreApplication::translate("Main", "Save a screenshot of the window and quit."), QStringLiteral("png"));
    QCommandLineOption sizeOpt(QStringLiteral("size"), QCoreApplication::translate("Main", "Window size for --screenshot."), QStringLiteral("WxH"), QStringLiteral("1440x900"));
    QCommandLineOption viewOpt(QStringLiteral("view"), QCoreApplication::translate("Main", "Start in this view: normal, outline, or sorter."), QStringLiteral("view"));
    cli.addOptions({shotOpt, sizeOpt, viewOpt});
    cli.addPositionalArgument(QStringLiteral("files"), QCoreApplication::translate("Main", "Presentations to open."), QStringLiteral("[files...]"));
    cli.process(app);

    jp::initCore();
    app.setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    jp::applyUiTheme(jp::Settings::get().value(QStringLiteral("ui/theme"), 0).toInt());
    jp::followSystemTheme();
    jp::installUiPolish();
    jp::installFocusRing(&app);

    auto *w = new jp::DeckWindow();
    w->setAttribute(Qt::WA_DeleteOnClose);
    const QStringList files = cli.positionalArguments();
    if (!files.isEmpty()) {
        w->openFile(files.first());
        for (int i = 1; i < files.size(); ++i) {
            auto *other = new jp::DeckWindow();
            other->setAttribute(Qt::WA_DeleteOnClose);
            other->openFile(files[i]);
            other->show();
        }
    }
    if (cli.isSet(viewOpt)) {
        const QString view = cli.value(viewOpt);
        QTimer::singleShot(0, w, [w, view] { w->setView(view == QLatin1String("sorter") ? jp::DeckWindow::View::Sorter : view == QLatin1String("outline") ? jp::DeckWindow::View::Outline : jp::DeckWindow::View::Normal); });
    }
    if (cli.isSet(shotOpt)) {
        const QStringList wh = cli.value(sizeOpt).split('x');
        w->resize(wh.value(0).toInt() > 0 ? wh.value(0).toInt() : 1440, wh.value(1).toInt() > 0 ? wh.value(1).toInt() : 900);
        w->show();
        const QString out = cli.value(shotOpt);
        QTimer::singleShot(600, w, [w, out] {
            w->grab().save(out);
            QApplication::exit(0);
        });
        return app.exec();
    }
    w->show();
    // A normal exit takes this run's AutoRecover copies with it; copies a
    // crashed run left are offered once the window is up.
    QObject::connect(&app, &QCoreApplication::aboutToQuit, [] { jp::recovery::endSession(); });
    QTimer::singleShot(0, w, [w] { w->offerRecovery(); });
    return app.exec();
}
