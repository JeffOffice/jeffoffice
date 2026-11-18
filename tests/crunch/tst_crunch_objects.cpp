// Tests of JeffCrunch's objects of other apps (Insert > Object, Paste Special as an object, double-click to edit, the list). Every file is made here in code; no other program is started
// (the edit's opener is a stand-in that notes the path).
#include "../withdialog.h"
#include "app/crunchwindow.h"
#include "app/objects.h"
#include "app/objectsdialog.h"
#include "app/theme.h"
#include "core/fonts.h"
#include "core/sheetobjects.h"
#include "crunchbooks.h"
#include "crunchwindowhelpers.h"
#include "editwindow.h"
#include "grid/gridview.h"
#include "io/zip.h"
#include "office/chart.h"
#include "office/chart_write.h"
#include "office/package.h"
#include "office/theme.h"
#include "savedok.h"

#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QPushButton>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QUndoStack>

#include "../testrunner.h"

using namespace jp;
using namespace crunchtest;
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


bool ready(EditWindow &e)
{
    if (!e.open(sheetOf({"A1=1"})) || !e.settle()) return false;
    return QTest::qWaitFor([&] { return e.win.w.objects()->shownSheet() == 0; }, 30000);
}

} // namespace

class CrunchObjectTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void initTestCase() { jp::initCore(); }

    void aPageFromAFileIsPutInAsAPictureThatCarriesIt()
    {
        QTemporaryDir dir;
        const QString page = savedFile(dir, QStringLiteral("page.docx"), docxOf({QStringLiteral("First page words")}));
        EditWindow e;
        QVERIFY(ready(e));
        QVERIFY(e.win.w.insertObjectFrom(objects::Source::Page, page, QStringLiteral("1")));
        QCOMPARE(e.win.w.objects()->layer()->count(), 1);
        const auto found = e.win.w.carriedObjects();
        QCOMPARE(int(found.size()), 1);
        QVERIFY(found[0].carried.object.isOurs);
        QCOMPARE(found[0].carried.object.kind(), ole::Kind::Page);
        QCOMPARE(objects::kindWords(found[0].carried.object), QStringLiteral("Document page"));
        QVERIFY(QImage::fromData(found[0].carried.object.previewPng, "PNG").width() > 0);
        // (A page past the end is said, and nothing is put in; the one step undoes.)
        QVERIFY(!e.win.w.insertObjectFrom(objects::Source::Page, page, QStringLiteral("9")));
        QVERIFY(e.win.w.statusText().contains(QStringLiteral("page")));
        QCOMPARE(e.win.w.objects()->layer()->count(), 1);
        e.win.w.undoStack()->undo();
        QTRY_COMPARE(int(e.win.w.carriedObjects().size()), 0);
    }

    void theObjectSurvivesSaveAndReopen()
    {
        QTemporaryDir dir;
        const QString page = savedFile(dir, QStringLiteral("page.docx"), docxOf({QStringLiteral("Words")}));
        EditWindow e;
        QVERIFY(ready(e));
        QVERIFY(e.win.w.insertObjectFrom(objects::Source::Page, page, QStringLiteral("1")));
        const auto before = e.win.w.carriedObjects();
        QCOMPARE(int(before.size()), 1);
        QSignalSpy saved(&e.win.w, &CrunchWindow::saved);
        QVERIFY(e.win.w.save());
        QVERIFY2(savedOk(saved), "the save failed");
        const QString copy = e.copyOfFile();
        QVERIFY(!copy.isEmpty());
        EditWindow again;
        QVERIFY(again.win.open(copy));
        QVERIFY(QTest::qWaitFor([&] { return again.win.w.objects()->shownSheet() == 0; }, 30000));
        QTRY_COMPARE(int(again.win.w.carriedObjects().size()), 1);
        const auto after = again.win.w.carriedObjects();
        QCOMPARE(after[0].carried.object.partBytes, before[0].carried.object.partBytes);
        QCOMPARE(after[0].carried.token, before[0].carried.token);
    }

    void aDoubleClickStartsTheEditAndASaveIsOneStep()
    {
        QTemporaryDir dir, temp;
        const QString page = savedFile(dir, QStringLiteral("page.docx"), docxOf({QStringLiteral("Words")}));
        EditWindow e;
        QVERIFY(ready(e));
        jp::EmbeddedEdit *ed = e.win.w.objectEditor();
        ed->setTempRoot(temp.path());
        ed->setPollInterval(600000);
        QStringList opened;
        ed->setOpener([&](const QString &path, ole::Kind) {
            opened << path;
            return true;
        });
        QVERIFY(e.win.w.insertObjectFrom(objects::Source::Page, page, QStringLiteral("1")));
        const auto old = e.win.w.carriedObjects();
        QCOMPARE(int(old.size()), 1);
        // The double click on the picture (the layer's activation, which a double click gives).
        Q_EMIT e.win.w.objects()->layer()->objectActivated(old[0].id);
        QTRY_COMPARE(opened.size(), 1);
        QSignalSpy saves(ed, &jp::EmbeddedEdit::objectSaved);
        const QByteArray changed = docxOf({QStringLiteral("Words"), QStringLiteral("and more words after them")});
        {
            QFile f(opened.first());
            QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Truncate));
            f.write(changed);
        }
        const int steps = e.win.w.undoStack()->count();
        QTRY_VERIFY((ed->pollNow(), saves.count() >= 1));
        QTRY_COMPARE(e.win.w.undoStack()->count(), steps + 1);
        const auto now = e.win.w.carriedObjects();
        QCOMPARE(int(now.size()), 1);
        QCOMPARE(now[0].carried.object.partBytes, changed);
        QVERIFY(now[0].carried.object.previewPng != old[0].carried.object.previewPng);
        QCOMPARE(now[0].carried.token, old[0].carried.token);
        // One step undoes it.
        e.win.w.undoStack()->undo();
        QTRY_COMPARE(int(e.win.w.carriedObjects().size()), 1);
        QCOMPARE(e.win.w.carriedObjects()[0].carried.object.partBytes, old[0].carried.object.partBytes);
    }

    void aMissingProgramIsSaidInTheStatusBar()
    {
        QTemporaryDir dir, temp;
        const QString page = savedFile(dir, QStringLiteral("page.docx"), docxOf({QStringLiteral("Words")}));
        EditWindow e;
        QVERIFY(ready(e));
        e.win.w.objectEditor()->setTempRoot(temp.path());
        e.win.w.objectEditor()->setOpener([](const QString &, ole::Kind) { return false; });
        QVERIFY(e.win.w.insertObjectFrom(objects::Source::Page, page, QStringLiteral("1")));
        const auto found = e.win.w.carriedObjects();
        QVERIFY(!e.win.w.editCarriedObject(found[0].sheet, found[0].id));
        QVERIFY(e.win.w.statusText().contains(QStringLiteral("can't be opened")));
        QCOMPARE(e.win.w.objectEditor()->openCount(), 0);
    }

    void pasteSpecialAsObjectPutsAnObjectIn()
    {
        QTemporaryDir dir;
        const QString page = savedFile(dir, QStringLiteral("page.docx"), docxOf({QStringLiteral("Words")}));
        const auto object = objects::pageObjectFromFile(page, 1);
        QVERIFY(object);
        EditWindow e;
        QVERIFY(ready(e));
        auto *mime = new QMimeData;
        mime->setText(QStringLiteral("plain words"));
        QApplication::clipboard()->setMimeData(objects::withObject(mime, [object] { return object; }));
        QTRY_VERIFY(e.win.w.act(QStringLiteral("edit.pasteObject"))->isEnabled());
        e.win.w.act(QStringLiteral("edit.pasteObject"))->trigger();
        const auto found = e.win.w.carriedObjects();
        QCOMPARE(int(found.size()), 1);
        QCOMPARE(found[0].carried.object.partBytes, object->partBytes);
    }

    void copyingCellsOffersARangeObject()
    {
        EditWindow e;
        QVERIFY(e.open(editBook()));
        QVERIFY(e.settle());
        e.select(QStringLiteral("A1"));
        QApplication::clipboard()->clear();
        e.win.w.act(QStringLiteral("edit.copy"))->trigger();
        QTRY_VERIFY(objects::hasObject(QApplication::clipboard()->mimeData()));
        const auto made = objects::objectOf(QApplication::clipboard()->mimeData());
        QVERIFY(made);
        QCOMPARE(made->kind(), ole::Kind::Sheet);
        QCOMPARE(objects::kindWords(*made), QStringLiteral("Spreadsheet range"));
        QVERIFY(QImage::fromData(made->previewPng, "PNG").width() > 0);
    }

    void copyingAChartOffersAChartObject()
    {
        EditWindow e;
        QVERIFY(e.open(sheetOf({"A1=1", "A2=2", "A3=3", "B1=4", "B2=5", "B3=6"})));
        QVERIFY(e.settle());
        QTRY_VERIFY(e.win.w.objects()->shownSheet() == 0);
        e.select(QStringLiteral("A1"));
        QVERIFY(e.win.w.insertChart(jp::office::sheet::ChartKind::Column));
        QApplication::clipboard()->clear();
        e.win.w.act(QStringLiteral("edit.copy"))->trigger();
        QTRY_VERIFY(objects::hasObject(QApplication::clipboard()->mimeData()));
        const auto made = objects::objectOf(QApplication::clipboard()->mimeData());
        QVERIFY(made);
        QCOMPARE(objects::kindWords(*made), QStringLiteral("Chart"));
        QVERIFY(QImage::fromData(made->previewPng, "PNG").width() > 0);
    }

    void theListShowsWordsAndConvertLeavesAPictureWithNoPart()
    {
        QTemporaryDir dir;
        const QString page = savedFile(dir, QStringLiteral("page.docx"), docxOf({QStringLiteral("Words")}));
        EditWindow e;
        QVERIFY(ready(e));
        QVERIFY(e.win.w.insertObjectFrom(objects::Source::Page, page, QStringLiteral("1")));
        QVERIFY(e.win.w.insertObjectFrom(objects::Source::Page, page, QStringLiteral("1")));
        QCOMPARE(int(e.win.w.carriedObjects().size()), 2);
        withDialog([&] { e.win.w.showObjects(); }, QStringLiteral("objectListDialog"), [&](QDialog *d) {
            auto *list = qobject_cast<objects::ObjectListDialog *>(d);
            QVERIFY(list);
            shotOf(d, QStringLiteral("crunch-objects-list"));
            QCOMPARE(list->rowCount(), 2);
            for (int i = 0; i < 2; ++i) {
                QCOMPARE(list->textAt(i, 1), QStringLiteral("Document page"));
                QVERIFY(!list->textAt(i, 2).isEmpty());
            }
            list->select(0);
            d->findChild<QPushButton *>(QStringLiteral("objectConvert"))->click();
            QCOMPARE(list->rowCount(), 1);
        });
        // One is a picture with no package now; the other is still an object.
        QCOMPARE(int(e.win.w.carriedObjects().size()), 1);
        QCOMPARE(int(e.win.w.objects()->describe(0).size()), 2);
        withDialog([&] { e.win.w.showObjects(); }, QStringLiteral("objectListDialog"), [&](QDialog *d) {
            d->findChild<QPushButton *>(QStringLiteral("objectDelete"))->click();
            QCOMPARE(qobject_cast<objects::ObjectListDialog *>(d)->rowCount(), 0);
        });
        QCOMPARE(int(e.win.w.carriedObjects().size()), 0);
    }

    void screenshotsOfTheInsertMenuAndTheList()
    {
        const QString dir = qEnvironmentVariable("JO_SHOT_DIR");
        if (dir.isEmpty()) QSKIP("JO_SHOT_DIR is not set");
        jp::applyUiTheme(1);
        QTemporaryDir files;
        QStringList lines;
        for (int i = 0; i < 30; ++i) lines << QStringLiteral("Line %1 of the page that is shown as the object").arg(i + 1);
        const QString page = savedFile(files, QStringLiteral("page.docx"), docxOf(lines));
        EditWindow e;
        QVERIFY(ready(e));
        QVERIFY(e.win.w.insertObjectFrom(objects::Source::Page, page, QStringLiteral("1")));
        // (the picture is drawn a moment after it is put in: wait until the window shows some ink inside the object's frame)
        QTRY_VERIFY([&] {
            const QImage shown = e.win.w.grab().toImage().convertToFormat(QImage::Format_RGB32);
            int dark = 0;
            for (int y = 230; y < 500; ++y)
                for (int x = 380; x < 620; ++x) dark += qGray(shown.pixel(x, y)) < 190;
            return dark > 100;
        }());
        QMenu menu;
        menu.addAction(e.win.w.act(QStringLiteral("insert.object.page")));
        menu.show();
        QVERIFY(menu.grab().save(dir + QStringLiteral("/crunch-insert-object-menu.png")));
        QVERIFY(e.win.w.grab().save(dir + QStringLiteral("/crunch-window-with-object.png")));
    }
};

JP_TEST_CLASS(CrunchObjectTests)
#include "tst_crunch_objects.moc"
