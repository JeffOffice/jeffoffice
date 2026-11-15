// JeffCrunch's Read Aloud and missing fonts (apps/crunch/app/crunchwindow_speech.cpp over libs/shell/app/readaloud and missingfonts, and EditSession::fontsInUse and replaceFont): Review > Read
// Aloud reads the selected cells row by row and shows the cell being said; the bar for a workbook with a font this computer lacks, Replace in Document as one step, and the PDF drawn in the
// stand-in the person chose. A fake speech engine stands for the system's voices. Every workbook is built here.
#include "app/crunchwindow.h"
#include "app/missingfonts.h"
#include "app/readaloud.h"
#include "core/editsession.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "../testrunner.h"
#include "../withdialog.h"

#include <QAction>
#include <QComboBox>
#include <QDockWidget>
#include <QFontDatabase>
#include <QPushButton>
#include <QScopeGuard>
#include <QTableWidget>
#include <QTest>
#include <QUndoStack>

using namespace jp;
using namespace crunchtest;

namespace {

const QString kGone = QStringLiteral("Zzyzx Gothic Display");

class FakeEngine : public jp::SpeechEngine {
public:
    QStringList said;
    QList<jp::SpeechVoice> voices() const override { return {{QStringLiteral("a"), QStringLiteral("Ann"), QStringLiteral("en_US")}}; }
    void setVoice(const QString &) override {}
    void setRate(double) override {}
    void say(const QString &text) override { said << text; }
    void pause() override {}
    void resume() override {}
    void stop() override {}
    void word(int start, int length) { Q_EMIT wordSpoken(start, length); }
    void end() { Q_EMIT finished(); }
};

FakeEngine *g_engine = nullptr;

struct FakeVoices {
    FakeVoices()
    {
        g_engine = nullptr;
        jp::setSpeechEngineFactory([] { return g_engine = new FakeEngine; });
    }
    ~FakeVoices() { jp::setSpeechEngineFactory({}); }
};

QString standIn() { return QFontDatabase::hasFamily(QStringLiteral("Cousine")) ? QStringLiteral("Cousine") : QFontDatabase::families().value(0); }

// Two rows: Alpha, Beta, Gamma, and 12, Delta. The cells A1, B1, A2, and B2 are in a font no computer has; C1 is in an installed one.
QByteArray speechBook()
{
    const QString styles = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<styleSheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\">"
                                          "<fonts count=\"2\"><font><sz val=\"11\"/><name val=\"%1\"/></font><font><sz val=\"11\"/><name val=\"%2\"/></font></fonts>"
                                          "<fills count=\"2\"><fill><patternFill patternType=\"none\"/></fill><fill><patternFill patternType=\"gray125\"/></fill></fills>"
                                          "<borders count=\"1\"><border/></borders><cellStyleXfs count=\"1\"><xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\"/></cellStyleXfs>"
                                          "<cellXfs count=\"2\"><xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"0\"/><xf numFmtId=\"0\" fontId=\"1\" fillId=\"0\" borderId=\"0\" xfId=\"0\" applyFont=\"1\"/></cellXfs>"
                                          "</styleSheet>")
                               .arg(kGone, standIn());
    const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\" t=\"s\"><v>0</v></c><c r=\"B1\" t=\"s\"><v>1</v></c><c r=\"C1\" s=\"1\" t=\"s\"><v>2</v></c></row>"
                                        "<row r=\"2\"><c r=\"A2\"><v>12</v></c><c r=\"B2\" t=\"s\"><v>3</v></c></row>");
    return buildXlsx({worksheet(rows)}, sharedStringsXml({QStringLiteral("<t>Alpha</t>"), QStringLiteral("<t>Beta</t>"), QStringLiteral("<t>Gamma</t>"), QStringLiteral("<t>Delta</t>")}), QString(), QString(),
                     styles.toUtf8());
}

class CrunchSpeechTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void cleanup() { jp::setFontChoice(kGone, QString()); }

    void theSelectedCellsAreReadRowByRowAndEachCellIsShown()
    {
        const FakeVoices voices;
        EditWindow e;
        QVERIFY(e.open(speechBook()));
        QVERIFY(e.win.w.act(QStringLiteral("review.readAloud")));
        e.view()->selectRange(0, 0, 1, 1);
        e.win.w.act(QStringLiteral("review.readAloud"))->trigger();
        jp::ReadAloudBar *bar = jp::readAloudBarOf(&e.win.w);
        QVERIFY(bar);
        QCOMPARE(g_engine->said, QStringList{QStringLiteral("Alpha, Beta")});
        g_engine->word(7, 4);   // "Beta"
        QCOMPARE(e.view()->marks().size(), size_t(1));
        QCOMPARE(e.view()->marks().front(), (grid::GridRange{0, 1, 0, 1}));
        g_engine->end();
        QCOMPARE(g_engine->said.last(), QStringLiteral("12, Delta"));
        g_engine->end();
        QCOMPARE(bar->state(), jp::ReadAloudBar::State::Stopped);
        QVERIFY(e.view()->marks().empty());   // (the cell shown is let go at the end)
    }
    void oneCellReadsFromItToTheEndOfTheSheet()
    {
        const FakeVoices voices;
        EditWindow e;
        QVERIFY(e.open(speechBook()));
        e.select(QStringLiteral("B1"));
        e.win.w.act(QStringLiteral("review.readAloud"))->trigger();
        QCOMPARE(g_engine->said, QStringList{QStringLiteral("Beta, Gamma")});
        g_engine->end();
        QCOMPARE(g_engine->said.last(), QStringLiteral("12, Delta"));
    }
    void aWorkbookWithAFontThisComputerLacksShowsTheBarAndReplaceChangesItInOneStep()
    {
        EditWindow e;
        QVERIFY(e.open(speechBook()));
        auto *dock = e.win.w.findChild<QDockWidget *>(QStringLiteral("missingFontsDock"));
        QVERIFY(dock);
        QTRY_VERIFY(dock->isVisible());
        QCOMPARE(e.win.w.session()->fontsInUse().size(), 2);
        withDialog([&] { e.win.w.findChild<QPushButton *>(QStringLiteral("missingFontsDetails"))->click(); }, QStringLiteral("missingFontsDialog"), [&](QDialog *d) {
            auto *t = d->findChild<QTableWidget *>(QStringLiteral("missingFontsTable"));
            QCOMPARE(t->rowCount(), 1);
            QCOMPARE(t->item(0, 0)->text(), kGone);
            QCOMPARE(t->item(0, 2)->text(), QStringLiteral("Sheet1"));
            auto *combo = qobject_cast<QComboBox *>(t->cellWidget(0, 1));
            combo->setCurrentIndex(combo->findData(standIn() == QStringLiteral("Cousine") ? QStringLiteral("Cousine") : standIn()));
            Q_EMIT combo->activated(combo->currentIndex());
            QCOMPARE(e.win.w.session()->fontsInUse().first().family, kGone);   // (a choice is for display: the workbook keeps the name)
            t->selectRow(0);
            d->findChild<QPushButton *>(QStringLiteral("missingFontsReplace"))->click();
        });
        // the four cells have the stand-in now, and the bar goes
        for (const auto &use : e.win.w.session()->fontsInUse()) QVERIFY2(use.family != kGone, qPrintable(use.family));
        QTRY_VERIFY(!dock->isVisible());
        QVERIFY(e.win.w.undoStack()->count() > 0);
        e.win.w.undoStack()->undo();   // (one step: the sheet's)
        auto gone = [&] {
            for (const auto &use : e.win.w.session()->fontsInUse())
                if (use.family == kGone) return true;
            return false;
        };
        QTRY_VERIFY(gone());
    }
    void aWorkbookWithOnlyInstalledFontsHasNoBar()
    {
        EditWindow e;
        QVERIFY(e.openBlank());
        auto *dock = e.win.w.findChild<QDockWidget *>(QStringLiteral("missingFontsDock"));
        QVERIFY(!dock || !dock->isVisible());
    }
};

} // namespace

JP_TEST_CLASS(CrunchSpeechTests)
#include "tst_crunch_speech.moc"
