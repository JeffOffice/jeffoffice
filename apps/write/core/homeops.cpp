#include "core/homeops.h"

#include "core/editops.h"
#include "core/wordformat.h"
#include "core/writeprops.h"

#include <QDate>
#include <QLocale>
#include <QRegularExpression>
#include <QTextBlock>
#include <QTextDocumentFragment>
#include <algorithm>

namespace jp::write {

namespace {

bool inWord(QChar ch) { return ch.isLetterOrNumber() || ch == QLatin1Char('\'') || ch == QChar(0x2019); }

// A letter written in the other case, one letter for one (a letter that becomes two, like the German sharp s in capitals, stays as it is).
QChar upperOf(QChar ch) { const QString u = QString(ch).toUpper(); return u.size() == 1 ? u.at(0) : ch; }
QChar lowerOf(QChar ch) { const QString l = QString(ch).toLower(); return l.size() == 1 ? l.at(0) : ch; }

bool endsSentence(QChar ch) { return ch == QLatin1Char('.') || ch == QLatin1Char('!') || ch == QLatin1Char('?') || ch == QChar::ParagraphSeparator || ch == QLatin1Char('\n'); }

} // namespace

QString caseOf(const QString &text, CaseMode mode, bool startsSentence, bool startsWord)
{
    QString out = text;
    bool sentence = startsSentence, word = startsWord;
    for (int i = 0; i < out.size(); ++i) {
        const QChar ch = out.at(i);
        QChar r = ch;
        switch (mode) {
        case CaseMode::Lower: r = lowerOf(ch); break;
        case CaseMode::Upper: r = upperOf(ch); break;
        case CaseMode::Toggle: r = ch.isUpper() ? lowerOf(ch) : (ch.isLower() ? upperOf(ch) : ch); break;
        case CaseMode::Capitalize: r = word ? upperOf(ch) : lowerOf(ch); break;
        case CaseMode::Sentence: r = sentence && ch.isLetter() ? upperOf(ch) : lowerOf(ch); break;
        }
        out[i] = r;
        if (ch.isLetter()) sentence = false;
        if (endsSentence(ch)) sentence = true;
        word = !inWord(ch);
    }
    return out;
}

int changeCase(WriteDoc &doc, QTextCursor &c, CaseMode mode)
{
    QTextCursor range = c;
    if (!range.hasSelection()) {
        range.select(QTextCursor::WordUnderCursor);
        if (!range.hasSelection()) return 0;
    }
    const NoteScopeGuard scopeGuard(doc, range);
    if (!intoBody(range)) return 0;
    QTextDocument *d = range.document();
    const int from = range.selectionStart(), to = range.selectionEnd();
    struct Piece { int from; QString text; QTextCharFormat format; };
    QVector<Piece> pieces;
    // A sentence begins at the start of a paragraph or after . ! ?, a word after anything that is not part of one: read from the text before the selection.
    bool sentence = true;
    for (int p = from - 1; p >= 0; --p) {
        const QChar ch = d->characterAt(p);
        if (ch.isLetterOrNumber()) { sentence = false; break; }
        if (endsSentence(ch)) break;
    }
    bool word = from == 0 || !inWord(d->characterAt(from - 1));
    for (QTextBlock b = d->findBlock(from); b.isValid() && b.position() < to; b = b.next()) {
        if (b.position() > from) { sentence = true; word = true; }
        if (isLocked(b)) continue;
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment f = it.fragment();
            const int fs = std::max(f.position(), from), fe = std::min(f.position() + f.length(), to);
            if (fs >= fe) continue;
            if (isAtom(f.charFormat())) { word = true; continue; }
            const QString text = f.text().mid(fs - f.position(), fe - fs);
            const QString out = caseOf(text, mode, sentence, word);
            for (const QChar ch : text) {
                if (ch.isLetter()) sentence = false;
                if (endsSentence(ch)) sentence = true;
            }
            word = !inWord(text.back());
            if (out != text) pieces.push_back({fs, out, f.charFormat()});
        }
    }
    if (pieces.isEmpty()) return 0;
    int changed = 0;
    QTextCursor edit(d);
    edit.beginEditBlock();
    for (int i = int(pieces.size()) - 1; i >= 0; --i) {   // (from the end: a replacement never moves the places of those before it)
        const Piece &p = pieces.at(i);
        QTextCursor t(d);
        t.setPosition(p.from);
        t.setPosition(p.from + int(p.text.size()), QTextCursor::KeepAnchor);
        const QString was = t.selectedText();
        for (int k = 0; k < was.size(); ++k) changed += was.at(k) != p.text.at(k);
        if (doc.trackChanges()) {
            removeSelection(doc, t);
            insertText(doc, t, p.text);
        } else {
            t.insertText(p.text, p.format);
        }
    }
    edit.endEditBlock();
    if (!doc.trackChanges() && c.hasSelection()) {
        c.setPosition(from);
        c.setPosition(to, QTextCursor::KeepAnchor);
    }
    return changed;
}

