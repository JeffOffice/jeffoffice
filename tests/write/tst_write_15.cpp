// JeffWrite's Read Aloud and missing fonts (apps/write/app/writewindow_speech.cpp; the shared parts are libs/shell/app/readaloud and missingfonts, tested in jptests): Review > Read Aloud
// reads the body from the caret or the selection and then the footnotes, and shows the word being said; the bar after a document with a font this computer lacks opens, its Details
// dialog, and Replace in Document as one step. A fake speech engine stands for the system's voices.
#include "tst_write.h"
#include "app/missingfonts.h"
#include "app/readaloud.h"
#include "../fakespeech.h"

#include <QComboBox>
#include <QDockWidget>
#include <QLabel>
#include <QPushButton>
#include <QScopeGuard>
#include <QTableWidget>

namespace {

const QString kGone = QStringLiteral("Zzyzx Gothic Display");

using FakeEngine = jptest::FakeSpeech;

FakeEngine *g_engine = nullptr;

// The test's engine for the bars made inside its scope.
struct FakeVoices {
    FakeVoices()
    {
        g_engine = nullptr;
        setSpeechEngineFactory([] { return g_engine = new FakeEngine; });
    }
    ~FakeVoices() { setSpeechEngineFactory({}); }
};

QStringList familiesOf(QTextDocument *d)
{
    QStringList out;
    for (QTextBlock b = d->begin(); b.isValid(); b = b.next())
        for (auto it = b.begin(); !it.atEnd(); ++it) {
            const QStringList f = it.fragment().charFormat().fontFamilies().toStringList();
            out << (f.isEmpty() ? QString() : f.first());
        }
    return out;
}

QString standIn() { return QFontDatabase::hasFamily(QStringLiteral("Arimo")) ? QStringLiteral("Arimo") : QFontDatabase::families().value(0); }

}

// Three paragraphs, the first in a font no computer has, with a footnote cited at the end of the second.
std::unique_ptr<WriteDoc> WriteTests::speechDoc()
{
    const QByteArray first = "<w:p><w:r><w:rPr><w:rFonts w:ascii=\"" + kGone.toUtf8() + "\" w:hAnsi=\"" + kGone.toUtf8() + "\"/></w:rPr><w:t>Alpha one.</w:t></w:r></w:p>";
    const QByteArray second = "<w:p><w:r><w:t xml:space=\"preserve\">Beta two.</w:t></w:r><w:r><w:rPr><w:rStyle w:val=\"FootnoteReference\"/></w:rPr><w:footnoteReference w:id=\"2\"/></w:r></w:p>";
    const QByteArray third = "<w:p><w:r><w:t>Gamma three.</w:t></w:r></w:p>";
    return open(notesDocx(first + second + third, footnotesPart({{2, plainRun(" Note text.")}})));
}

void WriteTests::readAloudIsACommandThatAReadOnlyDocumentKeeps()
{
    WriteWindow w;
    w.setDocument(speechDoc(), QString());
    QVERIFY(w.act(QStringLiteral("review.readAloud")));
    QVERIFY(AppWindow::commandAllowedReadOnly(QStringLiteral("review.readAloud")));
}

void WriteTests::readAloudGoesFromTheCaretThroughTheTextAndThenTheFootnotes()
{
    const FakeVoices voices;
    WriteWindow w;
    w.show();
    w.setDocument(speechDoc(), QString());
    QTextCursor c(w.document()->story());
    c.setPosition(w.document()->story()->toPlainText().indexOf(QStringLiteral("Beta")));
    w.view()->setCursor(c);
    w.act(QStringLiteral("review.readAloud"))->trigger();
    ReadAloudBar *bar = readAloudBarOf(&w);
    QVERIFY(bar);
    QVERIFY(g_engine);
    QVERIFY(g_engine->said.value(0).startsWith(QStringLiteral("Beta two.")));
    g_engine->word(5, 3);
    QVERIFY(!w.view()->cursor().hasSelection());   // (the word is tinted, never selected: what is typed is not put over it)
    QCOMPARE(w.view()->findHighlights().size(), 1);
    g_engine->end();
    QCOMPARE(g_engine->said.last(), QStringLiteral("Gamma three."));
    g_engine->end();
    QVERIFY(g_engine->said.last().contains(QStringLiteral("Note text.")));   // (the footnote comes after the text)
    g_engine->end();
    QCOMPARE(bar->state(), ReadAloudBar::State::Stopped);
    QVERIFY(!w.view()->cursor().hasSelection());
    QVERIFY(w.view()->findHighlights().isEmpty());   // (the word shown is taken away at the end)
}

