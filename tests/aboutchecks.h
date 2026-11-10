#pragma once
// What every app's About page has, checked the same way in each app's tests: the page is the shared one, Check for
// Updates asks the updater (a fake here, never the update server) only when pressed, the automatic check and the
// statistics boxes show the settings as they are and change nothing by being shown, and the facts that are the app's own.
#include "settingguard.h"
#include "app/aboutpage.h"
#include "app/filescreen.h"
#include "app/netspy.h"
#include "app/updater.h"
#include "app/settings.h"
#include "app/telemetry.h"
#include "core/appinfo.h"
#include "core/version.h"

#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QNetworkRequest>
#include <QScopeGuard>
#include <QTest>

namespace jptest {

struct AboutExpect {
    QString tagline;            // a piece of the app's one-line description
    QString thirdParty;         // a project only this app bundles ("" for none)
    QString notThirdParty;      // one only other apps bundle ("" for none)
};

// `screen` shows the About page.
inline void checkAboutPage(jp::FileScreen *screen, const AboutExpect &want)
{
    using namespace jp;
    QVERIFY(screen);
    QCOMPARE(screen->currentPage(), QStringLiteral("about"));
    QWidget *page = screen->findChild<QWidget *>(QStringLiteral("aboutPage"));
    QVERIFY2(page, "the page is the shared About page (object name aboutPage)");
    // The real updater and statistics code is under a spy: nothing goes out, and every request is counted.
    QList<QUrl> requests;
    setNetworkSpy([&](const QNetworkRequest &r) { requests << r.url(); });
    const auto restore = qScopeGuard([] { setNetworkSpy({}); });
    SettingGuard autoCheck(QStringLiteral("updates/auto"), true, true);
    const bool statsWere = telemetry::enabled();
    const auto restoreStats = qScopeGuard([statsWere] { telemetry::setEnabled(statsWere); });
    const int checksBefore = Updater::checkCalls();
    // The page shows what the app is: its name, version, and license text, and none of the other apps' names.
    const QString name = appInfo().name;
    QStringList labels;
    for (QLabel *l : page->findChildren<QLabel *>()) labels << l->text();
    const QString all = labels.join(QLatin1Char('\n'));
    QVERIFY2(labels.contains(name), qPrintable(name));
    QVERIFY2(all.contains(QStringLiteral("Version %1 ").arg(versionText())), "version");
    QVERIFY2(all.contains(want.tagline), qPrintable(want.tagline));
    QVERIFY2(all.contains(QStringLiteral("%1 is free software under the").arg(name)), "license text names the app");
    QVERIFY2(all.contains(QStringLiteral("%1 is an independent open-source project").arg(name)), "notice names the app");
    for (const QString &other : {QStringLiteral("JeffPub"), QStringLiteral("JeffDeck"), QStringLiteral("JeffWrite"), QStringLiteral("JeffCrunch")})
        if (other != name) QVERIFY2(!all.contains(other + QStringLiteral(" is ")), qPrintable(other));
    auto *tp = page->findChild<QLabel *>(QStringLiteral("thirdParty"));
    QVERIFY(tp);
    if (!want.thirdParty.isEmpty()) QVERIFY2(tp->text().contains(want.thirdParty), qPrintable(want.thirdParty));
    if (!want.notThirdParty.isEmpty()) QVERIFY2(!tp->text().contains(want.notThirdParty), qPrintable(want.notThirdParty));
    // Building and showing the page asks nothing of the update server or the statistics collector.
    QCOMPARE(Updater::checkCalls(), checksBefore);
    QVERIFY2(requests.isEmpty(), qPrintable(requests.value(0).toString()));
    QPushButton *button = nullptr;
    for (QPushButton *b : page->findChildren<QPushButton *>())
        if (b->text() == QStringLiteral("Check for Updates")) button = b;
    QVERIFY2(button, "Check for Updates");
    // Pressed, the real button makes exactly one check, and it goes to the releases list (a copy a system updates asks nothing).
    if (!Updater::updatedBySystem().isEmpty()) QSKIP("this copy is updated by the system, so Check for Updates asks nothing");
    button->click();
    QCOMPARE(Updater::checkCalls(), checksBefore + 1);
    QCOMPARE(requests.size(), 1);
    QVERIFY2(requests.first().host().contains(QStringLiteral("github")), qPrintable(requests.first().toString()));
    // The boxes show the settings, follow a change made elsewhere when the page is shown again, and showing them changes nothing.
    QCheckBox *autoBox = nullptr, *stats = nullptr;
    for (QCheckBox *c : page->findChildren<QCheckBox *>()) {
        if (c->text() == QStringLiteral("Check for updates automatically")) autoBox = c;
        if (c->text() == QStringLiteral("Send anonymous usage statistics")) stats = c;
    }
    QVERIFY2(autoBox, "Check for updates automatically");
    QVERIFY2(stats, "Send anonymous usage statistics");
    QVERIFY2(stats->toolTip().contains(name), "the statistics tip names this app");
    QVERIFY2(stats->toolTip().contains(QStringLiteral("how often each command is used")), "the statistics tip says what is sent");
    auto shownAgain = [&] {
        page->hide();
        page->show();
    };
    QCOMPARE(autoBox->isChecked(), true);
    Settings::get().setValue(QStringLiteral("updates/auto"), false);
    shownAgain();
    QVERIFY2(!autoBox->isChecked(), "the automatic check follows a change made in another app");
    Settings::get().setValue(QStringLiteral("updates/auto"), true);
    shownAgain();
    QVERIFY(autoBox->isChecked());
    telemetry::setEnabled(false);
    shownAgain();
    QVERIFY2(!stats->isChecked(), "the statistics box follows the setting (off)");
    telemetry::setEnabled(true);
    shownAgain();
    QVERIFY2(stats->isChecked(), "the statistics box follows the setting (on)");
    QCOMPARE(telemetry::enabled(), true);   // (showing the page changed nothing)
    // and the person's own change is kept
    autoBox->setChecked(false);
    QCOMPARE(Settings::get().value(QStringLiteral("updates/auto"), true).toBool(), false);
    stats->setChecked(false);
    QCOMPARE(telemetry::enabled(), false);
    QCOMPARE(Updater::checkCalls(), checksBefore + 1);
    QCOMPARE(requests.size(), 1);   // (nothing else was asked of the network)
}

} // namespace jptest
