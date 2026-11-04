// The document's review settings: whether Track Changes is on, who reviews, how changes are shown, and the Ids of new changes.

#include "core/writedoc.h"

#include "core/commentops.h"
#include "core/reviewops.h"
#include "core/writeprops.h"
#include "text/textprops.h"

#include <QTextBlock>
#include <QTextDocument>

#include <atomic>

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

void WriteDoc::setProtection(const std::optional<office::wml::DocumentProtection> &protection)
{
    const bool same = m_protection.has_value() == protection.has_value() && (!protection || (m_protection->xml == protection->xml));
    if (same) return;
    m_protection = protection;
    setModified(true);
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

namespace { std::atomic<qint64> s_reviewerProbes{0}; }
qint64 reviewerProbesForTest() { return s_reviewerProbes.load(); }

int WriteDoc::reviewerIndex(const QString &author)
{
    ++s_reviewerProbes;
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
        auto lookThrough = [&](QTextDocument *d) {
            for (QTextBlock b = d->begin(); b.isValid(); b = b.next()) {
                const QTextBlockFormat bf = b.blockFormat();
                if (bf.hasProperty(prop::RawPPr)) note(storedBytes(bf, prop::RawPPr));
                if (b.charFormat().hasProperty(prop::RawRPr)) note(storedBytes(b.charFormat(), prop::RawRPr));
                for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
                    const QTextCharFormat cf = it.fragment().charFormat();
                    if (cf.hasProperty(prop::RawRPr)) note(storedBytes(cf, prop::RawRPr));
                    if (cf.hasProperty(prop::Atom)) {
                        const Inline a = unpackAtom(storedBytes(cf, prop::Atom));
                        if (a.kind == Inline::ParaAtom) {
                            bool ok = false;
                            const qint64 v = a.key.toLongLong(&ok);
                            if (ok && v > most && v < 2000000000) most = v;
                        }
                        note(a.raw);
                    }
                }
            }
        };
        lookThrough(story());
        // Every other story the program has made of a part, the parts as they were read (headers, footers, notes, comments), and every comment.
        for (const HeaderPart &part : std::as_const(m_headerParts))
            if (part.story && part.editForm) lookThrough(part.story.get());
        for (const KeptPart &part : std::as_const(m_wml.parts)) {
            const QString n = part.name.toLower();
            if (n.contains(QLatin1String("header")) || n.contains(QLatin1String("footer")) || n.contains(QLatin1String("footnotes")) || n.contains(QLatin1String("endnotes")) || n.contains(QLatin1String("comments")))
                note(part.data);
        }
        for (const int id : allCommentIds(*this))
            if (id > most && id < 2000000000) most = id;
        m_nextRevisionId = int(most) + 1;
    }
    return m_nextRevisionId++;
}

QVector<QTextDocument *> WriteDoc::storiesWithChanges()
{
    auto marked = [](const QByteArray &x) {
        static const char *const names[] = {"<w:ins", "<w:del", "<w:moveFrom", "<w:moveTo", "rPrChange", "pPrChange"};
        for (const char *n : names)
            if (x.contains(n)) return true;
        return false;
    };
    QVector<QTextDocument *> out;
    QStringList names = m_headerParts.keys();
    names.sort();
    for (const QString &name : std::as_const(names)) {
        HeaderPart &part = m_headerParts[name];
        if (part.unreadable || part.missing || part.created) {
            if (!(part.created && part.editForm && part.story)) continue;
        }
        if (!part.editForm && !marked(partData(part.name))) continue;
        if (!makeEditable(part) || !part.story) continue;
        out.push_back(part.story.get());
    }
    QStringList notes = m_noteBlocks.keys();
    notes.sort();
    for (const QString &key : std::as_const(notes)) {
        bool has = false;
        for (const Block &b : m_noteBlocks.value(key))
            if (!b.isRaw && marked(office::wml::blockXml(b))) {
                has = true;
                break;
            }
        if (!has) continue;
        if (QTextDocument *n = editableFootnote(key.mid(2).toInt())) out.push_back(n);
    }
    return out;
}

void WriteDoc::setRevisionView(int view)
{
    if (view == m_revisionView) return;
    m_revisionView = view;
    m_flow->env.revisionView = view | m_fieldCodeView;
    story()->setProperty(tp::kRevisionViewProperty, view | m_fieldCodeView);
    if (m_pager) m_pager->invalidateFrom(0);
    Q_EMIT sectionsChanged();
    Q_EMIT reviewChanged();
}

void WriteDoc::setFieldCodes(bool all, const QSet<int> &ids)
{
    if (all == m_fieldCodesAll && ids == m_fieldCodeIds) return;
    // (The token in the view's high bits is this document's own for as long as it shows codes singly, and is new with each change, so that what was laid out for the last view is laid out again.)
    static int s_lastToken = 0;
    if (m_fieldCodesCount > 0) tp::shownFieldCodeSets().remove(m_fieldCodesCount);
    m_fieldCodesAll = all;
    m_fieldCodeIds = ids;
    m_fieldCodesCount = 0;
    m_fieldCodeView = 0;
    if (all || !ids.isEmpty()) {
        s_lastToken = s_lastToken % 4000000 + 1;
        m_fieldCodesCount = s_lastToken;
        if (!ids.isEmpty()) tp::shownFieldCodeSets().insert(m_fieldCodesCount, ids);
        m_fieldCodeView = (all ? tp::kShowFieldCodes : 0) | (m_fieldCodesCount << tp::kFieldCodesCountShift);
    }
    m_flow->env.revisionView = m_revisionView | m_fieldCodeView;
    story()->setProperty(tp::kRevisionViewProperty, m_revisionView | m_fieldCodeView);
    if (m_pager) m_pager->invalidateFrom(0);
    Q_EMIT sectionsChanged();
}

// Track Changes is a setting of the document: written when it differs from the file's.
void WriteDoc::writeReviewSettings(office::wml::Document *out)
{
    // The protection is written first, into the settings part, when it is not the file's.
    const bool protectionChanged = m_protection.has_value() != m_protectionAtOpen.has_value() || (m_protection && m_protection->xml != m_protectionAtOpen->xml);
    if (protectionChanged) {
        bool done = false;
        if (!out->settingsName.isEmpty()) {
            for (KeptPart &kept : out->parts)
                if (kept.name.compare(out->settingsName, Qt::CaseInsensitive) == 0) {
                    kept.data = settingsWithDocumentProtection(kept.data, m_protection);
                    done = true;
                }
        }
        if (!done && m_protection) {
            KeptPart kept;
            kept.name = QStringLiteral("/word/settings.xml");
            for (int n = 2; partExists(kept.name) || kept.name.compare(out->mainName, Qt::CaseInsensitive) == 0; ++n) kept.name = QStringLiteral("/word/settings%1.xml").arg(n);
            kept.contentType = QString(kSettingsType);
            kept.data = settingsWithDocumentProtection(QByteArray(), m_protection);
            out->parts.push_back(kept);
            Relationship r;
            r.id = nextRelationshipId(out->mainRelationships);
            r.type = QString(kSettingsRel);
            r.target = kept.name;
            out->mainRelationships.push_back(r);
            out->settingsName = kept.name;
        }
    }
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