CaseMode nextCaseInCycle(const QString &selected)
{
    if (selected == caseOf(selected, CaseMode::Lower)) return CaseMode::Capitalize;
    if (selected == caseOf(selected, CaseMode::Capitalize)) return CaseMode::Upper;
    if (selected == caseOf(selected, CaseMode::Upper)) return CaseMode::Lower;
    return CaseMode::Capitalize;
}

QString sortFieldOf(const QString &paragraph, const SortKey &key, QChar separator)
{
    if (separator.isNull()) return paragraph;
    const QStringList fields = paragraph.split(separator);
    const int i = std::max(1, key.field) - 1;
    return i < fields.size() ? fields.at(i) : QString();
}

namespace {

// The first number in the text (digits with a point or commas between them), or none.
bool numberOf(const QString &text, double *value)
{
    static const QRegularExpression re(QStringLiteral("[-+]?\\d[\\d,]*(?:\\.\\d+)?|[-+]?\\.\\d+"));
    const QRegularExpressionMatch m = re.match(text);
    if (!m.hasMatch()) return false;
    QString digits = m.captured(0);
    digits.remove(QLatin1Char(','));
    bool ok = false;
    *value = digits.toDouble(&ok);
    return ok;
}

bool dateOf(const QString &text, QDate *date)
{
    const QString t = text.trimmed();
    for (const char *format : {"yyyy-MM-dd", "M/d/yyyy", "M/d/yy", "MMMM d, yyyy", "MMM d, yyyy", "d MMMM yyyy", "d MMM yyyy", "yyyy/M/d"}) {
        const QString f = QString::fromLatin1(format);
        const QDate d = QLocale::c().toDate(t, f);
        if (d.isValid()) {
            // (A two-digit year is read as this century.)
            *date = d.year() < 1950 && f.endsWith(QLatin1String("yy")) ? d.addYears(100) : d;
            return true;
        }
    }
    return false;
}

// -1, 0, or 1; a field that is not a number (or date) sorts before those that are.
int compareField(const QString &a, const QString &b, SortKey::Type type, bool caseSensitive)
{
    if (type == SortKey::Number) {
        double x = 0, y = 0;
        const bool hx = numberOf(a, &x), hy = numberOf(b, &y);
        if (hx != hy) return hx ? 1 : -1;
        return !hx || x == y ? 0 : (x < y ? -1 : 1);
    }
    if (type == SortKey::Date) {
        QDate x, y;
        const bool hx = dateOf(a, &x), hy = dateOf(b, &y);
        if (hx != hy) return hx ? 1 : -1;
        return !hx || x == y ? 0 : (x < y ? -1 : 1);
    }
    const int r = QString::localeAwareCompare(caseSensitive ? a : a.toLower(), caseSensitive ? b : b.toLower());
    return r < 0 ? -1 : (r > 0 ? 1 : 0);
}

} // namespace

