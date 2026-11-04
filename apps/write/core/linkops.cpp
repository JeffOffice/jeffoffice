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

QString attribute(const QByteArray &raw, const char *name)
{
    const QRegularExpression re(QStringLiteral("\\s(?:w:|r:)?%1=\"([^\"]*)\"").arg(QLatin1String(name)));
    const QRegularExpressionMatch m = re.match(QString::fromUtf8(raw.left(4000)));
    return m.hasMatch() ? m.captured(1).replace(QStringLiteral("&amp;"), QStringLiteral("&")).replace(QStringLiteral("&quot;"), QStringLiteral("\"")).replace(QStringLiteral("&lt;"), QStringLiteral("<")) : QString();
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
    if (scheme.match(t).hasMatch()) return t;
    if (mail.match(t).hasMatch()) return QStringLiteral("mailto:") + t;
    return QStringLiteral("https://") + t;
}

bool mayFollow(const QString &address) { return address.size() <= kMostAddress && allowedAddress(address); }

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
                    found.anchor = attribute(o.second, "anchor");
                    found.screenTip = attribute(o.second, "tooltip");
                    const QString id = attribute(o.second, "id");
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
    QTextCursor edit(story);
    edit.beginEditBlock();
    {
        QTextCursor look(story);
        look.setPosition(from);
        look.setPosition(to, QTextCursor::KeepAnchor);
        applyCharacterStyle(doc, look, QStringLiteral("Hyperlink"));
    }
    const QByteArray open = openTag(doc, address, anchor, screenTip);
    putAtom(edit, to, linkAtom(false, "</w:hyperlink>"));
    putAtom(edit, from, linkAtom(true, open));
    edit.endEditBlock();
    c.setPosition(to + 2);
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
