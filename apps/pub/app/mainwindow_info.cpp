// JeffPub's Info page: the publication's properties, Mark as Final, Inspect Publication, and Version History. The page itself is libs/shell/app/infopage.cpp; this gives it the publication's facts
// and does what its cards ask. A password is for .jpub files only: a .pub file must stay one the other publisher program opens. The properties are kept in the publication (so Undo covers them) and saved with it.
#include "app/mainwindow.h"

#include "app/backstage.h"
#include "app/dialogs.h"
#include "app/passworddialogs.h"
#include "app/plaintext.h"
#include "app/settings.h"
#include "app/versionhistory.h"
#include "io/crypto.h"
#include "io/jpubfile.h"
#include "office/officecrypt.h"
#include "office/privacy.h"

#include <QDateTime>
#include <QFileInfo>
#include <QLocale>
#include <QRegularExpression>
#include <QStatusBar>
#include <algorithm>

namespace jp {

void MainWindow::notice(const QString &title, const QString &text, bool warning)
{
    if (m_notifier) m_notifier(title, text);
    else if (warning) jp::plainWarning(this, title, text);
    else jp::plainInformation(this, title, text);
}

office::PackageProps MainWindow::properties() const
{
    const Document *d = m_ed->doc();
    return d ? d->props.toPackageProps() : office::PackageProps();
}

void MainWindow::applyProperties(const office::PackageProps &edited)
{
    Document *d = m_ed->doc();
    if (!d) return;
    office::PackageProps p = edited;
    p.modified = QDateTime::currentDateTimeUtc();
    if (m_personalInfoRemoved) {
        // (the names stay out until the publication is closed)
        p.creator.clear();
        p.lastModifiedBy.clear();
        p.company.clear();
        p.manager.clear();
    } else if (!Settings::get().userName().isEmpty()) {
        p.lastModifiedBy = Settings::get().userName();
    }
    if (!p.created.isValid()) p.created = p.modified;
    m_ed->change(tr("Properties"), [&] { d->props.setFromPackageProps(p); });
    updateTitle();
}

int MainWindow::removePersonalInformation()
{
    office::PackageProps p = properties();
    const int n = office::removePersonalNames(p);
    m_personalInfoRemoved = true;
    if (n > 0) applyProperties(p);
    return n;
}

bool MainWindow::markedAsFinal() const
{
    return office::isMarkedFinal(properties().custom);
}

QWidget *MainWindow::buildInfoPage(QWidget *parent)
{
    InfoHost h;
    h.kind = InfoHost::Kind::Publication;
    h.path = [this] { return m_ed->filePath(); };
    h.props = [this] { return properties(); };
    h.setProps = [this](const office::PackageProps &p) { applyProperties(p); };
    h.statistics = [this] {
        const Document *d = m_ed->doc();
        QVector<QPair<QString, QString>> rows;
        if (!d) return rows;
        int pics = 0, words = 0;
        d->forEachItem([&](Item *it, int, const QString &) {
            if (it->type() == ItemType::Picture) ++pics;
        });
        for (auto it = d->stories.cbegin(); it != d->stories.cend(); ++it)
            words += (*it)->doc->toPlainText().split(QRegularExpression("\\s+"), Qt::SkipEmptyParts).size();
        rows.push_back({tr("Pages"), QString::number(d->pages.size())});
        rows.push_back({tr("Page size"), QStringLiteral("%1 × %2").arg(Settings::get().format(d->pageSize().width()), Settings::get().format(d->pageSize().height()))});
        rows.push_back({tr("Pictures"), QString::number(pics)});
        rows.push_back({tr("Words"), QLocale().toString(words)});
        return rows;
    };
    h.markFinalBlocked = [this] { return markFinalBlocked(); };
    h.markedFinal = [this] { return markedAsFinal(); };
    h.setMarkedFinal = [this](bool on) { setMarkedFinal(on); };
    h.hasPassword = [this] { return m_fileProtected; };
    h.encrypt = [this] { encryptWithPassword(); };
    h.encryptBlocked = [this] { return encryptBlocked(); };
    h.inspect = [this] { inspectPublication(); };
    h.versionHistory = [this] { versionHistory(); };
    h.checkAccessibility = [this] {
        hideBackstage();
        act(QStringLiteral("rev.accessibility"))->trigger();
    };
    h.thumbnail = [this] {
        const Document *d = m_ed->doc();
        return d && !d->pages.isEmpty() ? pageThumbnail(std::clamp(m_ed->currentPage(), 0, int(d->pages.size()) - 1), 400) : QImage();
    };
    // What only a publication has: its business details, its print shop settings, and its checker.
    h.extraCards.push_back({QStringLiteral("infoBusiness"), QStringLiteral("contact"), tr("Business Information"), tr("Edit the contact details that templates and Business Information fields use."),
                            [this] { businessInfoDialog(this, m_ed); }});
    h.extraCards.push_back({QStringLiteral("infoPrintSettings"), QStringLiteral("printer"), tr("Commercial Print Settings"),
                            tr("Choose the color model (RGB, process CMYK, spot colors) and font embedding for a print shop."), [this] { documentPropertiesDialog(this, m_ed, 1); }});
    h.extraCards.push_back({QStringLiteral("infoDesignChecker"), QStringLiteral("shield-check"), tr("Run Design Checker"),
                            tr("Find problems such as text that doesn't fit, empty frames and low-resolution pictures."), [this] {
                                showTaskPane(QStringLiteral("designchecker"));
                                hideBackstage();
                            }});
    return jp::buildInfoPage(parent, h);
}

QString MainWindow::encryptBlocked() const
{
    if (m_ed->filePath().endsWith(QLatin1String(".pub"), Qt::CaseInsensitive)) return tr("Passwords are for .jpub files. A .pub file can't keep one; save the publication as a .jpub file first.");
    return {};
}

void MainWindow::encryptWithPassword()
{
    if (!m_ed->doc()) return;
    const QString why = encryptBlocked();
    if (!why.isEmpty()) {
        notice(tr("Encrypt with Password"), why);
        return;
    }
    QString password;
    const NewPasswordChoice choice = askNewFilePassword(this, m_fileProtected, &password);
    if (choice == NewPasswordChoice::Cancelled) return;
    crypto::wipe(m_filePassword);
    if (choice == NewPasswordChoice::Removed) {
        m_fileProtected = false;
        statusBar()->showMessage(tr("The password is taken off when you save."), 8000);
    } else {
        m_fileProtected = true;
        m_filePassword = password;
        m_protectionNoted = false;
        statusBar()->showMessage(tr("The publication is protected with a password when you save."), 8000);
    }
    // (the versions kept follow the password when the file is saved; a recovery copy written before is in the clear, and goes now)
    if (m_fileProtected) dropRecoveryCopy();
    crypto::wipe(password);
    // (a password is a change to save, as the properties are)
    m_ed->markUnsaved();
    updateTitle();
    if (m_backstage && m_backstage->isVisible()) showBackstage(QStringLiteral("info"));
}

QString MainWindow::markFinalBlocked() const
{
    if (properties().truncated) return tr("This file has more properties than %1 reads, so the mark can't be added.").arg(QStringLiteral("JeffPub"));
    if (m_ed->filePath().endsWith(QLatin1String(".pub"), Qt::CaseInsensitive)) return tr("A .pub file can't hold the mark. Save the publication as a .jpub file first.");
    return {};
}

void MainWindow::setMarkedFinal(bool on)
{
    if (!m_ed->doc() || m_ed->doc()->pages.empty()) return;
    if (on) {
        const QString why = markFinalBlocked();
        if (!why.isEmpty()) {
            notice(tr("Mark as Final"), why);
            return;
        }
    }
    office::PackageProps p = properties();
    p.custom = office::withMarkAsFinal(p.custom, on);
    if (on) {
        // The publication is marked and saved (it is the saved file that is final), then held for reading only.
        const bool wasReadOnly = isReadOnly();
        if (wasReadOnly) setReadOnly(false);
        applyProperties(p);
        if (!save()) {
            // (not saved: it is not final)
            p.custom = office::withMarkAsFinal(p.custom, false);
            applyProperties(p);
            if (wasReadOnly) setReadOnly(true);
            return;
        }
        // (saved as a kind of file that can't hold the mark: the save left it out, and the publication is not final)
        if (!markedAsFinal()) {
            statusBar()->showMessage(tr("The file that was saved can't hold the mark, so the publication is not marked as final."), 8000);
            return;
        }
        setReadOnly(true);
        statusBar()->showMessage(tr("Marked as final and saved. It opens for reading only."), 8000);
    } else {
        // (the editor changes nothing while it is read-only)
        setReadOnly(false);
        applyProperties(p);
        statusBar()->showMessage(tr("This publication can be edited again. Save it to keep the mark off."), 8000);
    }
    if (m_backstage && m_backstage->isVisible()) showBackstage(QStringLiteral("info"));
}

QVector<InspectItem> MainWindow::inspectItems()
{
    QVector<InspectItem> items;
    if (!m_ed->doc()) return items;
    Editor *ed = m_ed;
    items.push_back({tr("Document properties and personal information"), tr("The author, the person who last saved it, the company, and the manager."),
                     [this] {
                         const office::PackageProps p = properties();
                         int n = 0;
                         for (const QString &s : {p.creator, p.lastModifiedBy, p.company, p.manager})
                             if (!s.isEmpty()) ++n;
                         return n;
                     },
                     [this] { return removePersonalInformation(); },
                     true, QString()});
    items.push_back({tr("Business information"), tr("The names, addresses, phone numbers, and other contact details kept in the publication for Business Information fields."),
                     [ed] {
                         int n = 0;
                         for (const BusinessInfo &b : ed->doc()->biz)
                             for (const QString &key : BusinessInfo::keys())
                                 if (!b.field(key).isEmpty()) {
                                     ++n;
                                     break;
                                 }
                         return n;
                     },
                     [ed] {
                         int n = 0;
                         ed->change(tr("Remove Business Information"), [&] {
                             for (BusinessInfo &b : ed->doc()->biz) {
                                 bool had = false;
                                 for (const QString &key : BusinessInfo::keys())
                                     if (!b.field(key).isEmpty()) {
                                         b.setField(key, QString());
                                         had = true;
                                     }
                                 n += had;
                             }
                         });
                         return n;
                     },
                     true, QString()});
    items.push_back({tr("Mail merge data"), tr("The list of names and details that a mail or catalog merge reads from, and the file it came from."),
                     [ed] { return ed->doc()->merge.isEmpty() && ed->doc()->merge.path.isEmpty() ? 0 : std::max(1, int(ed->doc()->merge.rows.size())); },
                     [ed] {
                         const int n = ed->doc()->merge.isEmpty() && ed->doc()->merge.path.isEmpty() ? 0 : std::max(1, int(ed->doc()->merge.rows.size()));
                         if (n > 0) ed->change(tr("Remove Mail Merge Data"), [&] { ed->doc()->merge = MergeSource(); });
                         return n;
                     },
                     true, QString()});
    items.push_back({tr("Objects on the scratch area"), tr("Text, pictures, and shapes parked beside the pages: they do not print or export, but they are in the file."),
                     [ed] { return int(ed->doc()->scratch.size()); },
                     [ed] {
                         const int n = int(ed->doc()->scratch.size());
                         if (n > 0) {
                             ed->clearSelection();
                             ed->change(tr("Delete Scratch Area Objects"), [&] { ed->doc()->scratch.clear(); });
                         }
                         return n;
                     },
                     true, QString()});
    return items;
}

void MainWindow::inspectPublication()
{
    if (!m_ed->doc()) return;
    m_ed->endTextEdit();
    InspectDialog dialog(tr("Inspect Publication"), inspectItems(), this);
    dialog.exec();
    updateTitle();
}

// ---- Version History

QVector<write::DocVersion> MainWindow::versions() const
{
    if (m_ed->filePath().isEmpty()) return {};
    return write::VersionStore(m_ed->filePath(), QStringLiteral(".jpub")).list();
}

bool MainWindow::saveVersion(const QString &name)
{
    Document *d = m_ed->doc();
    if (!d) return false;
    const QString path = m_ed->filePath();
    if (path.isEmpty()) {
        notice(tr("Version History"), versionsNeedAFile(VersionHistoryHost::Kind::Publication), true);
        return false;
    }
    m_ed->endTextEdit();
    QString err;
    QByteArray bytes = publicationBytes(*d, pageThumbnail(0, 256), QFileInfo(path).absolutePath());
    // (a version of a publication that asks for a password asks for it too)
    const QString kept = m_versions.forNew(m_fileProtected ? m_filePassword : QString());
    if (!bytes.isEmpty() && !kept.isEmpty()) bytes = office::encryptPackage(bytes, kept);
    write::VersionStore store(path, QStringLiteral(".jpub"));
    if (bytes.isEmpty() || !store.add(name.trimmed().isEmpty() ? tr("Version %1").arg(store.list().size() + 1) : name, bytes, &err)) {
        notice(tr("Version History"), tr("The version couldn't be kept.\n%1").arg(err), true);
        return false;
    }
    statusBar()->showMessage(tr("Kept a version of %1.").arg(m_ed->displayName()), 4000);
    return true;
}

bool MainWindow::restoreVersion(const QString &file)
{
    if (!m_ed->doc() || m_ed->filePath().isEmpty()) return false;
    const QString path = m_ed->filePath();
    write::VersionStore store(path, QStringLiteral(".jpub"));
    QString err;
    QByteArray bytes;
    std::unique_ptr<Document> doc;
    if (m_versions.open(store.read(file), m_fileProtected ? m_filePassword : QString(), &bytes) && !bytes.isEmpty()) doc = publicationFromBytes(bytes, &err, QFileInfo(path).absolutePath());
    if (!doc) {
        notice(tr("Version History"), tr("That version can't be opened.\n%1").arg(err), true);
        return false;
    }
    QString which = file;
    for (const write::DocVersion &v : store.list())
        if (v.file == file) which = v.name;
    // What the publication is now is kept first: going back loses nothing.
    if (!saveVersion(tr("Before restoring \"%1\"").arg(which))) return false;
    const bool namesOut = m_personalInfoRemoved;
    m_ed->endTextEdit();
    m_ed->setDocument(std::move(doc), path);
    m_personalInfoRemoved = namesOut;
    m_ed->markUnsaved();
    updateTitle();
    statusBar()->showMessage(tr("Went back to \"%1\". Save the publication to keep it.").arg(which), 8000);
    return true;
}

void MainWindow::versionHistory()
{
    if (!m_ed->doc()) return;
    VersionHistoryHost host;
    host.kind = VersionHistoryHost::Kind::Publication;
    host.path = m_ed->filePath();
    host.extension = QStringLiteral(".jpub");
    host.keep = [this](const QString &name, QObject *, const std::function<void()> &refresh) {
        if (!saveVersion(name)) return false;
        refresh();
        return true;
    };
    host.openCopy = [this](const QString &copy) { openFile(copy, OpenHow::Copy); };
    host.restore = [this](const QString &file) { return restoreVersion(file); };
    showVersionHistory(this, host);
    if (m_backstage && m_backstage->isVisible()) showBackstage(QStringLiteral("info"));
}

} // namespace jp
