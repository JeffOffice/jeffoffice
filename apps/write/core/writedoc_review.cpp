// The document's review settings: whether Track Changes is on, who reviews, how changes are shown, and the Ids of new changes.

#include "core/writedoc.h"

#include "core/writeprops.h"
#include "text/textprops.h"

#include <QTextBlock>
#include <QTextDocument>

using namespace jp::office;
using namespace jp::office::wml;

namespace jp::write {

namespace {
constexpr char kSettingsRel[] = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings";
constexpr char kSettingsType[] = "application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml";
}

void WriteDoc::setTrackChanges(bool on)
{
    if (m_trackChanges == on) return;
    m_trackChanges = on;
    Q_EMIT reviewChanged();
}

void WriteDoc::setReviewer(const QString &name, const QString &initials)
{
    const QString who = name.trimmed().isEmpty() ? QStringLiteral("Author") : name.trimmed();
    if (who == m_reviewer && initials == m_reviewerInitials) return;
    m_reviewer = who;
    m_reviewerInitials = initials;
    Q_EMIT reviewChanged();
}

int WriteDoc::reviewerIndex(const QString &author)
{
    const auto known = m_reviewerPlaces.constFind(author);
    if (known != m_reviewerPlaces.constEnd()) return known.value();
    m_reviewers.push_back(author);
    const int at = int(m_reviewers.size()) - 1;
    m_reviewerPlaces.insert(author, at);
    return at;
}

int WriteDoc::nextRevisionId()
{
    if (m_nextRevisionId <= 0) {
        // Above every Id that a mark of the text has (a change, a bookmark, a comment's range): one look through the story, the first time one is wanted.
        qint64 most = 0;
        auto note = [&](const QByteArray &bytes) {
            for (qsizetype at = bytes.indexOf(":id=\""); at >= 0; at = bytes.indexOf(":id=\"", at + 5)) {
                const qsizetype end = bytes.indexOf('"', at + 5);
                if (end < 0) break;
                bool ok = false;
                const qint64 v = bytes.mid(at + 5, end - at - 5).toLongLong(&ok);
                if (ok && v > most && v < 2000000000) most = v;
            }
        };
        for (QTextBlock b = story()->begin(); b.isValid(); b = b.next()) {
            const QTextBlockFormat bf = b.blockFormat();
            if (bf.hasProperty(prop::RawPPr)) note(storedBytes(bf, prop::RawPPr));
            for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
                const QTextCharFormat cf = it.fragment().charFormat();
                if (cf.hasProperty(prop::RawRPr)) note(storedBytes(cf, prop::RawRPr));
                if (cf.hasProperty(prop::Atom)) {
                    const Inline a = unpackAtom(storedBytes(cf, prop::Atom));
                    if (a.kind == Inline::ParaAtom) {
                        bool ok = false;
                        const qint64 v = a.key.toLongLong(&ok);
                        if (ok && v > most && v < 2000000000) most = v;
                        note(a.raw);
                    }
                }
            }
        }
        m_nextRevisionId = int(most) + 1;
    }
    return m_nextRevisionId++;
}

void WriteDoc::setRevisionView(int view)
{
    if (view == m_revisionView) return;
    m_revisionView = view;
    m_flow->env.revisionView = view;
    story()->setProperty(tp::kRevisionViewProperty, view);
    if (m_pager) m_pager->invalidateFrom(0);
    Q_EMIT sectionsChanged();
    Q_EMIT reviewChanged();
}

// Track Changes is a setting of the document: written when it differs from the file's.
void WriteDoc::writeReviewSettings(office::wml::Document *out)
{
    if (m_trackChanges == m_trackAtOpen) return;
    const bool on = m_trackChanges;
    if (!out->settingsName.isEmpty()) {
        for (KeptPart &kept : out->parts)
            if (kept.name.compare(out->settingsName, Qt::CaseInsensitive) == 0) {
                kept.data = settingsWithTrackRevisions(kept.data, on);
                return;
            }
    }
    if (!on) return;
    KeptPart kept;
    kept.name = QStringLiteral("/word/settings.xml");
    for (int n = 2; partExists(kept.name) || kept.name.compare(out->mainName, Qt::CaseInsensitive) == 0; ++n) kept.name = QStringLiteral("/word/settings%1.xml").arg(n);
    kept.contentType = QString(kSettingsType);
    kept.data = settingsWithTrackRevisions(QByteArray(), true);
    out->parts.push_back(kept);
    Relationship r;
    r.id = nextRelationshipId(out->mainRelationships);
    r.type = QString(kSettingsRel);
    r.target = kept.name;
    out->mainRelationships.push_back(r);
    out->settingsName = kept.name;
}

} // namespace jp::write
