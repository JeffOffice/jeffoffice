#include "core/blockops.h"

#include "core/editops.h"
#include "core/commentops.h"
#include "core/fieldops.h"
#include "core/refops.h"
#include "core/reviewops.h"
#include "core/tableops.h"
#include "core/writeprops.h"
#include "core/styleops.h"
#include "text/textprops.h"

#include <QCoreApplication>
#include <QRegularExpression>
#include <QSet>
#include <QTextBlock>
#include <QTextDocument>
#include <algorithm>

namespace jp::write {

namespace {


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

// Whether the text from `from` to `to` has a table in it, and whether it has the reference of a footnote or an endnote.
void whatIsIn(QTextDocument *d, int from, int to, bool *table, bool *note)
{
    for (QTextBlock b = d->findBlock(from); b.isValid() && b.position() < to; b = b.next()) {
        if (inTable(b)) *table = true;
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextCharFormat cf = it.fragment().charFormat();
            if (!cf.hasProperty(prop::Atom)) continue;
            const QByteArray bytes = storedBytes(cf, prop::Atom);
            if (bytes.contains("footnoteReference") || bytes.contains("endnoteReference")) *note = true;
        }
    }
}

// The value of an attribute of a kept element (a bookmark's name is letters, digits, and underscores: nothing to unescape).
QByteArray attributeValue(const QByteArray &raw, const char *name)
{
    const QByteArray key = QByteArray("w:") + name + "=\"";
    const qsizetype at = raw.indexOf(key);
    if (at < 0) return QByteArray();
    const qsizetype end = raw.indexOf('"', at + key.size());
    return end < 0 ? QByteArray() : raw.mid(at + key.size(), end - at - key.size());
}

// The bookmarks that came with a block, from `from` to `to` in the text: each has a name no other bookmark of the document has (a name taken gets "_2", "_3", and so on after it, within the 40
// characters a name may have), and numbers of their own, so that putting a block in twice is two bookmarks and never one that is two places.
void giveBlockBookmarksNamesOfTheirOwn(WriteDoc &doc, int from, int to)
{
    struct Mark {
        int at = 0;
        office::wml::Inline atom;
        QTextCharFormat format;
    };
    QTextDocument *d = doc.story();
    QVector<Mark> pasted;
    QSet<QString> taken;
    long highest = 0;
    static const QRegularExpression idPattern(QStringLiteral("(w:id=\")[^\"]*(\")")), namePattern(QStringLiteral("(w:name=\")([^\"]*)(\")"));
    for (QTextBlock b = d->begin(); b.isValid(); b = b.next())
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment f = it.fragment();
            const QTextCharFormat cf = f.charFormat();
            if (!cf.hasProperty(prop::Atom)) continue;
            const QByteArray bytes = storedBytes(cf, prop::Atom);
            if (!bytes.contains("bookmark")) continue;
            const office::wml::Inline atom = unpackAtom(bytes);
            const bool start = atom.tag == QLatin1String("bookmarkStart");
            if (!start && atom.tag != QLatin1String("bookmarkEnd")) continue;
            highest = std::max(highest, atom.key.toLong());
            const QString text = f.text();
            for (int k = 0; k < text.size(); ++k) {
                const int at = f.position() + k;
                if (text[k] != QChar::ObjectReplacementCharacter) continue;
                if (at >= from && at < to) pasted.push_back({at, atom, cf});
                else if (start) taken.insert(QString::fromUtf8(attributeValue(atom.raw, "name")).toLower());
            }
        }
    if (pasted.isEmpty()) return;
    QHash<QString, QString> nameOf, idOf;   // by the id the mark had
    for (const Mark &m : std::as_const(pasted)) {
        if (!idOf.contains(m.atom.key)) idOf.insert(m.atom.key, QString::number(++highest));
        if (m.atom.tag != QLatin1String("bookmarkStart")) continue;
        const QString name = QString::fromUtf8(attributeValue(m.atom.raw, "name"));
        QString now = name;
        for (int n = 2; taken.contains(now.toLower()) && n < 100000; ++n) now = name.left(kMostBookmarkNameLength - 1 - int(QString::number(n).size())) + QLatin1Char('_') + QString::number(n);
        taken.insert(now.toLower());
        nameOf.insert(m.atom.key, now);
    }
    for (const Mark &m : std::as_const(pasted)) {
        office::wml::Inline atom = m.atom;
        QString raw = QString::fromUtf8(atom.raw);
        raw.replace(idPattern, QLatin1String("\\1") + idOf.value(atom.key) + QLatin1String("\\2"));
        if (atom.tag == QLatin1String("bookmarkStart") && nameOf.contains(atom.key)) {
            QString escaped = nameOf.value(atom.key);
            escaped.replace(QLatin1Char('&'), QLatin1String("&amp;")).replace(QLatin1Char('<'), QLatin1String("&lt;")).replace(QLatin1Char('"'), QLatin1String("&quot;"));
            raw.replace(namePattern, QLatin1String("\\1") + escaped + QLatin1String("\\3"));
        }
        atom.key = idOf.value(atom.key);
        atom.raw = raw.toUtf8();
        QTextCharFormat nf = m.format;
        setStored(nf, prop::Atom, packAtom(atom));
        QTextCursor t(d);
        t.setPosition(m.at);
        t.setPosition(m.at + 1, QTextCursor::KeepAnchor);
        t.setCharFormat(nf);
    }
}

} // namespace

