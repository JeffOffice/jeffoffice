// JeffWrite's Read Aloud and missing fonts (apps/write/app/writewindow_speech.cpp; the shared parts are libs/shell/app/readaloud and missingfonts, tested in jptests): Review > Read Aloud
// reads the body from the caret or the selection and then the footnotes, and shows the word being said; the bar after a document with a font this computer lacks opens, its Details
// dialog, and Replace in Document as one step. A fake speech engine stands for the system's voices.
#include "tst_write.h"
#include "app/missingfonts.h"
#include "app/readaloud.h"

#include <QComboBox>
#include <QDockWidget>
#include <QPushButton>
#include <QScopeGuard>
#include <QTableWidget>

namespace {

const QString kGone = QStringLiteral("Zzyzx Gothic Display");

class FakeEngine : public SpeechEngine {
public:
    QStringList said;
    QList<SpeechVoice> voices() const override { return {{QStringLiteral("a"), QStringLiteral("Ann"), QStringLiteral("en_US")}}; }
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
    QCOMPARE(w.view()->cursor().selectedText(), QStringLiteral("two"));
    g_engine->end();
    QCOMPARE(g_engine->said.last(), QStringLiteral("Gamma three."));
    g_engine->end();
    QVERIFY(g_engine->said.last().contains(QStringLiteral("Note text.")));   // (the footnote comes after the text)
    g_engine->end();
    QCOMPARE(bar->state(), ReadAloudBar::State::Stopped);
    QVERIFY(!w.view()->cursor().hasSelection());   // (the word shown is taken away at the end)
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
