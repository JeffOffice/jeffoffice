#include "core/linkops.h"

#include "core/editops.h"
#include "core/fieldcode.h"
#include "core/fieldops.h"
#include "core/refops.h"
#include "office/hyperlink.h"
#include "core/writeprops.h"
#include "text/textprops.h"
#include "wml/xmltree.h"

#include <QCoreApplication>
#include <QDir>
#include <QRegularExpression>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QUrl>
#include <algorithm>

using namespace jp::office::wml;

namespace jp::write {

namespace {


constexpr int kMostAddress = 2000;
const QString kHyperlinkType = QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/hyperlink");

Inline linkAtom(bool open, const QByteArray &raw)
{
    Inline a;
    a.kind = Inline::ParaAtom;
    a.role = open ? Inline::Open : Inline::Close;
    a.tag = QStringLiteral("hyperlink");
    a.raw = raw;
    return a;
}

void putAtom(QTextCursor &c, int position, const Inline &atom)
{
    QTextCursor at(c.document());
    at.setPosition(std::max(0, std::min(position, c.document()->characterCount() - 1)));
    QTextCharFormat look = at.charFormat();
    look.clearProperty(prop::Atom);
    look.clearProperty(tp::Field);
    look.clearProperty(tp::LinkResult);
    look.clearProperty(QTextFormat::FontFamilies);
    setStored(look, prop::Atom, packAtom(atom));
    look.setProperty(tp::Field, QStringLiteral("raw:"));
    c.setPosition(position);
    c.insertText(QString(QChar::ObjectReplacementCharacter), look);
}

QByteArray openTag(WriteDoc &doc, const QString &address, const QString &anchor, const QString &screenTip)
{
    QByteArray tag = "<w:hyperlink";
    if (!anchor.isEmpty()) tag += " w:anchor=\"" + escapeXml(anchor, true) + "\"";
    else tag += " r:id=\"" + doc.addExternalLink(kHyperlinkType, address).toUtf8() + "\"";
    if (!screenTip.isEmpty()) tag += " w:tooltip=\"" + escapeXml(screenTip.left(260), true) + "\"";
    return tag + " w:history=\"1\">";
}

// The value of an attribute of the tag's own (a start tag's attributes, each in single or double quotes, by its name with or without the namespace prefix `prefix`): the tag is read
// attribute by attribute, so that text inside another attribute's value is never taken for one.
QString attribute(const QByteArray &raw, const char *name, const char *prefix)
{
    const QString tag = QString::fromUtf8(raw.left(4000));
    static const QRegularExpression head(QStringLiteral("^\\s*<[^\\s>/]+"));
    static const QRegularExpression one(QStringLiteral("^\\s+([^\\s=/>]+)\\s*=\\s*(?:\"([^\"]*)\"|'([^']*)')"));
    qsizetype at = head.match(tag).capturedLength();
    if (at <= 0) return QString();
    const QString wanted = QLatin1String(prefix) + QLatin1Char(':') + QLatin1String(name);
    while (at < tag.size()) {
        const QRegularExpressionMatch m = one.match(tag.mid(at));
        if (!m.hasMatch()) break;
        at += m.capturedLength();
        if (m.captured(1) != wanted) continue;
        QString v = m.captured(2).isNull() ? m.captured(3) : m.captured(2);
        return v.replace(QStringLiteral("&quot;"), QStringLiteral("\"")).replace(QStringLiteral("&apos;"), QStringLiteral("'")).replace(QStringLiteral("&lt;"), QStringLiteral("<"))
            .replace(QStringLiteral("&gt;"), QStringLiteral(">")).replace(QStringLiteral("&amp;"), QStringLiteral("&"));
    }
    return QString();
}

// Whether the address names a place a link may go to (a web page or a mail address; no program is started by a link).
bool allowedAddress(const QString &address)
{
    const QUrl url(address, QUrl::TolerantMode);
    const QString scheme = url.scheme().toLower();
    return url.isValid() && (scheme == QLatin1String("http") || scheme == QLatin1String("https") || scheme == QLatin1String("mailto") || scheme == QLatin1String("ftp") || scheme == QLatin1String("tel"));
}

// (U+0001 is no character of a name in a file, so no bookmark has a name that begins like this.)
const QString kHeadingPrefix = QString(QChar(1)) + QStringLiteral("heading:");

int headingPositionOf(const QString &anchor)
{
    if (!anchor.startsWith(kHeadingPrefix)) return -1;
    bool ok = false;
    const int n = anchor.mid(kHeadingPrefix.size()).toInt(&ok);
    return ok && n >= 0 ? n : -1;
}

bool isHeadingBlock(const QTextBlock &b)
{
    return b.isValid() && b.blockFormat().hasProperty(prop::Outline) && b.blockFormat().intProperty(prop::Outline) < 9;
}

bool checkTarget(WriteDoc &doc, const QString &address, const QString &anchor, QString *problem)
{
    auto fail = [&](const QString &why) {
        if (problem) *problem = why;
        return false;
    };
    if (anchor.startsWith(kHeadingPrefix)) {
        const int at = headingPositionOf(anchor);
        if (at < 0 || at >= doc.story()->characterCount() || !isHeadingBlock(doc.story()->findBlock(at))) return fail(QCoreApplication::translate("jp::write::Hyperlinks", "That heading is no longer in this document."));
        return true;
    }
    if (!anchor.isEmpty()) {
        const StoryScan scan = scanStory(doc.story());
        if (!scan.bookmarkByName.contains(anchor.toLower())) return fail(QCoreApplication::translate("jp::write::Hyperlinks", "There is no bookmark with that name in this document."));
        return true;
    }
    if (address.isEmpty()) return fail(QCoreApplication::translate("jp::write::Hyperlinks", "Type an address for the link to go to."));
    if (address.size() > kMostAddress || !(allowedAddress(address) || isFileLink(address))) return fail(QCoreApplication::translate("jp::write::Hyperlinks", "That is not a web address, an email address, or a file on this computer."));
    return true;
}

} // namespace

QString normalizedAddress(const QString &typed)
{
    const QString t = typed.trimmed();
    if (t.isEmpty() || t.size() > kMostAddress || t.contains(QLatin1Char(' '))) return QString();
    static const QRegularExpression scheme(QStringLiteral("^[A-Za-z][A-Za-z0-9+.-]*:"));
    static const QRegularExpression mail(QStringLiteral("^[^@\\s/:]+@[^@\\s/:]+\\.[^@\\s/:]+$"));
    // (A host and a port, "example.com:8080/x" or "localhost:3000", looks like a scheme and is not one.)
    static const QRegularExpression known(QStringLiteral("^(?:mailto|tel|sms|http|https|ftp|file):"), QRegularExpression::CaseInsensitiveOption);
    if (known.match(t).hasMatch()) return t;
    static const QRegularExpression hostPort(QStringLiteral("^[^:/\\s@]+:[0-9]{1,5}(?:[/?#].*)?$"));
    if (hostPort.match(t).hasMatch()) return QStringLiteral("https://") + t;
    if (scheme.match(t).hasMatch()) return t;
    if (mail.match(t).hasMatch()) return QStringLiteral("mailto:") + t;
    return QStringLiteral("https://") + t;
}

bool isFileLink(const QString &address) { return address.size() <= kMostAddress && office::isOpenableFileLink(address); }

QString fileLinkOfPath(const QString &typed)
{
    const QString t = typed.trimmed();
    if (t.isEmpty() || t.size() > kMostAddress) return QString();
    const QString link = t.startsWith(QLatin1String("file:"), Qt::CaseInsensitive) ? t : office::isLocalAbsolutePath(t) ? QUrl::fromLocalFile(t).toString(QUrl::FullyEncoded) : QString();
    return isFileLink(link) ? link : QString();
}

QString pathOfFileLink(const QString &address)
{
    QString path;
    return address.size() <= kMostAddress && office::isOpenableFileLink(address, &path) ? path : QString();
}

bool mayFollow(const QString &address) { return address.size() <= kMostAddress && allowedAddress(address); }

bool linkNeedsAsking(const QString &address, const QString &shown)
{
    const QString scheme = QUrl(address, QUrl::TolerantMode).scheme().toLower();
    if (scheme == QLatin1String("mailto") || scheme == QLatin1String("tel") || scheme == QLatin1String("file")) return true;
    // The words look like an address of their own, and it is not where the link goes.
    const QString words = shown.trimmed();
    static const QRegularExpression looksLikeAddress(QStringLiteral("^(?:[A-Za-z][A-Za-z0-9+.-]*://|www\\.)\\S+$|^[A-Za-z0-9-]+(?:\\.[A-Za-z0-9-]+)+(?::[0-9]+)?(?:[/?#]\\S*)?$"));
    if (!looksLikeAddress.match(words).hasMatch()) return false;
    auto plain = [](QString a) {
        a = a.trimmed().toLower();
        while (a.endsWith(QLatin1Char('/'))) a.chop(1);
        return a;
    };
    return plain(normalizedAddress(words)) != plain(address);
}

Hyperlink hyperlinkAt(WriteDoc &doc, int position)
{
    QTextDocument *story = doc.story();
    Hyperlink found;
    if (position < 0 || position >= story->characterCount()) return found;
    QTextBlock b = story->findBlock(position);
    QVector<std::pair<int, QByteArray>> open;
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
                open.push_back({at, a.raw});
            } else if (a.role == Inline::Close && !open.isEmpty()) {
                const auto o = open.takeLast();
                if (position >= o.first && position <= at && (!found.valid() || o.first > found.start)) {
                    found.start = o.first;
                    found.end = at;
                    found.anchor = attribute(o.second, "anchor", "w");
                    found.screenTip = attribute(o.second, "tooltip", "w");
                    const QString id = attribute(o.second, "id", "r");
                    if (!id.isEmpty()) found.address = doc.mainRelationship(id).target;
                    QTextCursor t(story);
                    t.setPosition(o.first + 1);
                    t.setPosition(at, QTextCursor::KeepAnchor);
                    found.text = t.selectedText().remove(QChar::ObjectReplacementCharacter);
                }
            }
        }
    }
    return found;
}

