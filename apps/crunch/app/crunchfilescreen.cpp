#include "app/crunchfilescreen.h"
#include "core/version.h"

#include "app/crunchwindow.h"
#include "app/filescreenparts.h"
#include "app/filetasks.h"
#include "app/icons.h"
#include "app/settings.h"
#include "core/appinfo.h"
#include "core/crunchtemplates.h"
#include <QDateTime>
#include <QDir>
#include <QAbstractButton>
#include <QEvent>
#include <QGridLayout>
#include <QPainter>
#include <QSaveFile>
#include <QStandardPaths>

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
namespace {

// A card for a built-in template: a small picture of its top, then its name and what it is for.
class TemplateTile : public QAbstractButton {
public:
    TemplateTile(const QImage &preview, const QString &title, const QString &desc, QWidget *parent) : QAbstractButton(parent), m_preview(preview), m_desc(desc)
    {
        setText(title);
        setCursor(Qt::PointingHandCursor);
        setAttribute(Qt::WA_Hover);
        setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
        setFixedHeight(kPictureHeight + 62);
        setAccessibleName(title);
        setAccessibleDescription(desc);
    }
    QSize sizeHint() const override { return QSize(300, kPictureHeight + 62); }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        const QRectF r = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
        const bool hover = underMouse(), down = isDown();
        QColor border = uiLine();
        if (hover || hasFocus()) border = uiAccent();
        p.setPen(QPen(border, hover ? 1.4 : 1));
        p.setBrush(down ? uiCardPressed() : uiCard());
        p.drawRoundedRect(r, 10, 10);
        const QRectF pic(r.left() + 10, r.top() + 10, r.width() - 20, kPictureHeight);
        p.save();
        p.setClipRect(pic);
        p.drawImage(pic.topLeft(), m_preview.scaledToWidth(int(pic.width()), Qt::SmoothTransformation));
        p.restore();
        p.setPen(uiLine());
        p.setBrush(Qt::NoBrush);
        p.drawRect(pic);
        QFont tf = font();
        tf.setWeight(QFont::DemiBold);
        p.setFont(tf);
        p.setPen(uiText());
        p.drawText(QRectF(r.left() + 12, pic.bottom() + 6, r.width() - 24, 20), Qt::AlignLeft | Qt::AlignVCenter, text());
        p.setFont(font());
        QColor dc = uiText();
        dc.setAlphaF(0.68f);
        p.setPen(dc);
        p.drawText(QRectF(r.left() + 12, pic.bottom() + 26, r.width() - 24, 20), Qt::AlignLeft | Qt::AlignVCenter, fontMetrics().elidedText(m_desc, Qt::ElideRight, int(r.width() - 24)));
    }
    bool event(QEvent *e) override
    {
        if (e->type() == QEvent::HoverEnter || e->type() == QEvent::HoverLeave) update();
        return QAbstractButton::event(e);
    }

private:
    static constexpr int kPictureHeight = 130;
    QImage m_preview;
    QString m_desc;
};

} // namespace
} // namespace jp

