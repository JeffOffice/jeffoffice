#pragma once
// What every app's About page has, checked the same way in each app's tests: the page is the shared one, Check for
// Updates asks the updater (a fake here, never the update server) only when pressed, the automatic check and the
// statistics boxes show the settings as they are and change nothing by being shown, and the facts that are the app's own.
#include "settingguard.h"
#include "app/aboutpage.h"
#include "app/filescreen.h"
#include "app/settings.h"
#include "app/telemetry.h"
#include "core/appinfo.h"
#include "core/version.h"

#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
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
    int checks = 0;
    QWidget *checked = nullptr;
    setUpdateCheckForTests([&](QWidget *window) { ++checks; checked = window; });
    const auto restore = qScopeGuard([] { setUpdateCheckForTests({}); });
    SettingGuard autoCheck(QStringLiteral("updates/auto"), true, true);
    // The page shows what the app is.
    const QString name = appInfo().name;
    QStringList labels;
    for (QLabel *l : page->findChildren<QLabel *>()) labels << l->text();
    const QString all = labels.join(QLatin1Char('\n'));
    QVERIFY2(labels.contains(name), qPrintable(name));
    QVERIFY2(all.contains(QStringLiteral("Version %1").arg(versionText())), "version");
    QVERIFY2(all.contains(want.tagline), qPrintable(want.tagline));
    QVERIFY2(all.contains(QStringLiteral("%1 is free software under the").arg(name)), "license text names the app");
    QVERIFY2(all.contains(appInfo().repository), "links go to the app's repository");
    auto *tp = page->findChild<QLabel *>(QStringLiteral("thirdParty"));
    QVERIFY(tp);
    if (!want.thirdParty.isEmpty()) QVERIFY2(tp->text().contains(want.thirdParty), qPrintable(want.thirdParty));
    if (!want.notThirdParty.isEmpty()) QVERIFY2(!tp->text().contains(want.notThirdParty), qPrintable(want.notThirdParty));
    // Check for Updates: there, and nothing is asked until it is pressed.
    QPushButton *button = nullptr;
    for (QPushButton *b : page->findChildren<QPushButton *>())
        if (b->text() == QStringLiteral("Check for Updates")) button = b;
    QVERIFY2(button, "Check for Updates");
    QCOMPARE(checks, 0);
    button->click();
    QCOMPARE(checks, 1);
    QVERIFY(checked);
    // The boxes show the settings and showing them changes nothing.
    QCheckBox *autoBox = nullptr, *stats = nullptr;
    for (QCheckBox *c : page->findChildren<QCheckBox *>()) {
        if (c->text() == QStringLiteral("Check for updates automatically")) autoBox = c;
        if (c->text() == QStringLiteral("Send anonymous usage statistics")) stats = c;
    }
    QVERIFY2(autoBox, "Check for updates automatically");
    QVERIFY2(stats, "Send anonymous usage statistics");
    QCOMPARE(autoBox->isChecked(), true);
    QCOMPARE(stats->isChecked(), telemetry::enabled());
    QVERIFY2(stats->toolTip().contains(name), "the statistics tip names this app");
    QVERIFY2(stats->toolTip().contains(QStringLiteral("how often each command is used")), "the statistics tip says what is sent");
    autoBox->setChecked(false);
    QCOMPARE(Settings::get().value(QStringLiteral("updates/auto"), true).toBool(), false);
    QCOMPARE(checks, 1);   // (turning the automatic check off asks nothing either)
}

} // namespace jptest
