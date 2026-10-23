// Headers and footers: the three slots of a section, making and linking them, putting a story in the form it is edited in, writing the parts back, and the one history of the text
// and the stories of its headers and footers.

#include "core/writedoc.h"

#include "core/tablemodel.h"
#include "core/writeprops.h"
#include "flow/paginator.h"

#include <QRegularExpression>
#include <QSet>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <algorithm>
#include <functional>

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

WriteDoc::HeaderPart *WriteDoc::addHeaderPart(bool footer, std::shared_ptr<QTextDocument> story, const QString &copyRelationshipsOf, bool withRelationship)
{
    HeaderPart part;
    part.name = newPartName(footer);
    part.footer = footer;
    part.story = std::move(story);
    part.created = true;
    part.editForm = true;
    if (withRelationship) {
        Relationship r;
        r.id = nextRelationshipId(m_wml.mainRelationships);
        r.type = footer ? QString(kFooterRel) : QString(kHeaderRel);
        r.target = part.name;
        m_wml.mainRelationships.push_back(r);
        part.relationship = r.id;
    }
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
    leaveOutDeletedParagraphs(&blocks, true);   // (as the page shows it: what a tracked change deleted is not in the story, and is written back as it was)
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

bool WriteDoc::isPendingHeaderFooter(const QTextDocument *story) const
{
    if (m_pending.section < 0 || !story) return false;
    const auto it = m_headerParts.constFind(m_pending.part);
    return it != m_headerParts.cend() && it->story.get() == story;
}

bool WriteDoc::createHeaderFooter(int section, bool footer, int slot, bool lazy)
{
    if (section < 0 || section >= sectionCount()) return false;
    slot = std::clamp(slot, 0, 2);
    // The empty one that is shown to be typed in: asked for again, it is the one (made at once, if it is not asked for lazily); another slot's takes its place.
    if (m_pending.section >= 0) {
        if (m_pending.section == section && m_pending.footer == footer && m_pending.slot == slot) return lazy ? true : commitPending();
        discardPendingHeaderFooter();
    }
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
    {
        // (The styles it adds are written only if a header or footer that is kept has them: see dropHeaderStyles.)
        const qsizetype had = m_wml.styles.styles.size();
        ensureStyle(style);
        for (qsizetype i = had; i < m_wml.styles.styles.size(); ++i) m_headerStyles.insert(m_wml.styles.styles[i].id);
    }
    Block blank;
    blank.para.props.style = style;
    auto story = std::make_shared<QTextDocument>();
    story->setDocumentMargin(0);
    story->setDefaultFont(this->story()->defaultFont());
    story->setUndoRedoEnabled(false);
    HeaderPart *part = addHeaderPart(footer, story, QString(), !lazy);
    fillStory(story.get(), {blank}, true, part->name, true, false, -1, true);
    finishEditable(*part);
    if (lazy) {
        m_pending = Pending{part->name, section, slot, footer};
        attachHeaders(section);
        if (m_pager) m_pager->invalidateFrom(section);
        Q_EMIT sectionsChanged();
        return true;
    }
    setHeaderReference(&props, footer, slot, part->relationship);
    setSectionProps(section, props);
    return true;
}

bool WriteDoc::commitPending()
{
    if (m_pending.section < 0) return false;
    const Pending pending = m_pending;
    const auto found = m_headerParts.find(pending.part);
    if (found == m_headerParts.end()) {
        m_pending = Pending();
        return false;
    }
    HeaderPart &part = found.value();
    QTextDocument *typed = part.story.get();
    // The part is in the file now: its relationship, and the section's reference to it.
    Relationship r;
    r.id = nextRelationshipId(m_wml.mainRelationships);
    r.type = part.footer ? QString(kFooterRel) : QString(kHeaderRel);
    r.target = part.name;
    m_wml.mainRelationships.push_back(r);
    part.relationship = r.id;
    m_pending = Pending();
    SectionProps props = sectionProps(pending.section);
    setHeaderReference(&props, pending.footer, pending.slot, part.relationship);
    // (The change of the section is a step of the history before the typing that made it: the typing's step was logged first, and is put after.)
    const bool typedLast = !m_undoLog.isEmpty() && m_undoLog.last().story == typed;
    const int logged = int(m_undoLog.size());
    m_committing = true;
    setSectionProps(pending.section, props);
    m_committing = false;
    if (typedLast && int(m_undoLog.size()) > logged) std::rotate(m_undoLog.begin() + (logged - 1), m_undoLog.begin() + logged, m_undoLog.end());
    return true;
}

void WriteDoc::discardPendingHeaderFooter()
{
    if (m_pending.section < 0) return;
    const Pending pending = m_pending;
    m_pending = Pending();
    std::shared_ptr<QTextDocument> story;
    const auto found = m_headerParts.find(pending.part);
    if (found != m_headerParts.end()) {
        story = found->story;
        m_headerParts.erase(found);
    }
    attachHeaders(pending.section);
    if (m_pager) {
        if (story) m_pager->releaseStory(story.get());
        m_pager->invalidateFrom(pending.section);
    }
    Q_EMIT sectionsChanged();
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
    copy->setUndoRedoEnabled(false);
    renewIds(copy.get());
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

// ---- ids of a copy

namespace {

// The XML pieces a story keeps as written (the atoms of its paragraphs, and the elements kept between them), to be read, and changed where `visit` says it changed one.
void eachKeptXml(QTextDocument *story, const std::function<bool(QByteArray *)> &visit)
{
    struct Fix {
        int from, to;
        QTextCharFormat format;
    };
    QVector<Fix> atoms;
    struct BlockFix {
        int position;
        QTextBlockFormat format;
    };
    QVector<BlockFix> blocks;
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
        QTextBlockFormat bf = b.blockFormat();
        bool blockChanged = false;
        for (const int key : {int(prop::RawBefore), int(prop::RawBlock), int(prop::RawAfter)}) {
            if (!bf.hasProperty(key)) continue;
            QVector<Block> kept = unpackBlocks(storedBytes(bf, key));
            bool changed = false;
            for (Block &k : kept)
                if (k.isRaw) changed = visit(&k.raw) || changed;
            if (changed) {
                setStored(bf, key, packBlocks(kept));
                blockChanged = true;
            }
        }
        if (blockChanged) blocks.push_back({b.position(), bf});
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment f = it.fragment();
            const QTextCharFormat cf = f.charFormat();
            if (!cf.hasProperty(prop::Atom)) continue;
            Inline a = unpackAtom(storedBytes(cf, prop::Atom));
            if (!visit(&a.raw)) continue;
            QTextCharFormat nf = cf;
            setStored(nf, prop::Atom, packAtom(a));
            atoms.push_back({f.position(), f.position() + f.length(), nf});
        }
    }
    for (const Fix &fix : atoms) {
        QTextCursor c(story);
        c.setPosition(fix.from);
        c.setPosition(fix.to, QTextCursor::KeepAnchor);
        c.setCharFormat(fix.format);
    }
    for (const BlockFix &fix : blocks) QTextCursor(story->findBlock(fix.position)).setBlockFormat(fix.format);
}

// The elements whose ids have to differ from those of all the others in a file (a bookmark's, a drawing's, a comment range's, a tracked change's).
const QRegularExpression &idElement()
{
    static const QRegularExpression re(QStringLiteral("<((?:[\\w.-]+:)?(?:bookmarkStart|bookmarkEnd|commentRangeStart|commentRangeEnd|docPr|ins|del|moveFrom|moveTo))\\b([^>]*)>"));
    return re;
}
const QRegularExpression &idAttribute()
{
    static const QRegularExpression re(QStringLiteral("(\\s(?:[\\w.-]+:)?id=\")(\\d+)(\")"));
    return re;
}
const QRegularExpression &nameAttribute()
{
    static const QRegularExpression re(QStringLiteral("(\\s(?:[\\w.-]+:)?name=\")([^\"]*)(\")"));
    return re;
}

} // namespace

