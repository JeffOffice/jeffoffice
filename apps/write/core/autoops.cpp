#include "core/autoops.h"

#include "core/editops.h"
#include "core/linkops.h"
#include "core/wordformat.h"
#include "core/writeprops.h"
#include "text/autocorrect.h"
#include "text/textprops.h"

#include <QTextBlock>

namespace jp::write {

namespace {

bool isSeparator(QChar c) { return c.isSpace() || QStringLiteral(".,;:!?").contains(c); }

// The text of a paragraph that is shown, up to a place: what a tracked deletion holds is not in it. `at` says where in the story each of its characters is.
struct Visible {
    QString text;
    QVector<int> at;
    // [from, to) of the text as a range of the story.
    QTextCursor range(QTextDocument *story, int from, int to) const
    {
        QTextCursor r(story);
        r.setPosition(at[from]);
        r.setPosition(to > from ? at[to - 1] + 1 : at[from], QTextCursor::KeepAnchor);
        return r;
    }
};

Visible visibleBefore(const QTextBlock &b, int end)
{
    Visible v;
    int deleted = 0;
    for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
        const QTextFragment f = it.fragment();
        if (!f.isValid() || f.position() >= end) break;
        const QTextCharFormat cf = f.charFormat();
        const QString words = f.text();
        const bool atom = isAtom(cf);
        office::wml::Inline a;
        if (atom) a = unpackAtom(storedBytes(cf, prop::Atom));
        for (int i = 0; i < words.size() && f.position() + i < end; ++i) {
            if (atom && a.kind == office::wml::Inline::ParaAtom && (a.tag == QLatin1String("del") || a.tag == QLatin1String("moveFrom"))) {
                if (a.role == office::wml::Inline::Open) ++deleted;
                else if (a.role == office::wml::Inline::Close && deleted > 0) --deleted;
            }
            if (deleted > 0) continue;
            // (The marks that open and close a tracked insertion are no text either.)
            if (atom && a.kind == office::wml::Inline::ParaAtom && (a.tag == QLatin1String("ins") || a.tag == QLatin1String("del") || a.tag == QLatin1String("moveFrom") || a.tag == QLatin1String("moveTo"))) continue;
            v.text += words[i];
            v.at.push_back(f.position() + i);
        }
    }
    return v;
}

// The shown text of the paragraph up to the separator that was just typed (the caret is after it).
Visible beforeSeparator(const QTextCursor &c) { return visibleBefore(c.block(), c.position() - 1); }

} // namespace

void typeText(WriteDoc &doc, QTextCursor &c, const QString &text, const AutoOptions &options)
{
    const bool any = options.correct || options.quotes || options.format || options.links;
    if (!any || text.size() != 1 || c.document() != doc.story()) {
        insertText(doc, c, text);
        return;
    }
    QString typed = text;
    if (options.quotes && (typed == QLatin1String("\"") || typed == QLatin1String("'"))) {
        // (Over a selection, the quote is chosen by what is before the selection.)
        const Visible before = visibleBefore(c.block(), c.selectionStart());
        typed = autotext::smartQuote(typed, before.text.isEmpty() ? QChar() : before.text.back());
    }
    if (c.hasSelection()) {
        insertText(doc, c, typed);
        return;
    }
    const int at = c.position();
    insertText(doc, c, typed);
    if (c.position() != at + 1 || !isSeparator(typed[0])) return;
    QTextDocument *story = doc.story();
    // A list: "1." or "*" and a space, at the start of a paragraph that is not in a list. The space goes with the mark.
    if (options.format && typed == QLatin1String(" ") && listKindOf(doc, c.block()) == ListKind::None) {
        const Visible before = beforeSeparator(c);
        const auto list = autotext::listStart(before.text);
        if (list && (!list->numbered || (list->number == 1 && !list->paren))) {
            const WriteDoc::StepGroup group(doc);
            QTextCursor mark(story);
            mark.setPosition(c.block().position());
            mark.setPosition(c.position(), QTextCursor::KeepAnchor);
            removeSelection(doc, mark);
            c.setPosition(mark.position());
            toggleList(doc, c, list->numbered);
            return;
        }
    }
    const WriteDoc::StepGroup group(doc);
    if (options.correct) {
        const Visible before = beforeSeparator(c);
        if (const auto fix = autotext::correctWord(before.text, int(before.text.size()), options.quotes)) {
            QTextCursor r = before.range(story, fix->from, fix->to);
            insertText(doc, r, fix->with);
        }
    }
    if (options.format) {
        const Visible before = beforeSeparator(c);
        if (const auto fix = autotext::formatWord(before.text)) {
            QTextCursor r = before.range(story, fix->from, fix->to);
            if (fix->superscript) changeCharFormat(doc, r, [](QTextCharFormat &f) { f.setVerticalAlignment(QTextCharFormat::AlignSuperScript); });
            else insertText(doc, r, fix->with);
        }
    }
    if (typed != QLatin1String(" ")) return;
    if (options.links) {
        const Visible before = beforeSeparator(c);
        if (const auto link = autotext::linkWord(before.text)) {
            QTextCursor r = before.range(story, link->from, link->to);
            if (!hyperlinkAt(doc, r.selectionStart() + 1).valid()) insertHyperlink(doc, r, link->with, QString(), QString(), QString());
        }
    }
    if (options.format) {
        const Visible before = beforeSeparator(c);
        if (const auto mark = autotext::markWord(before.text)) {
            QTextCursor inner = before.range(story, mark->from + 1, mark->to - 1);
            if (mark->mark == QLatin1Char('*')) {
                if (!allSelected(inner, [](const QTextCharFormat &f) { return f.fontWeight() >= QFont::DemiBold; })) toggleBold(doc, inner);
            } else if (!allSelected(inner, [](const QTextCharFormat &f) { return f.underlineStyle() != QTextCharFormat::NoUnderline; })) {
                toggleUnderline(doc, inner);
            }
            // The two marks go, the last first.
            QTextCursor closing = before.range(story, mark->to - 1, mark->to);
            QTextCursor opening = before.range(story, mark->from, mark->from + 1);
            removeSelection(doc, closing);
            removeSelection(doc, opening);
        }
    }
}

bool enterAfterRule(WriteDoc &doc, QTextCursor &c, const AutoOptions &options)
{
    if (!options.format || c.hasSelection() || c.document() != doc.story() || !c.atBlockEnd()) return false;
    const QTextBlock b = c.block();
    const auto line = autotext::ruleLine(b.text());
    if (!line || isLocked(b) || listKindOf(doc, b) != ListKind::None) return false;
    const WriteDoc::StepGroup group(doc);
    const int start = b.position();
    QTextCursor words(doc.story());
    words.setPosition(start);
    words.setPosition(start + int(b.text().size()), QTextCursor::KeepAnchor);
    removeSelection(doc, words);
    c.setPosition(start);
    // The paragraph below is made first: it has what the style and the paragraph give a new one, and the line is added to the paragraph above it alone.
    insertParagraph(doc, c);
    QTextCursor above(doc.story());
    above.setPosition(start);
    changeBlockFormat(doc, above, [&](QTextBlockFormat &bf) {
        tp::ParaBox box = tp::ParaBox::parse(bf.stringProperty(tp::Borders));
        box.bottom.style = *line == QLatin1Char('=') ? QStringLiteral("double") : QStringLiteral("single");
        box.bottom.eighths = *line == QLatin1Char('_') ? 18 : 6;
        box.bottom.space = 1;
        bf.setProperty(tp::Borders, box.spec());
    });
    return true;
}

} // namespace jp::write
