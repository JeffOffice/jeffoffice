#include "core/autoops.h"

#include "core/editops.h"
#include "core/linkops.h"
#include "core/writeprops.h"
#include "text/autocorrect.h"
#include "text/textprops.h"

#include <QTextBlock>

namespace jp::write {

namespace {

bool isSeparator(QChar c) { return c.isSpace() || QStringLiteral(".,;:!?").contains(c); }

// The text of the paragraph up to the separator that was just typed (the caret is after it).
QString textBeforeSeparator(const QTextCursor &c)
{
    const QTextBlock b = c.block();
    return b.text().left(c.position() - 1 - b.position());
}

// [from, to) of the paragraph, as a selection of the story, replaced by `with`.
QTextCursor rangeOf(QTextDocument *story, const QTextBlock &b, int from, int to)
{
    QTextCursor r(story);
    r.setPosition(b.position() + from);
    r.setPosition(b.position() + to, QTextCursor::KeepAnchor);
    return r;
}

} // namespace

void typeText(WriteDoc &doc, QTextCursor &c, const QString &text, const AutoOptions &options)
{
    const bool any = options.correct || options.quotes || options.format;
    if (!any || text.size() != 1 || c.hasSelection() || c.document() != doc.story()) {
        insertText(doc, c, text);
        return;
    }
    QString typed = text;
    if (options.quotes && (typed == QLatin1String("\"") || typed == QLatin1String("'"))) {
        const QTextBlock b = c.block();
        const int rel = c.position() - b.position();
        typed = autotext::smartQuote(typed, rel > 0 ? b.text().at(rel - 1) : QChar());
    }
    const int at = c.position();
    insertText(doc, c, typed);
    if (c.position() != at + 1 || !isSeparator(typed[0])) return;
    QTextDocument *story = doc.story();
    // A list: "1." or "*" and a space, at the start of a paragraph that is not in a list. The space goes with the mark.
    if (options.format && typed == QLatin1String(" ") && listKindOf(doc, c.block()) == ListKind::None) {
        const auto list = autotext::listStart(textBeforeSeparator(c));
        if (list && (!list->numbered || (list->number == 1 && !list->paren))) {
            const WriteDoc::StepGroup group(doc);
            QTextCursor mark = rangeOf(story, c.block(), 0, list->length + 1);
            removeSelection(doc, mark);
            c.setPosition(mark.position());
            toggleList(doc, c, list->numbered);
            return;
        }
    }
    const WriteDoc::StepGroup group(doc);
    if (options.correct) {
        const QString before = textBeforeSeparator(c);
        if (const auto fix = autotext::correctWord(before, int(before.size()), options.quotes)) {
            QTextCursor r = rangeOf(story, c.block(), fix->from, fix->to);
            insertText(doc, r, fix->with);
        }
    }
    if (!options.format) return;
    if (const auto fix = autotext::formatWord(textBeforeSeparator(c))) {
        QTextCursor r = rangeOf(story, c.block(), fix->from, fix->to);
        if (fix->superscript) changeCharFormat(doc, r, [](QTextCharFormat &f) { f.setVerticalAlignment(QTextCharFormat::AlignSuperScript); });
        else insertText(doc, r, fix->with);
    }
    if (typed != QLatin1String(" ")) return;
    if (const auto link = autotext::linkWord(textBeforeSeparator(c))) {
        QTextCursor r = rangeOf(story, c.block(), link->from, link->to);
        if (!hyperlinkAt(doc, r.selectionStart() + 1).valid()) insertHyperlink(doc, r, link->with, QString(), QString(), QString());
    }
    if (const auto mark = autotext::markWord(textBeforeSeparator(c))) {
        const QTextBlock b = c.block();
        QTextCursor inner = rangeOf(story, b, mark->from + 1, mark->to - 1);
        if (mark->mark == QLatin1Char('*')) {
            if (!allSelected(inner, [](const QTextCharFormat &f) { return f.fontWeight() >= QFont::DemiBold; })) toggleBold(doc, inner);
        } else if (!allSelected(inner, [](const QTextCharFormat &f) { return f.underlineStyle() != QTextCharFormat::NoUnderline; })) {
            toggleUnderline(doc, inner);
        }
        // The two marks go, the last first.
        QTextCursor closing = rangeOf(story, b, mark->to - 1, mark->to);
        removeSelection(doc, closing);
        QTextCursor opening = rangeOf(story, b, mark->from, mark->from + 1);
        removeSelection(doc, opening);
    }
}

bool enterAfterRule(WriteDoc &doc, QTextCursor &c, const AutoOptions &options)
{
    if (!options.format || c.hasSelection() || c.document() != doc.story() || !c.atBlockEnd()) return false;
    const QTextBlock b = c.block();
    const auto line = autotext::ruleLine(b.text());
    if (!line || isLocked(b) || listKindOf(doc, b) != ListKind::None) return false;
    const WriteDoc::StepGroup group(doc);
    QTextCursor words = rangeOf(doc.story(), b, 0, int(b.text().size()));
    removeSelection(doc, words);
    c.setPosition(b.position());
    tp::ParaBox box;
    box.bottom.style = *line == QLatin1Char('=') ? QStringLiteral("double") : QStringLiteral("single");
    box.bottom.eighths = *line == QLatin1Char('_') ? 18 : 6;
    box.bottom.space = 1;
    changeBlockFormat(doc, c, [&](QTextBlockFormat &bf) { bf.setProperty(tp::Borders, box.spec()); });
    insertParagraph(doc, c);
    changeBlockFormat(doc, c, [](QTextBlockFormat &bf) { bf.clearProperty(tp::Borders); });
    return true;
}

} // namespace jp::write
