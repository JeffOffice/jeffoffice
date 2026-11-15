#include "core/fontops.h"

#include "core/writeprops.h"
#include "flow/flowdocument.h"
#include "text/fontreplace.h"
#include "text/textprops.h"
#include "wml/wml.h"

#include <QCoreApplication>
#include <QMap>
#include <QRegularExpression>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextFragment>
#include <algorithm>

using namespace jp::office::wml;

namespace jp::write {

namespace {

bool same(const QString &a, const QString &b) { return a.compare(b, Qt::CaseInsensitive) == 0; }

// The fonts a style or a run names for each script (a slot that follows the theme names none).
void forEachNamedFont(const Fonts &f, const std::function<void(const QString &)> &fn)
{
    if (f.asciiTheme.isEmpty() && !f.ascii.isEmpty()) fn(f.ascii);
    if (f.hAnsiTheme.isEmpty() && !f.hAnsi.isEmpty() && !same(f.hAnsi, f.ascii)) fn(f.hAnsi);
    if (f.eastAsiaTheme.isEmpty() && !f.eastAsia.isEmpty()) fn(f.eastAsia);
    if (f.csTheme.isEmpty() && !f.cs.isEmpty()) fn(f.cs);
}

bool replaceInFonts(Fonts *f, const QString &from, const QString &to)
{
    bool changed = false;
    auto one = [&](QString &name, const QString &theme) {
        if (theme.isEmpty() && same(name, from) && name != to) {
            name = to;
            changed = true;
        }
    };
    one(f->ascii, f->asciiTheme);
    one(f->hAnsi, f->hAnsiTheme);
    one(f->eastAsia, f->eastAsiaTheme);
    one(f->cs, f->csTheme);
    return changed;
}

QString unescaped(QString v)
{
    v.replace(QLatin1String("&lt;"), QLatin1String("<")).replace(QLatin1String("&gt;"), QLatin1String(">")).replace(QLatin1String("&quot;"), QLatin1String("\"")).replace(QLatin1String("&apos;"), QLatin1String("'"));
    return v.replace(QLatin1String("&amp;"), QLatin1String("&"));
}

// The fonts a piece of the file's XML names (a run's w:rFonts, a drawing's a:latin and its kin), counted (one use for each element, whatever the slots of a w:rFonts say), or put in the
// place of `from` (when `counts` is null): how many elements changed. A slot that a theme attribute overrides (w:ascii beside w:asciiTheme) names no font.
int fontsInXml(QByteArray *xml, const QString &from, const QString &to, QMap<QString, int> *counts)
{
    static const QRegularExpression element(QStringLiteral("<(?:w:rFonts|a:latin|a:ea|a:cs)\\b[^>]*>"));
    static const QRegularExpression attribute(QStringLiteral("(\\b(?:w:ascii|w:hAnsi|w:eastAsia|w:cs|typeface))=\"([^\"]*)\""));
    auto overridden = [](const QString &tag, const QString &slot) {
        if (slot == QLatin1String("w:ascii")) return tag.contains(QLatin1String("w:asciiTheme="));
        if (slot == QLatin1String("w:hAnsi")) return tag.contains(QLatin1String("w:hAnsiTheme="));
        if (slot == QLatin1String("w:eastAsia")) return tag.contains(QLatin1String("w:eastAsiaTheme="));
        if (slot == QLatin1String("w:cs")) return tag.contains(QLatin1String("w:cstheme=")) || tag.contains(QLatin1String("w:csTheme="));
        return false;
    };
    const QString text = QString::fromUtf8(*xml);
    int n = 0;
    QString out;
    qsizetype last = 0;
    auto it = element.globalMatch(text);
    while (it.hasNext()) {
        const QRegularExpressionMatch m = it.next();
        const QString tag = m.captured(0);
        QStringList named;
        QString rewritten;
        qsizetype taken = 0;
        auto slots = attribute.globalMatch(tag);
        while (slots.hasNext()) {
            const QRegularExpressionMatch a = slots.next();
            const QString name = unescaped(a.captured(2));
            if (name.isEmpty() || name.startsWith(QLatin1Char('+')) || overridden(tag, a.captured(1))) continue;
            if (!named.contains(name, Qt::CaseInsensitive)) named << name;
            if (counts || !same(name, from) || name == to) continue;
            rewritten += tag.mid(taken, a.capturedStart() - taken) + a.captured(1) + QStringLiteral("=\"") + to.toHtmlEscaped() + QLatin1Char('"');
            taken = a.capturedEnd();
        }
        if (counts) {
            for (const QString &name : std::as_const(named)) ++(*counts)[name];
            continue;
        }
        if (taken == 0) continue;
        rewritten += tag.mid(taken);
        out += text.mid(last, m.capturedStart() - last) + rewritten;
        last = m.capturedEnd();
        ++n;
    }
    if (counts) return 0;
    if (n > 0) {
        out += text.mid(last);
        *xml = out.toUtf8();
    }
    return n;
}

// The drawings of a story (shapes and text boxes keep their words as XML inside the character that stands for them).
struct Drawing {
    int position;
    Inline atom;
    QTextCharFormat format;
};

QVector<Drawing> drawingsOf(QTextDocument *story)
{
    QVector<Drawing> out;
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextCharFormat cf = it.fragment().charFormat();
            if (!cf.hasProperty(prop::Atom)) continue;
            const Inline a = unpackAtom(storedBytes(cf, prop::Atom));
            if (a.kind != Inline::RunAtom || !a.raw.contains("<w:drawing")) continue;
            out.push_back({it.fragment().position(), a, cf});
        }
    }
    return out;
}

