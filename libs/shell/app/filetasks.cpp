#include "app/filetasks.h"

#include "app/filescreenparts.h"
#include "app/icons.h"
#include "app/plaintext.h"
#include "app/settings.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QStandardPaths>
#include <QStyleHints>
#include <QVBoxLayout>

namespace jp {

namespace {

std::function<QString(const QString &)> &nameStandIn()
{
    static std::function<QString(const QString &)> standIn;
    return standIn;
}

QString thumbnailPath(const QString &templatePath) { return QFileInfo(templatePath).absolutePath() + QLatin1Char('/') + QFileInfo(templatePath).completeBaseName() + QStringLiteral(".png"); }

} // namespace

// ---- Backup copy

bool backupOnSave() { return Settings::get().value(QStringLiteral("save/backup"), false).toBool(); }

QString backupPathFor(const QString &path)
{
    const QFileInfo fi(path);
    return fi.absoluteDir().filePath(QStringLiteral("Backup of ") + fi.fileName());
}

bool keepBackupBeforeSave(const QString &path)
{
    if (!backupOnSave() || !QFileInfo(path).isFile()) return false;
    const QString backup = backupPathFor(path);
    QFile::remove(backup);
    return QFile::copy(path, backup);
}

// ---- My Templates

QString myTemplatesFolder() { return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/Templates"); }

QStringList myTemplateFiles(const QStringList &nameFilters)
{
    QStringList out;
    for (const QFileInfo &fi : QDir(myTemplatesFolder()).entryInfoList(nameFilters, QDir::Files, QDir::Name | QDir::IgnoreCase)) out << fi.absoluteFilePath();
    return out;
}

QString myTemplatePath(const QString &name, const QString &ending)
{
    QString clean;
    for (const QChar c : name.trimmed())
        if (!QStringLiteral("\\/:*?\"<>|").contains(c) && c.unicode() >= 32) clean += c;
    clean = clean.trimmed();
    while (clean.endsWith(QLatin1Char('.'))) clean.chop(1);
    if (clean.isEmpty()) return QString();
    return myTemplatesFolder() + QLatin1Char('/') + clean + ending;
}

void setTemplateNameStandIn(const std::function<QString(const QString &)> &standIn) { nameStandIn() = standIn; }

QString askMyTemplatePath(QWidget *parent, const QString &suggested, const QString &ending)
{
    QString name;
    if (nameStandIn()) name = nameStandIn()(suggested);
    else {
        bool ok = false;
        name = QInputDialog::getText(parent, QCoreApplication::translate("FileTasks", "Save as Template"), QCoreApplication::translate("FileTasks", "Template name:"), QLineEdit::Normal, suggested, &ok);
        if (!ok) return QString();
    }
    const QString path = myTemplatePath(name, ending);
    if (path.isEmpty()) return QString();
    if (QFileInfo::exists(path) && !nameStandIn()
        && plainQuestion(parent, QCoreApplication::translate("FileTasks", "Save as Template"), QCoreApplication::translate("FileTasks", "A template called \"%1\" is already in My Templates. Replace it?").arg(QFileInfo(path).completeBaseName()))
            != QMessageBox::Yes)
        return QString();
    QDir().mkpath(myTemplatesFolder());
    return path;
}

bool saveTemplateThumbnail(const QString &templatePath, const QImage &picture)
{
    if (picture.isNull()) return false;
    return picture.save(thumbnailPath(templatePath), "PNG");
}

QImage templateThumbnail(const QString &templatePath)
{
    QImage img;
    img.load(thumbnailPath(templatePath));
    return img;
}

bool removeMyTemplate(const QString &templatePath)
{
    QFile::remove(thumbnailPath(templatePath));
    return QFile::remove(templatePath);
}

QWidget *myTemplatesPanel(QWidget *parent, const QStringList &nameFilters, const QString &iconName, const std::function<void(const QString &)> &create)
{
    auto *w = new QWidget(parent);
    auto *v = new QVBoxLayout(w);
    v->setContentsMargins(0, 0, 0, 0);
    QLabel *title = sectionLabel(QCoreApplication::translate("FileTasks", "My Templates"), w);
    title->setContentsMargins(2, 12, 0, 2);
    v->addWidget(title);
    const QStringList files = myTemplateFiles(nameFilters);
    if (files.isEmpty()) {
        auto *empty = mutedLabel(QCoreApplication::translate("FileTasks", "Templates you save appear here. Choose Save as Template on the File screen to make one from the document you are working on."), w, 0.6);
        empty->setWordWrap(true);
        empty->setContentsMargins(2, 4, 0, 0);
        v->addWidget(empty);
        return w;
    }
    auto *list = new QListWidget(w);
    list->setObjectName(QStringLiteral("myTemplates"));
    list->setIconSize(QSize(40, 52));
    list->setContextMenuPolicy(Qt::CustomContextMenu);
    list->setMouseTracking(true);
    list->setMaximumWidth(820);
    list->setStyleSheet(QStringLiteral("QListWidget{background:%1; border:1px solid %2; border-radius:10px; padding:6px; outline:0;}"
                                       "QListWidget::item{padding:6px 8px; border-radius:7px; color:%4;}"
                                       "QListWidget::item:hover{background:%3;}"
                                       "QListWidget::item:selected{background:%3; color:%4;}")
                            .arg(uiCard().name(), uiLine().name(), uiCardPressed().name(), uiText().name()));
    for (const QString &f : files) {
        const QFileInfo fi(f);
        const QImage thumb = templateThumbnail(f);
        auto *it = new QListWidgetItem(thumb.isNull() ? icon(iconName) : QIcon(QPixmap::fromImage(thumb.scaled(QSize(80, 104), Qt::KeepAspectRatio, Qt::SmoothTransformation))),
                                       QStringLiteral("%1\n%2").arg(fi.completeBaseName(), QCoreApplication::translate("FileTasks", "Saved %1").arg(QLocale().toString(fi.lastModified(), QLocale::ShortFormat))));
        it->setData(Qt::UserRole, f);
        list->addItem(it);
    }
    list->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    list->setFixedHeight(list->sizeHintForRow(0) * list->count() + 2 * list->frameWidth() + 12);
    auto start = [create](QListWidgetItem *it) { create(it->data(Qt::UserRole).toString()); };
    QObject::connect(list, &QListWidget::itemActivated, list, start);
    if (!QGuiApplication::styleHints()->singleClickActivation()) QObject::connect(list, &QListWidget::itemClicked, list, start);
    QObject::connect(list, &QListWidget::customContextMenuRequested, list, [list](const QPoint &pos) {
        QListWidgetItem *it = list->itemAt(pos);
        if (!it) return;
        const QString f = it->data(Qt::UserRole).toString();
        QMenu m;
        m.addAction(QCoreApplication::translate("FileTasks", "Remove from My Templates"), [list, f, it] {
            if (removeMyTemplate(f)) delete list->takeItem(list->row(it));
        });
        m.exec(list->viewport()->mapToGlobal(pos));
    });
    v->addWidget(list);
    return w;
}

// ---- The other ways to open

QString openHowText(OpenHow how)
{
    switch (how) {
    case OpenHow::ReadOnly: return QCoreApplication::translate("FileTasks", "Open Read-Only");
    case OpenHow::Copy: return QCoreApplication::translate("FileTasks", "Open as Copy");
    case OpenHow::NewWindow: return QCoreApplication::translate("FileTasks", "Open in New Window");
    case OpenHow::Repair: return QCoreApplication::translate("FileTasks", "Open and Repair");
    case OpenHow::Normal: break;
    }
    return QCoreApplication::translate("FileTasks", "Open");
}

void addOpenWays(QMenu *menu, const std::function<void(OpenHow)> &choose)
{
    for (OpenHow how : {OpenHow::ReadOnly, OpenHow::Copy, OpenHow::NewWindow, OpenHow::Repair}) {
        QAction *a = menu->addAction(openHowText(how), [choose, how] { choose(how); });
        a->setObjectName(QStringLiteral("openWay%1").arg(int(how)));
    }
}

QWidget *moreWaysToOpenCard(QWidget *parent, const std::function<void(OpenHow)> &choose)
{
    auto *card = new ActionCard(QStringLiteral("app-window"), QCoreApplication::translate("FileTasks", "More Ways to Open"),
                                QCoreApplication::translate("FileTasks", "Open a file for reading only, as a copy, in a new window, or repaired."), parent);
    card->setObjectName(QStringLiteral("moreWaysToOpen"));
    QObject::connect(card, &QAbstractButton::clicked, card, [card, choose] {
        QMenu menu(card);
        addOpenWays(&menu, choose);
        menu.exec(card->mapToGlobal(QPoint(0, card->height())));
    });
    return card;
}

} // namespace jp
