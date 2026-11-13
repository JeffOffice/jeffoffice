// Options: the pages every app shares (General, with the suite's settings, and
// Proofing), then the app's own.

#include "app/optionsdialog.h"

#include "app/appwords.h"
#include "app/editor.h"
#include "app/appwindow.h"
#include "app/filetasks.h"
#include "app/keymappage.h"
#include "app/quicktoolbarpage.h"
#include "app/ribbonpage.h"
#include "app/i18n.h"
#include "app/settings.h"
#include "app/telemetry.h"
#include "app/theme.h"
#include "app/updater.h"
#include "grammar/grammarrules.h"

#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTabWidget>
#include <QVBoxLayout>
#include "app/plaintext.h"

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

// The kinds of grammar issue: each is one checkbox, and what the choice is goes into the suite's settings when the dialog is accepted (the open documents are looked at again at once).
bool grammarSettingsDialog(QWidget *p, quint32 *chosen)
{
    struct Group {
        GrammarIssue::Kind kind;
        QString label, about;
    };
    const QVector<Group> groups = {
        {GrammarIssue::Grammar, QCoreApplication::translate("Dialogs", "Grammar"), QCoreApplication::translate("Dialogs", "Verbs that do not agree, \"a\" and \"an\", and the wrong form of a verb.")},
        {GrammarIssue::Punctuation, QCoreApplication::translate("Dialogs", "Punctuation"), QCoreApplication::translate("Dialogs", "Extra or missing spaces, commas, and hyphens.")},
        {GrammarIssue::Capitalization, QCoreApplication::translate("Dialogs", "Capitalization"), QCoreApplication::translate("Dialogs", "Sentences, \"I\", and the days and months.")},
        {GrammarIssue::WordChoice, QCoreApplication::translate("Dialogs", "Mixed-up words"), QCoreApplication::translate("Dialogs", "Words that sound alike, such as \"its\" and \"it's\".")},
        {GrammarIssue::Repetition, QCoreApplication::translate("Dialogs", "Repetition"), QCoreApplication::translate("Dialogs", "A word typed twice.")},
        {GrammarIssue::Style, QCoreApplication::translate("Dialogs", "Style"), QCoreApplication::translate("Dialogs", "Long sentences, passive voice, and wordy phrases. These are suggestions, not mistakes.")},
    };
    Dlg dlg(p, QCoreApplication::translate("Dialogs", "Grammar Settings"));
    dlg.d.setObjectName(QStringLiteral("grammarSettingsDialog"));
    auto *intro = new QLabel(QCoreApplication::translate("Dialogs", "Look for these kinds of issues:"), &dlg.d);
    dlg.v->addWidget(intro);
    const quint32 now = chosen ? *chosen : grammarKinds();
    QVector<QCheckBox *> boxes;
    for (const Group &g : groups) {
        auto *box = new QCheckBox(g.label, &dlg.d);
        box->setObjectName(QStringLiteral("grammarKind%1").arg(int(g.kind)));
        box->setChecked(now & grammar::kindBit(g.kind));
        box->setToolTip(jp::plainToolTip(g.about));
        box->setAccessibleDescription(g.about);
        dlg.v->addWidget(box);
        auto *about = jp::plainLabel(new QLabel(g.about, &dlg.d));
        about->setWordWrap(true);
        about->setContentsMargins(22, 0, 0, 4);
        dlg.v->addWidget(about);
        boxes << box;
    }
    if (!dlg.exec()) return false;
    quint32 kinds = grammar::kindBit(GrammarIssue::Other);   // (what no group names is always looked for)
    for (int i = 0; i < groups.size(); ++i)
        if (boxes[i]->isChecked()) kinds |= grammar::kindBit(groups[i].kind);
    if (chosen) *chosen = kinds;
    else setGrammarKinds(kinds);
    return true;
}

