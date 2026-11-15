#include "core/fontops.h"

#include "core/writeprops.h"
#include "flow/flowdocument.h"
#include "text/fontreplace.h"
#include "text/textprops.h"
#include "wml/wml.h"

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

// The fonts a piece of the file's XML names (a run's w:rFonts, a drawing's a:latin and its kin), counted (one use for each element, whatever the slots of a w:rFonts say), or put in the
// place of `from` (when `counts` is null): how many elements changed.
QString unescaped(QString v)
{
    v.replace(QLatin1String("&lt;"), QLatin1String("<")).replace(QLatin1String("&gt;"), QLatin1String(">")).replace(QLatin1String("&quot;"), QLatin1String("\"")).replace(QLatin1String("&apos;"), QLatin1String("'"));
    return v.replace(QLatin1String("&amp;"), QLatin1String("&"));
}

int fontsInXml(QByteArray *xml, const QString &from, const QString &to, QMap<QString, int> *counts)
{
    static const QRegularExpression element(QStringLiteral("<(?:w:rFonts|a:latin|a:ea|a:cs)\\b[^>]*>"));
    static const QRegularExpression attribute(QStringLiteral("(\\b(?:w:ascii|w:hAnsi|w:eastAsia|w:cs|typeface))=\"([^\"]*)\""));
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
            if (name.isEmpty() || name.startsWith(QLatin1Char('+'))) continue;
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

} // namespace

QVector<FontUse> fontsUsed(WriteDoc &doc)
{
    QMap<QString, int> uses;
    auto fromStory = [&](QTextDocument *story) {
        collectFontFamilies(story, &uses);
        otherFontsOf(story, [&](const QString &n) { ++uses[n]; });
        for (const Drawing &d : drawingsOf(story)) {
            QByteArray raw = d.atom.raw;
            fontsInXml(&raw, QString(), QString(), &uses);
        }
    };
    fromStory(doc.story());
    for (QTextDocument *story : doc.storiesWithChanges(true)) fromStory(story);
    for (const Style &s : doc.styleSheet().styles) {
        if (s.rPr.fonts) forEachNamedFont(*s.rPr.fonts, [&](const QString &n) { ++uses[n]; });
        if (!s.tail.isEmpty()) {
            QByteArray raw = s.tail;
            fontsInXml(&raw, QString(), QString(), &uses);
        }
    }
    // (Names that differ only in case are one font: the first spelling stands for them.)
    QVector<FontUse> merged;
    for (const FontUse &u : fontUsesOf(uses)) {
        bool found = false;
        for (FontUse &m : merged)
            if (same(m.key, u.key)) {
                m.uses += u.uses;
                found = true;
            }
        if (!found) merged.push_back(u);
    }
    std::stable_sort(merged.begin(), merged.end(), [](const FontUse &a, const FontUse &b) { return a.uses > b.uses; });
    return merged;
}

int replaceFont(WriteDoc &doc, const QString &from, const QString &to)
{
    if (from.isEmpty() || to.trimmed().isEmpty()) return 0;
    bool named = false;
    for (const FontUse &u : fontsUsed(doc)) named = named || same(u.key, from);
    if (!named) return 0;
    const WriteDoc::StepGroup together(doc);
    int n = 0;
    QTextDocument *body = doc.story();
    {
        // The styles are steps of the text's history: they change in the edit that changes the text.
        QTextCursor edit(body);
        edit.beginEditBlock();
        bool styled = false;
        const QVector<Style> styles = doc.styleSheet().styles;
        for (const Style &s : styles) {
            Style changed = s;
            bool did = false;
            if (changed.rPr.fonts) did = replaceInFonts(&*changed.rPr.fonts, from, to);
            if (!changed.tail.isEmpty()) {
                const int k = fontsInXml(&changed.tail, from, to, nullptr);
                did = did || k > 0;
            }
            if (!did) continue;
            changed.raw.clear();
            doc.putStyle(changed);
            styled = true;
            ++n;
        }
        if (styled) doc.markStyleStep();
        n += replaceInStory(doc, body, from, to);
        edit.endEditBlock();
        doc.settleStyleEdit();
    }
    for (QTextDocument *story : doc.storiesWithChanges(true)) n += replaceInStory(doc, story, from, to);
    return n;
}

} // namespace jp::write
