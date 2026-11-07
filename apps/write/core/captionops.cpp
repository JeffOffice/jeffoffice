#include "core/captionops.h"

#include "core/editops.h"
#include "core/fieldops.h"
#include "core/refops.h"
#include "core/tocops.h"
#include "core/writeprops.h"
#include "flow/flowdocument.h"
#include "flow/tabledata.h"
#include "text/textprops.h"

#include <QCoreApplication>
#include <QTextBlock>
#include <QTextDocument>
#include <algorithm>

namespace jp::write {

QStringList captionLabelChoices(WriteDoc &doc)
{
    QStringList out = {QStringLiteral("Figure"), QStringLiteral("Table"), QStringLiteral("Equation")};
    for (const QString &l : captionLabels(doc))
        if (!out.contains(l, Qt::CaseInsensitive)) out << l;
    return out;
}

namespace {

// The paragraphs of the table the block is in (its first and last, with any table inside it), or the block itself.
std::pair<QTextBlock, QTextBlock> extentOf(WriteDoc &doc, const QTextBlock &block)
{
    const flow::TableStore &store = doc.flow().tables;
    const flow::CellRecord *own = store.get(block.blockFormat().intProperty(flow::fp::Cell));
    if (!own) return {block, block};
    const std::shared_ptr<const flow::TableData> table = own->table;
    auto inside = [&](const QTextBlock &b) {
        int key = b.blockFormat().intProperty(flow::fp::Cell);
        for (int steps = 0; key != 0 && steps < 64; ++steps) {
            const flow::CellRecord *r = store.get(key);
            if (!r) return false;
            if (r->table == table) return true;
            key = r->parent;
        }
        return false;
    };
    QTextBlock first = block, last = block;
    while (first.previous().isValid() && inside(first.previous())) first = first.previous();
    while (last.next().isValid() && inside(last.next())) last = last.next();
    return {first, last};
}

QString captionStyleId(WriteDoc &doc)
{
    for (const office::wml::Style &st : doc.styleSheet().styles)
        if (st.type == QLatin1String("paragraph") && st.name.compare(QLatin1String("caption"), Qt::CaseInsensitive) == 0) return st.id;
    const QString id = QStringLiteral("Caption");
    const quint64 sheet = doc.styleSheetRevision();
    if (doc.ensureStyle(id) && doc.styleSheetRevision() != sheet) doc.markStyleStep();
    return id;
}

} // namespace

bool insertCaption(WriteDoc &doc, QTextCursor &c, const CaptionOptions &o, QString *problem)
{
    QTextDocument *story = doc.story();
    const QString label = o.label.simplified();
    if (c.document() != story || inEndnoteWords(c) || !intoBody(c)) {
        if (problem) *problem = QCoreApplication::translate("jp::write::Captions", "A caption goes in the text, not in a header, a footer, or a note.");
        return false;
    }
    if (label.isEmpty() && !o.excludeLabel) {
        if (problem) *problem = QCoreApplication::translate("jp::write::Captions", "Give the caption a label, such as Figure or Table.");
        return false;
    }
    QString id = label;
    id.remove(QLatin1Char(' ')).remove(QLatin1Char('"'));
    if (id.isEmpty()) id = QStringLiteral("Figure");
    WriteDoc::StepGroup group(doc);
    EditScope scope(doc, story);
    const QString style = captionStyleId(doc);
    const auto [first, last] = extentOf(doc, story->findBlock(c.selectionEnd()));
    const QTextBlock anchor = o.above ? first : last;
    QTextCursor at(story);
    int begin = 0;
    if (o.above) {
        // (A block split at its start leaves the empty one before it with the properties the old one had: the old one's are put back on the second, the first is the caption's below.)
        const QTextBlockFormat oldFormat = anchor.blockFormat();
        const QTextCharFormat oldLook = anchor.charFormat();
        const int blockStart = anchor.position();
        at.setPosition(blockStart);
        at.insertBlock(oldFormat, oldLook);
        begin = blockStart;
    } else {
        at.setPosition(anchor.position() + anchor.length() - 1);
        at.insertBlock(doc.styleBlockFormat(style), doc.styleCharFormat(style));
        begin = at.block().position();
    }
    // A paragraph inserted above takes the properties of the paragraph it was cut from; the caption's own are the style's.
    {
        QTextCursor fix(story);
        fix.setPosition(begin);
        QTextBlockFormat bf = doc.styleBlockFormat(style);
        for (const int key : {int(flow::fp::Cell), int(flow::fp::Frame)}) bf.clearProperty(key);
        fix.setBlockFormat(bf);
        fix.setBlockCharFormat(doc.styleCharFormat(style));
    }
    QTextCursor put(story);
    put.setPosition(begin);
    const QTextCharFormat look = doc.styleCharFormat(style);
    bool ok = true;
    if (!o.excludeLabel) put.insertText(label + QLatin1Char(' '), look);
    if (o.chapterLevel > 0) {
        ok = ok && insertField(doc, put, QStringLiteral("STYLEREF %1 \\s").arg(std::clamp(o.chapterLevel, 1, 9)), QStringLiteral("1"));
        put.insertText(o.separator, look);
    }
    ok = ok && insertField(doc, put, QStringLiteral("SEQ %1 \\* ARABIC%2").arg(id, o.chapterLevel > 0 ? QStringLiteral(" \\s %1").arg(std::clamp(o.chapterLevel, 1, 9)) : QString()), QStringLiteral("1"));
    if (!o.text.isEmpty()) put.insertText(o.text, look);
    if (!ok) return false;
    // The captions after this one count on from it.
    UpdateOptions u;
    u.skip = [](const QString &name) { return name != QLatin1String("SEQ") && name != QLatin1String("STYLEREF"); };
    updateFields(doc, u);
    c.setPosition(story->findBlock(begin).position() + story->findBlock(begin).length() - 1);
    return true;
}

QStringList paragraphStyleNames(WriteDoc &doc)
{
    QStringList names;
    for (const office::wml::Style &s : doc.styleSheet().styles)
        if (s.type == QLatin1String("paragraph") && !s.semiHidden && !s.name.isEmpty() && !names.contains(s.name)) names << s.name;
    names.sort(Qt::CaseInsensitive);
    return names;
}

QString figuresCode(const FiguresOptions &o)
{
    QString id = o.label.simplified();
    id.remove(QLatin1Char('"'));
    const QString style = QString(o.style).simplified().remove(QLatin1Char('"')).remove(QLatin1Char(',')).remove(QLatin1Char(';'));
    if (!style.isEmpty()) {
        QString byStyle = QStringLiteral(" TOC \\h \\z \\t \"%1,1\"").arg(style);
        if (!o.hyperlinks) byStyle.remove(QStringLiteral(" \\h"));
        if (!o.pageNumbers) byStyle += QStringLiteral(" \\n");
        else if (!o.rightAlign) byStyle += QStringLiteral(" \\p \" \"");
        return byStyle + QLatin1Char(' ');
    }
    QString code = o.includeLabel ? QStringLiteral(" TOC \\h \\z \\c \"%1\"").arg(id) : QStringLiteral(" TOC \\h \\z \\a \"%1\"").arg(id);
    if (!o.hyperlinks) code.remove(QStringLiteral(" \\h"));
    if (!o.pageNumbers) code += QStringLiteral(" \\n");
    else if (!o.rightAlign) code += QStringLiteral(" \\p \" \"");
    return code + QLatin1Char(' ');
}

bool insertTableOfFigures(WriteDoc &doc, QTextCursor &c, const FiguresOptions &options, bool *truncated)
{
    return insertTableFromCode(doc, c, figuresCode(options), options.leader, truncated);
}

} // namespace jp::write
