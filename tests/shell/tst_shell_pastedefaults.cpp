// Set Default Paste, the part every app shares (libs/shell/app/pastedefaults.h): the Options section, the settings it keeps, and the stamp a copy carries so that a paste can tell whether the
// content is from its own document, another document, or another program.
#include "app/pastedefaults.h"
#include "app/settings.h"
#include "../isolatesettings.h"
#include "../testrunner.h"

#include <QComboBox>
#include <QCoreApplication>
#include <QMimeData>
#include <QtTest>

using namespace jp;

namespace {

QVector<PasteDefaultRow> rows()
{
    const QVector<PasteDefaultChoice> choices{{QStringLiteral("Keep"), 0}, {QStringLiteral("Merge"), 1}, {QStringLiteral("Text"), 2}};
    return {PasteDefaultRow{PasteFrom::SameDocument, QStringLiteral("Within"), QStringLiteral("test/pasteSame"), choices, 0},
            PasteDefaultRow{PasteFrom::OtherDocument, QStringLiteral("Between"), QStringLiteral("test/pasteOther"), choices, 1},
            PasteDefaultRow{PasteFrom::OtherProgram, QStringLiteral("Programs"), QStringLiteral("test/pasteProgram"), choices, 2}};
}

} // namespace

class ShellPasteDefaultsTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    // A box for each way of pasting, holding what is kept (or the row's own default), and apply() keeps what is chosen under the row's setting.
    void theSectionHoldsWhatIsKeptAndKeepsWhatIsChosen()
    {
        Settings::get().remove(QStringLiteral("test/pasteSame"));
        Settings::get().remove(QStringLiteral("test/pasteOther"));
        Settings::get().remove(QStringLiteral("test/pasteProgram"));
        Settings::get().setValue(QStringLiteral("test/pasteOther"), 2);
        PasteDefaultsSection section(rows());
        auto box = [&](int from) { return section.findChild<QComboBox *>(QStringLiteral("pasteDefault%1").arg(from)); };
        QVERIFY(box(0) && box(1) && box(2));
        QCOMPARE(box(0)->currentData().toInt(), 0);   // (none kept: the row's own)
        QCOMPARE(box(1)->currentData().toInt(), 2);   // (kept)
        QCOMPARE(box(2)->currentData().toInt(), 2);
        box(0)->setCurrentIndex(box(0)->findData(1));
        box(1)->setCurrentIndex(box(1)->findData(0));
        QCOMPARE(Settings::get().value(QStringLiteral("test/pasteSame"), -1).toInt(), -1);   // (nothing is kept until OK)
        section.apply();
        QCOMPARE(Settings::get().value(QStringLiteral("test/pasteSame")).toInt(), 1);
        QCOMPARE(Settings::get().value(QStringLiteral("test/pasteOther")).toInt(), 0);
        const auto all = rows();
        QCOMPARE(PasteDefaultsSection::value(all[0]), 1);
        QCOMPARE(PasteDefaultsSection::value(all[1]), 0);
        QVERIFY(!Settings::get().value(QStringLiteral("test/pasteProgram")).isValid());   // (a box nobody changed keeps nothing)
        // a kept number that is not one of the choices (a setting from another version, or a hand edit) is the row's own
        Settings::get().setValue(QStringLiteral("test/pasteSame"), 77);
        QCOMPARE(PasteDefaultsSection::value(all[0]), 0);
        QVERIFY(PasteDefaultsSection::rowFor(all, PasteFrom::OtherProgram));
        QVERIFY(!PasteDefaultsSection::rowFor(QVector<PasteDefaultRow>(), PasteFrom::OtherProgram));
    }

    // A copy says which document it is from; a paste in that document says "same", in another document of the program "other", and a clipboard with no stamp, or another program's, is another program.
    void aCopyTellsWhereItCameFrom()
    {
        const QString was = QCoreApplication::applicationName();
        const QString mine = newPasteDocumentId(), other = newPasteDocumentId();
        QVERIFY(mine != other && !mine.isEmpty());
        QMimeData stamped;
        stampPasteOrigin(&stamped, mine);
        QCOMPARE(pasteOriginOf(&stamped, mine), PasteFrom::SameDocument);
        QCOMPARE(pasteOriginOf(&stamped, other), PasteFrom::OtherDocument);
        QCOMPARE(pasteOriginOf(&stamped, QString()), PasteFrom::OtherDocument);
        QMimeData plain;
        plain.setText(QStringLiteral("words"));
        QCOMPARE(pasteOriginOf(&plain, mine), PasteFrom::OtherProgram);
        QCOMPARE(pasteOriginOf(nullptr, mine), PasteFrom::OtherProgram);
        QCoreApplication::setApplicationName(QStringLiteral("AnotherProgram"));
        QCOMPARE(pasteOriginOf(&stamped, mine), PasteFrom::OtherProgram);   // (the same document id in another program is not the same document)
        QCoreApplication::setApplicationName(was);
        QCOMPARE(pasteOriginOf(&stamped, mine), PasteFrom::SameDocument);
        QMimeData broken;
        broken.setData(QStringLiteral("application/x-jeffoffice-origin"), "no newline here");
        QCOMPARE(pasteOriginOf(&broken, mine), PasteFrom::OtherProgram);
    }

    // What a copy leaves on the clipboard is a short digest of the program's name and the document's random id: no name. A stamp that is long or not of that shape is not read, whatever it holds.
    void theStampIsShortAndAMalformedOneIsNotRead()
    {
        const QString mine = newPasteDocumentId();
        QMimeData stamped;
        stampPasteOrigin(&stamped, mine);
        const QByteArray bytes = stamped.data(QStringLiteral("application/x-jeffoffice-origin"));
        QVERIFY2(bytes.size() <= 64 && bytes.endsWith(mine.toUtf8()), bytes.constData());
        QVERIFY(!bytes.contains(QCoreApplication::applicationName().toUtf8()) || QCoreApplication::applicationName().size() < 4);
        const QString tag = QString::fromLatin1(bytes.left(bytes.indexOf('\n')));
        auto read = [&](const QByteArray &stamp) {
            QMimeData m;
            m.setData(QStringLiteral("application/x-jeffoffice-origin"), stamp);
            return pasteOriginOf(&m, mine);
        };
        QCOMPARE(read(bytes), PasteFrom::SameDocument);
        // 5 MB of the right start, and of a long second line, a third line, no tag, a tag of a wrong shape, and the end of the line left on
        QByteArray huge = (tag + QLatin1Char('\n') + mine).toUtf8();
        huge += QByteArray(5 * 1024 * 1024, 'a');
        QCOMPARE(read(huge), PasteFrom::OtherProgram);
        QCOMPARE(read(QByteArray(300, 'a')), PasteFrom::OtherProgram);
        QCOMPARE(read((tag + QLatin1Char('\n') + mine + QLatin1Char('\n') + mine).toUtf8()), PasteFrom::OtherProgram);
        QCOMPARE(read(("\n" + mine).toUtf8()), PasteFrom::OtherProgram);
        QCOMPARE(read(("zz" + tag.mid(2) + "\n" + mine).toUtf8()), PasteFrom::OtherProgram);
        QCOMPARE(read((tag + QLatin1Char('\n') + mine + QLatin1Char('\n')).toUtf8()), PasteFrom::OtherProgram);
        QCOMPARE(read((tag + QLatin1Char('\n') + QStringLiteral("not a uuid!")).toUtf8()), PasteFrom::OtherProgram);
        QCOMPARE(read(QByteArray()), PasteFrom::OtherProgram);
    }
};

JP_TEST_CLASS(ShellPasteDefaultsTests)
#include "tst_shell_pastedefaults.moc"
