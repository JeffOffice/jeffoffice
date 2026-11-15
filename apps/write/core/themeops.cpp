#include "core/themeops.h"

#include "core/writeprops.h"
#include "flow/flowdocument.h"
#include "text/textprops.h"
#include "wml/wml.h"

#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextFragment>

using namespace jp::office;
using namespace jp::office::wml;

namespace jp::write {

namespace {

// The slot a w:themeColor value names (the names the file uses for the document's colors, besides those of the theme part).
bool slotOf(const QString &name, Theme::Color *slot)
{
    static const struct {
        const char *name;
        Theme::Color slot;
    } extra[] = {{"text1", Theme::Dark1}, {"background1", Theme::Light1}, {"text2", Theme::Dark2}, {"background2", Theme::Light2}, {"dark1", Theme::Dark1}, {"light1", Theme::Light1},
                 {"dark2", Theme::Dark2}, {"light2", Theme::Light2}, {"followedHyperlink", Theme::FollowedHyperlink}};
    for (const auto &e : extra)
        if (name == QLatin1String(e.name)) {
            *slot = e.slot;
            return true;
        }
    return themeColorFromName(name, slot);
}

QString hex(const QColor &c) { return c.name(QColor::HexRgb).mid(1).toUpper(); }

// The color a slot with a tint or shade (two hex digits each, as the file writes them) is in `theme`.
QColor colorOf(const Theme &theme, const QString &name, const QString &tint, const QString &shade)
{
    Theme::Color slot;
    if (!slotOf(name, &slot)) return QColor();
    QColor base = theme.colors[slot];
    bool ok = false;
    if (const int t = tint.toInt(&ok, 16); ok && !tint.isEmpty()) base = mix(QColor(Qt::white), base, t / 255.0);
    if (const int s = shade.toInt(&ok, 16); ok && !shade.isEmpty()) base = mix(QColor(Qt::black), base, s / 255.0);
    return base;
}

// The runs of a story that took a theme color (and still show it) show the new theme's.
int recolorStory(QTextDocument *story, const Theme &theme)
{
    struct Change {
        int begin, end;
        QColor color;
    };
    QVector<Change> changes;
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next())
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment fragment = it.fragment();
            const QTextCharFormat cf = fragment.charFormat();
            const QString attrs = cf.stringProperty(prop::ColorAttrs);
            if (attrs.isEmpty() || cf.stringProperty(prop::ColorAttrsRgb) != hex(cf.foreground().color())) continue;
            const QStringList parts = attrs.split(QLatin1Char(';'));
            const QColor now = colorOf(theme, parts.value(0), parts.value(1), parts.value(2));
            if (now.isValid() && now != cf.foreground().color()) changes.push_back({fragment.position(), fragment.position() + fragment.length(), now});
        }
    if (changes.isEmpty()) return 0;
    QTextCursor edit(story);
    edit.beginEditBlock();
    for (const Change &c : std::as_const(changes)) {
        QTextCursor cursor(story);
        cursor.setPosition(c.begin);
        cursor.setPosition(c.end, QTextCursor::KeepAnchor);
        QTextCharFormat f;
        f.setForeground(QBrush(c.color));
        f.setProperty(tp::ColorRefP, ColorRef::rgb(c.color).toString());
        f.setProperty(prop::ColorAttrsRgb, hex(c.color));
        cursor.mergeCharFormat(f);
    }
    edit.endEditBlock();
    return int(changes.size());
}

// The drawings of a story that name theme colors are made again, so they are drawn in the new theme's.
int redrawDrawings(WriteDoc &doc, QTextDocument *story)
{
    struct Drawing {
        int position;
        Inline atom;
        QTextCharFormat format;
    };
    QVector<Drawing> found;
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next())
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextCharFormat cf = it.fragment().charFormat();
            if (!cf.hasProperty(prop::Atom)) continue;
            const Inline a = unpackAtom(storedBytes(cf, prop::Atom));
            if (a.kind != Inline::RunAtom || !a.raw.contains("<w:drawing") || !a.raw.contains("schemeClr")) continue;
            found.push_back({it.fragment().position(), a, cf});
        }
    int done = 0;
    QTextCursor t(story);
    for (const Drawing &d : std::as_const(found)) {
        QTextCharFormat f = d.format;
        f.clearProperty(flow::fp::AnchorId);
        f.clearProperty(tp::InlineObject);
        setStored(f, prop::Atom, packAtom(d.atom));
        if (!doc.makeDrawnFormat(d.atom, &f)) continue;
        t.setPosition(d.position);
        t.setPosition(d.position + 1, QTextCursor::KeepAnchor);
        t.setCharFormat(f);
        ++done;
    }
    if (done) doc.keepObjectsForTheHistory();
    return done;
}

bool same(const Theme &a, const Theme &b)
{
    for (int i = 0; i < Theme::ColorCount; ++i)
        if (a.colors[i] != b.colors[i]) return false;
    return a.majorLatin == b.majorLatin && a.minorLatin == b.minorLatin && a.colorSchemeName == b.colorSchemeName && a.fontSchemeName == b.fontSchemeName && a.xml == b.xml;
}

// Puts `to` as the document's theme: the part, the styles and runs that follow its colors, and the fonts the layout takes from it.
bool change(WriteDoc &doc, const Theme &to)
{
    if (!canChangeTheme(doc)) return false;
    Theme next = to;
    if (next.xml.isEmpty()) next.xml = writeTheme(next);
    if (same(documentTheme(doc), next)) return false;
    const WriteDoc::StepGroup together(doc);
    QTextDocument *body = doc.story();
    QTextCursor edit(body);
    edit.beginEditBlock();
    WriteDoc::FontParts parts = doc.fontParts();
    parts.theme = next;
    parts.themePart = next.xml;
    doc.putFontParts(parts);
    // the styles that name a theme color
    for (const Style &s : doc.styleSheet().styles) {
        if (!s.rPr.color || s.rPr.color->theme.isEmpty()) continue;
        const QColor now = colorOf(next, s.rPr.color->theme, s.rPr.color->tint, s.rPr.color->shade);
        if (!now.isValid() || hex(now) == s.rPr.color->val) continue;
        Style changed = s;
        changed.rPr.color->val = hex(now);
        changed.raw.clear();
        doc.putStyle(changed);
    }
    recolorStory(body, next);
    redrawDrawings(doc, body);
    for (QTextDocument *story : doc.storiesWithChanges(true)) {
        recolorStory(story, next);
        redrawDrawings(doc, story);
    }
    doc.markStyleStep();
    edit.endEditBlock();
    doc.settleStyleEdit();
    return true;
}

} // namespace

bool canChangeTheme(const WriteDoc &doc) { return doc.theme() != nullptr; }

Theme documentTheme(const WriteDoc &doc)
{
    if (!doc.theme()) return Theme();
    Theme t = *doc.theme();
    const WriteDoc::FontParts parts = doc.fontParts();
    if (!parts.themePart.isEmpty()) t.xml = parts.themePart;
    return t;
}

bool applyThemeColors(WriteDoc &doc, const ThemeColors &colors)
{
    if (!canChangeTheme(doc)) return false;
    return change(doc, withThemeColors(documentTheme(doc), colors));
}

bool applyThemeFonts(WriteDoc &doc, const ThemeFonts &fonts)
{
    if (!canChangeTheme(doc)) return false;
    return change(doc, withThemeFonts(documentTheme(doc), fonts));
}

bool applyTheme(WriteDoc &doc, const Theme &theme)
{
    if (!canChangeTheme(doc)) return false;
    return change(doc, theme);
}

} // namespace jp::write
