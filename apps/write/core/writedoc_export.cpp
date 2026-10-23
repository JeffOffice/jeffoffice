// Writing the story back: blocks become paragraphs, formatted text becomes runs whose properties are what differs from their styles', and
// the pieces kept as XML go back where they were.

#include "core/writedoc.h"

#include "core/editops.h"
#include "core/tablemodel.h"
#include "core/wordformat.h"
#include "core/writeprops.h"
#include "text/textprops.h"

#include <QSet>
#include <QTextBlock>
#include <algorithm>
#include <QTextCursor>
#include <QTextDocument>

using namespace jp::office;
using namespace jp::office::wml;

namespace jp::write {

namespace {

// The text engine's formats hold one kind of capitals and one kind of strike-through: a run in capitals has no small capitals in its format, and one struck twice is not
// struck once. What a run in capitals says of small capitals, or a double strike of the single, is then what its style says (the capitals and the double strike win
// where both are on), not an "off" that the file never said. And a run with no typeface of its own, in a file that names none, is set in the body font of the theme:
// that is no typeface said, not a theme font said.
void settleKinds(RunProps *run, const RunProps &base)
{
    if (run->caps.value_or(false)) run->smallCaps = base.smallCaps;
    if (run->dstrike.value_or(false)) run->strike = base.strike;
    const Fonts none;
    const Fonts &b = base.fonts ? *base.fonts : none;
    if (run->fonts && run->fonts->ascii.isEmpty() && run->fonts->hAnsi.isEmpty() && run->fonts->asciiTheme == QLatin1String("minorHAnsi") && run->fonts->hAnsiTheme == QLatin1String("minorHAnsi") &&
        b.ascii.isEmpty() && b.hAnsi.isEmpty() && b.asciiTheme.isEmpty() && b.hAnsiTheme.isEmpty()) {
        run->fonts->asciiTheme.clear();
        run->fonts->hAnsiTheme.clear();
    }
}

} // namespace

void WriteDoc::exportBody(QVector<Block> *out)
{
    VectorSink sink(out);
    exportInto(sink);
}

void WriteDoc::exportInto(BlockSink &sink, QTextDocument *whose)
{
    QTextDocument *story = whose ? whose : this->story();
    const StyleSheet &sheet = m_wml.styles;
    const QString defaultStyle = defaultParagraphStyle();
    int sectionIndex = 0;
    QHash<QString, ParaProps> baselines;
    TableExporter tables(m_flow.get());
    // (A table is never the last thing in a body: a paragraph follows it, so what the last block was is known.)
    bool lastClosesTable = false;
    struct Tap : BlockSink {
        BlockSink &to;
        bool &lastClosesTable;
        Tap(BlockSink &t, bool &l) : to(t), lastClosesTable(l) {}
        void add(Block &&block) override {
            lastClosesTable = block.isRaw && block.role == Inline::Close && block.tag == QLatin1String("tbl");
            to.add(std::move(block));
        }
        void tableOpened(TablePart *p) override { to.tableOpened(p); }
        void tableClosed(TablePart *p) override { to.tableClosed(p); }
    } out(sink, lastClosesTable);

    for (QTextBlock b = story->begin(); b.isValid();) {
        const QTextBlockFormat bf = b.blockFormat();
        tables.moveTo(bf.intProperty(flow::fp::Cell), out);   // (the end of the tables the paragraph is not in, and the start of those it is)
        if (bf.hasProperty(prop::EndnoteBlock)) {   // (the endnotes the pages show after the text: the file's part is kept as it is)
            b = b.next();
            continue;
        }
        if (bf.hasProperty(prop::RawBefore))
            for (const Block &r : unpackBlocks(storedBytes(bf, prop::RawBefore))) out.add(Block(r));
        if (bf.hasProperty(prop::RawBlock)) {
            for (const Block &r : unpackBlocks(storedBytes(bf, prop::RawBlock))) out.add(Block(r));
            if (bf.hasProperty(prop::RawAfter))
                for (const Block &r : unpackBlocks(storedBytes(bf, prop::RawAfter))) out.add(Block(r));
            b = b.next();
            continue;
        }

        // The paragraph, and the paragraphs that a page break cut from it.
        ParaProps full = paraPropsOf(bf);
        const QString styleId = full.style.isEmpty() ? defaultStyle : full.style;
        const int numId = full.numId.value_or(0), level = full.ilvl.value_or(0);
        const bool numberedByItself = numId > 0 && bf.boolProperty(prop::NumDirect);
        const TableLayer *layer = tableLayerOf(bf.intProperty(flow::fp::Cell));   // (what the table's style gives the paragraphs of the cell)
        ParaProps direct = full.differences(baselineParagraph(styleId, numId, level, numberedByItself, &baselines, layer));
        if (numberedByItself && sheet.paragraph(styleId, layer).numId.value_or(0) == numId) {
            direct.numId = numId;   // (said again, as the file had it)
            direct.ilvl = level;
        }
        // (A file that flags a default style has it left out of its paragraphs. One that flags none has paragraphs that say Normal, which the program takes for the default:
        // they are saved saying it, since a reader that doesn't would give them another look.)
        direct.style = styleId == defaultStyle && (!sheet.defaultId(QStringLiteral("paragraph")).isEmpty() || !sheet.find(styleId)) ? QString() : styleId;
        if (bf.hasProperty(prop::OddTabs)) {
            QVector<TabStop> tabs = direct.tabs.value_or(QVector<TabStop>());
            tabs += unpackTabs(storedBytes(bf, prop::OddTabs));
            std::stable_sort(tabs.begin(), tabs.end(), [](const TabStop &a, const TabStop &b) { return a.pos < b.pos; });
            direct.tabs = tabs;
        }
        if (numId == 0 && !sheet.paragraph(styleId, layer).numId.value_or(0)) {
            direct.numId.reset();
            direct.ilvl.reset();
        }
        Block blk;
        Paragraph &p = blk.para;
        if (bf.hasProperty(prop::ParaAttrs)) p.attrs = storedBytes(bf, prop::ParaAttrs);

        const QTextBlock lastPart = b;
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment f = it.fragment();
            const QTextCharFormat cf = f.charFormat();
            const QString text = f.text();
            RunProps run = runPropsOf(cf);
            const RunProps base = sheet.run(styleId, run.style, layer);
            settleKinds(&run, base);
            RunProps diff = run.differences(base);
            diff.style = run.style;
            if (cf.hasProperty(prop::Atom)) {
                Inline a = unpackAtom(storedBytes(cf, prop::Atom));
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

        // The paragraph mark's own formatting: the last paragraph's.
        {
            RunProps mark = runPropsOf(markFormat(lastPart));
            const RunProps base = sheet.run(styleId, mark.style, layer);
            settleKinds(&mark, base);
            RunProps diff = mark.differences(base);
            diff.style = mark.style;
            if (!runPropsXml(diff).isEmpty()) direct.markRun = diff;
        }
        // (A section whose endnotes the pages set at its end has its break on the last of them in the story: it is the paragraph before them that ends the section in the file.)
        bool endsSection = lastPart.blockFormat().boolProperty(flow::fp::SectionBreakAfter);
        for (QTextBlock n = lastPart.next(); !endsSection && n.isValid() && n.blockFormat().hasProperty(prop::EndnoteBlock); n = n.next()) endsSection = n.blockFormat().boolProperty(flow::fp::SectionBreakAfter);
        if (endsSection && sectionIndex + 1 < int(m_sections.size()))
            direct.section = std::make_shared<SectionProps>(m_sections[sectionIndex++]);
        p.props = direct;
        out.add(std::move(blk));
        const QTextBlockFormat lastFormat = lastPart.blockFormat();
        if (lastFormat.hasProperty(prop::RawAfter))
            for (const Block &r : unpackBlocks(storedBytes(lastFormat, prop::RawAfter))) out.add(Block(r));
        b = b.next();
    }
    tables.finish(out);
    if (lastClosesTable) out.add(Block());
}

QByteArray WriteDoc::toDocx(QString *error, bool template_)
{
    syncSections();
    office::wml::Document out = m_wml;
    exportBody(&out.body);
    out.lastSection = m_last;
    out.lastSectionStated = true;
    writeHeaderParts(&out);
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
    // Macros are not kept: a file that had them is saved without, as a plain document (the project, its signature, the data it has, and the
    // links to them).
    QSet<QString> gone;
    for (int i = int(out.parts.size()) - 1; i >= 0; --i) {
        const QString &type = out.parts[i].contentType;
        if (type.startsWith(QLatin1String("application/vnd.ms-office.vba")) || type.contains(QLatin1String("macroEnabled")) || type.contains(QLatin1String("vbaData"))) {
            gone.insert(out.parts[i].name.toLower());
            out.partRelationships.remove(out.parts[i].name);
            out.parts.removeAt(i);
        }
    }
    auto dropMacroLinks = [&](QVector<office::Relationship> *rels) {
        rels->erase(std::remove_if(rels->begin(), rels->end(), [&](const office::Relationship &r) {
                        return r.type.endsWith(QLatin1String("/vbaProject")) || r.type.endsWith(QLatin1String("/vbaProjectSignature")) || r.type.endsWith(QLatin1String("/wordVbaData")) ||
                               (!r.external && gone.contains(r.target.toLower()));
                    }),
                    rels->end());
    };
    dropMacroLinks(&out.mainRelationships);
    dropMacroLinks(&out.packageRelationships);
    for (auto it = out.partRelationships.begin(); it != out.partRelationships.end(); ++it) dropMacroLinks(&it.value());
    out.mainContentType = template_ ? QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.template.main+xml")
                                    : QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml");
    return writeDocx(out, error);
}

} // namespace jp::write
