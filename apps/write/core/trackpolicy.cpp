#include "core/trackpolicy.h"

#include <QCoreApplication>
#include <QSet>
#include <QStringList>

namespace jp::write {

namespace {

bool startsWithAny(const QString &id, const QStringList &prefixes)
{
    for (const QString &p : prefixes)
        if (id.startsWith(p)) return true;
    return false;
}

QSet<QString> setOf(std::initializer_list<const char *> ids)
{
    QSet<QString> out;
    for (const char *id : ids) out.insert(QLatin1String(id));
    return out;
}

} // namespace

TrackPolicy trackPolicyOf(const QString &id, bool *known)
{
    if (known) *known = true;
    // Recorded as tracked changes (the probe in the review tests shows each makes marks, or is held by the code it calls): the look of characters and paragraphs, special characters, breaks,
    // styles of paragraphs, cut and paste, and the table commands (core/tableops.cpp turns down the ones it does not record).
    static const QStringList trackedPrefixes = {QStringLiteral("fmt."), QStringLiteral("ls."), QStringLiteral("insert.special."), QStringLiteral("para.border."), QStringLiteral("style.heading"),
                                                QStringLiteral("style.apply:"), QStringLiteral("symbol.insert:"), QStringLiteral("table."), QStringLiteral("insert.ready."),
                                                QStringLiteral("edit.pasteTable"), QStringLiteral("layout.sectionBreak")};
    static const QSet<QString> tracked = setOf({"edit.cut", "edit.paste", "edit.pasteMerge", "edit.pasteText", "edit.pasteSpecial", "edit.pasteFormat", "edit.pasteListContinue", "edit.pasteListMerge",
        "edit.pasteListNew", "insert.pageBreak", "insert.blankPage", "insert.symbol", "layout.columnBreak", "para.horizontalLine", "para.bordersShading", "para.dialog", "style.normal",
        "refs.addText.1", "refs.addText.2", "refs.addText.3", "refs.addText.none", "insert.cover.band", "insert.cover.classic", "insert.cover.plain", "review.noProof", "review.language",
        "insert.table", "insert.convertTextToTable", "outline.moveUp", "outline.moveDown", "outline.promote", "outline.demote", "outline.toHeading1", "outline.toBody"});
    // Tracked in the text, but not yet in a table's cell.
    static const QSet<QString> inText = setOf({"insert.dropCap.dropped", "insert.dropCap.none", "insert.dropCap.options"});
    // Looking, finding, saving, opening, the way changes are shown, and settings of the document that are not edits of it.
    static const QStringList neutralPrefixes = {QStringLiteral("ribbon"), QStringLiteral("_q_"), QStringLiteral("file."), QStringLiteral("view."), QStringLiteral("zoom."), QStringLiteral("win."),
                                                QStringLiteral("help."), QStringLiteral("find."), QStringLiteral("show."), QStringLiteral("review."), QStringLiteral("outline.level."),
                                                QStringLiteral("hf.go"), QStringLiteral("comment."), QStringLiteral("layout.hyphenation."), QStringLiteral("mail.recipients."),
                                                QStringLiteral("mail.finish."), QStringLiteral("mail.type.")};
    static const QSet<QString> neutral = setOf({"edit.copy", "edit.copyFormat", "edit.repeat", "edit.find", "edit.formatPainter", "edit.goTo", "edit.lastPlace", "edit.selectAll", "edit.selectObjects",
        "edit.clipboardPane", "edit.undo", "edit.redo", "edit.replace", "outline.close", "outline.collapse", "outline.expand", "outline.firstLineOnly", "outline.showFormatting", "outline.showLevel",
        "style.pane", "style.inspector", "style.inspectorPane", "style.manage", "style.modify", "style.new", "style.update", "refs.nextFootnote", "refs.previousFootnote", "refs.nextEndnote",
        "refs.previousEndnote", "refs.showNotes", "refs.citationStyle", "refs.manageSources", "hf.close", "hf.showDocumentText", "hf.previousSection", "hf.nextSection", "fields.toggleCode",
        "fields.toggleAllCodes", "insert.manageBlocks", "insert.saveBlock", "insert.cover.save", "insert.textBox.save", "design.themes.browse", "design.themes.save", "mail.preview",
        "mail.highlight", "mail.first", "mail.last", "mail.next", "mail.prev", "mail.find", "mail.check", "mail.editList", "mail.matchFields", "picture.lockAspect", "shape.createLink",
        "shape.breakLink", "para.sort"});
    // Changes that are not recorded yet, each made as it is with Track Changes off, with a notice (see DESIGN: the slices that record them are listed there): the properties of sections (page setup,
    // columns, margins, line numbers, a watermark, page borders), numbering and indents, notes, generated tables and indexes, fields and links, content controls, headers and footers, the
    // drawn objects and pictures and their properties, and what is made of the merge.
    static const QStringList untrackedPrefixes = {QStringLiteral("layout."), QStringLiteral("design."), QStringLiteral("list."), QStringLiteral("refs."), QStringLiteral("insert.control."),
                                                QStringLiteral("insert.chart."), QStringLiteral("insert.equation"), QStringLiteral("insert.textBox."), QStringLiteral("insert.object."),
                                                QStringLiteral("insert.cover."), QStringLiteral("equation."), QStringLiteral("chart"), QStringLiteral("shape."),
                                                QStringLiteral("picture."), QStringLiteral("pic."), QStringLiteral("arrange"), QStringLiteral("graphics"), QStringLiteral("pn."),
                                                QStringLiteral("hf."), QStringLiteral("mail."), QStringLiteral("fields."), QStringLiteral("outline."), QStringLiteral("para.")};
    static const QSet<QString> untracked = setOf({"insert.header", "insert.footer", "insert.field", "insert.bookmark", "insert.hyperlink", "insert.removeHyperlink", "insert.dateTime",
        "insert.barcode", "insert.icons", "insert.onlinePicture", "insert.picture", "insert.pictureClipboard", "insert.drawingCanvas", "insert.expandShortcut", "insert.textFromFile",
        "edit.pastePicture", "edit.pasteObject", "edit.replaceFonts"});
    if (tracked.contains(id) || startsWithAny(id, trackedPrefixes)) return TrackPolicy::Tracked;
    if (inText.contains(id)) return TrackPolicy::TrackedInText;
    if (neutral.contains(id) || startsWithAny(id, neutralPrefixes)) return TrackPolicy::Neutral;
    if (untracked.contains(id) || startsWithAny(id, untrackedPrefixes)) return TrackPolicy::Untracked;
    // What is not named is Untracked too, and a command that is added later has to be put on one side.
    if (known) *known = false;
    return TrackPolicy::Untracked;
}

QString untrackedNoticeText() { return QCoreApplication::translate("jp::write::Tracking", "This change was not recorded as a tracked change."); }

QString trackedRowsText() { return QCoreApplication::translate("jp::write::Tracking", "This change was not made: it would take part of a row that has tracked changes. Accept or reject them first."); }

} // namespace jp::write