QVector<LinkSpan> linkSpans(WriteDoc &doc)
{
    QVector<LinkSpan> out;
    QTextDocument *story = doc.story();
    const StoryScan scan = scanStory(story);
    auto place = [&](const QString &name) {
        const auto found = scan.bookmarkByName.constFind(name.trimmed().toLower());
        return found == scan.bookmarkByName.constEnd() ? -1 : scan.bookmarks[found.value()].start;
    };
    for (QTextBlock b = story->begin(); b.isValid() && out.size() < kMostLinkSpans; b = b.next()) {
        QVector<std::pair<int, QByteArray>> open;
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
                    open.push_back({at, a.raw});
                } else if (a.role == Inline::Close && !open.isEmpty()) {
                    const auto o = open.takeLast();
                    if (at <= o.first + 1) continue;
                    LinkSpan s;
                    s.from = o.first + 1;
                    s.to = at;
                    const QString anchor = attribute(o.second, "anchor", "w");
                    const QString id = attribute(o.second, "id", "r");
                    if (!anchor.isEmpty()) s.target = place(anchor);
                    else if (!id.isEmpty()) {
                        const QString address = doc.linkAddress(story, id, doc.endnoteIdOf(b));
                        if (mayFollow(address)) s.uri = address;
                    }
                    if (s.target >= 0 || !s.uri.isEmpty()) out.push_back(s);
                }
            }
        }
    }
    // The cross-references with the \h switch, and the HYPERLINK fields, that are not inside a hyperlink already.
    const int hyperlinks = int(out.size());
    for (int i = 0; i < scan.fields.size() && out.size() < kMostLinkSpans; ++i) {
        const FieldPlace &f = scan.fields[i];
        const bool cross = f.name == QLatin1String("REF") || f.name == QLatin1String("PAGEREF") || f.name == QLatin1String("NOTEREF");
        if (!cross && f.name != QLatin1String("HYPERLINK")) continue;
        if (f.separate < 0 || f.end <= f.separate + 1) continue;
        const fields::Code code = fields::parseCode(scan.instructionOf(i));
        LinkSpan s;
        s.from = f.separate + 1;
        s.to = f.end;
        if (cross) {
            if (!code.has(QLatin1Char('h'))) continue;
            s.target = place(code.arguments.value(0));
        } else if (code.has(QLatin1Char('l'))) {
            s.target = place(code.argumentOf(QLatin1Char('l')));   // (a place in this document: its bookmark)
        } else if (mayFollow(code.arguments.value(0))) {
            s.uri = code.arguments.value(0);
        }
        if (s.target < 0 && s.uri.isEmpty()) continue;
        bool inside = false;
        for (int k = 0; k < hyperlinks && !inside; ++k) inside = out[k].from <= s.from && s.to <= out[k].to;
        if (!inside) out.push_back(s);
    }
    return out;
}

