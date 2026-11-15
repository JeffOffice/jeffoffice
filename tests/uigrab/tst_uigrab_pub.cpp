// Developer tool, not a test of behavior: with JO_SHOT_DIR set, opens JeffPub's dialogs and File pages one by one and saves a picture of each (docs/design/STYLE.md).
//   JO_SHOT_DIR=/some/folder QT_QPA_PLATFORM=offscreen ./jo_uigrab_pub
#include "uigrab_app.h"
#include "app/mainwindow.h"
#include "app/theme.h"
#include "core/fonts.h"
#include "../testrunner.h"

#include <QtTest>

namespace {

class UiGrabPub : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void initTestCase() { jp::initCore(); jp::applyUiTheme(1); }

    void grabTheDialogs()
    {
        const QString dir = qEnvironmentVariable("JO_SHOT_DIR");
        if (dir.isEmpty()) QSKIP("JO_SHOT_DIR is not set");
        jp::MainWindow w;
        w.resize(1300, 900);
        auto doc = jp::Document::blank(QSizeF(396, 612));
        for (int i = 0; i < 3; ++i) doc->pages.push_back(std::make_shared<jp::Page>());
        w.editor()->setDocument(std::move(doc));
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        uigrab::grabDialogs(w, dir + QStringLiteral("/dialogs"));
        uigrab::grabPages(w, dir + QStringLiteral("/pages"), {QStringLiteral("new"), QStringLiteral("open"), QStringLiteral("print"), QStringLiteral("export"), QStringLiteral("share"), QStringLiteral("info"), QStringLiteral("about")});
    }
};

} // namespace

JP_TEST_CLASS(UiGrabPub)
#include "tst_uigrab_pub.moc"