// What the runs of a story name for East Asian and complex scripts (a packed string of a run).
void otherFontsOf(const QTextDocument *story, const std::function<void(const QString &)> &fn)
{
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next())
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextCharFormat cf = it.fragment().charFormat();
            if (cf.hasProperty(prop::FontsOther)) forEachNamedFont(unpackOtherFonts(cf.stringProperty(prop::FontsOther)), fn);
        }
}

int replaceOtherFonts(QTextDocument *story, const QString &from, const QString &to)
{
    struct Change {
        int begin, end;
        QString packed;
    };
    QVector<Change> changes;
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next())
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment fragment = it.fragment();
            const QTextCharFormat cf = fragment.charFormat();
            if (!cf.hasProperty(prop::FontsOther)) continue;
            Fonts o = unpackOtherFonts(cf.stringProperty(prop::FontsOther));
            if (replaceInFonts(&o, from, to)) changes.push_back({fragment.position(), fragment.position() + fragment.length(), packOtherFonts(o)});
        }
    if (changes.isEmpty()) return 0;
    QTextCursor edit(story);
    edit.beginEditBlock();
    for (const Change &c : std::as_const(changes)) {
        QTextCursor cursor(story);
        cursor.setPosition(c.begin);
        cursor.setPosition(c.end, QTextCursor::KeepAnchor);
        QTextCharFormat f;
        f.setProperty(prop::FontsOther, c.packed);
        cursor.mergeCharFormat(f);
    }
    edit.endEditBlock();
    return int(changes.size());
}

int replaceInDrawings(WriteDoc &doc, QTextDocument *story, const QString &from, const QString &to)
{
    int done = 0;
    QTextCursor t(story);
    for (const Drawing &d : drawingsOf(story)) {
        Inline made = d.atom;
        const int n = fontsInXml(&made.raw, from, to, nullptr);
        if (n == 0) continue;
        QTextCharFormat f = d.format;
        f.clearProperty(flow::fp::AnchorId);
        f.clearProperty(tp::InlineObject);
        setStored(f, prop::Atom, packAtom(made));
        if (!doc.makeDrawnFormat(made, &f)) continue;
        t.setPosition(d.position);
        t.setPosition(d.position + 1, QTextCursor::KeepAnchor);
        t.setCharFormat(f);
        done += n;
    }
    if (done) doc.keepObjectsForTheHistory();
    return done;
}

int replaceInStory(WriteDoc &doc, QTextDocument *story, const QString &from, const QString &to)
{
    int n = replaceFontFamily(story, from, to);
    n += replaceOtherFonts(story, from, to);
    n += replaceInDrawings(doc, story, from, to);
    return n;
}

// The font a level of a list names for its number (a bullet's font), in each of the document's definitions and in the levels an id writes out itself.
template <typename Fn>
void eachLevelRun(Numbering &numbering, Fn &&fn)
{
    for (AbstractNum &a : numbering.abstracts)
        for (Level &l : a.levels)
            if (l.present) fn(l.rPr, &a);
    for (Num &n : numbering.nums)
        for (Level &l : n.levelOverrides) fn(l.rPr, static_cast<AbstractNum *>(nullptr));
}

bool isScheme(const QString &key) { return key == QLatin1String("+Heading") || key == QLatin1String("+Body"); }

} // namespace