QVector<LinkSpan> linkSpansIn(WriteDoc &doc, QTextDocument *story, const QTextDocument *source, const StoryScan &scan)
{
    QVector<LinkSpan> out;
    for (QTextBlock b = story->begin(); b.isValid() && out.size() < kMostLinkSpans; b = b.next()) {
        QVector<std::pair<int, QByteArray>> open;
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
                    open.push_back({at, a.raw});
                } else if (a.role == Inline::Close && !open.isEmpty()) {
                    const auto o = open.takeLast();
                    if (at <= o.first + 1) continue;
                    LinkSpan s;
                    s.from = o.first + 1;
                    s.to = at;
                    const QString anchor = attribute(o.second, "anchor", "w");
                    const QString id = attribute(o.second, "id", "r");
                    if (!anchor.isEmpty()) {
                        const auto found = scan.bookmarkByName.constFind(anchor.trimmed().toLower());
                        if (found != scan.bookmarkByName.constEnd()) s.target = scan.bookmarks[found.value()].start;
                    } else if (!id.isEmpty()) {
                        const QString address = doc.linkAddress(source, id);
                        if (mayFollow(address)) s.uri = address;
                    }
                    if (s.target >= 0 || !s.uri.isEmpty()) out.push_back(s);
                }
            }
        }
    }
    return out;
}

