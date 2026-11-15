#include "app/pagefonts.h"

#include "core/document.h"
#include "text/fontreplace.h"

#include <QCoreApplication>
#include <QMap>

namespace jp {

QVector<FontUse> pageFontsUsed(const Document &doc)
{
    QMap<QString, int> uses;
    int major = 0, minor = 0;
    for (const auto &story : doc.stories)
        if (story && story->doc) collectFontFamilies(story->doc.get(), &uses, &major, &minor);
    for (const TextStyle &style : doc.styles) {
        const QStringList families = fontFamiliesOf(style.chr);
        if (!families.isEmpty() && !families.first().isEmpty()) ++uses[families.first()];
    }
    doc.forEachItem([&](Item *item, int, const QString &) {
        if (item->type() == ItemType::TextArt) {
            const QString font = static_cast<TextArtItem *>(item)->font;
            if (!font.isEmpty()) ++uses[font];
        }
    });
    // (A font a .pub file named and no run names any more is not a use.)
    QVector<FontUse> out = fontUsesOf(uses);
    // (Names that differ only in case are one font: the first spelling stands for them.)
    QVector<FontUse> merged;
    for (const FontUse &u : std::as_const(out)) {
        bool found = false;
        for (FontUse &m : merged)
            if (m.key.compare(u.key, Qt::CaseInsensitive) == 0) {
                m.uses += u.uses;
                found = true;
            }
        if (!found) merged.push_back(u);
    }
    if (major > 0) merged.push_back({QStringLiteral("+Heading"), QCoreApplication::translate("jp::ReplaceFonts", "%1 (Headings)").arg(doc.fonts.heading), major, true});
    if (minor > 0) merged.push_back({QStringLiteral("+Body"), QCoreApplication::translate("jp::ReplaceFonts", "%1 (Body)").arg(doc.fonts.body), minor, true});
    return merged;
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

} // namespace jp
