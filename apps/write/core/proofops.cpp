#include "core/proofops.h"

#include "core/editops.h"
#include "core/wordformat.h"
#include "core/writeprops.h"
#include "text/textprops.h"

#include <QRegularExpression>
#include <QUrl>
#include <QUrlQuery>
#include <QTextBlock>

namespace jp::write {

namespace {

void addTo(WordStats &s, const QTextDocument *story, int from, int to, bool withEndnotes)
{
    for (QTextBlock b = story->findBlock(from); b.isValid() && b.position() < to; b = b.next()) {
        if (!withEndnotes && b.blockFormat().hasProperty(prop::EndnoteBlock)) continue;
        bool any = false, in = false;
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment f = it.fragment();
            const int fs = std::max(f.position(), from), fe = std::min(f.position() + f.length(), to);
            if (fs >= fe) continue;
            const QTextCharFormat cf = f.charFormat();
            if (cf.boolProperty(tp::HiddenText) || cf.hasProperty(prop::TrackedDeleted)) {
                in = false;
                continue;
            }
            if (isAtom(cf)) {
                in = false;
                continue;
            }
            for (const QChar ch : f.text().mid(fs - f.position(), fe - fs)) {
                if (ch == QChar::ObjectReplacementCharacter) continue;
                const bool space = ch.isSpace();
                ++s.charactersWithSpaces;
                if (!space) {
                    ++s.charactersNoSpaces;
                    if (!in) ++s.words;
                    any = true;
                }
                in = !space;
            }
        }
        if (any) ++s.paragraphs;
    }
}

} // namespace

WordStats countWordsIn(const QTextDocument *story, int from, int to, bool withEndnotes)
{
    WordStats s;
    addTo(s, story, from, to < 0 ? story->characterCount() : to, withEndnotes);
    return s;
}

WordStats countWords(WriteDoc &doc, const QTextCursor &selection, bool notes)
{
    WordStats s;
    const QTextDocument *story = selection.document() ? selection.document() : doc.story();
    if (selection.hasSelection()) {
        addTo(s, story, selection.selectionStart(), selection.selectionEnd(), notes);
        return s;
    }
    addTo(s, doc.story(), 0, doc.story()->characterCount(), notes);
    if (notes)
        for (const int id : doc.citedFootnotes())
            if (const QTextDocument *note = doc.editableFootnote(id)) addTo(s, note, 0, note->characterCount(), true);
    return s;
}

QString proofingLanguageAt(WriteDoc &doc, const QTextCursor &c)
{
    QTextCursor at = c;
    if (at.hasSelection()) at.setPosition(at.selectionStart() + 1);
    const QString own = at.charFormat().stringProperty(tp::Language);
    if (!own.isEmpty()) return own;
    const QString doc_ = doc.proofingLanguage();
    return doc_.isEmpty() ? QStringLiteral("en-US") : doc_;
}

QTextCharFormat withProofingLanguage(QTextCharFormat f, const QString &tag)
{
    using office::wml::RawChild;
    QVector<RawChild> extra = f.hasProperty(prop::RawRPr) ? unpackRaw(storedBytes(f, prop::RawRPr)) : QVector<RawChild>();
    QByteArray element;
    for (int i = 0; i < extra.size(); ++i) {
        if (extra.at(i).xml.startsWith("<w:lang")) {
            element = extra.at(i).xml;
            extra.removeAt(i);
            break;
        }
    }
    if (tag.isEmpty()) {
        f.clearProperty(tp::Language);
    } else {
        QByteArray fresh;
        const QString safe = QString(tag).remove(QRegularExpression(QStringLiteral("[^A-Za-z0-9-]")));
        if (element.isEmpty()) {
            fresh = "<w:lang w:val=\"" + safe.toUtf8() + "\"/>";
        } else {
            // (The other attributes of the element stay: the East Asian and right-to-left languages.)
            static const QRegularExpression val(QStringLiteral("(\\sw:val=\")[^\"]*(\")"));
            QString text = QString::fromUtf8(element);
            if (val.match(text).hasMatch()) text.replace(val, QStringLiteral("\\1") + safe + QStringLiteral("\\2"));
            else text.replace(QStringLiteral("<w:lang"), QStringLiteral("<w:lang w:val=\"") + safe + QStringLiteral("\""));
            fresh = text.toUtf8();
        }
        const int rank = office::wml::runChildRank("lang");
        int at = 0;
        while (at < extra.size() && extra.at(at).rank <= rank) ++at;
        extra.insert(at, RawChild{rank, fresh});
        f.setProperty(tp::Language, tag);
    }
    if (extra.isEmpty()) f.clearProperty(prop::RawRPr);
    else setStored(f, prop::RawRPr, packRaw(extra));
    return f;
}

void setProofingLanguage(WriteDoc &doc, QTextCursor &c, const QString &tag)
{
    changeCharFormat(doc, c, [&](QTextCharFormat &f) { f = withProofingLanguage(f, tag); });
}

void setNoProof(WriteDoc &doc, QTextCursor &c, bool noProof)
{
    changeCharFormat(doc, c, [&](QTextCharFormat &f) {
        if (noProof) f.setProperty(tp::NoProof, true);
        else f.clearProperty(tp::NoProof);
    });
}

bool noProofAt(const QTextCursor &c)
{
    return allSelected(c, [](const QTextCharFormat &f) { return f.boolProperty(tp::NoProof); });
}

QString translateUrl(const QString &server, const QString &target, const QString &text)
{
    QUrl url(server.trimmed().isEmpty() ? QStringLiteral("https://libretranslate.com/") : server.trimmed(), QUrl::StrictMode);
    if (!url.isValid() || (url.scheme() != QLatin1String("https") && url.scheme() != QLatin1String("http")) || url.host().isEmpty()) return QString();
    QUrlQuery q;
    q.addQueryItem(QStringLiteral("source"), QStringLiteral("auto"));
    q.addQueryItem(QStringLiteral("target"), target);
    q.addQueryItem(QStringLiteral("q"), text.left(5000));
    url.setQuery(q);
    return url.toString(QUrl::FullyEncoded);
}

} // namespace jp::write
