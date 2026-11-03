// How the notes are numbered and where they are set: the document's settings (footnotePr and endnotePr), read, changed, and written back with the file.

#include "core/wordformat.h"
#include "core/writedoc.h"
#include "flow/flowdocument.h"
#include "flow/paginator.h"

using namespace jp::office;
using namespace jp::office::wml;

namespace jp::write {

namespace {
constexpr char kSettingsRel[] = "http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings";
constexpr char kSettingsType[] = "application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml";

NoteProperties propertiesOf(const WriteDoc::NoteSettings &s)
{
    NoteProperties p;
    p.format = s.format;
    p.start = s.start;
    p.restart = s.restart;
    p.position = s.position;
    return p;
}
} // namespace

WriteDoc::NoteSettings WriteDoc::noteSettings(bool endnotes) const
{
    if (m_noteEdited[endnotes ? 1 : 0]) return m_noteEdit[endnotes ? 1 : 0];
    NoteSettings out;
    const QByteArray settings = m_wml.settingsName.isEmpty() ? QByteArray() : partData(m_wml.settingsName);
    const char *group = endnotes ? "endnotePr" : "footnotePr";
    const NoteNumbering n = readNoteNumbering(settings, group);
    if (!n.format.isEmpty()) out.format = n.format;
    else if (endnotes) out.format = QStringLiteral("lowerRoman");   // (the endnotes' own where the settings say none)
    if (n.start > 0) out.start = n.start;
    out.position = n.position;
    const QString restart = noteNumberRestart(settings, group);
    if (!restart.isEmpty()) out.restart = restart;
    return out;
}

void WriteDoc::setNoteSettings(bool endnotes, const NoteSettings &s)
{
    const int k = endnotes ? 1 : 0;
    if (noteSettings(endnotes) == s) return;
    m_noteEdit[k] = s;
    m_noteEdited[k] = true;
    if (endnotes) {
        // (What the endnotes' numbers show is made again when the document is read from what is saved.)
        m_endnoteFormat = s.format;
        m_endnoteStart = std::max(1, s.start);
        m_endnotePosition = s.position;
        m_endnoteRestartSections = s.restart == QLatin1String("eachSect");
    } else {
        applyFootnoteSettings();
        if (m_pager) m_pager->invalidate();
    }
    setModified(true);
}

QString NoteFields::noteMarkText(bool endnote, int number) const
{
    if (endnote || number <= 0) return QString::number(number);
    return noteNumberText(footnoteFormat.isEmpty() ? QStringLiteral("decimal") : footnoteFormat, number + std::max(footnoteStart, 1) - 1);
}

void WriteDoc::applyFootnoteSettings()
{
    const NoteSettings s = noteSettings(false);
    m_flow->restartNotesInSections = s.restart == QLatin1String("eachSect");
    if (m_noteFields) {
        m_noteFields->footnoteFormat = s.format;
        m_noteFields->footnoteStart = s.start;
        m_noteFields->eachPage = s.restart == QLatin1String("eachPage");
    }
}

void WriteDoc::writeNoteSettings(office::wml::Document *out)
{
    for (int k = 0; k < 2; ++k) {
        if (!m_noteEdited[k]) continue;
        const NoteProperties p = propertiesOf(m_noteEdit[k]);
        bool done = false;
        if (!out->settingsName.isEmpty()) {
            for (KeptPart &kept : out->parts)
                if (kept.name.compare(out->settingsName, Qt::CaseInsensitive) == 0) {
                    kept.data = settingsWithNoteProperties(kept.data, k == 1, p);
                    done = true;
                    break;
                }
        }
        if (done) continue;
        KeptPart kept;
        kept.name = QStringLiteral("/word/settings.xml");
        for (int n = 2; partExists(kept.name) || kept.name.compare(out->mainName, Qt::CaseInsensitive) == 0; ++n) kept.name = QStringLiteral("/word/settings%1.xml").arg(n);
        kept.contentType = QString(kSettingsType);
        kept.data = settingsWithNoteProperties(QByteArray(), k == 1, p);
        out->parts.push_back(kept);
        Relationship r;
        r.id = nextRelationshipId(out->mainRelationships);
        r.type = QString(kSettingsRel);
        r.target = kept.name;
        out->mainRelationships.push_back(r);
        out->settingsName = kept.name;
    }
}

} // namespace jp::write
