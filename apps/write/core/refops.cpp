#include "core/refops.h"

#include "core/editops.h"
#include "core/writeprops.h"
#include "text/textprops.h"
#include "wml/xmltree.h"

#include <QCoreApplication>
#include <QRegularExpression>
#include <QTextBlock>
#include <QTextDocument>
#include <algorithm>

using namespace jp::office::wml;

namespace jp::write {

namespace {

QByteArray escaped(const QString &s) { return jp::office::wml::escapeXml(s, true); }

QTextCharFormat formatAtPos(QTextDocument *story, int position)
{
    QTextCursor c(story);
    c.setPosition(position + 1);
    return c.charFormat();
}

Inline bookmarkAtom(bool start, long id, const QString &name)
{
    Inline a;
    a.kind = Inline::ParaAtom;
    a.tag = start ? QStringLiteral("bookmarkStart") : QStringLiteral("bookmarkEnd");
    a.key = QString::number(id);
    a.raw = start ? "<w:bookmarkStart w:id=\"" + QByteArray::number(qlonglong(id)) + "\" w:name=\"" + escaped(name) + "\"/>" : "<w:bookmarkEnd w:id=\"" + QByteArray::number(qlonglong(id)) + "\"/>";
    return a;
}

void putBookmarkAtom(QTextCursor &c, int position, const Inline &atom)
{
    QTextCharFormat look = formatAtPos(c.document(), std::max(0, position - 1));
    look.clearProperty(prop::Atom);
    look.clearProperty(tp::Field);
    look.clearProperty(tp::LinkResult);
    setStored(look, prop::Atom, packAtom(atom));
    look.setProperty(tp::Field, QStringLiteral("raw:"));
    c.setPosition(position);
    c.insertText(QString(QChar::ObjectReplacementCharacter), look);
}

// A new bookmark over [from, to) (positions before anything is put in), id made from the highest of the text.
void putBookmark(QTextDocument *story, const StoryScan &scan, const QString &name, int from, int to)
{
    long id = 0;
    for (const BookmarkSpan &s : scan.bookmarks) id = std::max(id, s.id.toLong());
    ++id;
    QTextCursor c(story);
    putBookmarkAtom(c, to, bookmarkAtom(false, id, name));
    putBookmarkAtom(c, from, bookmarkAtom(true, id, name));
}

QString textOfBlock(const QTextBlock &b)
{
    QString t = b.text();
    t.remove(QChar::ObjectReplacementCharacter);
    t.replace(QChar::LineSeparator, QLatin1Char(' ')).replace(QLatin1Char('\t'), QLatin1Char(' '));
    return t.trimmed();
}

// Where a paragraph's text begins and ends: past a leading page break and marks, before its end.
void textRange(const QTextBlock &b, int *from, int *to)
{
    const QString t = b.text();
    int a = 0;
    while (a < t.size() && (t[a] == QChar(0x000C) || t[a] == QChar::ObjectReplacementCharacter)) ++a;
    *from = b.position() + std::min<int>(a, int(t.size()));
    *to = b.position() + int(t.size());
}

QString hiddenName(const StoryScan &scan, int &serial)
{
    QString name;
    do name = QStringLiteral("_Ref%1").arg(100000001 + serial++);
    while (scan.bookmarkByName.contains(name.toLower()));
    return name;
}

} // namespace

QVector<BookmarkItem> bookmarkList(WriteDoc &doc, bool hidden, bool byLocation)
{
    const StoryScan scan = scanStory(doc.story());
    QVector<BookmarkItem> out;
    QSet<QString> seen;
    for (const BookmarkSpan &s : scan.bookmarks) {
        if (s.name.isEmpty() || s.end < 0 || (s.hidden && !hidden) || seen.contains(s.name.toLower())) continue;
        seen.insert(s.name.toLower());
        out.push_back({s.name, s.start, s.end, s.hidden});
    }
    if (byLocation) std::stable_sort(out.begin(), out.end(), [](const BookmarkItem &a, const BookmarkItem &b) { return a.start < b.start; });
    else std::stable_sort(out.begin(), out.end(), [](const BookmarkItem &a, const BookmarkItem &b) { return a.name.compare(b.name, Qt::CaseInsensitive) < 0; });
    return out;
}

QString bookmarkNameProblem(const QString &name)
{
    if (name.isEmpty()) return QCoreApplication::translate("jp::write::Bookmarks", "Type a name for the bookmark.");
    if (name.size() > kMostBookmarkNameLength) return QCoreApplication::translate("jp::write::Bookmarks", "A bookmark name has 40 characters at most.");
    if (!name[0].isLetter()) return QCoreApplication::translate("jp::write::Bookmarks", "A bookmark name must begin with a letter.");
    for (const QChar c : name)
        if (!c.isLetterOrNumber() && c != QLatin1Char('_')) return QCoreApplication::translate("jp::write::Bookmarks", "A bookmark name has letters, numbers, and underscores only (no spaces).");
    return QString();
}

bool addBookmark(WriteDoc &doc, const QTextCursor &c, const QString &name, QString *problem)
{
    const QString why = bookmarkNameProblem(name);
    if (!why.isEmpty()) {
        if (problem) *problem = why;
        return false;
    }
    QTextDocument *story = doc.story();
    if (c.document() != story) {
        if (problem) *problem = QCoreApplication::translate("jp::write::Bookmarks", "A bookmark goes in the text, not in a header, a footer, or a note.");
        return false;
    }
    const StoryScan scan = scanStory(story);
    if (scan.bookmarks.size() >= kMostBookmarks) {
        if (problem) *problem = QCoreApplication::translate("jp::write::Bookmarks", "This document has the most bookmarks JeffWrite holds.");
        return false;
    }
    WriteDoc::StepGroup group(doc);
    EditScope scope(doc, story);
    QTextCursor edit(story);
    edit.beginEditBlock();
    int from = c.selectionStart(), to = c.selectionEnd();
    StoryScan current = scan;
    // One of the name that is there is moved: its marks come out (the end first, so the start's place stays).
    if (const auto found = scan.bookmarkByName.constFind(name.toLower()); found != scan.bookmarkByName.constEnd()) {
        const BookmarkSpan old = scan.bookmarks[found.value()];
        QVector<int> places;
        places << old.start;
        if (old.end >= 0) places << old.end;
        std::sort(places.begin(), places.end(), std::greater<int>());
        for (int p : places) {
            QTextCursor t(story);
            t.setPosition(p);
            t.setPosition(p + 1, QTextCursor::KeepAnchor);
            t.removeSelectedText();
            if (from > p) --from;
            if (to > p) --to;
        }
        current = scanStory(story);
    }
    putBookmark(story, current, name, from, to);
    edit.endEditBlock();
    return true;
}

bool deleteBookmark(WriteDoc &doc, const QString &name)
{
    QTextDocument *story = doc.story();
    const StoryScan scan = scanStory(story);
    const auto found = scan.bookmarkByName.constFind(name.toLower());
    if (found == scan.bookmarkByName.constEnd()) return false;
    const BookmarkSpan s = scan.bookmarks[found.value()];
    WriteDoc::StepGroup group(doc);
    EditScope scope(doc, story);
    QTextCursor edit(story);
    edit.beginEditBlock();
    QVector<int> places;
    places << s.start;
    if (s.end >= 0) places << s.end;
    std::sort(places.begin(), places.end(), std::greater<int>());
    for (int p : places) {
        QTextCursor t(story);
        t.setPosition(p);
        t.setPosition(p + 1, QTextCursor::KeepAnchor);
        t.removeSelectedText();
    }
    edit.endEditBlock();
    return true;
}

int bookmarkStart(WriteDoc &doc, const QString &name)
{
    const StoryScan scan = scanStory(doc.story());
    const auto found = scan.bookmarkByName.constFind(name.toLower());
    return found == scan.bookmarkByName.constEnd() ? -1 : scan.bookmarks[found.value()].start;
}

QStringList captionLabels(WriteDoc &doc)
{
    const StoryScan scan = scanStory(doc.story(), false);
    QStringList out;
    for (int i = 0; i < scan.fields.size(); ++i)
        if (scan.fields[i].name == QLatin1String("SEQ")) {
            const QString id = fields::parseCode(scan.instructionOf(i)).arguments.value(0).trimmed();
            if (!id.isEmpty() && !out.contains(id, Qt::CaseInsensitive) && out.size() < 64) out << id;
        }
    return out;
}

QVector<RefTarget> crossReferenceTargets(WriteDoc &doc, RefKind kind, const QString &label)
{
    QTextDocument *story = doc.story();
    const StoryScan scan = scanStory(story);
    QVector<RefTarget> out;
    int serial = 0;
    // The bookmark a paragraph's text already has (a hidden _Ref over all of it), if any.
    auto existing = [&](int from, int to) -> QString {
        for (const BookmarkSpan &s : scan.bookmarks)
            if (s.name.startsWith(QLatin1String("_Ref")) && s.start >= 0 && s.start <= from + 1 && s.end >= to - 1 && s.end <= to + 1) return s.name;
        return QString();
    };
    auto add = [&](const QString &text, int position, int from, int to, const QString &number) {
        RefTarget t;
        t.label = text.left(120);
        t.position = position;
        t.from = from;
        t.to = to;
        t.number = number;
        t.bookmark = existing(from, to);
        if (t.bookmark.isEmpty()) {
            t.bookmark = hiddenName(scan, serial);
            t.newBookmark = true;
        }
        out.push_back(t);
    };
    if (kind == RefKind::Bookmark) {
        for (const BookmarkItem &b : bookmarkList(doc, false, false)) {
            if (out.size() >= kMostCrossReferenceTargets) break;
            RefTarget t;
            t.label = b.name;
            t.position = b.start;
            t.bookmark = b.name;
            out.push_back(t);
        }
        return out;
    }
    if (kind == RefKind::Footnote || kind == RefKind::Endnote) {
        const QVector<int> ids = kind == RefKind::Footnote ? doc.citedFootnotes() : doc.citedEndnotes();
        QVector<std::pair<int, int>> refs;   // (position, ordinal id)
        for (int id : ids) {
            const QVector<int> at = kind == RefKind::Footnote ? doc.footnoteReferences(id) : doc.endnoteReferences(id);
            for (int p : at) refs.push_back({p, id});
        }
        std::sort(refs.begin(), refs.end());
        int number = 0;
        for (const auto &r : std::as_const(refs)) {
            if (out.size() >= kMostCrossReferenceTargets) break;
            ++number;
            const QTextBlock b = story->findBlock(r.first);
            add(QStringLiteral("%1  %2").arg(number).arg(textOfBlock(b).left(60)), r.first, r.first, r.first + 1, QString::number(number));
        }
        return out;
    }
    for (QTextBlock b = story->begin(); b.isValid() && out.size() < kMostCrossReferenceTargets; b = b.next()) {
        const QTextBlockFormat bf = b.blockFormat();
        if (bf.hasProperty(prop::EndnoteBlock) || bf.hasProperty(prop::RawBlock)) continue;
        int from = 0, to = 0;
        textRange(b, &from, &to);
        const QString text = textOfBlock(b);
        if (text.isEmpty()) continue;
        if (kind == RefKind::Heading) {
            if (bf.hasProperty(prop::Outline) && bf.intProperty(prop::Outline) < 9) add(paragraphLabelOf(b).isEmpty() ? text : paragraphLabelOf(b) + QLatin1Char(' ') + text, b.position(), from, to, paragraphNumberOf(b));
        } else if (kind == RefKind::NumberedItem) {
            if (!paragraphNumberOf(b).isEmpty()) add(paragraphLabelOf(b) + QLatin1Char(' ') + text, b.position(), from, to, paragraphNumberOf(b));
        }
    }
    if (kind == RefKind::Caption) {
        for (int i = 0; i < scan.fields.size() && out.size() < kMostCrossReferenceTargets; ++i) {
            if (scan.fields[i].name != QLatin1String("SEQ")) continue;
            if (fields::parseCode(scan.instructionOf(i)).arguments.value(0).trimmed().compare(label, Qt::CaseInsensitive) != 0) continue;
            const QTextBlock b = story->findBlock(scan.fields[i].begin);
            int from = 0, to = 0;
            textRange(b, &from, &to);
            add(textOfBlock(b), b.position(), from, to, QString());
            // (What the label and number cover: from the start of the text to the end of the field.)
            out.last().number = QString::number(scan.fields[i].end);
            out.last().to = to;
        }
    }
    return out;
}

QString crossReferenceCode(const RefTarget &t, RefKind kind, RefWhat what, bool hyperlink)
{
    const QString h = hyperlink ? QStringLiteral(" \\h") : QString();
    const QString name = t.bookmark;
    switch (what) {
    case RefWhat::Text:
    case RefWhat::CaptionText:
    case RefWhat::LabelAndNumber: return QStringLiteral("REF %1%2").arg(name, h);
    case RefWhat::Number: return QStringLiteral("REF %1 \\r%2").arg(name, h);
    case RefWhat::NumberNoContext: return QStringLiteral("REF %1 \\n%2").arg(name, h);
    case RefWhat::NumberFullContext: return QStringLiteral("REF %1 \\w%2").arg(name, h);
    case RefWhat::Page: return QStringLiteral("PAGEREF %1%2").arg(name, h);
    case RefWhat::AboveBelow: return QStringLiteral("REF %1 \\p%2").arg(name, h);
    case RefWhat::FootnoteNumber: return kind == RefKind::Footnote || kind == RefKind::Endnote ? QStringLiteral("NOTEREF %1%2").arg(name, h) : QString();
    case RefWhat::FootnoteNumberFormatted: return kind == RefKind::Footnote || kind == RefKind::Endnote ? QStringLiteral("NOTEREF %1 \\f%2").arg(name, h) : QString();
    }
    return QString();
}

bool insertCrossReference(WriteDoc &doc, QTextCursor &c, const RefTarget &target, RefKind kind, RefWhat what, bool hyperlink, QString *problem)
{
    QTextDocument *story = doc.story();
    if (c.document() != story) {
        if (problem) *problem = QCoreApplication::translate("jp::write::Bookmarks", "A cross-reference goes in the text, not in a header, a footer, or a note.");
        return false;
    }
    const QString code = crossReferenceCode(target, kind, what, hyperlink);
    if (code.isEmpty()) {
        if (problem) *problem = QCoreApplication::translate("jp::write::Bookmarks", "That cannot be referred to that way.");
        return false;
    }
    WriteDoc::StepGroup group(doc);
    EditScope scope(doc, story);
    QTextCursor edit = c;
    edit.beginEditBlock();
    int at = c.selectionStart();
    const int selected = c.selectionEnd() - c.selectionStart();
    if (target.newBookmark) {
        const StoryScan scan = scanStory(story);
        int from = target.from, to = target.to;
        if (kind == RefKind::Caption && what == RefWhat::LabelAndNumber) to = target.number.toInt() + 1;
        else if (kind == RefKind::Caption && what == RefWhat::CaptionText) {
            // (The text after the label and number.)
            from = target.number.toInt() + 1;
        }
        putBookmark(story, scan, target.bookmark, from, std::max(from, to));
        // The bookmark's marks are before the caret when the target is: the caret moves with them.
        if (at > target.to) at += 2;
        else if (at > from) at += 1;
    }
    QTextCursor put(story);
    put.setPosition(at);
    if (selected > 0) put.setPosition(at + selected, QTextCursor::KeepAnchor);
    const bool ok = insertField(doc, put, code);
    edit.endEditBlock();
    if (ok) c = put;
    else if (problem) *problem = QCoreApplication::translate("jp::write::Bookmarks", "The reference could not be put there.");
    return ok;
}

int linkTargetAt(WriteDoc &doc, int position)
{
    QTextDocument *story = doc.story();
    if (position < 0 || position >= story->characterCount()) return -1;
    const StoryScan scan = scanStory(story);
    // A hyperlink to a place in the text: the innermost anchor whose tags have the position between them.
    {
        QTextBlock b = story->findBlock(position);
        QVector<std::pair<int, QString>> open;
        for (auto it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment frag = it.fragment();
            const QTextCharFormat cf = frag.charFormat();
            if (!cf.hasProperty(prop::Atom)) continue;
            const QByteArray bytes = storedBytes(cf, prop::Atom);
            if (!bytes.contains("hyperlink")) continue;
            const Inline a = unpackAtom(bytes);
            if (a.tag != QLatin1String("hyperlink")) continue;
            for (int k = 0; k < frag.length(); ++k) {
                const int at = frag.position() + k;
                if (a.role == Inline::Open) {
                    static const QRegularExpression anchor(QStringLiteral("anchor=\"([^\"]*)\""));
                    open.push_back({at, anchor.match(QString::fromUtf8(a.raw)).captured(1)});
                } else if (a.role == Inline::Close && !open.isEmpty()) {
                    const auto o = open.takeLast();
                    if (position >= o.first && position <= at && !o.second.isEmpty()) {
                        const auto found = scan.bookmarkByName.constFind(o.second.toLower());
                        if (found != scan.bookmarkByName.constEnd()) return scan.bookmarks[found.value()].start;
                    }
                }
            }
        }
    }
    const int at = fieldContaining(scan, position);
    if (at < 0) return -1;
    int f = at;
    while (f >= 0) {
        const fields::Code code = fields::parseCode(scan.instructionOf(f));
        if ((code.name == QLatin1String("REF") || code.name == QLatin1String("PAGEREF") || code.name == QLatin1String("NOTEREF")) && code.has(QLatin1Char('h'))) {
            const auto found = scan.bookmarkByName.constFind(code.arguments.value(0).trimmed().toLower());
            return found == scan.bookmarkByName.constEnd() ? -1 : scan.bookmarks[found.value()].start;
        }
        f = scan.fields[f].parent;
    }
    return -1;
}

int nextNoteReference(WriteDoc &doc, int from, bool forward, bool endnote)
{
    QVector<int> all;
    const QVector<int> ids = endnote ? doc.citedEndnotes() : doc.citedFootnotes();
    for (int id : ids) all += endnote ? doc.endnoteReferences(id) : doc.footnoteReferences(id);
    std::sort(all.begin(), all.end());
    if (forward) {
        for (int p : all)
            if (p > from) return p;
    } else {
        for (int i = int(all.size()) - 1; i >= 0; --i)
            if (all[i] + 1 < from) return all[i];
    }
    return -1;
}

} // namespace jp::write
