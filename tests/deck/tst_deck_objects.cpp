// Tests of JeffDeck's objects of other apps (Insert > Object, Paste Special as an object, double-click to edit, the list). Every file is made here in code; no other program is started
// (the edit's opener is a stand-in that notes the path).
#include "../isolatesettings.h"
#include "../withdialog.h"
#include "app/objects.h"
#include "app/objectsdialog.h"
#include "app/theme.h"
#include "deckhelpers.h"
#include "io/pptxexport.h"
#include "io/zip.h"
#include "office/chart.h"
#include "office/chart_write.h"
#include "office/package.h"
#include "office/theme.h"

#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QPushButton>
#include <QSignalSpy>
#include <QStatusBar>
#include <QTemporaryDir>

namespace ole = jp::office::ole;

namespace {

objects::Grid gridOf(const QStringList &texts, int columns)
{
    objects::Grid grid;
    for (int i = 0; i < texts.size(); i += columns) {
        QVector<objects::CellValue> row;
        for (int c = 0; c < columns && i + c < texts.size(); ++c) {
            objects::CellValue v;
            v.text = texts[i + c];
            bool number = false;
            v.value = v.text.toDouble(&number);
            v.number = number;
            row.push_back(v);
        }
        grid.push_back(row);
    }
    return grid;
}

QByteArray withChart(const QByteArray &zip)
{
    QMap<QString, QByteArray> parts;
    readZip(zip, parts);
    parts[QStringLiteral("xl/charts/chart1.xml")] = office::writeChartPart(office::blankChart(office::defaultTheme()));
    QByteArray types = parts.value(QStringLiteral("[Content_Types].xml"));
    types.replace("</Types>", "<Override PartName=\"/xl/charts/chart1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.drawingml.chart+xml\"/></Types>");
    parts[QStringLiteral("[Content_Types].xml")] = types;
    ZipWriter zw;
    for (auto it = parts.cbegin(); it != parts.cend(); ++it) zw.add(it.key(), it.value(), true);
    return zw.finish();
}

QByteArray docxOf(const QStringList &paragraphs)
{
    office::PackageWriter w;
    const QString rel = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/");
    QByteArray body;
    for (const QString &p : paragraphs) body += "<w:p><w:r><w:t xml:space=\"preserve\">" + p.toUtf8() + "</w:t></w:r></w:p>";
    w.addPart(QStringLiteral("/word/document.xml"),
              QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><w:document xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\"><w:body>") + body + "</w:body></w:document>",
              QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"));
    w.addRelationship(QStringLiteral("/"), rel + QStringLiteral("officeDocument"), QStringLiteral("/word/document.xml"));
    return w.finish();
}

QString savedFile(const QTemporaryDir &dir, const QString &name, const QByteArray &bytes)
{
    const QString path = dir.filePath(name);
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly)) return QString();
    f.write(bytes);
    return path;
}


std::unique_ptr<Document> sample()
{
    auto d = deck::newPresentation();
    deck::addSlide(*d, *deck::layoutOfType(*d, QStringLiteral("obj")));
    return d;
}

QVector<QString> objectItems(const Document &d)
{
    QVector<QString> ids;
    for (const auto &page : d.pages)
        for (const QJsonValue &v : page->props.value(QStringLiteral("charts")).toArray())
            if (v.toObject().value(QStringLiteral("kind")).toString() == QLatin1String("object")) ids << v.toObject().value(QStringLiteral("item")).toString();
    return ids;
}

std::optional<ole::EmbeddedObject> objectOnSlide(const Document &d, int slide, const QString &item) { return deck::pageObjectOf(d, *d.pages[slide], item); }

} // namespace

class DeckObjectTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void initTestCase() { initDeckTestCase(); }

    void aRangeAChartAndAPageAreEachPutInWithAPicture()
    {
        QTemporaryDir dir;
        const QString book = savedFile(dir, QStringLiteral("book.xlsx"), objects::rangePackage(gridOf({"Name", "Total", "North", "12", "South", "30"}, 2)));
        const QString chart = savedFile(dir, QStringLiteral("chart.xlsx"), withChart(objects::rangePackage(gridOf({"a", "1"}, 2))));
        const QString page = savedFile(dir, QStringLiteral("page.docx"), docxOf({QStringLiteral("First page words")}));
        DeckWindow w;
        w.editor()->setDocument(sample());
        QVERIFY(w.insertObjectFrom(objects::Source::Range, book, QStringLiteral("A1:B3")));
        QVERIFY(w.insertObjectFrom(objects::Source::Chart, chart, QString()));
        QVERIFY(w.insertObjectFrom(objects::Source::Page, page, QStringLiteral("1")));
        const QVector<QString> items = objectItems(*w.editor()->doc());
        QCOMPARE(items.size(), 3);
        QStringList kinds;
        for (const QString &id : items) {
            const auto o = objectOnSlide(*w.editor()->doc(), 0, id);
            QVERIFY(o && o->isOurs);
            QVERIFY(QImage::fromData(o->previewPng, "PNG").width() > 0);
            kinds << objects::kindWords(*o);
        }
        QCOMPARE(kinds, (QStringList{QStringLiteral("Spreadsheet range"), QStringLiteral("Chart"), QStringLiteral("Document page")}));
        // (A page past the end is said, and nothing is put in.)
        QVERIFY(!w.insertObjectFrom(objects::Source::Page, page, QStringLiteral("9")));
        QVERIFY(w.statusBar()->currentMessage().contains(QStringLiteral("page")));
        QCOMPARE(objectItems(*w.editor()->doc()).size(), 3);
        // One step undoes an insert.
        w.editor()->undoStack()->undo();
        QCOMPARE(objectItems(*w.editor()->doc()).size(), 2);
    }

    void theObjectsSurviveSaveAndReopen()
    {
        QTemporaryDir dir;
        const QString book = savedFile(dir, QStringLiteral("book.xlsx"), objects::rangePackage(gridOf({"a", "1", "b", "2"}, 2)));
        const QString page = savedFile(dir, QStringLiteral("page.docx"), docxOf({QStringLiteral("Words")}));
        DeckWindow w;
        w.editor()->setDocument(sample());
        QVERIFY(w.insertObjectFrom(objects::Source::Range, book, QString()));
        QVERIFY(w.insertObjectFrom(objects::Source::Page, page, QStringLiteral("1")));
        const QVector<QString> before = objectItems(*w.editor()->doc());
        QCOMPARE(before.size(), 2);
        const auto first = objectOnSlide(*w.editor()->doc(), 0, before[0]);
        QVERIFY(first);
        const QString file = dir.filePath(QStringLiteral("out.pptx"));
        QVERIFY(w.saveTo(file));
        DeckWindow again;
        QVERIFY(again.openFile(file));
        const QVector<QString> after = objectItems(*again.editor()->doc());
        QCOMPARE(after.size(), 2);
        int sheets = 0, pages = 0;
        for (const QString &id : after) {
            const auto o = objectOnSlide(*again.editor()->doc(), 0, id);
            QVERIFY(o && o->isOurs);
            QVERIFY(!o->previewPng.isEmpty());
            sheets += o->kind() == ole::Kind::Sheet;
            pages += o->kind() == ole::Kind::Page;
            if (o->kind() == ole::Kind::Sheet) QCOMPARE(o->partBytes, first->partBytes);
        }
        QCOMPARE(sheets, 1);
        QCOMPARE(pages, 1);
    }

    void doubleClickStartsTheEditAndASaveIsOneStep()
    {
        QTemporaryDir dir, temp;
        const QString book = savedFile(dir, QStringLiteral("book.xlsx"), objects::rangePackage(gridOf({"Name", "Total", "North", "12"}, 2)));
        DeckWindow w;
        w.resize(1200, 800);
        w.show();
        w.editor()->setDocument(sample());
        EmbeddedEdit *ed = w.objectEditor();
        ed->setTempRoot(temp.path());
        ed->setPollInterval(600000);
        QStringList opened;
        ed->setOpener([&](const QString &path, ole::Kind) {
            opened << path;
            return true;
        });
        QVERIFY(w.insertObjectFrom(objects::Source::Range, book, QStringLiteral("A1:B2")));
        const QString item = objectItems(*w.editor()->doc()).first();
        const auto old = objectOnSlide(*w.editor()->doc(), 0, item);
        QVERIFY(old);
        w.editor()->select(item);
        Item *shown = w.editor()->doc()->item(item);
        QVERIFY(shown);
        const QPoint at = w.canvas()->pageToView(shown->rect.center()).toPoint();
        QTest::mouseDClick(w.canvas()->viewport(), Qt::LeftButton, Qt::NoModifier, at);
        QTRY_COMPARE(opened.size(), 1);
        QSignalSpy saves(ed, &EmbeddedEdit::objectSaved);
        const QByteArray changed = objects::rangePackage(gridOf({"Name", "Total", "North", "99", "West", "5"}, 2));
        {
            QFile f(opened.first());
            QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Truncate));
            f.write(changed);
        }
        QTRY_VERIFY((ed->pollNow(), saves.count() >= 1));
        const auto now = objectOnSlide(*w.editor()->doc(), 0, item);
        QVERIFY(now);
        QCOMPARE(now->partBytes, changed);
        QVERIFY(now->previewPng != old->previewPng);
        // One step undoes it.
        w.editor()->undoStack()->undo();
        const auto undone = objectOnSlide(*w.editor()->doc(), 0, item);
        QVERIFY(undone);
        QCOMPARE(undone->partBytes, old->partBytes);
        QCOMPARE(undone->previewPng, old->previewPng);
    }

    void aMissingProgramIsSaidInTheStatusBar()
    {
        QTemporaryDir dir, temp;
        const QString book = savedFile(dir, QStringLiteral("book.xlsx"), objects::rangePackage(gridOf({"a", "1"}, 2)));
        DeckWindow w;
        w.editor()->setDocument(sample());
        w.objectEditor()->setTempRoot(temp.path());
        w.objectEditor()->setOpener([](const QString &, ole::Kind) { return false; });
        QVERIFY(w.insertObjectFrom(objects::Source::Range, book, QString()));
        QVERIFY(!w.editObject(objectItems(*w.editor()->doc()).first()));
        QVERIFY(w.statusBar()->currentMessage().contains(QStringLiteral("can't be opened")));
        QCOMPARE(w.objectEditor()->openCount(), 0);
    }

    void pasteSpecialAsObjectPutsAnObjectIn()
    {
        const auto object = objects::rangeObject(gridOf({"x", "1", "y", "2"}, 2));
        QVERIFY(object);
        DeckWindow w;
        w.editor()->setDocument(sample());
        auto *mime = new QMimeData;
        mime->setText(QStringLiteral("plain words"));
        QApplication::clipboard()->setMimeData(objects::withObject(mime, [object] { return object; }));
        QTRY_VERIFY(w.act(QStringLiteral("edit.pasteObject"))->isEnabled());
        // Plain paste stays as it was: no object.
        w.editor()->paste();
        QCOMPARE(objectItems(*w.editor()->doc()).size(), 0);
        w.act(QStringLiteral("edit.pasteObject"))->trigger();
        const QVector<QString> items = objectItems(*w.editor()->doc());
        QCOMPARE(items.size(), 1);
        const auto put = objectOnSlide(*w.editor()->doc(), 0, items.first());
        QVERIFY(put && put->isOurs);
        QCOMPARE(put->partBytes, object->partBytes);
    }

    void theListShowsWordsAndConvertLeavesAPictureWithNoPart()
    {
        QTemporaryDir dir;
        const QString book = savedFile(dir, QStringLiteral("book.xlsx"), objects::rangePackage(gridOf({"a", "1", "b", "2"}, 2)));
        const QString page = savedFile(dir, QStringLiteral("page.docx"), docxOf({QStringLiteral("Words")}));
        DeckWindow w;
        w.editor()->setDocument(sample());
        QVERIFY(w.insertObjectFrom(objects::Source::Range, book, QString()));
        QVERIFY(w.insertObjectFrom(objects::Source::Page, page, QStringLiteral("1")));
        const QVector<QString> items = objectItems(*w.editor()->doc());
        QCOMPARE(items.size(), 2);
        withDialog([&] { w.showObjects(); }, QStringLiteral("objectListDialog"), [&](QDialog *d) {
            auto *list = qobject_cast<objects::ObjectListDialog *>(d);
            QVERIFY(list);
            shotOf(d, QStringLiteral("deck-objects-list"));
            QCOMPARE(list->rowCount(), 2);
            QCOMPARE(list->textAt(0, 1), QStringLiteral("Spreadsheet range"));
            QCOMPARE(list->textAt(1, 1), QStringLiteral("Document page"));
            for (int i = 0; i < 2; ++i) {
                QVERIFY(!list->textAt(i, 2).isEmpty());
                QVERIFY(!list->textAt(i, 1).contains(QStringLiteral(".")));   // (no program ID)
            }
            list->select(0);
            d->findChild<QPushButton *>(QStringLiteral("objectConvert"))->click();
            QCOMPARE(list->rowCount(), 1);
        });
        // The first is a picture with no part now; the other is still an object.
        QCOMPARE(objectItems(*w.editor()->doc()).size(), 1);
        QVERIFY(w.editor()->doc()->item(items[0]));
        QVERIFY(!objectOnSlide(*w.editor()->doc(), 0, items[0]));
        // Saved, the file holds one package.
        const QString file = dir.filePath(QStringLiteral("out.pptx"));
        QVERIFY(w.saveTo(file));
        office::Package pkg;
        QVERIFY(pkg.readFile(file));
        int embeddings = 0;
        for (const QString &name : pkg.partNames())
            if (name.contains(QLatin1String("embeddings"))) ++embeddings;
        QCOMPARE(embeddings, 1);
        withDialog([&] { w.showObjects(); }, QStringLiteral("objectListDialog"), [&](QDialog *d) {
            d->findChild<QPushButton *>(QStringLiteral("objectDelete"))->click();
            QCOMPARE(qobject_cast<objects::ObjectListDialog *>(d)->rowCount(), 0);
        });
        QCOMPARE(objectItems(*w.editor()->doc()).size(), 0);
        QVERIFY(!w.editor()->doc()->item(items[1]));
    }

    void screenshotsOfTheInsertMenuAndTheList()
    {
        const QString dir = qEnvironmentVariable("JO_SHOT_DIR");
        if (dir.isEmpty()) QSKIP("JO_SHOT_DIR is not set");
        jp::applyUiTheme(1);
        QTemporaryDir files;
        const QString book = savedFile(files, QStringLiteral("book.xlsx"), objects::rangePackage(gridOf({"Name", "Total", "North", "12", "South", "30"}, 2)));
        DeckWindow w;
        w.resize(1200, 800);
        w.show();
        w.editor()->setDocument(sample());
        QVERIFY(w.insertObjectFrom(objects::Source::Range, book, QString()));
        QMenu menu;
        for (const char *id : {"insert.object.range", "insert.object.chart", "insert.object.page"}) menu.addAction(w.act(QString::fromLatin1(id)));
        menu.show();
        QVERIFY(menu.grab().save(dir + QStringLiteral("/deck-insert-object-menu.png")));
        QVERIFY(w.grab().save(dir + QStringLiteral("/deck-window-with-object.png")));
    }
};

JP_TEST_CLASS(DeckObjectTests)
#include "tst_deck_objects.moc"
