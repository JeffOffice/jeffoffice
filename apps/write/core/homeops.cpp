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

// A key's field read once for the sort: as text (in the case the sort compares it in), and as a number or a date.
struct ParsedField {
    QString text;
    double number = 0;
    qint64 day = 0;
    bool hasNumber = false, hasDate = false;
};

ParsedField parseField(const QString &field, SortKey::Type type, bool caseSensitive)
{
    ParsedField p;
    if (type == SortKey::Number) {
        p.hasNumber = numberOf(field, &p.number);
    } else if (type == SortKey::Date) {
        QDate d;
        p.hasDate = dateOf(field, &d);
        if (p.hasDate) p.day = d.toJulianDay();
    } else {
        p.text = caseSensitive ? field : field.toLower();
    }
    return p;
}

// -1, 0, or 1; a field that is not a number (or date) sorts before those that are.
int compareParsed(const ParsedField &a, const ParsedField &b, SortKey::Type type)
{
    if (type == SortKey::Number) {
        if (a.hasNumber != b.hasNumber) return a.hasNumber ? 1 : -1;
        return !a.hasNumber || a.number == b.number ? 0 : (a.number < b.number ? -1 : 1);
    }
    if (type == SortKey::Date) {
        if (a.hasDate != b.hasDate) return a.hasDate ? 1 : -1;
        return !a.hasDate || a.day == b.day ? 0 : (a.day < b.day ? -1 : 1);
    }
    const int r = QString::localeAwareCompare(a.text, b.text);
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
    if (blocks.size() > kMostSortParagraphs) return 0;
    for (const QTextBlock &b : blocks)
        if (isLocked(b) || b.blockFormat().hasProperty(prop::EndnoteBlock)) return 0;
    const int skip = options.header ? 1 : 0;
    if (int(blocks.size()) - skip < 2) return 0;
    QTextDocument *d = sel.document();
    const int keyCount = int(std::min<qsizetype>(3, options.keys.size()));
    // Each paragraph's keys are read once (decorate), the order is worked out on those, and only the paragraphs that move are taken out as pieces.
    const int n = int(blocks.size()) - skip;
    QVector<QVector<ParsedField>> parsed(n);
    for (int i = 0; i < n; ++i) {
        const QTextBlock &b = blocks.at(i + skip);
        QTextCursor t(d);
        t.setPosition(b.position());
        t.setPosition(b.position() + b.length() - 1, QTextCursor::KeepAnchor);
        const QString plain = t.selectedText().replace(QChar::LineSeparator, QLatin1Char('\n'));
        for (int k = 0; k < keyCount; ++k) parsed[i] << parseField(sortFieldOf(plain, options.keys.at(k), options.separator), options.keys.at(k).type, options.caseSensitive);
    }
    QVector<int> order(n);
    for (int i = 0; i < n; ++i) order[i] = i;
    std::stable_sort(order.begin(), order.end(), [&](int x, int y) {
        for (int k = 0; k < keyCount; ++k) {
            const SortKey &key = options.keys.at(k);
            const int r = compareParsed(parsed.at(x).at(k), parsed.at(y).at(k), key.type);
            if (r != 0) return key.descending ? r > 0 : r < 0;
        }
        return false;
    });
    QVector<int> moving;
    for (int slot = 0; slot < n; ++slot)
        if (order.at(slot) != slot) moving << slot;
    if (moving.isEmpty()) return n;
    // Each place keeps its own paragraph look; the text of the paragraphs that move is put into the places in the new order.
    QHash<int, QTextDocumentFragment> pieces;
    for (const int slot : std::as_const(moving)) {
        const QTextBlock &b = blocks.at(slot + skip);
        QTextCursor t(d);
        t.setPosition(b.position());
        t.setPosition(b.position() + b.length() - 1, QTextCursor::KeepAnchor);
        pieces.insert(slot, t.selection());
    }
    QTextCursor edit(d);
    edit.beginEditBlock();
    for (int m = int(moving.size()) - 1; m >= 0; --m) {
        const int slot = moving.at(m);
        const QTextBlock target = blocks.at(slot + skip);
        QTextCursor t(d);
        t.setPosition(target.position());
        t.setPosition(target.position() + target.length() - 1, QTextCursor::KeepAnchor);
        t.removeSelectedText();
        t.insertFragment(pieces.value(order.at(slot)));
    }
    edit.endEditBlock();
    return n;
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