QVector<FontUse> fontsUsed(WriteDoc &doc)
{
    QMap<QString, int> content, styles;
    int major = 0, minor = 0;
    auto fromStory = [&](QTextDocument *story) {
        collectFontFamilies(story, &content, &major, &minor);
        otherFontsOf(story, [&](const QString &n) { ++content[n]; });
        for (const Drawing &d : drawingsOf(story)) {
            QByteArray raw = d.atom.raw;
            fontsInXml(&raw, QString(), QString(), &content);
        }
    };
    fromStory(doc.story());
    for (QTextDocument *story : doc.storiesWithChanges(true)) fromStory(story);
    // What only styles (and the defaults and the list definitions) name is not content: it is listed as styles only.
    int themeInStyles = 0;
    auto fromRun = [&](const RunProps &r) {
        if (!r.fonts) return;
        forEachNamedFont(*r.fonts, [&](const QString &n) { ++styles[n]; });
        if (!r.fonts->asciiTheme.isEmpty() || !r.fonts->hAnsiTheme.isEmpty()) ++themeInStyles;
    };
    for (const Style &s : doc.styleSheet().styles) {
        fromRun(s.rPr);
        if (!s.tail.isEmpty()) {
            QByteArray raw = s.tail;
            fontsInXml(&raw, QString(), QString(), &styles);
        }
    }
    fromRun(doc.styleSheet().defaultRun);
    {
        QByteArray raw = doc.styleSheet().docDefaults;
        fontsInXml(&raw, QString(), QString(), &styles);
    }
    {
        Numbering numbering = doc.numbering();
        eachLevelRun(numbering, [&](RunProps &r, AbstractNum *) { fromRun(r); });
    }
    QVector<FontUse> out = fontUsesOf(content, styles);
    // The theme's two fonts, when the document has a theme: offered, and changed only when picked.
    if (const office::Theme *theme = doc.theme()) {
        if (!theme->majorLatin.isEmpty() && (major > 0 || themeInStyles > 0))
            out.push_back({QStringLiteral("+Heading"), QCoreApplication::translate("jp::ReplaceFonts", "%1 (Headings)").arg(theme->majorLatin), major, true});
        if (!theme->minorLatin.isEmpty() && (minor > 0 || themeInStyles > 0))
            out.push_back({QStringLiteral("+Body"), QCoreApplication::translate("jp::ReplaceFonts", "%1 (Body)").arg(theme->minorLatin), minor, true});
    }
    return out;
}

int replaceFont(WriteDoc &doc, const QString &from, const QString &to)
{
    if (from.isEmpty() || to.trimmed().isEmpty()) return 0;
    const WriteDoc::StepGroup together(doc);
    int n = 0;
    QTextDocument *body = doc.story();
    {
        // The styles, the defaults, the list definitions, and the theme are steps of the text's history: they change in the edit that changes the text.
        QTextCursor edit(body);
        edit.beginEditBlock();
        bool styled = false;
        if (isScheme(from)) {
            // The theme's own font (not the runs that follow it, which stay on it).
            WriteDoc::FontParts parts = doc.fontParts();
            const bool major = from == QLatin1String("+Heading");
            QString &name = major ? parts.theme.majorLatin : parts.theme.minorLatin;
            if (doc.theme() && !name.isEmpty() && name != to) {
                static const QRegularExpression majorFont(QStringLiteral("(<a:majorFont>\\s*<a:latin typeface=\")[^\"]*(\")")), minorFont(QStringLiteral("(<a:minorFont>\\s*<a:latin typeface=\")[^\"]*(\")"));
                QString xml = QString::fromUtf8(parts.themePart);
                xml.replace(major ? majorFont : minorFont, QStringLiteral("\\1") + QString(to).toHtmlEscaped().replace(QLatin1Char('\\'), QStringLiteral("\\\\")) + QStringLiteral("\\2"));
                parts.themePart = xml.toUtf8();
                parts.theme.xml.clear();
                name = to;
                // (every run that follows the theme's font is a use)
                int followed = 0, other = 0;
                QMap<QString, int> ignored;
                collectFontFamilies(body, &ignored, major ? &followed : &other, major ? &other : &followed);
                for (QTextDocument *story : doc.storiesWithChanges(true)) collectFontFamilies(story, &ignored, major ? &followed : &other, major ? &other : &followed);
                doc.putFontParts(parts);
                styled = true;
                n = std::max(followed, 1);
            }
        } else {
            for (const Style &s : doc.styleSheet().styles) {
                Style changed = s;
                bool did = false;
                if (changed.rPr.fonts) did = replaceInFonts(&*changed.rPr.fonts, from, to);
                if (!changed.tail.isEmpty()) did = fontsInXml(&changed.tail, from, to, nullptr) > 0 || did;
                if (!did) continue;
                changed.raw.clear();
                doc.putStyle(changed);
                styled = true;
                ++n;
            }
            // The defaults of the style sheet, and the fonts of the lists' numbers.
            WriteDoc::FontParts parts = doc.fontParts();
            int parted = 0;
            if (parts.defaultRun.fonts && replaceInFonts(&*parts.defaultRun.fonts, from, to)) ++parted;
            parted += fontsInXml(&parts.docDefaults, from, to, nullptr);
            eachLevelRun(parts.numbering, [&](RunProps &r, AbstractNum *a) {
                if (r.fonts && replaceInFonts(&*r.fonts, from, to)) {
                    ++parted;
                    if (a) a->raw.clear();
                }
            });
            if (parted > 0) {
                for (Num &num : parts.numbering.nums) num.raw.clear();
                parts.numbering.modified = true;
                doc.putFontParts(parts);
                styled = true;
                n += parted;
            }
        }
        if (styled) doc.markStyleStep();
        if (!isScheme(from)) n += replaceInStory(doc, body, from, to);
        edit.endEditBlock();
        doc.settleStyleEdit();
    }
    if (!isScheme(from))
        for (QTextDocument *story : doc.storiesWithChanges(true)) n += replaceInStory(doc, story, from, to);
    return n;
}

} // namespace jp::write
