#include "app/pagefonts.h"

#include "app/editor.h"
#include "core/document.h"
#include "text/fontreplace.h"

#include <QCoreApplication>
#include <QMap>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <algorithm>
#include <utility>

namespace jp {

QVector<FontUse> pageFontsUsed(const Document &doc)
{
    QMap<QString, int> content, styles;
    int major = 0, minor = 0;
    for (const auto &story : doc.stories)
        if (story && story->doc) collectFontFamilies(story->doc.get(), &content, &major, &minor);
    for (const TextStyle &style : doc.styles) {
        const QStringList families = fontFamiliesOf(style.chr);
        if (!families.isEmpty() && !families.first().isEmpty()) ++styles[families.first()];
    }
    doc.forEachItem([&](Item *item, int, const QString &) {
        if (item->type() == ItemType::TextArt) {
            const QString font = static_cast<TextArtItem *>(item)->font;
            if (!font.isEmpty()) ++content[font];
        }
    });
    // (A font a .pub file named that nothing else names is a font of the file, not of any content.)
    for (const QString &name : doc.pubFonts)
        if (!name.isEmpty()) styles.insert(name, styles.value(name));
    QVector<FontUse> out = fontUsesOf(content, styles);
    if (major > 0) out.push_back({QStringLiteral("+Heading"), QCoreApplication::translate("jp::ReplaceFonts", "%1 (Headings)").arg(doc.fonts.heading), major, true});
    if (minor > 0) out.push_back({QStringLiteral("+Body"), QCoreApplication::translate("jp::ReplaceFonts", "%1 (Body)").arg(doc.fonts.body), minor, true});
    return out;
}

int replacePageFont(Document &doc, const QString &key, const QString &to)
{
    if (key.isEmpty() || to.trimmed().isEmpty()) return 0;
    int changed = 0;
    if (key == QLatin1String("+Heading") || key == QLatin1String("+Body")) {
        QString &scheme = key == QLatin1String("+Heading") ? doc.fonts.heading : doc.fonts.body;
        if (scheme == to) return 0;
        // (Every run that follows the theme's font is a use.)
        for (const auto &story : doc.stories) {
            if (!story || !story->doc) continue;
            QMap<QString, int> ignored;
            int major = 0, minor = 0;
            collectFontFamilies(story->doc.get(), &ignored, &major, &minor);
            changed += key == QLatin1String("+Heading") ? major : minor;
        }
        scheme = to;
        return std::max(changed, 1);
    }
    for (const auto &story : doc.stories)
        if (story && story->doc) changed += replaceFontFamily(story->doc.get(), key, to);
    for (TextStyle &style : doc.styles)
        if (replaceFontFamilyIn(&style.chr, key, to)) ++changed;
    doc.forEachItem([&](Item *item, int, const QString &) {
        if (item->type() == ItemType::TextArt) {
            auto *art = static_cast<TextArtItem *>(item);
            if (art->font.compare(key, Qt::CaseInsensitive) == 0 && art->font != to) {
                art->font = to;
                ++changed;
            }
        }
    });
    for (QString &name : doc.pubFonts)
        if (name.compare(key, Qt::CaseInsensitive) == 0) name = to;
    doc.pubFonts.removeDuplicates();
    return changed;
}

ReplaceFontsResult askReplacePageFonts(QWidget *parent, Editor *ed, const std::function<int(Document &, const QString &, const QString &)> &theme)
{
    ed->endTextEdit();
    ReplaceFontsHost host;
    host.fontsUsed = [ed] { return pageFontsUsed(*ed->doc()); };
    host.replace = [ed, theme](const QString &key, const QString &to) {
        int n = 0;
        ed->change(QCoreApplication::translate("jp::ReplaceFonts", "Replace Fonts"), [&] {
            Document &d = *ed->doc();
            const bool scheme = key == QLatin1String("+Heading") || key == QLatin1String("+Body");
            n = scheme && theme ? theme(d, key, to) : replacePageFont(d, key, to);
        });
        return n;
    };
    return askReplaceFonts(parent, host);
}

} // namespace jp

namespace jp {

namespace {

const FindScope kEveryStory{true, true, 0, false};

QTextDocument *wordsOf(const Document &doc, const FindStory &s) { return s.art ? nullptr : doc.storyDoc(s.storyId); }

}

MissingFontsHost pageMissingFontsHost(Editor *ed, const std::function<QString(const FindStory &)> &place, const std::function<bool()> &refuse)
{
    MissingFontsHost host;
    host.usage = [ed, place] {
        FontUsage u;
        const Document &doc = *ed->doc();
        for (const FindStory &s : findStories(doc, kEveryStory))
            if (const QTextDocument *d = wordsOf(doc, s)) u.addDocument(d, place(s));
        return u;
    };
    host.replace = [ed, refuse](const QString &from, const QString &to) {
        if (refuse && refuse()) return -1;
        int changed = 0;
        ed->change(QCoreApplication::translate("jp::MissingFonts", "Replace Font"), [&] {
            const Document &doc = *ed->doc();
            for (const FindStory &s : findStories(doc, kEveryStory)) {
                QTextDocument *d = wordsOf(doc, s);
                if (!d) continue;
                QVector<std::pair<int, int>> runs;
                // Text that names no font is set in the story's own (default) font, which is the missing one then: it is named, and the default is changed too.
                const bool ownIsFrom = d->defaultFont().family().compare(from, Qt::CaseInsensitive) == 0;
                for (QTextBlock b = d->begin(); b.isValid(); b = b.next())
                    for (auto f = b.begin(); !f.atEnd(); ++f) {
                        const QStringList fams = f.fragment().charFormat().fontFamilies().toStringList();
                        if (fams.isEmpty() ? ownIsFrom : fams.first().compare(from, Qt::CaseInsensitive) == 0) runs.push_back({f.fragment().position(), f.fragment().length()});
                    }
                if (ownIsFrom) {
                    QFont own = d->defaultFont();
                    own.setFamily(to);
                    d->setDefaultFont(own);
                    if (runs.isEmpty()) ++changed;   // (a story with no text: its font is a place the font was used)
                }
                QTextCursor edit(d);
                edit.beginEditBlock();
                // The paragraph marks (and the marks of list items) that name the font.
                for (QTextBlock b = d->begin(); b.isValid(); b = b.next()) {
                    const QStringList fams = b.charFormat().fontFamilies().toStringList();
                    if (fams.isEmpty() || fams.first().compare(from, Qt::CaseInsensitive) != 0) continue;
                    QTextCharFormat mark = b.charFormat();
                    mark.setFontFamilies({to});
                    QTextCursor bc(b);
                    bc.setBlockCharFormat(mark);
                    ++changed;
                }
                for (const auto &r : std::as_const(runs)) {
                    QTextCursor c(d);
                    c.setPosition(r.first);
                    c.setPosition(r.first + r.second, QTextCursor::KeepAnchor);
                    QTextCharFormat f;
                    f.setFontFamilies({to});
                    c.mergeCharFormat(f);
                    ++changed;
                }
                edit.endEditBlock();
            }
        });
        return changed;
    };
    host.refresh = [ed] {
        ed->cache().clear();
        Q_EMIT ed->viewChanged();
    };
    return host;
}

} // namespace jp