namespace jp {

CrunchFileScreen::CrunchFileScreen(CrunchWindow *win) : FileScreen(win), m_win(win)
{
    setBackButton(tr("Back to workbook"), tr("Return to your workbook (Esc)"));
    setPageBuilder([this](const QString &key) { return buildPage(key); });
    addSection(tr("Start"));
    addPage(QStringLiteral("new"), QStringLiteral("file-plus"), tr("New"));
    addPage(QStringLiteral("open"), QStringLiteral("folder-open"), tr("Open"));
    addSection(tr("This workbook"));
    addPage(QStringLiteral("info"), QStringLiteral("file-text"), tr("Info"));
    addCommand(QStringLiteral("save"), QStringLiteral("save"), tr("Save"), [this] {
        if (m_win->act("file.save")->isEnabled()) m_win->act("file.save")->trigger();
        Q_EMIT closeRequested();
    });
    addCommand(QStringLiteral("saveAs"), QStringLiteral("save"), tr("Save As"), [this] {
        Q_EMIT closeRequested();
        if (m_win->act("file.saveAs")->isEnabled()) m_win->act("file.saveAs")->trigger();
    });
    addCommand(QStringLiteral("saveCopy"), QStringLiteral("files"), tr("Save a Copy"), [this] {
        Q_EMIT closeRequested();
        if (m_win->act("file.saveCopy")->isEnabled()) m_win->act("file.saveCopy")->trigger();
    });
    addCommand(QStringLiteral("saveTemplate"), QStringLiteral("layout-template"), tr("Save as Template"), [this] {
        Q_EMIT closeRequested();
        if (m_win->act("file.saveTemplate")->isEnabled()) m_win->act("file.saveTemplate")->trigger();
    });
    addCommand(QStringLiteral("protectPassword"), QStringLiteral("key"), tr("Protect with Password"), [this] {
        Q_EMIT closeRequested();
        if (m_win->act("protect.password")->isEnabled()) m_win->act("protect.password")->trigger();
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
    if (key == "info") return m_win->buildInfoPage(nullptr);
    if (key == "new") return buildNew();
    if (key == "open") return buildOpen();
    if (key == "print") return buildPrint();
    if (key == "export") return buildExport();
    if (key == "about") return buildAbout();
    return nullptr;
}

// ---------------- New ----------------
void CrunchFileScreen::startBlank()
{
    // A new workbook nobody has changed is already what is asked for; any other stays, and the new one opens in a window of its own (as Open does).
    if (!m_win->isUntouchedBlank()) m_win->act("file.new")->trigger();
    Q_EMIT closeRequested();
}

QWidget *CrunchFileScreen::buildNew()
{
    auto *w = new QWidget();
    auto *v = new QVBoxLayout(w);
    v->setContentsMargins(40, 30, 40, 30);
    v->addWidget(heading(tr("New"), w));
    auto *cards = new QHBoxLayout();
    cards->setSpacing(12);
    auto *blank = new ActionCard(QStringLiteral("file-spreadsheet"), tr("Blank Workbook"), tr("Start with an empty workbook that has one sheet."), w);
    connect(blank, &QAbstractButton::clicked, this, [this] { startBlank(); });
    cards->addWidget(blank, 1);
    cards->addStretch(1);
    v->addLayout(cards);
    // The templates JeffCrunch has ready, by kind: a workbook made from one is a copy with no file of its own.
    {
        const QString dir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QStringLiteral("/templates");
        QDir().mkpath(dir);
        for (const QFileInfo &old : QDir(dir).entryInfoList(QDir::Files))
            if (old.lastModified().daysTo(QDateTime::currentDateTime()) > 7) QFile::remove(old.absoluteFilePath());
        QStringList categories;
        const QVector<crunch::TemplateInfo> all = crunch::builtinTemplates();
        for (const crunch::TemplateInfo &t : all)
            if (!categories.contains(t.category)) categories << t.category;
        for (const QString &category : categories) {
            auto *label = sectionLabel(category, w);
            label->setContentsMargins(2, 12, 0, 2);
            v->addWidget(label);
            auto *row = new QGridLayout();
            row->setSpacing(12);
            int n = 0;
            for (const crunch::TemplateInfo &t : all) {
                if (t.category != category) continue;
                auto *card = new TemplateTile(crunch::builtinTemplatePreview(t.id, 420), t.name, t.description, w);
                card->setObjectName(QStringLiteral("template_") + t.id);
                connect(card, &QAbstractButton::clicked, this, [this, id = t.id, dir] {
                    const QByteArray bytes = crunch::builtinTemplateBytes(id);
                    const QString path = QStringLiteral("%1/%2-%3.xlsx").arg(dir, id, QString::number(QDateTime::currentMSecsSinceEpoch()));
                    QSaveFile file(path);
                    if (bytes.isEmpty() || !file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size() || !file.commit()) return;
                    m_win->openFile(path, OpenHow::Copy);
                    Q_EMIT closeRequested();
                });
                row->addWidget(card, n / 3, n % 3);
                ++n;
            }
            for (int c = 0; c < 3; ++c) row->setColumnStretch(c, 1);
            v->addLayout(row);
        }
    }
    // The templates a person saved: a workbook made from one starts as the template is, with no file of its own.
    v->addWidget(myTemplatesPanel(w, {QStringLiteral("*.xltx"), QStringLiteral("*.ots")}, QStringLiteral("file-spreadsheet"), [this](const QString &path) {
        m_win->openFile(path, OpenHow::Copy);
        Q_EMIT closeRequested();
    }));
    v->addStretch(1);
    return w;
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
    auto *recover = new ActionCard(QStringLiteral("life-buoy"), tr("Recover Unsaved Work"), tr("Open copies of unsaved work %1 kept when it closed unexpectedly.").arg(appInfo().name), w);
    connect(recover, &QAbstractButton::clicked, this, [this] { m_win->offerRecovery(true); });
    cards->addWidget(recover, 1);
    v->addLayout(cards);
    v->addWidget(moreWaysToOpenCard(w, [this](OpenHow how) { m_win->browseAndOpen(how); }));
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
            m.addSeparator();
            addOpenWays(&m, [this, f](OpenHow how) { m_win->openFile(f, how); });
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
    iv->addWidget(mutedLabel(tr("Version %1 · Built with Qt %2").arg(jp::versionText(), QString::fromLatin1(qVersion())), id));
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
    lv->addWidget(legalLinks(legal));
    // Every bundled component, linked to its project and its license (the same parts as the other apps).
    addThirdPartyList(lv, legal, lf);
    v->addWidget(legal);
    v->addStretch(1);
    return w;
}

} // namespace jp