int headingPositionOfPlace(const QString &place) { return headingPositionOf(place); }

QString headingPlace(int paragraphPosition) { return kHeadingPrefix + QString::number(paragraphPosition); }

QString headingBookmark(WriteDoc &doc, int paragraphPosition, bool make, QString *problem)
{
    QTextDocument *story = doc.story();
    const QTextBlock b = paragraphPosition >= 0 && paragraphPosition < story->characterCount() ? story->findBlock(paragraphPosition) : QTextBlock();
    if (!isHeadingBlock(b)) {
        if (problem) *problem = QCoreApplication::translate("jp::write::Hyperlinks", "That heading is no longer in this document.");
        return QString();
    }
    const StoryScan scan = scanStory(story);
    const int first = b.position(), last = b.position() + b.length() - 1;
    for (const BookmarkSpan &s : scan.bookmarks)
        if (s.hidden && (s.name.startsWith(QLatin1String("_Toc")) || s.name.startsWith(QLatin1String("_Ref"))) && s.start >= first && s.start <= last && s.end >= s.start && s.end <= last + 1) return s.name;
    if (!make) return QString();
    // The text of the paragraph, past a page break and the marks that begin it.
    const QString t = b.text();
    int from = 0;
    while (from < t.size() && (t[from] == QChar(0x000C) || t[from] == QChar::ObjectReplacementCharacter)) ++from;
    QString name;
    int serial = 0;
    do name = QStringLiteral("_Toc%1").arg(100000001 + serial++);
    while (scan.bookmarkByName.contains(name.toLower()));
    QTextCursor over(story);
    over.setPosition(b.position() + std::min<int>(from, int(t.size())));
    over.setPosition(b.position() + int(t.size()), QTextCursor::KeepAnchor);
    if (!addBookmark(doc, over, name, problem, true)) return QString();
    return name;
}