bool optionsDialog(QWidget *p, Editor *, const QVector<OptionsPage> &appPages, bool grammar)
{
    Settings &st = Settings::get();
    Dlg dlg(p, forThisApp(QCoreApplication::translate("Dialogs", "JeffPub Options")));
    dlg.d.setObjectName(QStringLiteral("optionsDialog"));
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
    // Every app keeps the file as it was before a save writes over it, when asked to.
    auto *backup = new QCheckBox(QCoreApplication::translate("Dialogs", "Always create backup copy"), gen);
    backup->setObjectName(QStringLiteral("backupCopy"));
    backup->setToolTip(jp::plainToolTip(QCoreApplication::translate("Dialogs", "Before a save writes over a file, keeps the file as it was, as \"Backup of\" and its name, in the same folder.")));
    backup->setChecked(backupOnSave());
    gf->addRow(backup);
    // The language of JeffPub's own text; it takes effect at the next start.
    auto *uiLang = new QComboBox(gen);
    uiLang->addItem(QCoreApplication::translate("Dialogs", "Use system language"), QString());
    uiLang->addItem(QStringLiteral("English"), QStringLiteral("en"));
    // (the English variants follow English; the rest are in order of their files)
    const QStringList languages = availableUiLanguages();
    for (const QString &l : languages)
        if (l.startsWith(QLatin1String("en_"))) uiLang->addItem(uiLanguageName(l), l);
    for (const QString &l : languages)
        if (!l.startsWith(QLatin1String("en_"))) uiLang->addItem(uiLanguageName(l), l);
    const QString oldLang = st.value("ui/language").toString();
    uiLang->setCurrentIndex(std::max(0, uiLang->findData(oldLang)));
    gf->addRow(QCoreApplication::translate("Dialogs", "Display language:"), uiLang);
    // The choice the first start offered, which its message says is here,
    // and the update check: only in an app that sends and checks (the
    // choices are the suite's, the same in each app's Options).
    const bool online = appInfo().online;
    auto *stats = new QCheckBox(QCoreApplication::translate("Dialogs", "Send anonymous usage statistics"), gen);
    stats->setObjectName(QStringLiteral("stats"));
    stats->setToolTip(jp::plainToolTip(forThisApp(QCoreApplication::translate("Dialogs", "Once a day, and the day it's updated: JeffPub's version, your operating system and language, and how often each command is used. "
                                                                       "Never your files, their names, or anything in them."))));
    stats->setChecked(telemetry::enabled());
    // The choice, the privacy notice, and the install number, in one row.
    auto *statsRow = new QWidget(gen);
    auto *statsCol = new QVBoxLayout(statsRow);
    statsCol->setContentsMargins(0, 0, 0, 0);
    auto *statsTop = new QHBoxLayout();
    statsTop->addWidget(stats);
    statsTop->addWidget(telemetry::privacyLink(statsRow));
    statsTop->addStretch(1);
    statsCol->addLayout(statsTop);
    statsCol->addWidget(telemetry::installNumberLine(statsRow));
    gf->addRow(statsRow);
    auto *updates = new QCheckBox(forThisApp(QCoreApplication::translate("Dialogs", "Check for updates when JeffPub starts")), gen);
    updates->setObjectName(QStringLiteral("updates"));
    updates->setChecked(st.value("updates/auto", true).toBool());
    gf->addRow(updates);
    // Beta versions are the test builds published ahead of each regular release.
    auto *beta = new QCheckBox(QCoreApplication::translate("Dialogs", "Get beta versions (early builds for testing)"), gen);
    beta->setObjectName(QStringLiteral("betaUpdates"));
    const QString betaAbout = QCoreApplication::translate("Dialogs", "Beta versions arrive before the regular ones and may have rough edges. "
                                                                    "Leave this off to get only the regular versions.");
    beta->setToolTip(jp::plainToolTip(betaAbout));
    beta->setWhatsThis(jp::plainToolTip(betaAbout));
    beta->setAccessibleDescription(betaAbout);
    beta->setChecked(Updater::betaChannel());
    gf->addRow(beta);
    for (QWidget *box : {statsRow, static_cast<QWidget *>(updates), static_cast<QWidget *>(beta)}) gf->setRowVisible(box, online);
    tabs->addTab(gen, QCoreApplication::translate("Dialogs", "General"));
    auto *proof = new QWidget();
    auto *pf = new QFormLayout(proof);
    auto *asType = new QCheckBox(QCoreApplication::translate("Dialogs", "Check spelling as you type"), proof);
    asType->setChecked(spellAsYouType());
    QCheckBox *grammarAsType = nullptr;
    QPushButton *grammarSettings = nullptr;
    quint32 grammarChoice = grammarKinds();   // (what the Grammar settings dialog chose: kept until this dialog's OK)
    if (grammar) {
        grammarAsType = new QCheckBox(QCoreApplication::translate("Dialogs", "Check grammar as you type"), proof);
        grammarAsType->setChecked(grammarAsYouType());
        grammarSettings = new QPushButton(QCoreApplication::translate("Dialogs", "Grammar settings..."), proof);
        grammarSettings->setObjectName(QStringLiteral("grammarSettingsButton"));
        QObject::connect(grammarSettings, &QPushButton::clicked, &dlg.d, [&dlg, &grammarChoice] { grammarSettingsDialog(&dlg.d, &grammarChoice); });
    }
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
    auto *autolink = new QCheckBox(QCoreApplication::translate("Dialogs", "Turn web addresses into links as you type"), proof);
    autolink->setChecked(st.value("proof/autolink", true).toBool());
    pf->addRow(asType);
    if (grammarAsType) pf->addRow(grammarAsType);
    if (grammarSettings) pf->addRow(grammarSettings);
    pf->addRow(ignoreUpper);
    pf->addRow(ignoreNum);
    pf->addRow(autocorrect);
    pf->addRow(quotes);
    pf->addRow(autoformat);
    pf->addRow(autolink);
    tabs->addTab(proof, QCoreApplication::translate("Dialogs", "Proofing"));
    for (const OptionsPage &page : appPages) tabs->addTab(page.build(), page.title);
    // Customize: the ribbon, the quick toolbar, and the keyboard (the commands of the window this was opened from).
    KeyboardPage *keyboard = nullptr;
    RibbonPage *ribbonPage = nullptr;
    QuickToolbarPage *quickPage = nullptr;
    if (auto *win = qobject_cast<AppWindow *>(p ? p->window() : nullptr)) {
        auto *custom = new QWidget();
        auto *cl = new QVBoxLayout(custom);
        cl->setContentsMargins(0, 0, 0, 0);
        auto *parts = new QTabWidget(custom);
        parts->setObjectName(QStringLiteral("customizeTabs"));
        ribbonPage = new RibbonPage(win, parts);
        parts->addTab(ribbonPage, QCoreApplication::translate("Dialogs", "Ribbon"));
        quickPage = new QuickToolbarPage(win, parts);
        parts->addTab(quickPage, QCoreApplication::translate("Dialogs", "Quick Toolbar"));
        keyboard = new KeyboardPage(win, parts);
        parts->addTab(keyboard, QCoreApplication::translate("Dialogs", "Keyboard"));
        cl->addWidget(parts);
        tabs->addTab(custom, QCoreApplication::translate("Dialogs", "Customize"));
        if (const AppWindow::OptionsStart start = win->takeOptionsStart(); start != AppWindow::OptionsStart::Default) {   // (asked for from the ribbon's right-click menu)
            tabs->setCurrentWidget(custom);
            parts->setCurrentWidget(start == AppWindow::OptionsStart::Ribbon ? static_cast<QWidget *>(ribbonPage) : quickPage);
        }
        dlg.d.setMinimumSize(700, 600);
    }
    dlg.v->addWidget(tabs);
    if (!dlg.exec()) return false;
    st.setValue("user/name", name->text());
    st.setValue("user/initials", initials->text());
    st.setValue("save/backup", backup->isChecked());
    st.setValue("ui/theme", theme->currentIndex());
    if (theme->currentIndex() != oldTheme) applyUiTheme(theme->currentIndex());   // the whole program changes now
    if (const QString lang = uiLang->currentData().toString(); lang != oldLang) {
        st.setValue("ui/language", lang);
        jp::plainInformation(p, QCoreApplication::translate("Dialogs", "Display Language"), forThisApp(QCoreApplication::translate("Dialogs", "JeffPub shows its text in the new language the next time it starts.")));
    }
    st.setValue("proof/ignoreUpper", ignoreUpper->isChecked());
    st.setValue("proof/ignoreNumbers", ignoreNum->isChecked());
    st.setValue("proof/autocorrect", autocorrect->isChecked());
    st.setValue("proof/smartQuotes", quotes->isChecked());
    st.setValue("proof/autoformat", autoformat->isChecked());
    st.setValue("proof/autolink", autolink->isChecked());
    for (const OptionsPage &page : appPages)
        if (page.apply) page.apply();
    if (online) {
        if (stats->isChecked() != telemetry::enabled()) telemetry::setEnabled(stats->isChecked());
        st.setValue("updates/auto", updates->isChecked());
        Updater::setBetaChannel(beta->isChecked());
    }
    setSpellAsYouType(asType->isChecked());
    if (grammarAsType) setGrammarAsYouType(grammarAsType->isChecked());
    if (grammarSettings) setGrammarKinds(grammarChoice);
    if (keyboard) keyboard->apply();   // (every open window of the app takes the keys now)
    if (ribbonPage) ribbonPage->apply();   // (and the ribbon the person arranged)
    if (quickPage) quickPage->apply();     // (and the quick toolbar)
    return true;
}

} // namespace jp