void WriteTests::typingWhileReadingStopsTheReadingAndKeepsWhatWasTyped()
{
    const FakeVoices voices;
    WriteWindow w;
    w.show();
    w.setDocument(speechDoc(), QString());
    const QString before = w.document()->story()->toPlainText();
    QTextCursor c(w.document()->story());
    c.setPosition(before.indexOf(QStringLiteral("Beta")));
    w.view()->setCursor(c);
    w.act(QStringLiteral("review.readAloud"))->trigger();
    ReadAloudBar *bar = readAloudBarOf(&w);
    QVERIFY(bar);
    g_engine->word(0, 4);   // (the word "Beta" is shown)
    QTest::keyClicks(w.view(), QStringLiteral("Z"));
    QCOMPARE(bar->state(), ReadAloudBar::State::Stopped);
    QTRY_VERIFY(!bar->isVisible());
    // only what was typed changed: the word being read was not selected, so it is not gone
    QString expected = before;
    expected.insert(before.indexOf(QStringLiteral("Beta")), QLatin1Char('Z'));
    QCOMPARE(w.document()->story()->toPlainText(), expected);
    g_engine->word(1, 2);   // (a word that was on its way does nothing)
    QVERIFY(w.view()->findHighlights().isEmpty());
}

void WriteTests::deletingTheFootnoteWhileItIsReadIsSafe()
{
    const FakeVoices voices;
    WriteWindow w;
    w.show();
    w.setDocument(speechDoc(), QString());
    w.act(QStringLiteral("review.readAloud"))->trigger();
    ReadAloudBar *bar = readAloudBarOf(&w);
    QVERIFY(bar);
    g_engine->end();
    g_engine->end();
    g_engine->end();   // (now the footnote is being said)
    QVERIFY(g_engine->said.last().contains(QStringLiteral("Note text.")));
    QTextDocument *story = w.document()->story();
    QTextCursor c(story);
    const int at = story->toPlainText().indexOf(QChar(QChar::ObjectReplacementCharacter));
    QVERIFY(at >= 0);
    c.setPosition(at);
    c.setPosition(at + 1, QTextCursor::KeepAnchor);
    c.removeSelectedText();   // (the footnote's reference goes, and with it the note)
    g_engine->word(0, 4);
    g_engine->end();
    QCOMPARE(bar->state(), ReadAloudBar::State::Stopped);
}

void WriteTests::footnotesAreScannedForMissingFontsAndReplaced()
{
    const QScopeGuard clean([] { setFontChoice(kGone, QString()); });
    const QByteArray font = "<w:rPr><w:rFonts w:ascii=\"" + standIn().toUtf8() + "\" w:hAnsi=\"" + standIn().toUtf8() + "\"/></w:rPr>";
    const QByteArray gone = "<w:r><w:rPr><w:rFonts w:ascii=\"" + kGone.toUtf8() + "\" w:hAnsi=\"" + kGone.toUtf8() + "\"/></w:rPr><w:t xml:space=\"preserve\"> In the note.</w:t></w:r>";
    const QByteArray body = "<w:p><w:r>" + font + "<w:t>Body text.</w:t></w:r><w:r><w:rPr><w:rStyle w:val=\"FootnoteReference\"/></w:rPr><w:footnoteReference w:id=\"2\"/></w:r></w:p>";
    WriteWindow w;
    w.show();
    w.setDocument(open(notesDocx(body, footnotesPart({{2, gone}}))), QString());
    auto *dock = w.findChild<QDockWidget *>(QStringLiteral("missingFontsDock"));
    QVERIFY(dock);
    QTRY_VERIFY(dock->isVisible());
    withDialog([&] { w.findChild<QPushButton *>(QStringLiteral("missingFontsDetails"))->click(); }, QStringLiteral("missingFontsDialog"), [&](QDialog *d) {
        auto *t = d->findChild<QTableWidget *>(QStringLiteral("missingFontsTable"));
        int row = -1;
        for (int r = 0; r < t->rowCount(); ++r)
            if (t->item(r, 0)->text() == kGone) row = r;
        QVERIFY(row >= 0);
        QVERIFY(t->item(row, 2)->text().contains(QStringLiteral("Footnotes")));
        auto *combo = qobject_cast<QComboBox *>(t->cellWidget(row, 1));
        combo->showPopup();
        combo->hidePopup();
        combo->setCurrentIndex(combo->findData(standIn()));
        Q_EMIT combo->activated(combo->currentIndex());
        t->selectRow(row);
        d->findChild<QPushButton *>(QStringLiteral("missingFontsReplace"))->click();
        QVERIFY(d->findChild<QLabel *>(QStringLiteral("missingFontsNote"))->text().contains(QStringLiteral("replaced")));
    });
    QCOMPARE(familiesOf(w.document()->editableFootnote(2)).last(), standIn());
}

