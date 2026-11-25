// JeffWrite's Info page: the document's properties, Mark as Final, a password, Inspect Document, and Check Compatibility. The page itself is libs/shell/app/infopage.cpp; this gives it the
// document's facts and does what its cards ask.
#include "app/writewindow.h"

#include "app/compatdialog.h"
#include "app/lossywarning.h"
#include "app/pageview.h"
#include "app/passworddialogs.h"
#include "app/plaintext.h"
#include "app/settings.h"
#include "core/appinfo.h"
#include "app/versionstore.h"
#include "core/commentops.h"
#include "core/inspectops.h"
#include "core/reviewops.h"
#include "core/mergeops.h"
#include "io/crypto.h"

#include <QApplication>
#include <QDateTime>
#include <QFileInfo>
#include <QStatusBar>
#include <memory>
#include <optional>

namespace jp {

QWidget *WriteWindow::buildInfoPage(QWidget *parent)
{
    InfoHost h;
    h.kind = InfoHost::Kind::Document;
    h.path = [this] { return m_path; };
    h.props = [this] {
        office::PackageProps p = m_doc->packageProps();
        return p;
    };
    h.setProps = [this](const office::PackageProps &p) { applyProperties(p); };
    h.statistics = [this] {
        const write::WriteDoc::TextCounts c = m_doc->textCounts();
        QVector<QPair<QString, QString>> rows;
        rows.push_back({tr("Pages"), QLocale().toString(std::max(1, m_view->pageCount()))});
        rows.push_back({tr("Words"), QLocale().toString(c.words)});
        rows.push_back({tr("Characters"), QLocale().toString(c.characters)});
        rows.push_back({tr("Characters (with spaces)"), QLocale().toString(c.charactersWithSpaces)});
        rows.push_back({tr("Paragraphs"), QLocale().toString(c.paragraphs)});
        return rows;
    };
    h.markFinalBlocked = [this] { return markFinalBlocked(); };
    h.markedFinal = [this] { return m_doc->markedAsFinal(); };
    h.setMarkedFinal = [this](bool on) { setMarkedFinal(on); };
    h.alwaysReadOnly = [this] { return alwaysOpensReadOnly(); };
    h.setAlwaysReadOnly = [this](bool on) { setAlwaysOpenReadOnly(on); };
    h.alwaysReadOnlyBlocked = [this] { return alwaysReadOnlyBlocked(); };
    h.hasPassword = [this] { return m_fileProtected; };
    h.encrypt = [this] { encryptWithPassword(); };
    h.inspect = [this] { inspectDocument(); };
    h.checkCompatibility = [this] { checkCompatibility(); };
    h.versionHistory = [this] { versionHistory(); };
    h.checkAccessibility = [this] {
        hideFileScreen();
        act(QStringLiteral("review.accessibility"))->trigger();
    };
    h.extraCards.push_back({QStringLiteral("objectsCard"), QStringLiteral("boxes"), tr("Objects"), tr("The pieces of other programs in the document: what each is, and opening, turning into a picture, or deleting it."), [this] {
        hideFileScreen();
        showObjects();
    }});
    h.thumbnail = [this] { return m_view->pageImage(0, 240); };
    return jp::buildInfoPage(parent, h);
}

void WriteWindow::applyProperties(const office::PackageProps &edited)
{
    if (!m_doc) return;
    office::PackageProps p = edited;
    const write::WriteDoc::TextCounts c = m_doc->textCounts();
    p.words = c.words;
    p.characters = c.characters;
    p.charactersWithSpaces = c.charactersWithSpaces;
    p.paragraphs = c.paragraphs;
    p.pages = std::max(1, m_view->pageCount());
    p.modified = QDateTime::currentDateTimeUtc();
    if (m_personalInfoRemoved) {
        // (the names stay out until the document is closed)
        p.creator.clear();
        p.lastModifiedBy.clear();
        p.company.clear();
        p.manager.clear();
    } else if (!Settings::get().userName().isEmpty()) {
        p.lastModifiedBy = Settings::get().userName();
    }
    if (!p.created.isValid()) p.created = p.modified;
    m_doc->setPackageProps(p);
    updateTitle();
}

QString WriteWindow::markFinalBlocked() const
{
    if (m_doc && m_doc->packageProps().truncated) return tr("This file has more properties than %1 reads, so the mark can't be added.").arg(appInfo().name);
    const QString ending = QFileInfo(m_path).suffix().toLower();
    if (ending == QLatin1String("odt") || ending == QLatin1String("ott") || ending == QLatin1String("fodt"))
        return tr("A .odt file can't hold the mark. Save the document as a .docx file first.");
    return {};
}

bool WriteWindow::alwaysOpensReadOnly() const
{
    return m_doc && office::isAlwaysOpenReadOnly(m_doc->packageProps().custom);
}

QString WriteWindow::alwaysReadOnlyBlocked() const
{
    if (m_doc && m_doc->packageProps().truncated) return tr("This file has more properties than %1 reads, so this choice can't be kept.").arg(appInfo().name);
    const QString ending = QFileInfo(m_path).suffix().toLower();
    if (ending == QLatin1String("odt") || ending == QLatin1String("ott") || ending == QLatin1String("fodt"))
        return tr("A .odt file can't hold this choice. Save the document as a .docx file first.");
    return {};
}

void WriteWindow::setAlwaysOpenReadOnly(bool on)
{
    if (!m_doc) return;
    if (on && !alwaysReadOnlyBlocked().isEmpty()) {
        notice(tr("Always Open Read-Only"), alwaysReadOnlyBlocked());
        return;
    }
    office::PackageProps p = m_doc->packageProps();
    p.custom = office::withAlwaysOpenReadOnly(p.custom, on);
    // (a window that is read-only for another reason, opened so or marked final, stays so)
    const bool keepReadOnly = isReadOnly() && !(!on && m_readOnlyByChoice);
    applyProperties(p);
    if (!on && m_readOnlyByChoice) setReadOnly(keepReadOnly);
    if (!on) m_readOnlyByChoice = false;
    statusBar()->showMessage(on ? tr("This document will open for reading only. Save it to keep this choice.") : tr("This document will open for editing again. Save it to keep this choice."), 8000);
    if (m_fileScreen) showFileScreen(QStringLiteral("info"));
}

int WriteWindow::removePersonalInformation()
{
    if (!m_doc) return 0;
    const int n = write::removePersonalInfo(*m_doc);
    m_personalInfoRemoved = true;
    return n;
}

void WriteWindow::setMarkedFinal(bool on)
{
    if (!m_doc) return;
    if (on) {
        const QString why = markFinalBlocked();
        if (!why.isEmpty()) {
            notice(tr("Mark as Final"), why);
            return;
        }
    }
    office::PackageProps p = m_doc->packageProps();
    p.custom = office::withMarkAsFinal(p.custom, on);
    if (on) {
        // The document is marked and saved (it is the saved file that is final), then held for reading only.
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
        // (saved as a kind of file that can't hold the mark: the save left it out, and the document is not final)
        if (!markFinalBlocked().isEmpty()) {
            p.custom = office::withMarkAsFinal(p.custom, false);
            applyProperties(p);
            statusBar()->showMessage(tr("The file that was saved can't hold the mark, so the document is not marked as final."), 8000);
            return;
        }
        setReadOnly(true);
        statusBar()->showMessage(tr("Marked as final and saved. It opens for reading only."), 8000);
    } else {
        applyProperties(p);
        setReadOnly(false);
        statusBar()->showMessage(tr("This document can be edited again. Save it to keep the mark off."), 8000);
    }
    if (m_fileScreen) showFileScreen(QStringLiteral("info"));
}

void WriteWindow::encryptWithPassword()
{
    if (!m_doc) return;
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
        statusBar()->showMessage(tr("The document is protected with a password when you save."), 8000);
    }
    crypto::wipe(password);
    m_doc->setModified(true);
    updateTitle();
    if (m_fileScreen) showFileScreen(QStringLiteral("info"));
}

