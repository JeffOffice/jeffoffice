#include "app/crunchfilescreen.h"

#include "app/crunchwindow.h"
#include "app/filescreenparts.h"
#include "app/icons.h"
#include "app/settings.h"
#include "core/appinfo.h"

#include <QAction>
#include <QFileInfo>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QStyleHints>
#include <QVBoxLayout>
#include "app/plaintext.h"

namespace jp {

CrunchFileScreen::CrunchFileScreen(CrunchWindow *win) : FileScreen(win), m_win(win)
{
    setBackButton(tr("Back to workbook"), tr("Return to your workbook (Esc)"));
    setPageBuilder([this](const QString &key) { return buildPage(key); });
    addSection(tr("Start"));
    addPage(QStringLiteral("open"), QStringLiteral("folder-open"), tr("Open"));
    addSection(tr("This workbook"));
    addCommand(QStringLiteral("save"), QStringLiteral("save"), tr("Save"), [this] {
        if (m_win->act("file.save")->isEnabled()) m_win->act("file.save")->trigger();
        Q_EMIT closeRequested();
    });
    addCommand(QStringLiteral("saveAs"), QStringLiteral("save"), tr("Save As"), [this] {
        Q_EMIT closeRequested();
        if (m_win->act("file.saveAs")->isEnabled()) m_win->act("file.saveAs")->trigger();
    });
    addPage(QStringLiteral("print"), QStringLiteral("printer"), tr("Print"));
    addPage(QStringLiteral("export"), QStringLiteral("file-output"), tr("Export"));
    addCommand(QStringLiteral("close"), QStringLiteral("circle-x"), tr("Close"), [this] { m_win->act("file.close")->trigger(); Q_EMIT closeRequested(); });
    addSection(appInfo().name);
    addCommand(QStringLiteral("options"), QStringLiteral("settings"), tr("Options"), [this] { m_win->act("file.options")->trigger(); });
    addPage(QStringLiteral("about"), QStringLiteral("info"), tr("About"));
}

// The pages are built when they are shown, so they show what is now.
QWidget *CrunchFileScreen::buildPage(const QString &key)
{
    if (key == "open") return buildOpen();
    if (key == "print") return buildPrint();
    if (key == "export") return buildExport();
    if (key == "about") return buildAbout();
    return nullptr;
}

// ---------------- Open ----------------
QWidget *CrunchFileScreen::buildOpen()
{
    auto *w = new QWidget();
    auto *v = new QVBoxLayout(w);
    v->setContentsMargins(40, 30, 40, 30);
    v->addWidget(heading(tr("Open"), w));
    auto *cards = new QHBoxLayout();
    cards->setSpacing(12);
    auto *browse = new ActionCard(QStringLiteral("folder-open"), tr("Browse"), tr("Open a workbook from your computer."), w);
    connect(browse, &QAbstractButton::clicked, this, [this] { m_win->act("file.openDialog")->trigger(); });
    cards->addWidget(browse, 1);
    auto *text = new ActionCard(QStringLiteral("file-text"), tr("Open a Text File"), tr("Choose how the columns of a .csv or .txt file are read."), w);
    connect(text, &QAbstractButton::clicked, this, [this] { m_win->act("file.openText")->trigger(); });
    cards->addWidget(text, 1);
    v->addLayout(cards);
    v->addSpacing(14);
    auto section = [&](const QString &t) {
        QLabel *l = sectionLabel(t, w);
        l->setContentsMargins(2, 12, 0, 2);
        v->addWidget(l);
    };
    const QStringList pinned = Settings::get().pinnedFiles();
    auto addList = [&](const QStringList &files, bool isPinned) {
        auto *list = new QListWidget(w);
        list->setIconSize(QSize(32, 32));
        list->setContextMenuPolicy(Qt::CustomContextMenu);
        list->setMouseTracking(true);
        list->setMaximumWidth(820);
        list->setStyleSheet(QStringLiteral("QListWidget{background:%1; border:1px solid %2; border-radius:10px; padding:6px; outline:0;}"
                                           "QListWidget::item{padding:6px 8px; border-radius:7px; color:%4;}"
                                           "QListWidget::item:hover{background:%3;}"
                                           "QListWidget::item:selected{background:%3; color:%4;}")
                                .arg(uiCard().name(), uiLine().name(), uiCardPressed().name(), uiText().name()));
        for (const QString &f : files) {
            if (!isPinned && pinned.contains(f)) continue;
            const QFileInfo fi(f);
            auto *it = new QListWidgetItem(icon(QStringLiteral("file-spreadsheet")), QStringLiteral("%1\n%2").arg(fi.fileName(), fi.absolutePath()));
            it->setData(Qt::UserRole, f);
            if (!fi.exists()) it->setForeground(QColor(150, 150, 150));
            list->addItem(it);
        }
        if (list->count() == 0) {
            delete list;
            auto *empty = mutedLabel(tr("Workbooks you open will appear here."), w, 0.6);
            empty->setContentsMargins(2, 4, 0, 0);
            v->addWidget(empty);
            return;
        }
        // As tall as its rows, so a short list leaves no empty space; the page scrolls.
        list->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        list->setFixedHeight(list->sizeHintForRow(0) * list->count() + 2 * list->frameWidth() + 12);
        auto open = [this](QListWidgetItem *it) { m_win->openFile(it->data(Qt::UserRole).toString()); };
        connect(list, &QListWidget::itemActivated, this, open);
        if (!QGuiApplication::styleHints()->singleClickActivation()) connect(list, &QListWidget::itemClicked, this, open);
        connect(list, &QListWidget::customContextMenuRequested, this, [this, list, isPinned](const QPoint &pos) {
            QListWidgetItem *it = list->itemAt(pos);
            if (!it) return;
            const QString f = it->data(Qt::UserRole).toString();
            QMenu m;
            m.addAction(isPinned ? tr("Unpin from list") : tr("Pin to list"), [this, f, isPinned] {
                Settings::get().setPinned(f, !isPinned);
                showPage(QStringLiteral("open"));
            });
            m.addAction(tr("Remove from list"), [this, f] {
                Settings::get().removeRecentFile(f);
                showPage(QStringLiteral("open"));
            });
            m.exec(list->viewport()->mapToGlobal(pos));
        });
        v->addWidget(list);
    };
    if (!pinned.isEmpty()) {
        section(tr("Pinned"));
        addList(pinned, true);
    }
    section(tr("Recent Workbooks"));
    addList(Settings::get().recentFiles(), false);
    v->addStretch(1);
    return w;
}

// ---------------- About ----------------
QWidget *CrunchFileScreen::buildAbout()
{
    auto *w = new QWidget();
    auto *v = new QVBoxLayout(w);
    v->setContentsMargins(40, 30, 40, 30);
    v->setSpacing(14);
    v->addWidget(heading(tr("About"), w));
    v->addWidget(suiteCard(w, this));

    // Identity card: badge, name, version, the one-line description, and where to find it.
    auto *id = cardFrame(w);
    id->setMaximumWidth(820);
    auto *ih = new QHBoxLayout(id);
    ih->setContentsMargins(20, 18, 20, 18);
    ih->setSpacing(16);
    auto *badge = new QLabel(id);
    badge->setPixmap(appBadge(this, 56, 24, 14));
    ih->addWidget(badge, 0, Qt::AlignTop);
    auto *iv = new QVBoxLayout();
    iv->setSpacing(3);
    auto *name = plainLabel(new QLabel(appInfo().name, id));
    QFont nf = name->font();
    nf.setWeight(QFont::DemiBold);
    nf.setPointSizeF(nf.pointSizeF() * 1.35);
    name->setFont(nf);
    iv->addWidget(name);
    iv->addWidget(mutedLabel(tr("Version %1 · Built with Qt %2").arg(QStringLiteral(JP_VERSION), QString::fromLatin1(qVersion())), id));
    auto *tag = new QLabel(tr("Free, open-source spreadsheets for Windows, macOS, and Linux. This is a Preview."), id);
    tag->setWordWrap(true);
    fitHeight(tag);
    iv->addWidget(tag);
    const QString repo = QStringLiteral("https://github.com/") + appInfo().repository;
    // plain: reviewed: our own text and links, no file text
    auto *links = new QLabel(QStringLiteral("<a href=\"%1\">%2</a> &nbsp;·&nbsp; <a href=\"%1/releases\">%3</a> &nbsp;·&nbsp; <a href=\"%1/issues\">%4</a>")
                                 .arg(repo, tr("Source code"), tr("Downloads"), tr("Report a problem")),
                             id);
    links->setOpenExternalLinks(true);
    iv->addWidget(links);
    ih->addLayout(iv, 1);
    v->addWidget(id);

    // License and legal notices.
    auto *legal = cardFrame(w);
    legal->setMaximumWidth(820);
    auto *lv = new QVBoxLayout(legal);
    lv->setContentsMargins(20, 16, 20, 16);
    lv->setSpacing(8);
    auto *lh = new QLabel(tr("License and notices"), legal);
    QFont lf = lh->font();
    lf.setWeight(QFont::DemiBold);
    lh->setFont(lf);
    lv->addWidget(lh);
    auto *lt = mutedLabel(tr("Copyright © 2026 JeffOffice LLC.<br><br>"
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
    v->addWidget(legal);
    v->addStretch(1);
    return w;
}

} // namespace jp
