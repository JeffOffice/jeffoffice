// Check Compatibility, the shared box (libs/shell/app/compatdialog.h): the choices a file holds are listed for every other kind, a list is bounded, the picker shows the kind picked, an
// answer that comes late for a kind no longer picked (or to a box that is gone) is dropped, and a window's stand-in for the box gets the first kind's report.
#include "app/compatdialog.h"
#include "core/lossnotes.h"
#include "../testrunner.h"

#include <QComboBox>
#include <QLabel>
#include <QPointer>
#include <QtTest>

using namespace jp;

namespace {

CompatFormat plain(const QString &format, const QStringList &found)
{
    return {format + QStringLiteral(" file"), format, QStringLiteral(".own"), QStringLiteral("thing"), [found](const CompatDone &done) { done(found); }};
}

} // namespace

class ShellCompatTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void theFilesChoicesAreListedOnlyWhenOn()
    {
        QVERIFY(compatFileChoices(QStringLiteral("thing"), false, false).isEmpty());
        QCOMPARE(compatFileChoices(QStringLiteral("thing"), true, false), QStringList{QStringLiteral("The mark that the thing is final.")});
        QCOMPARE(compatFileChoices(QStringLiteral("thing"), false, true), QStringList{QStringLiteral("The choice to always open the thing for reading only.")});
        CompatHost host;
        bool final = false, readOnly = false;
        host.markedFinal = [&] { return final; };
        host.alwaysReadOnly = [&] { return readOnly; };
        const CompatFormat f = plain(QStringLiteral(".x"), {QStringLiteral("Glows")});
        QCOMPARE(compatLines(f, host, {QStringLiteral("Glows")}), QStringList{QStringLiteral("Glows")});
        final = readOnly = true;
        QCOMPARE(compatLines(f, host, {QStringLiteral("Glows")}).size(), 3);
        // (an app that gives no functions has no choices to list)
        QCOMPARE(compatLines(f, CompatHost(), {QStringLiteral("Glows")}).size(), 1);
        QVERIFY(compatReport(f, host, {}).contains(QLatin1String("always open")));
    }

    // A list is bounded and says how many it left off ("And N more."); the file's own choices always stay inside the bound; each line is cut too.
    void aListAndEachLineAreBoundedAndTheRestIsCounted()
    {
        CompatHost host;
        host.markedFinal = [] { return true; };
        host.alwaysReadOnly = [] { return true; };
        const CompatFormat f = plain(QStringLiteral(".x"), {});
        const int room = kCompatMaxLines - 2;   // (the two choices keep their lines)
        QStringList many;
        for (int i = 0; i < kCompatMaxLines * 5; ++i) many << QStringLiteral("part %1").arg(i);
        many[0] = QString(kCompatMaxLineChars * 10, QLatin1Char('x'));
        const QStringList lines = compatLines(f, host, CompatAnswer(many));
        QCOMPARE(lines.size(), kCompatMaxLines);
        QCOMPARE(lines.first().size(), kCompatMaxLineChars + 1);
        QCOMPARE(lines[room - 2], QStringLiteral("part %1").arg(room - 2));
        QCOMPARE(lines[room - 1], QStringLiteral("And %1 more.").arg(many.size() - (room - 1)));
        QVERIFY(lines[room].contains(QLatin1String("is final")) && lines[room + 1].contains(QLatin1String("always open")));
        // exactly what fits is all shown; one more than fits is cut to make room for the line that counts
        QCOMPARE(compatLines(f, host, CompatAnswer(many.mid(0, room))).size(), kCompatMaxLines);
        QVERIFY(!compatLines(f, host, CompatAnswer(many.mid(0, room))).join(QLatin1Char('|')).contains(QLatin1String("more.")));
        QVERIFY(compatLines(f, host, CompatAnswer(many.mid(0, room + 1))).contains(QStringLiteral("And 2 more.")));
        // what the app's own writer already left off is counted in too, even when the list is short
        CompatAnswer short3(QStringList{QStringLiteral("a"), QStringLiteral("b"), QStringLiteral("c")});
        short3.more = 7;
        QCOMPARE(compatLines(f, CompatHost(), short3), (QStringList{QStringLiteral("a"), QStringLiteral("b"), QStringLiteral("c"), QStringLiteral("And 7 more.")}));
        short3.more = 1;
        QCOMPARE(compatLines(f, CompatHost(), short3).last(), QStringLiteral("And 1 more."));
        // the one box shows it
        QVERIFY(compatReport(f, host, CompatAnswer(many)).contains(QStringLiteral("And %1 more.").arg(many.size() - (room - 1))));
    }

    // The writers bound the lists they build: the first kMaxLossNotes, the rest counted.
    void theWritersHelpersKeepTheFirstAndCountTheRest()
    {
        QStringList notes;
        qint64 dropped = 0;
        for (int i = 0; i < kMaxLossNotes * 3; ++i) addLossNote(&notes, QString::number(i), &dropped);
        QCOMPARE(notes.size(), kMaxLossNotes);
        QCOMPARE(dropped, qint64(kMaxLossNotes * 2));
        QCOMPARE(notes.last(), QString::number(kMaxLossNotes - 1));
        QStringList long_;
        qint64 cut = 5;
        for (int i = 0; i < kMaxLossNotes + 10; ++i) long_ << QString::number(i);
        boundLossNotes(&long_, &cut);
        QCOMPARE(long_.size(), kMaxLossNotes);
        QCOMPARE(cut, qint64(15));
        QCOMPARE(moreLossNotes(15), QStringLiteral("And 15 more."));
    }

    void thePickerShowsTheKindPickedAndAWorkingKindSaysSo()
    {
        CompatHost host;
        host.formats << plain(QStringLiteral(".a"), {QStringLiteral("Sections")}) << plain(QStringLiteral(".b"), {});
        CompatibilityDialog dialog(host, 0, nullptr);
        QCOMPARE(dialog.formatCount(), 2);
        QVERIFY(dialog.reportText().contains(QLatin1String("Sections")));
        QVERIFY(dialog.reportText().contains(QLatin1String(".a")));
        dialog.choose(1);
        QVERIFY(dialog.reportText().contains(QLatin1String("Nothing in this thing")));
        QVERIFY(!dialog.working());
        CompatibilityDialog second(host, 1, nullptr);   // (opens on the kind asked for)
        QVERIFY(second.reportText().contains(QLatin1String("Nothing")));
    }

    void aLateAnswerForAnotherKindOrAGoneBoxIsDropped()
    {
        std::vector<CompatDone> waiting;
        CompatHost host;
        for (const char *name : {".a", ".b"})
            host.formats.push_back({QString::fromLatin1(name), QString::fromLatin1(name), QStringLiteral(".own"), QStringLiteral("thing"),
                                    [&waiting](const CompatDone &done) { waiting.push_back(done); }});
        {
            CompatibilityDialog dialog(host, 0, nullptr);
            QVERIFY(dialog.working());
            dialog.choose(1);
            QCOMPARE(int(waiting.size()), 2);
            waiting[0]({QStringLiteral("Old answer")});   // (for .a, which is no longer picked)
            QVERIFY(dialog.working());
            QVERIFY(!dialog.reportText().contains(QLatin1String("Old answer")));
            waiting[1]({QStringLiteral("New answer")});
            QVERIFY(!dialog.working());
            QVERIFY(dialog.reportText().contains(QLatin1String("New answer")));
            dialog.choose(0);
        }
        waiting[2]({QStringLiteral("Too late")});   // (the box is gone: nothing happens)
    }

    // A check that could not be made is a state of its own: it says so and why, in the box and in a window's stand-in, and never "nothing is lost".
    void aCheckThatCouldNotBeMadeNeverSaysNothingIsLost()
    {
        CompatHost host;
        host.formats << CompatFormat{QStringLiteral("A file"), QStringLiteral(".a"), QStringLiteral(".own"), QStringLiteral("thing"), [](const CompatDone &done) { done(CompatAnswer::failed(QStringLiteral("It is still being read."))); }};
        host.formats << CompatFormat{QStringLiteral("B file"), QStringLiteral(".b"), QStringLiteral(".own"), QStringLiteral("thing"), [](const CompatDone &done) { done(CompatAnswer::failed(QString())); }};
        CompatibilityDialog dialog(host, 0, nullptr);
        QVERIFY2(dialog.reportText().startsWith(QLatin1String("Couldn't check")), qPrintable(dialog.reportText()));
        QVERIFY(dialog.reportText().contains(QLatin1String("It is still being read.")));
        QVERIFY(!dialog.reportText().contains(QLatin1String("Nothing")) && !dialog.reportText().contains(QLatin1String("is lost")));
        dialog.choose(1);   // (no reason given: a plain one is used)
        QVERIFY(dialog.reportText().startsWith(QLatin1String("Couldn't check")) && dialog.reportText().contains(QLatin1String("Something went wrong.")));
        QVERIFY(!dialog.working());
        QStringList told;
        bool warned = false;
        showCompatibility(nullptr, host, 0, [&](const QString &, const QString &text, bool warn) { told << text; warned = warn; });
        QCOMPARE(told.size(), 1);
        QVERIFY(told[0].startsWith(QLatin1String("Couldn't check")) && warned);
        // (even the file's own choices do not make a check that was not made look like a list)
        QVERIFY(!compatReport(host.formats[0], host, CompatAnswer::failed(QStringLiteral("x"))).contains(QLatin1String("can't hold")));
    }

    void aWindowsStandInGetsTheFirstKindsReport()
    {
        CompatHost host;
        host.formats << plain(QStringLiteral(".a"), {QStringLiteral("Sections")}) << plain(QStringLiteral(".b"), {QStringLiteral("Glows")});
        QStringList told;
        auto say = [&](const QString &title, const QString &text, bool warn) { told << title + QLatin1Char('|') + text + QLatin1Char('|') + (warn ? QStringLiteral("warn") : QStringLiteral("ok")); };
        showCompatibility(nullptr, host, 0, say);
        showCompatibility(nullptr, host, 1, say);
        QCOMPARE(told.size(), 2);
        QVERIFY(told[0].startsWith(QLatin1String("Check Compatibility|")) && told[0].contains(QLatin1String("Sections")) && told[0].endsWith(QLatin1String("warn")));
        QVERIFY(told[1].contains(QLatin1String("Glows")));
    }
};

JP_TEST_CLASS(ShellCompatTests)
#include "tst_shell_compat.moc"
