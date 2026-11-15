// JeffPub's Read Aloud and missing fonts (apps/pub/app/mainwindow_actions.cpp over libs/page/app/readaloudpage and pagefonts; the shared bar and dialog are tested in jptests): Review > Read
// Aloud reads the text boxes in order from the text cursor or the selection and shows the word; the bar for a publication with a font this computer lacks, Replace in Document as one step,
// and the PDF that is drawn in the stand-in the person chose. A fake speech engine stands for the system's voices.
#include "pubhelpers.h"
#include "app/missingfonts.h"
#include "app/readaloud.h"
#include "../withdialog.h"

#include <QComboBox>
#include <QDockWidget>
#include <QPushButton>
#include <QScopeGuard>
#include <QTableWidget>
#include <QTemporaryDir>

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

// A text box of the page with these words (in `family` when it is given).
std::shared_ptr<jp::TextItem> addBox(jp::Document &doc, int page, const QString &text, double y, const QString &family = QString())
{
    auto box = std::make_shared<jp::TextItem>();
    box->rect = QRectF(72, y, 400, 60);
    box->storyId = doc.createStory();
    QTextCursor c(doc.storyDoc(box->storyId));
    QTextCharFormat cf;
    cf.setFontPointSize(14);
    if (!family.isEmpty()) cf.setFontFamilies(QStringList{family});
    c.insertText(text, cf);
    doc.pages[page]->items.push_back(box);
    return box;
}

QString familyOf(jp::Editor *ed, const std::shared_ptr<jp::TextItem> &box)
{
    const QTextDocument *d = ed->doc()->storyDoc(box->storyId);
    const QStringList f = d->begin().begin().fragment().charFormat().fontFamilies().toStringList();
    return f.isEmpty() ? QString() : f.first();
}

QString standIn() { return QFontDatabase::hasFamily(QStringLiteral("Cousine")) ? QStringLiteral("Cousine") : QFontDatabase::families().value(0); }

class PubSpeechTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initPubTestCase(); }
    void cleanup() { jp::setFontChoice(kGone, QString()); }

    void theStoriesAreReadInOrderFromTheTextCursor()
    {
        const FakeVoices voices;
        jp::MainWindow w;
        w.show();
        auto doc = std::make_unique<jp::Document>();
        doc->addPage();
        doc->addPage();
        addBox(*doc, 0, QStringLiteral("First box words."), 72);
        auto second = addBox(*doc, 0, QStringLiteral("Second box words."), 200);
        addBox(*doc, 1, QStringLiteral("Third box words."), 72);
        w.editor()->setDocument(std::move(doc));
        // from the second box's start
        w.editor()->beginTextEdit(second->id, 0);
        w.act(QStringLiteral("rev.readAloud"))->trigger();
        jp::ReadAloudBar *bar = jp::readAloudBarOf(&w);
        QVERIFY(bar);
        QCOMPARE(g_engine->said, QStringList{QStringLiteral("Second box words.")});
        g_engine->word(7, 3);
        QVERIFY(w.editor()->isEditingText());
        QCOMPARE(w.editor()->cursor().selectedText(), QStringLiteral("box"));
        g_engine->end();
        QCOMPARE(g_engine->said.last(), QStringLiteral("Third box words."));
        g_engine->word(0, 5);
        QCOMPARE(w.editor()->currentPage(), 1);   // (the page of the word shown comes up)
        g_engine->end();
        QCOMPARE(bar->state(), jp::ReadAloudBar::State::Stopped);
        QVERIFY(!w.editor()->cursor().hasSelection());
    }
    void withNoCursorItStartsAtTheFirstStoryOfThePage()
    {
        const FakeVoices voices;
        jp::MainWindow w;
        w.show();
        auto doc = std::make_unique<jp::Document>();
        doc->addPage();
        addBox(*doc, 0, QStringLiteral("Alpha."), 72);
        addBox(*doc, 0, QStringLiteral("Beta."), 200);
        w.editor()->setDocument(std::move(doc));
        w.act(QStringLiteral("rev.readAloud"))->trigger();
        QCOMPARE(g_engine->said, QStringList{QStringLiteral("Alpha.")});
    }
    void aSelectionOfWordsIsAllThatIsRead()
    {
        const FakeVoices voices;
        jp::MainWindow w;
        w.show();
        auto doc = std::make_unique<jp::Document>();
        doc->addPage();
        auto box = addBox(*doc, 0, QStringLiteral("One two three"), 72);
        addBox(*doc, 0, QStringLiteral("Other box."), 200);
        w.editor()->setDocument(std::move(doc));
        w.editor()->beginTextEdit(box->id, 0);
        QTextCursor c = w.editor()->cursor();
        c.setPosition(4);
        c.setPosition(7, QTextCursor::KeepAnchor);
        w.editor()->setCursor(c);
        w.act(QStringLiteral("rev.readAloud"))->trigger();
        QCOMPARE(g_engine->said, QStringList{QStringLiteral("two")});
    }
    void aPublicationWithAFontThisComputerLacksShowsTheBarAndReplaceChangesItInOneStep()
    {
        jp::MainWindow w;
        w.show();
        auto doc = std::make_unique<jp::Document>();
        doc->addPage();
        auto box = addBox(*doc, 0, QStringLiteral("Headline"), 72, kGone);
        addBox(*doc, 0, QStringLiteral("Plain"), 200);
        w.editor()->setDocument(std::move(doc));
        auto *dock = w.findChild<QDockWidget *>(QStringLiteral("missingFontsDock"));
        QVERIFY(dock);
        QTRY_VERIFY(dock->isVisible());
        withDialog([&] { w.findChild<QPushButton *>(QStringLiteral("missingFontsDetails"))->click(); }, QStringLiteral("missingFontsDialog"), [&](QDialog *d) {
            auto *t = d->findChild<QTableWidget *>(QStringLiteral("missingFontsTable"));
            QCOMPARE(t->rowCount(), 1);
            QCOMPARE(t->item(0, 0)->text(), kGone);
            QCOMPARE(t->item(0, 2)->text(), QStringLiteral("Page 1"));
            auto *combo = qobject_cast<QComboBox *>(t->cellWidget(0, 1));
            combo->setCurrentIndex(combo->findData(standIn()));
            Q_EMIT combo->activated(combo->currentIndex());
            QCOMPARE(familyOf(w.editor(), box), kGone);   // (a choice is for display: the publication keeps the name)
            t->selectRow(0);
            d->findChild<QPushButton *>(QStringLiteral("missingFontsReplace"))->click();
        });
        QCOMPARE(familyOf(w.editor(), box), standIn());
        w.editor()->undo();
        QCOMPARE(familyOf(w.editor(), box), kGone);
    }
    void aPublicationWithOnlyInstalledFontsHasNoBar()
    {
        jp::MainWindow w;
        w.show();
        auto doc = std::make_unique<jp::Document>();
        doc->addPage();
        addBox(*doc, 0, QStringLiteral("Plain"), 72, standIn());
        w.editor()->setDocument(std::move(doc));
        auto *dock = w.findChild<QDockWidget *>(QStringLiteral("missingFontsDock"));
        QVERIFY(!dock || !dock->isVisible());
    }
    void thePdfIsDrawnInTheFontThePersonChose()
    {
        if (!QFontDatabase::hasFamily(QStringLiteral("Cousine"))) QSKIP("the bundled font Cousine is not loaded");
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        jp::MainWindow w;
        auto doc = std::make_unique<jp::Document>();
        doc->addPage();
        addBox(*doc, 0, QStringLiteral("Headline in a font nobody has"), 72, kGone);
        w.editor()->setDocument(std::move(doc));
        auto pdf = [&](const QString &name) {
            jp::MainWindow::PdfSettings s;
            const QString path = dir.filePath(name);
            w.exportPdfTo(path, s);
            QFile f(path);
            f.open(QIODevice::ReadOnly);
            return f.readAll();
        };
        QVERIFY(!pdf(QStringLiteral("a.pdf")).contains("Cousine"));
        jp::setFontChoice(kGone, QStringLiteral("Cousine"));
        w.editor()->cache().clear();   // (as the dialog does when a choice is made)
        QVERIFY(pdf(QStringLiteral("b.pdf")).contains("Cousine"));
        QCOMPARE(familyOf(w.editor(), std::static_pointer_cast<jp::TextItem>(w.editor()->doc()->pages[0]->items.front())), kGone);
    }
};

} // namespace

JP_TEST_CLASS(PubSpeechTests)
#include "tst_pub_speech.moc"
