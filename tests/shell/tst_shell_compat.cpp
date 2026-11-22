// Check Compatibility, the shared box (libs/shell/app/compatdialog.h): the choices a file holds are listed for every other kind, a list is bounded, the picker shows the kind picked, an
// answer that comes late for a kind no longer picked (or to a box that is gone) is dropped, and a window's stand-in for the box gets the first kind's report.
#include "app/compatdialog.h"
#include "../testrunner.h"

#include <QComboBox>
#include <QLabel>
#include <QPointer>
#include <QtTest>

using namespace jp;

namespace {

CompatFormat plain(const QString &format, const QStringList &found)
{
    return {format + QStringLiteral(" file"), format, QStringLiteral(".own"), QStringLiteral("thing"), [found](const std::function<void(const QStringList &)> &done) { done(found); }};
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

    void aListAndEachLineAreBounded()
    {
        QStringList many;
        for (int i = 0; i < kCompatMaxLines * 5; ++i) many << QStringLiteral("part %1").arg(i);
        many[0] = QString(kCompatMaxLineChars * 10, QLatin1Char('x'));
        const QStringList lines = compatLines(plain(QStringLiteral(".x"), {}), CompatHost(), many);
        QCOMPARE(lines.size(), kCompatMaxLines);
        QCOMPARE(lines.first().size(), kCompatMaxLineChars + 1);
        QCOMPARE(lines.last(), QStringLiteral("part %1").arg(kCompatMaxLines - 1));
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
        std::vector<std::function<void(const QStringList &)>> waiting;
        CompatHost host;
        for (const char *name : {".a", ".b"})
            host.formats.push_back({QString::fromLatin1(name), QString::fromLatin1(name), QStringLiteral(".own"), QStringLiteral("thing"),
                                    [&waiting](const std::function<void(const QStringList &)> &done) { waiting.push_back(done); }});
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