// A copy of a story has the same ids as the story: its bookmarks (by id and by name), drawings, comment ranges, and tracked changes are given others, in pairs as they were.
void WriteDoc::renewIds(QTextDocument *copy)
{
    qint64 highest = 0;
    QSet<QString> names;
    auto read = [&](QByteArray *xml) {
        const QString text = QString::fromUtf8(*xml);
        for (auto e = idElement().globalMatch(text); e.hasNext();) {
            const QString attributes = e.next().captured(2);
            if (const auto id = idAttribute().match(attributes); id.hasMatch()) highest = std::max(highest, id.captured(2).toLongLong());
            if (const auto name = nameAttribute().match(attributes); name.hasMatch()) names.insert(name.captured(2));
        }
        return false;
    };
    // (The ids of the text and of every header and footer, in the form it is shown or edited in: the pieces kept as written are in the story in both.)
    eachKeptXml(story(), read);
    for (auto it = m_headerParts.cbegin(); it != m_headerParts.cend(); ++it)
        if (it->story) eachKeptXml(it->story.get(), read);
    qint64 next = highest + 1;
    QHash<QString, QString> ids, renamed;
    eachKeptXml(copy, [&](QByteArray *xml) {
        const QString text = QString::fromUtf8(*xml);
        QString out;
        qsizetype from = 0;
        bool changed = false;
        for (auto e = idElement().globalMatch(text); e.hasNext();) {
            const QRegularExpressionMatch m = e.next();
            const QString element = m.captured(1).section(QLatin1Char(':'), -1);
            const bool bookmark = element.startsWith(QLatin1String("bookmark"));
            const QString kind = bookmark ? QStringLiteral("b") : element.startsWith(QLatin1String("comment")) ? QStringLiteral("c") : element == QLatin1String("docPr") ? QStringLiteral("d") : QStringLiteral("r");
            QString replaced = m.captured(2);
            if (const auto id = idAttribute().match(replaced); id.hasMatch()) {
                const QString key = kind + id.captured(2);
                if (!ids.contains(key)) ids.insert(key, QString::number(next++));
                replaced.replace(id.capturedStart(2), id.capturedLength(2), ids.value(key));
            }
            if (element == QLatin1String("bookmarkStart")) {
                if (const auto name = nameAttribute().match(replaced); name.hasMatch()) {
                    const QString old = name.captured(2);
                    if (!renamed.contains(old)) {
                        QString fresh;
                        for (int n = 2; fresh.isEmpty() || names.contains(fresh); ++n) fresh = old.left(36 - QString::number(n).size() - 1) + QLatin1Char('_') + QString::number(n);
                        names.insert(fresh);
                        renamed.insert(old, fresh);
                    }
                    replaced.replace(name.capturedStart(2), name.capturedLength(2), renamed.value(old));
                }
            }
            out += text.mid(from, m.capturedStart(2) - from) + replaced;
            from = m.capturedEnd(2);
            changed = true;
        }
        if (!changed) return false;
        out += text.mid(from);
        *xml = out.toUtf8();
        return true;
    });
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
    // Whether a relationship of the file (of the package, the main part, or another part) is to the part.
    auto referenced = [&](const QString &target) {
        auto has = [&](const QVector<Relationship> &list) {
            return std::any_of(list.begin(), list.end(), [&](const Relationship &r) { return !r.external && r.target.compare(target, Qt::CaseInsensitive) == 0; });
        };
        if (has(out->mainRelationships) || has(out->packageRelationships)) return true;
        for (auto it = out->partRelationships.cbegin(); it != out->partRelationships.cend(); ++it)
            if (has(it.value())) return true;
        return false;
    };
    // A part that is let go, with its relationships, and the parts only it had a relationship to (its pictures) with theirs.
    std::function<void(const QString &)> dropPart = [&](const QString &name) {
        QStringList theirs;
        for (auto it = out->partRelationships.cbegin(); it != out->partRelationships.cend(); ++it)
            if (it.key().compare(name, Qt::CaseInsensitive) == 0)
                for (const Relationship &r : it.value())
                    if (!r.external) theirs << r.target;
        for (int i = int(out->parts.size()) - 1; i >= 0; --i)
            if (out->parts[i].name.compare(name, Qt::CaseInsensitive) == 0) out->parts.removeAt(i);
        out->mainRelationships.erase(std::remove_if(out->mainRelationships.begin(), out->mainRelationships.end(),
                                                    [&](const Relationship &r) { return !r.external && r.target.compare(name, Qt::CaseInsensitive) == 0; }),
                                     out->mainRelationships.end());
        for (auto it = out->partRelationships.begin(); it != out->partRelationships.end();) {
            if (it.key().compare(name, Qt::CaseInsensitive) == 0) it = out->partRelationships.erase(it);
            else ++it;
        }
        for (const QString &target : std::as_const(theirs))
            if (target.compare(name, Qt::CaseInsensitive) != 0 && !referenced(target)) dropPart(target);
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
        if (partEdited(part)) {
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

void WriteDoc::dropHeaderStyles(office::wml::Document *out) const
{
    if (m_headerStyles.isEmpty()) return;
    // The styles a paragraph or a run that is written has: the text's, and those of the parts the program made or rewrote (a header or footer it made, a notes part).
    QSet<QString> keep;
    for (const Block &b : std::as_const(out->body)) {
        if (b.isRaw) continue;
        if (m_headerStyles.contains(b.para.props.style)) keep.insert(b.para.props.style);
        for (const Inline &in : b.para.content)
            if (m_headerStyles.contains(in.props.style)) keep.insert(in.props.style);
    }
    for (const KeptPart &part : std::as_const(out->parts)) {
        const auto found = m_headerParts.constFind(part.name);
        const bool notes = part.name.compare(m_notePart, Qt::CaseInsensitive) == 0 || part.name.compare(m_endnotePart, Qt::CaseInsensitive) == 0;
        if (!notes && (found == m_headerParts.cend() || !found->created)) continue;
        for (const QString &id : m_headerStyles)
            if (part.data.contains("w:val=\"" + id.toUtf8() + "\"")) keep.insert(id);
    }
    // (And the styles those are based on, or go on to.)
    for (bool grew = true; grew;) {
        grew = false;
        for (const QString &id : QSet<QString>(keep))
            if (const Style *style = out->styles.find(id))
                for (const QString &other : {style->basedOn, style->next, style->link})
                    if (m_headerStyles.contains(other) && !keep.contains(other)) {
                        keep.insert(other);
                        grew = true;
                    }
    }
    bool dropped = false;
    for (const QString &id : m_headerStyles)
        if (!keep.contains(id)) dropped = out->styles.remove(id) || dropped;
    // All that the program added is gone and nothing else was: the styles are as the file had them.
    if (dropped && out->styles.styles.size() == m_stylesCountAtOpen) out->styles.modified = m_stylesModifiedAtOpen;
}

// ---- the one history

void WriteDoc::watchHistory(QTextDocument *story)
{
    connect(story, &QTextDocument::undoCommandAdded, this, [this, story] { stepAdded(story); });
    connect(story, &QTextDocument::contentsChange, this, [this, story](int, int, int) { storyEdited(story); });
}

bool WriteDoc::partEdited(const HeaderPart &part) const
{
    // (Edited since it was read, as the history has it: one that is edited and all of it taken back is as it was, and is written as the file had it.)
    return part.editForm && part.story && part.story->availableUndoSteps() > 0;
}

void WriteDoc::stepAdded(QTextDocument *story)
{
    const quint64 serial = m_nextSerial++;
    m_undoLog.push_back(Step{story, serial, story->availableUndoSteps()});
    m_redoLog.clear();
    // (The notes made by this edit are known by its step.)
    for (const auto &made : std::as_const(m_noteStepPending)) (made.first ? m_createdEndnoteStep : m_createdFootnoteStep)[made.second] = serial;
    m_noteStepPending.clear();
    checkModified();
    Q_EMIT historyChanged();
}

void WriteDoc::storyEdited(QTextDocument *story)
{
    // The first edit of the empty header or footer that was shown to be typed in makes it, in the document.
    if (m_pending.section >= 0 && isPendingHeaderFooter(story)) commitPending();
    // The next edit of any other story is not a continuation of its last step (the steps are in the order they were made).
    if (m_committing) return;
    if (story != this->story()) this->story()->setModified(false);
    for (auto it = m_headerParts.begin(); it != m_headerParts.end(); ++it)
        if (it->editForm && it->story.get() != story) it->story->setModified(false);
    for (QTextDocument *note : std::as_const(m_dirtyNotes))
        if (note != story) note->setModified(false);
    if (story == this->story()) return;
    if (footnoteIdOf(story)) noteEdited(story);
    else headerEdited(story);
}

void WriteDoc::headerEdited(QTextDocument *story)
{
    if (m_pager) m_pager->furnitureChanged(story);
    Q_EMIT headerFooterChanged(story);
}

void WriteDoc::sealStories()
{
    story()->setModified(false);
    for (auto it = m_headerParts.begin(); it != m_headerParts.end(); ++it)
        if (it->editForm) it->story->setModified(false);
    for (QTextDocument *note : std::as_const(m_dirtyNotes)) note->setModified(false);
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
    discardPendingHeaderFooter();   // (the empty one shown to be typed in is no step)
    if (m_undoLog.isEmpty()) return nullptr;
    const Step step = m_undoLog.takeLast();
    QTextCursor at(step.story);
    step.story->undo(&at);
    if (cursor) *cursor = at;
    m_redoLog.push_back(step);
    // (An edit that joined the block of the step before it was told of twice: the entry of the step it was a part of is taken back with it.)
    const int left = step.story->availableUndoSteps();
    while (!m_undoLog.isEmpty() && m_undoLog.last().story == step.story && m_undoLog.last().raw > left) m_redoLog.push_back(m_undoLog.takeLast());
    sealStories();
    checkModified();
    Q_EMIT historyChanged();
    return step.story;
}

QTextDocument *WriteDoc::redo(QTextCursor *cursor)
{
    discardPendingHeaderFooter();
    if (m_redoLog.isEmpty()) return nullptr;
    const Step step = m_redoLog.takeLast();
    QTextCursor at(step.story);
    step.story->redo(&at);
    if (cursor) *cursor = at;
    m_undoLog.push_back(step);
    const int now = step.story->availableUndoSteps();
    while (!m_redoLog.isEmpty() && m_redoLog.last().story == step.story && m_redoLog.last().raw <= now) m_undoLog.push_back(m_redoLog.takeLast());
    sealStories();
    checkModified();
    Q_EMIT historyChanged();
    return step.story;
}

} // namespace jp::write
