#include "core/blockops.h"

#include "core/editops.h"
#include "core/fieldops.h"
#include "core/styleops.h"
#include "text/textprops.h"

#include <QCoreApplication>
#include <QSet>
#include <QTextBlock>
#include <QTextDocument>
#include <algorithm>

namespace jp::write {

namespace {

QString tr(const char *text) { return QCoreApplication::translate("jp::write::Blocks", text); }

// The paragraph and character styles the text from `from` to `to` uses.
QStringList stylesBetween(QTextDocument *d, int from, int to)
{
    QSet<QString> ids;
    for (QTextBlock b = d->findBlock(from); b.isValid() && b.position() < to; b = b.next()) {
        const QString own = b.blockFormat().stringProperty(tp::StyleName);
        if (!own.isEmpty()) ids.insert(own);
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QString character = it.fragment().charFormat().stringProperty(tp::CharStyle);
            if (!character.isEmpty()) ids.insert(character);
        }
    }
    QStringList out(ids.begin(), ids.end());
    out.sort();
    return out;
}

} // namespace

QByteArray blockFromSelection(const WriteDoc &doc, const QTextCursor &selection, QString *problem)
{
    auto fail = [&](const QString &why) {
        if (problem) *problem = why;
        return QByteArray();
    };
    if (!selection.hasSelection()) return fail(tr("Select what you want to save first."));
    const Clip clip = copySelection(doc, selection);
    if (clip.isEmpty()) return fail(tr("There is nothing in the selection that can be saved as a block."));
    std::unique_ptr<WriteDoc> blank = WriteDoc::blank();
    // (The styles first, so the paragraphs pasted find them; a style the blank document has under the same name is kept as it is.)
    importStyles(*blank, doc, stylesBetween(selection.document(), selection.selectionStart(), selection.selectionEnd()), false);
    QTextCursor into(blank->story());
    pasteClip(*blank, into, clip);
    if (blank->story()->characterCount() <= 1) return fail(tr("There is nothing in the selection that can be saved as a block."));
    QString err;
    const QByteArray bytes = blank->toDocx(&err);
    if (bytes.isEmpty()) return fail(tr("The selection could not be saved as a block."));
    if (bytes.size() > BlockLibrary::kMostEntryBytes) return fail(tr("The selection is too large to save as a block."));
    return bytes;
}

QString suggestedBlockName(const QTextCursor &selection)
{
    QString text = selection.selection().toPlainText().simplified();
    text.remove(QChar::ObjectReplacementCharacter);
    text = text.simplified();
    if (text.size() > 40) {
        text = text.left(40);
        const int space = text.lastIndexOf(QLatin1Char(' '));
        if (space > 12) text = text.left(space);
    }
    return BlockLibrary::cleanName(text);
}

bool insertBlock(WriteDoc &doc, QTextCursor &c, const QByteArray &block, BlockInsert how, QString *problem)
{
    auto fail = [&](const QString &why) {
        if (problem) *problem = why;
        return false;
    };
    if (block.isEmpty() || block.size() > BlockLibrary::kMostEntryBytes) return fail(tr("This block cannot be read."));
    QString err;
    std::unique_ptr<WriteDoc> entry = WriteDoc::fromDocx(block, &err);
    if (!entry) return fail(tr("This block cannot be read."));
    QTextCursor all(entry->story());
    all.select(QTextCursor::Document);
    const Clip clip = copySelection(*entry, all);
    if (clip.isEmpty()) return fail(tr("This block is empty."));
    const QStringList styles = stylesBetween(entry->story(), 0, entry->story()->characterCount());
    QTextDocument *story = c.document();
    c.beginEditBlock();
    importStyles(doc, *entry, styles, false);
    if (c.hasSelection()) removeSelection(doc, c);
    const bool own = how != BlockInsert::Content;
    // (In a paragraph of its own: the caret is made an empty paragraph between what was before it and what was after it, so the block's first paragraph keeps its own format.)
    if (own) {
        if (!c.atBlockStart()) insertParagraph(doc, c);
        if (!c.atBlockEnd()) {
            insertParagraph(doc, c);
            c.movePosition(QTextCursor::PreviousBlock);
        }
    }
    if (how == BlockInsert::Page && c.position() > 0) insertPageBreak(doc, c);
    const int from = c.position();
    pasteClip(doc, c, clip);
    const int to = c.position();
    if (how == BlockInsert::Page) insertPageBreak(doc, c);
    c.endEditBlock();
    if (to <= from && story == doc.story()) return fail(tr("The block could not be put in here."));
    if (story == doc.story()) {
        UpdateOptions o;
        o.from = from;
        o.to = c.position();
        updateFields(doc, o);
    }
    return true;
}

std::optional<ShortcutMatch> shortcutBeforeCaret(const QTextCursor &c, const QVector<BlockInfo> &shortcuts)
{
    const QTextBlock block = c.block();
    const QString before = block.text().left(c.position() - block.position());
    std::optional<ShortcutMatch> best;
    for (const BlockInfo &s : shortcuts) {
        const int n = int(s.name.size());
        if (n == 0 || n > before.size() || (best && best->info.name.size() >= n)) continue;
        if (before.right(n).compare(s.name, Qt::CaseInsensitive) != 0) continue;
        if (n < before.size() && before.at(before.size() - n - 1).isLetterOrNumber() && s.name.at(0).isLetterOrNumber()) continue;
        ShortcutMatch m;
        m.info = s;
        m.start = c.position() - n;
        best = m;
    }
    return best;
}

bool expandShortcut(WriteDoc &doc, QTextCursor &c, BlockLibrary &library, QString *problem)
{
    if (c.hasSelection()) return false;
    const auto match = shortcutBeforeCaret(c, library.list(kShortcutsGallery));
    if (!match) return false;
    const QByteArray bytes = library.data(match->info.id);
    QTextCursor at(c);
    at.setPosition(match->start);
    at.setPosition(c.position(), QTextCursor::KeepAnchor);
    if (!insertBlock(doc, at, bytes, match->info.insertAs, problem)) return false;
    c = at;
    return true;
}

} // namespace jp::write
