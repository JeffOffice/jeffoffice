#include "app/pagefonts.h"

#include "app/editor.h"
#include "core/document.h"
#include "text/fontreplace.h"

#include <QCoreApplication>
#include <QMap>
#include <algorithm>

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