QString placeOfAnchor(WriteDoc &doc, const QString &anchor)
{
    if (anchor.isEmpty() || !anchor.startsWith(QLatin1Char('_'))) return anchor;
    const StoryScan scan = scanStory(doc.story());
    const auto found = scan.bookmarkByName.constFind(anchor.toLower());
    if (found == scan.bookmarkByName.constEnd()) return anchor;
    const QTextBlock b = doc.story()->findBlock(scan.bookmarks[found.value()].start);
    return isHeadingBlock(b) ? headingPlace(b.position()) : anchor;
}

namespace {
// The bookmark's name a link is to hold: a name as it is; a heading's place made into its hidden bookmark. Empty (with the reason) where it can't be.
QString resolvedAnchor(WriteDoc &doc, const QString &anchor, QString *problem)
{
    if (!anchor.startsWith(kHeadingPrefix)) return anchor;
    return headingBookmark(doc, headingPositionOf(anchor), true, problem);
}
}

bool insertHyperlink(WriteDoc &doc, QTextCursor &c, const QString &address, const QString &anchorAsked, const QString &screenTip, const QString &text, QString *problem)
{
    QTextDocument *story = doc.story();
    if (c.document() != story) {
        if (problem) *problem = QCoreApplication::translate("jp::write::Hyperlinks", "A link goes in the text, not in a header, a footer, or a note.");
        return false;
    }
    if (!checkTarget(doc, address, anchorAsked, problem)) return false;
    if (isLocked(c.block())) return false;
    const WriteDoc::StepGroup group(doc);
    EditScope scope(doc, story);
    const bool hadBookmark = anchorAsked.startsWith(kHeadingPrefix) && !headingBookmark(doc, headingPositionOf(anchorAsked), false).isEmpty();
    const QString anchor = resolvedAnchor(doc, anchorAsked, problem);
    if (!anchorAsked.isEmpty() && anchor.isEmpty()) return false;
    // (A bookmark made for this link goes again if the link can't be made.)
    auto giveUp = [&] {
        if (anchorAsked.startsWith(kHeadingPrefix) && !hadBookmark) deleteBookmark(doc, anchor);
        return false;
    };
    // The text the link covers: the selection, or the words typed for it.
    if (!c.hasSelection()) {
        const QString shown = text.isEmpty() ? (anchorAsked.isEmpty() ? address : anchorAsked.startsWith(kHeadingPrefix) ? doc.story()->findBlock(headingPositionOf(anchorAsked)).text().remove(QChar::ObjectReplacementCharacter).trimmed().left(120) : anchor) : text;
        insertText(doc, c, shown);
        c.setPosition(c.position() - int(shown.size()));
        c.setPosition(c.position() + int(shown.size()), QTextCursor::KeepAnchor);
    }
    const int from = c.selectionStart(), to = c.selectionEnd();
    if (from == to) return giveUp();
    // A link is inside one paragraph (its two tags are in it): a selection over several becomes a link for each paragraph's part, all to the same place.
    QVector<std::pair<int, int>> ranges;
    for (QTextBlock b = story->findBlock(from); b.isValid() && b.position() < to; b = b.next()) {
        if (isLocked(b)) continue;
        const int a = std::max(from, b.position()), z = std::min(to, b.position() + b.length() - 1);
        if (z > a) ranges.push_back({a, z});
    }
    if (ranges.isEmpty()) return giveUp();
    QTextCursor edit(story);
    edit.beginEditBlock();
    {
        QTextCursor look(story);
        look.setPosition(from);
        look.setPosition(to, QTextCursor::KeepAnchor);
        applyCharacterStyle(doc, look, QStringLiteral("Hyperlink"));
    }
    const QByteArray open = openTag(doc, address, anchor, screenTip);
    for (qsizetype i = ranges.size() - 1; i >= 0; --i) {   // (the last first, so that the places of the earlier stay)
        putAtom(edit, ranges[i].second, linkAtom(false, "</w:hyperlink>"));
        putAtom(edit, ranges[i].first, linkAtom(true, open));
    }
    edit.endEditBlock();
    c.setPosition(ranges.last().second + 2 * int(ranges.size()));
    return true;
}

