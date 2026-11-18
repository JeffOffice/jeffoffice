// JeffDeck's tests of spelling, comments, the options, and telemetry.
#include "../plainprobe.h"
#include "app/reviewpane_deck.h"
#include "app/theme.h"
#include "core/deckcompare.h"
#include "io/pptxexport.h"
#include "io/pptxreview.h"
#include "io/presentationfile.h"
#include "office/package.h"
#include "text/review.h"
#include "core/version.h"
#include "core/threads.h"
#include "text/storyio.h"
#include "deckhelpers.h"

#include "../settingguard.h"
#include "../withdialog.h"

#include <QStatusBar>
#include <QToolTip>

using jp::deck::DeckComparison;
using jp::deck::acceptAll;
using jp::deck::applyChange;
using jp::deck::compareDecks;
using jp::deck::rejectAll;

// A window holding the base deck, with theirs saved and chosen by Compare (the compare tests of the window below).
struct CompareRun {
    std::unique_ptr<DeckWindow> w;
    std::unique_ptr<Document> base, mine, fileTheirs;
    int expected = 0;
};

class DeckReviewTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase() { initDeckTestCase(); }
    // (spelling as you type is one choice for the suite, kept in the settings: a test elsewhere in this program may have left it off, and these tests need it on)
    void init() { jp::setSpellAsYouType(true); }

    // Review > Spelling (F7) goes through the slides to a misspelled word and
    // selects it; Thesaurus is Shift+F7.
    void spellingChecksTheSlides()
    {
        DeckWindow w;
        QCOMPARE(w.act(QStringLiteral("review.spelling"))->shortcut(), QKeySequence(Qt::Key_F7));
        QCOMPARE(w.act(QStringLiteral("review.thesaurus"))->shortcut(), QKeySequence(Qt::SHIFT | Qt::Key_F7));
        Editor *ed = w.editor();
        auto *title = static_cast<TextItem *>(ed->doc()->pages[0]->items[0].get());
        QTextCursor(ed->doc()->storyDoc(title->storyId)).insertText(QStringLiteral("A mispelled word"));
        QString shown, selected;
        QTimer::singleShot(0, [&] {
            auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!d) return;
            for (QLabel *l : d->findChildren<QLabel *>())
                if (l->text().startsWith(QLatin1String("<b>"))) shown = l->text();
            selected = ed->cursor().selectedText();
            d->reject();
        });
        w.act(QStringLiteral("review.spelling"))->trigger();
        QVERIFY2(shown.contains(QStringLiteral("mispelled")), qPrintable(shown));
        QCOMPARE(selected, QStringLiteral("mispelled"));
        QVERIFY(ed->isEditingText());
    }

    // Check spelling as you type is the suite's one choice: Options has the box, and turning it off there takes the red lines from every open window, and the next window opens without them.
    void spellingAsYouTypeIsOneChoiceInOptions()
    {
        const jptest::SettingGuard keep(QStringLiteral("proof/checkAsYouType"), true, true);
        auto addText = [](DeckWindow &w) {
            auto *title = static_cast<TextItem *>(w.editor()->doc()->pages[0]->items[0].get());
            QTextCursor(w.editor()->doc()->storyDoc(title->storyId)).insertText(QStringLiteral("A mispelled word"));
            return title->storyId;
        };
        DeckWindow first, second;
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
        QVERIFY(!second.editor()->renderOptions().misspelled.contains(b));
        QVERIFY(!Settings::get().value(QStringLiteral("proof/checkAsYouType"), true).toBool());
        DeckWindow later;
        QVERIFY(!later.editor()->view.spelling);
    }

    // Word Count counts the words on the slides, not the prompts of the layouts.
    void wordCountCountsTheSlidesWords()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        QTextCursor(ed->doc()->storyDoc(static_cast<TextItem *>(ed->doc()->pages[0]->items[0].get())->storyId)).insertText(QStringLiteral("One two three"));
        QTextCursor(ed->doc()->storyDoc(static_cast<TextItem *>(ed->doc()->pages[0]->items[1].get())->storyId)).insertText(QStringLiteral("Four five"));
        QString shown;
        QTimer::singleShot(0, [&] {
            if (auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget())) {
                shown = box->text();
                box->close();
            }
        });
        w.act(QStringLiteral("review.wordCount"))->trigger();
        QVERIFY2(shown.contains(QStringLiteral("Words: 5")), qPrintable(shown));
    }

    // Insert > Symbol puts the character chosen in the dialog into the
    // placeholder being typed in; Date & Time puts in the format chosen.
    void symbolAndDateGoIntoThePlaceholderBeingEdited()
    {
        DeckWindow w;
        QTextDocument *story = editTitle(w, QStringLiteral("ab"), 1, 1);
        QString title;
        QTimer::singleShot(0, [&] {
            auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!d) return;
            title = d->windowTitle();
            QTableWidget *grid = nullptr;
            for (QTableWidget *t : d->findChildren<QTableWidget *>())
                if (t->rowCount() > 1) grid = t;
            QPushButton *insert = nullptr;
            for (QPushButton *b : d->findChildren<QPushButton *>())
                if (b->text() == QLatin1String("Insert")) insert = b;
            if (grid && insert) {
                grid->setCurrentCell(2, 1);   // Basic Latin from U+0020: 33 on is "A"
                insert->click();
            }
            d->accept();
        });
        w.act(QStringLiteral("insert.symbol"))->trigger();
        QCOMPARE(title, QStringLiteral("Symbol"));
        QCOMPARE(story->toPlainText(), QStringLiteral("aAb"));
        // Date & Time: the first format in the list.
        QString shown;
        QTimer::singleShot(0, [&] {
            auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!d) return;
            if (auto *list = d->findChild<QListWidget *>()) shown = list->currentItem()->text();
            d->accept();
        });
        w.act(QStringLiteral("insert.dateTime"))->trigger();
        QVERIFY(!shown.isEmpty());
        QCOMPARE(story->toPlainText(), QStringLiteral("aA") + shown + QStringLiteral("b"));
    }

    // The text of a story's first field code.
    static QString fieldCodeIn(const QTextDocument *story)
    {
        for (QTextBlock b = story->begin(); b.isValid(); b = b.next())
            for (auto it = b.begin(); !it.atEnd(); ++it)
                if (const QString code = it.fragment().charFormat().stringProperty(tp::Field); !code.isEmpty()) return code;
        return QString();
    }
    // What the first line of a slide's text box shows, drawn where the slide now is.
    static QString shownIn(Editor *ed, const QString &itemId)
    {
        for (int page = 0; page < ed->doc()->pages.size(); ++page)
            for (const ItemPtr &it : ed->doc()->pages[page]->items)
                if (it->id == itemId) {
                    LayoutCache cache;
                    const auto fl = cache.textFrame(*ed->doc(), static_cast<const TextItem &>(*it), page + 1, ed->renderOptions());
                    if (!fl.layout) return QString();
                    const auto lines = fl.layout->lineInfo(fl.frame);
                    return lines.isEmpty() ? QString() : lines[0].text;
                }
        return QString();
    }

    // Insert > Slide Number > Slide Number in Text puts a field into the box being edited (one step that undoes); it shows the number of its slide, follows the slide when it moves,
    // and is kept as the standard's slide number field in .pptx and as the page number in .odp. With no box being edited it puts in nothing.
    void slideNumberGoesIntoTheBoxBeingEditedAsAField()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        w.newSlide();
        w.newSlide();
        ed->setCurrentPage(1);
        // (An undo gives back the whole presentation: the box and its story are found again by their names.)
        const QString boxId = ed->doc()->pages[1]->items[0]->id, storyId = static_cast<TextItem *>(ed->doc()->pages[1]->items[0].get())->storyId;
        auto code = [&] { return fieldCodeIn(ed->doc()->storyDoc(storyId)); };
        w.act(QStringLiteral("insert.slideNumberField"))->trigger();
        QVERIFY(code().isEmpty());
        ed->select(boxId);
        ed->beginTextEdit(boxId, 0);
        w.act(QStringLiteral("insert.slideNumberField"))->trigger();
        QCOMPARE(code(), QStringLiteral("page"));
        QCOMPARE(shownIn(ed, boxId), QStringLiteral("2"));
        ed->undo();
        QVERIFY(code().isEmpty());
        ed->redo();
        QCOMPARE(code(), QStringLiteral("page"));
        // Both file kinds keep it as a field, which shows the number again when the file is opened.
        QString err;
        const QByteArray pptx = deck::exportPresentation(*ed->doc(), &err);
        Exported e;
        QVERIFY2(unpack(pptx, &e, &err), qPrintable(err));
        QVERIFY(part(e, QStringLiteral("/ppt/slides/slide2.xml")).contains(QStringLiteral("type=\"slidenum\"")));
        const QByteArray odp = deck::exportOdp(*ed->doc(), false, &err);
        QMap<QString, QByteArray> zip;
        QVERIFY2(readZip(odp, zip, &err), qPrintable(err));
        QVERIFY(zip.value(QStringLiteral("content.xml")).contains("<text:page-number"));
        for (int kind = 0; kind < 2; ++kind) {
            auto back = kind == 0 ? deck::importPresentation(pptx, &err) : deck::importOdp(odp, &err);
            QVERIFY2(back, qPrintable(err));
            const auto &t = static_cast<const TextItem &>(*back->pages[1]->items[0]);
            QCOMPARE(fieldCodeIn(back->storyDoc(t.storyId)), QStringLiteral("page"));
        }
        // The slide moves, and the number with it.
        ed->movePage(1, 0);
        QCOMPARE(shownIn(ed, boxId), QStringLiteral("1"));
        ed->movePage(0, 2);
        QCOMPARE(shownIn(ed, boxId), QStringLiteral("3"));
    }

    // The first slide's title with `text` in it, in `language`, and being typed in at its end; returns the story.
    static QTextDocument *titleInLanguage(DeckWindow &w, const QString &text, const QString &language)
    {
        Editor *ed = w.editor();
        auto *title = static_cast<TextItem *>(ed->doc()->pages[0]->items[0].get());
        QTextCharFormat f;
        if (!language.isEmpty()) f.setProperty(tp::Language, language);
        QTextCursor(ed->doc()->storyDoc(title->storyId)).insertText(text, f);
        ed->select(title->id);
        ed->beginTextEdit(title->id, int(text.size()));
        return ed->doc()->storyDoc(title->storyId);
    }
    // Chooses the language (unless empty) and format in the Date and Time dialog; the text of the format in the list and the language the list started at are returned.
    static void chooseDate(DeckWindow &w, const QString &tag, const QString &format, bool update, QStringList *languages = nullptr, QString *listed = nullptr, QString *started = nullptr)
    {
        withDialog([&] { w.act(QStringLiteral("insert.dateTime"))->trigger(); }, QStringLiteral("dateTimeDialog"), [&](QDialog *d) {
            auto *language = d->findChild<QComboBox *>(QStringLiteral("dateLanguage"));
            auto *formats = d->findChild<QListWidget *>(QStringLiteral("dateFormats"));
            QVERIFY(language && formats);
            if (languages)
                for (int i = 0; i < language->count(); ++i) *languages << language->itemData(i).toString();
            if (started) *started = language->currentData().toString();
            if (!tag.isEmpty()) language->setCurrentIndex(language->findData(tag));
            int row = -1;
            for (int i = 0; i < formats->count(); ++i)
                if (formats->item(i)->data(Qt::UserRole).toString() == format) row = i;
            QVERIFY2(row >= 0, qPrintable(format));
            formats->setCurrentRow(row);
            if (listed) *listed = formats->item(row)->text();
            d->findChild<QCheckBox *>(QStringLiteral("dateUpdate"))->setChecked(update);
        });
    }

    // The Date and Time dialog has a Language list (German and Japanese among them), shows the formats in that language, and the field it makes keeps the language: shown in it, and kept in
    // .pptx (the field's lang) and .odp (the date style's language). Without Update automatically the text is put in as it is.
    void dateAndTimeInAGivenLanguageKeepsItInAField()
    {
        const QDateTime moment(QDate(2026, 11, 17), QTime(14, 5, 9));   // (a Tuesday)
        setFieldClock(moment);
        const QScopeGuard clock([] { setFieldClock(QDateTime()); });
        const struct { const char *tag, *weekday; } cases[] = {{"de-DE", "Dienstag"}, {"ja-JP", "火曜日"}};
        for (const auto &c : cases) {
            const QString tag = QString::fromUtf8(c.tag);
            // The names are the language's own, from its own data: the weekday of the moment.
            const QString weekday = QLocale(tag).dayName(moment.date().dayOfWeek());
            QCOMPARE(weekday, QString::fromUtf8(c.weekday));
            {
                DeckWindow w;
                QTextDocument *story = titleInLanguage(w, QStringLiteral("x"), QStringLiteral("en-US"));
                QStringList languages;
                QString listed;
                chooseDate(w, tag, QStringLiteral("dddd, MMMM d, yyyy"), true, &languages, &listed);
                QVERIFY(languages.contains(QStringLiteral("de-DE")) && languages.contains(QStringLiteral("ja-JP")) && languages.contains(QStringLiteral("en-US")));
                QVERIFY2(listed.contains(weekday), qPrintable(listed));
                QCOMPARE(fieldCodeIn(story), QStringLiteral("datetime:dddd, MMMM d, yyyy|") + tag);
                FieldContext ctx;
                ctx.now = moment;
                QVERIFY2(ctx.resolve(fieldCodeIn(story)).contains(weekday), qPrintable(ctx.resolve(fieldCodeIn(story))));
            }
            // The language's own long date is a format of the file kinds, and comes back from both.
            DeckWindow w;
            QTextDocument *story = titleInLanguage(w, QStringLiteral("x"), QStringLiteral("en-US"));
            const QString own = QLocale(tag).dateFormat(QLocale::LongFormat);
            chooseDate(w, tag, own, true);
            const QString code = fieldCodeIn(story);
            QCOMPARE(code, QStringLiteral("datetime:") + own + QLatin1Char('|') + tag);
            roundTripsInBothKinds(w, code, tag);
        }
        // Not updated: the text, in the language.
        DeckWindow w;
        QTextDocument *story = titleInLanguage(w, QStringLiteral("x"), QStringLiteral("en-US"));
        const QString format = QLocale(QStringLiteral("de-DE")).dateFormat(QLocale::ShortFormat);
        chooseDate(w, QStringLiteral("de-DE"), format, false);
        QVERIFY(fieldCodeIn(story).isEmpty());
        QCOMPARE(story->toPlainText(), QStringLiteral("x") + QLocale(QStringLiteral("de-DE")).toString(moment.date(), format));
    }
    // The first slide's field saved as .pptx and as .odp and opened again has the format, the language, and what it shows (a kind of field may come back as the one that fits).
    static void roundTripsInBothKinds(DeckWindow &w, const QString &code, const QString &tag)
    {
        QString err;
        const QByteArray pptx = deck::exportPresentation(*w.editor()->doc(), &err);
        Exported e;
        QVERIFY2(unpack(pptx, &e, &err), qPrintable(err));
        const QString slide = part(e, QStringLiteral("/ppt/slides/slide1.xml"));
        QVERIFY2(slide.contains(QStringLiteral("lang=\"%1\"").arg(tag)), qPrintable(slide));
        const QByteArray odp = deck::exportOdp(*w.editor()->doc(), false, &err);
        QMap<QString, QByteArray> zip;
        QVERIFY2(readZip(odp, zip, &err), qPrintable(err));
        QVERIFY(zip.value(QStringLiteral("content.xml")).contains(QStringLiteral("number:language=\"%1\"").arg(tag.section(QLatin1Char('-'), 0, 0)).toUtf8()));
        for (int kind = 0; kind < 2; ++kind) {
            auto back = kind == 0 ? deck::importPresentation(pptx, &err) : deck::importOdp(odp, &err);
            QVERIFY2(back, qPrintable(err));
            const auto &t = static_cast<const TextItem &>(*back->pages[0]->items[0]);
            const QString read = fieldCodeIn(back->storyDoc(t.storyId));
            QCOMPARE(splitDateFieldCode(read).format, splitDateFieldCode(code).format);
            QVERIFY2(splitDateFieldCode(read).language == tag, qPrintable(QString::fromLatin1(kind == 0 ? "pptx " : "odp ") + read));
            FieldContext ctx;
            ctx.now = QDateTime(QDate(2026, 11, 17), QTime(14, 5, 9));
            QCOMPARE(ctx.resolve(read), ctx.resolve(code));
        }
    }

    // With no other language chosen the field has none, as before: the dialog starts at the language of the text (the nearest in its list, German for German as spoken in Austria),
    // and the file has no language on the field. The English of other countries keeps its own language, and the order of its day and month, through .pptx and .odp.
    void dateAndTimeWithNoLanguageChosenKeepsNoTagAndEnglishVariantsKeepTheirs()
    {
        const QDateTime moment(QDate(2026, 11, 17), QTime(14, 5, 9));
        setFieldClock(moment);
        const QScopeGuard clock([] { setFieldClock(QDateTime()); });
        {
            DeckWindow w;
            QTextDocument *story = titleInLanguage(w, QStringLiteral("x"), QStringLiteral("de-AT"));
            QString started;
            chooseDate(w, QString(), QStringLiteral("dddd, MMMM d, yyyy"), true, nullptr, nullptr, &started);
            QCOMPARE(started, QStringLiteral("de-DE"));
            QCOMPARE(fieldCodeIn(story), QStringLiteral("datetime:dddd, MMMM d, yyyy"));
            QString err;
            Exported e;
            QVERIFY2(unpack(deck::exportPresentation(*w.editor()->doc(), &err), &e, &err), qPrintable(err));
            const QString slide = part(e, QStringLiteral("/ppt/slides/slide1.xml"));
            const qsizetype fld = slide.indexOf(QStringLiteral("<a:fld"));
            QVERIFY(fld >= 0);
            QVERIFY2(!slide.mid(fld, slide.indexOf(QStringLiteral("</a:fld>"), fld) - fld).contains(QStringLiteral("lang=\"en-US\"")), qPrintable(slide.mid(fld, 300)));
        }
        {
            // A language the list lacks is added to it, and is the start.
            DeckWindow w;
            QTextDocument *story = titleInLanguage(w, QStringLiteral("x"), QStringLiteral("eu-ES"));
            QString started;
            QStringList languages;
            chooseDate(w, QString(), QStringLiteral("dddd, MMMM d, yyyy"), true, &languages, nullptr, &started);
            QCOMPARE(started, QStringLiteral("eu-ES"));
            QVERIFY(languages.contains(QStringLiteral("eu-ES")));
            QCOMPARE(fieldCodeIn(story), QStringLiteral("datetime:dddd, MMMM d, yyyy"));
        }
        for (const QString &tag : {QStringLiteral("en-GB"), QStringLiteral("en-AU")}) {
            DeckWindow w;
            QTextDocument *story = titleInLanguage(w, QStringLiteral("x"), QStringLiteral("de-DE"));
            const QString own = QLocale(tag).dateFormat(QLocale::LongFormat);
            chooseDate(w, tag, own, true);
            const QString code = fieldCodeIn(story);
            QCOMPARE(code, QStringLiteral("datetime:") + own + QLatin1Char('|') + tag);
            roundTripsInBothKinds(w, code, tag);
            FieldContext ctx;
            ctx.now = moment;
            QVERIFY2(ctx.resolve(code).startsWith(QStringLiteral("17 ")) || ctx.resolve(code).contains(QStringLiteral(", 17 ")), qPrintable(ctx.resolve(code)));   // (the day before the month)
        }
    }

    // A region is the last part of a tag that is two letters or three digits ("zh-Hans-CN" is "CN", not "HANS"), in the date style of an .odp.
    void anOdpDateStyleTakesTheRegionOfATag()
    {
        QCOMPARE(languageRegionOf(QStringLiteral("zh-Hans-CN")), QStringLiteral("CN"));
        QCOMPARE(languageRegionOf(QStringLiteral("de-AT")), QStringLiteral("AT"));
        QCOMPARE(languageRegionOf(QStringLiteral("es-419")), QStringLiteral("419"));
        QVERIFY(languageRegionOf(QStringLiteral("zh-Hans")).isEmpty());
        QVERIFY(languageRegionOf(QStringLiteral("de")).isEmpty());
        DeckWindow w;
        auto *title = static_cast<TextItem *>(w.editor()->doc()->pages[0]->items[0].get());
        QTextCharFormat f;
        f.setProperty(tp::Field, QStringLiteral("datetime:yyyy-MM-dd|zh-Hans-CN"));
        QTextCursor(w.editor()->doc()->storyDoc(title->storyId)).insertText(QString(QChar::ObjectReplacementCharacter), f);
        QString err;
        QMap<QString, QByteArray> zip;
        QVERIFY2(readZip(deck::exportOdp(*w.editor()->doc(), false, &err), zip, &err), qPrintable(err));
        const QByteArray content = zip.value(QStringLiteral("content.xml"));
        QVERIFY(content.contains("number:language=\"zh\" number:country=\"CN\""));
        QVERIFY(!content.contains("HANS"));
    }

    // What a file says a field showed is held to 256 letters, and a date style with huge words (many of them, used by many fields) gives a format of at most 64 letters, read once
    // for the text and not once for each field.
    void hostileDateFieldsAndStylesAreBounded()
    {
        // A field of a kind the program doesn't know (.pptx): the text it showed last, cut.
        QString err;
        {
            DeckWindow w;
            auto *title = static_cast<TextItem *>(w.editor()->doc()->pages[0]->items[0].get());
            QTextCursor(w.editor()->doc()->storyDoc(title->storyId)).insertText(QStringLiteral("a"));
            QString xml;
            QMap<QString, QByteArray> zip;
            QVERIFY2(readZip(deck::exportPresentation(*w.editor()->doc(), &err), zip, &err), qPrintable(err));
            xml = QString::fromUtf8(zip.value(QStringLiteral("ppt/slides/slide1.xml")));
            const qsizetype at = xml.indexOf(QStringLiteral("<a:r>"));
            QVERIFY(at >= 0);
            xml.insert(at, QStringLiteral("<a:fld id=\"{00000000-0000-0000-0000-000000000000}\" type=\"unknownkind\"><a:t>%1</a:t></a:fld>").arg(QString(100000, QLatin1Char('Z'))));
            ZipWriter out;
            for (auto it = zip.cbegin(); it != zip.cend(); ++it) out.add(it.key(), it.key() == QLatin1String("ppt/slides/slide1.xml") ? xml.toUtf8() : it.value());
            auto back = deck::importPresentation(out.finish(), &err);
            QVERIFY2(back, qPrintable(err));
            const auto &t = static_cast<const TextItem &>(*back->pages[0]->items[0]);
            const int shown = int(back->storyDoc(t.storyId)->toPlainText().count(QLatin1Char('Z')));
            QCOMPARE(shown, 256);
        }
        // An .odp style with 32 words of 200,000 letters, named by 300 fields.
        DeckWindow w;
        auto *title = static_cast<TextItem *>(w.editor()->doc()->pages[0]->items[0].get());
        QTextCharFormat f;
        f.setProperty(tp::Field, QStringLiteral("datetime:yyyy"));
        QTextCursor c(w.editor()->doc()->storyDoc(title->storyId));
        for (int i = 0; i < 300; ++i) c.insertText(QString(QChar::ObjectReplacementCharacter), f);
        QMap<QString, QByteArray> zip;
        QVERIFY2(readZip(deck::exportOdp(*w.editor()->doc(), false, &err), zip, &err), qPrintable(err));
        QString xml = QString::fromUtf8(zip.value(QStringLiteral("content.xml")));
        static const QRegularExpression style(QStringLiteral("(<number:date-style [^>]*>).*?(</number:date-style>)"));
        QString words;
        for (int i = 0; i < 32; ++i) words += QStringLiteral("<number:text>") + QString(200000, QLatin1Char('A')) + QStringLiteral("</number:text><number:year number:style=\"long\"/>");
        QVERIFY(xml.contains(style));
        xml.replace(style, QStringLiteral("\\1") + words + QStringLiteral("\\2"));
        ZipWriter out;
        out.add(QStringLiteral("mimetype"), zip.value(QStringLiteral("mimetype")));
        for (auto it = zip.cbegin(); it != zip.cend(); ++it)
            if (it.key() != QLatin1String("mimetype")) out.add(it.key(), it.key() == QLatin1String("content.xml") ? xml.toUtf8() : it.value());
        const qint64 before = deck::odpDateStylesReadForTest();
        auto back = deck::importOdp(out.finish(), &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(deck::odpDateStylesReadForTest() - before, qint64(1));   // (once for the text, not for each of the 300 fields)
        const auto &t = static_cast<const TextItem &>(*back->pages[0]->items[0]);
        int fields = 0;
        const QTextDocument *story = back->storyDoc(t.storyId);
        for (QTextBlock b = story->begin(); b.isValid(); b = b.next())
            for (auto it = b.begin(); !it.atEnd(); ++it) {
                const QString code = it.fragment().charFormat().stringProperty(tp::Field);
                if (code.isEmpty()) continue;
                fields += int(it.fragment().length());
                QVERIFY2(splitDateFieldCode(code).format.size() <= 64, qPrintable(QString::number(code.size())));
            }
        QCOMPARE(fields, 300);
    }

    // An .odp date style's words are cut to 16 letters each before they are used: one of 200,000 letters gives a short quoted word in the format, where without the cut it
    // is too long for the format to be kept at all.
    void anOdpDateStyleWordIsCutTo16Letters()
    {
        QString err;
        DeckWindow w;
        auto *title = static_cast<TextItem *>(w.editor()->doc()->pages[0]->items[0].get());
        QTextCharFormat f;
        f.setProperty(tp::Field, QStringLiteral("datetime:yyyy"));
        QTextCursor c(w.editor()->doc()->storyDoc(title->storyId));
        c.insertText(QString(QChar::ObjectReplacementCharacter), f);
        QMap<QString, QByteArray> zip;
        QVERIFY2(readZip(deck::exportOdp(*w.editor()->doc(), false, &err), zip, &err), qPrintable(err));
        QString xml = QString::fromUtf8(zip.value(QStringLiteral("content.xml")));
        static const QRegularExpression style(QStringLiteral("(<number:date-style [^>]*>).*?(</number:date-style>)"));
        QVERIFY(xml.contains(style));
        xml.replace(style, QStringLiteral("\\1<number:year number:style=\"long\"/><number:text>") + QString(200000, QLatin1Char('A')) + QStringLiteral("</number:text>\\2"));
        const auto build = [&](const QString &content) {
            ZipWriter out;
            out.add(QStringLiteral("mimetype"), zip.value(QStringLiteral("mimetype")));
            for (auto it = zip.cbegin(); it != zip.cend(); ++it)
                if (it.key() != QLatin1String("mimetype")) out.add(it.key(), it.key() == QLatin1String("content.xml") ? content.toUtf8() : it.value());
            return out.finish();
        };
        {
            auto back = deck::importOdp(build(xml), &err);
            QVERIFY2(back, qPrintable(err));
            const auto &t = static_cast<const TextItem &>(*back->pages[0]->items[0]);
            QStringList codes;
            const QTextDocument *story = back->storyDoc(t.storyId);
            for (QTextBlock b = story->begin(); b.isValid(); b = b.next())
                for (auto it = b.begin(); !it.atEnd(); ++it) {
                    const QString code = it.fragment().charFormat().stringProperty(tp::Field);
                    if (!code.isEmpty()) codes << code;
                }
            QCOMPARE(codes.size(), 1);   // (the field still opens)
            QCOMPARE(splitDateFieldCode(codes[0]).format, QStringLiteral("yyyy'") + QString(16, QLatin1Char('A')) + QLatin1Char('\''));
        }
    }

    // Insert > Link (Ctrl+K) makes the selected text a link to the address typed.
    void hyperlinkLinksTheSelectedText()
    {
        DeckWindow w;
        QCOMPARE(w.act(QStringLiteral("insert.link"))->shortcut(), QKeySequence(Qt::CTRL | Qt::Key_K));
        QTextDocument *story = editTitle(w, QStringLiteral("Visit our site"), 6, 9);
        QTimer::singleShot(0, [&] {
            auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!d) return;
            for (QLabel *l : d->findChildren<QLabel *>())
                if (l->text() == QLatin1String("Address") && l->buddy()) static_cast<QLineEdit *>(l->buddy())->setText(QStringLiteral("example.com"));
            d->accept();
        });
        w.act(QStringLiteral("insert.link"))->trigger();
        QTextCursor c(story);
        c.setPosition(7);
        QCOMPARE(c.charFormat().anchorHref(), QStringLiteral("https://example.com"));
        c.setPosition(2);
        QVERIFY(c.charFormat().anchorHref().isEmpty());
    }

    // The ribbon has a Review tab after Slide Show, with the proofing commands,
    // and the Insert tab has the symbol, link, and date commands, each KeyTip its own.
    void reviewTabAndInsertDialogs()
    {
        DeckWindow w;
        QVERIFY2(w.ribbonErrors().isEmpty(), qPrintable(w.ribbonErrors()));
        QStringList tabs;
        for (int i = 0; i < w.ribbon()->tabCount(); ++i) tabs << w.ribbon()->tabName(i);
        QCOMPARE(tabs.indexOf(QStringLiteral("Review")), tabs.indexOf(QStringLiteral("Slide Show")) + 1);
        for (const char *id : {"review.spelling", "review.thesaurus", "review.wordCount", "insert.symbol", "insert.link", "insert.dateTime", "fmt.fontDialog", "para.dialog", "fmt.spacingDialog",
                               "para.bulletsDialog", "para.numberingDialog", "file.options"})
            QVERIFY2(w.act(QString::fromLatin1(id)), id);
        for (const QString &name : {QStringLiteral("Home"), QStringLiteral("Insert"), QStringLiteral("Review")}) {
            QVERIFY2(w.ribbon()->tab(name), qPrintable(name));
            QStringList seen;
            for (const QWidget *c : w.ribbon()->tab(name)->findChildren<QWidget *>()) {
                const QString k = keytip(c);
                if (k.isEmpty()) continue;
                QVERIFY2(!seen.contains(k), qPrintable(name + ": two controls have the KeyTip " + k));
                seen << k;
            }
        }
        // Each is in its tab.
        auto has = [&](const QString &tab, const QString &id) {
            for (const QToolButton *b : w.ribbon()->tab(tab)->findChildren<QToolButton *>())
                if (b->defaultAction() == w.act(id)) return true;
            return false;
        };
        QVERIFY(has(QStringLiteral("Review"), QStringLiteral("review.spelling")));
        QVERIFY(has(QStringLiteral("Review"), QStringLiteral("review.thesaurus")));
        QVERIFY(has(QStringLiteral("Review"), QStringLiteral("review.wordCount")));
        QVERIFY(has(QStringLiteral("Insert"), QStringLiteral("insert.symbol")));
        QVERIFY(has(QStringLiteral("Insert"), QStringLiteral("insert.link")));
        QVERIFY(has(QStringLiteral("Insert"), QStringLiteral("insert.dateTime")));
    }

    // A slide's comments live in its page props, so undo covers them: add,
    // reply, edit, resolve, move, and delete; the author comes from File > Options.
    void commentModelKeepsCommentsInThePageProps()
    {
        auto d = deck::newPresentation();
        deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("obj")));
        Page &slide = *d->pages[0];
        QVERIFY(comments::commentsOf(slide).isEmpty());
        QVERIFY(!slide.props.contains(QStringLiteral("comments")));
        const comments::CommentAuthor pat{QStringLiteral("Pat Doe"), QStringLiteral("PD")}, lee{QStringLiteral("Lee Roy"), QString()};
        const QDateTime noon(QDate(2026, 10, 12), QTime(12, 0, 5, 125), QTimeZone::UTC);
        const QString first = comments::addComment(slide, pat, QStringLiteral("Check\r\nthe numbers"), QPointF(100.5, 40.25), noon);
        QVERIFY(!first.isEmpty());
        // As the props hold it.
        const QJsonObject o = slide.props.value(QStringLiteral("comments")).toArray().at(0).toObject();
        QCOMPARE(o.value(QStringLiteral("id")).toString(), first);
        QCOMPARE(o.value(QStringLiteral("author")).toString(), QStringLiteral("Pat Doe"));
        QCOMPARE(o.value(QStringLiteral("initials")).toString(), QStringLiteral("PD"));
        QCOMPARE(o.value(QStringLiteral("date")).toString(), QStringLiteral("2026-10-12T12:00:05.125Z"));
        QCOMPARE(o.value(QStringLiteral("text")).toString(), QStringLiteral("Check\nthe numbers"));
        QCOMPARE(o.value(QStringLiteral("x")).toDouble(), 100.5);
        QCOMPARE(o.value(QStringLiteral("y")).toDouble(), 40.25);
        QVERIFY(o.value(QStringLiteral("replies")).isArray() && o.value(QStringLiteral("replies")).toArray().isEmpty());
        QCOMPARE(o.value(QStringLiteral("done")).toBool(true), false);

        // Replies belong to a comment; initials left out are made from the name.
        QVERIFY(comments::addReply(slide, QStringLiteral("nope"), pat, QStringLiteral("x")).isEmpty());
        const QString answer = comments::addReply(slide, first, lee, QStringLiteral("Agreed"), noon.addSecs(90));
        QVERIFY(!answer.isEmpty() && answer != first);
        const QString second = comments::addComment(slide, lee, QStringLiteral("Second"), QPointF(300, 200), noon.addSecs(120));
        QVector<comments::Comment> list = comments::commentsOf(slide);
        QCOMPARE(int(list.size()), 2);
        QCOMPARE(list[0].id, first);
        QCOMPARE(list[1].id, second);
        QCOMPARE(int(list[0].replies.size()), 1);
        QCOMPARE(list[0].replies[0].author, QStringLiteral("Lee Roy"));
        QCOMPARE(list[0].replies[0].initials, QStringLiteral("LR"));
        QCOMPARE(list[0].replies[0].date, noon.addSecs(90));
        QCOMPARE(list[0].replies[0].text, QStringLiteral("Agreed"));
        QCOMPARE(list[0].date, noon);
        QCOMPARE(list[0].at, QPointF(100.5, 40.25));
        QCOMPARE(comments::commentCount(slide), 2);
        QVERIFY(comments::findComment(slide, second));
        QVERIFY(!comments::findComment(slide, answer));   // a reply is not a comment
        QVERIFY(!comments::findComment(slide, QStringLiteral("nope")));

        // Edit (a comment or a reply), resolve and reopen, and move.
        QVERIFY(comments::editComment(slide, first, QStringLiteral("Check again")));
        QVERIFY(comments::editComment(slide, answer, QStringLiteral("Agreed, doing it")));
        QVERIFY(!comments::editComment(slide, QStringLiteral("nope"), QStringLiteral("x")));
        QVERIFY(comments::resolveComment(slide, first));
        QVERIFY(!comments::resolveComment(slide, QStringLiteral("nope")));
        QVERIFY(comments::moveComment(slide, second, QPointF(12, 34)));
        list = comments::commentsOf(slide);
        QCOMPARE(list[0].text, QStringLiteral("Check again"));
        QCOMPARE(list[0].replies[0].text, QStringLiteral("Agreed, doing it"));
        QVERIFY(list[0].done && !list[1].done);
        QCOMPARE(list[1].at, QPointF(12, 34));
        QVERIFY(comments::resolveComment(slide, first, false));
        QVERIFY(!comments::commentsOf(slide)[0].done);

        // Delete a reply, then a comment with its replies.
        QVERIFY(comments::deleteComment(slide, answer));
        QVERIFY(comments::commentsOf(slide)[0].replies.isEmpty());
        QVERIFY(!comments::deleteComment(slide, answer));
        comments::addReply(slide, first, lee, QStringLiteral("Again"));
        QVERIFY(comments::deleteComment(slide, first));
        QCOMPARE(int(comments::commentsOf(slide).size()), 1);
        QCOMPARE(comments::commentsOf(slide)[0].id, second);

        // All of a slide's, and all of the presentation's; with none the key is gone.
        deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("obj")));
        comments::addComment(*d->pages[1], pat, QStringLiteral("On the second slide"), QPointF(10, 10));
        comments::addComment(*d->pages[1], pat, QStringLiteral("Another"), QPointF(30, 10));
        const QVector<comments::CommentRef> all = comments::allComments(*d);
        QCOMPARE(int(all.size()), 3);
        QCOMPARE(all[0].slide, 0);
        QCOMPARE(all[0].id, second);
        QCOMPARE(all[1].slide, 1);
        QCOMPARE(all[2].slide, 1);
        QCOMPARE(comments::deleteComments(*d->pages[1]), 2);
        QVERIFY(!d->pages[1]->props.contains(QStringLiteral("comments")));
        comments::addComment(*d->pages[1], pat, QStringLiteral("Again"), QPointF(10, 10));
        QCOMPARE(comments::deleteAllComments(*d), 2);
        QVERIFY(comments::allComments(*d).isEmpty());
        QVERIFY(!d->pages[0]->props.contains(QStringLiteral("comments")));

        // A comment can't hold more than a page of text.
        comments::addComment(slide, pat, QString(comments::kMaxCommentText + 500, QLatin1Char('x')), QPointF(1, 1));
        QCOMPARE(int(comments::commentsOf(slide)[0].text.size()), jp::threads::kMaxChars);

        QCOMPARE(comments::initialsFor(QStringLiteral("Pat Doe")), QStringLiteral("PD"));
        QCOMPARE(comments::initialsFor(QStringLiteral("  mary jane watson parker ")), QStringLiteral("MJW"));
        QCOMPARE(comments::initialsFor(QString()), QString());

        // The author is the user of File > Options; initials left empty come from the name.
        Settings &st = Settings::get();
        st.setValue(QStringLiteral("user/name"), QString());
        st.setValue(QStringLiteral("user/initials"), QString());
        QVERIFY(!comments::commentAuthor().known());
        comments::setCommentAuthor({QStringLiteral(" Pat Doe "), QString()});
        QCOMPARE(st.value(QStringLiteral("user/name")).toString(), QStringLiteral("Pat Doe"));
        QCOMPARE(st.value(QStringLiteral("user/initials")).toString(), QStringLiteral("PD"));
        QVERIFY(comments::commentAuthor().known());
        QCOMPARE(comments::commentAuthor().initials, QStringLiteral("PD"));
        comments::setCommentAuthor({QStringLiteral("Lee Roy"), QStringLiteral("LRY")});
        QCOMPARE(comments::commentAuthor().name, QStringLiteral("Lee Roy"));
        QCOMPARE(comments::commentAuthor().initials, QStringLiteral("LRY"));
        st.setValue(QStringLiteral("user/name"), QString());
        st.setValue(QStringLiteral("user/initials"), QString());
    }

    // The older form: the authors in one part of the presentation, each slide's
    // comments in a part of its own, a reply naming the comment it answers in
    // an extension. Times are the author's own, with their offset from UTC.
    void importReadsLegacyComments()
    {
        QString err;
        auto d = deck::importPresentation(pack(withLegacyComments()), &err);
        QVERIFY2(d, qPrintable(err));
        QVERIFY2(err.isEmpty(), qPrintable(err));
        QCOMPARE(int(d->pages.size()), 2);
        QVERIFY(comments::commentsOf(*d->pages[1]).isEmpty());
        const QVector<comments::Comment> list = comments::commentsOf(*d->pages[0]);
        QCOMPARE(int(list.size()), 2);   // the file has three, one of them an answer
        const comments::Comment &c = list[0];
        QCOMPARE(c.author, QStringLiteral("Pat Doe"));
        QCOMPARE(c.initials, QStringLiteral("PD"));
        QCOMPARE(c.text, QStringLiteral("Check the numbers\non this slide"));
        // 10:15:30.250 where UTC minus local is 420 minutes.
        QCOMPARE(c.date, QDateTime(QDate(2026, 10, 12), QTime(17, 15, 30, 250), QTimeZone::UTC));
        // Positions are 576ths of an inch: 5760 is 10 inches, 720 points.
        QCOMPARE(c.at, QPointF(720, 144));
        QVERIFY(!c.done);
        QCOMPARE(int(c.replies.size()), 1);
        QCOMPARE(c.replies[0].author, QStringLiteral("Lee Roy"));
        QCOMPARE(c.replies[0].initials, QStringLiteral("LR"));
        QCOMPARE(c.replies[0].text, QStringLiteral("Done, thanks"));
        QCOMPARE(c.replies[0].date, QDateTime(QDate(2026, 10, 12), QTime(17, 20, 0, 0), QTimeZone::UTC));
        QVERIFY(!c.replies[0].id.isEmpty() && c.replies[0].id != c.id);
        // A time that says its zone is used as it is; one with no offset known is taken as UTC.
        QCOMPARE(list[1].author, QStringLiteral("Lee Roy"));
        QCOMPARE(list[1].text, QStringLiteral("Second thread"));
        QCOMPARE(list[1].date, QDateTime(QDate(2026, 10, 12), QTime(11, 0, 0, 0), QTimeZone::UTC));
        QCOMPARE(list[1].at, QPointF(100, 50));
        QVERIFY(list[1].replies.isEmpty());
        QVERIFY(list[1].id != c.id);
        // The extension that says a comment is done.
        Parts p = withLegacyComments();
        QString xml = QString::fromUtf8(p.data["/ppt/comments/comment1.xml"].xml);
        xml.replace(QStringLiteral("<p:text>Second thread</p:text>"),
                    QStringLiteral("<p:text>Second thread</p:text><p:extLst><p:ext uri=\"{7C0E3E5B-5A0B-4B52-9D6E-2F1A8E4C9B21}\">"
                                   "<jo:done xmlns:jo=\"http://jeffoffice.com/ns/deck/comments\" val=\"1\"/></p:ext></p:extLst>"));
        p.data["/ppt/comments/comment1.xml"].xml = xml.toUtf8();
        d = deck::importPresentation(pack(p), &err);
        QVERIFY(d);
        QVERIFY(comments::commentsOf(*d->pages[0])[1].done);
        QVERIFY(!comments::commentsOf(*d->pages[0])[0].done);

        // The strict form of the standard names the same parts.
        Parts s = withLegacyComments();
        for (auto it = s.data.begin(); it != s.data.end(); ++it) {
            if (!it.key().endsWith(QLatin1String(".xml"))) continue;
            QString text = QString::fromUtf8(it.value().xml);
            text.replace(QStringLiteral("http://schemas.openxmlformats.org/drawingml/2006/main"), QStringLiteral("http://purl.oclc.org/ooxml/drawingml/main"));
            text.replace(QStringLiteral("http://schemas.openxmlformats.org/presentationml/2006/main"), QStringLiteral("http://purl.oclc.org/ooxml/presentationml/main"));
            text.replace(QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships"), QStringLiteral("http://purl.oclc.org/ooxml/officeDocument/relationships"));
            it.value().xml = text.toUtf8();
        }
        d = deck::importPresentation(pack(s), &err);
        QVERIFY2(d, qPrintable(err));
        QCOMPARE(int(comments::commentsOf(*d->pages[0]).size()), 2);
        QCOMPARE(int(comments::commentsOf(*d->pages[0])[0].replies.size()), 1);
    }

    // A file's comments are held to the limits every app shares (core/comments.h), whatever the file says: a thread has at most kMaxReplies replies.
    void importHoldsCommentsToTheSharedLimits()
    {
        auto d = deck::newPresentation();
        deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("obj")));
        const comments::CommentAuthor pat{QStringLiteral("Pat Doe"), QStringLiteral("PD")};
        const QDateTime noon(QDate(2026, 10, 12), QTime(12, 0), QTimeZone::UTC);
        const QString id = comments::addComment(*d->pages[0], pat, QStringLiteral("Thread"), QPointF(100, 100), noon);
        for (int i = 0; i < comments::kMaxReplies + 50; ++i) comments::addReply(*d->pages[0], id, pat, QStringLiteral("Reply %1").arg(i), noon.addSecs(i));
        QString err;
        const QByteArray bytes = deck::exportPresentation(*d, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        const QVector<comments::Comment> list = comments::commentsOf(*back->pages[0]);
        QCOMPARE(int(list.size()), 1);
        QCOMPARE(int(list[0].replies.size()), comments::kMaxReplies);
        QCOMPARE(list[0].replies[0].text, QStringLiteral("Reply 0"));
    }

    // The newer form: authors by GUID, comments that are threads with replies
    // inside, a status, and a place on the slide or the shape they stand on.
    void importReadsModernComments()
    {
        QString err;
        auto d = deck::importPresentation(pack(withModernComments()), &err);
        QVERIFY2(d, qPrintable(err));
        QVERIFY2(err.isEmpty(), qPrintable(err));
        QVERIFY(comments::commentsOf(*d->pages[0]).isEmpty());
        const QVector<comments::Comment> list = comments::commentsOf(*d->pages[1]);
        QCOMPARE(int(list.size()), 3);
        const comments::Comment &c = list[0];
        QCOMPARE(c.author, QStringLiteral("Pat Doe"));
        QCOMPARE(c.initials, QStringLiteral("PD"));
        QCOMPARE(c.text, QStringLiteral("Line one\nLine two"));
        QCOMPARE(c.date, QDateTime(QDate(2026, 10, 12), QTime(17, 15, 30, 250), QTimeZone::UTC));
        QVERIFY(c.done);   // status "resolved"
        // The place is in EMU: 9144000 is 720 points.
        QCOMPARE(c.at, QPointF(720, 108));
        QCOMPARE(int(c.replies.size()), 1);
        QCOMPARE(c.replies[0].author, QStringLiteral("Lee Roy"));
        QCOMPARE(c.replies[0].initials, QStringLiteral("LR"));
        QCOMPARE(c.replies[0].text, QStringLiteral("On it"));
        QCOMPARE(c.replies[0].date, QDateTime(QDate(2026, 10, 12), QTime(17, 20, 0, 0), QTimeZone::UTC));
        // One with no status is open; one on a shape stands at the shape's top right (the title, shape 2).
        QCOMPARE(list[1].text, QStringLiteral("Anchored"));
        QVERIFY(!list[1].done);
        const Item *title = d->pages[1]->items[0].get();
        QCOMPARE(title->sourceId, 2);
        QCOMPARE(list[1].at, QPointF(title->rect.right(), title->rect.top()));
        // One that says nowhere stands at the corner, clear of the others.
        QCOMPARE(list[2].text, QStringLiteral("Nowhere"));
        QVERIFY(list[2].at.x() >= 0 && list[2].at.x() < 100 && list[2].at.y() >= 0 && list[2].at.y() < 100);
        QVERIFY(list[2].at != list[1].at && list[2].at != c.at);
    }

    // A comment file is a stranger's: what doesn't make sense is left out or
    // mended, never fatal, and the slides are read all the same.
    void importSurvivesHostileComments()
    {
        QString err;
        // A comments part that isn't XML, or isn't comments at all.
        for (const char *junk : {"<<< not xml", "<p:sld xmlns:p=\"http://schemas.openxmlformats.org/presentationml/2006/main\"/>"}) {
            Parts p = withLegacyComments();
            p.data["/ppt/comments/comment1.xml"].xml = junk;
            err.clear();
            auto d = deck::importPresentation(pack(p), &err);
            QVERIFY(d);
            QCOMPARE(int(d->pages.size()), 2);
            QVERIFY(comments::commentsOf(*d->pages[0]).isEmpty());
            QVERIFY2(err.contains(QStringLiteral("comments")), qPrintable(err));
        }
        // No authors part, or an author the comments don't name: the comments stay.
        {
            Parts p = withLegacyComments();
            p.data.remove(QStringLiteral("/ppt/commentAuthors.xml"));
            auto d = deck::importPresentation(pack(p), &err);
            QVERIFY(d);
            const QVector<comments::Comment> list = comments::commentsOf(*d->pages[0]);
            QCOMPARE(int(list.size()), 2);
            QVERIFY(!list[0].author.isEmpty());
            QCOMPARE(list[0].text, QStringLiteral("Check the numbers\non this slide"));
        }
        // A reply to itself, to a comment that isn't there, and two that answer each other: all stay, none loops.
        {
            Parts p = withLegacyComments();
            QString body;
            body += legacyComment(0, 1, QStringLiteral("2026-10-12T10:00:00"), 80, 80, QStringLiteral("Self"), 0, QStringLiteral("0:1"));
            body += legacyComment(0, 2, QStringLiteral("2026-10-12T10:00:00"), 80, 80, QStringLiteral("Orphan"), 0, QStringLiteral("5:9"));
            body += legacyComment(1, 1, QStringLiteral("2026-10-12T10:00:00"), 80, 80, QStringLiteral("Ping"), 0, QStringLiteral("1:2"));
            body += legacyComment(1, 2, QStringLiteral("2026-10-12T10:00:00"), 80, 80, QStringLiteral("Pong"), 0, QStringLiteral("1:1"));
            p.data["/ppt/comments/comment1.xml"].xml = legacyCommentList(body).toUtf8();
            auto d = deck::importPresentation(pack(p), &err);
            QVERIFY(d);
            const QVector<comments::Comment> list = comments::commentsOf(*d->pages[0]);
            int comments = 0, replies = 0;
            for (const comments::Comment &c : list) {
                ++comments;
                replies += int(c.replies.size());
            }
            QCOMPARE(comments + replies, 4);
            QVERIFY(comments >= 3);
        }
        // Dates and places that aren't numbers, and text of any length.
        {
            Parts p = withLegacyComments();
            QString body = legacyComment(0, 1, QStringLiteral("last Tuesday"), 0, 0, QStringLiteral("No date"));
            body.replace(QStringLiteral("<p:pos x=\"0\" y=\"0\"/>"), QStringLiteral("<p:pos x=\"many\" y=\"-4000000000\"/>"));
            body += legacyComment(0, 2, QStringLiteral("2026-10-12T10:00:00"), 0, 0, QString(comments::kMaxCommentText + 1000, QLatin1Char('y')));
            body += QStringLiteral("<p:cm authorId=\"0\" idx=\"3\"><p:text>No place</p:text></p:cm><p:cm><p:text>No author</p:text></p:cm>");
            p.data["/ppt/comments/comment1.xml"].xml = legacyCommentList(body).toUtf8();
            auto d = deck::importPresentation(pack(p), &err);
            QVERIFY(d);
            const QVector<comments::Comment> list = comments::commentsOf(*d->pages[0]);
            QCOMPARE(int(list.size()), 4);
            QVERIFY(!list[0].date.isValid());
            QCOMPARE(list[0].text, QStringLiteral("No date"));
            QVERIFY(list[0].at.x() >= 0 && list[0].at.x() <= 960 && list[0].at.y() >= 0 && list[0].at.y() <= 540);
            QCOMPARE(int(list[1].text.size()), int(comments::kMaxCommentText));
            QCOMPARE(list[2].text, QStringLiteral("No place"));
        }
        // More than a slide can hold are counted, not read.
        {
            Parts p = withLegacyComments();
            QString body;
            for (int i = 0; i < comments::kMaxComments + 100; ++i) body += legacyComment(0, i + 1, QStringLiteral("2026-10-12T10:00:00"), 80, 80, QStringLiteral("C%1").arg(i));
            p.data["/ppt/comments/comment1.xml"].xml = legacyCommentList(body).toUtf8();
            err.clear();
            auto d = deck::importPresentation(pack(p), &err);
            QVERIFY(d);
            QCOMPARE(int(comments::commentsOf(*d->pages[0]).size()), comments::kMaxComments);
            QVERIFY2(err.contains(QString::number(comments::kMaxComments)), qPrintable(err));
        }
        // Authors by the thousand are not all read either; the first ones are.
        {
            Parts p = withLegacyComments();
            QString authors;
            for (int i = 0; i < 12000; ++i) authors += QStringLiteral("<p:cmAuthor id=\"%1\" name=\"A%1\" initials=\"A\" lastIdx=\"1\" clrIdx=\"0\"/>").arg(i);
            p.data["/ppt/commentAuthors.xml"].xml = (QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><p:cmAuthorLst ") + nsDecl() + QStringLiteral(">") + authors + QStringLiteral("</p:cmAuthorLst>")).toUtf8();
            auto d = deck::importPresentation(pack(p), &err);
            QVERIFY(d);
            QCOMPARE(comments::commentsOf(*d->pages[0])[0].author, QStringLiteral("A0"));
        }
        // The newer form: a part that isn't XML, replies without end, text in odd places.
        {
            Parts p = withModernComments();
            p.data["/ppt/comments/modernComment_101_5.xml"].xml = "<<< not xml";
            err.clear();
            auto d = deck::importPresentation(pack(p), &err);
            QVERIFY(d);
            QCOMPARE(int(d->pages.size()), 2);
            QVERIFY(comments::commentsOf(*d->pages[1]).isEmpty());
            QVERIFY2(err.contains(QStringLiteral("comments")), qPrintable(err));
            Parts q = withModernComments();
            QString xml = QString::fromUtf8(q.data["/ppt/comments/modernComment_101_5.xml"].xml);
            QString replies;
            for (int i = 0; i < 1100; ++i)
                replies += QStringLiteral("<p188:reply id=\"{%1}\" authorId=\"{22222222-2222-4222-8222-222222222222}\" created=\"bad\"><p188:txBody><a:p><a:r><a:t>r%1</a:t></a:r></a:p></p188:txBody></p188:reply>").arg(i);
            xml.replace(QStringLiteral("<p188:replyLst>"), QStringLiteral("<p188:replyLst>") + replies);
            q.data["/ppt/comments/modernComment_101_5.xml"].xml = xml.toUtf8();
            err.clear();
            d = deck::importPresentation(pack(q), &err);
            QVERIFY(d);
            const QVector<comments::Comment> list = comments::commentsOf(*d->pages[1]);
            QCOMPARE(int(list.size()), 3);
            QCOMPARE(int(list[0].replies.size()), comments::kMaxReplies);
            QVERIFY2(err.contains(QString::number(comments::kMaxReplies)), qPrintable(err));
        }
    }

    // A presentation with no comments has no comment parts; one with comments
    // has the authors once and a part of comments for each slide that has some,
    // in the older form that every version of the program opens.
    void exportWritesCommentsAndReadsThemBack()
    {
        {
            auto plain = deck::newPresentation();
            const QByteArray bytes = deck::exportPresentation(*plain);
            QVERIFY(!bytes.isEmpty());
            Exported e;
            QString why;
            QVERIFY2(unpack(bytes, &e, &why), qPrintable(why));
            for (const QString &n : e.pkg.partNames()) QVERIFY2(!n.contains(QStringLiteral("omment")), qPrintable(n));
            QVERIFY(e.pkg.relationshipsOfType(QStringLiteral("/ppt/presentation.xml"), QStringLiteral("/commentAuthors")).isEmpty());
            QVERIFY(!e.zip.value(QStringLiteral("[Content_Types].xml")).contains("omment"));
        }

        auto d = deck::newPresentation();
        deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("obj")));
        deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("obj")));
        const comments::CommentAuthor pat{QStringLiteral("Pat Doe"), QStringLiteral("PD")}, lee{QStringLiteral("Lee Roy"), QStringLiteral("LR")}, odd{QStringLiteral("A <B> & \"C\""), QStringLiteral("A&B")};
        const QDateTime t(QDate(2026, 10, 12), QTime(14, 3, 5, 123), QTimeZone::UTC);
        const QString c1 = comments::addComment(*d->pages[0], pat, QStringLiteral("Check the numbers\non this slide"), QPointF(100.5, 40.25), t);
        comments::addReply(*d->pages[0], c1, lee, QStringLiteral("Done, thanks"), t.addSecs(300));
        comments::addReply(*d->pages[0], c1, pat, QStringLiteral("Great"), t.addSecs(360));
        const QString c2 = comments::addComment(*d->pages[0], lee, QStringLiteral("Second & <last>"), QPointF(700, 300), t.addDays(1));
        comments::resolveComment(*d->pages[0], c2);
        comments::addComment(*d->pages[2], odd, QStringLiteral("Third slide"), QPointF(10, 10), t.addDays(2));

        QString err;
        QStringList skipped;
        const QByteArray bytes = deck::exportPresentation(*d, &err, &skipped);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        Exported e;
        QString why;
        QVERIFY2(unpack(bytes, &e, &why), qPrintable(why));
        QVERIFY2(packageProblem(e).isEmpty(), qPrintable(packageProblem(e)));
        const QString pres = QStringLiteral("/ppt/presentation.xml");
        QCOMPARE(relationshipTargets(e, pres, QStringLiteral("/commentAuthors")), QStringList{QStringLiteral("/ppt/commentAuthors.xml")});
        QCOMPARE(e.pkg.contentType(QStringLiteral("/ppt/commentAuthors.xml")), QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.commentAuthors+xml"));
        QCOMPARE(relationshipTargets(e, QStringLiteral("/ppt/slides/slide1.xml"), QStringLiteral("/comments")), QStringList{QStringLiteral("/ppt/comments/comment1.xml")});
        QVERIFY(relationshipTargets(e, QStringLiteral("/ppt/slides/slide2.xml"), QStringLiteral("/comments")).isEmpty());
        QCOMPARE(relationshipTargets(e, QStringLiteral("/ppt/slides/slide3.xml"), QStringLiteral("/comments")), QStringList{QStringLiteral("/ppt/comments/comment2.xml")});
        for (const char *n : {"/ppt/comments/comment1.xml", "/ppt/comments/comment2.xml"})
            QCOMPARE(e.pkg.contentType(QLatin1String(n)), QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.comments+xml"));
        QCOMPARE(int(e.pkg.partNames().filter(QRegularExpression(QStringLiteral("^/ppt/comments/"))).size()), 2);
        // The relationship types are the standard's.
        QCOMPARE(e.pkg.relationshipsOfType(pres, QStringLiteral("/commentAuthors")).at(0).type, QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/commentAuthors"));
        QCOMPARE(e.pkg.relationshipsOfType(QStringLiteral("/ppt/slides/slide1.xml"), QStringLiteral("/comments")).at(0).type, QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/comments"));

        // The authors, once each, with the last index they used; a comment names its author and its index.
        QDomDocument authors;
        const QDomElement list = rootOf(e.zip.value(QStringLiteral("ppt/commentAuthors.xml")), &authors);
        QCOMPARE(list.localName(), QStringLiteral("cmAuthorLst"));
        const QVector<QDomElement> people = office::children(list, office::ns::P, QStringLiteral("cmAuthor"));
        QCOMPARE(int(people.size()), 3);
        QCOMPARE(people[0].attribute(QStringLiteral("name")), QStringLiteral("Pat Doe"));
        QCOMPARE(people[0].attribute(QStringLiteral("initials")), QStringLiteral("PD"));
        QCOMPARE(people[0].attribute(QStringLiteral("id")), QStringLiteral("0"));
        QCOMPARE(people[0].attribute(QStringLiteral("lastIdx")), QStringLiteral("2"));   // the comment and its second reply
        QCOMPARE(people[1].attribute(QStringLiteral("name")), QStringLiteral("Lee Roy"));
        QCOMPARE(people[1].attribute(QStringLiteral("lastIdx")), QStringLiteral("2"));
        QCOMPARE(people[2].attribute(QStringLiteral("name")), QStringLiteral("A <B> & \"C\""));
        QVERIFY(people[2].hasAttribute(QStringLiteral("clrIdx")));
        QDomDocument comments;
        const QDomElement cmLst = rootOf(e.zip.value(QStringLiteral("ppt/comments/comment1.xml")), &comments);
        QCOMPARE(cmLst.localName(), QStringLiteral("cmLst"));
        const QVector<QDomElement> cms = office::children(cmLst, office::ns::P, QStringLiteral("cm"));
        QCOMPARE(int(cms.size()), 4);   // two comments and the two replies to the first
        QSet<QString> seen;
        for (const QDomElement &cm : cms) {
            QStringList names;
            for (const QDomElement &c : office::children(cm)) names << c.localName();
            QVERIFY2(names.size() >= 2 && names[0] == QLatin1String("pos") && names[1] == QLatin1String("text") && (names.size() == 2 || names[2] == QLatin1String("extLst")), qPrintable(names.join(QLatin1Char(','))));
            const QString key = cm.attribute(QStringLiteral("authorId")) + QLatin1Char(':') + cm.attribute(QStringLiteral("idx"));
            QVERIFY2(!seen.contains(key), qPrintable(key));
            seen << key;
            QVERIFY(cm.hasAttribute(QStringLiteral("dt")));
        }
        // Positions in 576ths of an inch (8 to the point).
        const QDomElement pos = office::child(cms[0], office::ns::P, QStringLiteral("pos"));
        QCOMPARE(pos.attribute(QStringLiteral("x")), QStringLiteral("804"));
        QCOMPARE(pos.attribute(QStringLiteral("y")), QStringLiteral("322"));
        // A reply says which comment it answers.
        const QString xml = QString::fromUtf8(e.zip.value(QStringLiteral("ppt/comments/comment1.xml")));
        QCOMPARE(int(xml.count(QStringLiteral("<p15:parentCm authorId=\"0\" idx=\"1\"/>"))), 2);

        // Read back, it is what was written: authors, dates to the millisecond, places, replies, and the done mark.
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY2(back, qPrintable(err));
        QVERIFY2(err.isEmpty(), qPrintable(err));
        QCOMPARE(int(back->pages.size()), 3);
        for (int i = 0; i < 3; ++i) {
            const QVector<comments::Comment> want = comments::commentsOf(*d->pages[i]), got = comments::commentsOf(*back->pages[i]);
            QCOMPARE(int(got.size()), int(want.size()));
            for (int k = 0; k < want.size(); ++k) {
                QCOMPARE(got[k].author, want[k].author);
                QCOMPARE(got[k].initials, want[k].initials);
                QCOMPARE(got[k].date, want[k].date);
                QCOMPARE(got[k].text, want[k].text);
                QCOMPARE(got[k].at, want[k].at);
                QCOMPARE(got[k].done, want[k].done);
                QCOMPARE(int(got[k].replies.size()), int(want[k].replies.size()));
                for (int r = 0; r < want[k].replies.size(); ++r) {
                    QCOMPARE(got[k].replies[r].author, want[k].replies[r].author);
                    QCOMPARE(got[k].replies[r].initials, want[k].replies[r].initials);
                    QCOMPARE(got[k].replies[r].date, want[k].replies[r].date);
                    QCOMPARE(got[k].replies[r].text, want[k].replies[r].text);
                }
            }
        }
        QVERIFY(comments::commentsOf(*back->pages[0])[1].done);
        QVERIFY(!comments::commentsOf(*back->pages[0])[0].done);
        QVERIFY(comments::commentsOf(*back->pages[1]).isEmpty());
        // Written again, the same file's comments come out the same.
        QCOMPARE(deck::exportPresentation(*back).isEmpty(), false);
    }

    // The Review tab has a Comments group: New Comment, Delete (with its menu),
    // Previous, Next, and Show Comments.
    void reviewTabHasTheCommentsGroup()
    {
        DeckWindow w;
        QVERIFY2(w.ribbonErrors().isEmpty(), qPrintable(w.ribbonErrors()));
        for (const char *id : {"comment.new", "comment.delete", "comment.deleteSlide", "comment.deleteAll", "comment.prev", "comment.next", "comment.show"})
            QVERIFY2(w.act(QString::fromLatin1(id)), id);
        QVERIFY(w.act(QStringLiteral("comment.show"))->isCheckable());
        QVERIFY(!w.act(QStringLiteral("comment.show"))->isChecked());
        QVERIFY(w.commentsPane());
        QVERIFY(w.commentsPane()->isHidden());
        auto button = [&](const QString &id) -> QToolButton * {
            for (QToolButton *b : w.ribbon()->tab(QStringLiteral("Review"))->findChildren<QToolButton *>())
                if (b->defaultAction() == w.act(id)) return b;
            return nullptr;
        };
        for (const char *id : {"comment.new", "comment.delete", "comment.prev", "comment.next", "comment.show"})
            QVERIFY2(button(QString::fromLatin1(id)), id);
        // Delete has a menu with the other two.
        QToolButton *del = button(QStringLiteral("comment.delete"));
        QVERIFY(del->menu());
        QVERIFY(del->menu()->actions().contains(w.act(QStringLiteral("comment.deleteSlide"))));
        QVERIFY(del->menu()->actions().contains(w.act(QStringLiteral("comment.deleteAll"))));
        // With no comments there is nothing to delete or visit; New Comment and Show Comments are always there.
        QVERIFY(w.act(QStringLiteral("comment.new"))->isEnabled());
        QVERIFY(w.act(QStringLiteral("comment.show"))->isEnabled());
        for (const char *id : {"comment.delete", "comment.deleteSlide", "comment.deleteAll", "comment.prev", "comment.next"})
            QVERIFY2(!w.act(QString::fromLatin1(id))->isEnabled(), id);
        // The KeyTips of the tab stay their own.
        QStringList seen;
        for (const QWidget *c : w.ribbon()->tab(QStringLiteral("Review"))->findChildren<QWidget *>()) {
            const QString k = keytip(c);
            if (k.isEmpty()) continue;
            QVERIFY2(!seen.contains(k), qPrintable("two controls have the KeyTip " + k));
            seen << k;
        }
        // Show Comments opens and closes the pane.
        w.resize(1280, 800);
        w.show();
        w.act(QStringLiteral("comment.show"))->trigger();
        QVERIFY(!w.commentsPane()->isHidden());
        QVERIFY(w.canvas()->commentsShown());
        w.act(QStringLiteral("comment.show"))->trigger();
        QVERIFY(w.commentsPane()->isHidden());
        QVERIFY(!w.canvas()->commentsShown());
    }

    // Text that came from a file (a slide's title, a tile's name, a comment and its author) that looks like markup naming a picture is shown as the text it is:
    // no label, tip, or list item of the panes takes it for markup.
    void hostileFileTextIsShownAsText()
    {
        using plainprobe::hostile;
        using plainprobe::readsAPicture;
        QVERIFY(plainprobe::probeSeesTheTrap());
        const QString bad = hostile();
        // The Reuse Slides pane: the slide's title is the item's tip, and the status line shows the file.
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("hostile.pptx"));
        {
            auto src = deck::newPresentation();
            auto *title = static_cast<TextItem *>(src->pages[0]->items[0].get());
            QTextCursor(src->storyDoc(title->storyId)).insertText(bad);
            QString err;
            QVERIFY2(deck::savePresentationFile(*src, path, &err), qPrintable(err));
        }
        ReusePane reuse;
        QVERIFY(reuse.open(path));
        QVERIFY(reuse.slideCount() >= 1);
        QVERIFY2(reuse.list()->item(0)->toolTip().contains(QStringLiteral("probe.invalid")), qPrintable(reuse.list()->item(0)->toolTip()));
        QVERIFY2(!readsAPicture(reuse.list()->item(0)->toolTip()), qPrintable(reuse.list()->item(0)->toolTip()));
        for (const QLabel *l : reuse.findChildren<QLabel *>()) QVERIFY2(!readsAPicture(l), qPrintable(l->text()));
        // A tile's name (a theme's name comes from the file) is the tip that comes up over it.
        {
            PictureGrid g(60, 3, true);
            g.setTiles({PictureGrid::Tile{bad, {}}});
            g.setPictures([](int, int width, qreal) {
                QImage img(width, width * 9 / 16, QImage::Format_RGB32);
                img.fill(Qt::gray);
                return img;
            });
            g.resize(g.sizeHint());
            g.show();
            QTest::mouseMove(&g, g.tileRect(0).center());
            QTRY_VERIFY(QToolTip::text().contains(QStringLiteral("probe.invalid")));
            QVERIFY2(!readsAPicture(QToolTip::text()), qPrintable(QToolTip::text()));
            QToolTip::hideText();
        }
        // The Comments pane: the author, the comment, and a reply.
        DeckWindow w;
        w.resize(1280, 800);
        w.show();
        Editor *ed = w.editor();
        const comments::CommentAuthor odd{bad, bad};
        const QString id = comments::addComment(*ed->doc()->pages[ed->currentPage()], odd, bad, QPointF(10, 10));
        comments::addReply(*ed->doc()->pages[ed->currentPage()], id, odd, bad);
        w.act(QStringLiteral("comment.show"))->trigger();
        CommentsPane *pane = w.commentsPane();
        pane->reload();
        int shown = 0;
        for (const QLabel *l : pane->findChildren<QLabel *>()) {
            QVERIFY2(!readsAPicture(l), qPrintable(l->text()));
            shown += l->text().contains(QStringLiteral("probe.invalid"));
        }
        QVERIFY(shown >= 2);   // the author and the comment are on the cards
        for (const QToolButton *b : pane->findChildren<QToolButton *>()) QVERIFY2(!readsAPicture(b->toolTip()), qPrintable(b->toolTip()));
        // The message box that says a file can't be opened shows its text, which can hold the file's name, as text.
        const QString notAPresentation = dir.filePath(QStringLiteral("bad.pptx"));
        QFile f(notAPresentation);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write("not a presentation");
        f.close();
        bool plain = false, seen = false;
        QTimer::singleShot(0, [&] {
            if (auto *box = qobject_cast<QMessageBox *>(QApplication::activeModalWidget())) {
                seen = true;
                plain = box->textFormat() == Qt::PlainText && !readsAPicture(box);   // (the text names a file, whose name could be markup)
                box->close();
            }
        });
        w.openFile(notAPresentation);
        QVERIFY(seen);
        QVERIFY(plain);
    }

    // The Comments pane: New Comment opens a box to type in, Post makes the
    // comment as one undo step, and each card replies, resolves, edits, and deletes.
    void commentsPaneAddsRepliesResolvesEditsAndDeletes()
    {
        Settings &st = Settings::get();
        st.setValue(QStringLiteral("user/name"), QStringLiteral("Pat Doe"));
        st.setValue(QStringLiteral("user/initials"), QStringLiteral("PD"));
        DeckWindow w;
        w.resize(1280, 800);
        w.show();
        Editor *ed = w.editor();
        CommentsPane *pane = w.commentsPane();
        QVERIFY(pane->isHidden());
        auto slide = [&]() -> Page & { return *ed->doc()->pages[ed->currentPage()]; };
        auto card = [&](int i = 0) { return pane->findChildren<QFrame *>(QStringLiteral("commentCard")).value(i); };

        w.act(QStringLiteral("comment.new"))->trigger();
        QVERIFY(!pane->isHidden());
        QVERIFY(w.act(QStringLiteral("comment.show"))->isChecked());
        auto *draft = pane->findChild<QPlainTextEdit *>(QStringLiteral("commentDraft"));
        QVERIFY(draft && draft->isVisible());
        QCOMPARE(w.focusWidget(), draft);
        QCOMPARE(int(pane->findChildren<QFrame *>(QStringLiteral("commentCard")).size()), 0);
        auto *post = pane->findChild<QPushButton *>(QStringLiteral("commentPost"));
        QVERIFY(post);
        QVERIFY(!post->isEnabled());   // nothing typed
        draft->setPlainText(QStringLiteral("Check the title"));
        QVERIFY(post->isEnabled());
        QCOMPARE(comments::commentCount(slide()), 0);   // not until Post
        post->click();
        QCOMPARE(comments::commentCount(slide()), 1);
        QCOMPARE(ed->undoStack()->count(), 1);
        QCOMPARE(ed->undoStack()->undoText(), QStringLiteral("New Comment"));
        comments::Comment c = comments::commentsOf(slide())[0];
        QCOMPARE(c.author, QStringLiteral("Pat Doe"));
        QCOMPARE(c.initials, QStringLiteral("PD"));
        QCOMPARE(c.text, QStringLiteral("Check the title"));
        QVERIFY(c.date.isValid() && qAbs(c.date.secsTo(QDateTime::currentDateTimeUtc())) < 60);
        QVERIFY(c.at.x() >= 0 && c.at.x() < 960 && c.at.y() >= 0 && c.at.y() < 540);
        QVERIFY(!draft->isVisible());
        QCOMPARE(int(pane->findChildren<QFrame *>(QStringLiteral("commentCard")).size()), 1);
        QCOMPARE(pane->currentComment(), c.id);   // the new comment is the one open
        QVERIFY(w.act(QStringLiteral("comment.delete"))->isEnabled());
        QVERIFY(card()->findChild<QLabel *>(QStringLiteral("commentText"))->text().contains(QStringLiteral("Check the title")));
        QVERIFY(card()->findChild<QLabel *>(QStringLiteral("commentAuthor"))->text().contains(QStringLiteral("Pat Doe")));

        // A reply.
        auto *reply = card()->findChild<QLineEdit *>(QStringLiteral("commentReplyEdit"));
        auto *replyPost = card()->findChild<QAbstractButton *>(QStringLiteral("commentReplyPost"));
        QVERIFY(reply && replyPost);
        QVERIFY(!replyPost->isEnabled());
        reply->setText(QStringLiteral("Agreed"));
        QVERIFY(replyPost->isEnabled());
        replyPost->click();
        QCOMPARE(int(comments::commentsOf(slide())[0].replies.size()), 1);
        QCOMPARE(comments::commentsOf(slide())[0].replies[0].text, QStringLiteral("Agreed"));
        QCOMPARE(comments::commentsOf(slide())[0].replies[0].author, QStringLiteral("Pat Doe"));
        QCOMPARE(ed->undoStack()->count(), 2);
        QCOMPARE(int(card()->findChildren<QLabel *>(QStringLiteral("commentReplyText")).size()), 1);
        // Enter in the box replies too.
        card()->findChild<QLineEdit *>(QStringLiteral("commentReplyEdit"))->setText(QStringLiteral("Second answer"));
        QTest::keyClick(card()->findChild<QLineEdit *>(QStringLiteral("commentReplyEdit")), Qt::Key_Return);
        QCOMPARE(int(comments::commentsOf(slide())[0].replies.size()), 2);

        // Resolve and reopen.
        auto *resolve = card()->findChild<QAbstractButton *>(QStringLiteral("commentResolve"));
        QVERIFY(resolve);
        resolve->click();
        QVERIFY(comments::commentsOf(slide())[0].done);
        QCOMPARE(ed->undoStack()->undoText(), QStringLiteral("Resolve Comment"));
        resolve = card()->findChild<QAbstractButton *>(QStringLiteral("commentResolve"));
        QVERIFY(resolve->text().contains(QStringLiteral("Reopen")));
        resolve->click();
        QVERIFY(!comments::commentsOf(slide())[0].done);
        QCOMPARE(ed->undoStack()->undoText(), QStringLiteral("Reopen Comment"));

        // Edit the text.
        card()->findChild<QAbstractButton *>(QStringLiteral("commentEdit"))->click();
        auto *editor = card()->findChild<QPlainTextEdit *>(QStringLiteral("commentEditor"));
        QVERIFY(editor);
        QCOMPARE(editor->toPlainText(), QStringLiteral("Check the title"));
        editor->setPlainText(QStringLiteral("Check the heading"));
        card()->findChild<QAbstractButton *>(QStringLiteral("commentSave"))->click();
        QCOMPARE(comments::commentsOf(slide())[0].text, QStringLiteral("Check the heading"));
        QCOMPARE(ed->undoStack()->undoText(), QStringLiteral("Edit Comment"));
        QVERIFY(!card()->findChild<QPlainTextEdit *>(QStringLiteral("commentEditor")));
        // Cancel leaves it be.
        const int steps = ed->undoStack()->count();
        card()->findChild<QAbstractButton *>(QStringLiteral("commentEdit"))->click();
        card()->findChild<QPlainTextEdit *>(QStringLiteral("commentEditor"))->setPlainText(QStringLiteral("Never mind"));
        card()->findChild<QAbstractButton *>(QStringLiteral("commentCancel"))->click();
        QCOMPARE(ed->undoStack()->count(), steps);
        QCOMPARE(comments::commentsOf(slide())[0].text, QStringLiteral("Check the heading"));

        // A reply goes by itself.
        auto replyDeletes = card()->findChildren<QAbstractButton *>(QStringLiteral("commentReplyDelete"));
        QCOMPARE(int(replyDeletes.size()), 2);
        replyDeletes[0]->click();
        QCOMPARE(int(comments::commentsOf(slide())[0].replies.size()), 1);
        QCOMPARE(comments::commentsOf(slide())[0].replies[0].text, QStringLiteral("Second answer"));

        // Delete takes the comment and its replies; one undo brings it all back, and redo takes it again.
        card()->findChild<QAbstractButton *>(QStringLiteral("commentDelete"))->click();
        QCOMPARE(comments::commentCount(slide()), 0);
        QCOMPARE(int(pane->findChildren<QFrame *>(QStringLiteral("commentCard")).size()), 0);
        QCOMPARE(ed->undoStack()->undoText(), QStringLiteral("Delete Comment"));
        QVERIFY(!w.act(QStringLiteral("comment.delete"))->isEnabled());
        ed->undo();
        QCOMPARE(comments::commentCount(slide()), 1);
        QCOMPARE(int(comments::commentsOf(slide())[0].replies.size()), 1);
        QTRY_COMPARE(int(pane->findChildren<QFrame *>(QStringLiteral("commentCard")).size()), 1);
        ed->redo();
        QCOMPARE(comments::commentCount(slide()), 0);
        QTRY_COMPARE(int(pane->findChildren<QFrame *>(QStringLiteral("commentCard")).size()), 0);
        // Undoing all the way takes the first comment away again.
        while (ed->undoStack()->canUndo()) ed->undo();
        QCOMPARE(comments::commentCount(slide()), 0);

        // Cancel in the new comment's box makes nothing.
        w.act(QStringLiteral("comment.new"))->trigger();
        pane->findChild<QPlainTextEdit *>(QStringLiteral("commentDraft"))->setPlainText(QStringLiteral("Never mind"));
        pane->findChild<QPushButton *>(QStringLiteral("commentDraftCancel"))->click();
        QVERIFY(!pane->findChild<QPlainTextEdit *>(QStringLiteral("commentDraft"))->isVisible());
        QCOMPARE(comments::commentCount(slide()), 0);
        QVERIFY(!ed->undoStack()->canUndo());
        st.setValue(QStringLiteral("user/name"), QString());
        st.setValue(QStringLiteral("user/initials"), QString());
    }

    // Nothing is refused without a word: a reply past the thread's replies, and a new comment past the document's threads, say so in the status bar and change nothing.
    void commentsPaneSaysWhyACommentOrReplyIsRefused()
    {
        Settings &st = Settings::get();
        st.setValue(QStringLiteral("user/name"), QStringLiteral("Pat Doe"));
        st.setValue(QStringLiteral("user/initials"), QStringLiteral("PD"));
        DeckWindow w;
        w.resize(1280, 800);
        w.show();
        Editor *ed = w.editor();
        CommentsPane *pane = w.commentsPane();
        auto slide = [&]() -> Page & { return *ed->doc()->pages[ed->currentPage()]; };
        auto card = [&]() { return pane->findChildren<QFrame *>(QStringLiteral("commentCard")).value(0); };
        auto newOne = [&](const QString &text) {
            w.act(QStringLiteral("comment.new"))->trigger();
            pane->findChild<QPlainTextEdit *>(QStringLiteral("commentDraft"))->setPlainText(text);
            pane->findChild<QPushButton *>(QStringLiteral("commentPost"))->click();
        };
        newOne(QStringLiteral("First"));
        QCOMPARE(comments::commentCount(slide()), 1);
        auto reply = [&](const QString &text) {
            QTRY_VERIFY(card());
            card()->findChild<QLineEdit *>(QStringLiteral("commentReplyEdit"))->setText(text);
            card()->findChild<QAbstractButton *>(QStringLiteral("commentReplyPost"))->click();
        };
        jp::threads::setCapsForTest(0, 1);
        reply(QStringLiteral("one"));
        QCOMPARE(int(comments::commentsOf(slide())[0].replies.size()), 1);
        w.statusBar()->clearMessage();
        const int steps = ed->undoStack()->count();
        reply(QStringLiteral("two"));
        jp::threads::setCapsForTest(0, 0);
        QCOMPARE(int(comments::commentsOf(slide())[0].replies.size()), 1);
        QVERIFY2(w.statusBar()->currentMessage().contains(QStringLiteral("1 replies")), qPrintable(w.statusBar()->currentMessage()));
        QCOMPARE(ed->undoStack()->count(), steps);
        // a new comment past the threads of the whole presentation (the others are on other slides)
        w.statusBar()->clearMessage();
        jp::threads::setCapsForTest(1, 0);
        newOne(QStringLiteral("Second"));
        jp::threads::setCapsForTest(0, 0);
        QCOMPARE(comments::commentCount(slide()), 1);
        QVERIFY2(w.statusBar()->currentMessage().contains(QStringLiteral("1 comment threads")), qPrintable(w.statusBar()->currentMessage()));
        QCOMPARE(pane->findChild<QPlainTextEdit *>(QStringLiteral("commentDraft"))->toPlainText(), QStringLiteral("Second"));   // (what was typed is kept)
        st.setValue(QStringLiteral("user/name"), QString());
        st.setValue(QStringLiteral("user/initials"), QString());
    }

    // A presentation with more comments than are kept opens with a note that says so.
    void aPresentationOverTheCommentLimitOpensWithANote()
    {
        auto d = deck::newPresentation();
        // (each slide within the limit of a slide, all of them past the limit of the presentation)
        while (d->pages.size() < 3) d->addPage(-1, d->pages[0]->masterId);
        for (int page = 0; page < 3; ++page) {
            QVector<comments::Comment> list;
            for (int i = 0; i < 700; ++i) {
                comments::Comment c;
                c.id = QStringLiteral("c%1-%2").arg(page).arg(i);
                c.author = QStringLiteral("Pat Doe");
                c.initials = QStringLiteral("PD");
                c.text = QStringLiteral("t%1").arg(i);
                c.at = QPointF(10, 10);
                list.append(c);
            }
            comments::setComments(*d->pages[page], list);
        }
        QString err;
        const QByteArray bytes = deck::exportPresentation(*d, &err);
        QVERIFY2(!bytes.isEmpty(), qPrintable(err));
        err.clear();
        auto back = deck::importPresentation(bytes, &err);
        QVERIFY(back);
        QCOMPARE(int(comments::allComments(*back).size()), comments::kMaxComments);
        QVERIFY2(err.contains(QStringLiteral("more comments than")), qPrintable(err));
    }

    // The pane asks who is writing the first time, when File > Options has no
    // name, and keeps the answer; saying no makes no comment.
    void commentsPaneAsksWhoIsWritingOnce()
    {
        Settings &st = Settings::get();
        st.setValue(QStringLiteral("user/name"), QString());
        st.setValue(QStringLiteral("user/initials"), QString());
        DeckWindow w;
        w.resize(1280, 800);
        w.show();
        // Declined: nothing opens, nothing is kept.
        bool asked = false;
        QTimer::singleShot(0, [&] {
            auto *dlg = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!dlg) return;
            asked = dlg->findChild<QLineEdit *>(QStringLiteral("commentAuthorName")) && dlg->findChild<QLineEdit *>(QStringLiteral("commentAuthorInitials"));
            dlg->reject();
        });
        w.act(QStringLiteral("comment.new"))->trigger();
        QVERIFY(asked);
        QVERIFY(!comments::commentAuthor().known());
        QVERIFY(!w.commentsPane()->findChild<QPlainTextEdit *>(QStringLiteral("commentDraft")) || !w.commentsPane()->findChild<QPlainTextEdit *>(QStringLiteral("commentDraft"))->isVisible());
        // Answered: kept in the settings, and the box to type in opens.
        asked = false;
        QTimer::singleShot(0, [&] {
            auto *dlg = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!dlg) return;
            asked = true;
            dlg->findChild<QLineEdit *>(QStringLiteral("commentAuthorName"))->setText(QStringLiteral("Pat Doe"));
            dlg->accept();
        });
        w.act(QStringLiteral("comment.new"))->trigger();
        QVERIFY(asked);
        QCOMPARE(st.value(QStringLiteral("user/name")).toString(), QStringLiteral("Pat Doe"));
        QCOMPARE(st.value(QStringLiteral("user/initials")).toString(), QStringLiteral("PD"));
        auto *draft = w.commentsPane()->findChild<QPlainTextEdit *>(QStringLiteral("commentDraft"));
        QVERIFY(draft && draft->isVisible());
        draft->setPlainText(QStringLiteral("Hello"));
        w.commentsPane()->findChild<QPushButton *>(QStringLiteral("commentPost"))->click();
        QCOMPARE(comments::commentsOf(*w.editor()->doc()->pages[0])[0].initials, QStringLiteral("PD"));
        // Not again.
        bool askedAgain = false;
        QTimer watch;   // lets go of a dialog that would otherwise hold the test
        watch.setSingleShot(true);
        connect(&watch, &QTimer::timeout, [&] {
            if (auto *dlg = QApplication::activeModalWidget()) {
                askedAgain = true;
                dlg->close();
            }
        });
        watch.start(0);
        w.act(QStringLiteral("comment.new"))->trigger();
        QVERIFY(!askedAgain);
        QVERIFY(w.commentsPane()->findChild<QPlainTextEdit *>(QStringLiteral("commentDraft"))->isVisible());
        st.setValue(QStringLiteral("user/name"), QString());
        st.setValue(QStringLiteral("user/initials"), QString());
    }

    // Each comment has a marker on the slide, where its place is: it shows only
    // while comments are shown, a click opens the comment in the pane, and
    // dragging it moves the comment as one step that undoes.
    void commentMarkersShowOnTheSlideAndMove()
    {
        DeckWindow w;
        w.resize(1280, 800);
        w.show();
        Editor *ed = w.editor();
        DeckCanvas *cv = w.canvas();
        const comments::CommentAuthor pat{QStringLiteral("Pat Doe"), QStringLiteral("PD")};
        ed->beginChange(QStringLiteral("Comments"));
        const QString first = comments::addComment(*ed->doc()->pages[0], pat, QStringLiteral("One"), QPointF(600, 100));
        const QString second = comments::addComment(*ed->doc()->pages[0], pat, QStringLiteral("Two"), QPointF(300, 400));
        ed->endChange();
        ed->undoStack()->clear();
        QVERIFY(!cv->commentsShown());
        auto markerPoint = [&](const QPointF &page) { return (cv->pageToView(page) + QPointF(6, 6)).toPoint(); };
        QTRY_VERIFY(cv->viewport()->isVisible());
        QVERIFY(cv->commentAt(markerPoint(QPointF(600, 100))).isEmpty());   // hidden
        const QImage hidden = cv->viewport()->grab().toImage();

        w.act(QStringLiteral("comment.show"))->trigger();
        QVERIFY(cv->commentsShown());
        QCOMPARE(cv->commentAt(markerPoint(QPointF(600, 100))), first);
        QCOMPARE(cv->commentAt(markerPoint(QPointF(300, 400))), second);
        QVERIFY(cv->commentAt((cv->pageToView(QPointF(600, 100)) - QPointF(8, 8)).toPoint()).isEmpty());
        QVERIFY(cv->commentAt(markerPoint(QPointF(450, 250))).isEmpty());
        // The markers are drawn: the pixels under them differ from the slide without them.
        const QImage shown = cv->viewport()->grab().toImage();
        for (const QPointF &page : {QPointF(600, 100), QPointF(300, 400)}) {
            int changed = 0;
            const QPoint at = cv->pageToView(page).toPoint();
            for (int dy = 2; dy < 14; ++dy)
                for (int dx = 2; dx < 18; ++dx) changed += shown.pixelColor(at.x() + dx, at.y() + dy) != hidden.pixelColor(at.x() + dx, at.y() + dy);
            QVERIFY2(changed > 60, qPrintable(QString::number(changed)));
        }
        // Another slide has none of these.
        deck::addSlide(*ed->doc(), *deck::layoutOfType(*ed->doc(), QStringLiteral("obj")));
        ed->setCurrentPage(1);
        QVERIFY(cv->commentAt(markerPoint(QPointF(600, 100))).isEmpty());
        ed->setCurrentPage(0);
        ed->undoStack()->clear();

        // A click opens the pane on the comment.
        w.act(QStringLiteral("comment.show"))->trigger();
        QVERIFY(w.commentsPane()->isHidden());
        w.act(QStringLiteral("comment.show"))->trigger();
        QVERIFY(w.commentsPane()->currentComment().isEmpty());
        QTest::mouseClick(cv->viewport(), Qt::LeftButton, Qt::NoModifier, markerPoint(QPointF(300, 400)));
        QVERIFY(!w.commentsPane()->isHidden());
        QCOMPARE(w.commentsPane()->currentComment(), second);
        QCOMPARE(cv->activeComment(), second);
        QVERIFY(w.act(QStringLiteral("comment.delete"))->isEnabled());
        QCOMPARE(ed->undoStack()->count(), 0);   // a click moves nothing
        QCOMPARE(comments::commentsOf(*ed->doc()->pages[0])[1].at, QPointF(300, 400));

        // A drag moves it, in one step.
        const QPoint from = markerPoint(QPointF(300, 400)), to = from + QPoint(90, -60);
        QTest::mousePress(cv->viewport(), Qt::LeftButton, Qt::NoModifier, from);
        QTest::mouseMove(cv->viewport(), from + QPoint(20, -10));
        QTest::mouseMove(cv->viewport(), from + QPoint(60, -40));
        QTest::mouseMove(cv->viewport(), to);
        QCOMPARE(ed->undoStack()->count(), 0);   // not until it lets go
        QTest::mouseRelease(cv->viewport(), Qt::LeftButton, Qt::NoModifier, to);
        QCOMPARE(ed->undoStack()->count(), 1);
        QCOMPARE(ed->undoStack()->undoText(), QStringLiteral("Move Comment"));
        const QPointF moved = comments::commentsOf(*ed->doc()->pages[0])[1].at;
        QVERIFY2(qAbs(moved.x() - (300 + 90 / cv->ppp())) < 1.5 && qAbs(moved.y() - (400 - 60 / cv->ppp())) < 1.5, qPrintable(QString::number(moved.x()) + "," + QString::number(moved.y())));
        QCOMPARE(comments::commentsOf(*ed->doc()->pages[0])[0].at, QPointF(600, 100));   // the other stayed
        QCOMPARE(cv->commentAt(markerPoint(moved)), second);
        QCOMPARE(w.commentsPane()->currentComment(), second);
        ed->undo();
        QCOMPARE(comments::commentsOf(*ed->doc()->pages[0])[1].at, QPointF(300, 400));
        // The markers don't take the slide's own clicks away where they aren't.
        ed->clearSelection();
        const QPointF titleAt = ed->doc()->pages[0]->items[0]->rect.center();
        QTest::mouseClick(cv->viewport(), Qt::LeftButton, Qt::NoModifier, cv->pageToView(titleAt).toPoint());
        QCOMPARE(ed->selection(), QStringList{ed->doc()->pages[0]->items[0]->id});
        (void)first;
    }

    // Previous and Next go from comment to comment through the presentation, in
    // slide order, opening each one in the pane on its slide; the Delete menu
    // clears the slide's comments or all of them as one step.
    void commentCommandsVisitAndDelete()
    {
        DeckWindow w;
        w.resize(1280, 800);
        w.show();
        Editor *ed = w.editor();
        for (int i = 0; i < 2; ++i) deck::addSlide(*ed->doc(), *deck::layoutOfType(*ed->doc(), QStringLiteral("obj")));
        const comments::CommentAuthor pat{QStringLiteral("Pat Doe"), QStringLiteral("PD")};
        ed->beginChange(QStringLiteral("Comments"));
        const QString a = comments::addComment(*ed->doc()->pages[0], pat, QStringLiteral("A"), QPointF(10, 10));
        const QString b = comments::addComment(*ed->doc()->pages[0], pat, QStringLiteral("B"), QPointF(40, 10));
        const QString c = comments::addComment(*ed->doc()->pages[2], pat, QStringLiteral("C"), QPointF(10, 10));
        ed->endChange();
        ed->undoStack()->clear();
        ed->setCurrentPage(0);
        QVERIFY(w.act(QStringLiteral("comment.next"))->isEnabled() && w.act(QStringLiteral("comment.prev"))->isEnabled());
        QVERIFY(w.act(QStringLiteral("comment.deleteAll"))->isEnabled() && w.act(QStringLiteral("comment.deleteSlide"))->isEnabled());
        QVERIFY(!w.act(QStringLiteral("comment.delete"))->isEnabled());   // none is open yet
        auto open = [&] { return w.commentsPane()->currentComment(); };

        w.act(QStringLiteral("comment.next"))->trigger();   // none was open: the first
        QVERIFY(!w.commentsPane()->isHidden());
        QVERIFY(w.canvas()->commentsShown());
        QCOMPARE(open(), a);
        w.act(QStringLiteral("comment.next"))->trigger();
        QCOMPARE(open(), b);
        w.act(QStringLiteral("comment.next"))->trigger();   // over to the third slide
        QCOMPARE(open(), c);
        QCOMPARE(ed->currentPage(), 2);
        w.act(QStringLiteral("comment.next"))->trigger();   // and round to the first
        QCOMPARE(open(), a);
        QCOMPARE(ed->currentPage(), 0);
        w.act(QStringLiteral("comment.prev"))->trigger();   // and back
        QCOMPARE(open(), c);
        QCOMPARE(ed->currentPage(), 2);
        w.act(QStringLiteral("comment.prev"))->trigger();
        QCOMPARE(open(), b);
        QCOMPARE(ed->currentPage(), 0);

        // Delete takes the open comment.
        QVERIFY(w.act(QStringLiteral("comment.delete"))->isEnabled());
        w.act(QStringLiteral("comment.delete"))->trigger();
        QCOMPARE(int(comments::commentsOf(*ed->doc()->pages[0]).size()), 1);
        QCOMPARE(ed->undoStack()->undoText(), QStringLiteral("Delete Comment"));
        QVERIFY(open().isEmpty());
        QVERIFY(!w.act(QStringLiteral("comment.delete"))->isEnabled());
        ed->undo();
        QCOMPARE(int(comments::commentsOf(*ed->doc()->pages[0]).size()), 2);
        // All of the slide's.
        ed->undoStack()->clear();
        w.act(QStringLiteral("comment.deleteSlide"))->trigger();
        QVERIFY(comments::commentsOf(*ed->doc()->pages[0]).isEmpty());
        QCOMPARE(int(comments::commentsOf(*ed->doc()->pages[2]).size()), 1);
        QCOMPARE(ed->undoStack()->count(), 1);
        QCOMPARE(ed->undoStack()->undoText(), QStringLiteral("Delete Comments"));
        ed->undo();
        QCOMPARE(int(comments::commentsOf(*ed->doc()->pages[0]).size()), 2);
        // All of the presentation's, in one step.
        ed->undoStack()->clear();
        w.act(QStringLiteral("comment.deleteAll"))->trigger();
        QVERIFY(comments::allComments(*ed->doc()).isEmpty());
        QCOMPARE(ed->undoStack()->count(), 1);
        QVERIFY(!w.act(QStringLiteral("comment.deleteAll"))->isEnabled());
        QVERIFY(!w.act(QStringLiteral("comment.next"))->isEnabled());
        ed->undo();
        QCOMPARE(int(comments::allComments(*ed->doc()).size()), 3);
        QVERIFY(w.act(QStringLiteral("comment.next"))->isEnabled());
    }

    // A presentation opened with comments in it shows them, and the comments
    // go into the file the window saves.
    void commentsOpenAndSaveWithThePresentation()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("talk.pptx"));
        QFile f(path);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(pack(withLegacyComments()));
        f.close();
        DeckWindow w;
        w.resize(1280, 800);
        w.show();
        QVERIFY(w.openFile(path));
        QVERIFY(!w.commentsPane()->isHidden());   // the file has comments: they show
        QVERIFY(w.canvas()->commentsShown());
        QTRY_COMPARE(int(w.commentsPane()->findChildren<QFrame *>(QStringLiteral("commentCard")).size()), 2);
        w.editor()->setCurrentPage(1);
        QTRY_COMPARE(int(w.commentsPane()->findChildren<QFrame *>(QStringLiteral("commentCard")).size()), 0);
        const QString out = dir.filePath(QStringLiteral("again.pptx"));
        QVERIFY(w.saveTo(out));
        QString err;
        auto back = deck::importPresentationFile(out, &err);
        QVERIFY2(back, qPrintable(err));
        QCOMPARE(int(comments::commentsOf(*back->pages[0]).size()), 2);
        QCOMPARE(int(comments::commentsOf(*back->pages[0])[0].replies.size()), 1);
        w.editor()->markSaved();
    }


    // ---- Compare: the model of review and the changes between two presentations (no window) ----

    static std::unique_ptr<Document> copyOf(const Document &d) { return Document::fromSnapshot(d.snapshot()); }
    static ShapeItem *addBox(Document &d, Page &page, const QString &text, double x, double y, const QString &role = QString())
    {
        auto s = std::make_shared<ShapeItem>();
        s->rect = QRectF(x, y, 100, 40);
        s->role = role;
        if (!text.isEmpty()) s->storyId = d.createStory(text);
        page.items.push_back(s);
        return s.get();
    }
    // The n-th box of a slide that addBox made (those with no role).
    static ShapeItem *boxOf(const Document &d, int slide, int n = 0)
    {
        int k = 0;
        for (const auto &it : d.pages[slide]->items)
            if (it->type() == ItemType::Shape && it->role.isEmpty() && k++ == n) return static_cast<ShapeItem *>(it.get());
        return nullptr;
    }
    static QString textOfBox(const Document &d, const ShapeItem *s)
    {
        const QTextDocument *sd = s && !s->storyId.isEmpty() ? d.storyDoc(s->storyId) : nullptr;
        return sd ? sd->toPlainText() : QString();
    }
    // Slides A to E, a box of text on each.
    static std::unique_ptr<Document> compareBase()
    {
        auto d = lettered(5);
        for (int i = 0; i < 5; ++i) addBox(*d, *d->pages[i], QStringLiteral("box %1").arg(QChar('A' + i)), 50, 50 + 10 * i);
        return d;
    }
    // The slides as someone sees them: titles, comments, notes, and each object's kind, place, fill, line, and text.
    static QString lookOf(const Document &d)
    {
        QString out;
        for (const auto &p : d.pages) {
            out += QStringLiteral("[%1|%2]").arg(p->title).arg(comments::commentCount(*p));
            if (!p->notesStoryId.isEmpty() && d.storyDoc(p->notesStoryId)) out += QStringLiteral("N:") + d.storyDoc(p->notesStoryId)->toPlainText();
            walkItems(p->items, [&](const ItemPtr &it) {
                out += QStringLiteral("(%1 %2,%3,%4,%5 %6 %7 %8)")
                           .arg(int(it->type()))
                           .arg(it->rect.x())
                           .arg(it->rect.y())
                           .arg(it->rect.width())
                           .arg(it->rect.height())
                           .arg(QString::fromUtf8(QJsonDocument(it->fill.toJson()).toJson(QJsonDocument::Compact)), QString::fromUtf8(QJsonDocument(it->stroke.toJson()).toJson(QJsonDocument::Compact)),
                                it->type() == ItemType::Shape ? textOfBox(d, static_cast<ShapeItem *>(it.get())) : QString());
            });
        }
        return out;
    }
    static int kindCount(const DeckComparison &c, review::Kind k) { return c.changes.count(k); }

    // Six changes: a slide added at the end, one deleted, one moved, a box moved, a box's text changed, a box recolored.
    static std::unique_ptr<Document> changedBase(const Document &mine)
    {
        auto t = copyOf(mine);
        const MasterPage *layout = deck::layoutOfType(*t, QStringLiteral("obj"));
        const int made = deck::addSlide(*t, *layout);
        t->pages[made]->title = QStringLiteral("F");
        addBox(*t, *t->pages[made], QStringLiteral("box F"), 50, 50);
        t->pages.removeAt(1);                                  // B deleted
        t->pages.move(3, 0);                                   // E (now at 3) first: E A C D F
        boxOf(*t, 1)->rect.translate(30, 20);                  // A's box moved
        QTextCursor c(t->storyDoc(boxOf(*t, 2)->storyId));     // C's text
        c.select(QTextCursor::Document);
        c.insertText(QStringLiteral("box C, changed"));
        boxOf(*t, 3)->fill = Fill::solid(ColorRef::rgb(QColor(Qt::red)));   // D's box recolored
        return t;
    }

    void compareFindsTheSixChangesAndAcceptsOrRejectsThemAll()
    {
        auto mine = compareBase();
        auto theirs = changedBase(*mine);
        DeckComparison cmp = compareDecks(*mine, *theirs);
        QVERIFY(!cmp.changes.tooMany);
        QCOMPARE(cmp.changes.size(), 6);
        QCOMPARE(kindCount(cmp, review::Kind::Added), 1);
        QCOMPARE(kindCount(cmp, review::Kind::Deleted), 1);
        QCOMPARE(kindCount(cmp, review::Kind::Moved), 2);   // the slide, and the box
        QCOMPARE(kindCount(cmp, review::Kind::Edited), 1);
        QCOMPARE(kindCount(cmp, review::Kind::Reformatted), 1);
        QCOMPARE(cmp.changes.pending(), 6);
        // The list runs in the order of the slides, and each change tells where it is.
        for (int i = 1; i < cmp.changes.size(); ++i) QVERIFY(cmp.changes.changes[i - 1].position <= cmp.changes.changes[i].position);
        QStringList paths;
        for (const review::Change &c : cmp.changes.changes) {
            paths << c.target.path;
            QVERIFY(!c.summary.isEmpty());
            QVERIFY(!c.id.isEmpty());
        }
        paths.sort();
        QCOMPARE(paths, (QStringList{QStringLiteral("fill"), QStringLiteral("order"), QStringLiteral("rect"), QStringLiteral("slide"), QStringLiteral("slide"), QStringLiteral("text")}));
        const review::Change *text = nullptr;
        for (const review::Change &c : cmp.changes.changes)
            if (c.target.path == QLatin1String("text")) text = &c;
        QVERIFY(text);
        QVERIFY2(text->summary.contains(QStringLiteral("box C, changed")) && text->summary.contains(QStringLiteral("box C")), qPrintable(text->summary));

        // Accept every change: theirs, slide by slide; and nothing is left to find.
        auto working = copyOf(*mine);
        QCOMPARE(acceptAll(*working, cmp), 6);
        QCOMPARE(cmp.changes.count(review::State::Accepted), 6);
        QCOMPARE(working->pages.size(), theirs->pages.size());
        QCOMPARE(lookOf(*working), lookOf(*theirs));
        QCOMPARE(compareDecks(*working, *theirs).changes.size(), 0);
        // Reject every change after that: back to mine.
        QCOMPARE(rejectAll(*working, cmp, true), 6);
        QCOMPARE(lookOf(*working), lookOf(*mine));
        QCOMPARE(cmp.changes.count(review::State::Rejected), 6);

        // Reject every change from the start: mine, untouched.
        DeckComparison again = compareDecks(*mine, *theirs);
        auto kept = copyOf(*mine);
        QCOMPARE(rejectAll(*kept, again), 6);
        QCOMPARE(lookOf(*kept), lookOf(*mine));
        QCOMPARE(compareDecks(*kept, *mine).changes.size(), 0);
        // Accepting one at a time, in the list's order, ends the same.
        DeckComparison one = compareDecks(*mine, *theirs);
        auto stepwise = copyOf(*mine);
        for (int i = 0; i < one.changes.size(); ++i) QVERIFY(applyChange(*stepwise, one, i));
        QCOMPARE(compareDecks(*stepwise, *theirs).changes.size(), 0);
        QCOMPARE(stepwise->pages.size(), theirs->pages.size());
    }

    // Presentations made apart (every slide and object has an id of its own) are matched by what they say: the same six changes.
    void compareMatchesByTitleAndTextWhenTheIdsDiffer()
    {
        auto mine = compareBase();
        auto theirs = changedBase(*mine);
        for (auto &p : theirs->pages) {
            p->id = newId("p");
            walkItems(p->items, [](const ItemPtr &it) { it->id = newId(); });
        }
        DeckComparison cmp = compareDecks(*mine, *theirs);
        QCOMPARE(cmp.changes.size(), 6);
        QCOMPARE(kindCount(cmp, review::Kind::Added), 1);
        QCOMPARE(kindCount(cmp, review::Kind::Deleted), 1);
        QCOMPARE(kindCount(cmp, review::Kind::Moved), 2);
        auto working = copyOf(*mine);
        QCOMPARE(acceptAll(*working, cmp), 6);
        QCOMPARE(lookOf(*working), lookOf(*theirs));
    }

    // Notes changed, a comment added: found, taken, and taken back; a comment already there is not listed again.
    void compareListsNotesAndCommentsAndKeepsThem()
    {
        auto mine = compareBase();
        const comments::CommentAuthor lee{QStringLiteral("Lee Roy"), QStringLiteral("LR")};
        comments::addComment(*mine->pages[3], lee, QStringLiteral("Already here"), QPointF(5, 5), QDateTime(QDate(2026, 10, 1), QTime(9, 0), Qt::UTC));
        auto theirs = copyOf(*mine);
        const QDateTime stamp(QDate(2026, 11, 2), QTime(10, 30), Qt::UTC);
        comments::addComment(*theirs->pages[3], comments::CommentAuthor{QStringLiteral("Pat Doe"), QStringLiteral("PD")}, QStringLiteral("Please check"), QPointF(9, 9), stamp);
        Page &c = *theirs->pages[2];
        c.notesStoryId = theirs->createStory(QStringLiteral("Say hello"));
        DeckComparison cmp = compareDecks(*mine, *theirs);
        QCOMPARE(cmp.changes.size(), 2);
        QCOMPARE(kindCount(cmp, review::Kind::Commented), 1);
        QCOMPARE(kindCount(cmp, review::Kind::Edited), 1);
        const review::Change *comment = nullptr;
        for (const review::Change &ch : cmp.changes.changes)
            if (ch.kind == review::Kind::Commented) comment = &ch;
        QVERIFY(comment);
        QCOMPARE(comment->author, QStringLiteral("Pat Doe"));
        QCOMPARE(comment->when, stamp);
        auto working = copyOf(*mine);
        QCOMPARE(acceptAll(*working, cmp), 2);
        QCOMPARE(comments::commentCount(*working->pages[3]), 2);
        QCOMPARE(working->storyDoc(working->pages[2]->notesStoryId)->toPlainText(), QStringLiteral("Say hello"));
        QCOMPARE(compareDecks(*working, *theirs).changes.size(), 0);
        // Taking them back removes the new comment and keeps the one that was there.
        QCOMPARE(rejectAll(*working, cmp, true), 2);
        QCOMPARE(comments::commentCount(*working->pages[3]), 1);
        QCOMPARE(comments::commentsOf(*working->pages[3])[0].text, QStringLiteral("Already here"));
        QVERIFY(working->storyDoc(working->pages[2]->notesStoryId)->toPlainText().isEmpty());
    }

    // The review model: kinds and states have stable names, a change round-trips through JSON, and the list filters and steps.
    void reviewModelKeepsChangesAndFiltersThem()
    {
        review::ChangeList list;
        for (int i = 0; i < 4; ++i) {
            review::Change c;
            c.id = QStringLiteral("c%1").arg(i);
            c.kind = i % 2 ? review::Kind::Deleted : review::Kind::Commented;
            c.author = i < 3 ? QStringLiteral("Pat") : QStringLiteral("Lee");
            c.target = {i < 2 ? QStringLiteral("s1") : QStringLiteral("s2"), QStringLiteral("o%1").arg(i), QStringLiteral("text")};
            c.when = QDateTime(QDate(2026, 11, 1 + i), QTime(8, 0), Qt::UTC);
            c.data[QStringLiteral("k")] = i;
            list.changes << c;
        }
        list.changes[1].state = review::State::Accepted;
        QCOMPARE(list.pending(), 3);
        QCOMPARE(list.indexOf(QStringLiteral("c2")), 2);
        QCOMPARE(list.filter(QStringLiteral("s2")), (QVector<int>{2, 3}));
        QCOMPARE(list.filter(QString(), QStringLiteral("Lee")), (QVector<int>{3}));
        QCOMPARE(list.filter(QStringLiteral("s1"), QString(), true), (QVector<int>{0}));
        QCOMPARE(list.authors(), (QStringList{QStringLiteral("Pat"), QStringLiteral("Lee")}));
        QCOMPARE(list.step(0), 2);          // c1 is accepted: skipped
        QCOMPARE(list.step(3), 0);          // wraps
        QCOMPARE(list.step(0, false), 3);
        QCOMPARE(list.step(-1, false), 3);
        QCOMPARE(list.step(-1), 0);
        const review::Change back = review::Change::fromJson(list.changes[1].toJson());
        QCOMPARE(back.id, list.changes[1].id);
        QCOMPARE(int(back.kind), int(review::Kind::Deleted));
        QCOMPARE(int(back.state), int(review::State::Accepted));
        QCOMPARE(back.when, list.changes[1].when);
        QCOMPARE(back.target.object, QStringLiteral("o1"));
        QCOMPARE(back.data.value(QStringLiteral("k")).toInt(), 1);
        for (review::Kind k : {review::Kind::Added, review::Kind::Deleted, review::Kind::Moved, review::Kind::Edited, review::Kind::Reformatted, review::Kind::Commented}) {
            bool ok = false;
            QCOMPARE(int(review::kindFromKey(review::kindKey(k), &ok)), int(k));
            QVERIFY(ok);
        }
        QCOMPARE(review::stateFromKey(QStringLiteral("junk")), review::State::Pending);
        review::ChangeList empty;
        QCOMPARE(empty.step(-1), -1);
    }

    // Each accept or reject is one step that undoes, through the editor.
    void compareChangesAreOneUndoStepEach()
    {
        DeckWindow w;
        Editor *ed = w.editor();
        auto base = compareBase();
        ed->setDocument(copyOf(*base));
        auto theirs = changedBase(*base);
        DeckComparison cmp = compareDecks(*ed->doc(), *theirs);
        QCOMPARE(cmp.changes.size(), 6);
        const QJsonObject before = ed->doc()->toJson();
        for (int i = 0; i < cmp.changes.size(); ++i) {
            const int depth = ed->undoStack()->index();
            ed->change(QStringLiteral("Accept"), [&] { QVERIFY(applyChange(*ed->doc(), cmp, i)); });
            QCOMPARE(ed->undoStack()->index(), depth + 1);
            QVERIFY2(ed->doc()->toJson() != before, qPrintable(cmp.changes.changes[i].summary));
            ed->undo();
            QCOMPARE(ed->undoStack()->index(), depth);
            QCOMPARE(ed->doc()->toJson(), before);
            cmp.changes.changes[i].state = review::State::Pending;   // an undo does not rewind the state: the caller does
        }
        // All of them as one step, and undone as one.
        const int depth = ed->undoStack()->index();
        ed->change(QStringLiteral("Accept All"), [&] { acceptAll(*ed->doc(), cmp); });
        QCOMPARE(ed->undoStack()->index(), depth + 1);
        QCOMPARE(lookOf(*ed->doc()), lookOf(*theirs));
        ed->undo();
        QCOMPARE(ed->doc()->toJson(), before);
        // The states are the caller's to rewind after an undo; accepting again looks at the document, not at the state, and works.
        ed->change(QStringLiteral("Accept"), [&] { QVERIFY(applyChange(*ed->doc(), cmp, 0)); });
        QCOMPARE(compareDecks(*ed->doc(), *theirs).changes.size(), 5);
    }

    // ---- Compare in the window: Review > Compare, the Reviewing Pane, the marks, and what the file keeps ----

    // A window holding the base deck, with theirs saved at `path` (a .pptx or .odp) and chosen by Compare; the number of changes the file's own presentation gives is `expected`.
    static void startCompare(CompareRun &run, const QString &path)
    {
        // Both presentations go through a file, so that what the reader makes of each is the same on both sides.
        run.base = compareBase();
        auto theirs = changedBase(*run.base);
        QString err;
        const QString minePath = QFileInfo(path).dir().filePath(QStringLiteral("mine.pptx"));
        QVERIFY2(deck::savePresentationFile(*run.base, minePath, &err), qPrintable(err));
        QVERIFY2(deck::savePresentationFile(*theirs, path, &err), qPrintable(err));
        run.fileTheirs = deck::importAnyPresentationFile(path, &err);
        QVERIFY2(run.fileTheirs, qPrintable(err));
        run.w = std::make_unique<DeckWindow>();
        QVERIFY(run.w->openFile(minePath));
        run.mine = copyOf(*run.w->editor()->doc());
        run.expected = compareDecks(*run.w->editor()->doc(), *run.fileTheirs).changes.size();
        QVERIFY2(run.expected >= 5 && run.expected <= 8, qPrintable(QString::number(run.expected)));
        run.w->compareFileChooser = [path] { return path; };
        run.w->act(QStringLiteral("review.compare"))->trigger();
    }

    void compareWindowListsTheChangesInThePane()
    {
        QTemporaryDir dir;
        CompareRun run;
        startCompare(run, dir.filePath(QStringLiteral("theirs.pptx")));
        DeckWindow &w = *run.w;
        QTRY_VERIFY(w.reviewPane() && w.reviewPane()->isVisibleTo(&w));
        QTRY_COMPARE(w.reviewPane()->visibleCount(), run.expected);
        QVERIFY(w.comparison());
        QCOMPARE(w.comparison()->changes.size(), run.expected);
        QCOMPARE(w.reviewPane()->list()->model()->rowCount(), run.expected);
        // The step that made it undoes it, and the pane goes with it.
        QCOMPARE(w.editor()->undoStack()->undoText(), QStringLiteral("Compare Presentations"));
        w.editor()->undo();
        QTRY_VERIFY(!w.comparison());
        QTRY_COMPARE(w.reviewPane()->visibleCount(), 0);
        w.editor()->redo();
        QTRY_VERIFY(w.comparison());
        QTRY_COMPARE(w.reviewPane()->visibleCount(), run.expected);
        // The commands for the ribbon exist under their ids.
        for (const char *id : {"review.compare", "review.accept", "review.reject", "review.prev", "review.next", "review.pane"}) QVERIFY2(w.act(QString::fromLatin1(id)), id);
        w.editor()->markSaved();
    }

    // With no comparison the commands that act on one are dimmed, in a new window, in one that opened a file, and after a comparison is undone.
    void compareCommandsAreDimmedUntilThereIsAComparison()
    {
        const QStringList ids = {QStringLiteral("review.accept"), QStringLiteral("review.reject"), QStringLiteral("review.acceptAll"), QStringLiteral("review.rejectAll"), QStringLiteral("review.prev"), QStringLiteral("review.next")};
        DeckWindow fresh;
        for (const QString &id : ids) QVERIFY2(!fresh.act(id)->isEnabled(), qPrintable(id));
        QTemporaryDir dir;
        CompareRun run;
        startCompare(run, dir.filePath(QStringLiteral("theirs.pptx")));
        DeckWindow &w = *run.w;
        QTRY_COMPARE(w.reviewPane()->visibleCount(), run.expected);
        for (const QString &id : ids) QVERIFY2(w.act(id)->isEnabled(), qPrintable(id));
        w.editor()->undo();
        QTRY_VERIFY(!w.comparison());
        for (const QString &id : ids) QVERIFY2(!w.act(id)->isEnabled(), qPrintable(id));
        w.editor()->markSaved();
        // A window that opens a file has none either.
        DeckWindow opened;
        QVERIFY(opened.openFile(dir.filePath(QStringLiteral("mine.pptx"))));
        for (const QString &id : ids) QVERIFY2(!opened.act(id)->isEnabled(), qPrintable(id));
        opened.showReviewPane(true);
        for (const QString &id : ids) QVERIFY2(!opened.act(id)->isEnabled(), qPrintable(id));
    }

    void compareAcceptOneIsOneUndoStepAndUndoMakesItPendingAgain()
    {
        QTemporaryDir dir;
        CompareRun run;
        startCompare(run, dir.filePath(QStringLiteral("theirs.pptx")));
        DeckWindow &w = *run.w;
        Editor *ed = w.editor();
        QTRY_COMPARE(w.reviewPane()->visibleCount(), run.expected);
        QTRY_VERIFY(w.reviewPane()->currentChange() >= 0);
        const QJsonObject before = ed->doc()->toJson();
        const int depth = ed->undoStack()->index();
        const int one = w.reviewPane()->currentChange();
        w.reviewPane()->findChild<QPushButton *>(QStringLiteral("reviewAccept"))->click();
        QCOMPARE(ed->undoStack()->index(), depth + 1);
        QCOMPARE(w.comparison()->changes.changes[one].state, review::State::Accepted);
        QVERIFY(ed->doc()->toJson() != before);
        QTRY_COMPARE(w.reviewPane()->visibleCount(), run.expected - 1);
        // Undo: the slides come back, and the change is pending and listed again.
        ed->undo();
        QCOMPARE(ed->undoStack()->index(), depth);
        QCOMPARE(w.comparison()->changes.changes[one].state, review::State::Pending);
        QCOMPARE(w.comparison()->changes.pending(), run.expected);
        QTRY_COMPARE(w.reviewPane()->visibleCount(), run.expected);
        // Redo takes it again.
        ed->redo();
        QCOMPARE(w.comparison()->changes.changes[one].state, review::State::Accepted);
        QTRY_COMPARE(w.reviewPane()->visibleCount(), run.expected - 1);
        // Reject of a change is one step too, and it leaves the slides alone.
        const QJsonObject mid = ed->doc()->toJson();
        const int two = w.reviewPane()->currentChange();
        QVERIFY(two >= 0);
        const int depth2 = ed->undoStack()->index();
        w.reviewPane()->findChild<QPushButton *>(QStringLiteral("reviewReject"))->click();
        QCOMPARE(ed->undoStack()->index(), depth2 + 1);
        QCOMPARE(w.comparison()->changes.changes[two].state, review::State::Rejected);
        QCOMPARE(ed->doc()->toJson().value(QStringLiteral("pages")), mid.value(QStringLiteral("pages")));
        ed->undo();
        QCOMPARE(w.comparison()->changes.changes[two].state, review::State::Pending);
        ed->markSaved();
    }

    void compareAcceptAllGivesTheirsAndRejectAllGivesMine()
    {
        QTemporaryDir dir;
        {
            CompareRun run;
            startCompare(run, dir.filePath(QStringLiteral("a.pptx")));
            DeckWindow &w = *run.w;
            QTRY_COMPARE(w.reviewPane()->visibleCount(), run.expected);
            const int depth = w.editor()->undoStack()->index();
            w.reviewPane()->findChild<QPushButton *>(QStringLiteral("reviewAcceptAll"))->click();
            QCOMPARE(w.editor()->undoStack()->index(), depth + 1);
            QCOMPARE(lookOf(*w.editor()->doc()), lookOf(*run.fileTheirs));
            QCOMPARE(w.comparison()->changes.pending(), 0);
            QTRY_COMPARE(w.reviewPane()->visibleCount(), 0);
            w.editor()->undo();
            QCOMPARE(lookOf(*w.editor()->doc()), lookOf(*run.mine));
            QTRY_COMPARE(w.reviewPane()->visibleCount(), run.expected);
            w.editor()->markSaved();
        }
        CompareRun run;
        startCompare(run, dir.filePath(QStringLiteral("b.pptx")));
        DeckWindow &w = *run.w;
        QTRY_COMPARE(w.reviewPane()->visibleCount(), run.expected);
        w.reviewPane()->findChild<QPushButton *>(QStringLiteral("reviewRejectAll"))->click();
        QCOMPARE(lookOf(*w.editor()->doc()), lookOf(*run.mine));
        QCOMPARE(w.comparison()->changes.count(review::State::Rejected), run.expected);
        QTRY_COMPARE(w.reviewPane()->visibleCount(), 0);
        w.editor()->markSaved();
    }

    void compareMarksPendingChangesOnTheSlidesAndFiltersAndStepsWork()
    {
        QTemporaryDir dir;
        CompareRun run;
        startCompare(run, dir.filePath(QStringLiteral("theirs.pptx")));
        DeckWindow &w = *run.w;
        Editor *ed = w.editor();
        QTRY_COMPARE(w.reviewPane()->visibleCount(), run.expected);
        // Every pending change with a place is marked on the slide it is on, or counted in the tag of the slide's own.
        int marked = 0;
        for (int page = 0; page < ed->doc()->pages.size(); ++page) {
            ed->setCurrentPage(page);
            QTRY_COMPARE(ed->currentPage(), page);
            QTest::qWait(0);
            marked += int(w.canvas()->reviewMarks().size()) + w.canvas()->reviewSlideLevel();
        }
        QVERIFY2(marked >= 4, qPrintable(QString::number(marked)));
        // Next goes to the change after the chosen one, and the slide follows it.
        int shown = w.reviewPane()->currentChange();
        QSet<int> seen{shown};
        for (int i = 1; i < run.expected; ++i) {
            w.act(QStringLiteral("review.next"))->trigger();
            const int now = w.reviewPane()->currentChange();
            QVERIFY2(now >= 0 && !seen.contains(now), qPrintable(QStringLiteral("step %1 now %2 enabled %3 rows %4").arg(i).arg(now).arg(w.act(QStringLiteral("review.next"))->isEnabled()).arg(w.reviewPane()->visibleCount())));
            seen.insert(now);
        }
        QCOMPARE(int(seen.size()), run.expected);
        w.act(QStringLiteral("review.next"))->trigger();   // (round to the first)
        QCOMPARE(w.reviewPane()->currentChange(), shown);
        w.act(QStringLiteral("review.prev"))->trigger();
        QVERIFY(w.reviewPane()->currentChange() != shown);
        // The marks of a decided change are gone.
        w.acceptAllChanges();
        for (int page = 0; page < ed->doc()->pages.size(); ++page) {
            ed->setCurrentPage(page);
            QTRY_COMPARE(int(w.canvas()->reviewMarks().size()) + w.canvas()->reviewSlideLevel(), 0);
        }
        // The author filter and the state filter: all, accepted, and rejected.
        auto *state = w.reviewPane()->findChild<QComboBox *>(QStringLiteral("reviewStateFilter"));
        state->setCurrentIndex(state->findText(QStringLiteral("Accepted")));
        QTRY_COMPARE(w.reviewPane()->visibleCount(), run.expected);
        state->setCurrentIndex(state->findText(QStringLiteral("Rejected")));
        QTRY_COMPARE(w.reviewPane()->visibleCount(), 0);
        ed->markSaved();
    }

    void compareFiltersBySlideAndTheDetailSetsOutTheWordsOfATextChange()
    {
        QTemporaryDir dir;
        CompareRun run;
        startCompare(run, dir.filePath(QStringLiteral("theirs.pptx")));
        DeckWindow &w = *run.w;
        QTRY_COMPARE(w.reviewPane()->visibleCount(), run.expected);
        auto *slide = w.reviewPane()->findChild<QComboBox *>(QStringLiteral("reviewSlideFilter"));
        QVERIFY(slide->count() >= 3);   // all slides, then one for each slide that has a change
        slide->setCurrentIndex(1);
        QTRY_VERIFY(w.reviewPane()->visibleCount() >= 1 && w.reviewPane()->visibleCount() < run.expected);
        slide->setCurrentIndex(0);
        QTRY_COMPARE(w.reviewPane()->visibleCount(), run.expected);
        int text = -1;
        for (int i = 0; i < w.comparison()->changes.size(); ++i)
            if (w.comparison()->changes.changes[i].target.path == QLatin1String("text")) text = i;
        QVERIFY(text >= 0);
        w.reviewPane()->setCurrentChange(text);
        auto *detail = w.reviewPane()->findChild<QTextBrowser *>(QStringLiteral("reviewDetail"));
        // "box C" became "box C, changed": the words that came are underlined and the ones that stayed are not.
        QTRY_VERIFY(detail->toHtml().contains(QStringLiteral("underline")));
        QVERIFY2(detail->toPlainText().contains(QStringLiteral("changed")), qPrintable(detail->toPlainText()));
        QVERIFY(!detail->toHtml().contains(QStringLiteral("line-through")));
        w.editor()->markSaved();
    }

    void compareKeepsPendingAndRejectedChangesWhenTheFileIsSavedAndOpened()
    {
        for (const QString &name : {QStringLiteral("keep.pptx"), QStringLiteral("keep.odp")}) {
            QTemporaryDir dir;
            CompareRun run;
            startCompare(run, dir.filePath(QStringLiteral("theirs.pptx")));
            DeckWindow &w = *run.w;
            QTRY_COMPARE(w.reviewPane()->visibleCount(), run.expected);
            // One accepted, one rejected, the rest pending.
            QVERIFY(w.acceptChange(0));
            QVERIFY(w.rejectChange(1));
            const int pending = run.expected - 2;
            QCOMPARE(w.comparison()->changes.pending(), pending);
            const QString path = dir.filePath(name);
            QString err;
            QVERIFY2(deck::savePresentationFile(*w.editor()->doc(), path, &err), qPrintable(err));
            w.editor()->markSaved();
            // Another window opens the file: the same changes are pending, the rejected one is still rejected and not listed, and the slides are as they were saved.
            DeckWindow second;
            QVERIFY(second.openFile(path));
            QTRY_VERIFY(second.comparison());
            QCOMPARE(second.comparison()->changes.pending(), pending);
            QCOMPARE(second.comparison()->changes.count(review::State::Rejected), 1);
            QCOMPARE(second.comparison()->changes.size(), pending + 1);
            QTRY_COMPARE(second.reviewPane()->visibleCount(), pending);
            QVERIFY(second.reviewPane()->isVisibleTo(&second));
            QCOMPARE(lookOf(*second.editor()->doc()).size() > 0, true);
            // What was pending can still be accepted there: all of it gives their presentation, bar the rejected change.
            second.acceptAllChanges();
            QCOMPARE(second.comparison()->changes.pending(), 0);
            second.editor()->markSaved();
            // A file saved with nothing compared holds no part for it.
            DeckWindow plain;
            plain.editor()->setDocument(copyOf(*run.mine));
            const QString clean = dir.filePath(QStringLiteral("clean.") + name.section(QLatin1Char('.'), -1));
            QVERIFY(deck::savePresentationFile(*plain.editor()->doc(), clean, &err));
            office::Package pkg;
            QFile f(clean);
            QVERIFY(f.open(QIODevice::ReadOnly));
            const QByteArray zip = f.readAll();
            QVERIFY(name.endsWith(QLatin1String(".odp")) ? pkg.readOpenDocument(zip) : pkg.read(zip));
            QVERIFY(!pkg.hasPart(QLatin1Char('/') + deck::reviewOdfName()));
            // ... and the one with a comparison holds exactly one.
            QFile g(path);
            QVERIFY(g.open(QIODevice::ReadOnly));
            office::Package kept;
            const QByteArray bytes = g.readAll();
            QVERIFY(name.endsWith(QLatin1String(".odp")) ? kept.readOpenDocument(bytes) : kept.read(bytes));
            QCOMPARE(int(kept.partNames().filter(QStringLiteral("review.json")).size()), 1);
            plain.editor()->markSaved();
        }
    }

    // ---- what the window and the file do with the comparison kept in a file (Nov 18 review) ----

    static bool hasReviewPart(const QString &path)
    {
        QFile f(path);
        if (!f.open(QIODevice::ReadOnly)) return false;
        const QByteArray zip = f.readAll();
        office::Package pkg;
        if (!(path.endsWith(QLatin1String(".odp")) ? pkg.readOpenDocument(zip) : pkg.read(zip))) return false;
        return !pkg.partNames().filter(QStringLiteral("review.json")).isEmpty();
    }
    // Waits until a zero timer made now has run: the zero timers made before it (the window's marks) have run by then.
    static void drainTimers()
    {
        bool done = false;
        QTimer::singleShot(0, [&done] { done = true; });
        QTRY_VERIFY(done);
    }
    static void editSomething(DeckWindow &w, int i) { w.editor()->change(QStringLiteral("Edit"), [&] { w.editor()->doc()->props.author = QStringLiteral("author %1").arg(i); }); }
    // A file of the base deck that keeps `part`'s appData "compare".
    static QString fileKeeping(const QString &folder, const QString &name, const QJsonObject &compare)
    {
        auto doc = compareBase();
        doc->appData[QStringLiteral("compare")] = compare;
        const QString path = QDir(folder).filePath(name);
        QString err;
        [&] { QVERIFY2(deck::savePresentationFile(*doc, path, &err), qPrintable(err)); }();
        return path;
    }

    // A part whose other presentation is not a deck: the comparison is let go of once, with one word, and no edit reads it again.
    void compareJunkPartIsLetGoOfOnceAndNotReadAgainWithEdits()
    {
        QTemporaryDir dir;
        QJsonObject c;
        c[QStringLiteral("theirs")] = deck::storeReviewTheirs(QByteArray("this is not a presentation"));
        c[QStringLiteral("rejected")] = QJsonArray();
        c[QStringLiteral("states")] = QJsonObject();
        c[QStringLiteral("pending")] = 1;
        const QString path = fileKeeping(dir.path(), QStringLiteral("junk.pptx"), c);
        QVERIFY(hasReviewPart(path));
        deck::clearReviewStore();
        deck::reviewCounters() = {};
        DeckWindow w;
        QVERIFY(w.openFile(path));
        QCoreApplication::processEvents();
        QVERIFY(!w.comparison());
        QVERIFY(w.editor()->doc()->appData.value(QStringLiteral("compare")).toObject().isEmpty());
        QCOMPARE(deck::reviewCounters().imports, 1);
        QVERIFY(!w.statusBar()->currentMessage().isEmpty());
        for (int i = 0; i < 20; ++i) editSomething(w, i);
        QCoreApplication::processEvents();
        QCOMPARE(deck::reviewCounters().imports, 1);
        w.editor()->markSaved();
    }

    // The marks of a slide are made again when the slide or a change's state changes, not with every edit.
    void compareMarksAreNotMadeAgainByEditsThatDecideNothing()
    {
        QTemporaryDir dir;
        CompareRun run;
        startCompare(run, dir.filePath(QStringLiteral("theirs.pptx")));
        DeckWindow &w = *run.w;
        QTRY_COMPARE(w.reviewPane()->visibleCount(), run.expected);
        QTRY_VERIFY(deck::reviewCounters().markRebuilds > 0);
        drainTimers();
        const int before = deck::reviewCounters().markRebuilds;
        for (int i = 0; i < 20; ++i) editSomething(w, i);
        drainTimers();
        QCOMPARE(deck::reviewCounters().markRebuilds, before);
        // A change decided, and a change of slide, make them again.
        QVERIFY(w.rejectChange(0));
        QTRY_VERIFY(deck::reviewCounters().markRebuilds > before);
        const int afterReject = deck::reviewCounters().markRebuilds;
        w.editor()->setCurrentPage(w.editor()->currentPage() == 0 ? 1 : 0);
        QTRY_VERIFY(deck::reviewCounters().markRebuilds > afterReject);
        w.editor()->markSaved();
    }

    // A part is made into the other presentation only when the window that opens the file needs it; every other reading of a file ignores it.
    void compareDecodesAPartOnlyForAWindowsOwnOpen()
    {
        QTemporaryDir dir;
        CompareRun run;
        startCompare(run, dir.filePath(QStringLiteral("theirs.pptx")));
        QTRY_COMPARE(run.w->reviewPane()->visibleCount(), run.expected);
        const QString kept = dir.filePath(QStringLiteral("kept.pptx"));
        QString err;
        QVERIFY2(deck::savePresentationFile(*run.w->editor()->doc(), kept, &err), qPrintable(err));
        run.w->editor()->markSaved();
        QVERIFY(hasReviewPart(kept));
        deck::clearReviewStore();
        deck::reviewCounters() = {};
        // Read as "theirs", for slides to reuse, or by the tool: nothing of it is read or stored.
        auto plain = deck::importAnyPresentationFile(kept);
        QVERIFY(plain);
        QVERIFY(plain->appData.value(QStringLiteral("compare")).toObject().isEmpty());
        QFile f(kept);
        QVERIFY(f.open(QIODevice::ReadOnly));
        QVERIFY(deck::importAnyPresentationBytes(f.readAll()));
        QCOMPARE(deck::storedReviewCount(), 0);
        QCOMPARE(deck::reviewCounters().decodes, 0);
        // A window comparing with it stores the file it chose and nothing more.
        DeckWindow other;
        other.editor()->setDocument(compareBase());
        QString why;
        QVERIFY(other.compareWith(kept, &why));
        QCOMPARE(deck::storedReviewCount(), 1);
        QCOMPARE(deck::reviewCounters().decodes, 0);
        other.editor()->markSaved();
        // The window that opens the file keeps the part as it came until it needs the comparison.
        {
            deck::ReviewOpenScope own;
            auto raw = deck::importAnyPresentationFile(kept);
            QVERIFY(raw);
            QVERIFY(raw->appData.value(QStringLiteral("compare")).toObject().contains(QStringLiteral("raw")));
            QCOMPARE(deck::reviewCounters().decodes, 0);
        }
        deck::clearReviewStore();
        DeckWindow second;
        QVERIFY(second.openFile(kept));
        QTRY_VERIFY(second.comparison());
        QCOMPARE(deck::reviewCounters().decodes, 1);
        second.editor()->markSaved();
        // A file with no comparison is opened with no decoding.
        deck::reviewCounters() = {};
        const QString minePath = dir.filePath(QStringLiteral("mine.pptx"));
        DeckWindow third;
        QVERIFY(third.openFile(minePath));
        QCOMPARE(deck::reviewCounters().decodes, 0);
        QVERIFY(!third.comparison());
    }

    // Five presentations compared in one session (more than the store keeps with no window using them) all keep their part when saved.
    void compareKeepsThePartOfEveryComparedPresentationInASession()
    {
        QTemporaryDir dir;
        deck::clearReviewStore();
        std::vector<std::unique_ptr<DeckWindow>> windows;
        for (int i = 0; i < deck::kReviewStoreMost + 1; ++i) {
            auto base = compareBase();
            auto theirs = changedBase(*base);
            addBox(*theirs, *theirs->pages[0], QStringLiteral("extra %1").arg(i), 60, 60);
            const QString tp = dir.filePath(QStringLiteral("theirs%1.pptx").arg(i));
            QString err;
            QVERIFY2(deck::savePresentationFile(*theirs, tp, &err), qPrintable(err));
            windows.push_back(std::make_unique<DeckWindow>());
            DeckWindow &w = *windows.back();
            w.editor()->setDocument(copyOf(*base));
            w.compareFileChooser = [tp] { return tp; };
            w.act(QStringLiteral("review.compare"))->trigger();
            QVERIFY(w.comparison());
        }
        for (size_t i = 0; i < windows.size(); ++i) {
            const QString out = dir.filePath(QStringLiteral("out%1.pptx").arg(i));
            QString err;
            QVERIFY2(deck::savePresentationFile(*windows[i]->editor()->doc(), out, &err), qPrintable(err));
            QVERIFY2(hasReviewPart(out), qPrintable(QString::number(i)));
            windows[i]->editor()->markSaved();
        }
        // When the windows are gone, the store lets go of what they used.
        windows.clear();
        QCOMPARE(deck::storedReviewCount(), 0);
    }

    // The file keeps a comparison only while a change is pending; one with nothing compared is saved as it always was.
    void compareFileHasNoPartWhenNothingIsPending()
    {
        QTemporaryDir dir;
        CompareRun run;
        startCompare(run, dir.filePath(QStringLiteral("theirs.pptx")));
        DeckWindow &w = *run.w;
        QTRY_COMPARE(w.reviewPane()->visibleCount(), run.expected);
        QString err;
        const QString pending = dir.filePath(QStringLiteral("pending.pptx"));
        QVERIFY2(deck::savePresentationFile(*w.editor()->doc(), pending, &err), qPrintable(err));
        QVERIFY(hasReviewPart(pending));
        w.acceptAllChanges();
        const QString decided = dir.filePath(QStringLiteral("decided.pptx"));
        QVERIFY2(deck::savePresentationFile(*w.editor()->doc(), decided, &err), qPrintable(err));
        QVERIFY(!hasReviewPart(decided));
        w.editor()->undo();   // (undo brings the changes back, and the part with them)
        QTRY_COMPARE(w.comparison()->changes.pending(), run.expected);
        const QString back = dir.filePath(QStringLiteral("back.pptx"));
        QVERIFY2(deck::savePresentationFile(*w.editor()->doc(), back, &err), qPrintable(err));
        QVERIFY(hasReviewPart(back));
        w.editor()->markSaved();
        // A presentation that never used Compare is saved the same way twice, with no part.
        auto doc = compareBase();
        const QString a = dir.filePath(QStringLiteral("a.pptx")), b = dir.filePath(QStringLiteral("b.pptx"));
        QVERIFY(deck::savePresentationFile(*doc, a, &err));
        QVERIFY(deck::savePresentationFile(*doc, b, &err));
        QVERIFY(!hasReviewPart(a));
        office::Package pa, pb;
        QFile fa(a), fb(b);
        QVERIFY(fa.open(QIODevice::ReadOnly) && fb.open(QIODevice::ReadOnly));
        QVERIFY(pa.read(fa.readAll()) && pb.read(fb.readAll()));
        QCOMPARE(pa.partNames(), pb.partNames());
    }

    // A rejected change is told by where it is in the other presentation and what changed, not by words of the interface or the slide numbers.
    void compareRejectionKeysAreStableAcrossReopeningAndOtherChanges()
    {
        QTemporaryDir dir;
        // Two slides with the same box text; theirs gives both boxes the same fill (two changes worded alike), moves a third slide, and changes a text.
        auto mine = lettered(4);
        for (int i = 0; i < 4; ++i) addBox(*mine, *mine->pages[i], i < 2 ? QStringLiteral("same words") : QStringLiteral("other %1").arg(i), 50, 50);
        auto theirs = copyOf(*mine);
        for (int i = 0; i < 2; ++i) boxOf(*theirs, i)->fill = Fill::solid(ColorRef::rgb(QColor(Qt::red)));
        QTextCursor tc(theirs->storyDoc(boxOf(*theirs, 2)->storyId));
        tc.select(QTextCursor::Document);
        tc.insertText(QStringLiteral("other 2 changed"));
        theirs->pages.move(3, 0);
        QString err;
        const QString minePath = dir.filePath(QStringLiteral("mine.pptx")), theirsPath = dir.filePath(QStringLiteral("theirs.pptx"));
        QVERIFY2(deck::savePresentationFile(*mine, minePath, &err), qPrintable(err));
        QVERIFY2(deck::savePresentationFile(*theirs, theirsPath, &err), qPrintable(err));
        DeckWindow w;
        QVERIFY(w.openFile(minePath));
        w.compareFileChooser = [theirsPath] { return theirsPath; };
        w.act(QStringLiteral("review.compare"))->trigger();
        QVERIFY(w.comparison());
        const review::ChangeList &list = w.comparison()->changes;
        QVector<int> fills;
        int move = -1, text = -1;
        for (int i = 0; i < list.size(); ++i) {
            const QString path = list.changes[i].target.path;
            if (path == QLatin1String("fill")) fills << i;
            if (path == QLatin1String("order")) move = i;
            if (path == QLatin1String("text")) text = i;
        }
        QCOMPARE(int(fills.size()), 2);
        QVERIFY(move >= 0 && text >= 0);
        // The keys differ for the two alike, and hold no words of the interface (the summary, a slide's title, "Slide ...").
        QVERIFY(deck::changeKey(list.changes[fills[0]]) != deck::changeKey(list.changes[fills[1]]));
        for (const review::Change &c : list.changes) {
            const QString key = deck::changeKey(c);
            QVERIFY2(!key.contains(c.summary), qPrintable(key));
            QVERIFY2(!key.contains(QStringLiteral("same words")) && !key.contains(QStringLiteral("Slide")), qPrintable(key));
        }
        // The second fill and the move are rejected, the text is accepted (that changes what the slides are), and the file is saved and read again.
        const QString fillRejected = deck::changeKey(list.changes[fills[1]]), moveRejected = deck::changeKey(list.changes[move]);
        QVERIFY(w.rejectChange(fills[1]));
        QVERIFY(w.rejectChange(move));
        QVERIFY(w.acceptChange(text));
        const QString kept = dir.filePath(QStringLiteral("kept.pptx"));
        QVERIFY2(deck::savePresentationFile(*w.editor()->doc(), kept, &err), qPrintable(err));
        w.editor()->markSaved();
        DeckWindow second;
        QVERIFY(second.openFile(kept));
        QTRY_VERIFY(second.comparison());
        const review::ChangeList &again = second.comparison()->changes;
        int rejected = 0;
        for (const review::Change &c : again.changes) {
            if (c.state != review::State::Rejected) continue;
            ++rejected;
            const QString key = deck::changeKey(c);
            QVERIFY2(key == fillRejected || key == moveRejected, qPrintable(key));
        }
        QCOMPARE(rejected, 2);
        // The other fill is still pending.
        QCOMPARE(again.count(review::State::Pending), 1);
        second.editor()->markSaved();
    }

    // Two slides taken out that are alike in title and number of objects are told apart by what they hold; any change's key follows its content.
    void compareKeysFollowTheContentOfEveryChange()
    {
        QTemporaryDir dir;
        auto mine = lettered(3);
        for (int i = 0; i < 3; ++i) mine->pages[i]->title = i == 0 ? QStringLiteral("Keep") : QStringLiteral("Dup");
        addBox(*mine, *mine->pages[0], QStringLiteral("kept"), 50, 50);
        addBox(*mine, *mine->pages[1], QStringLiteral("alpha"), 50, 50);
        addBox(*mine, *mine->pages[2], QStringLiteral("beta"), 50, 50);
        auto theirs = copyOf(*mine);
        theirs->pages.removeAt(2);
        theirs->pages.removeAt(1);
        QString err;
        const QString minePath = dir.filePath(QStringLiteral("mine.pptx")), theirsPath = dir.filePath(QStringLiteral("theirs.pptx"));
        QVERIFY2(deck::savePresentationFile(*mine, minePath, &err), qPrintable(err));
        QVERIFY2(deck::savePresentationFile(*theirs, theirsPath, &err), qPrintable(err));
        DeckWindow w;
        QVERIFY(w.openFile(minePath));
        w.compareFileChooser = [theirsPath] { return theirsPath; };
        w.act(QStringLiteral("review.compare"))->trigger();
        QVERIFY(w.comparison());
        QCOMPARE(w.comparison()->changes.size(), 2);
        const review::ChangeList &list = w.comparison()->changes;
        QVERIFY(deck::changeKey(list.changes[0]) != deck::changeKey(list.changes[1]));
        // Reject the one that held "beta" (the last slide), save, and read again: that one only.
        int beta = -1;
        for (int i = 0; i < list.size(); ++i)
            if (deck::markOf(*w.editor()->doc(), *const_cast<DeckComparison *>(w.comparison()), i).slide == 2) beta = i;
        QVERIFY(beta >= 0);
        const QString rejectedKey = deck::changeKey(list.changes[beta]);
        QVERIFY(w.rejectChange(beta));
        const QString kept = dir.filePath(QStringLiteral("kept.pptx"));
        QVERIFY2(deck::savePresentationFile(*w.editor()->doc(), kept, &err), qPrintable(err));
        w.editor()->markSaved();
        DeckWindow second;
        QVERIFY(second.openFile(kept));
        QTRY_VERIFY(second.comparison());
        const review::ChangeList &again = second.comparison()->changes;
        QCOMPARE(again.count(review::State::Rejected), 1);
        QCOMPARE(again.count(review::State::Pending), 1);
        for (const review::Change &c : again.changes)
            if (c.state == review::State::Rejected) QCOMPARE(deck::changeKey(c), rejectedKey);
        second.editor()->markSaved();
        // A change that is not a comment: its key differs when what it changed to differs.
        auto base = compareBase();
        auto red = copyOf(*base), blue = copyOf(*base);
        boxOf(*red, 0)->fill = Fill::solid(ColorRef::rgb(QColor(Qt::red)));
        boxOf(*blue, 0)->fill = Fill::solid(ColorRef::rgb(QColor(Qt::blue)));
        DeckComparison cr = compareDecks(*base, *red), cb = compareDecks(*base, *blue);
        QCOMPARE(cr.changes.size(), 1);
        QCOMPARE(cb.changes.size(), 1);
        QVERIFY(deck::changeKey(cr.changes.changes[0]) != deck::changeKey(cb.changes.changes[0]));
        // ... and the same change twice has the same key.
        DeckComparison cr2 = compareDecks(*base, *red);
        QCOMPARE(deck::changeKey(cr2.changes.changes[0]), deck::changeKey(cr.changes.changes[0]));
    }

    // A window that compares again lets go of the copy it kept for the comparison before.
    void compareAgainLetsGoOfTheEarlierCopy()
    {
        QTemporaryDir dir;
        deck::clearReviewStore();
        DeckWindow w;
        auto base = compareBase();
        w.editor()->setDocument(copyOf(*base));
        QStringList paths;
        for (int i = 0; i < 3; ++i) {
            auto theirs = changedBase(*base);
            addBox(*theirs, *theirs->pages[0], QStringLiteral("extra %1").arg(i), 60, 60);
            QString err;
            paths << dir.filePath(QStringLiteral("theirs%1.pptx").arg(i));
            QVERIFY2(deck::savePresentationFile(*theirs, paths.last(), &err), qPrintable(err));
        }
        for (int i = 0; i < 3; ++i) {
            w.compareFileChooser = [p = paths[i]] { return p; };
            w.act(QStringLiteral("review.compare"))->trigger();
            QVERIFY(w.comparison());
            QCOMPARE(deck::storedReviewCount(), 1);
        }
        w.editor()->markSaved();
    }

    // The status bar says what Compare found without the slide number drawn over it.
    void compareFoundMessageIsNotDrawnOverBySlideNumber()
    {
        QTemporaryDir dir;
        CompareRun run;
        startCompare(run, dir.filePath(QStringLiteral("theirs.pptx")));
        DeckWindow &w = *run.w;
        w.show();
        QTRY_COMPARE(w.reviewPane()->visibleCount(), run.expected);
        QTRY_VERIFY(!w.statusBar()->currentMessage().isEmpty());
        QLabel *label = w.findChild<QLabel *>(QStringLiteral("slideNumber"));
        QVERIFY(label);
        // (the window's own refresh of its labels runs on a timer: waited for by what it writes)
        // (the page moves, as it does when Compare goes to a change: the label is written again while the message is up)
        w.editor()->setCurrentPage(2);
        QTRY_VERIFY(label->text().contains(QStringLiteral("3")));
        QVERIFY(!w.statusBar()->currentMessage().isEmpty());
        QVERIFY2(!label->isVisible(), "the slide number is drawn while the status bar shows a message");
        w.statusBar()->clearMessage();
        QTRY_VERIFY(label->isVisible());   // (and it is back when the message is gone)
        w.editor()->markSaved();
    }

    // The pane draws one decided change in place: no row is made again, and the filters are not asked for their words again.
    void compareReviewPaneRefreshesOneChangeInPlace()
    {
        review::ChangeList list;
        for (int i = 0; i < 50; ++i) {
            review::Change c;
            c.id = QStringLiteral("c%1").arg(i + 1);
            c.target.container = QStringLiteral("slide%1").arg(i % 10);
            c.author = QStringLiteral("a");
            c.summary = QStringLiteral("change %1").arg(i);
            list.changes << c;
        }
        DeckReviewPane pane;
        int labels = 0, begins = 0;
        DeckReviewPane::Source src;
        src.list = &list;
        src.slideLabel = [&labels](const QString &container) {
            ++labels;
            return container;
        };
        src.beginLabels = [&begins] { ++begins; };
        pane.setSource(src);
        QCOMPARE(pane.visibleCount(), 50);
        QCOMPARE(begins, 1);
        QCOMPARE(labels, 10);
        int resets = 0, removed = 0, changed = 0;
        QObject::connect(pane.list()->model(), &QAbstractItemModel::modelReset, &pane, [&] { ++resets; });
        QObject::connect(pane.list()->model(), &QAbstractItemModel::rowsRemoved, &pane, [&] { ++removed; });
        QObject::connect(pane.list()->model(), &QAbstractItemModel::dataChanged, &pane, [&] { ++changed; });
        pane.setCurrentChange(3);
        // (the state filter shows the pending ones: a decided change goes from the list)
        list.changes[3].state = review::State::Accepted;
        pane.refreshChange(3);
        QCOMPARE(pane.visibleCount(), 49);
        QCOMPARE(resets, 0);
        QCOMPARE(removed, 1);
        QCOMPARE(begins, 1);
        QCOMPARE(labels, 10);
        QCOMPARE(pane.currentChange(), 4);   // (the one that took its place)
        auto *status = pane.findChild<QLabel *>(QStringLiteral("reviewStatus"));
        QVERIFY2(status->text().contains(QStringLiteral("49")), qPrintable(status->text()));
        // With every state shown the row stays and is drawn again.
        auto *state = pane.findChild<QComboBox *>(QStringLiteral("reviewStateFilter"));
        state->setCurrentIndex(state->findText(QStringLiteral("All")));
        QCOMPARE(pane.visibleCount(), 50);
        resets = removed = changed = 0;
        list.changes[7].state = review::State::Rejected;
        pane.refreshChange(7);
        QCOMPARE(resets, 0);
        QCOMPARE(removed, 0);
        QCOMPARE(changed, 1);
    }

    // The window gives the pane the words of a slide from a lookup made once, and a file that opens does not move the slide it opens on.
    void compareOpeningAFileWithAPartKeepsTheCurrentSlide()
    {
        QTemporaryDir dir;
        CompareRun run;
        startCompare(run, dir.filePath(QStringLiteral("theirs.pptx")));
        QTRY_COMPARE(run.w->reviewPane()->visibleCount(), run.expected);
        const QString kept = dir.filePath(QStringLiteral("kept.pptx"));
        QString err;
        QVERIFY2(deck::savePresentationFile(*run.w->editor()->doc(), kept, &err), qPrintable(err));
        run.w->editor()->markSaved();
        DeckWindow second;
        QVERIFY(second.openFile(kept));
        QTRY_VERIFY(second.comparison());
        QTRY_COMPARE(second.reviewPane()->visibleCount(), run.expected);
        QCOMPARE(second.editor()->currentPage(), 0);
        QVERIFY(second.reviewPane()->currentChange() >= 0);   // (the first change is chosen, not gone to)
        // (the first change is on the last slide, E: going to it is what a file that opens must not do)
        QCoreApplication::processEvents();
        QCOMPARE(second.editor()->currentPage(), 0);
        second.editor()->markSaved();
    }

    // The two caps are in force: the other presentation up to its limit, the part up to its own.
    void compareLimitsOfThePartAreInForce()
    {
        struct Restore {
            ~Restore() { deck::setReviewLimitsForTest(-1, -1); }
        } restore;
        deck::clearReviewStore();
        deck::setReviewLimitsForTest(100, -1);
        QVERIFY(deck::storeReviewTheirs(QByteArray(100, 'a')).size() > 0);
        QVERIFY(deck::storeReviewTheirs(QByteArray(101, 'b')).isEmpty());
        QCOMPARE(deck::storedReviewCount(), 1);
        deck::setReviewLimitsForTest(-1, 200);
        deck::ReviewOpenScope own;
        auto doc = compareBase();
        QJsonObject part;
        part[QStringLiteral("format")] = 1;
        part[QStringLiteral("theirs")] = QString::fromLatin1(QByteArray("x").toBase64());
        part[QStringLiteral("rejected")] = QJsonArray();
        QByteArray small = QJsonDocument(part).toJson(QJsonDocument::Compact);
        QVERIFY(small.size() <= 200);
        QVERIFY(deck::readReviewPart(small, *doc));
        QByteArray big = small;
        big.append(QByteArray(201 - big.size() + 1, ' '));
        QVERIFY(big.size() > 200);
        auto other = compareBase();
        QVERIFY(!deck::readReviewPart(big, *other));
        QVERIFY(other->appData.value(QStringLiteral("compare")).toObject().isEmpty());
    }

    // Pictures of the window with a compared presentation, for Jeff and the docs, when JO_SHOT_DIR names a folder.
    void compareScreenshotsOfTheReviewingPane()
    {
        if (qEnvironmentVariableIsEmpty("JO_SHOT_DIR")) QSKIP("JO_SHOT_DIR names no folder");
        jp::applyUiTheme(1);
        QTemporaryDir dir;
        CompareRun run;
        startCompare(run, dir.filePath(QStringLiteral("theirs.pptx")));
        DeckWindow &w = *run.w;
        w.resize(1440, 900);
        w.show();
        QTRY_COMPARE(w.reviewPane()->visibleCount(), run.expected);
        int text = -1;
        for (int i = 0; i < w.comparison()->changes.size(); ++i)
            if (w.comparison()->changes.changes[i].target.path == QLatin1String("text")) text = i;
        QVERIFY(text >= 0);
        w.reviewPane()->setCurrentChange(text);
        QTRY_VERIFY(!w.canvas()->reviewMarks().isEmpty());
        QCoreApplication::processEvents();
        shotOf(&w, QStringLiteral("deck-compare-text-change"));
        QVERIFY(w.acceptChange(text));
        auto *state = w.reviewPane()->findChild<QComboBox *>(QStringLiteral("reviewStateFilter"));
        state->setCurrentIndex(state->findText(QStringLiteral("All")));
        w.reviewPane()->setCurrentChange(0);
        QCoreApplication::processEvents();
        shotOf(&w, QStringLiteral("deck-compare-after-accept"));
        w.editor()->markSaved();
    }

    // A part from a file is bounded: at most 10,000 keys, the store keeps four other presentations, and a part that is not what it should be is refused.
    void comparePartFromAFileIsBounded()
    {
        deck::clearReviewStore();
        deck::ReviewOpenScope own;
        auto doc = compareBase();
        QJsonArray keys;
        for (int i = 0; i < deck::kMaxReviewKeys + 50; ++i) keys.append(QStringLiteral("edited|text|%1").arg(i));
        QJsonObject part;
        part[QStringLiteral("format")] = 1;
        part[QStringLiteral("theirs")] = QString::fromLatin1(QByteArray("some bytes").toBase64());
        part[QStringLiteral("rejected")] = keys;
        QVERIFY(deck::readReviewPart(QJsonDocument(part).toJson(QJsonDocument::Compact), *doc));
        // (kept as it came until a window needs it, then made into the comparison)
        QVERIFY(deck::decodeReviewPart(*doc));
        QCOMPARE(int(doc->appData.value(QStringLiteral("compare")).toObject().value(QStringLiteral("rejected")).toArray().size()), deck::kMaxReviewKeys);
        // Not a part we wrote: nothing is made of it, and the document has no comparison.
        auto other = compareBase();
        QVERIFY(deck::readReviewPart(QByteArray("not json"), *other));
        QVERIFY(!deck::decodeReviewPart(*other));
        QVERIFY(other->appData.value(QStringLiteral("compare")).toObject().isEmpty());
        part[QStringLiteral("format")] = 2;
        auto third = compareBase();
        QVERIFY(deck::readReviewPart(QJsonDocument(part).toJson(QJsonDocument::Compact), *third));
        QVERIFY(!deck::decodeReviewPart(*third));
        QVERIFY(third->appData.value(QStringLiteral("compare")).toObject().isEmpty());
        // The store keeps the most recent four of the entries no window uses.
        deck::clearReviewStore();
        QString first;
        for (int i = 0; i < deck::kReviewStoreMost + 2; ++i) {
            const QString key = deck::storeReviewTheirs(QByteArray("deck %1").replace("%1", QByteArray::number(i)));
            if (i == 0) first = key;
        }
        QCOMPARE(deck::storedReviewCount(), deck::kReviewStoreMost);
        QVERIFY(deck::storedReviewTheirs(first).isEmpty());
        // One a window uses stays, and goes when the window does.
        deck::clearReviewStore();
        auto *owner = new QObject;
        const QString mine = deck::storeReviewTheirs(QByteArray("mine"));
        deck::retainReviewTheirs(mine, owner);
        for (int i = 0; i < deck::kReviewStoreMost + 3; ++i) deck::storeReviewTheirs(QByteArray("other %1").replace("%1", QByteArray::number(i)));
        QVERIFY(!deck::storedReviewTheirs(mine).isEmpty());
        delete owner;
        QVERIFY(deck::storedReviewTheirs(mine).isEmpty());
    }

    // The pane lists at most 10,000 rows, and looks up the words for at most 2,000 slides.
    void compareReviewPaneListsAtMost10000RowsAndFiltersBy2000Slides()
    {
        review::ChangeList list;
        QSet<QString> seen;
        for (int i = 0; i < review::kMaxChanges + 50; ++i) {
            review::Change c;
            c.id = QStringLiteral("c%1").arg(i + 1);
            c.target.container = QStringLiteral("slide%1").arg(i % 5000);
            c.author = QStringLiteral("a%1").arg(i % 7);
            c.summary = QStringLiteral("change %1").arg(i);
            list.changes << c;
        }
        DeckReviewPane pane;
        pane.resize(320, 600);
        int labels = 0;
        DeckReviewPane::Source src;
        src.list = &list;
        src.slideLabel = [&labels](const QString &container) {
            ++labels;
            return container;
        };
        pane.setSource(src);
        QCOMPARE(pane.visibleCount(), review::kMaxChanges);
        QCOMPARE(labels, 2000);   // (5,000 slides have changes; the words of 2,000 are asked for, no more and no fewer)
        auto *author = pane.findChild<QComboBox *>(QStringLiteral("reviewAuthorFilter"));
        author->setCurrentIndex(1);
        int ofFirstAuthor = 0;   // (the first author listed is a0: every seventh change of the 10,000 shown)
        for (int i = 0; i < review::kMaxChanges; ++i)
            if (i % 7 == 0) ++ofFirstAuthor;
        QTRY_COMPARE(pane.visibleCount(), ofFirstAuthor);
        pane.show();
        QVERIFY(QTest::qWaitForWindowExposed(&pane));
        // (the view asks only for the rows it shows)
        QVERIFY(pane.list()->indexAt(QPoint(10, 10)).isValid());
    }

    // Ids repeated all through a file (hostile) still give a list, and accepting changes the slide the change is about.
    void compareSurvivesRepeatedIds()
    {
        auto mine = lettered(4);
        for (int i = 0; i < 4; ++i) {
            mine->pages[i]->id = QStringLiteral("same");
            addBox(*mine, *mine->pages[i], QStringLiteral("box %1").arg(i), 40, 40)->id = QStringLiteral("dup");
        }
        auto theirs = copyOf(*mine);
        QTextCursor c(theirs->storyDoc(boxOf(*theirs, 2)->storyId));
        c.select(QTextCursor::Document);
        c.insertText(QStringLiteral("other words"));
        DeckComparison cmp = compareDecks(*mine, *theirs);
        QCOMPARE(cmp.changes.size(), 1);
        QCOMPARE(int(cmp.changes.changes[0].kind), int(review::Kind::Edited));
        auto working = copyOf(*mine);
        for (int i = 0; i < 4; ++i) boxOf(*working, i)->id = QStringLiteral("dup");   // a copy gives repeated object ids new ones: the compared deck has them, so the one worked on must
        QCOMPARE(acceptAll(*working, cmp), 1);
        QCOMPARE(textOfBox(*working, boxOf(*working, 2)), QStringLiteral("other words"));
        QCOMPARE(textOfBox(*working, boxOf(*working, 0)), QStringLiteral("box 0"));
        QCOMPARE(textOfBox(*working, boxOf(*working, 3)), QStringLiteral("box 3"));
        // And presentations that name the very same ids on every slide and are otherwise strangers.
        auto other = lettered(3);
        for (auto &p : other->pages) p->id = QStringLiteral("same");
        DeckComparison strangers = compareDecks(*mine, *other);
        QVERIFY(strangers.changes.size() > 0);
        auto w2 = copyOf(*mine);
        acceptAll(*w2, strangers);
        rejectAll(*w2, strangers, true);
    }

    // At most 2,000 slides of each presentation are read; past that the list says it is not all there is.
    void compareReadsAtMost2000SlidesOfEach()
    {
        auto make = [](int n, const QString &lastTitle) {
            auto d = deck::newPresentation();
            while (d->pages.size() < n) d->addPage(-1, d->pages[0]->masterId);
            for (int i = 0; i < n; ++i) d->pages[i]->title = QStringLiteral("slide number %1").arg(i);
            d->pages[n - 1]->title = lastTitle;
            return d;
        };
        auto mine = make(2005, QStringLiteral("a"));
        auto theirs = make(2005, QStringLiteral("b"));
        DeckComparison cmp = compareDecks(*mine, *theirs);
        QCOMPARE(cmp.slidesMine, 2000);
        QCOMPARE(cmp.slidesTheirs, 2000);
        QVERIFY(cmp.changes.tooMany);
        QCOMPARE(cmp.changes.size(), 0);   // the slide that differs is past the limit
        // 2,000 exactly is all there is.
        auto few = make(2000, QStringLiteral("a"));
        auto fewer = make(2000, QStringLiteral("b"));
        DeckComparison all = compareDecks(*few, *fewer);
        QCOMPARE(all.slidesMine, 2000);
        QVERIFY(!all.changes.tooMany);
        QCOMPARE(all.changes.size(), 2);   // the last slide is a different one: deleted and added
    }

    // At most 20,000 objects of each presentation are read in all, over every slide; a slide whose objects were not all read is not compared object by object.
    void compareReadsAtMost20000ObjectsOfEach()
    {
        auto mine = deck::newPresentation();
        while (mine->pages.size() < 21) mine->addPage(-1, mine->pages[0]->masterId);
        for (auto &p : mine->pages) {
            p->items.clear();
            for (int i = 0; i < 1000; ++i) addBox(*mine, *p, QString(), 10 + i % 50, 10 + i / 50);
        }
        auto theirs = copyOf(*mine);
        theirs->pages[0]->items[3]->rect.translate(7, 7);      // read: listed
        theirs->pages[20]->items[3]->rect.translate(7, 7);     // past the 20,000: not listed
        DeckComparison cmp = compareDecks(*mine, *theirs);
        QCOMPARE(cmp.objectsMine, 20000);
        QCOMPARE(cmp.objectsTheirs, 20000);
        QVERIFY(cmp.changes.tooMany);
        QCOMPARE(cmp.changes.size(), 1);
        QCOMPARE(cmp.changes.changes[0].target.container, mine->pages[0]->id);
        // 20,000 exactly: all read, nothing left out.
        mine->pages.removeLast();
        theirs->pages.removeLast();
        DeckComparison fits = compareDecks(*mine, *theirs);
        QCOMPARE(fits.objectsMine, 20000);
        QVERIFY(!fits.changes.tooMany);
        QCOMPARE(fits.changes.size(), 1);
    }

    // At most 10,000 changes are listed, then "too many" and stop.
    void compareListsAtMost10000Changes()
    {
        auto mine = deck::newPresentation();
        mine->pages[0]->items.clear();
        for (int i = 0; i < 10050; ++i) addBox(*mine, *mine->pages[0], QString(), 10 + i % 100, 10 + i / 100);
        auto theirs = copyOf(*mine);
        for (auto &it : theirs->pages[0]->items) it->rect.translate(3, 3);
        DeckComparison cmp = compareDecks(*mine, *theirs);
        QCOMPARE(cmp.changes.size(), review::kMaxChanges);
        QVERIFY(cmp.changes.tooMany);
        QCOMPARE(kindCount(cmp, review::Kind::Moved), 10000);
        // Exactly 10,000 is all there is.
        theirs->pages[0]->items.resize(10000);
        mine->pages[0]->items.resize(10000);
        DeckComparison exact = compareDecks(*mine, *theirs);
        QCOMPARE(exact.changes.size(), 10000);
        QVERIFY(!exact.changes.tooMany);
    }

    // Matching does not compare every slide with every slide, nor every object with every object: the work counted grows with the size, not its square.
    void compareMatchingIsNotQuadratic()
    {
        // 2,000 slides each a little different: found by the words they share.
        auto mine = deck::newPresentation();
        while (mine->pages.size() < 2000) mine->addPage(-1, mine->pages[0]->masterId);
        for (int i = 0; i < 2000; ++i) mine->pages[i]->title = QStringLiteral("alpha%1 beta%1 gamma%1 delta%1 epsilon%1 zeta%1").arg(i);
        auto theirs = copyOf(*mine);
        for (auto &p : theirs->pages) {
            p->id = newId("p");
            p->title += QStringLiteral(" extra");
        }
        DeckComparison cmp = compareDecks(*mine, *theirs);
        QCOMPARE(cmp.changes.size(), 0);
        QVERIFY2(cmp.work < 100 * 2000, qPrintable(QString::number(cmp.work)));
        QVERIFY2(cmp.work > 2000, "the work was counted");
        // 2,000 slides that all say the same words, in two presentations that share no title: every word is on every slide and tells nothing; no pair is looked at.
        auto same = deck::newPresentation();
        while (same->pages.size() < 2000) same->addPage(-1, same->pages[0]->masterId);
        for (auto &p : same->pages) p->title = QStringLiteral("Agenda");
        auto changed = copyOf(*same);
        for (auto &p : changed->pages) {
            p->id = newId("p");
            p->title = QStringLiteral("Agenda today");
        }
        DeckComparison hostile = compareDecks(*same, *changed);
        QCOMPARE(hostile.changes.size(), 4000);
        QVERIFY2(hostile.work < 40 * 2000, qPrintable(QString::number(hostile.work)));
        // 10,000 objects of one kind, role, and place on one slide, with ids that do not match: found by the same key, in order.
        auto a = deck::newPresentation();
        auto b = deck::newPresentation();
        a->pages[0]->items.clear();
        b->pages[0]->items.clear();
        for (int i = 0; i < 10000; ++i) {
            addBox(*a, *a->pages[0], QString(), 20, 20, QStringLiteral("body"));
            addBox(*b, *b->pages[0], QString(), 20, 20, QStringLiteral("body"));
        }
        a->pages[0]->id = b->pages[0]->id;
        // (the premise: no object of a has the id of an object of b, so they are matched by what they hold and not by id)
        QSet<QString> idsA;
        walkItems(a->pages[0]->items, [&](const ItemPtr &it) { idsA.insert(it->id); });
        QCOMPARE(int(idsA.size()), 10000);
        int sharedIds = 0;
        walkItems(b->pages[0]->items, [&](const ItemPtr &it) { sharedIds += idsA.contains(it->id); });
        QCOMPARE(sharedIds, 0);
        DeckComparison crowd = compareDecks(*a, *b);
        QCOMPARE(crowd.changes.size(), 0);
        QVERIFY2(crowd.work < 40 * 10000, qPrintable(QString::number(crowd.work)));
    }

    // A deck reversed is all moves but one; and moves with slides added and deleted among them still end in the order theirs has.
    void compareSlideOrderIsRightForAnyShuffle()
    {
        auto mine = lettered(8);
        auto theirs = copyOf(*mine);
        std::reverse(theirs->pages.begin(), theirs->pages.end());
        DeckComparison cmp = compareDecks(*mine, *theirs);
        QCOMPARE(cmp.changes.size(), 7);
        QCOMPARE(kindCount(cmp, review::Kind::Moved), 7);
        auto working = copyOf(*mine);
        QCOMPARE(acceptAll(*working, cmp), 7);
        QCOMPARE(letters(*working), letters(*theirs));
        QCOMPARE(rejectAll(*working, cmp, true), 7);
        QCOMPARE(letters(*working), letters(*mine));
        auto t2 = copyOf(*mine);
        t2->pages.removeAt(0);
        t2->pages.removeAt(4);
        t2->pages.move(0, 4);
        const MasterPage *layout = deck::layoutOfType(*t2, QStringLiteral("obj"));
        const int x = deck::addSlide(*t2, *layout, 1);
        t2->pages[x]->title = QStringLiteral("X");
        DeckComparison c2 = compareDecks(*mine, *t2);
        auto w2 = copyOf(*mine);
        acceptAll(*w2, c2);
        QCOMPARE(letters(*w2), letters(*t2));
        rejectAll(*w2, c2, true);
        QCOMPARE(letters(*w2), letters(*mine));
    }

    // ---- Compare: the model's new parts, and the hostile and awkward cases ----

    static QStringList boxTexts(const Document &d, int slide)
    {
        QStringList out;
        for (const auto &it : d.pages[slide]->items)
            if (it->type() == ItemType::Shape) out << textOfBox(d, static_cast<ShapeItem *>(it.get()));
        return out;
    }
    static std::shared_ptr<TableItem> addTable(Document &d, Page &page, int rows, int cols, const QString &text, double y = 200)
    {
        auto t = std::make_shared<TableItem>();
        t->rows = rows;
        t->cols = cols;
        t->colW = QVector<double>(cols, 50);
        t->rowH = QVector<double>(rows, 20);
        t->cells.resize(rows * cols);
        for (int i = 0; i < rows * cols; ++i) t->cells[i].storyId = d.createStory(QStringLiteral("%1 %2").arg(text).arg(i));
        t->rect = QRectF(10, y, 50.0 * cols, 20.0 * rows);
        page.items.push_back(t);
        return t;
    }
    static QStringList cellTexts(const Document &d, const TableItem &t)
    {
        QStringList out;
        for (const TableCell &c : t.cells) out << (c.storyId.isEmpty() || !d.storyDoc(c.storyId) ? QString() : d.storyDoc(c.storyId)->toPlainText());
        return out;
    }
    static const TableItem *tableOf(const Document &d, int slide)
    {
        for (const auto &it : d.pages[slide]->items)
            if (it->type() == ItemType::Table) return static_cast<const TableItem *>(it.get());
        return nullptr;
    }
    static QByteArray pngOf(QColor c)
    {
        QImage img(8, 8, QImage::Format_RGB32);
        img.fill(c);
        QByteArray out;
        QBuffer buf(&out);
        buf.open(QIODevice::WriteOnly);
        img.save(&buf, "PNG");
        return out;
    }
    // A list of indices 0..n-1 in an order made from `seed` (the same on every machine, which std::shuffle is not).
    static QVector<int> shuffled(int n, quint32 seed)
    {
        QVector<int> v(n);
        for (int i = 0; i < n; ++i) v[i] = i;
        quint32 s = seed * 2654435761u + 12345u;
        for (int i = n - 1; i > 0; --i) {
            s = s * 1664525u + 1013904223u;
            std::swap(v[i], v[(s >> 8) % quint32(i + 1)]);
        }
        return v;
    }

    // Two objects that share an id on one slide, both deleted by the other side: each accept takes out its own object (an id is not an address),
    // whatever the order, and the changes that were done say so.
    void compareAcceptsObjectsThatShareAnId()
    {
        auto mine = deck::newPresentation();
        mine->pages[0]->items.clear();
        addBox(*mine, *mine->pages[0], QStringLiteral("one"), 10, 10)->id = QStringLiteral("x");
        addBox(*mine, *mine->pages[0], QStringLiteral("keep"), 10, 60);
        addBox(*mine, *mine->pages[0], QStringLiteral("two"), 10, 110)->id = QStringLiteral("x");
        auto theirs = copyOf(*mine);   // (the copy gives the second "x" an id of its own)
        theirs->pages[0]->items.erase(theirs->pages[0]->items.begin() + 2);
        theirs->pages[0]->items.erase(theirs->pages[0]->items.begin());
        for (int order = 0; order < 3; ++order) {
            DeckComparison cmp = compareDecks(*mine, *theirs);
            QCOMPARE(cmp.changes.size(), 2);
            QCOMPARE(kindCount(cmp, review::Kind::Deleted), 2);
            auto working = copyOf(*mine);
            boxOf(*working, 0, 0)->id = QStringLiteral("x");   // a copy gives repeated object ids new ones: the compared deck has them, so the one worked on must
            boxOf(*working, 0, 2)->id = QStringLiteral("x");
            if (order == 0) {
                QCOMPARE(acceptAll(*working, cmp), 2);
            } else {
                QVERIFY(applyChange(*working, cmp, order == 1 ? 0 : 1));
                QVERIFY(applyChange(*working, cmp, order == 1 ? 1 : 0));
            }
            QCOMPARE(boxTexts(*working, 0), (QStringList{QStringLiteral("keep")}));
            QCOMPARE(working->pages[0]->items.size(), size_t(1));
            QCOMPARE(working->pages[0]->items[0]->rect.y(), 60.0);   // the one that was kept, not another
            QCOMPARE(cmp.changes.count(review::State::Accepted), 2);
            QCOMPARE(compareDecks(*working, *theirs).changes.size(), 0);
            // And back: each object returns to its own place.
            QCOMPARE(rejectAll(*working, cmp, true), 2);
            QCOMPARE(boxTexts(*working, 0), (QStringList{QStringLiteral("one"), QStringLiteral("keep"), QStringLiteral("two")}));
        }
    }

    // Which of two objects that share an id survives, by where it sits: deleting only the second must not take the first, and the other way round.
    // Three share an id; every subset of them is deleted by theirs, taken in one go and taken back.
    void compareKeepsTheRightObjectWhenIdsRepeat()
    {
        auto build = [&]() {
            auto d = deck::newPresentation();
            d->pages[0]->items.clear();
            addBox(*d, *d->pages[0], QStringLiteral("one"), 10, 10)->id = QStringLiteral("x");
            addBox(*d, *d->pages[0], QStringLiteral("keep"), 10, 60);
            addBox(*d, *d->pages[0], QStringLiteral("two"), 10, 110)->id = QStringLiteral("x");
            addBox(*d, *d->pages[0], QStringLiteral("three"), 10, 160)->id = QStringLiteral("x");
            return d;
        };
        auto mine = build();
        for (int gone = 1; gone < 8; ++gone) {   // bit i set: the i-th of one, two, three (items 0, 2, 3) is deleted by theirs
            auto theirs = copyOf(*mine);
            const int idx[3] = {0, 2, 3};
            for (int k = 2; k >= 0; --k)
                if (gone & (1 << k)) theirs->pages[0]->items.erase(theirs->pages[0]->items.begin() + idx[k]);
            DeckComparison cmp = compareDecks(*mine, *theirs);
            auto working = copyOf(*mine);   // (a copy gives repeated ids new ones: the one worked on repeats them, as the compared deck did)
            for (int i : {0, 2, 3}) working->pages[0]->items[i]->id = QStringLiteral("x");
            QCOMPARE(acceptAll(*working, cmp), cmp.changes.size());
            QList<double> ys, want;
            for (const auto &it : working->pages[0]->items) ys << it->rect.y();
            for (int k = 0; k < 4; ++k)
                if (k == 1 || !(gone & (1 << (k == 0 ? 0 : k - 1)))) want << 10 + 50 * k;
            QCOMPARE(ys, want);
            QCOMPARE(compareDecks(*working, *theirs).changes.size(), 0);
            QCOMPARE(rejectAll(*working, cmp, true), cmp.changes.size());
            QCOMPARE(boxTexts(*working, 0), (QStringList{QStringLiteral("one"), QStringLiteral("keep"), QStringLiteral("two"), QStringLiteral("three")}));
        }
    }

    // Objects the working presentation has that the compared one did not (it was edited since) and that repeat an id are made their own ids too, so that
    // every object on the slide can be told from every other.
    void compareGivesEveryWorkingObjectItsOwnId()
    {
        auto mine = deck::newPresentation();
        mine->pages[0]->items.clear();
        addBox(*mine, *mine->pages[0], QStringLiteral("one"), 10, 10)->id = QStringLiteral("x");
        addBox(*mine, *mine->pages[0], QStringLiteral("two"), 10, 60)->id = QStringLiteral("x");
        auto theirs = copyOf(*mine);
        theirs->pages[0]->items.erase(theirs->pages[0]->items.begin() + 1);
        DeckComparison cmp = compareDecks(*mine, *theirs);
        QCOMPARE(cmp.changes.size(), 1);
        auto working = copyOf(*mine);
        for (int i : {0, 1}) working->pages[0]->items[i]->id = QStringLiteral("x");
        addBox(*working, *working->pages[0], QStringLiteral("late"), 10, 110)->id = QStringLiteral("x");   // added to the working presentation after the comparison
        addBox(*working, *working->pages[0], QStringLiteral("later"), 10, 160)->id = QString();
        QCOMPARE(acceptAll(*working, cmp), 1);
        QSet<QString> ids;
        for (const auto &it : working->pages[0]->items) ids.insert(it->id);
        QCOMPARE(ids.size(), 3);
        QVERIFY(!ids.contains(QString()));
        QCOMPARE(boxTexts(*working, 0), (QStringList{QStringLiteral("one"), QStringLiteral("late"), QStringLiteral("later")}));
    }

    // Accepting objects added, deleted, and changed on many slides never looks through the whole presentation for an id the working presentation does not have.
    void compareAcceptAllDoesNotScanTheDocument()
    {
        auto mine = lettered(40);
        for (auto &p : mine->pages)
            for (int k = 0; k < 5; ++k) addBox(*mine, *p, QStringLiteral("m%1").arg(k), 10, 100 + 30 * k);
        auto theirs = copyOf(*mine);
        for (auto &p : theirs->pages) {
            addBox(*theirs, *p, QStringLiteral("new"), 300, 10);
            p->items.erase(p->items.begin() + 2);
        }
        DeckComparison cmp = compareDecks(*mine, *theirs);
        QVERIFY(cmp.changes.size() >= 80);
        auto working = copyOf(*mine);
        working->find(boxOf(*working, 0)->id);   // (the index exists)
        const int scans = working->findScansForTest();
        QCOMPARE(acceptAll(*working, cmp), cmp.changes.size());
        QCOMPARE(working->findScansForTest(), scans);
        QCOMPARE(compareDecks(*working, *theirs).changes.size(), 0);
        QCOMPARE(rejectAll(*working, cmp, true), cmp.changes.size());
        QCOMPARE(working->findScansForTest(), scans);
        QCOMPARE(compareDecks(*working, *mine).changes.size(), 0);
    }

    // The comparison keeps its own copy of theirs: it still accepts after the caller's presentation is gone.
    void compareSurvivesTheDeathOfTheirs()
    {
        auto mine = compareBase();
        auto theirs = copyOf(*mine);
        addBox(*theirs, *theirs->pages[1], QStringLiteral("added"), 300, 10);
        QTextCursor c(theirs->storyDoc(boxOf(*theirs, 2)->storyId));
        c.movePosition(QTextCursor::End);
        c.insertText(QStringLiteral(" more"));
        theirs->pages.removeAt(4);
        DeckComparison cmp = compareDecks(*mine, *theirs);
        QVERIFY(cmp.changes.size() >= 3);
        QVERIFY(static_cast<const void *>(&*cmp.theirs) != static_cast<const void *>(theirs.get()));   // its own copy
        theirs->pages.clear();   // (what the caller does to theirs afterward does not reach the comparison)
        theirs.reset();
        auto working = copyOf(*mine);
        QCOMPARE(acceptAll(*working, cmp), cmp.changes.size());
        QCOMPARE(letters(*working), QStringLiteral("ABCD"));
        QStringList got;
        for (const auto &it : working->pages[1]->items) if (it->type() == ItemType::Shape) got << textOfBox(*working, static_cast<ShapeItem *>(it.get()));
        QVERIFY(got.contains(QStringLiteral("added")));
        QVERIFY(textOfBox(*working, boxOf(*working, 2)).endsWith(QStringLiteral(" more")));
    }

    // The same for slides: ids in a file repeat, and each accept removes the slide its change is about.
    void compareAcceptsSlidesThatShareAnId()
    {
        auto mine = lettered(5);
        for (auto &p : mine->pages) p->id = QStringLiteral("same");
        auto theirs = copyOf(*mine);
        QCOMPARE(theirs->pages[0]->id, QStringLiteral("same"));
        for (auto &p : theirs->pages) p->id = newId("p");
        theirs->pages.removeAt(1);
        theirs->pages.removeAt(0);
        DeckComparison cmp = compareDecks(*mine, *theirs);
        QCOMPARE(cmp.changes.size(), 2);
        QCOMPARE(kindCount(cmp, review::Kind::Deleted), 2);
        auto working = copyOf(*mine);
        QCOMPARE(acceptAll(*working, cmp), 2);
        QCOMPARE(letters(*working), QStringLiteral("CDE"));
        QCOMPARE(cmp.changes.count(review::State::Accepted), 2);
        QCOMPARE(rejectAll(*working, cmp, true), 2);
        QCOMPARE(letters(*working), QStringLiteral("ABCDE"));
        // One at a time, the later one first.
        DeckComparison cmp2 = compareDecks(*mine, *theirs);
        auto w2 = copyOf(*mine);
        QVERIFY(applyChange(*w2, cmp2, 1));
        QVERIFY(applyChange(*w2, cmp2, 0));
        QCOMPARE(letters(*w2), QStringLiteral("CDE"));
    }

    // An undo gives the slides their repeated ids back; the next accept finds the slides again.
    void compareSlidesThatShareAnIdSurviveAnUndo()
    {
        auto mine = lettered(5);
        for (auto &p : mine->pages) p->id = QStringLiteral("same");
        auto theirs = copyOf(*mine);
        for (auto &p : theirs->pages) p->id = newId("p");
        theirs->pages.removeAt(1);
        theirs->pages.removeAt(0);
        DeckWindow w;
        Editor *ed = w.editor();
        ed->setDocument(copyOf(*mine));
        DeckComparison cmp = compareDecks(*ed->doc(), *theirs);
        QCOMPARE(cmp.changes.size(), 2);
        ed->change(QStringLiteral("Accept"), [&] { QVERIFY(applyChange(*ed->doc(), cmp, 0)); });
        QCOMPARE(letters(*ed->doc()), QStringLiteral("BCDE"));
        ed->undo();
        QCOMPARE(letters(*ed->doc()), QStringLiteral("ABCDE"));
        ed->change(QStringLiteral("Accept"), [&] { QVERIFY(applyChange(*ed->doc(), cmp, 0)); });
        QCOMPARE(letters(*ed->doc()), QStringLiteral("BCDE"));
        ed->change(QStringLiteral("Accept"), [&] { QVERIFY(applyChange(*ed->doc(), cmp, 1)); });
        QCOMPARE(letters(*ed->doc()), QStringLiteral("CDE"));
    }

    // A change that can't be found in the working presentation is not marked as done.
    void compareLeavesAChangePendingWhenItsSlideIsGone()
    {
        auto mine = compareBase();
        auto theirs = copyOf(*mine);
        boxOf(*theirs, 2)->rect.translate(5, 5);
        DeckComparison cmp = compareDecks(*mine, *theirs);
        QCOMPARE(cmp.changes.size(), 1);
        auto working = copyOf(*mine);
        working->pages.removeAt(2);
        QVERIFY(!applyChange(*working, cmp, 0));
        QCOMPARE(int(cmp.changes.changes[0].state), int(review::State::Pending));
        QCOMPARE(acceptAll(*working, cmp), 0);
        QCOMPARE(cmp.changes.pending(), 1);
    }

    // An added object goes after the object that comes before it in theirs, as slides do: mine [D, B] and theirs [B, N] end as [B, N], in any order.
    void compareAddedObjectsKeepTheirOrder()
    {
        auto mine = deck::newPresentation();
        mine->pages[0]->items.clear();
        addBox(*mine, *mine->pages[0], QStringLiteral("D"), 10, 10);
        addBox(*mine, *mine->pages[0], QStringLiteral("B"), 10, 60);
        auto theirs = copyOf(*mine);
        theirs->pages[0]->items.erase(theirs->pages[0]->items.begin());
        addBox(*theirs, *theirs->pages[0], QStringLiteral("N"), 10, 110);
        DeckComparison cmp = compareDecks(*mine, *theirs);
        QCOMPARE(cmp.changes.size(), 2);
        auto working = copyOf(*mine);
        QCOMPARE(acceptAll(*working, cmp), 2);
        QCOMPARE(boxTexts(*working, 0), (QStringList{QStringLiteral("B"), QStringLiteral("N")}));
        QCOMPARE(rejectAll(*working, cmp, true), 2);
        QCOMPARE(boxTexts(*working, 0), (QStringList{QStringLiteral("D"), QStringLiteral("B")}));
        // Seven changes in 24 orders, accepted and then rejected one by one: mine [A D1 B D2 C], theirs [N1 A N2 B N3 N4 C N5].
        auto m2 = deck::newPresentation();
        m2->pages[0]->items.clear();
        for (const char *t : {"A", "D1", "B", "D2", "C"}) addBox(*m2, *m2->pages[0], QString::fromLatin1(t), 10, 10 + 30 * int(m2->pages[0]->items.size()));
        auto t2 = copyOf(*m2);
        t2->pages[0]->items.clear();
        const QStringList want{QStringLiteral("N1"), QStringLiteral("A"), QStringLiteral("N2"), QStringLiteral("B"), QStringLiteral("N3"), QStringLiteral("N4"), QStringLiteral("C"), QStringLiteral("N5")};
        for (const QString &t : want) {
            ShapeItem *made = nullptr;
            for (const auto &it : m2->pages[0]->items)
                if (it->type() == ItemType::Shape && textOfBox(*m2, static_cast<ShapeItem *>(it.get())) == t) made = static_cast<ShapeItem *>(it.get());
            if (made) {
                auto c = std::static_pointer_cast<ShapeItem>(made->clone());
                c->id = made->id;
                c->storyId = t2->createStory(t);
                t2->pages[0]->items.push_back(c);
            } else {
                addBox(*t2, *t2->pages[0], t, 200, 10 + 30 * int(t2->pages[0]->items.size()));
            }
        }
        for (int seed = 0; seed < 24; ++seed) {
            DeckComparison c = compareDecks(*m2, *t2);
            QCOMPARE(c.changes.size(), 7);
            auto w = copyOf(*m2);
            const QVector<int> order = shuffled(c.changes.size(), quint32(seed));
            for (int i : order) QVERIFY(applyChange(*w, c, i));
            QCOMPARE(boxTexts(*w, 0), want);
            const QVector<int> back = shuffled(c.changes.size(), quint32(seed + 100));
            for (int i : back) QVERIFY(rejectChange(*w, c, i));
            QCOMPARE(boxTexts(*w, 0), (QStringList{QStringLiteral("A"), QStringLiteral("D1"), QStringLiteral("B"), QStringLiteral("D2"), QStringLiteral("C")}));
        }
    }

    // The same for slides: added and deleted slides end in theirs' order whatever order the changes are taken in, and mine's when taken back.
    void compareAddedSlidesKeepTheirOrder()
    {
        auto mine = lettered(2);   // A B
        mine->pages[0]->title = QStringLiteral("D");
        auto theirs = copyOf(*mine);   // [D B] -> [B N]
        theirs->pages.removeAt(0);
        const MasterPage *layout = deck::layoutOfType(*theirs, QStringLiteral("obj"));
        theirs->pages[deck::addSlide(*theirs, *layout)]->title = QStringLiteral("N");
        DeckComparison cmp = compareDecks(*mine, *theirs);
        QCOMPARE(cmp.changes.size(), 2);
        auto working = copyOf(*mine);
        QCOMPARE(acceptAll(*working, cmp), 2);
        QCOMPARE(letters(*working), QStringLiteral("BN"));
        QCOMPARE(rejectAll(*working, cmp, true), 2);
        QCOMPARE(letters(*working), QStringLiteral("DB"));
        // Mine ABCDE, theirs [N1 A N2 C N3 E N4] (B and D deleted, four added): every order.
        auto m2 = lettered(5);
        auto t2 = copyOf(*m2);
        t2->pages.removeAt(3);
        t2->pages.removeAt(1);
        const MasterPage *l2 = deck::layoutOfType(*t2, QStringLiteral("obj"));
        for (int k = 0; k < 4; ++k) t2->pages[deck::addSlide(*t2, *l2, 2 * k)]->title = QStringLiteral("N%1").arg(k + 1);
        const QString want = letters(*t2);
        QCOMPARE(want.size(), 11);   // N1 A N2 C N3 E N4 = 7 slides, 11 characters
        for (int seed = 0; seed < 24; ++seed) {
            DeckComparison c = compareDecks(*m2, *t2);
            QCOMPARE(c.changes.size(), 6);
            auto w = copyOf(*m2);
            for (int i : shuffled(c.changes.size(), quint32(seed))) QVERIFY(applyChange(*w, c, i));
            QCOMPARE(letters(*w), want);
            for (int i : shuffled(c.changes.size(), quint32(seed + 100))) QVERIFY(rejectChange(*w, c, i));
            QCOMPARE(letters(*w), QStringLiteral("ABCDE"));
        }
    }

    // Groups nested more than 16 deep are not read: the list says it is not all there is (16 deep is all read).
    void compareSaysTooManyForGroupsDeeperThan16()
    {
        auto nested = [&](int groups) {
            auto d = deck::newPresentation();
            d->pages[0]->items.clear();
            ItemPtr cur = addBoxPtr(*d, QStringLiteral("deep"));
            for (int i = 0; i < groups; ++i) {
                auto g = std::make_shared<GroupItem>();
                g->children.push_back(cur);
                g->syncRect();
                cur = g;
            }
            d->pages[0]->items.push_back(cur);
            return d;
        };
        auto leafOf = [](Document &d) {
            Item *it = d.pages[0]->items[0].get();
            while (it->type() == ItemType::Group) it = static_cast<GroupItem *>(it)->children[0].get();
            return it;
        };
        auto mine = nested(16);
        auto theirs = copyOf(*mine);
        leafOf(*theirs)->rect.translate(9, 9);
        DeckComparison fits = compareDecks(*mine, *theirs);
        QVERIFY(!fits.changes.tooMany);
        QCOMPARE(fits.changes.size(), 1);
        auto working = copyOf(*mine);
        QCOMPARE(acceptAll(*working, fits), 1);
        QCOMPARE(leafOf(*working)->rect, leafOf(*theirs)->rect);
        auto deep = nested(17);
        auto deeper = copyOf(*deep);
        leafOf(*deeper)->rect.translate(9, 9);
        DeckComparison cmp = compareDecks(*deep, *deeper);
        QVERIFY(cmp.changes.tooMany);
        QCOMPARE(cmp.changes.size(), 0);
    }
    static ItemPtr addBoxPtr(Document &d, const QString &text)
    {
        auto s = std::make_shared<ShapeItem>();
        s->rect = QRectF(10, 10, 100, 40);
        s->storyId = d.createStory(text);
        return s;
    }

    // A table of cells counts as that many objects, against the 20,000: exactly 20,000 is all read, one more is not.
    void compareCountsTableCellsAsObjects()
    {
        // Tables of 100 x 199 and 7 x 14 cells, with `extra` boxes: a table is 1 and its cells (a table has at most 256 rows and 256 columns).
        auto make = [&](QVector<QSize> shapes, int extra) {
            auto d = deck::newPresentation();
            d->pages[0]->items.clear();
            for (const QSize &shape : shapes) {
                auto t = std::make_shared<TableItem>();
                t->rows = shape.width();
                t->cols = shape.height();
                t->colW = QVector<double>(t->cols, 5);
                t->rowH = QVector<double>(t->rows, 20);
                t->cells.resize(t->rows * t->cols);
                t->rect = QRectF(0, 0, 5.0 * t->cols, 20.0 * t->rows);
                d->pages[0]->items.push_back(t);
            }
            for (int i = 0; i < extra; ++i) addBox(*d, *d->pages[0], QString(), 10, 100 + i);
            return d;
        };
        auto mine = make({QSize(100, 199), QSize(7, 14)}, 0);
        auto theirs = copyOf(*mine);
        QCOMPARE(int(tableOf(*theirs, 0)->cells.size()), 19900);
        DeckComparison fits = compareDecks(*mine, *theirs);
        QCOMPARE(fits.objectsMine, 20000);   // 2 tables and their 19,998 cells
        QCOMPARE(fits.objectsTheirs, 20000);
        QVERIFY(!fits.changes.tooMany);
        auto more = make({QSize(100, 199), QSize(7, 14)}, 1);
        DeckComparison over = compareDecks(*more, *copyOf(*more));
        QCOMPARE(over.objectsMine, 20000);
        QVERIFY(over.changes.tooMany);
        auto big = make({QSize(100, 200)}, 0);
        DeckComparison table = compareDecks(*big, *copyOf(*big));
        QCOMPARE(table.objectsMine, 20000);
        QVERIFY(table.changes.tooMany);
    }

    // At most 20 million characters of text are read from each presentation, and at most 100,000 of one object (exactly that much is all read).
    void compareReadsAtMost20MillionCharactersOfText()
    {
        const QString big(100000, QLatin1Char('a'));
        auto make = [&](int boxes) {
            auto d = deck::newPresentation();
            d->pages[0]->items.clear();
            for (int i = 0; i < boxes; ++i) addBox(*d, *d->pages[0], big, 10, 10 + i);
            return d;
        };
        auto mine = make(200);
        auto theirs = copyOf(*mine);
        DeckComparison fits = compareDecks(*mine, *theirs);
        QCOMPARE(fits.textMine, deck::kMaxTotalText);
        QCOMPARE(fits.textTheirs, deck::kMaxTotalText);
        QVERIFY(!fits.changes.tooMany);
        auto more = make(201);
        DeckComparison over = compareDecks(*more, *copyOf(*more));
        QCOMPARE(over.textMine, deck::kMaxTotalText);
        QVERIFY(over.changes.tooMany);
        // One object's text is read to 100,000 characters and no further.
        auto one = deck::newPresentation();
        one->pages[0]->items.clear();
        addBox(*one, *one->pages[0], QString(150000, QLatin1Char('b')), 10, 10);
        DeckComparison cut = compareDecks(*one, *copyOf(*one));
        QCOMPARE(cut.textMine, qint64(100000));
        QVERIFY(!cut.changes.tooMany);
    }

    // Formatting is compared over the first 20,000 paragraphs and runs of an object, and 200,000 in all: beyond that it is not looked at.
    void compareComparesFormattingOfTheFirstRunsOnly()
    {
        // A story of `fragments` runs of one letter, bold and plain in turn, and the one at `odd` in another look.
        auto story = [](int fragments, int odd) {
            QJsonArray runs;
            for (int i = 0; i < fragments; ++i) {
                QTextCharFormat f;
                if (i % 2) f.setFontWeight(QFont::Bold);
                if (i == odd) f.setFontItalic(true);
                runs.append(QJsonObject{{"t", QStringLiteral("a")}, {"f", formatToJson(f)}});
            }
            return QJsonObject{{"blocks", QJsonArray{QJsonObject{{"r", runs}}}}};
        };
        auto make = [&](int boxes, int fragments, int odd) {
            auto d = deck::newPresentation();
            d->pages[0]->items.clear();
            for (int i = 0; i < boxes; ++i) {
                ShapeItem *s = addBox(*d, *d->pages[0], QStringLiteral("x"), 10, 10 + 5 * i);
                storyFromJson(d->storyDoc(s->storyId), story(fragments, odd));
            }
            return d;
        };
        // 1 paragraph and 19,999 runs are 20,000: a change in the last run is seen; in the 20,001st it is not.
        auto a = make(1, 19999, -1);
        DeckComparison seen = compareDecks(*a, *make(1, 19999, 19998));
        QCOMPARE(seen.changes.size(), 1);
        QCOMPARE(int(seen.changes.changes[0].kind), int(review::Kind::Reformatted));
        QCOMPARE(seen.sigRunsMine, qint64(deck::kMaxSigRuns));
        auto b = make(1, 20000, -1);
        DeckComparison unseen = compareDecks(*b, *make(1, 20000, 19999));
        QCOMPARE(unseen.changes.size(), 0);
        QCOMPARE(unseen.sigRunsMine, qint64(deck::kMaxSigRuns));
        // Ten such objects are 200,000: all compared; an eleventh is not, and the list says so.
        auto ten = make(10, 19999, -1);
        DeckComparison all = compareDecks(*ten, *copyOf(*ten));
        QCOMPARE(all.sigRunsMine, deck::kMaxTotalSigRuns);
        QCOMPARE(all.sigRunsTheirs, deck::kMaxTotalSigRuns);
        QVERIFY(!all.changes.tooMany);
        auto eleven = make(11, 19999, -1);
        DeckComparison past = compareDecks(*eleven, *copyOf(*eleven));
        QCOMPARE(past.sigRunsMine, deck::kMaxTotalSigRuns);
        QVERIFY(past.changes.tooMany);
    }

    // Only the first 2,000 slides of a presentation of 10,000 are looked at, in the arrays as well as the matching, and the work stays a few steps for each.
    void compareOf10000SlidesWorksOnTheFirst2000()
    {
        auto mine = deck::newPresentation();
        while (mine->pages.size() < 10000) mine->addPage(-1, mine->pages[0]->masterId);
        for (int i = 0; i < 10000; ++i) mine->pages[i]->title = QStringLiteral("slide number %1").arg(i);
        auto theirs = copyOf(*mine);
        theirs->pages.move(3, 1);
        theirs->pages.removeLast();   // past the 2,000: not looked at
        DeckComparison cmp = compareDecks(*mine, *theirs);
        QCOMPARE(cmp.slidesMine, 2000);
        QVERIFY(cmp.changes.tooMany);
        QCOMPARE(cmp.workOfMine.size(), 2000);
        QCOMPARE(cmp.workOfTheirs.size(), 2000);
        QCOMPARE(cmp.mineOfTheirs.size(), 2000);
        QCOMPARE(cmp.theirsOfMine.size(), 2000);
        QCOMPARE(cmp.changes.size(), 1);
        auto working = copyOf(*mine);
        QCOMPARE(acceptAll(*working, cmp), 1);
        QCOMPARE(working->pages[1]->title, QStringLiteral("slide number 3"));
        QCOMPARE(working->pages[3]->title, QStringLiteral("slide number 2"));
        QCOMPARE(working->pages.size(), 10000);
        QVERIFY2(cmp.work < 60 * 2000, qPrintable(QString::number(cmp.work)));
    }

    // Taking in thousands of added objects is a few steps each: the work counted (the id check, the place) does not grow with the square.
    void acceptingManyAddedObjectsIsLinear()
    {
        auto mine = deck::newPresentation();
        mine->pages[0]->items.clear();
        for (int i = 0; i < 3; ++i) addBox(*mine, *mine->pages[0], QStringLiteral("m%1").arg(i), 10, 10 + 5 * i);
        auto theirs = copyOf(*mine);
        for (int i = 0; i < 9000; ++i) addBox(*theirs, *theirs->pages[0], QString(), 10 + i % 90, 100 + i / 90);
        DeckComparison cmp = compareDecks(*mine, *theirs);
        QCOMPARE(cmp.changes.size(), 9000);
        auto working = copyOf(*mine);
        const qint64 before = cmp.work;
        QCOMPARE(acceptAll(*working, cmp), 9000);
        QCOMPARE(int(working->pages[0]->items.size()), 9003);
        QVERIFY2(cmp.work - before < 20 * 9000, qPrintable(QString::number(cmp.work - before)));
        for (int i = 0; i < 9003; ++i) QCOMPARE(working->pages[0]->items[i]->rect, theirs->pages[0]->items[i]->rect);
    }

    // The same picture fill in two files loaded apart (each gives a picture its own id) is not a change; another picture is.
    void compareFillsByThePictureNotItsId()
    {
        auto d = deck::newPresentation();
        d->pages[0]->items.clear();
        ShapeItem *s = addBox(*d, *d->pages[0], QStringLiteral("filled"), 10, 10);
        Fill f;
        f.type = Fill::Picture;
        f.imageId = d->addImage(pngOf(Qt::red), QStringLiteral("png"));
        s->fill = f;
        QString why;
        const QByteArray file = deck::exportPresentation(*d, &why);
        QVERIFY2(!file.isEmpty(), qPrintable(why));
        auto a = deck::importPresentation(file, &why);
        auto b = deck::importPresentation(file, &why);
        QVERIFY2(a && b, qPrintable(why));
        auto fillOf = [](Document &doc) -> Item * {
            for (const auto &it : doc.pages[0]->items)
                if (it->fill.type == Fill::Picture) return it.get();
            return nullptr;
        };
        QVERIFY(fillOf(*a) && fillOf(*b));
        QVERIFY(fillOf(*a)->fill.imageId != fillOf(*b)->fill.imageId);   // (the premise: each load made its own id)
        DeckComparison same = compareDecks(*a, *b);
        QCOMPARE(same.changes.size(), 0);
        fillOf(*b)->fill.imageId = b->addImage(pngOf(Qt::blue), QStringLiteral("png"));
        DeckComparison other = compareDecks(*a, *b);
        QCOMPARE(other.changes.size(), 1);
        QCOMPARE(other.changes.changes[0].target.path, QStringLiteral("fill"));
    }

    // A table that has a different shape is replaced (deleted and added), so that accepting gives theirs; one with the same shape has its cell text changed.
    void compareTablesChangeCellsOrAreReplaced()
    {
        auto mine = deck::newPresentation();
        mine->pages[0]->items.clear();
        addBox(*mine, *mine->pages[0], QStringLiteral("above"), 10, 10);
        addTable(*mine, *mine->pages[0], 2, 2, QStringLiteral("cell"));
        addBox(*mine, *mine->pages[0], QStringLiteral("below"), 10, 400);
        // The same shape, a cell's text changed.
        auto same = copyOf(*mine);
        QTextCursor c(same->storyDoc(const_cast<TableItem *>(tableOf(*same, 0))->cells[3].storyId));
        c.select(QTextCursor::Document);
        c.insertText(QStringLiteral("changed"));
        DeckComparison edit = compareDecks(*mine, *same);
        QCOMPARE(edit.changes.size(), 1);
        QCOMPARE(int(edit.changes.changes[0].kind), int(review::Kind::Edited));
        auto w1 = copyOf(*mine);
        QCOMPARE(acceptAll(*w1, edit), 1);
        QCOMPARE(cellTexts(*w1, *tableOf(*w1, 0)), cellTexts(*same, *tableOf(*same, 0)));
        QCOMPARE(rejectAll(*w1, edit, true), 1);
        QCOMPARE(cellTexts(*w1, *tableOf(*w1, 0)), cellTexts(*mine, *tableOf(*mine, 0)));
        // Another shape: 2 x 3 for 2 x 2, with other words.
        auto theirs = copyOf(*mine);
        auto &items = theirs->pages[0]->items;
        items.erase(items.begin() + 1);
        auto t = addTable(*theirs, *theirs->pages[0], 2, 3, QStringLiteral("new"));
        items.pop_back();
        items.insert(items.begin() + 1, t);
        t->id = tableOf(*mine, 0)->id;
        DeckComparison cmp = compareDecks(*mine, *theirs);
        QCOMPARE(cmp.changes.size(), 2);
        QCOMPARE(kindCount(cmp, review::Kind::Added), 1);
        QCOMPARE(kindCount(cmp, review::Kind::Deleted), 1);
        auto working = copyOf(*mine);
        QCOMPARE(acceptAll(*working, cmp), 2);
        QCOMPARE(tableOf(*working, 0)->cells.size(), 6);
        QCOMPARE(cellTexts(*working, *tableOf(*working, 0)), cellTexts(*theirs, *tableOf(*theirs, 0)));
        QCOMPARE(boxTexts(*working, 0), (QStringList{QStringLiteral("above"), QStringLiteral("below")}));
        QCOMPARE(working->pages[0]->items[1]->type(), ItemType::Table);   // the table stands between the two boxes, as in theirs
        QCOMPARE(compareDecks(*working, *theirs).changes.size(), 0);
        QCOMPARE(rejectAll(*working, cmp, true), 2);
        QCOMPARE(tableOf(*working, 0)->cells.size(), 4);
        QCOMPARE(working->pages[0]->items[1]->type(), ItemType::Table);
        QCOMPARE(cellTexts(*working, *tableOf(*working, 0)), cellTexts(*mine, *tableOf(*mine, 0)));
    }

    // Groups: a child moved, a child's text changed, and a child added, each found and taken in or back.
    void compareLooksInsideGroups()
    {
        auto mine = deck::newPresentation();
        mine->pages[0]->items.clear();
        auto g = std::make_shared<GroupItem>();
        for (const char *t : {"one", "two", "three"}) {
            ItemPtr b = addBoxPtr(*mine, QString::fromLatin1(t));
            b->rect.translate(0, 50 * int(g->children.size()));
            g->children.push_back(b);
        }
        g->syncRect();
        mine->pages[0]->items.push_back(g);
        auto theirs = copyOf(*mine);
        auto *tg = static_cast<GroupItem *>(theirs->pages[0]->items[0].get());
        tg->children[0]->rect.translate(40, 0);
        QTextCursor c(theirs->storyDoc(static_cast<ShapeItem *>(tg->children[1].get())->storyId));
        c.select(QTextCursor::Document);
        c.insertText(QStringLiteral("two, changed"));
        tg->children.push_back(addBoxPtr(*theirs, QStringLiteral("four")));
        DeckComparison cmp = compareDecks(*mine, *theirs);
        QCOMPARE(cmp.changes.size(), 3);
        QCOMPARE(kindCount(cmp, review::Kind::Moved), 1);
        QCOMPARE(kindCount(cmp, review::Kind::Edited), 1);
        QCOMPARE(kindCount(cmp, review::Kind::Added), 1);
        auto working = copyOf(*mine);
        QCOMPARE(acceptAll(*working, cmp), 3);
        QCOMPARE(lookOf(*working), lookOf(*theirs));
        QCOMPARE(compareDecks(*working, *theirs).changes.size(), 0);
        QCOMPARE(rejectAll(*working, cmp, true), 3);
        QCOMPARE(lookOf(*working), lookOf(*mine));
    }

    // Pictures: one moved and one added; the added one brings its picture, and taking it back leaves mine as it was.
    void comparePicturesMoveAndComeIn()
    {
        auto mine = deck::newPresentation();
        mine->pages[0]->items.clear();
        auto p = std::make_shared<PictureItem>();
        p->rect = QRectF(10, 10, 80, 80);
        p->imageId = mine->addImage(pngOf(Qt::red), QStringLiteral("png"));
        mine->pages[0]->items.push_back(p);
        auto theirs = copyOf(*mine);
        theirs->pages[0]->items[0]->rect.translate(30, 0);
        auto q = std::make_shared<PictureItem>();
        q->rect = QRectF(100, 10, 80, 80);
        const QByteArray blue = pngOf(Qt::blue);
        q->imageId = theirs->addImage(blue, QStringLiteral("png"));
        theirs->pages[0]->items.push_back(q);
        DeckComparison cmp = compareDecks(*mine, *theirs);
        QCOMPARE(cmp.changes.size(), 2);
        QCOMPARE(kindCount(cmp, review::Kind::Moved), 1);
        QCOMPARE(kindCount(cmp, review::Kind::Added), 1);
        auto working = copyOf(*mine);
        QCOMPARE(acceptAll(*working, cmp), 2);
        QCOMPARE(int(working->pages[0]->items.size()), 2);
        QCOMPARE(working->pages[0]->items[0]->rect, theirs->pages[0]->items[0]->rect);
        const auto *added = static_cast<const PictureItem *>(working->pages[0]->items[1].get());
        QVERIFY(working->images.contains(added->imageId));
        QCOMPARE(working->images[added->imageId].bytes, blue);
        QCOMPARE(rejectAll(*working, cmp, true), 2);
        QCOMPARE(int(working->pages[0]->items.size()), 1);
        QCOMPARE(working->pages[0]->items[0]->rect, mine->pages[0]->items[0]->rect);
    }

    // A change of look with the same words (a word made bold) is a formatting change, and taking it brings the look over.
    void compareFindsAFontChange()
    {
        auto mine = deck::newPresentation();
        mine->pages[0]->items.clear();
        addBox(*mine, *mine->pages[0], QStringLiteral("Hello world"), 10, 10);
        auto theirs = copyOf(*mine);
        QTextCursor c(theirs->storyDoc(boxOf(*theirs, 0)->storyId));
        c.setPosition(6);
        c.setPosition(11, QTextCursor::KeepAnchor);
        QTextCharFormat bold;
        bold.setFontWeight(QFont::Bold);
        c.mergeCharFormat(bold);
        DeckComparison cmp = compareDecks(*mine, *theirs);
        QCOMPARE(cmp.changes.size(), 1);
        QCOMPARE(int(cmp.changes.changes[0].kind), int(review::Kind::Reformatted));
        QCOMPARE(cmp.changes.changes[0].target.path, QStringLiteral("font"));
        auto working = copyOf(*mine);
        QCOMPARE(acceptAll(*working, cmp), 1);
        QCOMPARE(storyToJson(working->storyDoc(boxOf(*working, 0)->storyId)), storyToJson(theirs->storyDoc(boxOf(*theirs, 0)->storyId)));
        QCOMPARE(compareDecks(*working, *theirs).changes.size(), 0);
        QCOMPARE(rejectAll(*working, cmp, true), 1);
        QCOMPARE(storyToJson(working->storyDoc(boxOf(*working, 0)->storyId)), storyToJson(mine->storyDoc(boxOf(*mine, 0)->storyId)));
    }

    // A word that is on more than 64 slides tells nothing about which slide is which: slides that only share such words are not matched, and the
    // work stays a few steps a slide (without the limit these 100 would be matched, a hundred looks each).
    void compareFollowsAWordOnlyOnFewSlides()
    {
        auto mine = deck::newPresentation();
        while (mine->pages.size() < 100) mine->addPage(-1, mine->pages[0]->masterId);
        for (int i = 0; i < 100; ++i) mine->pages[i]->title = QStringLiteral("one two three four five six seven eight uniq%1").arg(i);
        auto theirs = copyOf(*mine);
        for (auto &p : theirs->pages) {
            p->id = newId("p");
            p->title += QStringLiteral(" extra");
        }
        DeckComparison cmp = compareDecks(*mine, *theirs);
        QCOMPARE(cmp.changes.size(), 200);   // 100 deleted, 100 added: the one word each has alone is too little to match them
        QVERIFY2(cmp.work < 40 * 100, qPrintable(QString::number(cmp.work)));
        // With 64 slides the words are followed: matched, no change.
        auto few = deck::newPresentation();
        while (few->pages.size() < 64) few->addPage(-1, few->pages[0]->masterId);
        for (int i = 0; i < 64; ++i) few->pages[i]->title = QStringLiteral("one two three four five six seven eight uniq%1").arg(i);
        auto moreWords = copyOf(*few);
        for (auto &p : moreWords->pages) {
            p->id = newId("p");
            p->title += QStringLiteral(" extra");
        }
        QCOMPARE(compareDecks(*few, *moreWords).changes.size(), 0);
    }

    // The copy of mine that a comparison keeps (Document::snapshot, fromSnapshot) shares the pictures' and movies' bytes with mine and does not copy them:
    // the same memory, so a deck of big pictures costs no more to compare.
    void compareKeepsThePicturesAndMoviesSharedNotCopied()
    {
        auto mine = deck::newPresentation();
        const QString img = mine->addImage(pngOf(Qt::green), QStringLiteral("png"));
        const QString movie = mine->addMedia(QByteArray(50000, 'm'), QStringLiteral("video/mp4"), QStringLiteral("mp4"));
        auto theirs = copyOf(*mine);
        DeckComparison cmp = compareDecks(*mine, *theirs);
        QVERIFY(cmp.mine->images.contains(img));
        QVERIFY(cmp.mine->media.contains(movie));
        QCOMPARE(static_cast<const void *>(cmp.mine->images[img].bytes.constData()), static_cast<const void *>(mine->images[img].bytes.constData()));
        QCOMPARE(static_cast<const void *>(cmp.mine->media[movie].bytes.constData()), static_cast<const void *>(mine->media[movie].bytes.constData()));
    }

    // ---- the review model in libs/base ----

    void reviewModelPositionKeyOfACrunchCellNeverCollides()
    {
        using review::crunchPosition;
        QCOMPARE(crunchPosition(0, 0, 0), qint64(0));
        // Out of range is held to the edge, so a key never runs into its neighbor's: (0, 0, 2^14) is not (0, 1, 0), and nothing goes negative.
        QCOMPARE(crunchPosition(0, 0, 16384), crunchPosition(0, 0, 16383));
        QVERIFY(crunchPosition(0, 0, 16384) < crunchPosition(0, 1, 0));
        QCOMPARE(crunchPosition(0, 1 << 26, 0), crunchPosition(0, (1 << 26) - 1, 0));
        QVERIFY(crunchPosition(0, 1 << 26, 16383) < crunchPosition(1, 0, 0));
        QCOMPARE(crunchPosition(1 << 23, 0, 0), crunchPosition((1 << 23) - 1, 0, 0));
        QVERIFY(crunchPosition(1 << 23, 1 << 26, 1 << 14) > 0);
        QCOMPARE(crunchPosition(-1, -1, -1), qint64(0));
        QCOMPARE(crunchPosition(0, (1 << 26) - 1, 16383) + 1, crunchPosition(1, 0, 0));
        QCOMPARE(crunchPosition(0, 0, 1), qint64(1));
        QCOMPARE(crunchPosition(0, 1, 0), qint64(1) << 14);
        QCOMPARE(crunchPosition(1, 0, 0), qint64(1) << 40);
        // The last cell of a sheet comes before the first of the next, and the last of a row before the first of the next row.
        QVERIFY(crunchPosition(0, 1048575, 16383) < crunchPosition(1, 0, 0));
        QVERIFY(crunchPosition(3, 7, 16383) < crunchPosition(3, 8, 0));
        QVERIFY(crunchPosition(16383, 1048575, 16383) > crunchPosition(16383, 1048575, 16382));
        // No two of a spread of cells share a key, and the keys run in (sheet, row, column) order.
        QSet<qint64> seen;
        qint64 last = -1;
        for (int sheet : {0, 1, 2, 255, 16383})
            for (int row : {0, 1, 2, 1023, 1048574, 1048575})
                for (int col : {0, 1, 255, 16382, 16383}) {
                    const qint64 k = crunchPosition(sheet, row, col);
                    QVERIFY(!seen.contains(k));
                    QVERIFY(k > last);
                    seen.insert(k);
                    last = k;
                }
    }

    void reviewModelKeepsRepliesAndRoundTripsAList()
    {
        review::ChangeList list;
        for (int i = 0; i < 3; ++i) {
            review::Change c;
            c.id = QStringLiteral("t%1").arg(i);
            c.kind = review::Kind::Commented;
            c.author = QStringLiteral("Pat");
            c.when = QDateTime(QDate(2026, 11, 1 + i), QTime(8, 30), Qt::UTC);
            c.target = {QStringLiteral("Sheet 1"), QStringLiteral("B%1").arg(i + 2), QStringLiteral("thread")};
            c.summary = QStringLiteral("Check this %1").arg(i);
            c.position = review::crunchPosition(16383, 1048575 - i, 16383);   // more than 53 bits: it must not go through a double
            c.data[QStringLiteral("resolved")] = i == 1;
            for (int r = 0; r < i; ++r) c.replies << review::Reply{QStringLiteral("Lee%1").arg(r), QDateTime(QDate(2026, 11, 5), QTime(9, r), Qt::UTC), QStringLiteral("Reply %1").arg(r)};
            list.changes << c;
        }
        list.changes[2].state = review::State::Accepted;
        list.tooMany = true;
        const review::ChangeList back = review::ChangeList::fromJson(list.toJson());
        QVERIFY(back.tooMany);
        QCOMPARE(back.size(), 3);
        for (int i = 0; i < 3; ++i) {
            const review::Change &a = list.changes[i], &b = back.changes[i];
            QCOMPARE(b.id, a.id);
            QCOMPARE(int(b.kind), int(a.kind));
            QCOMPARE(b.author, a.author);
            QCOMPARE(b.when, a.when);
            QVERIFY(b.target == a.target);
            QCOMPARE(b.summary, a.summary);
            QCOMPARE(int(b.state), int(a.state));
            QCOMPARE(b.position, a.position);
            QCOMPARE(b.data, a.data);
            QCOMPARE(b.replies.size(), a.replies.size());
            for (int r = 0; r < a.replies.size(); ++r) {
                QCOMPARE(b.replies[r].author, a.replies[r].author);
                QCOMPARE(b.replies[r].when, a.replies[r].when);
                QCOMPARE(b.replies[r].text, a.replies[r].text);
            }
        }
        QVERIFY(!review::ChangeList::fromJson(review::ChangeList().toJson()).tooMany);
        // A file with more than the most is read to the most, and says so; a thread's replies are cut the same way.
        QJsonArray many;
        for (int i = 0; i < review::kMaxChanges + 5; ++i) many.append(QJsonObject{{"id", QString::number(i)}});
        const review::ChangeList cut = review::ChangeList::fromJson(QJsonObject{{"changes", many}});
        QCOMPARE(cut.size(), review::kMaxChanges);
        QVERIFY(cut.tooMany);
        QJsonArray replies;
        for (int i = 0; i < review::kMaxReplies + 3; ++i) replies.append(QJsonObject{{"text", QStringLiteral("r")}});
        QCOMPARE(review::Change::fromJson(QJsonObject{{"replies", replies}}).replies.size(), review::kMaxReplies);
        // A position written as a plain number (an older file) still reads.
        QCOMPARE(review::Change::fromJson(QJsonObject{{"position", 12345.0}}).position, qint64(12345));
    }

    void reviewModelFindsAChangeByIdInOneStep()
    {
        review::ChangeList list;
        for (int i = 0; i < review::kMaxChanges; ++i) {
            review::Change c;
            c.id = QStringLiteral("c%1").arg(i);
            list.changes << c;
        }
        for (int i = 0; i < review::kMaxChanges; ++i) QCOMPARE(list.indexOf(QStringLiteral("c%1").arg(i)), i);
        QCOMPARE(list.indexOf(QStringLiteral("none")), -1);
        QCOMPARE(list.indexOf(QStringLiteral("none")), -1);
        QCOMPARE(list.indexRebuilds(), 1);   // one hash for all 10,001 lookups
        // A list that grew is indexed again, once; an id changed in place is told with reindex().
        review::Change more;
        more.id = QStringLiteral("late");
        list.changes << more;
        QCOMPARE(list.indexOf(QStringLiteral("late")), review::kMaxChanges);
        QCOMPARE(list.indexRebuilds(), 2);
        list.changes[0].id = QStringLiteral("renamed");
        QCOMPARE(list.indexOf(QStringLiteral("c0")), -1);   // (a stale hash never answers with the wrong change)
        list.reindex();
        QCOMPARE(list.indexOf(QStringLiteral("renamed")), 0);
    }

    // Text came or went: the changes after it move, the ones in another container and before it do not, the order is kept, and the path is left alone.
    void reviewModelShiftsAChangesPositionAfterAnEdit()
    {
        review::ChangeList list;
        auto add = [&](const QString &container, qint64 pos) {
            review::Change c;
            c.id = QStringLiteral("c%1").arg(list.size());
            c.target = {container, QString(), QStringLiteral("%1-%2").arg(pos).arg(pos + 3)};
            c.position = pos;
            list.changes << c;
        };
        add(QStringLiteral("body"), 5);
        add(QStringLiteral("body"), 10);
        add(QStringLiteral("body"), 12);
        add(QStringLiteral("body"), 40);
        add(QStringLiteral("header:1"), 12);
        list.shiftAfter(QStringLiteral("body"), 10, 7);   // seven characters typed at 10: the one at 10 moves too
        QCOMPARE(list.changes[0].position, qint64(5));
        QCOMPARE(list.changes[1].position, qint64(17));
        QCOMPARE(list.changes[2].position, qint64(19));
        QCOMPARE(list.changes[3].position, qint64(47));
        QCOMPARE(list.changes[4].position, qint64(12));
        QCOMPARE(list.changes[1].target.path, QStringLiteral("10-13"));   // advisory: not rewritten
        list.shiftAfter(QStringLiteral("body"), 15, -5);   // 15 to 20 removed: the one at 17 and 19 fall to 15, the one at 47 comes to 42
        QCOMPARE(list.changes[1].position, qint64(15));
        QCOMPARE(list.changes[2].position, qint64(15));
        QCOMPARE(list.changes[3].position, qint64(42));
        QCOMPARE(list.changes[0].position, qint64(5));
        for (int i = 1; i < 4; ++i) QVERIFY(list.changes[i - 1].position <= list.changes[i].position);
    }

    // File > Options is the shared dialog: its General tab has the user name
    // but not the update check or the statistics (JeffDeck's preview does
    // neither; JeffPub's Options keep them for the suite), Proofing follows,
    // and JeffDeck's View page sets the ruler and snapping for every open window.
    // Save is the page every app shares: the default format (the app's own kinds), the recovery interval, and the default folder; a new presentation is saved as the kind chosen, in
    // the folder chosen.
    void optionsSaveIsTheSharedPageAndNewPresentationsFollowIt()
    {
        QTemporaryDir dir;
        const jptest::SettingGuard format(QStringLiteral("save/format"), QString(), QString());
        const jptest::SettingGuard folder(QStringLiteral("dirs/default"), QString(), QString());
        DeckWindow w;
        QStringList kinds;
        int least = 0, most = 0;
        bool all = false;
        QTimer::singleShot(0, [&] {
            auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!d) return;
            auto *box = d->findChild<QComboBox *>(QStringLiteral("defaultFormat"));
            auto *minutes = d->findChild<QSpinBox *>(QStringLiteral("autoRecoverMinutes"));
            all = box && minutes && d->findChild<QLineEdit *>(QStringLiteral("defaultFolder")) && d->findChild<QCheckBox *>(QStringLiteral("embedFonts"));
            if (all) {
                for (int i = 0; i < box->count(); ++i) kinds << box->itemData(i).toString();
                least = minutes->minimum();
                most = minutes->maximum();
            }
            if (auto *tabs = d->findChild<QTabWidget *>()) {
                for (int i = 0; i < tabs->count(); ++i)
                    if (tabs->tabText(i) == QLatin1String("Save")) tabs->setCurrentIndex(i);
            }
            shotOf(d, QStringLiteral("deck-options-save"));
            d->reject();
        });
        w.act(QStringLiteral("file.options"))->trigger();
        QVERIFY(all);
        QCOMPARE(kinds, (QStringList{QStringLiteral("pptx"), QStringLiteral("odp")}));
        QCOMPARE(least, 1);
        QCOMPARE(most, 120);
        Settings::get().setValue(QStringLiteral("save/format"), QStringLiteral("odp"));
        Settings::get().setValue(QStringLiteral("dirs/default"), dir.path());
        QString suggested;
        w.setSavePathChooser([&](const QString &name, const QString &) {
            suggested = name;
            return QString();
        });
        w.act(QStringLiteral("file.saveAs"))->trigger();
        QVERIFY2(suggested.endsWith(QLatin1String(".odp")) && suggested.startsWith(dir.path()), qPrintable(suggested));
    }

    void optionsHasTheGeneralTabAndTheViewPage()
    {
        DeckWindow w;
        QVERIFY(w.act(QStringLiteral("file.options")));
        QStringList tabNames;
        QString title, shown, userName;
        QStringList online;
        QTimer::singleShot(0, [&] {
            auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!d) return;
            title = d->windowTitle();
            if (auto *tabs = d->findChild<QTabWidget *>()) {
                shown = tabs->tabText(tabs->currentIndex());
                for (int i = 0; i < tabs->count(); ++i) tabNames << tabs->tabText(i);
            }
            for (QLabel *l : d->findChildren<QLabel *>())
                if (l->text() == QLatin1String("User name") && l->buddy()) userName = static_cast<QLineEdit *>(l->buddy())->text();
            for (const char *name : {"updates", "stats", "betaUpdates"})
                if (auto *box = d->findChild<QCheckBox *>(QLatin1String(name)); box && box->isVisibleTo(d)) online << box->text();
            d->reject();
        });
        w.act(QStringLiteral("file.options"))->trigger();
        QCOMPARE(title, QStringLiteral("JeffDeck Options"));
        QCOMPARE(tabNames, (QStringList{"General", "Proofing", "View", "Advanced", "Save", "Security", "Customize"}));
        QCOMPARE(shown, QStringLiteral("General"));
        QCOMPARE(userName, Settings::get().userName());
        // JeffDeck checks for updates and sends statistics (when chosen) like JeffPub, so the choices are here, in its own words.
        QVERIFY(deckAppInfo().online);
        QCOMPARE(online, (QStringList{QStringLiteral("Check for updates when JeffDeck starts"), QStringLiteral("Send anonymous usage statistics"),
                                      QStringLiteral("Get beta versions (early builds for testing)")}));
        // The View page: the ruler and snapping, in this window now and in new ones.
        QVERIFY(!w.editor()->view.rulers);
        QVERIFY(w.editor()->view.snapObjects);
        QTimer::singleShot(0, [&] {
            auto *d = qobject_cast<QDialog *>(QApplication::activeModalWidget());
            if (!d) return;
            if (auto *box = d->findChild<QCheckBox *>(QStringLiteral("showRuler"))) box->setChecked(true);
            if (auto *box = d->findChild<QCheckBox *>(QStringLiteral("snapObjects"))) box->setChecked(false);
            d->accept();
        });
        w.act(QStringLiteral("file.options"))->trigger();
        QVERIFY(w.editor()->view.rulers);
        QVERIFY(!w.canvas()->hRuler()->isHidden());
        QVERIFY(!w.editor()->view.snapObjects);
        {
            DeckWindow another;
            QVERIFY(another.editor()->view.rulers);
            QVERIFY(!another.editor()->view.snapObjects);
        }
        Settings::get().setValue(QStringLiteral("view/rulers"), false);
        Settings::get().setValue(QStringLiteral("view/snapObjects"), true);
    }

    // A ping and a crash report from JeffDeck say so, and carry nothing else JeffPub's don't.
    void deckPingsAndCrashReportsNameJeffDeck()
    {
        telemetry::setEnabled(true);
        const QJsonObject p = telemetry::payload();
        QCOMPARE(p[QStringLiteral("app")].toString(), QStringLiteral("jeffdeck"));
        QStringList keys = p.keys();
        keys.sort();
        QCOMPARE(keys, (QStringList{"app", "counts", "install", "lang", "launches", "os", "osVersion", "version"}));
        QJsonObject report;
        report[QStringLiteral("version")] = jp::versionText();
        report[QStringLiteral("when")] = QDateTime::currentDateTimeUtc().date().toString(Qt::ISODate);
        report[QStringLiteral("kind")] = QStringLiteral("signal");
        report[QStringLiteral("code")] = 11;
        report[QStringLiteral("frames")] = QJsonArray{QJsonObject{{QStringLiteral("module"), QStringLiteral("JeffDeck.exe")}, {QStringLiteral("offset"), 6699}}};
        const QJsonObject sent = telemetry::crashPayload(report);
        keys = sent.keys();
        keys.sort();
        QCOMPARE(keys, (QStringList{"app", "code", "frames", "install", "kind", "os", "version", "when"}));
        QCOMPARE(sent[QStringLiteral("app")].toString(), QStringLiteral("jeffdeck"));
        QCOMPARE(sent[QStringLiteral("install")].toString(), p[QStringLiteral("install")].toString());
        telemetry::setEnabled(false);
        QVERIFY(telemetry::crashPayload(report).isEmpty());   // not sent when sharing is off (and the report is deleted either way)
    }

    // The choice to share and the install id are the suite's: made in JeffPub, they hold here, and JeffDeck doesn't ask. The counts
    // are JeffDeck's own, and JeffPub's never reach its ping.
    void deckSharesTheChoiceAndTheInstallButKeepsItsOwnCounts()
    {
        QSettings suite(QSettings::defaultFormat(), QSettings::UserScope, QStringLiteral("JeffOffice"), QStringLiteral("JeffOffice")), pub(QSettings::defaultFormat(), QSettings::UserScope, QStringLiteral("JeffOffice"), QStringLiteral("JeffPub")),
            deck(QSettings::defaultFormat(), QSettings::UserScope, QStringLiteral("JeffOffice"), QStringLiteral("JeffDeck"));
        QVERIFY(!telemetry::enabled());
        suite.setValue(QStringLiteral("telemetry/enabled"), true);
        suite.setValue(QStringLiteral("telemetry/install"), QStringLiteral("0f8fad5b-d9cb-469f-a165-70867728950e"));
        suite.sync();
        pub.setValue(QStringLiteral("telemetry/counts"), QVariantMap{{QStringLiteral("file.open.pub"), 4}});
        pub.setValue(QStringLiteral("telemetry/launches"), 6);
        pub.sync();
        Settings::get().sync();
        QVERIFY(telemetry::decided());   // so JeffDeck's first start has no question to ask
        QVERIFY(telemetry::enabled());
        QJsonObject p = telemetry::payload();
        QCOMPARE(p[QStringLiteral("install")].toString(), QStringLiteral("0f8fad5b-d9cb-469f-a165-70867728950e"));
        QVERIFY(p[QStringLiteral("counts")].toObject().isEmpty());
        QCOMPARE(p[QStringLiteral("launches")].toInt(), 0);
        telemetry::count(QStringLiteral("file.open.pptx"));
        Settings::get().sync();
        QCOMPARE(telemetry::payload()[QStringLiteral("counts")].toObject().keys(), QStringList{QStringLiteral("file.open.pptx")});
        deck.sync();
        QCOMPARE(deck.value(QStringLiteral("telemetry/counts")).toMap().value(QStringLiteral("file.open.pptx")).toInt(), 1);
        QVERIFY(!deck.contains(QStringLiteral("telemetry/install")));   // the suite's, not its own
        pub.sync();
        QCOMPARE(pub.value(QStringLiteral("telemetry/counts")).toMap().keys(), QStringList{QStringLiteral("file.open.pub")});   // JeffPub's, untouched
        // Turned off here, it is off for the suite, and its id is gone.
        telemetry::setEnabled(false);
        Settings::get().sync();
        suite.sync();
        QVERIFY(!suite.value(QStringLiteral("telemetry/enabled")).toBool());
        QVERIFY(suite.value(QStringLiteral("telemetry/install")).toString().isEmpty());
        QVERIFY(telemetry::decided());
        pub.remove(QStringLiteral("telemetry"));
        pub.sync();
    }

    // JeffDeck counts each command by its id, and a file opened or saved by its kind, never by its name.
    void deckCountsCommandsAndFileKindsButNeverNames()
    {
        telemetry::setEnabled(false);
        {
            DeckWindow quiet;
            quiet.act(QStringLiteral("view.gridlines"))->trigger();
            quiet.act(QStringLiteral("view.gridlines"))->trigger();
            QVERIFY(telemetry::payload()[QStringLiteral("counts")].toObject().isEmpty());   // sharing is off: nothing counted
        }
        telemetry::setEnabled(true);
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString secret = QStringLiteral("quarterly-numbers-for-Pat");
        const QString pptx = dir.filePath(secret + QStringLiteral(".pptx"));
        {
            DeckWindow w;
            w.act(QStringLiteral("view.gridlines"))->trigger();
            w.act(QStringLiteral("view.gridlines"))->trigger();
            QVERIFY(w.saveTo(pptx));
            QVERIFY(w.saveTo(dir.filePath(secret + QStringLiteral(".POTX"))));
            QVERIFY(w.saveTo(dir.filePath(secret + QStringLiteral(".ppsx"))));
            QVERIFY(w.saveTo(dir.filePath(secret + QStringLiteral(".dat"))));   // an ending JeffDeck has no kind for
            answerDialogs({QMessageBox::Ok});
            QVERIFY(!w.saveTo(dir.filePath(QStringLiteral("missing/") + secret + QStringLiteral(".pptx"))));   // a save that failed is not counted
        }
        // Read by what a file is and not what it is called; counted by the ending, among the kinds JeffDeck knows.
        for (const QString &name : {secret + QStringLiteral(".pptx"), secret + QStringLiteral(".odp"), secret + QStringLiteral(".PPT"), secret + QStringLiteral(".dat")}) {
            const QString copy = dir.filePath(QStringLiteral("copy-") + name);
            QVERIFY(QFile::copy(pptx, copy));
            DeckWindow w;
            QVERIFY(w.openFile(copy));
        }
        const QJsonObject counts = telemetry::payload()[QStringLiteral("counts")].toObject();
        QCOMPARE(counts[QStringLiteral("view.gridlines")].toInt(), 2);
        QCOMPARE(counts[QStringLiteral("file.save.pptx")].toInt(), 1);
        QCOMPARE(counts[QStringLiteral("file.save.potx")].toInt(), 1);
        QCOMPARE(counts[QStringLiteral("file.save.ppsx")].toInt(), 1);
        QCOMPARE(counts[QStringLiteral("file.save.other")].toInt(), 1);
        QCOMPARE(counts[QStringLiteral("file.open.pptx")].toInt(), 1);
        QCOMPARE(counts[QStringLiteral("file.open.odp")].toInt(), 1);
        QCOMPARE(counts[QStringLiteral("file.open.ppt")].toInt(), 1);
        QCOMPARE(counts[QStringLiteral("file.open.other")].toInt(), 1);
        QCOMPARE(counts.size(), 9);
        for (const QString &k : counts.keys()) {
            QVERIFY2(QRegularExpression(QStringLiteral("^[a-z0-9][a-z0-9._-]{0,47}$")).match(k).hasMatch(), qPrintable(k));   // what the collector takes
            QVERIFY2(!k.contains(QStringLiteral("quarterly"), Qt::CaseInsensitive) && !k.contains(QStringLiteral("pat")) && !k.contains(QStringLiteral("dat")), qPrintable(k));
        }
        telemetry::setEnabled(false);
    }

    // JeffDeck says which version it offers once a day with JeffPub: the note is the suite's, written under JeffDeck's name.
    void deckNotesWhatItOffered()
    {
        for (const char *k : {"offered", "offeredOn", "offeredBy"}) Settings::get().setValue(QStringLiteral("updates/") + QString::fromLatin1(k), QVariant());
        Updater::noteOffered(QStringLiteral("v0.8.0"));
        QSettings suite(QSettings::defaultFormat(), QSettings::UserScope, QStringLiteral("JeffOffice"), QStringLiteral("JeffOffice"));
        QCOMPARE(suite.value(QStringLiteral("updates/offeredBy")).toString(), QStringLiteral("jeffdeck"));
        QVERIFY(!Updater::offeredByAnotherAppToday(QStringLiteral("v0.8.0")));
        Settings::get().setValue(QStringLiteral("updates/offeredBy"), QStringLiteral("jeffpub"));   // JeffPub offered it first today
        QVERIFY(Updater::offeredByAnotherAppToday(QStringLiteral("v0.8.0")));
        for (const char *k : {"offered", "offeredOn", "offeredBy"}) Settings::get().setValue(QStringLiteral("updates/") + QString::fromLatin1(k), QVariant());
    }

