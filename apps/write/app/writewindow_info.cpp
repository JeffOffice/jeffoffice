// JeffWrite's Info page: the document's properties, Mark as Final, a password, Inspect Document, and Check Compatibility. The page itself is libs/shell/app/infopage.cpp; this gives it the
// document's facts and does what its cards ask.
#include "app/writewindow.h"

#include "app/pageview.h"
#include "app/passworddialogs.h"
#include "app/plaintext.h"
#include "app/settings.h"
#include "core/appinfo.h"
#include "app/versionstore.h"
#include "core/commentops.h"
#include "core/inspectops.h"
#include "core/reviewops.h"
#include "io/crypto.h"

#include <QApplication>
#include <QDateTime>
#include <QFileInfo>
#include <QStatusBar>

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
    h.hasPassword = [this] { return m_fileProtected; };
    h.encrypt = [this] { encryptWithPassword(); };
    h.inspect = [this] { inspectDocument(); };
    h.checkCompatibility = [this] { checkCompatibility(); };
    h.versionHistory = [this] { versionHistory(); };
    h.checkAccessibility = [this] {
        hideFileScreen();
        act(QStringLiteral("review.accessibility"))->trigger();
    };
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
    // (the versions kept follow the password: none is left in the clear, and Go Back opens them with the new one)
    const QString before = m_fileProtected ? m_filePassword : QString();
    crypto::wipe(m_filePassword);
    if (choice == NewPasswordChoice::Removed) {
        m_fileProtected = false;
        statusBar()->showMessage(tr("The password is taken off when you save."), 8000);
    } else {
        m_fileProtected = true;
        m_filePassword = password;
        statusBar()->showMessage(tr("The document is protected with a password when you save."), 8000);
    }
    if (!m_path.isEmpty()) {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        const int failed = write::reprotectVersions(m_path, QStringLiteral(".docx"), before, m_fileProtected ? m_filePassword : QString());
        QApplication::restoreOverrideCursor();
        if (failed > 0) notice(tr("Version History"), tr("Some versions kept before couldn't be given the new password, and stay as they were."), true);
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
                     [doc] { return write::revisionCount(doc->story()); }, [doc] { return write::acceptAll(*doc); }, true, tr("Accept All")});
    items.push_back({tr("Document properties and personal information"), tr("The author, the person who last saved it, the company, and the manager."), [doc] { return write::personalInfoCount(*doc); },
                     [this] { return removePersonalInformation(); }, true, QString()});
    items.push_back({tr("Hidden text"), tr("Text that is set to be hidden: it does not show or print, but it is in the file."), [doc] { return write::hiddenTextCount(*doc); },
                     [doc] { return write::removeHiddenText(*doc); }, true, QString()});
    items.push_back({tr("Headers and footers"), tr("The text at the top and bottom of the pages."), [doc] { return write::headerFooterCount(*doc); },
                     [doc] { return write::clearHeadersAndFooters(*doc); }, true, QString()});
    items.push_back({tr("Custom XML data"), tr("Data that other programs added to the file, which you can't see on a page."), [doc] { return write::customXmlCount(*doc); },
                     [doc] { return write::removeCustomXml(*doc); }, true, QString()});
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

QStringList WriteWindow::incompatibilities()
{
    QStringList kept;
    if (!m_doc) return kept;
    QString err;
    QStringList notKept;
    const QByteArray bytes = m_doc->toOdt(&err, &notKept);
    if (bytes.isEmpty()) kept << err;
    else kept = notKept;
    if (m_fileProtected) kept << tr("The password: a .odt file can't hold a password yet.");
    return kept;
}

void WriteWindow::checkCompatibility()
{
    const QStringList kept = incompatibilities();
    if (kept.isEmpty()) notice(tr("Check Compatibility"), tr("Nothing in this document is lost when it is saved as an OpenDocument text file (.odt)."));
    else notice(tr("Check Compatibility"), tr("An OpenDocument text file (.odt) can't hold some of what this document has:\n\n%1").arg(kept.join(QLatin1Char('\n'))), true);
}

} // namespace jp
