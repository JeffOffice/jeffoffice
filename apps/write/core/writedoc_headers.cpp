// Headers and footers: the three slots of a section, making and linking them, putting a story in the form it is edited in, writing the parts back, and the one history of the text
// and the stories of its headers and footers.

#include "core/writedoc.h"

#include "core/tablemodel.h"
#include "core/writeprops.h"
#include "flow/paginator.h"

#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <algorithm>

using namespace jp::office;
using namespace jp::office::wml;

namespace jp::write {

namespace {
const QLatin1String kHeaderRel("http://schemas.openxmlformats.org/officeDocument/2006/relationships/header");
const QLatin1String kFooterRel("http://schemas.openxmlformats.org/officeDocument/2006/relationships/footer");
const QLatin1String kSettingsRel("http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings");
const QLatin1String kHeaderType("application/vnd.openxmlformats-officedocument.wordprocessingml.header+xml");
const QLatin1String kFooterType("application/vnd.openxmlformats-officedocument.wordprocessingml.footer+xml");
const QLatin1String kSettingsType("application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml");
} // namespace

// ---- the parts

WriteDoc::HeaderPart *WriteDoc::partOfStory(const QTextDocument *story)
{
    if (!story) return nullptr;
    for (auto it = m_headerParts.begin(); it != m_headerParts.end(); ++it)
        if (it->story.get() == story) return &it.value();
    return nullptr;
}

const WriteDoc::HeaderPart *WriteDoc::partOfStory(const QTextDocument *story) const
{
    return const_cast<WriteDoc *>(this)->partOfStory(story);
}

bool WriteDoc::partExists(const QString &name) const
{
    for (const KeptPart &p : m_wml.parts)
        if (p.name.compare(name, Qt::CaseInsensitive) == 0) return true;
    return false;
}

QString WriteDoc::newPartName(bool footer) const
{
    const QString stem = footer ? QStringLiteral("/word/footer%1.xml") : QStringLiteral("/word/header%1.xml");
    for (int n = 1;; ++n) {
        const QString name = stem.arg(n);
        bool taken = partExists(name) || name.compare(m_wml.mainName, Qt::CaseInsensitive) == 0;
        for (auto it = m_headerParts.cbegin(); !taken && it != m_headerParts.cend(); ++it) taken = it.key().compare(name, Qt::CaseInsensitive) == 0;
        for (const Relationship &r : m_wml.mainRelationships) taken = taken || (!r.external && r.target.compare(name, Qt::CaseInsensitive) == 0);
        if (!taken) return name;
    }
}

WriteDoc::HeaderPart *WriteDoc::addHeaderPart(bool footer, std::shared_ptr<QTextDocument> story, const QString &copyRelationshipsOf)
{
    HeaderPart part;
    part.name = newPartName(footer);
    part.footer = footer;
    part.story = std::move(story);
    part.created = true;
    part.modified = true;
    part.editForm = true;
    Relationship r;
    r.id = nextRelationshipId(m_wml.mainRelationships);
    r.type = footer ? QString(kFooterRel) : QString(kHeaderRel);
    r.target = part.name;
    m_wml.mainRelationships.push_back(r);
    part.relationship = r.id;
    if (!copyRelationshipsOf.isEmpty()) {
        // (A copy of a part that has pictures refers to the same pictures through relationships of its own.)
        const QVector<Relationship> rels = m_wml.partRelationships.value(copyRelationshipsOf);
        if (!rels.isEmpty()) m_wml.partRelationships.insert(part.name, rels);
    }
    return &m_headerParts.insert(part.name, part).value();
}

void WriteDoc::finishEditable(HeaderPart &part)
{
    QTextDocument *s = part.story.get();
    flow::watchEdits(s);
    s->setUndoRedoEnabled(true);
    s->setModified(false);
    part.editForm = true;
    watchHistory(s);
}

bool WriteDoc::makeEditable(HeaderPart &part)
{
    if (part.editForm) return true;
    if (part.unreadable || part.missing || part.created) return false;
    QVector<Block> blocks;
    QByteArray root;
    if (!readBlocks(partData(part.name), &blocks, true, &root)) return false;
    QTextDocument *s = part.story.get();
    // (The pages keep the room that the story leaves the text: it is the same story, and the pages stay as they are unless the new form takes another room.)
    if (m_pager) m_pager->watchFurniture(s);
    s->clear();
    s->setDocumentMargin(0);
    s->setDefaultFont(story()->defaultFont());
    s->setUndoRedoEnabled(false);
    part.rootStart = root;
    fillStory(s, blocks, true, part.name, true, false, -1, true);
    finishEditable(part);
    if (m_pager) m_pager->furnitureChanged(s);
    Q_EMIT headerFooterChanged(s);
    return true;
}

QTextDocument *WriteDoc::editableHeaderFooter(int section, bool footer, int slot)
{
    QTextDocument *s = headerFooterStory(section, footer, slot);
    HeaderPart *part = partOfStory(s);
    if (!part || !makeEditable(*part)) return nullptr;
    return s;
}

// ---- the slots

QTextDocument *WriteDoc::headerFooterStory(int section, bool footer, int slot) const
{
    if (section < 0 || section >= m_flow->sections.size()) return nullptr;
    const flow::Section &s = m_flow->sections[section];
    return (footer ? s.footer : s.header).slot(std::clamp(slot, 0, 2)).get();
}

bool WriteDoc::hasOwnHeaderFooter(int section, bool footer, int slot) const
{
    if (section < 0 || section >= int(m_sections.size())) return false;
    const HeaderRefs refs = headerRefsOf(m_sections[section]);
    const int at = std::clamp(slot, 0, 2);
    return footer ? !refs.footer[at].isEmpty() : !refs.header[at].isEmpty();
}

bool WriteDoc::linkedToPrevious(int section, bool footer, int slot) const
{
    return section > 0 && section < int(m_sections.size()) && !hasOwnHeaderFooter(section, footer, slot);
}

int WriteDoc::headerSlotOf(int section, bool firstOfSection, int pageNumber) const
{
    if (section < 0 || section >= m_flow->sections.size()) return kDefaultSlot;
    return m_flow->sections[section].slotFor(firstOfSection, pageNumber);
}

bool WriteDoc::isHeaderFooterStory(const QTextDocument *story) const { return partOfStory(story) != nullptr; }

bool WriteDoc::headerFooterSlotOf(const QTextDocument *story, int *section, bool *footer, int *slot) const
{
    if (!story) return false;
    for (int i = 0; i < m_flow->sections.size(); ++i)
        for (const bool f : {false, true})
            for (int k = 0; k < 3; ++k)
                if ((f ? m_flow->sections[i].footer : m_flow->sections[i].header).slot(k).get() == story) {
                    if (section) *section = i;
                    if (footer) *footer = f;
                    if (slot) *slot = k;
                    return true;
                }
    return false;
}

bool WriteDoc::createHeaderFooter(int section, bool footer, int slot)
{
    if (section < 0 || section >= sectionCount()) return false;
    slot = std::clamp(slot, 0, 2);
    if (QTextDocument *have = headerFooterStory(section, footer, slot)) {
        const HeaderPart *part = partOfStory(have);
        if (part && !part->missing) return !part->unreadable;
    }
    SectionProps props = sectionProps(section);
    const HeaderRefs refs = headerRefsOf(props);
    const QString own = footer ? refs.footer[slot] : refs.header[slot];
    if (!own.isEmpty()) {
        // A reference that names nothing (no such relationship, or no such part) is replaced; one that names a part the program does not show (past its limit, or an address outside
        // the file) is not: it would be lost.
        const Relationship rel = mainRelationship(own);
        if (!rel.id.isEmpty() && (rel.external || partExists(rel.target))) return false;
    }
    const QString style = footer ? QStringLiteral("Footer") : QStringLiteral("Header");
    ensureStyle(style);
    Block blank;
    blank.para.props.style = style;
    auto story = std::make_shared<QTextDocument>();
    story->setDocumentMargin(0);
    story->setDefaultFont(this->story()->defaultFont());
    story->setUndoRedoEnabled(false);
    HeaderPart *part = addHeaderPart(footer, story);
    fillStory(story.get(), {blank}, true, part->name, true, false, -1, true);
    finishEditable(*part);
    setHeaderReference(&props, footer, slot, part->relationship);
    setSectionProps(section, props);
    return true;
}

bool WriteDoc::setLinkedToPrevious(int section, bool footer, int slot, bool linked)
{
    if (section <= 0 || section >= sectionCount()) return false;
    slot = std::clamp(slot, 0, 2);
    SectionProps props = sectionProps(section);
    const HeaderRefs refs = headerRefsOf(props);
    const QString own = footer ? refs.footer[slot] : refs.header[slot];
    if (linked) {
        if (own.isEmpty()) return true;
        const Relationship rel = mainRelationship(own);
        // (A part that is in the file and that the program could not show is not let go.)
        const HeaderPart *part = rel.id.isEmpty() ? nullptr : m_headerParts.constFind(rel.target) == m_headerParts.cend() ? nullptr : &m_headerParts.constFind(rel.target).value();
        if (!rel.id.isEmpty() && !rel.external && partExists(rel.target) && (!part || part->unreadable)) return false;
        if (!rel.id.isEmpty() && !rel.external) m_released.insert(rel.target);
        setHeaderReference(&props, footer, slot, QString());
        setSectionProps(section, props);
        return true;
    }
    if (!own.isEmpty()) return true;
    // Off: a copy of what the section shows, to edit by itself.
    QTextDocument *shown = headerFooterStory(section, footer, slot);
    HeaderPart *source = partOfStory(shown);
    if (!source || !makeEditable(*source)) return false;
    auto copy = std::shared_ptr<QTextDocument>(flow::cloneStory(shown).release());
    copy->setDocumentMargin(shown->documentMargin());
    copy->setDefaultFont(shown->defaultFont());
    copy->setDefaultTextOption(shown->defaultTextOption());
    const QByteArray root = source->rootStart;
    HeaderPart *part = addHeaderPart(footer, copy, source->name);
    part->rootStart = root;
    finishEditable(*part);
    setHeaderReference(&props, footer, slot, part->relationship);
    setSectionProps(section, props);
    return true;
}

bool WriteDoc::differentFirstPage(int section) const
{
    return section >= 0 && section < int(m_sections.size()) && m_sections[section].titlePg;
}

void WriteDoc::setDifferentFirstPage(int section, bool on)
{
    if (section < 0 || section >= sectionCount() || differentFirstPage(section) == on) return;
    SectionProps props = sectionProps(section);
    props.titlePg = on;
    setSectionProps(section, props);
}

void WriteDoc::setDifferentOddEven(bool on)
{
    if (m_wml.evenAndOddHeaders == on) return;
    SectionProps last = m_last;
    last.oddEvenHeaders = on;
    setSectionProps(sectionCount() - 1, last);
}

// ---- writing them

QByteArray WriteDoc::headerPartXml(const HeaderPart &part)
{
    QVector<Block> blocks;
    VectorSink sink(&blocks);
    exportInto(sink, part.story.get());
    return blocksPartXml(blocks, part.rootStart, part.footer ? QByteArray("w:ftr") : QByteArray("w:hdr"));
}

void WriteDoc::writeHeaderParts(office::wml::Document *out)
{
    // The parts the sections name.
    QSet<QString> used;
    for (const SectionProps &props : m_sections) {
        const HeaderRefs refs = headerRefsOf(props);
        for (int k = 0; k < 3; ++k)
            for (const QString &id : {refs.header[k], refs.footer[k]}) {
                if (id.isEmpty()) continue;
                const Relationship rel = mainRelationship(id);
                if (!rel.id.isEmpty() && !rel.external) used.insert(rel.target.toLower());
            }
    }
    auto dropPart = [&](const QString &name) {
        for (int i = int(out->parts.size()) - 1; i >= 0; --i)
            if (out->parts[i].name.compare(name, Qt::CaseInsensitive) == 0) out->parts.removeAt(i);
        out->mainRelationships.erase(std::remove_if(out->mainRelationships.begin(), out->mainRelationships.end(),
                                                    [&](const Relationship &r) { return !r.external && r.target.compare(name, Qt::CaseInsensitive) == 0; }),
                                     out->mainRelationships.end());
        for (auto it = out->partRelationships.begin(); it != out->partRelationships.end();) {
            if (it.key().compare(name, Qt::CaseInsensitive) == 0) it = out->partRelationships.erase(it);
            else ++it;
        }
    };
    QStringList names = m_headerParts.keys();
    names.sort();
    for (const QString &name : names) {
        const HeaderPart &part = m_headerParts[name];
        const bool isUsed = used.contains(name.toLower());
        if (part.created) {
            if (!isUsed) {
                dropPart(name);
                continue;
            }
            KeptPart kept;
            kept.name = name;
            kept.contentType = part.footer ? QString(kFooterType) : QString(kHeaderType);
            kept.data = headerPartXml(part);
            out->parts.push_back(kept);
            continue;
        }
        if (m_released.contains(name) && !isUsed) {
            dropPart(name);
            continue;
        }
        if (part.modified && part.editForm) {
            const QByteArray data = headerPartXml(part);
            for (KeptPart &kept : out->parts)
                if (kept.name.compare(name, Qt::CaseInsensitive) == 0) {
                    kept.data = data;
                    break;
                }
        }
    }
    // Different headers on odd and even pages is a setting of the document.
    if (m_wml.evenAndOddHeaders != m_oddEvenAtOpen) {
        const bool on = m_wml.evenAndOddHeaders;
        bool done = false;
        if (!out->settingsName.isEmpty())
            for (KeptPart &kept : out->parts)
                if (kept.name.compare(out->settingsName, Qt::CaseInsensitive) == 0) {
                    kept.data = settingsWithEvenAndOddHeaders(kept.data, on);
                    done = true;
                    break;
                }
        if (!done && on) {
            KeptPart kept;
            kept.name = QStringLiteral("/word/settings.xml");
            for (int n = 2; partExists(kept.name) || kept.name.compare(out->mainName, Qt::CaseInsensitive) == 0; ++n) kept.name = QStringLiteral("/word/settings%1.xml").arg(n);
            kept.contentType = QString(kSettingsType);
            kept.data = settingsWithEvenAndOddHeaders(QByteArray(), true);
            out->parts.push_back(kept);
            Relationship r;
            r.id = nextRelationshipId(out->mainRelationships);
            r.type = QString(kSettingsRel);
            r.target = kept.name;
            out->mainRelationships.push_back(r);
            out->settingsName = kept.name;
        }
    }
}

// ---- the one history

void WriteDoc::watchHistory(QTextDocument *story)
{
    connect(story, &QTextDocument::undoCommandAdded, this, [this, story] { stepAdded(story); });
    connect(story, &QTextDocument::contentsChange, this, [this, story](int, int, int) { storyEdited(story); });
}

void WriteDoc::stepAdded(QTextDocument *story)
{
    m_undoLog.push_back(Step{story, m_nextSerial++});
    m_redoLog.clear();
    checkModified();
    Q_EMIT historyChanged();
}

void WriteDoc::storyEdited(QTextDocument *story)
{
    // The next edit of any other story is not a continuation of its last step (the steps are in the order they were made).
    if (story != this->story()) this->story()->setModified(false);
    for (auto it = m_headerParts.begin(); it != m_headerParts.end(); ++it)
        if (it->editForm && it->story.get() != story) it->story->setModified(false);
    if (story != this->story()) headerEdited(story);
}

void WriteDoc::headerEdited(QTextDocument *story)
{
    if (HeaderPart *part = partOfStory(story)) part->modified = true;
    if (m_pager) m_pager->furnitureChanged(story);
    Q_EMIT headerFooterChanged(story);
}

void WriteDoc::sealStories()
{
    story()->setModified(false);
    for (auto it = m_headerParts.begin(); it != m_headerParts.end(); ++it)
        if (it->editForm) it->story->setModified(false);
}

bool WriteDoc::isModified() const
{
    const quint64 top = m_undoLog.isEmpty() ? 0 : m_undoLog.last().serial;
    return m_forcedDirty || top != m_cleanSerial;
}

void WriteDoc::setModified(bool modified)
{
    if (modified) {
        m_forcedDirty = true;
    } else {
        m_forcedDirty = false;
        m_cleanSerial = m_undoLog.isEmpty() ? 0 : m_undoLog.last().serial;
        sealStories();
    }
    checkModified();
}

void WriteDoc::checkModified()
{
    const bool now = isModified();
    if (now == m_wasModified) return;
    m_wasModified = now;
    Q_EMIT modifiedChanged();
}

QTextDocument *WriteDoc::undo(QTextCursor *cursor)
{
    if (m_undoLog.isEmpty()) return nullptr;
    const Step step = m_undoLog.takeLast();
    QTextCursor at(step.story);
    step.story->undo(&at);
    if (cursor) *cursor = at;
    m_redoLog.push_back(step);
    sealStories();
    checkModified();
    Q_EMIT historyChanged();
    return step.story;
}

QTextDocument *WriteDoc::redo(QTextCursor *cursor)
{
    if (m_redoLog.isEmpty()) return nullptr;
    const Step step = m_redoLog.takeLast();
    QTextCursor at(step.story);
    step.story->redo(&at);
    if (cursor) *cursor = at;
    m_undoLog.push_back(step);
    sealStories();
    checkModified();
    Q_EMIT historyChanged();
    return step.story;
}

} // namespace jp::write
