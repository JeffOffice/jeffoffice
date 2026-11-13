// JeffPub's tests of find, replace, spelling, and hyphenation.
#include "pubhelpers.h"
#include "../settingguard.h"
#include "../withdialog.h"

#include "app/grammar.h"
#include <QElapsedTimer>
#include <QLabel>
#include <QMenu>
#include <QPushButton>

namespace {
const QString kGrammarOn = QStringLiteral("proof/checkGrammarAsYouType");
// A sentence with one grammar issue and no misspelling (as in JeffWrite's tests).
const QString kAnTest = QStringLiteral("This is an test of the checker.");

// A publication of one text box with `text` (in `language`, when given), set into the window's editor: the box.
std::shared_ptr<jp::TextItem> boxWith(jp::MainWindow &w, const QString &text, const QString &language = QString())
{
    auto doc = jp::Document::blank(QSizeF(612, 792));
    auto t = std::make_shared<jp::TextItem>();
    t->rect = QRectF(72, 72, 300, 100);
    t->storyId = doc->createStory(text);
    doc->pages[0]->items.push_back(t);
    if (!language.isEmpty()) {
        QTextCursor c(doc->storyDoc(t->storyId));
        c.select(QTextCursor::Document);
        QTextCharFormat f;
        f.setProperty(jp::tp::Language, language);
        c.mergeCharFormat(f);
    }
    w.editor()->setDocument(std::move(doc));
    w.editor()->setGrammarDelayMs(0);
    return t;
}
}

class PubFindTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initPubTestCase(); }
    // (spelling as you type is one choice for the suite, kept in the settings: a test elsewhere in this program may have left it off, and these tests need it on)
    void init()
    {
        jp::setSpellAsYouType(true);
        jp::setGrammarAsYouType(true);
    }

    // Hyphenation in each bundled language matches LibreOffice's (libhyphen
    // 2.8 with the same pattern files, at least two letters each side),
    // including German compounds split in two stages and words with
    // apostrophes and hyphens. Checked on 26,700 dictionary words; these are
    // a sample.
    void hyphenationByLanguage()
    {
        const struct { const char *lang, *word, *points; } cases[] = {
            {"", "hyphenation", "2,6"},
            {"en-US", "kettledrum", "3,6"},
            {"en-GB", "kettledrum", "3,6"},
            {"es-MX", "constitucional", "4,6,8,11"},
            {"fr-FR", "démystificateur", "2,5,7,9,11"},
            {"de-DE", "Senkungsbewegung", "3,8,10,12"},
            {"de-DE", "reservierungsnummer", "2,5,8,13,16"},
            {"de-DE", "premierentermin", "3,5,6,9,12"},
            {"de-AT", "Biographieforschung", "3,6,10,13"},
            {"it-IT", "scalpare", "4"},
            {"nl-NL", "zeeëgelsoorten", "3,4,7,11"},
            {"pt-BR", "latinolátrico", "2,4,6,11"},
            {"pt-PT", "exemplarismo", "5,7,10"},
            {"en-US", "children's", "4"},
            // English breaks with two letters after the hyphen, as Publisher's
            // own layout does (nev-er, locat-ed: 37 of 494 breaks it made).
            {"en-US", "never", "3"},
            {"en-US", "located", "2,5"},
            {"en-US", "recently", "2,6"},
            {"fr-FR", "aujourd'hui", "2,6"},
            {"ja-JP", "hyphenation", ""},
            {"id-ID", "berlari", "3,5"},
            {"ru-RU", "электричество", "4,7,9"},
            {"ru-RU", "программирование", "3,7,9,11,13"},
            {"ru-RU", "государственный", "2,4,7,12"},
            {"ru-RU", "университет", "3,6,8"},
            {"ru-RU", "информация", "2,5,7"},
            {"ru-RU", "компьютер", "3,6"},
            {"ru-RU", "документ", "2,4"},
            {"ru-RU", "студентами", "3,6,8"},
        };
        for (const auto &c : cases) {
            QStringList got;
            for (int p : jp::hyphenationPoints(QString::fromUtf8(c.word), QString::fromLatin1(c.lang))) got << QString::number(p);
            QCOMPARE(got.join(','), QString::fromLatin1(c.points));
        }
        // Ukrainian: the patterns find breaks, none within two letters of an end, and "про-" is one.
        const QString ukWord = QString::fromUtf8("програмування");
        const QList<int> ukPoints = jp::hyphenationPoints(ukWord, QStringLiteral("uk-UA"));
        QVERIFY(!ukPoints.isEmpty());
        QVERIFY(ukPoints.contains(3));
        for (int p : ukPoints) QVERIFY(p >= 2 && p <= ukWord.size() - 2);
        // Russian: a word that is one exception in the patterns ("помни") has no break.
        QVERIFY(jp::hyphenationPoints(QString::fromUtf8("помни"), QStringLiteral("ru-RU")).isEmpty());
        // Layout breaks a word with its own language's patterns.
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto t = std::make_shared<jp::TextItem>();
        t->rect = QRectF(72, 72, 130, 200);
        t->storyId = doc->createStory(QStringLiteral("Wir planen die Reservierungsnummer."));
        doc->pages[0]->items.push_back(t);
        auto firstLine = [&](const QString &lang) {
            QTextCursor c(doc->storyDoc(t->storyId));
            c.select(QTextCursor::Document);
            QTextCharFormat f;
            f.setProperty(jp::tp::Language, lang);
            c.mergeCharFormat(f);
            jp::LayoutCache cache;
            jp::RenderOptions opt;
            const auto fl = cache.textFrame(*doc, *t, 1, opt);
            return fl.layout ? fl.layout->lineInfo(0).value(0).text.remove(QChar(0x00AD)) : QString();
        };
        // German breaks it (Reser-vie-rungs-num-mer); as English it's an
        // unknown word, and Japanese has no patterns, so it stays whole.
        const QString de = firstLine(QStringLiteral("de-DE"));
        QVERIFY2(QRegularExpression(QStringLiteral("Reser(vie(rungs(num)?)?)?$")).match(de).hasMatch(), qPrintable(de));
        for (const char *other : {"en-US", "ja-JP"}) {
            const QString line = firstLine(QString::fromLatin1(other));
            QVERIFY2(!line.contains(QStringLiteral("Reser")), qPrintable(line));
        }
    }

    // Which bundled dictionary text in a language uses.
    void dictionaryMatch()
    {
        // Windows: a folder whose name doesn't fit the system's code page
        // still opens (spelling was silently off for such user names).
#ifdef Q_OS_WIN
        {
            QTemporaryDir tmp;
            const QString odd = tmp.filePath(QString::fromUtf8("\u5b57\u5178 \u03a9"));
            QVERIFY(QDir().mkpath(odd));
            QFile f(odd + QStringLiteral("/x.dic"));
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write("1\nword\n");
            f.close();
            const QByteArray name = jp::dict::hunspellFileName(f.fileName());
            QVERIFY(QFile::exists(QFile::decodeName(name)));
        }
#endif
        QCOMPARE(jp::dict::match(QString()), QStringLiteral("en-US"));
        QCOMPARE(jp::dict::match(QStringLiteral("en-US")), QStringLiteral("en-US"));
        QCOMPARE(jp::dict::match(QStringLiteral("en_gb")), QStringLiteral("en-GB"));
        QCOMPARE(jp::dict::match(QStringLiteral("en-NZ")), QStringLiteral("en-GB"));
        QCOMPARE(jp::dict::match(QStringLiteral("en")), QStringLiteral("en-US"));
        QCOMPARE(jp::dict::match(QStringLiteral("es-AR")), QStringLiteral("es-AR"));
        QCOMPARE(jp::dict::match(QStringLiteral("es_cl")), QStringLiteral("es-CL"));
        QCOMPARE(jp::dict::match(QStringLiteral("es-419")), QStringLiteral("es-MX"));
        QCOMPARE(jp::dict::match(QStringLiteral("es-GQ")), QStringLiteral("es-MX"));
        QCOMPARE(jp::dict::match(QStringLiteral("es")), QStringLiteral("es-MX"));
        QCOMPARE(jp::dict::match(QStringLiteral("es-ES")), QStringLiteral("es-ES"));
        QCOMPARE(jp::dict::match(QStringLiteral("fr-CA")), QStringLiteral("fr-FR"));
        QCOMPARE(jp::dict::match(QStringLiteral("de-CH")), QStringLiteral("de-DE"));
        QCOMPARE(jp::dict::match(QStringLiteral("pt-AO")), QStringLiteral("pt-PT"));
        QCOMPARE(jp::dict::match(QStringLiteral("pt")), QStringLiteral("pt-BR"));
        QCOMPARE(jp::dict::match(QStringLiteral("nl-BE")), QStringLiteral("nl-NL"));
        QCOMPARE(jp::dict::match(QStringLiteral("ru")), QStringLiteral("ru-RU"));
        QCOMPARE(jp::dict::match(QStringLiteral("id_ID")), QStringLiteral("id-ID"));
        QCOMPARE(jp::dict::match(QStringLiteral("uk")), QStringLiteral("uk-UA"));
        QCOMPARE(jp::dict::languageName(QStringLiteral("uk-UA")), QStringLiteral("Ukrainian"));
        QCOMPARE(jp::dict::match(QStringLiteral("ja-JP")), QString());
        // Every listed language has its spelling and hyphenation files.
        for (const jp::dict::Language &l : jp::dict::languages()) {
            QVERIFY2(jp::dict::hasSpelling(l.code), qPrintable(l.code));
            QVERIFY2(!jp::dict::hyphenationFile(l.code).isEmpty(), qPrintable(l.code));
        }
        QCOMPARE(jp::dict::languageName(QStringLiteral("pt-BR")), QStringLiteral("Portuguese (Brazil)"));
    }

    // Each Spanish region has its own dictionary: a word only that region uses
    // is right there and wrong in Mexico's, and every region hyphenates.
    void spanishRegions()
    {
        const struct { const char *region, *name, *word; } regions[] = {
            {"AR", "Argentina", "abrime"}, {"BO", "Bolivia", "urpila"}, {"CL", "Chile", "afrechar"}, {"CO", "Colombia", "ahuyama"},
            {"CR", "Costa Rica", "natilla"}, {"CU", "Cuba", "boniatillo"}, {"DO", "Dominican Republic", "abulencia"}, {"EC", "Ecuador", "bolsico"},
            {"GT", "Guatemala", "pinolate"}, {"HN", "Honduras", "cumbear"}, {"NI", "Nicaragua", "coyundear"}, {"PA", "Panama", "ranchar"},
            {"PE", "Peru", "huaico"}, {"PR", "Puerto Rico", "caculear"}, {"PY", "Paraguay", "garandumba"}, {"SV", "El Salvador", "ruchique"},
            {"UY", "Uruguay", "paragolpe"}, {"VE", "Venezuela", "caney"},
        };
        for (const auto &r : regions) {
            const QString code = QStringLiteral("es-") + QLatin1String(r.region);
            QCOMPARE(jp::dict::match(code), code);
            QCOMPARE(jp::dict::languageName(code), QStringLiteral("Spanish (%1)").arg(QLatin1String(r.name)));
            QVERIFY2(jp::dict::hasSpelling(code), qPrintable(code));
            QVERIFY2(jp::dict::spell(code, QString::fromUtf8(r.word)), qPrintable(code));
            QVERIFY2(!jp::dict::spell(QStringLiteral("es-MX"), QString::fromUtf8(r.word)), qPrintable(code));
            QVERIFY2(jp::dict::spell(code, QStringLiteral("acción")), qPrintable(code));
            QVERIFY2(!jp::dict::spell(code, QStringLiteral("accion")), qPrintable(code));
        }
        // The United States has its own list, without the other regions' words.
        QCOMPARE(jp::dict::match(QStringLiteral("es-US")), QStringLiteral("es-US"));
        QVERIFY(jp::dict::spell(QStringLiteral("es-US"), QStringLiteral("acción")));
        QVERIFY(!jp::dict::spell(QStringLiteral("es-US"), QStringLiteral("paragolpe")));
        // The list names Mexico, then Spain, then the other regions.
        QStringList codes;
        for (const jp::dict::Language &l : jp::dict::languages())
            if (l.code.startsWith(QLatin1String("es-"))) codes << l.code;
        QCOMPARE(codes.value(0), QStringLiteral("es-MX"));
        QCOMPARE(codes.value(1), QStringLiteral("es-ES"));
        QCOMPARE(codes.size(), 21);
    }

    // Spelling checks each word in its own language: accented words, both
    // apostrophes, the German dictionary's older encoding, and no checking
    // for a language without a dictionary.
    void spellingByLanguage()
    {
        const struct { const char *lang, *word; bool ok; } words[] = {
            {"es-MX", "acción", true}, {"es-ES", "niño", true}, {"es-MX", "accion", false},
            {"fr-FR", "l’homme", true}, {"fr-FR", "l'homme", true}, {"fr-FR", "aujourd’hui", true}, {"fr-FR", "naïve", true}, {"fr-FR", "hommme", false},
            {"de-DE", "Straße", true}, {"de-DE", "größer", true}, {"de-DE", "Reservierungsnummer", true}, {"de-DE", "grösser", false},
            {"it-IT", "città", true}, {"it-IT", "dell’anno", true}, {"it-IT", "cittá", false},
            {"nl-NL", "één", true}, {"nl-NL", "fietsen", true}, {"nl-NL", "fietssen", false},
            {"pt-BR", "coração", true}, {"pt-PT", "acção", false}, {"pt-BR", "coracao", false},
            {"en-GB", "colour", true}, {"en-US", "colour", false}, {"en-CA", "colour", true}, {"en-AU", "organise", true},
            {"en-US", "don’t", true}, {"en-US", "don't", true},
            {"ru-RU", "привет", true}, {"ru-RU", "превет", false}, {"ru-RU", "студентами", true},
            {"id-ID", "selamat", true}, {"id-ID", "selamaat", false}, {"id-ID", "berlari", true},
            {"uk-UA", "привіт", true}, {"uk-UA", "прувіт", false}, {"uk-UA", "студентами", true},
        };
        for (const auto &w : words) {
            const QString code = jp::dict::match(QString::fromLatin1(w.lang));
            QVERIFY2(jp::dict::spell(code, QString::fromUtf8(w.word)) == w.ok, qPrintable(QStringLiteral("%1 %2").arg(code, QString::fromUtf8(w.word))));
        }
        QVERIFY(jp::spellingSuggestions(QStringLiteral("grösser"), QStringLiteral("de-DE")).contains(QStringLiteral("größer")));
        QVERIFY(jp::spellingSuggestions(QStringLiteral("accion"), QStringLiteral("es-MX")).contains(QStringLiteral("acción")));

        // In a story: each word by its own language.
        jp::MainWindow w;
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto t = std::make_shared<jp::TextItem>();
        t->rect = QRectF(72, 72, 300, 100);
        t->storyId = doc->createStory(QStringLiteral("Teh colour. Die Straße ist größer. Esto es una pruebba. こんにちは xyzzyq."));
        doc->pages[0]->items.push_back(t);
        QTextDocument *sd = doc->storyDoc(t->storyId);
        auto mark = [&](const QString &from, const QString &to, const QString &lang) {
            const QString text = sd->toPlainText();
            QTextCursor c(sd);
            c.setPosition(int(text.indexOf(from)));
            c.setPosition(int(text.indexOf(to) + to.size()), QTextCursor::KeepAnchor);
            QTextCharFormat f;
            f.setProperty(jp::tp::Language, lang);
            c.mergeCharFormat(f);
        };
        mark(QStringLiteral("Die"), QStringLiteral("größer."), QStringLiteral("de-DE"));
        mark(QStringLiteral("Esto"), QStringLiteral("pruebba."), QStringLiteral("es-ES"));
        mark(QStringLiteral("こんにちは"), QStringLiteral("xyzzyq."), QStringLiteral("ja-JP"));
        w.editor()->setDocument(std::move(doc));
        QStringList flagged;
        const QString text = sd->toPlainText();
        for (const auto &r : w.editor()->misspelledIn(t->storyId)) flagged << text.mid(r.first, r.second - r.first);
        QCOMPARE(flagged, (QStringList{QStringLiteral("Teh"), QStringLiteral("colour"), QStringLiteral("pruebba")}));
    }

    // Grammar as you type, as JeffWrite has it (the same rules and the same setting): a text box and a table cell get their issues, the setting takes them away, and the
    // renderer is told where to draw the blue dotted line.
    void grammarIssuesAreFlaggedInATextBoxAndATableCell()
    {
        const jptest::SettingGuard grammar(kGrammarOn, true, true);
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        const auto box = boxWith(w, kAnTest);
        auto table = std::make_shared<jp::TableItem>();
        table->rows = 1;
        table->cols = 1;
        table->colW = {200};
        table->rowH = {40};
        table->rect = QRectF(72, 300, 0, 0);
        table->syncRect();
        table->cells.resize(1);
        table->cells[0].storyId = ed->doc()->createStory(kAnTest);
        ed->doc()->pages[0]->items.push_back(table);
        const QString cell = table->cells[0].storyId;
        QTRY_VERIFY(!ed->grammarIssuesIn(box->storyId).isEmpty());
        QTRY_VERIFY(!ed->grammarIssuesIn(cell).isEmpty());
        const auto issue = ed->grammarIssuesIn(box->storyId).first();
        QVERIFY(issue.start >= 0 && issue.end > issue.start && issue.end <= kAnTest.size());
        QVERIFY(!issue.message.isEmpty());
        QTRY_VERIFY(ed->renderOptions().grammar.contains(box->storyId));
        QVERIFY(ed->renderOptions().grammar.contains(cell));
        QCOMPARE(ed->renderOptions().grammar.value(box->storyId).first(), qMakePair(issue.start, issue.end));
        // The setting off, and the marks are gone (the same setting JeffWrite follows); on again, and they are back.
        jp::setGrammarAsYouType(false);
        QVERIFY(ed->grammarIssuesIn(box->storyId).isEmpty());
        QVERIFY(ed->renderOptions().grammar.isEmpty());
        jp::setGrammarAsYouType(true);
        QTRY_VERIFY(!ed->grammarIssuesIn(box->storyId).isEmpty());
    }

    // Text in a language without rules is not checked, as in JeffWrite (only English, French, German, and Spanish have them).
    void grammarIsNotCheckedInALanguageWithoutRules()
    {
        const jptest::SettingGuard grammar(kGrammarOn, true, true);
        jp::MainWindow w;
        const auto french = boxWith(w, kAnTest, QStringLiteral("it-IT"));
        w.editor()->grammarIssuesIn(french->storyId);
        QVERIFY(w.editor()->grammarCheckOf(french->storyId));
        QVERIFY(w.editor()->grammarIssuesIn(french->storyId).isEmpty());   // (nothing is ever sent for it: there is nothing to wait for)
        QVERIFY(w.editor()->renderOptions().grammar.isEmpty());
        QVERIFY(!w.editor()->grammarPending());
        QCOMPARE(w.editor()->grammarParagraphsChecked(), 0);
    }

    // The right-click menu on an issue: its message, the suggestion (a click takes it, in one undo step), and Ignore This Issue.
    void aGrammarIssueOffersItsSuggestionAndIgnore()
    {
        const jptest::SettingGuard grammar(kGrammarOn, true, true);
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        const auto box = boxWith(w, kAnTest);
        QTRY_VERIFY(!ed->grammarIssuesIn(box->storyId).isEmpty());
        const auto issue = ed->grammarIssuesIn(box->storyId).first();
        ed->beginTextEdit(box->id, issue.start + 1);
        QTextDocument *story = ed->doc()->storyDoc(box->storyId);
        {
            QMenu menu;
            QVERIFY(jp::addGrammarSuggestions(menu, ed));
            QAction *message = nullptr, *accept = nullptr, *ignore = nullptr;
            for (QAction *a : menu.actions()) {
                if (a->text() == issue.message) message = a;
                if (!issue.suggestions.isEmpty() && a->text() == issue.suggestions.first()) accept = a;
                if (a->text() == QStringLiteral("Ignore This Issue")) ignore = a;
            }
            QVERIFY(message && !message->isEnabled());
            QVERIFY(accept && accept->font().bold());
            QVERIFY(ignore);
            accept->trigger();
        }
        QCOMPARE(story->toPlainText(), QStringLiteral("This is a test of the checker."));
        // Put the text back: Ignore This Issue takes the mark away, and the issue is not offered again.
        story->setPlainText(kAnTest);
        QTRY_VERIFY(!ed->grammarIssuesIn(box->storyId).isEmpty());
        ed->beginTextEdit(box->id, issue.start + 1);
        QMenu menu;
        QVERIFY(jp::addGrammarSuggestions(menu, ed));
        for (QAction *a : menu.actions())
            if (a->text() == QStringLiteral("Ignore This Issue")) a->trigger();
        QVERIFY(ed->grammarIssuesIn(box->storyId).isEmpty());
        QVERIFY(ed->renderOptions().grammar.isEmpty());
        QMenu again;
        QVERIFY(!jp::addGrammarSuggestions(again, ed));
    }

    // The grammar of a window that is closed with a backlog of paragraphs waiting is not finished first: closing is quick.
    void closingAnEditorWithAGrammarBacklogDoesNotWaitForIt()
    {
        const jptest::SettingGuard grammar(kGrammarOn, true, true);
        auto ed = std::make_unique<jp::Editor>();
        auto doc = jp::Document::blank(QSizeF(612, 792));
        for (int i = 0; i < 300; ++i) {
            auto t = std::make_shared<jp::TextItem>();
            t->rect = QRectF(10, 10 + i * 2, 300, 20);
            t->storyId = doc->createStory(QStringLiteral("This is the checker. ").repeated(3000) + QString::number(i));   // (63,000 letters: a few milliseconds each)
            doc->pages[0]->items.push_back(t);
        }
        ed->setDocument(std::move(doc));
        ed->setGrammarDelayMs(0);
        ed->renderOptions();
        QTRY_VERIFY(ed->grammarParagraphsChecked() > 20);   // (the checks are sent, and wait their turn on the one thread)
        QElapsedTimer timer;
        timer.start();
        ed.reset();
        QVERIFY2(timer.elapsed() < 1000, qPrintable(QString::number(timer.elapsed())));
    }

    // An editor that goes while checks are under way, again and again, is safe (an answer that comes late finds its check gone).
    void anEditorThatGoesWhileChecksAreUnderWayIsSafe()
    {
        const jptest::SettingGuard grammar(kGrammarOn, true, true);
        for (int round = 0; round < 15; ++round) {
            auto ed = std::make_unique<jp::Editor>();
            auto doc = jp::Document::blank(QSizeF(612, 792));
            for (int i = 0; i < 20; ++i) {
                auto t = std::make_shared<jp::TextItem>();
                t->rect = QRectF(10, 10 + i * 5, 300, 20);
                t->storyId = doc->createStory(kAnTest + QString::number(i));
                doc->pages[0]->items.push_back(t);
            }
            ed->setDocument(std::move(doc));
            ed->setGrammarDelayMs(0);
            ed->renderOptions();
            QTest::qWait(round % 4);
            ed.reset();
            QCoreApplication::processEvents();
        }
    }

    // A repaint or a caret move that changes no text gathers no issues again, and only the stories of the objects on the page are checked: 1,000 boxes, and 50 on another page.
    void repaintsOfAThousandTextBoxesGatherNothingAgain()
    {
        const jptest::SettingGuard grammar(kGrammarOn, true, true);
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        auto doc = jp::Document::blank(QSizeF(612, 792));
        QStringList ids;
        for (int i = 0; i < 1000; ++i) {
            auto t = std::make_shared<jp::TextItem>();
            t->rect = QRectF(10, 10 + (i % 700), 300, 20);
            t->storyId = doc->createStory(QStringLiteral("Box %1 is an test of the checker.").arg(i));
            doc->pages[0]->items.push_back(t);
            ids << t->id;
        }
        doc->addPage();
        for (int i = 0; i < 50; ++i) {
            auto t = std::make_shared<jp::TextItem>();
            t->rect = QRectF(10, 10 + i, 300, 20);
            t->storyId = doc->createStory(QStringLiteral("Other %1 is an test of the checker.").arg(i));
            doc->pages[1]->items.push_back(t);
        }
        ed->setDocument(std::move(doc));
        ed->setGrammarDelayMs(0);
        QTRY_VERIFY_WITH_TIMEOUT(ed->renderOptions().grammar.size() == 1000, 90000);
        QTRY_VERIFY_WITH_TIMEOUT(!ed->grammarPending(), 30000);
        QCOMPARE(ed->grammarParagraphsChecked(), 1000);   // (nothing of the other page)
        ed->renderOptions();
        const int scans = ed->grammarScans();
        for (int i = 0; i < 50; ++i) {
            ed->beginTextEdit(ids[i * 7], 1);   // (a caret move: the box being typed in is one of the stories)
            QCOMPARE(ed->renderOptions().grammar.size(), 1000);
        }
        QCOMPARE(ed->grammarScans(), scans);
    }

    // A box taken away and another put in its place: the check of the old story goes, by name and not by count.
    void theCheckOfAStoryThatIsGoneIsLetGo()
    {
        const jptest::SettingGuard grammar(kGrammarOn, true, true);
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        auto doc = jp::Document::blank(QSizeF(612, 792));
        QStringList stories;
        for (int i = 0; i < 3; ++i) {
            auto t = std::make_shared<jp::TextItem>();
            t->rect = QRectF(10, 10 + i * 30, 300, 20);
            t->storyId = doc->createStory(kAnTest + QString::number(i));
            doc->pages[0]->items.push_back(t);
            stories << t->storyId;
        }
        ed->setDocument(std::move(doc));
        ed->setGrammarDelayMs(0);
        ed->renderOptions();
        QCOMPARE(ed->grammarCheckCount(), 3);
        ed->doc()->pages[0]->items.pop_back();
        ed->doc()->stories.remove(stories.last());
        auto t = std::make_shared<jp::TextItem>();
        t->rect = QRectF(10, 200, 300, 20);
        t->storyId = ed->doc()->createStory(kAnTest + QStringLiteral("new"));
        ed->doc()->pages[0]->items.push_back(t);
        ed->renderOptions();
        QCOMPARE(ed->grammarCheckCount(), 3);
    }

    // A new document starts with nothing of the old one's checking: what was ignored in it is marked again.
    void aNewDocumentForgetsWhatWasIgnored()
    {
        const jptest::SettingGuard grammar(kGrammarOn, true, true);
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        auto box = boxWith(w, kAnTest);
        QTRY_VERIFY(!ed->grammarIssuesIn(box->storyId).isEmpty());
        ed->ignoreGrammarAll(ed->grammarIssuesIn(box->storyId).first(), box->storyId);
        QVERIFY(ed->grammarIssuesIn(box->storyId).isEmpty());
        box = boxWith(w, kAnTest);
        QTRY_VERIFY(!ed->grammarIssuesIn(box->storyId).isEmpty());
    }

    // Ignore All holds for the stories made after it, too.
    void ignoreAllHoldsForStoriesMadeLater()
    {
        const jptest::SettingGuard grammar(kGrammarOn, true, true);
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        const auto first = boxWith(w, kAnTest);
        QTRY_VERIFY(!ed->grammarIssuesIn(first->storyId).isEmpty());
        ed->ignoreGrammarAll(ed->grammarIssuesIn(first->storyId).first(), first->storyId);
        auto later = std::make_shared<jp::TextItem>();
        later->rect = QRectF(72, 300, 300, 100);
        later->storyId = ed->doc()->createStory(kAnTest);
        ed->doc()->pages[0]->items.push_back(later);
        // Both are asked for and settle; the second has an issue that is not ignored, the first has the one that is (found, and not shown).
        auto other = std::make_shared<jp::TextItem>();
        other->rect = QRectF(72, 450, 300, 100);
        other->storyId = ed->doc()->createStory(QStringLiteral("Then it is the the end."));
        ed->doc()->pages[0]->items.push_back(other);
        QTRY_VERIFY((ed->renderOptions(), !ed->grammarPending() && ed->grammarCheckOf(later->storyId)->paragraphsChecked() > 0 && !ed->grammarIssuesIn(other->storyId).isEmpty()));
        QVERIFY(ed->grammarIssuesIn(later->storyId).isEmpty());
    }

    // The words of another language in a paragraph are never changed by what a grammar suggestion does to the English around them.
    void aSuggestionNeverTouchesWordsMarkedAnotherLanguage()
    {
        const jptest::SettingGuard grammar(kGrammarOn, true, true);
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        const QString text = QStringLiteral("This is an test. Das ist  gut alot. Und the the end.");
        const auto box = boxWith(w, text);
        QTextDocument *sd = ed->doc()->storyDoc(box->storyId);
        QTextCursor c(sd);
        c.setPosition(int(text.indexOf(QLatin1String("Das"))));
        c.setPosition(int(text.indexOf(QLatin1String("gut")) + 3), QTextCursor::KeepAnchor);
        QTextCharFormat f;
        f.setProperty(jp::tp::Language, QStringLiteral("de-DE"));
        c.mergeCharFormat(f);
        QTRY_VERIFY(!ed->grammarIssuesIn(box->storyId).isEmpty());
        // Every issue, last first so that the places hold: its first suggestion, taken.
        auto issues = ed->grammarIssuesIn(box->storyId);
        QVERIFY(issues.size() >= 2);
        for (int i = int(issues.size()) - 1; i >= 0; --i) {
            QVERIFY(issues[i].end <= text.indexOf(QLatin1String("Das")) || issues[i].start >= text.indexOf(QLatin1String("gut")) + 3);   // (none on the German words)
            for (const QString &s : issues[i].suggestions) QVERIFY(!s.contains(QChar(0xFFFC)));
            if (issues[i].suggestions.isEmpty()) continue;
            QTextCursor e(sd);
            e.setPosition(issues[i].start);
            e.setPosition(issues[i].end, QTextCursor::KeepAnchor);
            e.insertText(issues[i].suggestions.first());
        }
        QVERIFY(sd->toPlainText().contains(QStringLiteral("Das ist  gut")));
        QVERIFY(!sd->toPlainText().contains(QChar(0xFFFC)));
    }

    // The spelling check (F7) stops at a grammar issue when the setting is on, with the issue's message; with it off, the check is complete.
    void theSpellingCheckStopsAtAGrammarIssue()
    {
        const jptest::SettingGuard grammar(kGrammarOn, true, true);
        jp::MainWindow w;
        boxWith(w, kAnTest);
        const QString message = jp::grammarCheck(kAnTest).first().message;
        QString shown;
        withDialog([&] { jp::spellingDialog(&w, w.editor()); }, QStringLiteral("pageSpellingDialog"), [&](QDialog *d) {
            shown = d->findChild<QLabel *>(QStringLiteral("spellingNotIn"))->text();
            QVERIFY(shown.contains(message.toHtmlEscaped()));
            QVERIFY(d->findChild<QPushButton *>(QStringLiteral("spellingIgnore")));
        }, false);
        QVERIFY2(shown.contains(message.toHtmlEscaped()), qPrintable(shown));
    }

    // A publication of 200 text boxes: typing in one checks that paragraph and no other (the others are kept by their text), and the repaint after it asks for nothing more.
    void typingInOneOf200TextBoxesDoesNotCheckTheOthers()
    {
        const jptest::SettingGuard grammar(kGrammarOn, true, true);
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        auto doc = jp::Document::blank(QSizeF(612, 792));
        QStringList stories;
        for (int i = 0; i < 200; ++i) {
            auto t = std::make_shared<jp::TextItem>();
            t->rect = QRectF(10, 10 + i * 3, 300, 20);
            t->storyId = doc->createStory(QStringLiteral("Box %1 is an test of the checker.").arg(i));
            doc->pages[0]->items.push_back(t);
            stories << t->storyId;
        }
        ed->setDocument(std::move(doc));
        ed->setGrammarDelayMs(0);
        auto allFlagged = [&] {
            const auto grammar = ed->renderOptions().grammar;   // (what a repaint asks for)
            return grammar.size() == 200;
        };
        QTRY_VERIFY_WITH_TIMEOUT(allFlagged(), 60000);
        QTRY_VERIFY_WITH_TIMEOUT(!ed->grammarPending(), 30000);
        const int before = ed->grammarParagraphsChecked();
        QCOMPARE(before, 200);
        // Typing: one more paragraph is checked.
        QTextDocument *sd = ed->doc()->storyDoc(stories[57]);
        QTextCursor c(sd);
        c.movePosition(QTextCursor::End);
        c.insertText(QStringLiteral(" Then it ends."));
        QTRY_VERIFY_WITH_TIMEOUT((ed->renderOptions(), ed->grammarParagraphsChecked() > before && !ed->grammarPending()), 30000);   // (the repaint a keystroke causes)
        QVERIFY(allFlagged());
        QCOMPARE(ed->grammarParagraphsChecked(), before + 1);
        // A repaint with nothing changed asks for nothing.
        for (int i = 0; i < 5; ++i) ed->renderOptions();
        QVERIFY(!ed->grammarPending());
        QCOMPARE(ed->grammarParagraphsChecked(), before + 1);
    }

    // Check spelling as you type: a misspelled word gets a range, a correct
    // one doesn't, and suggestions include the fix.
    void spellingAsYouType()
    {
        jp::MainWindow w;
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto t = std::make_shared<jp::TextItem>();
        t->rect = QRectF(72, 72, 300, 100);
        t->storyId = doc->createStory(QStringLiteral("This is teh test."));
        doc->pages[0]->items.push_back(t);
        w.editor()->setDocument(std::move(doc));
        const auto ranges = w.editor()->misspelledIn(t->storyId);
        QCOMPARE(ranges.size(), 1);
        QCOMPARE(ranges.first(), qMakePair(8, 11));
        QVERIFY(jp::spellingSuggestions(QStringLiteral("teh")).contains(QStringLiteral("the")));
        QVERIFY(w.editor()->renderOptions().misspelled.contains(t->storyId));
    }

    // Check spelling as you type is the suite's one choice: Options has the box, turning it off there takes the red lines from every open window and is remembered by the next.
    void spellingAsYouTypeIsOneChoiceInOptions()
    {
        const jptest::SettingGuard keep(QStringLiteral("proof/checkAsYouType"), true, true);
        auto addText = [](jp::MainWindow &w) {
            auto doc = jp::Document::blank(QSizeF(612, 792));
            auto t = std::make_shared<jp::TextItem>();
            t->rect = QRectF(72, 72, 300, 100);
            t->storyId = doc->createStory(QStringLiteral("This is teh test."));
            doc->pages[0]->items.push_back(t);
            w.editor()->setDocument(std::move(doc));
            return t->storyId;
        };
        jp::MainWindow first, second;
        const QString a = addText(first), b = addText(second);
        QVERIFY(first.editor()->renderOptions().misspelled.contains(a));
        QVERIFY(second.editor()->renderOptions().misspelled.contains(b));
        // Options: the box is found (by its words), looked at, set, and the dialog accepted; the timer keeps looking until the dialog is up.
        QString boxText;
        bool boxWas = false;
        QTimer watch;
        watch.setInterval(20);
        QObject::connect(&watch, &QTimer::timeout, &watch, [&] {
            auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!d) return;
            watch.stop();
            for (QCheckBox *box : d->findChildren<QCheckBox *>())
                if (box->text().contains(QLatin1String("spelling as you type"), Qt::CaseInsensitive)) {
                    boxText = box->text();
                    boxWas = box->isChecked();
                    box->setChecked(false);
                }
            d->accept();
        });
        watch.start();
        first.act(QStringLiteral("file.options"))->trigger();
        QCOMPARE(boxText, QStringLiteral("Check spelling as you type"));
        QVERIFY(boxWas);
        QVERIFY(!first.editor()->renderOptions().misspelled.contains(a));
        QVERIFY(!second.editor()->renderOptions().misspelled.contains(b));   // (the other window, at once)
        QTRY_VERIFY(!second.act(QStringLiteral("rev.checkAsType"))->isChecked());
        QVERIFY(!jp::Settings::get().value(QStringLiteral("proof/checkAsYouType"), true).toBool());
        jp::MainWindow later;
        QVERIFY(!later.editor()->view.spelling);   // (remembered)
    }

    // The Find and Replace pane: Find Next walks the pages and shows each match
    // typed-in and selected, Replace swaps the one shown, and Replace All is one
    // undo step that says how many it made.
    void findPaneFindsAcrossPagesAndReplaces()
    {
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        auto doc = jp::Document::blank(QSizeF(612, 792));
        doc->addPage();
        QStringList stories;
        for (int p = 0; p < 2; ++p) {
            auto t = std::make_shared<jp::TextItem>();
            t->rect = QRectF(72, 72, 300, 100);
            t->storyId = doc->createStory(p == 0 ? QStringLiteral("One Cat and a cat.") : QStringLiteral("Another cat, and a catalog."));
            doc->pages[p]->items.push_back(t);
            stories << t->storyId;
        }
        ed->setDocument(std::move(doc));
        w.showTaskPane(QStringLiteral("find"));
        auto *tp = w.findChild<jp::TaskPane *>();
        QVERIFY(tp);
        auto edit = [&](const QString &name) -> QLineEdit * {
            for (QLineEdit *e : tp->findChildren<QLineEdit *>())
                if (name.isEmpty() ? e->objectName() != QLatin1String("replace") : e->objectName() == name) return e;
            return nullptr;
        };
        auto button = [&](const QString &text) -> QPushButton * {
            for (QPushButton *b : tp->findChildren<QPushButton *>())
                if (b->text() == text) return b;
            return nullptr;
        };
        auto check = [&](const QString &text) -> QCheckBox * {
            for (QCheckBox *c : tp->findChildren<QCheckBox *>())
                if (c->text() == text) return c;
            return nullptr;
        };
        QVERIFY(edit(QString()) && edit(QStringLiteral("replace")) && button(QStringLiteral("Find Next")) && check(QStringLiteral("Match case")));
        auto status = [&] {
            QString s;
            for (QLabel *l : tp->findChildren<QLabel *>())
                if (l->wordWrap()) s = l->text();
            return s;
        };
        edit(QString())->setText(QStringLiteral("cat"));
        // Case is ignored unless asked; the first match is the capital one, then each after it, then round to the first.
        QStringList seen;
        for (int i = 0; i < 6; ++i) {
            button(QStringLiteral("Find Next"))->click();
            QVERIFY(ed->isEditingText());
            seen << QStringLiteral("%1:%2@%3").arg(ed->currentPage()).arg(ed->cursor().selectedText()).arg(ed->cursor().selectionStart());
        }
        QCOMPARE(seen, (QStringList{QStringLiteral("0:Cat@4"), QStringLiteral("0:cat@14"), QStringLiteral("1:cat@8"), QStringLiteral("1:cat@19"),
                                    QStringLiteral("0:Cat@4"), QStringLiteral("0:cat@14")}));
        check(QStringLiteral("Match case"))->setChecked(true);
        edit(QString())->setText(QStringLiteral("Cat"));
        ed->setCurrentPage(1);
        button(QStringLiteral("Find Next"))->click();
        QCOMPARE(ed->currentPage(), 0);
        QCOMPARE(ed->cursor().selectionStart(), 4);
        check(QStringLiteral("Match case"))->setChecked(false);
        check(QStringLiteral("Find whole words only"))->setChecked(true);
        edit(QString())->setText(QStringLiteral("cat"));
        edit(QStringLiteral("replace"))->setText(QStringLiteral("dog"));
        button(QStringLiteral("Replace All"))->click();
        QCOMPARE(ed->doc()->storyDoc(stories[0])->toPlainText(), QStringLiteral("One dog and a dog."));
        QCOMPARE(ed->doc()->storyDoc(stories[1])->toPlainText(), QStringLiteral("Another dog, and a catalog."));
        QVERIFY2(status().contains(QLatin1String("3 replacement")), qPrintable(status()));
        ed->undo();
        QCOMPARE(ed->doc()->storyDoc(stories[0])->toPlainText(), QStringLiteral("One Cat and a cat."));
        QCOMPARE(ed->doc()->storyDoc(stories[1])->toPlainText(), QStringLiteral("Another cat, and a catalog."));
        // Replace swaps the match that is shown, then moves to the next.
        edit(QString())->setText(QStringLiteral("a cat"));
        edit(QStringLiteral("replace"))->setText(QStringLiteral("the cat"));
        check(QStringLiteral("Find whole words only"))->setChecked(false);
        button(QStringLiteral("Find Next"))->click();
        button(QStringLiteral("Replace"))->click();
        QCOMPARE(ed->doc()->storyDoc(stories[0])->toPlainText(), QStringLiteral("One Cat and the cat."));
        QCOMPARE(ed->currentPage(), 1);
        edit(QString())->setText(QStringLiteral("zzz"));
        button(QStringLiteral("Find Next"))->click();
        QVERIFY2(status().contains(QLatin1String("\"zzz\" was not found")), qPrintable(status()));
    }

    // The shared search: stories in the order to search them (the pages from the one asked for round to the
    // ones before it, a page's speaker notes after its own, the master pages last), the next match round the
    // end, backward too, and Replace All's count.
    void findEngineWalksStoriesInOrder()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        doc->addPage();
        doc->addPage();
        QStringList boxes, notes;
        for (int p = 0; p < 3; ++p) {
            auto t = std::make_shared<jp::TextItem>();
            t->rect = QRectF(72, 72, 300, 100);
            t->storyId = doc->createStory(QStringLiteral("Box %1 has red and Red.").arg(p));
            doc->pages[p]->items.push_back(t);
            boxes << t->storyId;
            if (p == 1) {
                auto tb = std::make_shared<jp::TableItem>();
                tb->rows = 1;
                tb->cols = 2;
                tb->rect = QRectF(72, 300, 300, 40);
                tb->cells.resize(2);
                tb->cells[0].storyId = doc->createStory(QStringLiteral("cell one red"));
                tb->cells[1].storyId = doc->createStory(QStringLiteral("cell two"));
                doc->pages[p]->items.push_back(tb);
                boxes << tb->cells[0].storyId << tb->cells[1].storyId;
            }
            if (p > 0) {
                doc->pages[p]->notesStoryId = doc->createStory(QStringLiteral("notes %1: red").arg(p));
                notes << doc->pages[p]->notesStoryId;
            }
        }
        auto ids = [](const QVector<jp::FindStory> &v) {
            QStringList out;
            for (const auto &s : v) out << s.storyId;
            return out;
        };
        QCOMPARE(ids(jp::findStories(*doc)), (QStringList{boxes[0], boxes[1], boxes[2], boxes[3], boxes[4]}));
        const auto from1 = jp::findStories(*doc, jp::FindScope{false, true, 1});
        QCOMPARE(ids(from1), (QStringList{boxes[1], boxes[2], boxes[3], notes[0], boxes[4], notes[1], boxes[0]}));
        QVERIFY(from1[3].notes && from1[3].page == 1 && from1[3].itemId.isEmpty());
        QVERIFY(!from1[0].notes && from1[1].row == 0 && from1[2].col == 1 && from1[1].itemId == from1[2].itemId);
        const auto plain = jp::findStories(*doc);
        const QTextDocument::FindFlags none;
        int story = -1;
        QTextCursor found;
        // Forward from the end of the first box, then from the end of the last: round to the start.
        QVERIFY(jp::findInStories(*doc, plain, QStringLiteral("red"), none, 0, 11, &story, &found));
        QCOMPARE(story, 0);
        QCOMPARE(found.selectedText(), QStringLiteral("Red"));
        QVERIFY(jp::findInStories(*doc, plain, QStringLiteral("red"), none, 0, 22, &story, &found));
        QCOMPARE(story, 1);
        QVERIFY(jp::findInStories(*doc, plain, QStringLiteral("red"), none, 4, 22, &story, &found));
        QCOMPARE(story, 0);
        QCOMPARE(found.selectionStart(), 10);
        QVERIFY(jp::findInStories(*doc, plain, QStringLiteral("red"), QTextDocument::FindBackward, 1, 0, &story, &found));
        QCOMPARE(story, 0);
        QCOMPARE(found.selectionStart(), 18);
        QVERIFY(!jp::findInStories(*doc, plain, QStringLiteral("blue"), none, 0, 0, &story, &found));
        QVERIFY(jp::findInStories(*doc, plain, QStringLiteral("red"), QTextDocument::FindCaseSensitively, 0, 0, &story, &found));
        QCOMPARE(found.selectionStart(), 10);
        QVERIFY(jp::findInStories(*doc, plain, QStringLiteral("Box 1 has red"), QTextDocument::FindWholeWords, 0, 0, &story, &found));
        QCOMPARE(story, 1);
        QVERIFY(!jp::findInStories(*doc, plain, QStringLiteral("has re"), QTextDocument::FindWholeWords, 0, 0, &story, &found));
        // Replace All counts every match (case ignored unless asked) in the stories it is given.
        QCOMPARE(jp::replaceInStories(*doc, from1, QStringLiteral("red"), none, QStringLiteral("blue")), 2 + 1 + 0 + 1 + 2 + 1 + 2);
        QCOMPARE(doc->storyDoc(boxes[0])->toPlainText(), QStringLiteral("Box 0 has blue and blue."));
        QCOMPARE(doc->storyDoc(notes[1])->toPlainText(), QStringLiteral("notes 2: blue"));
        QCOMPARE(jp::replaceInStories(*doc, plain, QStringLiteral("red"), none, QStringLiteral("x")), 0);
    }

    // The spelling help for the right-click menu: nothing and false off a misspelled word.
    void spellingMenuHelpNeedsAMisspelledWord()
    {
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto t = std::make_shared<jp::TextItem>();
        t->rect = QRectF(72, 72, 300, 100);
        t->storyId = doc->createStory(QStringLiteral("This is teh test."));
        doc->pages[0]->items.push_back(t);
        ed->setDocument(std::move(doc));
        ed->beginTextEdit(t->id, 2);
        QMenu none;
        QVERIFY(!jp::addSpellingSuggestions(none, ed));
        QVERIFY(none.isEmpty());
        ed->beginTextEdit(t->id, 10);
        QMenu menu;
        QVERIFY(jp::addSpellingSuggestions(menu, ed));
        QVERIFY(menu.actions().size() > 3);
        QVERIFY(menu.actions().last()->isSeparator());
        // The first suggestion replaces the word, as one change.
        QAction *first = menu.actions().first();
        QVERIFY(jp::spellingSuggestions(QStringLiteral("teh")).contains(first->text()));
        first->trigger();
        QVERIFY(!w.editor()->doc()->storyDoc(t->storyId)->toPlainText().contains(QLatin1String("teh")));
    }

    // A linked chain is searched once, at its first frame, wherever its frames sit in the page's list.
    void findStoriesUseTheFirstFrameOfAChain()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto a = std::make_shared<jp::TextItem>(), b = std::make_shared<jp::TextItem>(), c = std::make_shared<jp::TextItem>();
        a->rect = QRectF(72, 72, 200, 100);
        b->rect = QRectF(300, 72, 200, 100);
        c->rect = QRectF(72, 300, 200, 100);
        a->storyId = b->storyId = doc->createStory(QStringLiteral("one story in two frames"));
        c->storyId = doc->createStory(QStringLiteral("alone"));
        a->nextId = b->id;
        doc->pages[0]->items.push_back(c);
        doc->pages[0]->items.push_back(b);
        doc->pages[0]->items.push_back(a);
        const auto stories = jp::findStories(*doc);
        QCOMPARE(stories.size(), 2);
        QCOMPARE(stories[0].itemId, c->id);
        QCOMPARE(stories[1].itemId, a->id);
        QCOMPARE(stories[1].storyId, a->storyId);
    }

    // The walk over the stories takes about as long again for each slide: four times the slides take about four times as long,
    // not sixteen (every text box once asked the whole document for the frame before it).
    void findStoriesTakesTimeInProportionToThePages()
    {
        auto documentOf = [](int pages) {
            auto doc = jp::Document::blank(QSizeF(960, 540));
            for (int p = 0; p < pages; ++p) {
                if (p) doc->addPage();
                for (int k = 0; k < 2; ++k) {
                    auto t = std::make_shared<jp::TextItem>();
                    t->rect = QRectF(72, 72 + k * 200, 300, 100);
                    t->storyId = doc->createStory(QStringLiteral("text %1").arg(p));
                    doc->pages[p]->items.push_back(t);
                }
            }
            return doc;
        };
        const auto smallDoc = documentOf(500), bigDoc = documentOf(8000);
        bool found = true;
        auto workOf = [&](const jp::Document &doc, int pages) {
            const qint64 before = jp::Document::itemsWalkedForTest();
            const auto stories = jp::findStories(doc);
            found = found && stories.size() == pages * 2;
            return jp::Document::itemsWalkedForTest() - before;
        };
        // Counted: the items the document handed out to find the stories. Sixteen times the pages: linear work is sixteen times, the old quadratic walk (the whole
        // document for each text box) 256 times, and the walk of one pass is one item for each item there is.
        const qint64 small = workOf(*smallDoc, 500), big = workOf(*bigDoc, 8000);
        QVERIFY(found);
        QVERIFY(small > 0 && big > 0);
        QVERIFY2(small <= 2 * 500 * 2 && big <= 2 * 8000 * 2 && big < small * 20, qPrintable(QStringLiteral("500 pages: %1 items, 8000 pages: %2 items").arg(small).arg(big)));
    }

    // A Text Art's words are searched when the scope says so (not for JeffPub, whose pane does not follow a match into one): found,
    // replaced as a whole pass, and put back in the Text Art; a single match replaced where the search found it.
    void findEngineSearchesTextArtWhenAsked()
    {
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto t = std::make_shared<jp::TextItem>();
        t->rect = QRectF(72, 72, 300, 100);
        t->storyId = doc->createStory(QStringLiteral("a red box"));
        auto art = std::make_shared<jp::TextArtItem>();
        art->rect = QRectF(72, 300, 300, 100);
        art->text = QStringLiteral("Red and\nred again");
        auto empty = std::make_shared<jp::TextArtItem>();
        empty->rect = QRectF(72, 500, 300, 100);
        empty->text.clear();
        doc->pages[0]->items.push_back(art);
        doc->pages[0]->items.push_back(t);
        doc->pages[0]->items.push_back(empty);
        QCOMPARE(jp::findStories(*doc).size(), 1);
        jp::FindScope scope;
        scope.textArt = true;
        const auto stories = jp::findStories(*doc, scope);
        QCOMPARE(stories.size(), 2);   // not the empty one
        QVERIFY(stories[0].art && stories[0].itemId == art->id && stories[0].storyId.isEmpty());
        QVERIFY(!stories[1].art);
        int story = -1;
        QTextCursor found;
        const QTextDocument::FindFlags none;
        QVERIFY(jp::findInStories(*doc, stories, QStringLiteral("red"), none, 0, 4, &story, &found));
        QCOMPARE(story, 0);
        QCOMPARE(found.selectionStart(), 8);   // after the line break, which counts as one
        QVERIFY(!jp::replaceArtWords(*doc, art->id, 8, 11, QStringLiteral("blue"), Qt::CaseInsensitive, QStringLiteral("x")));
        QVERIFY(jp::replaceArtWords(*doc, art->id, 8, 11, QStringLiteral("RED"), Qt::CaseInsensitive, QStringLiteral("blue")));
        QCOMPARE(art->text, QStringLiteral("Red and\nblue again"));
        QVector<jp::FindStory> changed;
        // (The Text Art's words in a story are a copy made when the stories were listed: list them again.)
        QCOMPARE(jp::replaceInStories(*doc, jp::findStories(*doc, scope), QStringLiteral("red"), none, QStringLiteral("green"), &changed), 2);
        QCOMPARE(art->text, QStringLiteral("green and\nblue again"));
        QCOMPARE(doc->storyDoc(t->storyId)->toPlainText(), QStringLiteral("a green box"));
        QCOMPARE(changed.size(), 2);
    }

    // What a message says of a search text: short, and breakable where it has no spaces.
    void findTextForMessageIsShortAndBreakable()
    {
        QCOMPARE(jp::findTextForMessage(QStringLiteral("two short words")), QStringLiteral("two short words"));
        QCOMPARE(jp::findTextForMessage(QString()), QString());
        const QString cut = jp::findTextForMessage(QString(100, QLatin1Char('x')));
        QString plain = cut;
        plain.remove(QChar(0x200B));
        QCOMPARE(plain, QString(40, QLatin1Char('x')) + QChar(0x2026));
        QVERIFY(cut.size() > plain.size());
        for (const QString &run : cut.split(QChar(0x200B))) QVERIFY(run.size() <= 13);
        // A pair of UTF-16 units is not cut in two.
        const QString emoji = QString(39, QLatin1Char('x')) + QStringLiteral("\U0001F600\U0001F600");
        const QString shown = jp::findTextForMessage(emoji);
        QVERIFY(!shown.contains(QChar::ReplacementCharacter));
        QVERIFY(shown.endsWith(QChar(0x2026)));
        QVERIFY(!QChar::isHighSurrogate(shown.at(shown.size() - 2).unicode()));
    }

    // Replace All fits a box that grows to fit its words and a table's rows to the new words, as typing them would, in the same
    // step (so one Undo gives back the sizes too).
    void findPaneReplaceAllRefitsBoxesAndTables()
    {
        jp::MainWindow w;
        jp::Editor *ed = w.editor();
        auto doc = jp::Document::blank(QSizeF(612, 792));
        ed->setDocument(std::move(doc));
        auto box = std::static_pointer_cast<jp::TextItem>(ed->newTextBox(QRectF(72, 72, 200, 30), QStringLiteral("word")));
        box->autofit = jp::TextItem::GrowBox;
        ed->addItem(box);
        auto table = std::static_pointer_cast<jp::TableItem>(ed->newTable(QRectF(72, 300, 300, 40), 1, 2));
        ed->addItem(table);
        QTextCursor(ed->doc()->storyDoc(table->cell(0, 0).storyId)).insertText(QStringLiteral("word"));
        const QString boxId = box->id, tableId = table->id;
        const double boxHeight = box->rect.height(), rowHeight = table->rowH[0];
        w.showTaskPane(QStringLiteral("find"));
        auto *tp = w.findChild<jp::TaskPane *>();
        QVERIFY(tp);
        QLineEdit *find = nullptr, *replace = nullptr;
        for (QLineEdit *e : tp->findChildren<QLineEdit *>()) (e->objectName() == QLatin1String("replace") ? replace : find) = e;
        QVERIFY(find && replace);
        find->setText(QStringLiteral("word"));
        QString longer;
        for (int i = 0; i < 40; ++i) longer += QStringLiteral("longer words ");
        replace->setText(longer);
        for (QPushButton *b : tp->findChildren<QPushButton *>())
            if (b->text() == QLatin1String("Replace All")) b->click();
        auto *grown = static_cast<jp::TextItem *>(ed->doc()->item(boxId));
        auto *fitted = static_cast<jp::TableItem *>(ed->doc()->item(tableId));
        QVERIFY2(grown->rect.height() > boxHeight + 50, qPrintable(QString::number(grown->rect.height())));
        QVERIFY2(fitted->rowH[0] > rowHeight + 50, qPrintable(QString::number(fitted->rowH[0])));
        // Already fitted: fitting again changes nothing.
        const double h = grown->rect.height(), r = fitted->rowH[0];
        ed->autoGrowText(grown);
        ed->fitTableRows(fitted);
        QCOMPARE(grown->rect.height(), h);
        QCOMPARE(fitted->rowH[0], r);
        ed->undo();
        QCOMPARE(static_cast<jp::TextItem *>(ed->doc()->item(boxId))->rect.height(), boxHeight);
        QCOMPARE(static_cast<jp::TableItem *>(ed->doc()->item(tableId))->rowH[0], rowHeight);
    }

    // The pane's message is plain text, and a long search text can't stretch the pane: it is cut short and may break anywhere.
    void findPaneMessageIsPlainTextAndNarrow()
    {
        jp::MainWindow w;
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto t = std::make_shared<jp::TextItem>();
        t->rect = QRectF(72, 72, 300, 100);
        t->storyId = doc->createStory(QStringLiteral("some words"));
        doc->pages[0]->items.push_back(t);
        w.editor()->setDocument(std::move(doc));
        w.showTaskPane(QStringLiteral("find"));
        auto *tp = w.findChild<jp::TaskPane *>();
        QVERIFY(tp);
        QLineEdit *find = nullptr;
        for (QLineEdit *e : tp->findChildren<QLineEdit *>())
            if (e->objectName() != QLatin1String("replace")) find = e;
        QLabel *status = nullptr;
        for (QLabel *l : tp->findChildren<QLabel *>())
            if (l->wordWrap()) status = l;
        QPushButton *next = nullptr;
        for (QPushButton *b : tp->findChildren<QPushButton *>())
            if (b->text() == QLatin1String("Find Next")) next = b;
        QVERIFY(find && status && next);
        find->setText(QStringLiteral("<b>x"));
        next->click();
        QCOMPARE(status->textFormat(), Qt::PlainText);
        QVERIFY2(status->text().contains(QStringLiteral("\"<b>x\" was not found")), qPrintable(status->text()));
        find->setText(QString(120, QLatin1Char('x')));
        next->click();
        QVERIFY2(status->text().size() < 200, qPrintable(QString::number(status->text().size())));
        QVERIFY2(status->minimumSizeHint().width() < 300, qPrintable(QString::number(status->minimumSizeHint().width())));
    }

    // Right-clicking a misspelled word in the text being typed offers the dictionary's
    // suggestions first, in bold, then Ignore All and Add to Dictionary.
    void contextMenuOffersSpellingSuggestions()
    {
        jp::MainWindow w;
        w.show();
        jp::Editor *ed = w.editor();
        auto doc = jp::Document::blank(QSizeF(612, 792));
        auto t = std::make_shared<jp::TextItem>();
        t->rect = QRectF(72, 72, 300, 100);
        t->storyId = doc->createStory(QStringLiteral("This is teh test."));
        doc->pages[0]->items.push_back(t);
        ed->setDocument(std::move(doc));
        ed->beginTextEdit(t->id, 9);
        QStringList texts;
        QList<bool> bold;
        QTimer::singleShot(100, [&] {
            if (auto *m = qobject_cast<QMenu *>(QApplication::activePopupWidget())) {
                for (QAction *a : m->actions()) {
                    texts << a->text();
                    bold << a->font().bold();
                }
                m->close();
            }
        });
        Q_EMIT w.canvas()->contextMenuWanted(w.mapToGlobal(QPoint(300, 300)));
        QVERIFY(texts.size() > 6);
        QVERIFY2(texts.first().compare(QLatin1String("the"), Qt::CaseInsensitive) == 0 || jp::spellingSuggestions(QStringLiteral("teh")).contains(texts.first()), qPrintable(texts.join(QLatin1Char('|'))));
        QVERIFY(bold.first());
        const int ignore = int(texts.indexOf(QStringLiteral("Ignore All")));
        QVERIFY(ignore > 0);
        QCOMPARE(texts[ignore + 1], QStringLiteral("Add to Dictionary"));
        QVERIFY(!bold[ignore]);
        QVERIFY(jp::spellingSuggestions(QStringLiteral("teh")).mid(0, 6).contains(texts.first()));
        QCOMPARE(ignore, int(qMin<qsizetype>(6, jp::spellingSuggestions(QStringLiteral("teh")).size())));
    }

    // The thesaurus finds synonyms, including for inflected words.
    void thesaurusFinds()
    {
        for (const char *w : {"happy", "Happy", "running", "houses", "quickly", "bigger"})
            QVERIFY2(!jp::thesaurusLookup(QString::fromLatin1(w)).isEmpty(), w);
        QVERIFY(jp::thesaurusLookup("zzxqv").isEmpty());
    }
};

JP_TEST_CLASS(PubFindTests)
#include "tst_pub_find.moc"
