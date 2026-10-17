#include "core/shapestyles.h"

#include "core/themes.h"
#include "text/textprops.h"

#include <QCoreApplication>
#include <algorithm>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>

namespace jp::deck {

namespace {

QString say(const char *text) { return QCoreApplication::translate("Deck", text); }

struct Row {
    const char *id;
    const char *name;
    int line, fill, effect;
    // The line is the accent darkened (a deeper edge on a filled shape); the text is the theme's light 1 (on the accent), else its dark 1.
    bool filled;
};

const Row kRows[] = {
    {"outline", QT_TRANSLATE_NOOP("Deck", "Outline Only"), 2, 0, 0, false},
    {"subtle", QT_TRANSLATE_NOOP("Deck", "Subtle Effect"), 1, 2, 1, true},
    {"moderate", QT_TRANSLATE_NOOP("Deck", "Moderate Effect"), 2, 3, 2, true},
    {"intense", QT_TRANSLATE_NOOP("Deck", "Intense Effect"), 3, 3, 3, true},
};

constexpr int kAccents = 6;

StyleColor accentColor(int accent, int shade = 0)
{
    StyleColor c;
    c.name = QStringLiteral("accent%1").arg(accent);
    if (shade) c.mods.append({QStringLiteral("shade"), shade});
    return c;
}

// The color an automatic text color is in a format, as the object's text has it: set, or the story's default.
QColor colorOfFormat(const QTextCharFormat &cf, const ColorScheme &scheme, bool *own)
{
    *own = true;
    if (const QString ref = cf.stringProperty(tp::ColorRefP); !ref.isEmpty()) return ColorRef::fromString(ref).resolve(scheme);
    if (cf.hasProperty(QTextFormat::ForegroundBrush) && cf.foreground().style() != Qt::NoBrush) return cf.foreground().color();
    *own = false;
    return QColor();
}

// The words of a story, those with no color of their own (or the one `automatic` is) set in `now`.
void retintText(Document &d, const QString &storyId, const QColor &automatic, const QColor &now)
{
    QTextDocument *doc = storyId.isEmpty() ? nullptr : d.storyDoc(storyId);
    if (!doc || !now.isValid()) return;
    auto isAutomatic = [&](const QTextCharFormat &cf) {
        // (Letters with a fill of their own, a gradient, are the person's: even where its first color is the one the old style gave.)
        if (!cf.stringProperty(tp::TextFill).isEmpty()) return false;
        bool own = false;
        const QColor c = colorOfFormat(cf, d.colors, &own);
        return !own || (automatic.isValid() && c.isValid() && c.rgb() == automatic.rgb());
    };
    auto tinted = [&](QTextCharFormat cf) {
        cf.setProperty(tp::ColorRefP, ColorRef::rgb(now).toString());
        cf.setProperty(tp::TextFill, QString());
        cf.setForeground(QBrush(now));
        return cf;
    };
    for (QTextBlock b = doc->begin(); b.isValid(); b = b.next()) {
        for (auto it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment f = it.fragment();
            if (!f.isValid() || !isAutomatic(f.charFormat())) continue;
            QTextCursor c(doc);
            c.setPosition(f.position());
            c.setPosition(f.position() + f.length(), QTextCursor::KeepAnchor);
            c.setCharFormat(tinted(f.charFormat()));
        }
        // (What is typed at the end of a paragraph takes the paragraph's own format.)
        if (isAutomatic(b.charFormat())) {
            QTextCursor c(b);
            c.setBlockCharFormat(tinted(b.charFormat()));
        }
    }
}

} // namespace

QVector<ShapeStyle> shapeStyles()
{
    QVector<ShapeStyle> out;
    for (const Row &row : kRows) {
        for (int accent = 1; accent <= kAccents; ++accent) {
            ShapeStyle s;
            s.id = QStringLiteral("%1:accent%2").arg(QLatin1String(row.id)).arg(accent);
            s.group = say(row.name);
            s.name = QCoreApplication::translate("Deck", "%1 - Accent %2").arg(s.group).arg(accent);
            StyleRef &r = s.ref;
            r.on = true;
            const StyleColor plain = accentColor(accent);
            r.line = {row.line, row.filled ? accentColor(accent, 50000) : plain};
            r.fill = {row.fill, plain};
            r.effect = {row.effect, plain};
            r.font = QStringLiteral("minor");
            r.fontColor.name = row.filled ? QStringLiteral("lt1") : QStringLiteral("tx1");
            out << s;
        }
    }
    return out;
}

ShapeStyle shapeStyleById(const QString &id)
{
    for (const ShapeStyle &s : shapeStyles())
        if (s.id == id) return s;
    return ShapeStyle();
}

const MasterPage *themeMasterOf(const Document &d, int page, const QString &masterView)
{
    const MasterPage *m = !masterView.isEmpty() ? d.master(masterView) : page >= 0 && page < int(d.pages.size()) ? d.masterFor(*d.pages[page]) : nullptr;
    if (m) {
        for (const MasterPage *c : d.masterChain(m))
            if (isSlideMaster(*c)) return c;
        if (isSlideMaster(*m)) return m;
    }
    for (const auto &candidate : d.masters)
        if (isSlideMaster(*candidate)) return candidate.get();
    return nullptr;
}

std::shared_ptr<office::StyleResolver> styleResolverOf(const Document &d, const MasterPage *master)
{
    QByteArray xml;
    office::ColorMap map;
    if (master) {
        xml = themePartOf(d, *master);
        // The usual map reads "bg1" as the theme's light 1 and "tx1" as its dark 1; a slide master with a dark background reads them the other way.
        if (isDarkBackground(master->background)) {
            map.bg1 = office::Theme::Dark1;
            map.tx1 = office::Theme::Light1;
            map.bg2 = office::Theme::Dark2;
            map.tx2 = office::Theme::Light2;
        }
    } else {
        xml = themePart(themeOf(d));
    }
    return std::make_shared<office::StyleResolver>(xml, map);
}

QColor accentOf(const Document &d, const MasterPage *master, int accent)
{
    const std::shared_ptr<office::StyleResolver> r = styleResolverOf(d, master);
    return r->themeColor(QStringLiteral("accent%1").arg(std::clamp(accent, 1, kAccents)));
}

bool takesShapeStyle(const Item &it)
{
    return it.type() == ItemType::Shape || it.type() == ItemType::Text || it.type() == ItemType::Line;
}

void applyShapeStyle(Document &d, Item &it, const StyleRef &ref, const office::StyleResolver &resolver)
{
    if (!takesShapeStyle(it)) return;
    const office::StyleLook look = resolver.look(ref, it.rect.size());
    // The text color the style it had gave is the one a word with no color of its own was set in: it follows the new style.
    const QColor was = it.styleRef.on ? resolver.color(it.styleRef.fontColor) : QColor();
    if (it.type() != ItemType::Line) it.fill = look.fill;
    Stroke stroke = look.stroke;
    if (it.type() == ItemType::Line) {
        stroke.startArrow = it.stroke.startArrow;
        stroke.endArrow = it.stroke.endArrow;
        stroke.startSize = it.stroke.startSize;
        stroke.endSize = it.stroke.endSize;
    }
    it.stroke = stroke;
    it.fx = look.fx;
    it.styleRef = ref;
    const QString story = it.type() == ItemType::Shape ? static_cast<ShapeItem &>(it).storyId : it.type() == ItemType::Text ? static_cast<TextItem &>(it).storyId : QString();
    retintText(d, story, was, look.text);
}

} // namespace jp::deck
