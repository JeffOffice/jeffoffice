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
#include "sheet/proofingext.h"
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
#include <QRegularExpression>
#include <QUrlQuery>
#include <QXmlStreamReader>
#include "io/zip.h"
#include "text/dictionaries.h"

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

// The cell's text, set in a window's session (as a person's entry).
void put(EditWindow &e, quint32 row, quint32 col, const QString &text) { QVERIFY(e.win.w.session()->type(0, row, col, text, false).ok); }

// The words of the walk over a range.
QStringList wordsOf(EditWindow &e, quint32 row1, quint32 col1, quint32 row2, quint32 col2)
{
    crunch::SpellWalk walk(e.win.w.session(), 0, jp::office::sheet::CellRange{row1, col1, row2, col2, true});
    QStringList found;
    while (auto hit = walk.next()) found << hit->word;
    return found;
}

QMap<QString, QByteArray> partsOf(const QByteArray &zip)
{
    QMap<QString, QByteArray> parts;
    jp::readZip(zip, parts);
    return parts;
}

QByteArray zipOf(const QMap<QString, QByteArray> &parts)
{
    jp::ZipWriter z;
    for (auto it = parts.begin(); it != parts.end(); ++it) z.add(it.key(), it.value(), true);
    return z.finish();
}

// A workbook of one sheet whose cell formats are `xfs`, A1 to A3 using the first three (A1 holds "bonjour monde").
QByteArray bookWithXfs(const QString &xfs)
{
    const QString rows = QStringLiteral("<row r=\"1\"><c r=\"A1\" t=\"inlineStr\" s=\"0\"><is><t>bonjour monde</t></is></c></row><row r=\"2\"><c r=\"A2\" t=\"inlineStr\" s=\"1\"><is><t>bonjour monde</t></is></c></row>");
    const QString lists = QStringLiteral("<fonts count=\"1\"><font><sz val=\"11\"/><name val=\"Calibri\"/><family val=\"2\"/></font></fonts>"
                                         "<fills count=\"2\"><fill><patternFill patternType=\"none\"/></fill><fill><patternFill patternType=\"gray125\"/></fill></fills>"
                                         "<borders count=\"1\"><border><left/><right/><top/><bottom/><diagonal/></border></borders>"
                                         "<cellStyles count=\"1\"><cellStyle name=\"Normal\" xfId=\"0\" builtinId=\"0\"/></cellStyles>");
    return buildXlsx({worksheet(rows)}, QByteArray(), QString(), QString(), stylesXml(QString(), xfs, lists));
}