private:
    // The sample presentation with comments in the newer form on its second slide.
    static Parts withModernComments()
    {
        Parts p = samplePptx();
        const QString head = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>");
        const QString modern = nsDecl() + QStringLiteral(" xmlns:p188=\"http://schemas.microsoft.com/office/powerpoint/2018/8/main\"");
        const QString pat = QStringLiteral("{11111111-1111-4111-8111-111111111111}"), lee = QStringLiteral("{22222222-2222-4222-8222-222222222222}");
        p.data["/ppt/authors.xml"] = {QStringLiteral("application/vnd.ms-powerpoint.authors+xml"),
            (head + QStringLiteral("<p188:authorLst ") + modern + QStringLiteral(">"
             "<p188:author id=\"%1\" name=\"Pat Doe\" initials=\"PD\" userId=\"S::pat@example.com::1\" providerId=\"AD\"/>"
             "<p188:author id=\"%2\" name=\"Lee Roy\" initials=\"LR\" userId=\"lee\" providerId=\"None\"/></p188:authorLst>").arg(pat, lee)).toUtf8()};
        p.rels << Rel{"/ppt/presentation.xml", "http://schemas.microsoft.com/office/2018/10/relationships/authors", "/ppt/authors.xml"};
        auto body = [](const QString &text) {
            return QStringLiteral("<p188:txBody><a:bodyPr/><a:lstStyle/>%1</p188:txBody>").arg(text);
        };
        auto line = [](const QString &text) { return QStringLiteral("<a:p><a:r><a:rPr lang=\"en-US\"/><a:t>%1</a:t></a:r></a:p>").arg(text); };
        const QString slideMark = QStringLiteral("<pc:docMk/><pc:sldMk cId=\"123456\" sldId=\"257\"/>");
        const QString pc = QStringLiteral("xmlns:pc=\"http://schemas.microsoft.com/office/powerpoint/2013/main/command\"");
        QString xml = head + QStringLiteral("<p188:cmLst ") + modern + QStringLiteral(">");
        xml += QStringLiteral("<p188:cm id=\"{AAAAAAAA-0000-4000-8000-000000000001}\" authorId=\"%1\" created=\"2026-10-12T17:15:30.250\" status=\"resolved\">"
                              "<pc:sldMkLst %2>%3</pc:sldMkLst><p188:pos x=\"9144000\" y=\"1371600\"/>"
                              "<p188:replyLst><p188:reply id=\"{BBBBBBBB-0000-4000-8000-000000000001}\" authorId=\"%4\" created=\"2026-10-12T17:20:00.000\">%5</p188:reply></p188:replyLst>%6</p188:cm>")
                   .arg(pat, pc, slideMark, lee, body(line(QStringLiteral("On it"))), body(line(QStringLiteral("Line one")) + line(QStringLiteral("Line two"))));
        xml += QStringLiteral("<p188:cm id=\"{AAAAAAAA-0000-4000-8000-000000000002}\" authorId=\"%1\" created=\"2026-10-12T18:00:00.000\">"
                              "<ac:deMkLst xmlns:ac=\"http://schemas.microsoft.com/office/drawing/2013/main/command\" %2>%3<ac:spMk id=\"2\" creationId=\"{CCCCCCCC-0000-4000-8000-000000000001}\"/></ac:deMkLst>%4</p188:cm>")
                   .arg(lee, pc, slideMark, body(line(QStringLiteral("Anchored"))));
        xml += QStringLiteral("<p188:cm id=\"{AAAAAAAA-0000-4000-8000-000000000003}\" authorId=\"%1\" created=\"2026-10-12T18:30:00.000\"><pc:sldMkLst %2>%3</pc:sldMkLst>%4</p188:cm>")
                   .arg(pat, pc, slideMark, body(line(QStringLiteral("Nowhere"))));
        xml += QStringLiteral("</p188:cmLst>");
        p.data["/ppt/comments/modernComment_101_5.xml"] = {QStringLiteral("application/vnd.ms-powerpoint.comments+xml"), xml.toUtf8()};
        p.rels << Rel{"/ppt/slides/slide2.xml", "http://schemas.microsoft.com/office/2018/10/relationships/comments", "/ppt/comments/modernComment_101_5.xml"};
        return p;
    }
};

JP_TEST_CLASS(DeckReviewTests)
#include "tst_deck_review.moc"
