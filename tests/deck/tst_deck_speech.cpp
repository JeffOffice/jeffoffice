// JeffDeck's Read Aloud and missing fonts (apps/deck/app/deckwindow_dialogs.cpp over libs/page/app/readaloudpage and pagefonts; the shared bar and dialog are tested in jptests): Review > Read
// Aloud reads the slides' text in order from the text cursor or the selection and shows the word on its slide; the bar for a presentation with a font this computer lacks, Replace in
// Document as one step, and the PDF that is drawn in the stand-in the person chose. A fake speech engine stands for the system's voices.
#include "deckhelpers.h"
#include "app/missingfonts.h"
#include "app/readaloud.h"
#include "../fakespeech.h"
#include "io/deckoutput.h"
#include "../withdialog.h"

#include <QComboBox>
#include <QDockWidget>
#include <QPushButton>
#include <QScopeGuard>
#include <QTableWidget>
#include <QTemporaryDir>

namespace {

const QString kGone = QStringLiteral("Zzyzx Gothic Display");

using FakeEngine = jptest::FakeSpeech;

FakeEngine *g_engine = nullptr;

struct FakeVoices {
    FakeVoices()
    {
        g_engine = nullptr;
        jp::setSpeechEngineFactory([] { return g_engine = new FakeEngine; });
    }
    ~FakeVoices() { jp::setSpeechEngineFactory({}); }
};

// A text box on a slide with these words (in `family` when it is given).
TextItem *addBox(Document &doc, int page, const QString &text, double y, const QString &family = QString())
{
    auto box = std::make_shared<TextItem>();
    box->rect = QRectF(72, y, 400, 60);
    box->storyId = doc.createStory();
    QTextCursor c(doc.storyDoc(box->storyId));
    QTextCharFormat cf;
    cf.setFontPointSize(24);
    if (!family.isEmpty()) cf.setFontFamilies(QStringList{family});
    c.insertText(text, cf);
    doc.pages[page]->items.push_back(box);
    return box.get();
}

QString familyOf(Editor *ed, const TextItem *box)
{
    const QStringList f = ed->doc()->storyDoc(box->storyId)->begin().begin().fragment().charFormat().fontFamilies().toStringList();
    return f.isEmpty() ? QString() : f.first();
}

QString standIn() { return QFontDatabase::hasFamily(QStringLiteral("Cousine")) ? QStringLiteral("Cousine") : QFontDatabase::families().value(0); }

class DeckSpeechTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initDeckTestCase(); }
    void cleanup() { jp::setFontChoice(kGone, QString()); }

    void theSlidesAreReadInOrderFromTheCursorAndEachWordIsShownOnItsSlide()
    {
        const FakeVoices voices;
        DeckWindow w;
        w.show();
        Editor *ed = w.editor();
        Document *d = ed->doc();
        d->pages.clear();
        d->addPage();
        d->addPage();
        addBox(*d, 0, QStringLiteral("First slide words."), 72);
        TextItem *second = addBox(*d, 1, QStringLiteral("Second slide words."), 72);
        ed->setCurrentPage(0);
        w.act(QStringLiteral("review.readAloud"))->trigger();
        jp::ReadAloudBar *bar = jp::readAloudBarOf(&w);
        QVERIFY(bar);
        QCOMPARE(g_engine->said, QStringList{QStringLiteral("First slide words.")});
        g_engine->word(6, 5);
        QVERIFY(ed->hasReadHighlight());   // (the word is tinted, never selected)
        QVERIFY(!ed->cursor().hasSelection());
        g_engine->end();
        QCOMPARE(g_engine->said.last(), QStringLiteral("Second slide words."));
        g_engine->word(0, 6);
        QCOMPARE(ed->currentPage(), 1);   // (the slide of the word comes up)
        g_engine->end();
        QCOMPARE(bar->state(), jp::ReadAloudBar::State::Stopped);
        // from the cursor: the second slide's text alone when the cursor is in it
        ed->beginTextEdit(second->id, 0);
        w.act(QStringLiteral("review.readAloud"))->trigger();
        QCOMPARE(g_engine->said.last(), QStringLiteral("Second slide words."));
    }
    void anEditWhileReadingStopsItAndClosesTheBar()
    {
        const FakeVoices voices;
        DeckWindow w;
        w.show();
        Editor *ed = w.editor();
        Document *d = ed->doc();
        d->pages.clear();
        d->addPage();
        TextItem *box = addBox(*d, 0, QStringLiteral("One two three"), 72);
        ed->setCurrentPage(0);
        w.act(QStringLiteral("review.readAloud"))->trigger();
        jp::ReadAloudBar *bar = jp::readAloudBarOf(&w);
        QVERIFY(bar);
        g_engine->word(4, 3);
        QVERIFY(ed->hasReadHighlight());
        ed->change(QStringLiteral("Typing"), [&] { QTextCursor(ed->doc()->storyDoc(box->storyId)).insertText(QStringLiteral("Z")); });
        QCOMPARE(bar->state(), jp::ReadAloudBar::State::Stopped);
        QVERIFY(!ed->hasReadHighlight());
        QCOMPARE(ed->doc()->storyDoc(box->storyId)->toPlainText(), QStringLiteral("ZOne two three"));
    }
    void aSelectionIsAllThatIsRead()
    {
        const FakeVoices voices;
        DeckWindow w;
        w.show();
        Editor *ed = w.editor();
        Document *d = ed->doc();
        d->pages.clear();
        d->addPage();
        TextItem *box = addBox(*d, 0, QStringLiteral("One two three"), 72);
        addBox(*d, 0, QStringLiteral("Other box."), 200);
        ed->setCurrentPage(0);
        ed->beginTextEdit(box->id, 0);
        QTextCursor c = ed->cursor();
        c.setPosition(4);
        c.setPosition(7, QTextCursor::KeepAnchor);
        ed->setCursor(c);
        w.act(QStringLiteral("review.readAloud"))->trigger();
        QCOMPARE(g_engine->said, QStringList{QStringLiteral("two")});
    }
    void aPresentationWithAFontThisComputerLacksShowsTheBarAndReplaceChangesItInOneStep()
    {
        DeckWindow w;
        w.show();
        Editor *ed = w.editor();
        Document *d = ed->doc();
        d->pages.clear();
        d->addPage();
        TextItem *box = addBox(*d, 0, QStringLiteral("Headline"), 72, kGone);
        Q_EMIT ed->documentReplaced();
        auto *dock = w.findChild<QDockWidget *>(QStringLiteral("missingFontsDock"));
        QVERIFY(dock);
        QTRY_VERIFY(dock->isVisible());
        withDialog([&] { w.findChild<QPushButton *>(QStringLiteral("missingFontsDetails"))->click(); }, QStringLiteral("missingFontsDialog"), [&](QDialog *dlg) {
            auto *t = dlg->findChild<QTableWidget *>(QStringLiteral("missingFontsTable"));
            int row = -1;   // (the master pages may use fonts this computer lacks too)
            for (int r = 0; r < t->rowCount(); ++r)
                if (t->item(r, 0)->text() == kGone) row = r;
            QVERIFY(row >= 0);
            QCOMPARE(t->item(row, 2)->text(), QStringLiteral("Slide 1"));
            auto *combo = qobject_cast<QComboBox *>(t->cellWidget(row, 1));
            combo->showPopup();   // (the fonts are put in the box when it is first opened)
            combo->hidePopup();
            combo->setCurrentIndex(combo->findData(standIn()));
            Q_EMIT combo->activated(combo->currentIndex());
            QCOMPARE(familyOf(ed, box), kGone);   // (a choice is for display: the file keeps the name)
            t->selectRow(row);
            dlg->findChild<QPushButton *>(QStringLiteral("missingFontsReplace"))->click();
        });
        QCOMPARE(familyOf(ed, box), standIn());
        ed->undo();
        QCOMPARE(familyOf(ed, box), kGone);
    }
    void thePdfIsDrawnInTheFontThePersonChose()
    {
        if (!QFontDatabase::hasFamily(QStringLiteral("Cousine"))) QSKIP("the bundled font Cousine is not loaded");
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        Document doc;
        doc.addPage();
        addBox(doc, 0, QStringLiteral("Headline in a font nobody has"), 72, kGone);
        auto pdf = [&](const QString &name) {
            const QString path = dir.filePath(name);
            QString err;
            if (!deck::exportPdf(doc, path, deck::OutputOptions(), &err)) return QByteArray();
            return fileBytes(path);
        };
        const QByteArray before = pdf(QStringLiteral("a.pdf"));
        QVERIFY(!before.isEmpty());
        QVERIFY(!before.contains("Cousine"));
        jp::setFontChoice(kGone, QStringLiteral("Cousine"));
        QVERIFY(pdf(QStringLiteral("b.pdf")).contains("Cousine"));
    }
};

} // namespace

JP_TEST_CLASS(DeckSpeechTests)
#include "tst_deck_speech.moc"