QByteArray blockFromSelection(const WriteDoc &doc, const QTextCursor &selection, QString *problem, QStringList *leftOut)
{
    auto fail = [&](const QString &why) {
        if (problem) *problem = why;
        return QByteArray();
    };
    if (!selection.hasSelection()) return fail(QCoreApplication::translate("jp::write::Blocks", "Select what you want to save first."));
    const Clip clip = copySelection(doc, selection, true);   // (a table with a tracked change is kept as a table: the changes are accepted below)
    if (clip.isEmpty()) return fail(QCoreApplication::translate("jp::write::Blocks", "There is nothing in the selection that can be saved as a block."));
    std::unique_ptr<WriteDoc> blank = WriteDoc::blank();
    // (The styles first, so the paragraphs pasted find them; a style the blank document has under the same name is kept as it is.)
    importStyles(*blank, doc, stylesBetween(selection.document(), selection.selectionStart(), selection.selectionEnd()), false);
    QTextCursor into(blank->story());
    pasteClip(*blank, into, clip);
    // What a block does not carry is taken out, so that nothing in it points at what it lacks (a comment's words, another person's tracked words, a note): comments are left out, changes are
    // accepted (inserted words stay, deleted words go), and the person is told before the block is saved.
    if (leftOut) {
        bool table = false, note = false;
        whatIsIn(selection.document(), selection.selectionStart(), selection.selectionEnd(), &table, &note);
        if (clip.tablesAsWords > 0) leftOut->push_back(QCoreApplication::translate("jp::write::Blocks", "Part of a table, or a table that is too large, is saved as its words, one paragraph for each cell, and not as a table."));
        if (note) leftOut->push_back(QCoreApplication::translate("jp::write::Blocks", "Footnotes and endnotes are left out."));
    }
    if (deleteAllComments(*blank) > 0 && leftOut) leftOut->push_back(QCoreApplication::translate("jp::write::Blocks", "Comments are left out."));
    if (acceptAll(*blank) > 0 && leftOut) leftOut->push_back(QCoreApplication::translate("jp::write::Blocks", "Tracked changes are accepted: the words that were inserted are kept, and the words that were deleted are not."));
    if (blank->story()->characterCount() <= 1) return fail(QCoreApplication::translate("jp::write::Blocks", "There is nothing in the selection that can be saved as a block."));
    QString err;
    const QByteArray bytes = blank->toDocx(&err);
    if (bytes.isEmpty()) return fail(QCoreApplication::translate("jp::write::Blocks", "The selection could not be saved as a block."));
    if (bytes.size() > BlockLibrary::kMostEntryBytes) return fail(QCoreApplication::translate("jp::write::Blocks", "The selection is too large to save as a block."));
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
    if (block.isEmpty() || block.size() > BlockLibrary::kMostEntryBytes) return fail(QCoreApplication::translate("jp::write::Blocks", "This block cannot be read."));
    QString err;
    std::unique_ptr<WriteDoc> entry = WriteDoc::fromDocx(block, &err);
    if (!entry) return fail(QCoreApplication::translate("jp::write::Blocks", "This block cannot be read."));
    QTextCursor all(entry->story());
    all.select(QTextCursor::Document);
    const Clip clip = copySelection(*entry, all);
    if (clip.isEmpty()) return fail(QCoreApplication::translate("jp::write::Blocks", "This block is empty."));
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
    if (story == doc.story() && to > from) giveBlockBookmarksNamesOfTheirOwn(doc, from, to);
    if (how == BlockInsert::Page) insertPageBreak(doc, c);
    c.endEditBlock();
    if (to <= from && story == doc.story()) return fail(QCoreApplication::translate("jp::write::Blocks", "The block could not be put in here."));
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
