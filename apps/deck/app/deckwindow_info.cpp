// JeffDeck's Info page: the presentation's properties, Mark as Final, a password, Inspect Presentation, and Version History. The page itself is libs/shell/app/infopage.cpp; this gives it the
// presentation's facts and does what its cards ask. The properties are kept in the presentation (so Undo covers them) and written to the file's properties when it is saved.
#include "app/deckwindow.h"

#include "app/deckcanvas.h"
#include "app/deckfilescreen.h"
#include "app/editor.h"
#include "app/notespane.h"
#include "app/passworddialogs.h"
#include "app/plaintext.h"
#include "app/settings.h"
#include "app/versionhistory.h"
#include "canvas/canvas.h"
#include "core/appinfo.h"
#include "core/comments.h"
#include "core/deck.h"
#include "io/crypto.h"
#include "io/deckoutput.h"
#include "io/pptxexport.h"
#include "io/pptximport.h"
#include "io/presentationfile.h"
#include "office/officecrypt.h"
#include "office/package.h"

#include <QApplication>
#include <QDateTime>
#include <algorithm>
#include <QFileInfo>
#include <QLocale>
#include <QStatusBar>

namespace jp {

void DeckWindow::notice(const QString &title, const QString &text, bool warning)
{
    if (m_notifier) m_notifier(title, text);
    else if (warning) jp::plainWarning(this, title, text);
    else jp::plainInformation(this, title, text);
}

office::PackageProps DeckWindow::properties() const
{
    const Document *d = m_ed->doc();
    return d ? d->props.toPackageProps() : office::PackageProps();
}

void DeckWindow::applyProperties(const office::PackageProps &edited)
{
    Document *d = m_ed->doc();
    if (!d) return;
    office::PackageProps p = edited;
    p.modified = QDateTime::currentDateTimeUtc();
    if (m_personalInfoRemoved) {
        // (the names stay out until the presentation is closed)
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

bool DeckWindow::markedAsFinal() const
{
    return office::isMarkedFinal(properties().custom);
}

QWidget *DeckWindow::buildInfoPage(QWidget *parent)
{
    InfoHost h;
    h.kind = InfoHost::Kind::Presentation;
    h.path = [this] { return m_ed->filePath(); };
    h.props = [this] { return properties(); };
    h.setProps = [this](const office::PackageProps &p) { applyProperties(p); };
    h.statistics = [this] {
        const Document *d = m_ed->doc();
        QVector<QPair<QString, QString>> rows;
        if (!d) return rows;
        int hidden = 0, withNotes = 0;
        for (const auto &page : d->pages) {
            hidden += page->hidden;
            withNotes += !deck::notesText(*d, *page).trimmed().isEmpty();
        }
        rows.push_back({tr("Slides"), QLocale().toString(int(d->pages.size()))});
        rows.push_back({tr("Hidden slides"), QLocale().toString(hidden)});
        rows.push_back({tr("Slides with notes"), QLocale().toString(withNotes)});
        return rows;
    };
    h.markFinalBlocked = [this] { return markFinalBlocked(); };
    h.markedFinal = [this] { return markedAsFinal(); };
    h.setMarkedFinal = [this](bool on) { setMarkedFinal(on); };
    h.hasPassword = [this] { return m_fileProtected; };
    h.encrypt = [this] { encryptWithPassword(); };
    h.inspect = [this] { inspectPresentation(); };
    h.versionHistory = [this] { versionHistory(); };
    h.thumbnail = [this] {
        const Document *d = m_ed->doc();
        return d && !d->pages.isEmpty() ? deck::slideImage(*d, &m_ed->cache(), 0, 480) : QImage();
    };
    return jp::buildInfoPage(parent, h);
}

QString DeckWindow::markFinalBlocked() const
{
    if (properties().truncated) return tr("This file has more properties than %1 reads, so the mark can't be added.").arg(appInfo().name);
    if (deck::formatOfFileName(m_ed->filePath()) == deck::FileFormat::Odp && !m_ed->filePath().isEmpty())
        return tr("An .odp file can't hold the mark. Save the presentation as a .pptx file first.");
    return {};
}

void DeckWindow::setMarkedFinal(bool on)
{
    if (!m_ed->doc() || m_ed->doc()->pages.isEmpty()) return;
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
        // The presentation is marked and saved (it is the saved file that is final), then held for reading only.
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
        // (saved as a kind of file that can't hold the mark: the save left it out, and the presentation is not final)
        if (!markedAsFinal()) {
            statusBar()->showMessage(tr("The file that was saved can't hold the mark, so the presentation is not marked as final."), 8000);
            return;
        }
        setReadOnly(true);
        statusBar()->showMessage(tr("Marked as final and saved. It opens for reading only."), 8000);
    } else {
        // (the editor changes nothing while it is read-only)
        setReadOnly(false);
        applyProperties(p);
        statusBar()->showMessage(tr("This presentation can be edited again. Save it to keep the mark off."), 8000);
    }
    if (m_fileScreen) showFileScreen(QStringLiteral("info"));
}

void DeckWindow::encryptWithPassword()
{
    if (!m_ed->doc()) return;
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
        dropRecoveryCopy();   // (a copy being made, or made, holds the text in the clear)
        statusBar()->showMessage(tr("The presentation is protected with a password when you save."), 8000);
    }
    crypto::wipe(password);
    // (a password is a change to save, as the properties are)
    m_ed->markUnsaved();
    updateTitle();
    if (m_fileScreen) showFileScreen(QStringLiteral("info"));
}

QVector<InspectItem> DeckWindow::inspectItems()
{
    QVector<InspectItem> items;
    if (!m_ed->doc()) return items;
    Editor *ed = m_ed;
    auto doc = [ed] { return ed->doc(); };
    items.push_back({tr("Comments"), tr("Notes that people left on the slides, with their names."),
                     [doc] {
                         int n = 0;
                         for (const auto &page : doc()->pages) n += deck::commentCount(*page);
                         return n;
                     },
                     [ed] {
                         int n = 0;
                         ed->change(tr("Delete Comments"), [&] { n = deck::deleteAllComments(*ed->doc()); });
                         return n;
                     },
                     true, QString()});
    items.push_back({tr("Speaker notes"), tr("The notes you wrote for the person who gives the talk. They are in the file, and anyone who gets it can read them."),
                     [doc] {
                         int n = 0;
                         for (const auto &page : doc()->pages) n += !deck::notesText(*doc(), *page).trimmed().isEmpty();
                         return n;
                     },
                     [this, ed] {
                         m_notes->commit();
                         int n = 0;
                         ed->change(tr("Delete Speaker Notes"), [&] {
                             Document *d = ed->doc();
                             for (const auto &page : d->pages) {
                                 if (deck::notesText(*d, *page).trimmed().isEmpty()) continue;
                                 if (QTextDocument *story = d->storyDoc(page->notesStoryId)) story->clear();
                                 ++n;
                             }
                         });
                         return n;
                     },
                     true, QString()});
    items.push_back({tr("Document properties and personal information"), tr("The author, the person who last saved it, the company, and the manager."),
                     [this] {
                         const office::PackageProps p = properties();
                         int n = 0;
                         for (const QString &s : {p.creator, p.lastModifiedBy, p.company, p.manager})
                             if (!s.isEmpty()) ++n;
                         return n;
                     },
                     [this] {
                         office::PackageProps p = properties();
                         int n = 0;
                         for (QString *s : {&p.creator, &p.lastModifiedBy, &p.company, &p.manager})
                             if (!s->isEmpty()) {
                                 s->clear();
                                 ++n;
                             }
                         m_personalInfoRemoved = true;
                         if (n > 0) applyProperties(p);
                         return n;
                     },
                     true, QString()});
    items.push_back({tr("Hidden slides"), tr("Slides that are set to be hidden: the slide show skips them, but they are in the file. Show them to see what they hold."),
                     [doc] {
                         int n = 0;
                         for (const auto &page : doc()->pages) n += page->hidden;
                         return n;
                     },
                     [ed] {
                         int n = 0;
                         ed->change(tr("Show Hidden Slides"), [&] {
                             for (const auto &page : ed->doc()->pages)
                                 if (page->hidden) {
                                     page->hidden = false;
                                     ++n;
                                 }
                         });
                         return n;
                     },
                     true, tr("Show All")});
    // What lies off the slide is not shown in the show or on paper, but it is in the file.
    const auto isOff = [](const Document &d, const ItemPtr &it) { return !it->bounds().intersects(QRectF(QPointF(0, 0), d.pageSize())); };
    items.push_back({tr("Content off the slides"), tr("Text, pictures, and shapes that lie outside a slide: they do not show in the slide show or on paper, but they are in the file."),
                     [doc, isOff] {
                         int n = 0;
                         for (const auto &page : doc()->pages) n += int(std::count_if(page->items.begin(), page->items.end(), [&](const ItemPtr &it) { return isOff(*doc(), it); }));
                         return n;
                     },
                     [ed, isOff] {
                         int n = 0;
                         ed->clearSelection();
                         ed->change(tr("Delete Content off the Slides"), [&] {
                             for (const auto &page : ed->doc()->pages) {
                                 const size_t before = page->items.size();
                                 page->items.erase(std::remove_if(page->items.begin(), page->items.end(), [&](const ItemPtr &it) { return isOff(*ed->doc(), it); }), page->items.end());
                                 n += int(before - page->items.size());
                             }
                         });
                         return n;
                     },
                     true, QString()});
    return items;
}

void DeckWindow::inspectPresentation()
{
    if (!m_ed->doc()) return;
    m_notes->commit();
    InspectDialog dialog(tr("Inspect Presentation"), inspectItems(), this);
    dialog.exec();
    updateTitle();
}

// ---- Version History

QVector<write::DocVersion> DeckWindow::versions() const
{
    if (m_ed->filePath().isEmpty()) return {};
    return write::VersionStore(m_ed->filePath(), QStringLiteral(".pptx")).list();
}

bool DeckWindow::saveVersion(const QString &name)
{
    Document *d = m_ed->doc();
    if (!d) return false;
    const QString path = m_ed->filePath();
    if (path.isEmpty()) {
        notice(tr("Version History"), versionsNeedAFile(VersionHistoryHost::Kind::Presentation), true);
        return false;
    }
    m_notes->commit();
    m_ed->endTextEdit();
    QString err;
    QByteArray bytes = deck::exportPresentation(*d, &err);
    // (a version of a presentation that asks for a password asks for it too)
    const QString kept = m_versions.forNew(m_fileProtected ? m_filePassword : QString());
    if (!bytes.isEmpty() && !kept.isEmpty()) bytes = office::encryptPackage(bytes, kept);
    if (bytes.isEmpty()) {
        notice(tr("Version History"), tr("The version couldn't be kept.\n%1").arg(err), true);
        return false;
    }
    write::VersionStore store(path, QStringLiteral(".pptx"));
    if (!store.add(name.trimmed().isEmpty() ? tr("Version %1").arg(store.list().size() + 1) : name, bytes, &err)) {
        notice(tr("Version History"), tr("The version couldn't be kept.\n%1").arg(err), true);
        return false;
    }
    statusBar()->showMessage(tr("Kept a version of %1.").arg(m_ed->displayName()), 4000);
    return true;
}

bool DeckWindow::restoreVersion(const QString &file)
{
    if (!m_ed->doc() || m_ed->filePath().isEmpty()) return false;
    const QString path = m_ed->filePath();
    write::VersionStore store(path, QStringLiteral(".pptx"));
    QByteArray zip;
    QString err;
    std::unique_ptr<Document> doc;
    if (m_versions.open(store.read(file), m_fileProtected ? m_filePassword : QString(), &zip) && !zip.isEmpty()) doc = deck::importPresentation(zip, &err);
    if (!doc) {
        notice(tr("Version History"), tr("That version can't be opened.\n%1").arg(err), true);
        return false;
    }
    QString which = file;
    for (const write::DocVersion &v : store.list())
        if (v.file == file) which = v.name;
    // What the presentation is now is kept first: going back loses nothing.
    if (!saveVersion(tr("Before restoring \"%1\"").arg(which))) return false;
    // (a new presentation in the window clears the password and the names kept out: neither is lost by going back)
    const bool protectedNow = m_fileProtected;
    QString password = m_filePassword;
    const QString keptWith = m_versions.kept;
    const bool namesOut = m_personalInfoRemoved;
    m_ed->endTextEdit();
    m_ed->setDocument(std::move(doc), path);
    m_fileProtected = protectedNow;
    m_filePassword = password;
    m_versions.reset(keptWith);
    crypto::wipe(password);
    m_personalInfoRemoved = namesOut;
    m_ed->markUnsaved();
    m_canvas->zoomToFit(Canvas::Fit::WholePage);
    updateTitle();
    statusBar()->showMessage(tr("Went back to \"%1\". Save the presentation to keep it.").arg(which), 8000);
    return true;
}

void DeckWindow::versionHistory()
{
    if (!m_ed->doc()) return;
    VersionHistoryHost host;
    host.kind = VersionHistoryHost::Kind::Presentation;
    host.path = m_ed->filePath();
    host.extension = QStringLiteral(".pptx");
    host.keep = [this](const QString &name, QObject *, const std::function<void()> &refresh) {
        if (!saveVersion(name)) return false;
        refresh();
        return true;
    };
    host.openCopy = [this](const QString &copy) { openFile(copy, OpenHow::Copy); };
    host.restore = [this](const QString &file) { return restoreVersion(file); };
    showVersionHistory(this, host);
    if (m_fileScreen && m_fileScreen->isVisible()) showFileScreen(QStringLiteral("info"));
}

} // namespace jp
