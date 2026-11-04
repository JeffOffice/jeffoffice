// JeffCrunch's Info page: the workbook's properties, Mark as Final, the password, and Inspect Workbook. The page itself is libs/shell/app/infopage.cpp; this gives it the workbook's facts and
// does what its cards ask. The properties a person edits are kept until the workbook is saved, and written then (libs/office/sheet/xlsxsave.cpp).
#include "app/crunchwindow.h"

#include "core/appinfo.h"
#include "app/settings.h"

#include <QDateTime>
#include <QRegularExpression>
#include <QStatusBar>

namespace jp {

office::PackageProps CrunchWindow::properties() const
{
    if (m_editedProps) return *m_editedProps;
    if (m_loaded && m_loaded->book) return office::readPackageProps(m_loaded->book->package());
    return {};
}

void CrunchWindow::applyProperties(const office::PackageProps &edited)
{
    if (!m_session) return;
    office::PackageProps p = edited;
    p.modified = QDateTime::currentDateTimeUtc();
    if (!Settings::get().userName().isEmpty()) p.lastModifiedBy = Settings::get().userName();
    if (!p.created.isValid()) p.created = p.modified;
    m_editedProps = p;
    // (a change of the file's properties is a change to save, as a password is)
    m_protectionChanged = true;
    setWindowModified(true);
    updateEditActions();
}

QWidget *CrunchWindow::buildInfoPage(QWidget *parent)
{
    InfoHost h;
    h.kind = InfoHost::Kind::Workbook;
    h.path = [this] { return m_path; };
    h.props = [this] { return properties(); };
    h.setProps = [this](const office::PackageProps &p) { applyProperties(p); };
    h.statistics = [this] {
        QVector<QPair<QString, QString>> rows;
        if (m_loaded && m_loaded->book) {
            int sheets = 0;
            for (const auto &s : m_loaded->book->info().sheets)
                if (!s.deleted) ++sheets;
            rows.push_back({tr("Sheets"), QLocale().toString(sheets)});
        }
        return rows;
    };
    h.markedFinal = [this] { return markedAsFinal(); };
    h.setMarkedFinal = [this](bool on) { setMarkedFinal(on); };
    h.hasPassword = [this] { return hasUserPassword(); };
    h.encrypt = [this] { protectWithPassword(); };
    h.inspect = [this] { inspectWorkbook(); };
    return jp::buildInfoPage(parent, h);
}

void CrunchWindow::setMarkedFinal(bool on)
{
    if (!m_session || m_saving) return;
    office::PackageProps p = properties();
    const office::PackageProps before = p;
    p.custom = office::withMarkAsFinal(p.custom, on);
    if (!on) {
        applyProperties(p);
        setReadOnly(false);
        setStatus(tr("This workbook can be edited again. Save it to keep the mark off."));
        if (m_fileScreen) showFileScreen(QStringLiteral("info"));
        return;
    }
    // Marked, then saved (it is the saved file that is final), then held for reading only.
    if (isReadOnly()) setReadOnly(false);
    applyProperties(p);
    auto link = std::make_shared<QMetaObject::Connection>();
    *link = connect(this, &CrunchWindow::saved, this, [this, link, before](const QString &, bool ok) {
        disconnect(*link);
        if (ok) {
            setReadOnly(true);
            setStatus(tr("Marked as final and saved. It opens for reading only."));
            if (m_fileScreen) showFileScreen(QStringLiteral("info"));
        } else {
            applyProperties(before);   // (not saved: it is not final)
        }
    });
    if (!save()) {
        disconnect(*link);
        applyProperties(before);
    }
}

QVector<InspectItem> CrunchWindow::inspectItems()
{
    QVector<InspectItem> items;
    if (!m_session || !m_loaded || !m_loaded->book) return items;
    auto book = m_loaded->book;
    crunch::EditSession *session = m_session.get();
    // Notes
    auto noteCount = [this, session, book] {
        int n = 0;
        for (int i = 0; i < book->info().sheets.size(); ++i) {
            if (book->info().sheets[i].deleted) continue;
            if (const auto notes = session->notesOf(i)) n += int(notes->size());
        }
        return n;
    };
    auto removeNotes = [this, session, book] {
        int n = 0;
        for (int i = 0; i < book->info().sheets.size(); ++i) {
            if (book->info().sheets[i].deleted || !session->canEditNotes(i)) continue;
            const auto notes = session->notesOf(i);
            if (!notes || notes->empty()) continue;
            std::vector<std::pair<quint32, quint32>> cells;
            for (const auto &note : *notes) cells.push_back({quint32(note.first >> 32), quint32(note.first & 0xFFFFFFFFu)});
            if (session->deleteNotes(i, cells, tr("Remove Notes")).ok) n += int(cells.size());
        }
        updateEditActions();
        return n;
    };
    items.push_back({tr("Notes"), tr("Notes that people left on cells, with their names."), noteCount, removeNotes, true, QString()});
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
                         if (n > 0) applyProperties(p);
                         // (applying the properties puts the name of whoever saves in; the mark of a clean-up is that nobody is named)
                         if (n > 0) {
                             office::PackageProps q = *m_editedProps;
                             q.lastModifiedBy.clear();
                             m_editedProps = q;
                         }
                         return n;
                     },
                     true, QString()});
    items.push_back({tr("Hidden sheets"), tr("Sheets that are set to be hidden: they do not show as tabs, but they are in the file. Show them to see what they hold."),
                     [session, book] {
                         int n = 0;
                         for (const auto &s : book->info().sheets)
                             if (!s.deleted && s.state != office::sheet::SheetState::Visible) ++n;
                         return n;
                     },
                     [this, session, book] {
                         int n = 0;
                         for (int i = 0; i < book->info().sheets.size(); ++i) {
                             const auto &s = book->info().sheets[i];
                             if (s.deleted || s.state == office::sheet::SheetState::Visible) continue;
                             if (session->setSheetHidden(i, false).ok) ++n;
                         }
                         fillTabs();
                         return n;
                     },
                     true, tr("Show All")});
    const auto partsMatching = [book](const QString &pattern) {
        const QRegularExpression re(pattern, QRegularExpression::CaseInsensitiveOption);
        int n = 0;
        for (const QString &name : book->package().partNames())
            if (re.match(name).hasMatch()) ++n;
        return n;
    };
    items.push_back({tr("External links"), tr("Links to values in other workbooks. Break them on the Data tab."), [partsMatching] { return partsMatching(QStringLiteral("^/xl/externalLinks/externalLink[0-9]*\\.xml$")); },
                     nullptr, false, QString()});
    items.push_back({tr("Custom XML data"), tr("Data that other programs added to the file, which you can't see on a sheet."), [partsMatching] { return partsMatching(QStringLiteral("^/customXml/item[0-9]*\\.xml$")); },
                     nullptr, false, QString()});
    return items;
}

void CrunchWindow::inspectWorkbook()
{
    if (!m_session) return;
    InspectDialog dialog(tr("Inspect Workbook"), inspectItems(), this);
    dialog.exec();
}

} // namespace jp