QByteArray withPrefix(const QByteArray &xml, const QString &prefix)
{
    QString text = QString::fromUtf8(xml);
    static const QRegularExpression open(QStringLiteral("<(/?)([A-Za-z][A-Za-z0-9]*)(?=[ />\\n])"));   // (an element that has a prefix already is left)
    text.replace(open, QStringLiteral("<\\1") + prefix + QStringLiteral(":\\2"));
    text.replace(QStringLiteral(" xmlns=\""), QStringLiteral(" xmlns:") + prefix + QStringLiteral("=\""));
    return text.toUtf8();
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
        e.type("bonjour monde speling ");
        // (the words are told of in one pass: "speling" is wrong in French too, so the pass is seen to be done, and "bonjour" and "monde" are not told of)
        QTRY_COMPARE(e.win.w.cellEditor()->misspellings().size(), 1);
        QCOMPARE(e.win.w.cellEditor()->misspellings().first(), qMakePair(14, 21));
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

    // A cell with no language and a text that begins with a language tag and a line break is not taken for the cell of that language with the rest as its text.
    void theSpellingCacheKeepsALanguageApartFromTheTextAfterIt()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        put(e, 4, 4, QStringLiteral("bonjour"));
        put(e, 5, 4, QStringLiteral("fr-FR\nbonjour"));
        QVERIFY(e.settle());
        e.select("E5");
        chooseLanguage(e, QStringLiteral("fr-FR"));
        crunch::SpellWalk walk(e.win.w.session(), 0, jp::office::sheet::CellRange{4, 4, 5, 4, true});
        QStringList found;
        while (auto hit = walk.next()) found << hit->word;
        QVERIFY2(found.contains(QStringLiteral("bonjour")), qPrintable(found.join(QLatin1Char('|'))));   // (the second cell, in the program's language)
        QCOMPARE(proofingOf(e, 5, 4).language, QString());
    }

    // "fr", "FR-fr", and "fr_FR" check in French, and the tag is kept as it was written.
    void bareLanguageTagsCheckInTheLanguage()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        quint32 row = 4;
        for (const QString &tag : {QStringLiteral("fr"), QStringLiteral("FR-fr"), QStringLiteral("fr_FR")}) {
            put(e, row, 4, QStringLiteral("bonjour monde"));
            jp::office::sheet::FormatChange change;
            change.language = tag;
            QVERIFY(e.win.w.session()->formatCells(0, jp::office::sheet::CellRange{row, 4, row, 4, true}, change, QStringLiteral("t")).ok);
            QCOMPARE(proofingOf(e, row, 4).language, tag);
            QVERIFY2(wordsOf(e, row, 4, row, 4).isEmpty(), qPrintable(tag));
            ++row;
        }
        QVERIFY(e.win.w.session()->type(0, row, 4, QStringLiteral("bonjour monde"), false).ok);
        QCOMPARE(wordsOf(e, row, 4, row, 4).size(), 2);   // (the check is real: a cell with none is in the program's language)
    }

    // A document whose default style says French checks its cells that state no language in French, in an .ods read, and written again.
    void anOdsDefaultLanguageIsTheLanguageOfCellsThatStateNone()
    {
        const QByteArray ods = buildOds(contentXml(table(QStringLiteral("S"), columns(2) + row(stringCell(QStringLiteral("bonjour monde"))))),
                                        QStringLiteral("<style:default-style style:family=\"table-cell\"><style:text-properties fo:language=\"fr\" fo:country=\"FR\"/></style:default-style>"));
        EditWindow e;
        QVERIFY(e.open(ods, QStringLiteral("book.ods")));
        QCOMPARE(proofingOf(e, 0, 0).language, QStringLiteral("fr-FR"));
        QVERIFY(wordsOf(e, 0, 0, 0, 0).isEmpty());
        const QString copy = e.folder.dir.filePath(QStringLiteral("copy.ods"));
        QVERIFY(savedCopy(e, copy));
        QCOMPARE(proofingOfFile(copy, 0, 0).language, QStringLiteral("fr-FR"));
    }

    // Thousands of different language tags, long ones among them, in an .xlsx and in an .ods: the file opens, no tag is kept longer than a tag may be, and the dictionaries keep no more
    // checkers than there are languages.
    void thousandsOfLanguageTagsStayBounded()
    {
        const int n = 4000;
        QString xfs;
        for (int i = 0; i < n; ++i)
            xfs += QStringLiteral("<xf numFmtId=\"0\"><extLst><ext uri=\"%1\" xmlns:jo=\"%2\"><jo:proofing lang=\"xx-%3%4\"/></ext></extLst></xf>")
                       .arg(QLatin1String(jp::office::sheet::proofingExtensionUri()), QLatin1String(jp::office::sheet::proofingExtensionNamespace()), QString::number(i), QString(200, QLatin1Char('q')));
        QString styles, cells;
        for (int i = 0; i < n; ++i) {
            styles += QStringLiteral("<style:style style:name=\"s%1\" style:family=\"table-cell\"><style:text-properties fo:language=\"yy%1%2\" fo:country=\"Z%1\"/></style:style>").arg(i).arg(QString(200, QLatin1Char('q')));
            cells += stringCell(QStringLiteral("bonjour"), QStringLiteral("table:style-name=\"s%1\"").arg(i));
        }
        const QByteArray ods = buildOds(contentXml(table(QStringLiteral("S"), columns(1) + row(cells)), styles));
        const QByteArray xlsx = bookWithXfs(xfs);
        QTemporaryDir dir;
        for (const auto &[name, bytes] : {std::pair<QString, QByteArray>{QStringLiteral("many.ods"), ods}, {QStringLiteral("many.xlsx"), xlsx}}) {
            QFile f(dir.filePath(name));
            QVERIFY(f.open(QIODevice::WriteOnly));
            f.write(bytes);
            f.close();
            jp::office::sheet::Workbook wb;
            QString why;
            QVERIFY2(wb.openFile(dir.filePath(name), &why), qPrintable(name + why));
            QVERIFY2(wb.styles().formats.size() <= n + 8, qPrintable(name));
            for (const auto &fmt : wb.styles().formats) QVERIFY2(fmt.language.size() <= 35, qPrintable(name));
        }
        EditWindow e;
        QVERIFY(e.open(ods, QStringLiteral("again.ods")));
        const int before = dict::loadedCount();
        crunch::SpellWalk walk(e.win.w.session(), 0, jp::office::sheet::CellRange{0, 0, 0, quint32(n), true});
        while (walk.next()) {}
        QVERIFY(dict::loadedCount() <= std::max(before, int(dict::languages().size())));
        QVERIFY(dict::loadedCount() - before <= 1);
        // (and a tag that names no language of the list, asked of the dictionaries themselves, is not kept)
        const int kept = dict::loadedCount();
        for (int i = 0; i < n; ++i) {
            dict::hasSpelling(QStringLiteral("zz-%1").arg(i));
            dict::spell(QStringLiteral("zz-%1").arg(i), QStringLiteral("word"));
        }
        QCOMPARE(dict::loadedCount(), kept);
    }

    // The box offers the language the cell has (a bare "fr" chooses French), else the language the program checks in: as JeffWrite's offers its document's.
    void theLanguageBoxPreselectsLikeJeffWrites()
    {
        const jptest::SettingGuard language(QStringLiteral("proof/language"), QStringLiteral("en-US"), QString());
        EditWindow e;
        QVERIFY(e.open(editBook()));
        e.select("E5");
        auto chosen = [&] {
            QString code;
            withDialog([&] { e.win.w.act(QStringLiteral("review.language"))->trigger(); }, QStringLiteral("languageDialog"), [&](QDialog *d) {
                code = d->findChild<QListWidget *>(QStringLiteral("languages"))->currentItem()->data(Qt::UserRole).toString();
            }, false);
            return code;
        };
        QCOMPARE(chosen(), QStringLiteral("en-US"));
        for (const QString &tag : {QStringLiteral("fr"), QStringLiteral("FR-fr"), QStringLiteral("fr_FR")}) {
            jp::office::sheet::FormatChange change;
            change.language = tag;
            QVERIFY(e.win.w.session()->formatCells(0, jp::office::sheet::CellRange{4, 4, 4, 4, true}, change, QStringLiteral("t")).ok);
            QCOMPARE(chosen(), QStringLiteral("fr-FR"));
        }
    }

    // A named style, the Format Cells result, and a change of another look keep the language and the mark; Clear Formats takes them away with the rest of the format.
    void aNamedStyleAndFormatCellsKeepTheLanguageAndClearFormatsResetsIt()
    {
        EditWindow e;
        QVERIFY(e.open(bookWithXfs(QStringLiteral("<xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"0\"/><xf numFmtId=\"0\" fontId=\"0\" fillId=\"0\" borderId=\"0\" xfId=\"0\"/>"))));
        crunch::EditSession *s = e.win.w.session();
        QVERIFY(e.settle());
        const jp::office::sheet::CellRange cell{0, 0, 0, 0, true};
        jp::office::sheet::FormatChange mark;
        mark.language = QStringLiteral("fr-FR");
        mark.noProof = true;
        QVERIFY(s->formatCells(0, cell, mark, QStringLiteral("t")).ok);
        QVERIFY(s->applyStyleNamed(0, cell, QStringLiteral("Good"), QStringLiteral("t")).ok);
        QCOMPARE(proofingOf(e, 0, 0).language, QStringLiteral("fr-FR"));
        QVERIFY(proofingOf(e, 0, 0).noProof);
        jp::office::sheet::FormatChange look;   // (what the Format Cells box hands over: no language, no mark)
        look.bold = true;
        look.horizontal = jp::office::sheet::HAlign::Center;
        QVERIFY(s->formatCells(0, cell, look, QStringLiteral("Format Cells")).ok);
        QCOMPARE(proofingOf(e, 0, 0).language, QStringLiteral("fr-FR"));
        QVERIFY(proofingOf(e, 0, 0).noProof);
        QVERIFY(s->clearFormats(0, cell).ok);
        QCOMPARE(proofingOf(e, 0, 0).language, QString());
        QVERIFY(!proofingOf(e, 0, 0).noProof);
    }

    // A workbook saved with no change of a format has the styles part it had, byte for byte.
    void aSaveThatChangesNoFormatLeavesTheStylesPartAsItWas()
    {
        EditWindow e;
        const QByteArray book = editBook();
        QVERIFY(e.open(book));
        put(e, 4, 4, QStringLiteral("typed"));
        QVERIFY(e.settle());
        const QString path = e.folder.dir.filePath(QStringLiteral("same.xlsx"));
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.saveAs(path));
        QVERIFY(saved.count() > 0 || saved.wait(20000));
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadOnly));
        QCOMPARE(partsOf(f.readAll()).value(QStringLiteral("xl/styles.xml")), partsOf(book).value(QStringLiteral("xl/styles.xml")));
    }

    // A format with an extension of another program's keeps it when the language is added, and an extension of ours that a program rewrote with a line break after `<ext` is replaced,
    // not joined by a second; in a part whose elements have a prefix too.
    void aForeignExtensionIsKeptAndOurOwnIsReplacedWhateverThePartLooksLike()
    {
        const QString foreign = QStringLiteral("<extLst><ext uri=\"{AAAAAAAA-0000-4000-8000-000000000001}\" xmlns:f=\"urn:foreign\"><f:thing a=\"1\"/></ext></extLst>");
        const QString ours = QStringLiteral("<extLst><ext\n uri=\"%1\"\n xmlns:jo=\"%2\"><jo:proofing lang=\"de-DE\"/></ext></extLst>")
                                 .arg(QLatin1String(jp::office::sheet::proofingExtensionUri()), QLatin1String(jp::office::sheet::proofingExtensionNamespace()));
        for (const bool prefixed : {false, true}) {
            QByteArray book = bookWithXfs(QStringLiteral("<xf numFmtId=\"0\">%1</xf><xf numFmtId=\"0\">%2</xf>").arg(foreign, ours));
            if (prefixed) {
                QMap<QString, QByteArray> parts = partsOf(book);
                for (const char *name : {"xl/workbook.xml", "xl/styles.xml", "xl/worksheets/sheet1.xml"}) parts[QString::fromLatin1(name)] = withPrefix(parts[QString::fromLatin1(name)], QStringLiteral("x"));
                book = zipOf(parts);
            }
            EditWindow e;
            QVERIFY(e.open(book));
            QVERIFY2(proofingOf(e, 1, 0).language == QStringLiteral("de-DE"), prefixed ? "prefixed: not read" : "plain: not read");   // (read from the extension)
            e.select("A1");
            chooseLanguage(e, QStringLiteral("fr-FR"));
            e.select("A2");
            chooseLanguage(e, QStringLiteral("es-MX"));
            const QString path = e.folder.dir.filePath(QStringLiteral("ext.xlsx"));
            QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
            QVERIFY(e.win.w.saveAs(path));
            QVERIFY(saved.count() > 0 || saved.wait(20000));
            QFile f(path);
            QVERIFY(f.open(QIODevice::ReadOnly));
            const QByteArray styles = partsOf(f.readAll()).value(QStringLiteral("xl/styles.xml"));
            QVERIFY2(styles.contains("{AAAAAAAA-0000-4000-8000-000000000001}"), prefixed ? "prefixed" : "plain");   // (the other program's extension is kept)
            // (the two formats the file had stay, one of ours in the second; the two the language made add one each; no format has two)
            QCOMPARE(styles.count(jp::office::sheet::proofingExtensionUri()), 3);
            const QRegularExpression oneFormat(QStringLiteral("<(?:x:)?xf[ >].*?</(?:x:)?xf>"), QRegularExpression::DotMatchesEverythingOption);
            for (auto it = oneFormat.globalMatch(QString::fromUtf8(styles)); it.hasNext();) QVERIFY(it.next().captured().count(QLatin1String(jp::office::sheet::proofingExtensionUri())) <= 1);
            if (prefixed) QVERIFY(styles.contains("<x:extLst>"));
            QXmlStreamReader xml(styles);
            while (!xml.atEnd()) xml.readNext();
            QVERIFY2(!xml.hasError(), qPrintable(xml.errorString()));
            QCOMPARE(proofingOfFile(path, 0, 0).language, QStringLiteral("fr-FR"));
            QCOMPARE(proofingOfFile(path, 1, 0).language, QStringLiteral("es-MX"));
        }
    }
};

JP_TEST_CLASS(CrunchLanguageTests)
#include "tst_crunch_language.moc"
