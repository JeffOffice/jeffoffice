#include "app/aboutpage.h"

#include "app/plaintext.h"
#include "app/settings.h"
#include "app/telemetry.h"
#include "app/updater.h"
#include "core/appinfo.h"
#include "core/version.h"

#include <QCheckBox>
#include <QCoreApplication>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSignalBlocker>
#include <QVBoxLayout>

namespace jp {

namespace {

// The page: its two choices are read again each time it is shown, so one made in another app is what it shows.
class AboutPageWidget : public QWidget {
public:
    using QWidget::QWidget;
    std::function<void()> refresh;

protected:
    void showEvent(QShowEvent *e) override
    {
        QWidget::showEvent(e);
        if (refresh) refresh();
    }
};

struct AboutPageText {
    Q_DECLARE_TR_FUNCTIONS(jp::AboutPage)
};

} // namespace

QWidget *buildAboutPage(QWidget *parent, const QWidget *screen, const AboutHost &host)
{
    auto *w = new AboutPageWidget(parent);
    w->setObjectName(QStringLiteral("aboutPage"));
    auto *v = new QVBoxLayout(w);
    v->setContentsMargins(40, 30, 40, 30);
    v->setSpacing(14);
    v->addWidget(heading(AboutPageText::tr("About"), w));

    // The suite the app belongs to: the JeffOffice logo, then each app.
    v->addWidget(suiteCard(w, screen));

    // Identity card: badge, name, version, the one-line description, and where to find it.
    auto *id = cardFrame(w);
    id->setMaximumWidth(820);
    auto *ih = new QHBoxLayout(id);
    ih->setContentsMargins(20, 18, 20, 18);
    ih->setSpacing(16);
    auto *badge = new QLabel(id);
    badge->setPixmap(appBadge(screen, 56, 24, 14));
    ih->addWidget(badge, 0, Qt::AlignTop);
    auto *iv = new QVBoxLayout();
    iv->setSpacing(3);
    auto *name = plainLabel(new QLabel(appInfo().name, id));
    QFont nf = name->font();
    nf.setWeight(QFont::DemiBold);
    nf.setPointSizeF(nf.pointSizeF() * 1.35);
    name->setFont(nf);
    iv->addWidget(name);
    iv->addWidget(mutedLabel(AboutPageText::tr("Version %1 · Built with Qt %2").arg(jp::versionText(), QString::fromLatin1(qVersion())), id));
    auto *tag = plainLabel(new QLabel(host.tagline, id));
    tag->setWordWrap(true);
    fitHeight(tag);
    iv->addWidget(tag);
    const QString repo = QStringLiteral("https://github.com/") + appInfo().repository;
    // plain: reviewed: our own text and links, no file text
    auto *links = new QLabel(QStringLiteral("<a href=\"%1\">%2</a> &nbsp;·&nbsp; <a href=\"%1/releases\">%3</a> &nbsp;·&nbsp; <a href=\"%1/issues\">%4</a>")
                                 .arg(repo, AboutPageText::tr("Source code"), AboutPageText::tr("Downloads"), AboutPageText::tr("Report a problem")),
                             id);
    links->setOpenExternalLinks(true);
    iv->addWidget(links);
    // Updates.
    auto *uh = new QHBoxLayout();
    uh->setSpacing(10);
    auto *checkBtn = new QPushButton(AboutPageText::tr("Check for Updates"), id);
    checkBtn->setCursor(Qt::PointingHandCursor);
    QObject::connect(checkBtn, &QPushButton::clicked, w, [w] {
        if (!updater()) setUpdater(new Updater(w->window()));
        updater()->check(true);
    });
    auto *autoBox = new QCheckBox(AboutPageText::tr("Check for updates automatically"), id);
    autoBox->setChecked(Settings::get().value(QStringLiteral("updates/auto"), true).toBool());
    QObject::connect(autoBox, &QCheckBox::toggled, w, [](bool on) { Settings::get().setValue(QStringLiteral("updates/auto"), on); });
    uh->addWidget(checkBtn);
    uh->addWidget(autoBox);
    uh->addStretch(1);
    iv->addSpacing(6);
    iv->addLayout(uh);
    // Anonymous usage statistics.
    auto *statsBox = new QCheckBox(AboutPageText::tr("Send anonymous usage statistics"), id);
    statsBox->setToolTip(plainToolTip(AboutPageText::tr("Once a day, and the day it's updated: %1's version, your operating system and language, and how often each command is used. "
                            "Never your files, their names, or anything in them.")
                             .arg(appInfo().name)));
    statsBox->setChecked(telemetry::enabled());
    QObject::connect(statsBox, &QCheckBox::toggled, w, [](bool on) {
        if (on != telemetry::enabled()) telemetry::setEnabled(on);   // (showing the page again sets the box to what is chosen: nothing to write)
    });
    auto *statsLink = new QLabel(QStringLiteral("<a href=\"https://telemetry-production-9964.up.railway.app/\">%1</a>").arg(AboutPageText::tr("What's sent")), id);
    statsLink->setOpenExternalLinks(true);
    auto *sh = new QHBoxLayout();
    sh->setSpacing(10);
    sh->addWidget(statsBox);
    sh->addWidget(statsLink);
    sh->addWidget(telemetry::privacyLink(id));
    sh->addStretch(1);
    iv->addLayout(sh);
    iv->addWidget(telemetry::installNumberLine(id, statsBox));
    ih->addLayout(iv, 1);
    v->addWidget(id);

    w->refresh = [autoBox, statsBox] {
        Settings::get().sync();   // a choice made meanwhile in another app counts
        const QSignalBlocker a(autoBox);
        autoBox->setChecked(Settings::get().value(QStringLiteral("updates/auto"), true).toBool());
        statsBox->setChecked(telemetry::enabled());
    };

    // Your name, used as the author.
    auto *you = cardFrame(w);
    you->setMaximumWidth(820);
    auto *yv = new QVBoxLayout(you);
    yv->setContentsMargins(20, 16, 20, 16);
    yv->setSpacing(6);
    auto *yh = new QLabel(AboutPageText::tr("Your name"), you);
    QFont yf = yh->font();
    yf.setWeight(QFont::DemiBold);
    yh->setFont(yf);
    yv->addWidget(yh);
    yv->addWidget(mutedLabel(host.nameNote, you));
    auto *nameEdit = new QLineEdit(Settings::get().userName(), you);
    nameEdit->setMinimumHeight(32);
    nameEdit->setMaximumWidth(360);
    QObject::connect(nameEdit, &QLineEdit::editingFinished, w, [nameEdit] { Settings::get().setValue("user/name", nameEdit->text()); });
    yv->addWidget(nameEdit);
    v->addWidget(you);

    // License and legal notices.
    auto *legal = cardFrame(w);
    legal->setMaximumWidth(820);
    auto *lv = new QVBoxLayout(legal);
    lv->setContentsMargins(20, 16, 20, 16);
    lv->setSpacing(8);
    auto *lh = new QLabel(AboutPageText::tr("License and notices"), legal);
    lh->setFont(yf);
    lv->addWidget(lh);
    auto *lt = mutedLabel(AboutPageText::tr("Copyright © 2026 JeffOffice LLC.<br><br>"
                             "%1 is free software under the "
                             "<a href=\"%2\">GNU General Public License v3.0</a>. "
                             "It comes with ABSOLUTELY NO WARRANTY; see the license for details. "
                             "<a href=\"%3\">Read the license on gnu.org</a>.<br><br>"
                             "%1 is an independent open-source project. It is not affiliated with, endorsed by or supported by any other "
                             "software company. Product names are trademarks of their owners.")
                              .arg(appInfo().name, repo + QStringLiteral("/blob/main/LICENSE"), QStringLiteral("https://www.gnu.org/licenses/gpl-3.0.html")),
                          legal, 0.75);
    lt->setWordWrap(true);
    fitHeight(lt);
    lt->setTextFormat(Qt::RichText);
    lt->setOpenExternalLinks(true);
    lv->addWidget(lt);
    lv->addWidget(legalLinks(legal));
    // Every bundled component, linked to its project and its license: the parts all four apps have, then the app's own.
    addThirdPartyList(lv, legal, yf, host.extras);
    v->addWidget(legal);
    v->addStretch(1);
    return w;
}

} // namespace jp