void WriteTests::textThatNamesNoFontIsReplacedThroughTheDefaultFont()
{
    const QScopeGuard clean([] { setFontChoice(kGone, QString()); });
    const QByteArray styles = "<w:docDefaults><w:rPrDefault><w:rPr><w:rFonts w:ascii=\"" + kGone.toUtf8() + "\" w:hAnsi=\"" + kGone.toUtf8() + "\"/></w:rPr></w:rPrDefault></w:docDefaults>";
    WriteWindow w;
    w.show();
    w.setDocument(open(makeDocx("<w:p><w:r><w:t>Plain words</w:t></w:r></w:p>", styles)), QString());
    auto *dock = w.findChild<QDockWidget *>(QStringLiteral("missingFontsDock"));
    QVERIFY(dock);
    QTRY_VERIFY(dock->isVisible());
    int changed = -1;
    withDialog([&] { w.findChild<QPushButton *>(QStringLiteral("missingFontsDetails"))->click(); }, QStringLiteral("missingFontsDialog"), [&](QDialog *d) {
        auto *t = d->findChild<QTableWidget *>(QStringLiteral("missingFontsTable"));
        int row = -1;
        for (int r = 0; r < t->rowCount(); ++r)
            if (t->item(r, 0)->text() == kGone) row = r;
        QVERIFY(row >= 0);
        auto *combo = qobject_cast<QComboBox *>(t->cellWidget(row, 1));
        combo->showPopup();
        combo->hidePopup();
        combo->setCurrentIndex(combo->findData(standIn()));
        Q_EMIT combo->activated(combo->currentIndex());
        t->selectRow(row);
        d->findChild<QPushButton *>(QStringLiteral("missingFontsReplace"))->click();
        const QString said = d->findChild<QLabel *>(QStringLiteral("missingFontsNote"))->text();
        changed = said.contains(QStringLiteral("replaced")) ? 1 : 0;
        QVERIFY2(!said.contains(QStringLiteral("nothing was changed")), qPrintable(said));
    });
    QCOMPARE(changed, 1);
    QCOMPARE(familiesOf(w.document()->story()).value(0), standIn());   // (the run that named no font names the new one now)
    QTRY_VERIFY(!dock->isVisible());   // (and the font is no longer missing)
}

void WriteTests::readAloudReadsOnlyTheSelection()
{
    const FakeVoices voices;
    WriteWindow w;
    w.show();
    w.setDocument(speechDoc(), QString());
    QTextCursor c(w.document()->story());
    const int at = w.document()->story()->toPlainText().indexOf(QStringLiteral("Gamma"));
    c.setPosition(at);
    c.setPosition(at + 5, QTextCursor::KeepAnchor);
    w.view()->setCursor(c);
    w.act(QStringLiteral("review.readAloud"))->trigger();
    QCOMPARE(g_engine->said, QStringList{QStringLiteral("Gamma")});
    g_engine->end();
    QCOMPARE(readAloudBarOf(&w)->state(), ReadAloudBar::State::Stopped);
}

void WriteTests::aDocumentWithAFontThisComputerLacksShowsTheMissingFontsBar()
{
    const QScopeGuard clean([] { setFontChoice(kGone, QString()); });
    WriteWindow w;
    w.show();
    w.setDocument(speechDoc(), QString());
    auto *dock = w.findChild<QDockWidget *>(QStringLiteral("missingFontsDock"));
    QVERIFY(dock);
    QTRY_VERIFY(dock->isVisible());
    withDialog([&] { w.findChild<QPushButton *>(QStringLiteral("missingFontsDetails"))->click(); }, QStringLiteral("missingFontsDialog"), [&](QDialog *d) {
        auto *t = d->findChild<QTableWidget *>(QStringLiteral("missingFontsTable"));
        QCOMPARE(t->rowCount(), 1);
        QCOMPARE(t->item(0, 0)->text(), kGone);
        QVERIFY(!t->item(0, 2)->text().isEmpty());   // (where it is used)
    });
}

void WriteTests::replaceInDocumentChangesTheRunsOfTheFontAsOneStep()
{
    const QScopeGuard clean([] { setFontChoice(kGone, QString()); });
    WriteWindow w;
    w.show();
    w.setDocument(speechDoc(), QString());
    QCOMPARE(familiesOf(w.document()->story()).value(0), kGone);
    withDialog([&] { w.findChild<QPushButton *>(QStringLiteral("missingFontsDetails"))->click(); }, QStringLiteral("missingFontsDialog"), [&](QDialog *d) {
        auto *t = d->findChild<QTableWidget *>(QStringLiteral("missingFontsTable"));
        auto *combo = qobject_cast<QComboBox *>(t->cellWidget(0, 1));
        combo->showPopup();   // (the fonts are put in the box when it is first opened)
        combo->hidePopup();
        combo->setCurrentIndex(combo->findData(standIn()));
        Q_EMIT combo->activated(combo->currentIndex());
        t->selectRow(0);
        d->findChild<QPushButton *>(QStringLiteral("missingFontsReplace"))->click();
    });
    QCOMPARE(familiesOf(w.document()->story()).value(0), standIn());
    w.document()->undo();   // (one step)
    QCOMPARE(familiesOf(w.document()->story()).value(0), kGone);
}

void WriteTests::aFontChoiceHoldsForTheNextDocumentAndTheFileKeepsTheName()
{
    const QScopeGuard clean([] { setFontChoice(kGone, QString()); });
    setFontChoice(kGone, standIn());
    setFontReplacement(kGone, QString());   // (as in a program just started: a choice is in force once a document is checked)
    WriteWindow w;
    w.show();
    w.setDocument(speechDoc(), QString());
    QCOMPARE(fontReplacement(kGone), standIn());
    QCOMPARE(familiesOf(w.document()->story()).value(0), kGone);
}
