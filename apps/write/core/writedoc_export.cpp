// Writing the story back: blocks become paragraphs, formatted text becomes runs whose properties are what differs from their styles', and
// the pieces kept as XML go back where they were.

#include "core/writedoc.h"

#include "core/wordformat.h"
#include "core/writeprops.h"
#include "text/textprops.h"

#include <QTextBlock>
#include <algorithm>
#include <QTextCursor>
#include <QTextDocument>

using namespace jp::office;
using namespace jp::office::wml;

namespace jp::write {

void WriteDoc::exportBody(QVector<Block> *out)
{
    QTextDocument *story = this->story();
    const StyleSheet &sheet = m_wml.styles;
    const QString defaultStyle = defaultParagraphStyle();
    int sectionIndex = 0;

    for (QTextBlock b = story->begin(); b.isValid();) {
        const QTextBlockFormat bf = b.blockFormat();
        if (bf.hasProperty(prop::RawBefore))
            for (const Block &r : unpackBlocks(bf.property(prop::RawBefore).toByteArray())) out->push_back(r);
        if (bf.hasProperty(prop::RawBlock)) {
            for (const Block &r : unpackBlocks(bf.property(prop::RawBlock).toByteArray())) out->push_back(r);
            if (bf.hasProperty(prop::RawAfter))
                for (const Block &r : unpackBlocks(bf.property(prop::RawAfter).toByteArray())) out->push_back(r);
            b = b.next();
            continue;
        }

        // The paragraph, and the paragraphs that a page break cut from it.
        ParaProps full = paraPropsOf(bf);
        const QString styleId = full.style.isEmpty() ? defaultStyle : full.style;
        const int numId = full.numId.value_or(0), level = full.ilvl.value_or(0);
        ParaProps direct = full.differences(baselineParagraph(styleId, numId, level));
        direct.style = styleId == defaultStyle ? QString() : styleId;
        if (bf.hasProperty(prop::OddTabs)) {
            QVector<TabStop> tabs = direct.tabs.value_or(QVector<TabStop>());
            tabs += unpackTabs(bf.property(prop::OddTabs).toByteArray());
            std::stable_sort(tabs.begin(), tabs.end(), [](const TabStop &a, const TabStop &b) { return a.pos < b.pos; });
            direct.tabs = tabs;
        }
        if (numId == 0 && !sheet.paragraph(styleId).numId.value_or(0)) {
            direct.numId.reset();
            direct.ilvl.reset();
        }
        Block blk;
        Paragraph &p = blk.para;
        if (bf.hasProperty(prop::ParaAttrs)) p.attrs = bf.property(prop::ParaAttrs).toByteArray();

        QTextBlock part = b;
        QTextBlock lastPart = b;
        for (;;) {
            lastPart = part;
            for (QTextBlock::iterator it = part.begin(); !it.atEnd(); ++it) {
                const QTextFragment f = it.fragment();
                const QTextCharFormat cf = f.charFormat();
                const QString text = f.text();
                RunProps run = runPropsOf(cf);
                RunProps diff = run.differences(sheet.run(styleId, run.style));
                diff.style = run.style;
                if (cf.hasProperty(prop::Atom)) {
                    Inline a = unpackAtom(cf.property(prop::Atom).toByteArray());
                    a.props = diff;
                    for (const QChar ch : text)
                        if (ch == QChar::ObjectReplacementCharacter) p.content.push_back(a);
                    continue;
                }
                QString clean = text;
                clean.remove(QChar::ObjectReplacementCharacter);
                if (clean.isEmpty()) continue;
                if (!p.content.isEmpty() && p.content.last().kind == Inline::Text && p.content.last().props == diff) {
                    p.content.last().text += clean;
                } else {
                    Inline t;
                    t.props = diff;
                    t.text = clean;
                    p.content.push_back(t);
                }
            }
            const QTextBlock next = part.next();
            if (!part.blockFormat().boolProperty(prop::JoinNext) || !next.isValid()) break;
            Inline br;
            br.kind = Inline::RunAtom;
            br.raw = "<w:br w:type=\"page\"/>";
            br.tag = QStringLiteral("br");
            br.key = QStringLiteral("page");
            p.content.push_back(br);
            part = next;
        }

        // The paragraph mark's own formatting: the last paragraph's.
        {
            RunProps mark = runPropsOf(lastPart.charFormat());
            RunProps diff = mark.differences(sheet.run(styleId, mark.style));
            diff.style = mark.style;
            if (!runPropsXml(diff).isEmpty()) direct.markRun = diff;
        }
        if (lastPart.blockFormat().boolProperty(flow::fp::SectionBreakAfter) && sectionIndex + 1 < int(m_sections.size()))
            direct.section = std::make_shared<SectionProps>(m_sections[sectionIndex++]);
        p.props = direct;
        out->push_back(blk);
        const QTextBlockFormat lastFormat = lastPart.blockFormat();
        if (lastFormat.hasProperty(prop::RawAfter))
            for (const Block &r : unpackBlocks(lastFormat.property(prop::RawAfter).toByteArray())) out->push_back(r);
        b = lastPart.next();
    }
}

QByteArray WriteDoc::toDocx(QString *error, bool template_)
{
    syncSections();
    office::wml::Document out = m_wml;
    exportBody(&out.body);
    out.lastSection = m_last;
    out.lastSectionStated = true;
    // Styles a document that had none got when one was applied: the part, and the link to it.
    if (out.stylesName.isEmpty() && !out.styles.styles.isEmpty()) {
        out.stylesName = QStringLiteral("/word/styles.xml");
        int id = 1;
        for (bool taken = true; taken;) {
            taken = false;
            for (const office::Relationship &r : std::as_const(out.mainRelationships)) taken = taken || r.id == QStringLiteral("rId%1").arg(id);
            if (taken) ++id;
        }
        out.mainRelationships.push_back(office::Relationship{QStringLiteral("rId%1").arg(id), QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles"), out.stylesName, false});
    }
    // Macros are not kept: a file that had them is saved without, as a plain document.
    for (int i = int(out.parts.size()) - 1; i >= 0; --i)
        if (out.parts[i].contentType.startsWith(QLatin1String("application/vnd.ms-office.vba")) || out.parts[i].contentType.contains(QLatin1String("macroEnabled"))) {
            out.partRelationships.remove(out.parts[i].name);
            out.parts.removeAt(i);
        }
    auto dropMacroLinks = [](QVector<office::Relationship> *rels) {
        rels->erase(std::remove_if(rels->begin(), rels->end(), [](const office::Relationship &r) { return r.type.endsWith(QLatin1String("/vbaProject")) || r.type.endsWith(QLatin1String("/vbaProjectSignature")); }), rels->end());
    };
    dropMacroLinks(&out.mainRelationships);
    out.mainContentType = template_ ? QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.template.main+xml")
                                    : QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml");
    return writeDocx(out, error);
}

} // namespace jp::write
