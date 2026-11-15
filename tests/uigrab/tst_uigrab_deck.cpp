// Developer tool, not a test of behavior: with JO_SHOT_DIR set, opens JeffDeck's dialogs and File pages one by one and saves a picture of each (docs/design/STYLE.md).
//   JO_SHOT_DIR=/some/folder QT_QPA_PLATFORM=offscreen ./jo_uigrab_deck
#include "uigrab_app.h"
#include "../deck/deckhelpers.h"
#include "app/deckwindow.h"
#include "app/theme.h"
#include "core/fonts.h"
#include "../testrunner.h"

#include <QtTest>

namespace {

class UiGrabDeck : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void initTestCase() { jp::initCore(); jp::applyUiTheme(1); }

    void grabTheDialogs()
    {
        const QString dir = qEnvironmentVariable("JO_SHOT_DIR");
        if (dir.isEmpty()) QSKIP("JO_SHOT_DIR is not set");
        jp::DeckWindow w;
        w.resize(1300, 900);
        w.editor()->setDocument(deckhelp::showDeck(3));
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        uigrab::grabDialogs(w, dir + QStringLiteral("/dialogs"));
        uigrab::grabPages(w, dir + QStringLiteral("/pages"));
    }
};

} // namespace

JP_TEST_CLASS(UiGrabDeck)
#include "tst_uigrab_deck.moc"
