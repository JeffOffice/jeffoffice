// The link target picker (libs/shell/app/linktargetpicker.h): the kinds, what each says as its target, the most places a list holds, and a new document made only where nothing is.
#include "app/linktargetpicker.h"
#include "../testrunner.h"

#include <QComboBox>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QtTest>

using namespace jp;

class ShellLinkPickerTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void theKindsAreInOrderAndEachSaysItsTarget()
    {
        LinkTargetPicker p;
        auto *kinds = p.findChild<QComboBox *>(QStringLiteral("linkKind"));
        QVERIFY(kinds);
        QCOMPARE(kinds->count(), 4);   // web, email, place, file
        p.enableAppDocument(QStringLiteral("Another"), QStringLiteral("*.x"));
        p.enableNewDocument(QStringLiteral("*.x"), {QStringLiteral("x")}, [](const QString &, QString *) { return true; });
        QCOMPARE(kinds->count(), 6);
        QCOMPARE(p.kind(), LinkTargetPicker::Web);
        p.setAddress(QStringLiteral("  example.com "));
        QCOMPARE(p.target(), QStringLiteral("example.com"));   // (the app makes it an address)
        p.setKind(LinkTargetPicker::Email);
        p.setAddress(QStringLiteral("a@example.com"));
        QCOMPARE(p.target(), QStringLiteral("mailto:a@example.com"));
        p.setAddress(QStringLiteral("MAILTO:b@example.com"));
        QCOMPARE(p.target(), QStringLiteral("mailto:b@example.com"));
        p.setPlaces({{QStringLiteral("One"), QStringLiteral("#1")}, {QStringLiteral("Two"), QStringLiteral("#2")}});
        p.setKind(LinkTargetPicker::Place);
        QCOMPARE(p.target(), QStringLiteral("#1"));
        QVERIFY(p.setPlaceValue(QStringLiteral("#2")));
        QCOMPARE(p.target(), QStringLiteral("#2"));
        QVERIFY(!p.setPlaceValue(QStringLiteral("#3")));
    }

    void aListHoldsAtMostTheMostPlacesAndSaysItWasCut()
    {
        LinkTargetPicker p;
        QVector<LinkPlace> many;
        for (int i = 0; i < kMostLinkPlaces + 1000; ++i) many.push_back({QString::number(i), QString::number(i)});
        p.setPlaces(many);
        QCOMPARE(p.placeCount(), kMostLinkPlaces);
        QVERIFY(p.truncated());
        many.resize(kMostLinkPlaces);
        p.setPlaces(many);
        QCOMPARE(p.placeCount(), kMostLinkPlaces);
        QVERIFY(!p.truncated());
        // The place a link has now is first and chosen even when it is not among the first places, and the list is still the most.
        p.setPlaces(many + QVector<LinkPlace>{{QStringLiteral("late"), QStringLiteral("late")}}, {QStringLiteral("late"), QStringLiteral("late")});
        QCOMPARE(p.placeCount(), kMostLinkPlaces);
        QCOMPARE(p.placeValue(), QStringLiteral("late"));
        QVERIFY(p.truncated());
    }

    void aNewDocumentIsMadeOnlyWhereNothingIsAndCanBeTakenBack()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        int made = 0;
        LinkTargetPicker p;
        p.enableNewDocument(QStringLiteral("*.doc"), {QStringLiteral("doc")}, [&](const QString &path, QString *) {
            ++made;
            QFile f(path);
            return f.open(QIODevice::WriteOnly) && f.write("empty") == 5;
        });
        p.setKind(LinkTargetPicker::NewDocument);
        QString why;
        p.setAddress(QStringLiteral("relative.doc"));
        QVERIFY(!p.prepare(&why));   // (a whole path is needed)
        QVERIFY(!why.isEmpty());
        QCOMPARE(made, 0);
        p.setAddress(dir.filePath(QStringLiteral("no-such-folder/a.doc")));
        QVERIFY(!p.prepare(&why));
        QCOMPARE(made, 0);
        const QString path = dir.filePath(QStringLiteral("a"));   // (the kind's ending is added)
        p.setAddress(path);
        QVERIFY2(p.prepare(&why), qPrintable(why));
        QCOMPARE(made, 1);
        QCOMPARE(p.target(), path + QStringLiteral(".doc"));
        QVERIFY(QFile::exists(path + QStringLiteral(".doc")));
        QVERIFY(p.prepare(&why));   // (asked again for the same path: not made twice)
        QCOMPARE(made, 1);
        p.discardCreated();
        QVERIFY(!QFile::exists(path + QStringLiteral(".doc")));
        QVERIFY(p.target().isEmpty());
        // A file that is there is never written over.
        QFile there(dir.filePath(QStringLiteral("b.doc")));
        QVERIFY(there.open(QIODevice::WriteOnly));
        there.write("mine");
        there.close();
        p.setAddress(there.fileName());
        QVERIFY(!p.prepare(&why));
        QCOMPARE(made, 1);
        QVERIFY(there.open(QIODevice::ReadOnly));
        QCOMPARE(there.readAll(), QByteArray("mine"));
        // The other kinds make nothing.
        p.setKind(LinkTargetPicker::Web);
        QVERIFY(p.prepare());
        QCOMPARE(made, 1);
    }
};

JP_TEST_CLASS(ShellLinkPickerTests)
#include "tst_shell_linkpicker.moc"