QVector<InspectItem> WriteWindow::inspectItems()
{
    QVector<InspectItem> items;
    if (!m_doc) return items;
    write::WriteDoc *doc = m_doc.get();
    items.push_back({tr("Comments"), tr("Notes that people left in the margin, with their names."), [doc] { return int(write::comments(*doc).size()); }, [doc] { return write::deleteAllComments(*doc); }, true, QString()});
    items.push_back({tr("Tracked changes"), tr("Insertions and deletions that are still waiting to be accepted or rejected. Accept All keeps every change as part of the text; to choose, use the Review tab."),
                     [doc] { return write::documentRevisionCount(*doc); }, [doc] { return write::acceptAll(*doc); }, true, tr("Accept All")});
    items.push_back({tr("Document properties and personal information"), tr("The author, the person who last saved it, the company, and the manager."), [doc] { return write::personalInfoCount(*doc); },
                     [this] { return removePersonalInformation(); }, true, QString()});
    items.push_back({tr("Hidden text"), tr("Text that is set to be hidden: it does not show or print, but it is in the file."), [doc] { return write::hiddenTextCount(*doc); },
                     [doc] { return write::removeHiddenText(*doc); }, true, QString()});
    items.push_back({tr("Headers and footers"), tr("The text at the top and bottom of the pages."), [doc] { return write::headerFooterCount(*doc); },
                     [doc] { return write::clearHeadersAndFooters(*doc); }, true, QString()});
    items.push_back({tr("Custom XML data"), tr("Data that other programs added to the file, which you can't see on a page."), [doc] { return write::customXmlCount(*doc); },
                     [doc] { return write::removeCustomXml(*doc); }, true, QString()});
    // (the box asks for the count and then for the sentence: one look through the document serves both, and a removal makes the next look a new one)
    auto looked = std::make_shared<std::optional<write::ExternalLinkScan>>();
    items.push_back({tr("External links"), tr("Linked pictures and objects, the template the document is attached to, and fields that read another file. Removing turns fields into text, takes out linked pictures and objects, including those in comments and text boxes, and detaches the template. Links in comments and other kept parts can't be undone. Web links stay."),
                     [doc, looked] { *looked = write::scanExternalLinks(*doc); return (*looked)->count; }, [doc, looked] { looked->reset(); return write::removeExternalLinks(*doc); }, true, QString(),
                     [this, doc, looked] {
                         const write::ExternalLinkUnread u = looked->has_value() ? (*looked)->unread : write::scanExternalLinks(*doc).unread;
                         looked->reset();
                         QStringList said;
                         if (u.unresolved > 0) said << tr("%n linked picture(s) or object(s) can't be checked, because the file does not say where each one goes.", nullptr, u.unresolved);
                         if (u.unreachable > 0) said << tr("%n link(s) to another file are kept where Remove All can't take them out: inside other fields, comments, text boxes, and other parts of the file.", nullptr, u.unreachable);
                         QStringList names;
                         if (u.kinds & write::KindTemplate) names << tr("the attached template");
                         if (u.kinds & write::KindMailMerge) names << tr("the mail merge data source");
                         if (u.kinds & write::KindFrames) names << tr("frames");
                         if (u.kinds & write::KindSubDocument) names << tr("subdocuments");
                         if (u.kinds & write::KindChartData) names << tr("a chart's outside data");
                         if (u.kinds & write::KindMovieOrSound) names << tr("linked movies or sounds");
                         if (u.kinds & write::KindStyleSheet) names << tr("linked style sheets");
                         if (!names.isEmpty()) {
                             QString list = names.size() == 1 ? names.first() : names.size() == 2 ? tr("%1 and %2").arg(names[0], names[1]) : names.mid(0, names.size() - 1).join(QStringLiteral(", ")) + tr(", and %1").arg(names.last());
                             said << tr("The file keeps these, and Remove All can't take them out: %1.").arg(list);
                         }
                         if (u.pastLimit > 0) said << tr("%n more link(s) are past the most that one Remove All takes out. Run Remove All again to take them out.", nullptr, u.pastLimit);
                         return said.join(QLatin1Char(' '));
                     }});
    return items;
}

