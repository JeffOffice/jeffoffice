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
        uigrab::grabPages(w, dir + QStringLiteral("/pages"), {QStringLiteral("new"), QStringLiteral("open"), QStringLiteral("info"), QStringLiteral("print"), QStringLiteral("export"), QStringLiteral("about")});
    }

    // The Date and Time dialog with its Language list (German chosen), and a slide with the slide number put in a box.
    void grabDateAndSlideNumber()
    {
        const QString dir = qEnvironmentVariable("JO_SHOT_DIR");
        if (dir.isEmpty()) QSKIP("JO_SHOT_DIR is not set");
        QDir().mkpath(dir + QStringLiteral("/fields"));
        jp::DeckWindow w;
        w.resize(1300, 900);
        w.show();
        QVERIFY(QTest::qWaitForWindowExposed(&w));
        jp::Editor *ed = w.editor();
        w.newSlide();
        ed->setCurrentPage(1);
        const QString boxId = ed->doc()->pages[1]->items[0]->id;
        QTextCursor(ed->doc()->storyDoc(static_cast<jp::TextItem *>(ed->doc()->pages[1]->items[0].get())->storyId)).insertText(QStringLiteral("Slide "));
        ed->select(boxId);
        ed->beginTextEdit(boxId, 6);
        w.act(QStringLiteral("insert.slideNumberField"))->trigger();
        QCoreApplication::processEvents();
        w.grab().save(dir + QStringLiteral("/fields/deck-slide-number.png"));
        QTimer watch;
        watch.setInterval(20);
        QObject::connect(&watch, &QTimer::timeout, &watch, [&] {
            auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!d) return;
            watch.stop();
            if (auto *language = d->findChild<QComboBox *>(QStringLiteral("dateLanguage"))) language->setCurrentIndex(language->findData(QStringLiteral("de-DE")));
            QCoreApplication::processEvents();
            d->grab().save(dir + QStringLiteral("/fields/deck-date-language.png"));
            d->reject();
        });
        watch.start();
        w.act(QStringLiteral("insert.dateTime"))->trigger();
    }
};

} // namespace

JP_TEST_CLASS(UiGrabDeck)
#include "tst_uigrab_deck.moc"
