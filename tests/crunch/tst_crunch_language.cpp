// Tests of the Review tab's Language group in JeffCrunch, which JeffWrite has too: Translate (the text of the selected cells, or of the entry being typed, opened in the translation service
// only after OK), Set Proofing Language and Do Not Check Spelling for the selected cells (the spelling walk and the cell editor follow them), and what a workbook keeps of them when it is saved
// (an OpenDocument workbook keeps them in the cell's style; the XML workbook has no place for them). Every workbook is built here.
#include "../sheet/odshelpers.h"
#include "../settingguard.h"
#include "../withdialog.h"
#include "app/crunchwindow.h"
#include "app/settings.h"
#include "app/crunchinfo.h"
#include "core/appinfo.h"
#include "core/fonts.h"
#include "core/spellwalk.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "grid/gridview.h"
#include "io/zip.h"
#include "sheet/styles.h"

#include <QAction>
#include <QComboBox>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
#include <QListWidget>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QUrlQuery>

#include "../testrunner.h"

using namespace jp;
using namespace crunchtest;

namespace {

QJsonObject groupNamed(const QString &tab, const QString &group)
{
    QFile json(QStringLiteral(":/crunch/ribbon.json"));
    if (!json.open(QIODevice::ReadOnly)) return {};
    for (const QJsonValue &t : QJsonDocument::fromJson(json.readAll()).object().value(QStringLiteral("tabs")).toArray())
        if (t.toObject().value(QStringLiteral("name")).toString() == tab)
            for (const QJsonValue &g : t.toObject().value(QStringLiteral("groups")).toArray())
                if (g.toObject().value(QStringLiteral("name")).toString() == group) return g.toObject();
    return {};
}

// An .ods with A1 "bonjour monde" (spelled right in French only), A2 "mispelled" (wrong in every language), and A3 "plain".
QByteArray odsWords()
{
    return buildOds(contentXml(table(QStringLiteral("S"), columns(2) + row(stringCell(QStringLiteral("bonjour monde"))) + row(stringCell(QStringLiteral("mispelled"))) + row(stringCell(QStringLiteral("plain"))))));
}

// The language and the mark of a cell as the workbook window holds them.
struct Proofing {
    QString language;
    bool noProof = false;
};
Proofing proofingOf(EditWindow &e, quint32 row, quint32 col)
{
    Proofing p;
    const auto c = e.win.w.session()->calculation()->content(0, row, col);
    p.language = e.win.w.session()->proofingLanguageOfStyle(c.style, &p.noProof);
    return p;
}

// The same as the file holds it.
Proofing proofingOfFile(const QString &path, quint32 row, quint32 col)
{
    Proofing p;
    jp::office::sheet::Workbook wb;
    jp::office::sheet::Sheet s;
    QString why;
    if (!wb.openFile(path, &why) || !wb.readSheet(0, &s)) return p;
    if (const auto *cell = s.cells.find(row, col)) {
        const auto &f = wb.styles().format(cell->style);
        p.language = f.language;
        p.noProof = f.noProof;
    }
    return p;
}

void chooseLanguage(EditWindow &e, const QString &tag)
{
    withDialog([&] { e.win.w.act(QStringLiteral("review.language"))->trigger(); }, QStringLiteral("languageDialog"), [&](QDialog *d) {
        auto *list = d->findChild<QListWidget *>(QStringLiteral("languages"));
        for (int i = 0; i < list->count(); ++i)
            if (list->item(i)->data(Qt::UserRole).toString() == tag) list->setCurrentRow(i);
        QCOMPARE(list->currentItem()->data(Qt::UserRole).toString(), tag);
    });
}

bool savedCopy(EditWindow &e, const QString &path)
{
    QSignalSpy copied(&e.win.w, &CrunchWindow::copyExported);
    if (!e.win.w.saveAs(path, true)) return false;
    return (copied.count() > 0 || copied.wait(20000)) && copied.first().at(1).toBool();
}

} // namespace

class CrunchLanguageTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        static QTemporaryDir settings;
        QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, settings.path());
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settings.path());
        Q_INIT_RESOURCE(resources);
        Q_INIT_RESOURCE(crunch);
        setAppInfo(crunchAppInfo());
        initCore();
    }
    void init() { jp::setSpellAsYouType(true); }

    // The three commands sit on the Review tab in a Language group as JeffWrite has them: the same words, a key tip each, Translate and Set Proofing Language large.
    void theReviewTabHasTheLanguageGroupAsJeffWriteHasIt()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        QVERIFY2(e.win.w.ribbonErrors().isEmpty(), qPrintable(e.win.w.ribbonErrors()));
        QAction *translate = e.win.w.act(QStringLiteral("review.translate")), *language = e.win.w.act(QStringLiteral("review.language")), *noProof = e.win.w.act(QStringLiteral("review.noProof"));
        QVERIFY(translate && language && noProof);
        QCOMPARE(translate->text(), QStringLiteral("Translate"));
        QCOMPARE(language->text(), QStringLiteral("Set Proofing Language…"));
        QCOMPARE(noProof->text(), QStringLiteral("Do Not Check Spelling"));
        QVERIFY(noProof->isCheckable());
        const QJsonObject group = groupNamed(QStringLiteral("Review"), QStringLiteral("Language"));
        QVERIFY(!group.isEmpty());
        const QJsonArray items = group.value(QStringLiteral("items")).toArray();
        QCOMPARE(items.size(), 3);
        QCOMPARE(items[0].toObject().value(QStringLiteral("large")).toString(), QStringLiteral("review.translate"));
        QCOMPARE(items[0].toObject().value(QStringLiteral("keytip")).toString(), QStringLiteral("TR"));
        QCOMPARE(items[1].toObject().value(QStringLiteral("large")).toString(), QStringLiteral("review.language"));
        QCOMPARE(items[1].toObject().value(QStringLiteral("keytip")).toString(), QStringLiteral("LA"));
        QCOMPARE(items[2].toObject().value(QStringLiteral("small")).toString(), QStringLiteral("review.noProof"));
        QCOMPARE(items[2].toObject().value(QStringLiteral("keytip")).toString(), QStringLiteral("NP"));
    }

    // The Help page of the Review tab names the commands.
    void helpNamesTheCommands()
    {
        QFile page(QStringLiteral(":/help/en/crunch/notes-links.md"));
        QVERIFY(page.open(QIODevice::ReadOnly));
        const QString text = QString::fromUtf8(page.readAll());
        for (const char *word : {"**Translate**", "**Set Proofing Language**", "**Do Not Check Spelling**", "**Language**"}) QVERIFY2(text.contains(QLatin1String(word)), word);
    }

    // Translate sends the text of the selected cells (a line for a row), only after OK; a Cancel opens nothing, and the box says where.
    void translateOpensTheTextOfTheSelectedCellsOnlyAfterOk()
    {
        const jptest::SettingGuard target(QStringLiteral("translate/targetCode"), QStringLiteral("es"), QStringLiteral("es"));
        const jptest::SettingGuard server(QStringLiteral("translate/server"), QStringLiteral("https://translate.example/"), QStringLiteral("https://libretranslate.com/"));
        EditWindow e;
        QVERIFY(e.open(editBook()));
        QList<QUrl> opened;
        QVERIFY(e.win.w.act(QStringLiteral("review.translate")));
        e.win.w.setUrlOpener([&](const QUrl &u) {
            opened << u;
            return true;
        });
        e.select("E5");
        e.enter("one");
        e.select("F5");
        e.enter("two");
        e.select("E6");
        e.enter("three");
        QVERIFY(e.settle());
        e.win.w.view()->selectRange(4, 4, 5, 5);   // E5:F6
        withDialog([&] { e.win.w.act(QStringLiteral("review.translate"))->trigger(); }, QStringLiteral("translateDialog"), [&](QDialog *d) {
            QVERIFY(opened.isEmpty());   // (nothing is sent while the box is up)
            QCOMPARE(d->findChild<QLineEdit *>(QStringLiteral("translateServer"))->text(), QStringLiteral("https://translate.example/"));
        }, false);
        QVERIFY(opened.isEmpty());
        withDialog([&] { e.win.w.act(QStringLiteral("review.translate"))->trigger(); }, QStringLiteral("translateDialog"), [&](QDialog *d) {
            auto *to = d->findChild<QComboBox *>(QStringLiteral("translateTarget"));
            to->setCurrentIndex(to->findData(QStringLiteral("fr")));
        });
        QCOMPARE(opened.size(), 1);
        QCOMPARE(opened[0].host(), QStringLiteral("translate.example"));
        const QUrlQuery q(opened[0]);
        QCOMPARE(q.queryItemValue(QStringLiteral("q"), QUrl::FullyDecoded), QStringLiteral("one two\nthree"));
        QCOMPARE(q.queryItemValue(QStringLiteral("target")), QStringLiteral("fr"));
        QCOMPARE(jp::Settings::get().value(QStringLiteral("translate/targetCode")).toString(), QStringLiteral("fr"));
    }

    // The words selected in the entry being typed are what is sent; with none selected, the whole entry; cells with no text say so and open no box.
    void translateTakesTheEntryBeingTypedOrTellsThereIsNoText()
    {
        const jptest::SettingGuard server(QStringLiteral("translate/server"), QStringLiteral("https://translate.example/"), QStringLiteral("https://libretranslate.com/"));
        EditWindow e;
        QVERIFY(e.open(editBook()));
        QList<QUrl> opened;
        QVERIFY(e.win.w.act(QStringLiteral("review.translate")));
        e.win.w.setUrlOpener([&](const QUrl &u) {
            opened << u;
            return true;
        });
        e.select("A1");   // (a number)
        e.win.w.act(QStringLiteral("review.translate"))->trigger();
        QVERIFY(opened.isEmpty());
        QVERIFY2(e.win.w.statusText().contains(QLatin1String("no text")), qPrintable(e.win.w.statusText()));
        e.select("E5");
        e.type("say this slowly");
        CellEditor *editor = e.win.w.cellEditor();
        editor->setSelection(4, 4);   // "this"
        withDialog([&] { e.win.w.act(QStringLiteral("review.translate"))->trigger(); }, QStringLiteral("translateDialog"), [](QDialog *) {});
        QCOMPARE(opened.size(), 1);
        QCOMPARE(QUrlQuery(opened[0]).queryItemValue(QStringLiteral("q")), QStringLiteral("this"));
        editor->deselect();
        withDialog([&] { e.win.w.act(QStringLiteral("review.translate"))->trigger(); }, QStringLiteral("translateDialog"), [](QDialog *) {});
        QCOMPARE(opened.size(), 2);
        QCOMPARE(QUrlQuery(opened[1]).queryItemValue(QStringLiteral("q")), QStringLiteral("say this slowly"));
        e.key(Qt::Key_Escape);
    }

    // The language of the selected cells is set (and only theirs), one Undo takes it back, and the spelling walk and the cell editor follow it.
    void setProofingLanguageMarksTheSelectedCellsAndSpellingFollowsIt()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        QVERIFY(e.win.w.act(QStringLiteral("review.language")));
        e.select("E5");
        e.enter("bonjour monde");
        e.select("E6");
        e.enter("bonjour monde");
        QVERIFY(e.settle());
        auto words = [&](quint32 row) {
            crunch::SpellWalk walk(e.win.w.session(), 0, jp::office::sheet::CellRange{row, 4, row, 4, true});
            QStringList found;
            while (auto hit = walk.next()) found << hit->word;
            return found;
        };
        QCOMPARE(words(4), (QStringList{QStringLiteral("bonjour"), QStringLiteral("monde")}));
        e.select("E5");
        chooseLanguage(e, QStringLiteral("fr-FR"));
        QCOMPARE(proofingOf(e, 4, 4).language, QStringLiteral("fr-FR"));
        QCOMPARE(proofingOf(e, 5, 4).language, QString());   // (only the selected cell)
        QVERIFY(words(4).isEmpty());
        QCOMPARE(words(5).size(), 2);
        // the cell editor of the cell checks in its language (a word is told of once the caret is past it)
        e.select("E5");
        e.type("bonjour monde ");
        QTest::qWait(300);
        QVERIFY(e.win.w.cellEditor()->misspellings().isEmpty());
        e.key(Qt::Key_Escape);
        e.select("E6");
        e.type("bonjour monde ");
        QTRY_COMPARE(e.win.w.cellEditor()->misspellings().size(), 2);
        e.key(Qt::Key_Escape);
        e.win.w.session()->undoStack()->undo();
        QCOMPARE(proofingOf(e, 4, 4).language, QString());
    }

    // Do Not Check Spelling marks the selected cells, shows as ticked on them, and the spelling walk passes them; again, and they are checked.
    void doNotCheckSpellingMarksCellsAndShowsTicked()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        QAction *noProof = e.win.w.act(QStringLiteral("review.noProof"));
        QVERIFY(noProof);
        e.select("E5");
        e.enter("mispelled");
        QVERIFY(e.settle());
        e.select("E5");
        QVERIFY(!noProof->isChecked());
        crunch::SpellWalk before(e.win.w.session(), 0, jp::office::sheet::CellRange{4, 4, 4, 4, true});
        QVERIFY(before.next().has_value());
        noProof->trigger();
        QVERIFY(proofingOf(e, 4, 4).noProof);
        QVERIFY(noProof->isChecked());
        crunch::SpellWalk after(e.win.w.session(), 0, jp::office::sheet::CellRange{4, 4, 4, 4, true});
        QVERIFY(!after.next().has_value());
        e.select("A1");
        QVERIFY(!noProof->isChecked());
        e.select("E5");
        QVERIFY(noProof->isChecked());
        noProof->trigger();
        QVERIFY(!proofingOf(e, 4, 4).noProof);
        crunch::SpellWalk again(e.win.w.session(), 0, jp::office::sheet::CellRange{4, 4, 4, 4, true});
        QVERIFY(again.next().has_value());
    }

    // An .ods keeps the language and the mark in the cell's style (fo:language and fo:country, and "zxx" for text that is not checked), and reads them again; the cell with neither
    // stays as it was, and the reopened workbook is checked in the language.
    void anOdsKeepsTheLanguageAndTheMark()
    {
        EditWindow e;
        QVERIFY(e.open(odsWords(), QStringLiteral("book.ods")));
        QVERIFY(e.win.w.act(QStringLiteral("review.language")));
        e.select("A1");
        chooseLanguage(e, QStringLiteral("fr-FR"));
        e.select("A2");
        e.win.w.act(QStringLiteral("review.noProof"))->trigger();
        QCOMPARE(proofingOf(e, 0, 0).language, QStringLiteral("fr-FR"));
        QVERIFY(proofingOf(e, 1, 0).noProof);
        const QString copy = e.folder.dir.filePath(QStringLiteral("copy.ods"));
        QVERIFY(savedCopy(e, copy));
        const Proofing a1 = proofingOfFile(copy, 0, 0), a2 = proofingOfFile(copy, 1, 0), a3 = proofingOfFile(copy, 2, 0);
        QCOMPARE(a1.language, QStringLiteral("fr-FR"));
        QVERIFY(!a1.noProof);
        QVERIFY(a2.noProof);
        QCOMPARE(a2.language, QString());
        QCOMPARE(a3.language, QString());
        QVERIFY(!a3.noProof);
        // the written file, opened again in a window, is checked by those marks
        EditWindow again;
        QFile written(copy);
        QVERIFY(written.open(QIODevice::ReadOnly));
        QVERIFY(again.open(written.readAll(), QStringLiteral("again.ods")));
        QCOMPARE(proofingOf(again, 0, 0).language, QStringLiteral("fr-FR"));
        crunch::SpellWalk walk(again.win.w.session(), 0, jp::office::sheet::CellRange{0, 0, 2, 0, true});
        QVERIFY(!walk.next().has_value());   // (the French words, the mark, and "plain")
    }

    // The XML workbook has no element for either, so they are kept in an extension of the cell format (which a reader that does not know it leaves alone): saved, and read again, in a
    // window too; a cell with neither is as it was.
    void anXlsxKeepsThemInAnExtensionOfTheCellFormat()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        QVERIFY(e.win.w.act(QStringLiteral("review.language")));
        e.select("E5");
        e.enter("bonjour");
        e.select("E6");
        e.enter("mispelled");
        QVERIFY(e.settle());
        e.select("E5");
        chooseLanguage(e, QStringLiteral("fr-FR"));
        e.select("E6");
        e.win.w.act(QStringLiteral("review.noProof"))->trigger();
        const QString path = e.folder.dir.filePath(QStringLiteral("saved.xlsx"));
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.saveAs(path));
        QVERIFY(saved.count() > 0 || saved.wait(20000));
        QVERIFY(saved.last().value(1).toBool());
        QCOMPARE(proofingOfFile(path, 4, 4).language, QStringLiteral("fr-FR"));
        QVERIFY(!proofingOfFile(path, 4, 4).noProof);
        QVERIFY(proofingOfFile(path, 5, 4).noProof);
        QCOMPARE(proofingOfFile(path, 5, 4).language, QString());
        QCOMPARE(proofingOfFile(path, 0, 0).language, QString());   // (a cell with neither)
        EditWindow again;
        QFile written(path);
        QVERIFY(written.open(QIODevice::ReadOnly));
        QVERIFY(again.open(written.readAll(), QStringLiteral("again.xlsx")));
        QCOMPARE(proofingOf(again, 4, 4).language, QStringLiteral("fr-FR"));
        QVERIFY(proofingOf(again, 5, 4).noProof);
        // a change of the language, saved over the file, replaces the old one
        again.select("E5");
        chooseLanguage(again, QStringLiteral("de-DE"));
        QSignalSpy savedAgain(&again.win.w, &CrunchWindow::saved);
        QVERIFY(again.win.w.save());
        QVERIFY(savedAgain.count() > 0 || savedAgain.wait(20000));
        QCOMPARE(proofingOfFile(again.path, 4, 4).language, QStringLiteral("de-DE"));
    }
};

JP_TEST_CLASS(CrunchLanguageTests)
#include "tst_crunch_language.moc"