void WriteWindow::inspectDocument()
{
    if (!m_doc) return;
    InspectDialog dialog(tr("Inspect Document"), inspectItems(), this);
    dialog.exec();
    refreshNow();
    updateTitle();
}

// What an .odt file would leave out of the document, by the writer's own list (nothing is put on disk).
QStringList WriteWindow::incompatibilities()
{
    return odtAnswer().found;
}

CompatAnswer WriteWindow::odtAnswer()
{
    if (!m_doc) return CompatAnswer::failed(tr("There is no document to check."));
    QString err;
    QStringList notKept;
    const QByteArray bytes = m_doc->toOdt(&err, &notKept);
    if (bytes.isEmpty()) return CompatAnswer::failed(err);
    return withMergeExclusions(notKept);
}

// The recipients left out of a mail merge are kept only in a .docx file.
CompatAnswer WriteWindow::withMergeExclusions(CompatAnswer answer)
{
    const write::mail::State *merge = m_doc ? write::mail::stateIfAny(*m_doc) : nullptr;
    if (merge && !merge->exclusions.isEmpty()) answer.found << tr("Which recipients are left out of the mail merge.");
    return answer;
}

// The kinds Check Compatibility offers: each one a way to save that is not a .docx, with the list its own writer gives.
jp::CompatHost WriteWindow::compatHost()
{
    CompatHost host;
    host.markedFinal = [this] { return m_doc && m_doc->markedAsFinal(); };
    host.alwaysReadOnly = [this] { return alwaysOpensReadOnly(); };
    const QString what = tr("document");
    host.formats.push_back({tr("OpenDocument text (.odt)"), QStringLiteral(".odt"), QStringLiteral(".docx"), what, [this](const CompatDone &done) { done(odtAnswer()); }});
    host.formats.push_back({tr("Rich Text Format (.rtf)"), QStringLiteral(".rtf"), QStringLiteral(".docx"), what, [this](const CompatDone &done) {
        if (!m_doc) return done(CompatAnswer::failed(tr("There is no document to check.")));
        QStringList kept;
        QString err;
        if (m_doc->toRtf(&err, &kept).isEmpty()) return done(CompatAnswer::failed(err));
        CompatAnswer answer = withMergeExclusions(CompatAnswer(kept));
        if (m_fileProtected) answer.cannotSave = tr("This document is protected with a password, and a .rtf file can't hold one. Save it as a .docx file, or remove the password first (File > Info).");
        done(answer);
    }});
    host.formats.push_back({tr("Plain text (.txt)"), QStringLiteral(".txt"), QStringLiteral(".docx"), what, [this](const CompatDone &done) {
        QStringList kept;
        if (m_doc) {
            // (what a plain text file keeps: the words of the text, the words in tables one after another, and the words a link, bookmark, tracked change, or content control holds)
            kept << tr("The fonts, colors, and other formatting.");
            kept << tr("Pictures, charts, and equations.");
            kept << tr("Tables as tables (the words in them stay, one after another).");
            kept << tr("Headers and footers, with the words in them.");
            kept << tr("Comments and footnotes, with the words in them.");
            kept << tr("Bookmarks, and links to them or to web pages (the words stay).");
            kept << tr("Tracked changes and content controls (the words stay).");
            // (only when a section asks for a tray: a document with none has nothing of it to lose)
            for (int i = 0; i < m_doc->sectionCount(); ++i)
                if (m_doc->flow().section(i).firstPageTray || m_doc->flow().section(i).otherPageTray) {
                    kept << tr("Printer trays for the first page and the other pages.");
                    break;
                }
        }
        CompatAnswer answer = withMergeExclusions(CompatAnswer(kept));
        if (m_fileProtected) answer.cannotSave = tr("This document is protected with a password, and a plain text file can't hold one. Save it as a .docx file, or remove the password first (File > Info).");
        done(answer);
    }});
    return host;
}

void WriteWindow::checkCompatibility(int format)
{
    if (!m_doc) return;
    std::function<void(const QString &, const QString &, bool)> say;
    if (m_notifier) say = [this](const QString &title, const QString &text, bool warn) { notice(title, text, warn); };
    showCompatibility(this, compatHost(), format, say);
}

} // namespace jp