int sortParagraphs(WriteDoc &doc, QTextCursor &c, const SortOptions &options)
{
    if (doc.trackChanges() || options.keys.isEmpty()) return 0;
    QTextCursor sel = c;
    const NoteScopeGuard scopeGuard(doc, sel);
    if (!intoBody(sel)) return 0;
    const QVector<QTextBlock> blocks = selectedBlocks(sel);
    for (const QTextBlock &b : blocks)
        if (isLocked(b) || b.blockFormat().hasProperty(prop::EndnoteBlock)) return 0;
    const int skip = options.header ? 1 : 0;
    if (int(blocks.size()) - skip < 2) return 0;
    QTextDocument *d = sel.document();
    const int keyCount = int(std::min<qsizetype>(3, options.keys.size()));
    struct Item { QTextDocumentFragment text; QStringList fields; };
    QVector<Item> items;
    for (int i = skip; i < blocks.size(); ++i) {
        const QTextBlock &b = blocks.at(i);
        QTextCursor t(d);
        t.setPosition(b.position());
        t.setPosition(b.position() + b.length() - 1, QTextCursor::KeepAnchor);
        Item it{t.selection(), {}};
        const QString plain = t.selectedText().replace(QChar::LineSeparator, QLatin1Char('\n'));
        for (int k = 0; k < keyCount; ++k) it.fields << sortFieldOf(plain, options.keys.at(k), options.separator);
        items.push_back(it);
    }
    QVector<int> order(items.size());
    for (int i = 0; i < order.size(); ++i) order[i] = i;
    std::stable_sort(order.begin(), order.end(), [&](int x, int y) {
        for (int k = 0; k < keyCount; ++k) {
            const SortKey &key = options.keys.at(k);
            const int r = compareField(items.at(x).fields.at(k), items.at(y).fields.at(k), key.type, options.caseSensitive);
            if (r != 0) return key.descending ? r > 0 : r < 0;
        }
        return false;
    });
    bool moved = false;
    for (int i = 0; i < order.size(); ++i) moved = moved || order.at(i) != i;
    if (!moved) return int(items.size());
    // Each place keeps its own paragraph look; the text is put into the places in the new order.
    QTextCursor edit(d);
    edit.beginEditBlock();
    for (int slot = int(items.size()) - 1; slot >= 0; --slot) {
        const QTextBlock target = blocks.at(slot + skip);
        QTextCursor t(d);
        t.setPosition(target.position());
        t.setPosition(target.position() + target.length() - 1, QTextCursor::KeepAnchor);
        t.removeSelectedText();
        t.insertFragment(items.at(order.at(slot)).text);
    }
    edit.endEditBlock();
    return int(items.size());
}

void pasteMerged(WriteDoc &doc, QTextCursor &c, const QTextDocumentFragment &fragment)
{
    // The words of the fragment, a paragraph to a line, and where its emphasis is in them.
    QTextDocument source;
    QTextCursor(&source).insertFragment(fragment);
    struct Mark { int from, to; bool bold, italic, underline, strike; };
    QVector<Mark> marks;
    QString text;
    bool firstBlock = true;
    for (QTextBlock b = source.begin(); b.isValid(); b = b.next()) {
        if (!firstBlock) text += QLatin1Char('\n');
        firstBlock = false;
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextCharFormat cf = it.fragment().charFormat();
            QString piece = it.fragment().text();
            piece.remove(QChar::ObjectReplacementCharacter);
            piece.replace(QChar::LineSeparator, QLatin1Char('\n'));
            piece.replace(QChar::Nbsp, QLatin1Char(' '));
            if (piece.isEmpty()) continue;
            const Mark m{int(text.size()), int(text.size() + piece.size()), cf.fontWeight() >= QFont::DemiBold, cf.fontItalic(), cf.underlineStyle() != QTextCharFormat::NoUnderline, cf.fontStrikeOut()};
            if (m.bold || m.italic || m.underline || m.strike) marks.push_back(m);
            text += piece;
        }
    }
    if (text.isEmpty()) return;
    QTextCursor edit = c;
    edit.beginEditBlock();
    const int start = std::min(c.position(), c.anchor());
    pasteText(doc, c, text);
    // (The text went in one character for a character, a paragraph break for each line: a place in it is where it was in the fragment.)
    if (!doc.trackChanges() && c.document()->characterCount() > start) {
        for (const Mark &m : std::as_const(marks)) {
            QTextCursor t(c.document());
            t.setPosition(start + m.from);
            t.setPosition(std::min(start + m.to, c.document()->characterCount() - 1), QTextCursor::KeepAnchor);
            changeCharFormat(doc, t, [&](QTextCharFormat &f) {
                if (m.bold) f.setFontWeight(QFont::Bold);
                if (m.italic) f.setFontItalic(true);
                if (m.underline) f.setFontUnderline(true);
                if (m.strike) f.setFontStrikeOut(true);
            });
        }
    }
    edit.endEditBlock();
}

} // namespace jp::write
