// Options: the pages every app shares (General, with the suite's settings, and
// Proofing), then the app's own.

#include "app/optionsdialog.h"

#include "app/appwords.h"
#include "app/editor.h"
#include "app/i18n.h"
#include "app/settings.h"
#include "app/telemetry.h"
#include "app/theme.h"
#include "app/updater.h"

#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QTabWidget>
#include <QVBoxLayout>

namespace jp {

namespace {

struct Dlg {
    QDialog d;
    QVBoxLayout *v;
    QDialogButtonBox *bb;
    explicit Dlg(QWidget *p, const QString &title, QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Ok | QDialogButtonBox::Cancel) : d(p)
    {
        d.setWindowTitle(title);
        v = new QVBoxLayout(&d);
        bb = new QDialogButtonBox(buttons, &d);
        QObject::connect(bb, &QDialogButtonBox::accepted, &d, &QDialog::accept);
        QObject::connect(bb, &QDialogButtonBox::rejected, &d, &QDialog::reject);
    }
    bool exec()
    {
        v->addWidget(bb);
        return d.exec() == QDialog::Accepted;
    }
};

} // namespace

bool optionsDialog(QWidget *p, Editor *ed, const QVector<OptionsPage> &appPages)
{
    Settings &st = Settings::get();
    Dlg dlg(p, forThisApp(QCoreApplication::translate("Dialogs", "JeffPub Options")));
    auto *tabs = new QTabWidget(&dlg.d);
    auto *gen = new QWidget();
    auto *gf = new QFormLayout(gen);
    auto *name = new QLineEdit(st.userName(), gen);
    auto *initials = new QLineEdit(st.value("user/initials").toString(), gen);
    auto *theme = new QComboBox(gen);
    theme->addItems({QCoreApplication::translate("Dialogs", "Use system setting"), QCoreApplication::translate("Dialogs", "Light"), QCoreApplication::translate("Dialogs", "Dark")});
    const int oldTheme = st.value("ui/theme", 0).toInt();
    theme->setCurrentIndex(oldTheme);
    gf->addRow(QCoreApplication::translate("Dialogs", "User name:"), name);
    gf->addRow(QCoreApplication::translate("Dialogs", "Initials:"), initials);
    gf->addRow(QCoreApplication::translate("Dialogs", "Theme:"), theme);
    // The language of JeffPub's own text; it takes effect at the next start.
    auto *uiLang = new QComboBox(gen);
    uiLang->addItem(QCoreApplication::translate("Dialogs", "Use system language"), QString());
    uiLang->addItem(QStringLiteral("English"), QStringLiteral("en"));
    for (const QString &l : availableUiLanguages()) uiLang->addItem(uiLanguageName(l), l);
    const QString oldLang = st.value("ui/language").toString();
    uiLang->setCurrentIndex(std::max(0, uiLang->findData(oldLang)));
    gf->addRow(QCoreApplication::translate("Dialogs", "Display language:"), uiLang);
    // The choice the first start offered, which its message says is here,
    // and the update check: only in an app that sends and checks (the
    // choices are the suite's, so JeffPub's Options keep them for JeffDeck).
    const bool online = appInfo().online;
    auto *stats = new QCheckBox(QCoreApplication::translate("Dialogs", "Send anonymous usage statistics"), gen);
    stats->setObjectName(QStringLiteral("stats"));
    stats->setToolTip(forThisApp(QCoreApplication::translate("Dialogs", "Once a day, and the day it's updated: JeffPub's version, your operating system and language, and how often each command is used. "
                                                                       "Never your files, their names, or anything in them.")));
    stats->setChecked(telemetry::enabled());
    gf->addRow(stats);
    auto *updates = new QCheckBox(forThisApp(QCoreApplication::translate("Dialogs", "Check for updates when JeffPub starts")), gen);
    updates->setObjectName(QStringLiteral("updates"));
    updates->setChecked(st.value("updates/auto", true).toBool());
    gf->addRow(updates);
    // Beta versions are the test builds published ahead of each regular release.
    auto *beta = new QCheckBox(QCoreApplication::translate("Dialogs", "Get beta versions (early builds for testing)"), gen);
    beta->setObjectName(QStringLiteral("betaUpdates"));
    const QString betaAbout = QCoreApplication::translate("Dialogs", "Beta versions arrive before the regular ones and may have rough edges. "
                                                                    "Leave this off to get only the regular versions.");
    beta->setToolTip(betaAbout);
    beta->setWhatsThis(betaAbout);
    beta->setAccessibleDescription(betaAbout);
    beta->setChecked(Updater::betaChannel());
    gf->addRow(beta);
    for (QWidget *box : {static_cast<QWidget *>(stats), static_cast<QWidget *>(updates), static_cast<QWidget *>(beta)}) gf->setRowVisible(box, online);
    tabs->addTab(gen, QCoreApplication::translate("Dialogs", "General"));
    auto *proof = new QWidget();
    auto *pf = new QFormLayout(proof);
    auto *asType = new QCheckBox(QCoreApplication::translate("Dialogs", "Check spelling as you type"), proof);
    asType->setChecked(ed->view.spelling);
    auto *ignoreUpper = new QCheckBox(QCoreApplication::translate("Dialogs", "Ignore words in UPPERCASE"), proof);
    ignoreUpper->setChecked(st.value("proof/ignoreUpper", true).toBool());
    auto *ignoreNum = new QCheckBox(QCoreApplication::translate("Dialogs", "Ignore words that contain numbers"), proof);
    ignoreNum->setChecked(st.value("proof/ignoreNumbers", true).toBool());
    auto *autocorrect = new QCheckBox(QCoreApplication::translate("Dialogs", "Replace text as you type (AutoCorrect)"), proof);
    autocorrect->setChecked(st.value("proof/autocorrect", true).toBool());
    auto *quotes = new QCheckBox(QCoreApplication::translate("Dialogs", "Replace straight quotes with smart quotes"), proof);
    auto *autoformat = new QCheckBox(QCoreApplication::translate("Dialogs", "AutoFormat as you type (dashes, fractions, ordinals, automatic lists)"), proof);
    autoformat->setChecked(st.value("proof/autoformat", true).toBool());
    quotes->setChecked(st.value("proof/smartQuotes", true).toBool());
    pf->addRow(asType);
    pf->addRow(ignoreUpper);
    pf->addRow(ignoreNum);
    pf->addRow(autocorrect);
    pf->addRow(quotes);
    pf->addRow(autoformat);
    tabs->addTab(proof, QCoreApplication::translate("Dialogs", "Proofing"));
    for (const OptionsPage &page : appPages) tabs->addTab(page.build(), page.title);
    dlg.v->addWidget(tabs);
    if (!dlg.exec()) return false;
    st.setValue("user/name", name->text());
    st.setValue("user/initials", initials->text());
    st.setValue("ui/theme", theme->currentIndex());
    if (theme->currentIndex() != oldTheme) applyUiTheme(theme->currentIndex());   // the whole program changes now
    if (const QString lang = uiLang->currentData().toString(); lang != oldLang) {
        st.setValue("ui/language", lang);
        QMessageBox::information(p, QCoreApplication::translate("Dialogs", "Display Language"), forThisApp(QCoreApplication::translate("Dialogs", "JeffPub shows its text in the new language the next time it starts.")));
    }
    st.setValue("proof/ignoreUpper", ignoreUpper->isChecked());
    st.setValue("proof/ignoreNumbers", ignoreNum->isChecked());
    st.setValue("proof/autocorrect", autocorrect->isChecked());
    st.setValue("proof/smartQuotes", quotes->isChecked());
    st.setValue("proof/autoformat", autoformat->isChecked());
    for (const OptionsPage &page : appPages)
        if (page.apply) page.apply();
    if (online) {
        if (stats->isChecked() != telemetry::enabled()) telemetry::setEnabled(stats->isChecked());
        st.setValue("updates/auto", updates->isChecked());
        Updater::setBetaChannel(beta->isChecked());
    }
    ed->setView([&](ViewOptions &v) { v.spelling = asType->isChecked(); });
    return true;
}

} // namespace jp