bool removeHyperlink(WriteDoc &doc, int position)
{
    const Hyperlink link = hyperlinkAt(doc, position);
    if (!link.valid()) return false;
    QTextDocument *story = doc.story();
    const WriteDoc::StepGroup group(doc);
    EditScope scope(doc, story);
    QTextCursor edit(story);
    edit.beginEditBlock();
    for (const int at : {link.end, link.start}) {
        QTextCursor t(story);
        t.setPosition(at);
        t.setPosition(at + 1, QTextCursor::KeepAnchor);
        t.removeSelectedText();
    }
    // (The words go back to the look of plain text.)
    QTextCursor words(story);
    words.setPosition(link.start);
    words.setPosition(link.end - 1, QTextCursor::KeepAnchor);
    clearCharacterStyle(doc, words, QStringLiteral("Hyperlink"));
    edit.endEditBlock();
    return true;
}

bool changeHyperlink(WriteDoc &doc, int positionAsked, const QString &address, const QString &anchorAsked, const QString &screenTip, const QString &text, QString *problem)
{
    Hyperlink link = hyperlinkAt(doc, positionAsked);
    if (!link.valid()) return false;
    if (!checkTarget(doc, address, anchorAsked, problem)) return false;
    QTextDocument *story = doc.story();
    const WriteDoc::StepGroup group(doc);
    EditScope scope(doc, story);
    QTextCursor inside(story);   // (stays in the link when a new bookmark moves it)
    inside.setPosition(link.start + 1);
    const bool hadBookmark = anchorAsked.startsWith(kHeadingPrefix) && !headingBookmark(doc, headingPositionOf(anchorAsked), false).isEmpty();
    const QString anchor = resolvedAnchor(doc, anchorAsked, problem);
    if (!anchorAsked.isEmpty() && anchor.isEmpty()) return false;
    link = hyperlinkAt(doc, inside.position());
    if (!link.valid()) {   // (a bookmark made for this link goes again)
        if (anchorAsked.startsWith(kHeadingPrefix) && !hadBookmark) deleteBookmark(doc, anchor);
        return false;
    }
    QTextCursor edit(story);
    edit.beginEditBlock();
    // The opening tag is made again; the words change only where new ones are given.
    QTextCursor t(story);
    t.setPosition(link.start);
    t.setPosition(link.start + 1, QTextCursor::KeepAnchor);
    t.removeSelectedText();
    putAtom(edit, link.start, linkAtom(true, openTag(doc, address, anchor, screenTip)));
    if (!text.isEmpty() && text != link.text) {
        QTextCursor words(story);
        words.setPosition(link.start + 1);
        words.setPosition(link.end, QTextCursor::KeepAnchor);
        words.insertText(text, words.charFormat());
    }
    edit.endEditBlock();
    return true;
}

} // namespace jp::write
