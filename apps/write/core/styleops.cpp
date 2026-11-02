#include "core/styleops.h"

#include "core/editops.h"
#include "core/wordformat.h"
#include "core/writeprops.h"
#include "flow/flowdocument.h"
#include "text/textprops.h"

#include <QCoreApplication>
#include <QHash>
#include <QSet>
#include <QTextBlock>
#include <QTextDocument>
#include <algorithm>

using namespace jp::office;
using namespace jp::office::wml;

namespace jp::write {

namespace {

QString capitalized(QString name)
{
    if (!name.isEmpty() && name[0].isLower()) name[0] = name[0].toUpper();
    return name;
}

QString shownName(const Style &s) { return capitalized((s.name.isEmpty() ? s.id : s.name).left(kMostStyleNameLength)); }

QString styleOfBlock(const QTextBlockFormat &bf) { return bf.hasProperty(prop::EndnoteBlock) ? bf.stringProperty(prop::EndnoteStyle) : bf.stringProperty(tp::StyleName); }

// The ids of the styles of the same kind that are `id` or based, however far, on it (every style is looked at once; a chain that names itself is followed no further than once).
QSet<QString> descendantsOf(const StyleSheet &sheet, const QString &id, const QString &type)
{
    QHash<QString, QStringList> kids;
    for (const Style &s : sheet.styles)
        if (!s.basedOn.isEmpty() && s.type == type) kids[s.basedOn].push_back(s.id);
    QSet<QString> out{id};
    QStringList work{id};
    while (!work.isEmpty()) {
        const QString at = work.takeLast();
        for (const QString &kid : kids.value(at))
            if (!out.contains(kid)) {
                out.insert(kid);
                work.push_back(kid);
            }
    }
    return out;
}

// What a style is, in the bytes it is written as (the same bytes: the same style).
QByteArray bytesOf(Style s)
{
    s.raw.clear();
    return styleXml(s);
}

// The format with what `was` gave it (and it still has) taken for what `now` gives.
QTextBlockFormat restyledBlock(const QTextBlockFormat &bf, const QTextBlockFormat &was, const QTextBlockFormat &now)
{
    QTextBlockFormat out = bf;
    QSet<int> keys;
    const QMap<int, QVariant> before = was.properties(), after = now.properties();
    for (auto it = before.cbegin(); it != before.cend(); ++it) keys.insert(it.key());
    for (auto it = after.cbegin(); it != after.cend(); ++it) keys.insert(it.key());
    for (int key : keys) {
        if (paragraphKeepsOwn(key) || bf.property(key) != was.property(key)) continue;   // (the paragraph says this itself)
        if (now.hasProperty(key)) out.setProperty(key, now.property(key));
        else out.clearProperty(key);
    }
    return out;
}

// Makes a change to the styles a step of the history of the text, whatever paragraphs it restyles: the first paragraph's count of changes goes up.
void touchStyles(WriteDoc &doc)
{
    doc.markStyleStep();
}

// Every paragraph and run of the text whose look the old style gave it follows the new one (in the text, not in the headers, footers, and notes, whose stories are made when they are
// first edited). `old` is the style sheet as it was, and is the document's own again when this returns.
void restyleText(WriteDoc &doc, StyleSheet &old, const QSet<QString> &paragraphStyles, const QSet<QString> &characterStyles)
{
    QHash<QString, QTextBlockFormat> blockWas, blockNow;
    QHash<QString, QTextCharFormat> charWas, charNow;
    struct Forget {
        WriteDoc &doc;
        ~Forget() { doc.forgetSwappedScratch(); }
    } forget{doc};
    doc.forgetSwappedScratch();
    const QString normal = doc.defaultParagraphStyle();
    auto blockFormat = [&](const QString &style, bool was) {
        QHash<QString, QTextBlockFormat> &cache = was ? blockWas : blockNow;
        const auto found = cache.constFind(style);
        if (found != cache.constEnd()) return found.value();
        if (was) doc.swapStyleSheet(old);
        const QTextBlockFormat bf = doc.styleBlockFormat(style);
        if (was) doc.swapStyleSheet(old);
        cache.insert(style, bf);
        return bf;
    };
    auto charFormat = [&](const QString &style, const QString &character, bool was) {
        QHash<QString, QTextCharFormat> &cache = was ? charWas : charNow;
        const QString key = style + QLatin1Char('\n') + character;
        const auto found = cache.constFind(key);
        if (found != cache.constEnd()) return found.value();
        if (was) doc.swapStyleSheet(old);
        const QTextCharFormat cf = doc.styleCharFormat(style, character);
        if (was) doc.swapStyleSheet(old);
        cache.insert(key, cf);
        return cf;
    };
    struct Edit {
        int from, to;
        QTextCharFormat format;
    };
    for (QTextBlock b = doc.story()->begin(); b.isValid(); b = b.next()) {
        if (isLocked(b)) continue;
        const QTextBlockFormat bf = b.blockFormat();
        QString style = styleOfBlock(bf);
        if (style.isEmpty()) style = normal;
        const bool whole = paragraphStyles.contains(style);
        if (whole) {
            QTextBlockFormat nf = restyledBlock(bf, blockFormat(style, true), blockFormat(style, false));
            if (nf != bf) {
                nf.setObjectIndex(bf.objectIndex());
                QTextCursor(b).setBlockFormat(nf);
            }
        }
        QVector<Edit> edits;
        for (QTextBlock::iterator it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment f = it.fragment();
            const QString character = f.charFormat().stringProperty(tp::CharStyle);
            if (!whole && !(characterStyles.contains(character) && !character.isEmpty())) continue;
            const QTextCharFormat nfmt = restyleCharFormat(f.charFormat(), charFormat(style, character, true), charFormat(style, character, false));
            if (nfmt != f.charFormat()) edits.push_back({f.position(), f.position() + f.length(), nfmt});
        }
        for (const Edit &e : std::as_const(edits)) {
            QTextCursor t(b);
            t.setPosition(e.from);
            t.setPosition(e.to, QTextCursor::KeepAnchor);
            t.setCharFormat(e.format);
        }
        if (whole) setMarkFormat(b, restyleCharFormat(markFormat(b), charFormat(style, QString(), true), charFormat(style, QString(), false)));
    }
}

// Puts a built-in style the document lacks (and the built-in styles it names) in the style sheet, as part of the edit being made.
void addBuiltinInStep(WriteDoc &doc, const QString &id, int depth = 0)
{
    if (id.isEmpty() || depth > 8 || doc.styleSheet().find(id)) return;
    const Style b = builtinStyle(id);
    if (b.id.isEmpty() || b.type == QLatin1String("table") || b.type == QLatin1String("numbering")) return;
    addBuiltinInStep(doc, b.basedOn, depth + 1);
    addBuiltinInStep(doc, b.next, depth + 1);
    if (!doc.styleSheet().find(id)) doc.putStyle(b);
}

QString newStyleId(const WriteDoc &doc, const QString &name)
{
    QString base;
    for (const QChar ch : name)
        if (ch.isLetterOrNumber()) base += ch;
    base = base.left(40);
    if (base.isEmpty() || base[0].isDigit()) base.prepend(QStringLiteral("Style"));
    QString id = base;
    for (int n = 2; (doc.styleSheet().find(id) || !builtinStyle(id).id.isEmpty()) && n < kMostStylesMade + 100; ++n) id = base + QString::number(n);
    return id;
}

// What a paragraph says itself, as a paragraph style would say it.
ParaProps paragraphSaid(WriteDoc &doc, const QTextBlock &b, const QString &style)
{
    const QTextBlockFormat bf = b.blockFormat();
    ParaProps full = paraPropsOf(bf);
    full.style.clear();
    const int numId = full.numId.value_or(0), level = full.ilvl.value_or(0);
    ParaProps said = full.differences(doc.baselineParagraph(style, numId, level, bf.boolProperty(prop::NumDirect)));
    said.numId.reset();
    said.ilvl.reset();
    said.section.reset();
    said.markRun.reset();
    said.extra.clear();
    said.style.clear();
    return said;
}

RunProps runSaid(WriteDoc &doc, const QTextCursor &c, const QString &style)
{
    QTextCursor at(c);
    if (c.hasSelection()) at.setPosition(c.selectionStart() + 1);
    RunProps said = doc.writtenRunProps(at.charFormat(), style);
    said.style.clear();
    said.extra.clear();
    return said;
}

} // namespace

// The properties of a paragraph's format that are the paragraph's own (the file's pieces, its place, its list), not what its style gives it.
bool paragraphKeepsOwn(int key)
{
    static const QSet<int> own = {int(QTextFormat::ObjectIndex), int(prop::MarkFormat), int(prop::LastSection), int(prop::RawBefore), int(prop::RawAfter), int(prop::RawBlock), int(prop::ParaAttrs),
                                  int(prop::RawPPr), int(prop::SpacingAttrs), int(prop::IndAttrs), int(prop::NumExtra), int(prop::OddTabs), int(flow::fp::SectionBreakAfter), int(prop::SectionData), int(flow::fp::Cell),
                                  int(flow::fp::Frame), int(tp::OutOfLayout), int(flow::fp::AdvanceTop), int(tp::StyleName), int(prop::NumId), int(prop::NumLevel), int(tp::BulletChar),
                                  int(prop::NumDirect), int(prop::EndnoteBlock), int(prop::EndnoteNote), int(prop::EndnoteStyle), int(prop::StylesRev)};
    return own.contains(key);
}

QVector<ListedStyle> listStyles(WriteDoc &doc, StyleFilter filter, const QString &caretStyle)
{
    const StyleSheet &sheet = doc.styleSheet();
    const QSet<QString> used = doc.stylesInUse();
    QVector<ListedStyle> out;
    QVector<int> priority;
    QSet<QString> added;
    bool firstPass = true;
    auto consider = [&](const Style &s, bool inDocument) {
        if (s.type != QLatin1String("paragraph") && s.type != QLatin1String("character")) return;
        if (added.contains(s.id)) return;
        const bool inUse = used.contains(s.id);
        if (firstPass && !inUse && s.id != caretStyle) return;
        // A style is recommended unless it is hidden, or half hidden until it is used; and it is either in the gallery, has a priority, or is used.
        const bool shownByDefault = s.qFormat || s.uiPriority.value_or(99) < 99 || inUse;
        const bool recommended = !s.hidden && (!s.semiHidden || (s.unhideWhenUsed && inUse)) && shownByDefault;
        if (filter == StyleFilter::InUse && !inUse) return;
        if (filter == StyleFilter::Recommended && !recommended && s.id != caretStyle) return;
        added.insert(s.id);
        ListedStyle e;
        e.id = s.id;
        e.name = shownName(s);
        e.character = s.type == QLatin1String("character");
        e.linked = !e.character && !s.link.isEmpty();
        e.inUse = inUse;
        e.recommended = recommended;
        e.inDocument = inDocument;
        out.push_back(e);
        priority.push_back(s.isDefault && !e.character ? -1 : s.uiPriority.value_or(99));
    };
    // (The styles the text uses and the caret's first, so that the cut at the most listed never takes them; then the rest in the order of the file.)
    for (bool pass : {true, false}) {
        firstPass = pass;
        for (const Style &s : sheet.styles) {
            if (out.size() >= kMostStylesListed) break;
            consider(s, true);
        }
    }
    firstPass = false;
    // The built-in styles the document does not have yet are offered as well (they are added when first used); they are in use only once they are.
    if (filter != StyleFilter::InUse)
        for (const QString &id : builtinStyleIds()) {
            if (out.size() >= kMostStylesListed) break;
            if (sheet.find(id)) continue;
            const Style s = builtinStyle(id);
            if (s.id.isEmpty() || s.type == QLatin1String("table") || s.type == QLatin1String("numbering")) continue;
            if (filter == StyleFilter::Recommended && !s.qFormat) continue;
            consider(s, false);
        }
    QVector<int> order(out.size());
    for (int i = 0; i < order.size(); ++i) order[i] = i;
    if (filter == StyleFilter::Recommended)
        std::stable_sort(order.begin(), order.end(), [&](int a, int b) {
            if (priority[a] != priority[b]) return priority[a] < priority[b];
            return QString::compare(out[a].name, out[b].name, Qt::CaseInsensitive) < 0;
        });
    else
        std::stable_sort(order.begin(), order.end(), [&](int a, int b) { return QString::compare(out[a].name, out[b].name, Qt::CaseInsensitive) < 0; });
    QVector<ListedStyle> sorted;
    sorted.reserve(out.size());
    for (int i : order) sorted.push_back(out[i]);
    return sorted;
}

std::optional<Style> styleOfDocument(const WriteDoc &doc, const QString &id)
{
    if (const Style *s = doc.styleSheet().find(id)) return *s;
    const Style b = builtinStyle(id);
    if (b.id.isEmpty() || (b.type != QLatin1String("paragraph") && b.type != QLatin1String("character"))) return std::nullopt;
    return b;
}

QString styleNameProblem(const WriteDoc &doc, const QString &name, const QString &exceptId)
{
    const QString trimmed = name.trimmed();
    if (trimmed.isEmpty()) return QCoreApplication::translate("jp::write::Styles", "Type a name for the style.");
    if (trimmed.size() > kMostStyleNameLength) return QCoreApplication::translate("jp::write::Styles", "A style's name can be no longer than 255 characters.");
    for (const Style &s : doc.styleSheet().styles)
        if (s.id != exceptId && (QString::compare(s.name, trimmed, Qt::CaseInsensitive) == 0 || (s.name.isEmpty() && QString::compare(s.id, trimmed, Qt::CaseInsensitive) == 0)))
            return QCoreApplication::translate("jp::write::Styles", "There is already a style with that name.");
    for (const QString &id : builtinStyleIds()) {
        const Style b = builtinStyle(id);
        if (id != exceptId && !doc.styleSheet().find(id) && QString::compare(b.name, trimmed, Qt::CaseInsensitive) == 0) return QCoreApplication::translate("jp::write::Styles", "There is already a style with that name.");
    }
    return QString();
}

QString styleBasedOnProblem(const WriteDoc &doc, const QString &id, const QString &basedOn, const QString &type)
{
    if (basedOn.isEmpty()) return QString();
    const StyleSheet &sheet = doc.styleSheet();
    const Style *base = sheet.find(basedOn);
    Style builtin;
    if (!base) {
        builtin = builtinStyle(basedOn);
        if (!builtin.id.isEmpty()) base = &builtin;   // (a built-in style the document lacks is added with the change)
    }
    if (!base) return QCoreApplication::translate("jp::write::Styles", "The style it is based on is not in the document.");
    if (base->type != type) return QCoreApplication::translate("jp::write::Styles", "A style can only be based on a style of its own kind.");
    // Walks up from the style it would be based on; a style that comes back to itself (or to the style being changed) is a loop. The walk ends at the first style it has met.
    QSet<QString> seen;
    for (const Style *at = base; at; at = at->basedOn.isEmpty() ? nullptr : sheet.find(at->basedOn)) {
        if (!id.isEmpty() && at->id == id) return QCoreApplication::translate("jp::write::Styles", "A style can't be based on itself, or on a style that is based on it.");
        if (seen.contains(at->id)) break;
        seen.insert(at->id);
    }
    return QString();
}

Style styleFromSelection(WriteDoc &doc, const QTextCursor &c, const QString &name, bool character)
{
    Style s;
    s.name = name.trimmed();
    s.type = character ? QStringLiteral("character") : QStringLiteral("paragraph");
    s.customStyle = true;
    s.qFormat = true;
    QTextBlock b = c.document()->findBlock(c.selectionStart());
    if (!b.isValid()) b = c.block();
    QString paragraph = styleOfBlock(b.blockFormat());
    if (paragraph.isEmpty()) paragraph = doc.defaultParagraphStyle();
    if (!character) {
        s.basedOn = doc.styleSheet().find(paragraph) ? paragraph : QString();
        s.pPr = paragraphSaid(doc, b, paragraph);
    }
    s.rPr = runSaid(doc, c, paragraph);
    return s;
}

QString createStyle(WriteDoc &doc, QTextCursor &c, const Style &def, QString *problem, bool apply)
{
    auto fail = [&](const QString &why) {
        if (problem) *problem = why;
        return QString();
    };
    if (doc.styleSheet().styles.size() >= kMostStylesMade) return fail(QCoreApplication::translate("jp::write::Styles", "The document has the most styles it can hold."));
    const QString name = def.name.trimmed();
    if (const QString why = styleNameProblem(doc, name); !why.isEmpty()) return fail(why);
    Style s = def;
    s.type = def.type == QLatin1String("character") ? QStringLiteral("character") : QStringLiteral("paragraph");
    if (!s.basedOn.isEmpty())
        if (const QString why = styleBasedOnProblem(doc, QString(), s.basedOn, s.type); !why.isEmpty()) return fail(why);
    if (s.type == QLatin1String("character") || (!s.next.isEmpty() && !styleOfDocument(doc, s.next))) s.next.clear();
    s.id = newStyleId(doc, name);
    s.name = name;
    s.customStyle = true;
    s.isDefault = false;
    s.link.clear();
    s.raw.clear();
    QTextCursor edit(doc.story());
    edit.beginEditBlock();
    addBuiltinInStep(doc, s.basedOn);
    addBuiltinInStep(doc, s.next);
    doc.putStyle(s);
    touchStyles(doc);
    if (apply) {
        if (s.type == QLatin1String("character")) applyCharacterStyle(doc, c, s.id);
        else applyParagraphStyle(doc, c, s.id);
    }
    edit.endEditBlock();
    if (!doc.settleStyleEdit()) return fail(QCoreApplication::translate("jp::write::Styles", "The document keeps no history, so the style was not made."));
    return s.id;
}

bool modifyStyle(WriteDoc &doc, QTextCursor &c, const QString &id, const Style &def, QString *problem)
{
    Q_UNUSED(c);
    auto fail = [&](const QString &why) {
        if (problem) *problem = why;
        return false;
    };
    const std::optional<Style> found = styleOfDocument(doc, id);
    if (!found) return fail(QCoreApplication::translate("jp::write::Styles", "The document has no such style."));
    const Style old = *found;
    Style s = def;
    s.id = old.id;
    s.type = old.type;
    s.isDefault = old.isDefault;
    s.customStyle = old.customStyle;
    s.link = old.link;
    s.raw.clear();
    // A built-in style keeps its name; a style of the person's can be renamed, to a name no other style has.
    const QString name = def.name.trimmed();
    if (!old.customStyle || name == old.name || name.isEmpty()) {
        s.name = old.name;
        if (old.customStyle && name.isEmpty() && !def.name.isEmpty()) return fail(QCoreApplication::translate("jp::write::Styles", "Type a name for the style."));
    } else {
        if (const QString why = styleNameProblem(doc, name, old.id); !why.isEmpty()) return fail(why);
        s.name = name;
    }
    if (s.basedOn != old.basedOn && !s.basedOn.isEmpty()) {
        if (const QString why = styleBasedOnProblem(doc, old.id, s.basedOn, old.type); !why.isEmpty()) return fail(why);
    }
    if (s.type == QLatin1String("character")) s.next.clear();
    else if (s.next != old.next && !s.next.isEmpty() && !styleOfDocument(doc, s.next)) s.next = old.next;
    // A definition that comes to the style as it is changes nothing (the file is written as it was).
    if (bytesOf(old) == bytesOf(s)) return true;
    StyleSheet before = doc.styleSheet();
    const QSet<QString> paragraphs = old.type == QLatin1String("paragraph") ? descendantsOf(before, old.id, old.type) : QSet<QString>();
    const QSet<QString> characters = old.type == QLatin1String("character") ? descendantsOf(before, old.id, old.type) : QSet<QString>();
    QTextCursor edit(doc.story());
    edit.beginEditBlock();
    addBuiltinInStep(doc, s.basedOn);
    addBuiltinInStep(doc, s.next);
    doc.putStyle(s);
    restyleText(doc, before, paragraphs, characters);
    touchStyles(doc);
    edit.endEditBlock();
    if (!doc.settleStyleEdit()) return fail(QCoreApplication::translate("jp::write::Styles", "The document keeps no history, so the style was not changed."));
    return true;
}

bool updateStyleToSelection(WriteDoc &doc, QTextCursor &c, QString *problem)
{
    QTextBlock b = c.document()->findBlock(c.selectionStart());
    if (!b.isValid() || c.document() != doc.story() || isLocked(b)) {
        if (problem) *problem = QCoreApplication::translate("jp::write::Styles", "There is no style to update here.");
        return false;
    }
    QString id = styleOfBlock(b.blockFormat());
    if (id.isEmpty()) id = doc.defaultParagraphStyle();
    const std::optional<Style> has = styleOfDocument(doc, id);
    if (!has) {
        if (problem) *problem = QCoreApplication::translate("jp::write::Styles", "The document has no such style.");
        return false;
    }
    Style s = *has;
    const ParaProps said = paragraphSaid(doc, b, id);
    s.pPr.overlay(said);
    s.rPr.overlay(runSaid(doc, c, id));
    return modifyStyle(doc, c, id, s, problem);
}

// ---- Format Painter

namespace {

// Whether a selection takes in the paragraph mark of the first paragraph it touches.
bool takesInMark(const QTextCursor &c)
{
    if (!c.hasSelection()) return false;
    const QTextBlock first = c.document()->findBlock(c.selectionStart());
    return c.selectionEnd() > first.position() + first.length() - 1;
}

} // namespace

FormatBrush copyFormat(WriteDoc &doc, const QTextCursor &c)
{
    FormatBrush brush;
    if (c.document() != doc.story()) return brush;
    QTextCursor at(c);
    if (c.hasSelection()) at.setPosition(c.selectionStart() + 1);
    brush.chars = withoutAtom(at.charFormat());
    brush.valid = true;
    if (!c.hasSelection() || takesInMark(c)) {
        const QTextBlock first = c.document()->findBlock(c.selectionStart());
        if (first.isValid() && !isLocked(first)) {
            brush.hasParagraph = true;
            brush.paragraph = first.blockFormat();
        }
    }
    return brush;
}

bool pasteFormat(WriteDoc &doc, QTextCursor &c, const FormatBrush &brush)
{
    if (!brush.valid) return false;
    const NoteScopeGuard scopeGuard(doc, c);
    const AutoRedefineOff noAutoUpdate;   // (a paint is not a person's formatting of one paragraph: the styles do not follow it)
    if (!intoBody(c)) return false;
    const bool paragraphs = brush.hasParagraph && (!c.hasSelection() || takesInMark(c));
    QTextCursor range(c);
    if (!range.hasSelection()) range.select(QTextCursor::WordUnderCursor);
    c.beginEditBlock();
    bool changed = false;
    if (range.hasSelection())
        changeCharFormat(doc, range, [&](QTextCharFormat &f) {
            if (isAtom(f)) return;
            QTextCharFormat nf = restyleCharFormat(f, f, brush.chars);
            if (brush.chars.hasProperty(tp::CharStyle)) nf.setProperty(tp::CharStyle, brush.chars.property(tp::CharStyle));
            else nf.clearProperty(tp::CharStyle);
            changed = changed || nf != f;
            f = nf;
        });
    if (paragraphs)
        for (const QTextBlock &b : selectedBlocks(c)) {
            if (isLocked(b)) continue;
            const QTextBlockFormat bf = b.blockFormat();
            QTextBlockFormat nf = brush.paragraph;
            const QMap<int, QVariant> mine = bf.properties(), theirs = brush.paragraph.properties();
            for (auto it = theirs.cbegin(); it != theirs.cend(); ++it)
                if (paragraphKeepsOwn(it.key())) nf.clearProperty(it.key());
            for (auto it = mine.cbegin(); it != mine.cend(); ++it)
                if (paragraphKeepsOwn(it.key())) nf.setProperty(it.key(), it.value());
            // (The style is the brush's, and a paragraph in a list keeps the list's indents.)
            if (brush.paragraph.hasProperty(tp::StyleName)) nf.setProperty(tp::StyleName, brush.paragraph.property(tp::StyleName));
            else nf.clearProperty(tp::StyleName);
            if (bf.hasProperty(prop::NumId)) {
                nf.setLeftMargin(bf.leftMargin());
                nf.setTextIndent(bf.textIndent());
            }
            nf.setObjectIndex(bf.objectIndex());
            if (nf != bf) {
                QTextCursor(b).setBlockFormat(nf);
                changed = true;
            }
        }
    c.endEditBlock();
    return changed;
}

} // namespace jp::write
