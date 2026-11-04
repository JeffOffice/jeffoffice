#include "core/linkops.h"

#include "core/editops.h"
#include "core/fieldops.h"
#include "core/refops.h"
#include "core/writeprops.h"
#include "text/textprops.h"
#include "wml/xmltree.h"

#include <QCoreApplication>
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

bool checkTarget(WriteDoc &doc, const QString &address, const QString &anchor, QString *problem)
{
    auto fail = [&](const QString &why) {
        if (problem) *problem = why;
        return false;
    };
    if (!anchor.isEmpty()) {
        const StoryScan scan = scanStory(doc.story());
        if (!scan.bookmarkByName.contains(anchor.toLower())) return fail(QCoreApplication::translate("jp::write::Hyperlinks", "There is no bookmark with that name in this document."));
        return true;
    }
    if (address.isEmpty()) return fail(QCoreApplication::translate("jp::write::Hyperlinks", "Type an address for the link to go to."));
    if (address.size() > kMostAddress || !allowedAddress(address)) return fail(QCoreApplication::translate("jp::write::Hyperlinks", "That is not a web or email address."));
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

bool mayFollow(const QString &address) { return address.size() <= kMostAddress && allowedAddress(address); }

bool linkNeedsAsking(const QString &address, const QString &shown)
{
    const QString scheme = QUrl(address, QUrl::TolerantMode).scheme().toLower();
    if (scheme == QLatin1String("mailto") || scheme == QLatin1String("tel")) return true;
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

bool insertHyperlink(WriteDoc &doc, QTextCursor &c, const QString &address, const QString &anchor, const QString &screenTip, const QString &text, QString *problem)
{
    QTextDocument *story = doc.story();
    if (c.document() != story) {
        if (problem) *problem = QCoreApplication::translate("jp::write::Hyperlinks", "A link goes in the text, not in a header, a footer, or a note.");
        return false;
    }
    if (!checkTarget(doc, address, anchor, problem)) return false;
    if (isLocked(c.block())) return false;
    const WriteDoc::StepGroup group(doc);
    EditScope scope(doc, story);
    // The text the link covers: the selection, or the words typed for it.
    if (!c.hasSelection()) {
        const QString shown = text.isEmpty() ? (anchor.isEmpty() ? address : anchor) : text;
        insertText(doc, c, shown);
        c.setPosition(c.position() - int(shown.size()));
        c.setPosition(c.position() + int(shown.size()), QTextCursor::KeepAnchor);
    }
    const int from = c.selectionStart(), to = c.selectionEnd();
    if (from == to) return false;
    // A link is inside one paragraph (its two tags are in it): a selection over several becomes a link for each paragraph's part, all to the same place.
    QVector<std::pair<int, int>> ranges;
    for (QTextBlock b = story->findBlock(from); b.isValid() && b.position() < to; b = b.next()) {
        if (isLocked(b)) continue;
        const int a = std::max(from, b.position()), z = std::min(to, b.position() + b.length() - 1);
        if (z > a) ranges.push_back({a, z});
    }
    if (ranges.isEmpty()) return false;
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

bool changeHyperlink(WriteDoc &doc, int position, const QString &address, const QString &anchor, const QString &screenTip, const QString &text, QString *problem)
{
    const Hyperlink link = hyperlinkAt(doc, position);
    if (!link.valid()) return false;
    if (!checkTarget(doc, address, anchor, problem)) return false;
    QTextDocument *story = doc.story();
    const WriteDoc::StepGroup group(doc);
    EditScope scope(doc, story);
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
