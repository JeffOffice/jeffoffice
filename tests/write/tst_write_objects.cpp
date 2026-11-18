// Tests of JeffWrite's objects of other apps (Insert > Object, Paste Special as an object, double-click to edit, the list). Every file is made here in code; no other program is started
// (the edit's opener is a stand-in that notes the path).
#include "../isolatesettings.h"
#include "../withdialog.h"
#include "app/objects.h"
#include "app/objectsdialog.h"
#include "app/pageview.h"
#include "app/theme.h"
#include "app/writeinfo.h"
#include "app/writewindow.h"
#include "core/appinfo.h"
#include "core/fonts.h"
#include "core/pictureops.h"
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
#include <QtTest>

using namespace jp;
using namespace jp::write;
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

QString saved(const QTemporaryDir &dir, const QString &name, const QByteArray &bytes)
{
    const QString path = dir.filePath(name);
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly)) return QString();
    f.write(bytes);
    return path;
}

std::unique_ptr<WriteDoc> openDocx(const QByteArray &zip)
{
    QString err;
    return WriteDoc::fromDocx(zip, &err);
}

struct Win {
    WriteWindow w;
    Win()
    {
        w.resize(1100, 800);
        w.show();
        w.setDocument(openDocx(docxOf({QStringLiteral("Hello world")})), QString());
        QTextCursor c(w.document()->story());
        c.setPosition(5);
        w.view()->setCursor(c);
    }
    QVector<int> objectsNow() { return w.view()->objectPositions(); }
};

int partsNamed(const QByteArray &zip, const QString &part)
{
    office::Package p;
    p.read(zip);
    int n = 0;
    for (const QString &name : p.partNames())
        if (name.contains(part)) ++n;
    return n;
}

} // namespace

class ObjectWindowTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        Q_INIT_RESOURCE(resources);
        Q_INIT_RESOURCE(write);
        setAppInfo(writeAppInfo());
        initCore();
    }

    void aRangeFromAFileIsPutInWithItsPicture()
    {
        QTemporaryDir dir;
        const QString book = saved(dir, QStringLiteral("book.xlsx"), objects::rangePackage(gridOf({"Name", "Total", "North", "12", "South", "30"}, 2)));
        Win win;
        QVERIFY(win.w.insertObjectFrom(objects::Source::Range, book, QStringLiteral("A1:B3")));
        QCOMPARE(win.objectsNow().size(), 1);
        const auto object = objectAt(*win.w.document(), win.objectsNow().first());
        QVERIFY(object);
        QVERIFY(object->isOurs);
        QCOMPARE(object->kind(), ole::Kind::Sheet);
        QCOMPARE(objects::kindWords(*object), QStringLiteral("Spreadsheet range"));
        QVERIFY(QImage::fromData(object->previewPng, "PNG").width() > 0);
        // (A range that is not one is said, and nothing is put in.)
        QVERIFY(!win.w.insertObjectFrom(objects::Source::Range, book, QStringLiteral("not a range")));
        QVERIFY(win.w.statusBar()->currentMessage().contains(QStringLiteral("range")));
        QCOMPARE(win.objectsNow().size(), 1);
    }

    void theObjectSurvivesSaveAndReopen()
    {
        QTemporaryDir dir;
        const QString book = saved(dir, QStringLiteral("book.xlsx"), withChart(objects::rangePackage(gridOf({"a", "1", "b", "2"}, 2))));
        Win win;
        QVERIFY(win.w.insertObjectFrom(objects::Source::Chart, book, QString()));
        const auto before = objectAt(*win.w.document(), win.objectsNow().first());
        QVERIFY(before);
        QCOMPARE(objects::kindWords(*before), QStringLiteral("Chart"));
        const QByteArray file = win.w.document()->toDocx();
        QVERIFY(!file.isEmpty());
        auto again = openDocx(file);
        QVERIFY(again);
        Win other;
        other.w.setDocument(std::move(again), QString());
        QCOMPARE(other.objectsNow(), win.objectsNow());
        const auto after = objectAt(*other.w.document(), other.objectsNow().first());
        QVERIFY(after);
        QVERIFY(after->isOurs);
        QCOMPARE(after->partBytes, before->partBytes);
        QCOMPARE(after->previewPng, before->previewPng);
    }

    void doubleClickStartsTheEditAndASaveIsOneStep()
    {
        QTemporaryDir dir, temp;
        const QString book = saved(dir, QStringLiteral("book.xlsx"), objects::rangePackage(gridOf({"Name", "Total", "North", "12"}, 2)));
        Win win;
        EmbeddedEdit *ed = win.w.objectEditor();
        ed->setTempRoot(temp.path());
        ed->setPollInterval(600000);
        QStringList opened;
        ed->setOpener([&](const QString &path, ole::Kind) {
            opened << path;
            return true;
        });
        QVERIFY(win.w.insertObjectFrom(objects::Source::Range, book, QStringLiteral("A1:B2")));
        win.w.view()->refresh();
        const int at = win.objectsNow().first();
        const auto old = objectAt(*win.w.document(), at);
        QVERIFY(old);
        QRectF r;
        QTRY_VERIFY(win.w.view()->objectRect(at, &r));
        QCOMPARE(win.w.view()->objectAtPoint(r.center().toPoint()), at);
        QTest::mouseDClick(win.w.view()->viewport(), Qt::LeftButton, Qt::NoModifier, r.center().toPoint());
        QTRY_COMPARE(opened.size(), 1);
        // The other program saves: new bytes, and the picture is drawn again.
        QSignalSpy saves(ed, &EmbeddedEdit::objectSaved);
        const QByteArray changed = objects::rangePackage(gridOf({"Name", "Total", "North", "99", "West", "5"}, 2));
        {
            QFile f(opened.first());
            QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Truncate));
            f.write(changed);
        }
        QTRY_VERIFY((ed->pollNow(), saves.count() >= 1));
        const auto now = objectAt(*win.w.document(), win.objectsNow().first());
        QVERIFY(now);
        QCOMPARE(now->partBytes, changed);
        QVERIFY(now->previewPng != old->previewPng);
        // One step undoes it.
        win.w.view()->undo();
        const auto undone = objectAt(*win.w.document(), win.objectsNow().first());
        QVERIFY(undone);
        QCOMPARE(undone->partBytes, old->partBytes);
        QCOMPARE(undone->previewPng, old->previewPng);
    }

    void aMissingProgramIsSaidInTheStatusBar()
    {
        QTemporaryDir dir, temp;
        const QString book = saved(dir, QStringLiteral("book.xlsx"), objects::rangePackage(gridOf({"a", "1"}, 2)));
        Win win;
        win.w.objectEditor()->setTempRoot(temp.path());
        win.w.objectEditor()->setOpener([](const QString &, ole::Kind) { return false; });
        QVERIFY(win.w.insertObjectFrom(objects::Source::Range, book, QString()));
        QVERIFY(!win.w.editObjectAt(win.objectsNow().first()));
        QVERIFY(win.w.statusBar()->currentMessage().contains(QStringLiteral("can't be opened")));
        QCOMPARE(win.w.objectEditor()->openCount(), 0);
    }

    void pasteSpecialAsObjectPutsAnObjectIn()
    {
        const auto object = objects::rangeObject(gridOf({"x", "1", "y", "2"}, 2));
        QVERIFY(object);
        Win win;
        auto *mime = new QMimeData;
        mime->setText(QStringLiteral("plain words"));
        QApplication::clipboard()->setMimeData(objects::withObject(mime, [object] { return object; }));
        QTRY_VERIFY(win.w.act(QStringLiteral("edit.pasteObject"))->isEnabled());
        // Plain paste stays as it was: the words.
        win.w.act(QStringLiteral("edit.paste"))->trigger();
        QVERIFY(win.w.document()->story()->toPlainText().contains(QStringLiteral("plain words")));
        QCOMPARE(win.objectsNow().size(), 0);
        win.w.act(QStringLiteral("edit.pasteObject"))->trigger();
        QCOMPARE(win.objectsNow().size(), 1);
        const auto put = objectAt(*win.w.document(), win.objectsNow().first());
        QVERIFY(put && put->isOurs);
        QCOMPARE(put->partBytes, object->partBytes);
    }

    void copyingInWriteOffersAPageObject()
    {
        Win win;
        QTextCursor c(win.w.document()->story());
        c.setPosition(0);
        c.setPosition(5, QTextCursor::KeepAnchor);
        win.w.view()->setCursor(c);
        QApplication::clipboard()->clear();
        win.w.view()->copy();
        QCOMPARE(QApplication::clipboard()->text(), QStringLiteral("Hello"));
        const auto made = objects::objectOf(QApplication::clipboard()->mimeData());
        QVERIFY(made);
        QCOMPARE(made->kind(), ole::Kind::Page);
        QCOMPARE(objects::kindWords(*made), QStringLiteral("Document page"));
        QVERIFY(QImage::fromData(made->previewPng, "PNG").width() > 0);
    }

    void theListShowsWordsAndConvertLeavesAPictureWithNoPart()
    {
        QTemporaryDir dir;
        const QString book = saved(dir, QStringLiteral("book.xlsx"), objects::rangePackage(gridOf({"a", "1", "b", "2"}, 2)));
        const QString chart = saved(dir, QStringLiteral("chart.xlsx"), withChart(objects::rangePackage(gridOf({"a", "1"}, 2))));
        Win win;
        QVERIFY(win.w.insertObjectFrom(objects::Source::Range, book, QString()));
        QVERIFY(win.w.insertObjectFrom(objects::Source::Chart, chart, QString()));
        QCOMPARE(win.objectsNow().size(), 2);
        QVERIFY(partsNamed(win.w.document()->toDocx(), QStringLiteral("embeddings")) >= 2);
        withDialog([&] { win.w.showObjects(); }, QStringLiteral("objectListDialog"), [&](QDialog *d) {
            auto *list = qobject_cast<objects::ObjectListDialog *>(d);
            QVERIFY(list);
            shotOf(d, QStringLiteral("write-objects-list"));
            QCOMPARE(list->rowCount(), 2);
            QSet<QString> kinds{list->textAt(0, 1), list->textAt(1, 1)};
            QCOMPARE(kinds, (QSet<QString>{QStringLiteral("Spreadsheet range"), QStringLiteral("Chart")}));
            for (int i = 0; i < 2; ++i) {
                QVERIFY(!list->textAt(i, 2).isEmpty());
                QVERIFY(!list->textAt(i, 1).contains(QStringLiteral(".")));   // (no program ID)
            }
            list->select(0);
            d->findChild<QPushButton *>(QStringLiteral("objectConvert"))->click();
            QCOMPARE(list->rowCount(), 1);
        });
        QCOMPARE(win.objectsNow().size(), 1);
        const QByteArray file = win.w.document()->toDocx();
        QCOMPARE(partsNamed(file, QStringLiteral("embeddings")), 1);
        // One picture is where the object was (the other object is still an object).
        int pictures = 0;
        for (int i = 0; i < win.w.document()->story()->characterCount(); ++i)
            if (isPictureAt(win.w.document()->story(), i) && !objectAt(*win.w.document(), i)) ++pictures;
        QCOMPARE(pictures, 1);
        // (Delete takes the other one out, with its part.)
        withDialog([&] { win.w.showObjects(); }, QStringLiteral("objectListDialog"), [&](QDialog *d) {
            d->findChild<QPushButton *>(QStringLiteral("objectDelete"))->click();
            QCOMPARE(qobject_cast<objects::ObjectListDialog *>(d)->rowCount(), 0);
        });
        QCOMPARE(partsNamed(win.w.document()->toDocx(), QStringLiteral("embeddings")), 0);
    }

    void screenshotsOfTheInsertMenuAndTheList()
    {
        const QString dir = qEnvironmentVariable("JO_SHOT_DIR");
        if (dir.isEmpty()) QSKIP("JO_SHOT_DIR is not set");
        applyUiTheme(1);
        QTemporaryDir files;
        const QString book = saved(files, QStringLiteral("book.xlsx"), objects::rangePackage(gridOf({"Name", "Total", "North", "12", "South", "30"}, 2)));
        Win win;
        QVERIFY(win.w.insertObjectFrom(objects::Source::Range, book, QString()));
        QMenu menu;
        menu.addAction(win.w.act(QStringLiteral("insert.object.range")));
        menu.addAction(win.w.act(QStringLiteral("insert.object.chart")));
        menu.show();
        QVERIFY(menu.grab().save(dir + QStringLiteral("/write-insert-object-menu.png")));
        win.w.view()->refresh();
        QVERIFY(win.w.grab().save(dir + QStringLiteral("/write-window-with-object.png")));
    }
};

QTEST_MAIN(ObjectWindowTests)
#include "tst_write_objects.moc"
