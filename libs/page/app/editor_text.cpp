// Text formatting commands. When editing text they apply to the selection
// (or the word at the cursor); when text boxes are selected but not being
// edited they apply to all of their text.

#include "app/editor.h"
#include "app/autocorrectconfig.h"
#include "app/settings.h"
#include "app/shapetextedit.h"

#include "core/texteffects.h"
#include "text/autocorrect.h"
#include "text/textprops.h"

#include <QRegularExpression>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextList>

namespace jp {

QVector<QTextCursor> Editor::formatTargets() const
{
    QVector<QTextCursor> out;
    if (isEditingText()) {
        out << m_cursor;
        return out;
    }
    QSet<QString> seen;
    auto addStory = [&](const QString &sid) {
        if (sid.isEmpty() || seen.contains(sid)) return;
        seen.insert(sid);
        if (QTextDocument *d = m_doc->storyDoc(sid)) {
            QTextCursor c(d);
            c.select(QTextCursor::Document);
            out << c;
        }
    };
    std::function<void(Item *)> visit = [&](Item *it) {
        switch (it->type()) {
        case ItemType::Text: addStory(static_cast<TextItem *>(it)->storyId); break;
        case ItemType::Shape: addStory(static_cast<ShapeItem *>(it)->storyId); break;
        case ItemType::Table: {
            // Selected cells: just their text; otherwise all of the table's.
            auto *t = static_cast<TableItem *>(it);
            const CellBlock b = cellBlock();
            for (int r = 0; r < t->rows; ++r)
                for (int c = 0; c < t->cols; ++c)
                    if (b.itemId != t->id || b.range.contains(r, c)) {
                        addStory(t->cell(r, c).storyId);
                        addStory(t->cell(r, c).storyId2);
                    }
            break;
        }
        case ItemType::Group: for (const auto &c : static_cast<GroupItem *>(it)->children) visit(c.get()); break;
        default: break;
        }
    };
    for (Item *it : selectedItems()) visit(it);
    return out;
}

QTextCharFormat Editor::currentCharFormat() const
{
    if (isEditingText()) {
        QTextCursor c = m_cursor;
        if (c.hasSelection() && c.position() > c.anchor()) {
            // Format of the first selected character.
            QTextCursor f(c.document());
            f.setPosition(c.selectionStart() + 1);
            return f.charFormat();
        }
        return c.charFormat();
    }
    const auto t = formatTargets();
    if (t.isEmpty()) return QTextCharFormat();
    QTextCursor f(t.first().document());
    f.setPosition(std::min(1, t.first().document()->characterCount() - 1));
    return f.charFormat();
}

QTextBlockFormat Editor::currentBlockFormat() const
{
    if (isEditingText()) return m_cursor.blockFormat();
    const auto t = formatTargets();
    return t.isEmpty() ? QTextBlockFormat() : t.first().document()->begin().blockFormat();
}

QString Editor::currentStyleName() const { return currentBlockFormat().stringProperty(tp::StyleName); }

static void selectWordIfCollapsed(QTextCursor &c)
{
    if (c.hasSelection()) return;
    QTextCursor w = c;
    w.select(QTextCursor::WordUnderCursor);
    if (w.hasSelection() && !w.selectedText().trimmed().isEmpty()) c = w;
}

// Text formatted by hand tells which properties were: a change to its style does not reach those. These mark them in a range of runs (and the paragraphs' own
// character formats), or in a range of paragraphs.
static void markDirectChars(QTextCursor c, const QTextCharFormat &what)
{
    QTextDocument *d = c.document();
    const int s = c.selectionStart(), e = c.selectionEnd();
    for (QTextBlock b = d->findBlock(s); b.isValid() && b.position() <= e; b = b.next()) {
        struct Run {
            int at, len;
            QTextCharFormat f;
        };
        QVector<Run> runs;
        for (auto it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment fr = it.fragment();
            const int fs = std::max(s, fr.position()), fe = std::min(e, fr.position() + fr.length());
            if (fs >= fe) continue;
            QTextCharFormat cf = fr.charFormat();
            if (tp::addDirect(cf, what)) runs.append({fs, fe - fs, cf});
        }
        for (const Run &r : runs) {
            QTextCursor fc(d);
            fc.setPosition(r.at);
            fc.setPosition(r.at + r.len, QTextCursor::KeepAnchor);
            fc.setCharFormat(r.f);
        }
        QTextCharFormat bcf = b.charFormat();
        QTextCharFormat mine;   // (the properties the paragraph's own character format took)
        const QMap<int, QVariant> props = what.properties();
        for (auto it = props.cbegin(); it != props.cend(); ++it)
            if (bcf.hasProperty(it.key())) mine.setProperty(it.key(), it.value());
        if (tp::addDirect(bcf, mine)) QTextCursor(b).setBlockCharFormat(bcf);
    }
}

static void markDirectBlocks(QTextCursor c, const QTextBlockFormat &what)
{
    QTextDocument *d = c.document();
    const int s = c.selectionStart(), e = c.selectionEnd();
    for (QTextBlock b = d->findBlock(s); b.isValid() && b.position() <= e; b = b.next()) {
        QTextBlockFormat bf = b.blockFormat();
        if (tp::addDirect(bf, what)) QTextCursor(b).setBlockFormat(bf);
    }
}

// A paragraph's format set by hand: what it changes is its own.
static void setBlockDirect(QTextBlock b, QTextBlockFormat bf)
{
    tp::addDirectDiff(bf, b.blockFormat());
    QTextCursor(b).setBlockFormat(bf);
}

void Editor::mergeCharFormat(const QTextCharFormat &f, const QString &label)
{
    auto targets = formatTargets();
    if (targets.isEmpty()) return;
    beginChange(label);
    if (isEditingText()) {
        QTextCursor c = m_cursor;
        selectWordIfCollapsed(c);
        if (c.hasSelection()) c.mergeCharFormat(f);
        m_cursor.mergeCharFormat(f);   // also affects text typed next
        if (!c.hasSelection() || c.selectionStart() == c.block().position()) c.mergeBlockCharFormat(f);
        if (c.hasSelection()) markDirectChars(c, f);
        if (!m_cursor.hasSelection()) {
            QTextCharFormat typed = m_cursor.charFormat();
            if (tp::addDirect(typed, f)) m_cursor.setCharFormat(typed);
        }
    } else {
        for (QTextCursor &c : targets) {
            c.mergeCharFormat(f);
            c.mergeBlockCharFormat(f);
            markDirectChars(c, f);
        }
    }
    endChange();
    textCursorChanged();
}

void Editor::mergeBlockFormat(const QTextBlockFormat &f, const QString &label)
{
    auto targets = formatTargets();
    if (targets.isEmpty()) return;
    beginChange(label);
    for (QTextCursor &c : targets) {
        c.mergeBlockFormat(f);
        markDirectBlocks(c, f);
    }
    if (isEditingText()) {
        m_cursor.mergeBlockFormat(f);
        markDirectBlocks(m_cursor, f);
    }
    endChange();
    Q_EMIT textCursorChanged();
}

void Editor::setCharProperty(int prop, const QVariant &v, const QString &label)
{
    QTextCharFormat f;
    f.setProperty(prop, v);
    mergeCharFormat(f, label);
}

void Editor::clearCharProperty(int prop, const QString &label)
{
    auto targets = formatTargets();
    if (targets.isEmpty()) return;
    beginChange(label);
    for (QTextCursor c : targets) {
        if (isEditingText()) selectWordIfCollapsed(c);
        // Walk fragments in the selection and remove the property.
        const int s = c.selectionStart(), e = c.selectionEnd();
        QTextDocument *d = c.document();
        for (QTextBlock b = d->findBlock(s); b.isValid() && b.position() <= e; b = b.next()) {
            for (auto it = b.begin(); !it.atEnd(); ++it) {
                const QTextFragment fr = it.fragment();
                const int fs = std::max(s, fr.position()), fe = std::min(e, fr.position() + fr.length());
                if (fs >= fe) continue;
                QTextCursor fc(d);
                fc.setPosition(fs);
                fc.setPosition(fe, QTextCursor::KeepAnchor);
                QTextCharFormat cf = fr.charFormat();
                cf.clearProperty(prop);
                tp::removeDirect(cf, {prop});
                fc.setCharFormat(cf);
            }
            QTextCursor bc(b);
            QTextCharFormat bcf = b.charFormat();
            bcf.clearProperty(prop);
            tp::removeDirect(bcf, {prop});
            bc.setBlockCharFormat(bcf);
        }
    }
    if (isEditingText()) {
        QTextCharFormat cf = m_cursor.charFormat();
        cf.clearProperty(prop);
        m_cursor.setCharFormat(cf);
    }
    endChange();
    Q_EMIT textCursorChanged();
}

void Editor::editCharFormats(const std::function<void(QTextCharFormat &)> &change, const QString &label)
{
    auto targets = formatTargets();
    if (targets.isEmpty()) return;
    beginChange(label);
    for (QTextCursor c : targets) {
        if (isEditingText()) selectWordIfCollapsed(c);
        const int s = c.selectionStart(), e = c.selectionEnd();
        QTextDocument *d = c.document();
        for (QTextBlock b = d->findBlock(s); b.isValid() && b.position() <= e; b = b.next()) {
            for (auto it = b.begin(); !it.atEnd(); ++it) {
                const QTextFragment fr = it.fragment();
                const int fs = std::max(s, fr.position()), fe = std::min(e, fr.position() + fr.length());
                if (fs >= fe) continue;
                QTextCursor fc(d);
                fc.setPosition(fs);
                fc.setPosition(fe, QTextCursor::KeepAnchor);
                QTextCharFormat cf = fr.charFormat();
                const QTextCharFormat before = cf;
                change(cf);
                tp::addDirectDiff(cf, before);
                fc.setCharFormat(cf);
            }
            QTextCursor bc(b);
            QTextCharFormat bcf = b.charFormat();
            const QTextCharFormat bcBefore = bcf;
            change(bcf);
            tp::addDirectDiff(bcf, bcBefore);
            bc.setBlockCharFormat(bcf);
        }
    }
    // (What is typed next, with no selection: a selection's runs are done, each by itself, and a cursor's format would set them all alike.)
    if (isEditingText() && !m_cursor.hasSelection()) {
        QTextCharFormat cf = m_cursor.charFormat();
        const QTextCharFormat before = cf;
        change(cf);
        tp::addDirectDiff(cf, before);
        m_cursor.setCharFormat(cf);
    }
    endChange();
    Q_EMIT textCursorChanged();
}

bool Editor::hasTextShadow(const QTextCharFormat &cf)
{
    const QString list = cf.stringProperty(tp::TextFx);
    return list.isEmpty() ? cf.boolProperty(tp::Shadow) : TextEffects::fromJson(list).outer.on;
}

void Editor::setTextShadow(bool on, const QString &label)
{
    editCharFormats([on](QTextCharFormat &cf) {
        const QString list = cf.stringProperty(tp::TextFx);
        TextEffects fx = list.isEmpty() ? TextEffects() : TextEffects::fromJson(list);
        if (on && !fx.outer.on) {
            fx.outer = defaultTextShadow().outer;
            fx.xml.clear();
            cf.setProperty(tp::TextFx, fx.toJson());
        } else if (!on && fx.outer.on) {
            // Off, in a list that says so (if nothing else is left in it): the shadow a style gives the run stays off too.
            fx.outer.on = false;
            fx.xml.clear();
            cf.setProperty(tp::TextFx, fx.any() ? fx.toJson() : TextEffects::noneJson());
        }
        cf.clearProperty(tp::Shadow);
    }, label);
}

void Editor::toggleBold()
{
    QTextCharFormat f;
    f.setFontWeight(currentCharFormat().fontWeight() >= QFont::DemiBold ? QFont::Normal : QFont::Bold);
    mergeCharFormat(f, tr("Bold"));
}

void Editor::toggleItalic()
{
    QTextCharFormat f;
    f.setFontItalic(!currentCharFormat().fontItalic());
    mergeCharFormat(f, tr("Italic"));
}

void Editor::toggleUnderline(QTextCharFormat::UnderlineStyle style, int kind)
{
    const QTextCharFormat cur = currentCharFormat();
    QTextCharFormat f;
    const bool on = cur.underlineStyle() != QTextCharFormat::NoUnderline && cur.underlineStyle() == style && cur.intProperty(tp::UnderlineKind) == kind;
    f.setUnderlineStyle(on ? QTextCharFormat::NoUnderline : style);
    f.setProperty(tp::UnderlineKind, on ? 0 : kind);
    mergeCharFormat(f, tr("Underline"));
}

void Editor::toggleStrike()
{
    QTextCharFormat f;
    f.setFontStrikeOut(!currentCharFormat().fontStrikeOut());
    mergeCharFormat(f, tr("Strikethrough"));
}

void Editor::toggleScript(bool super)
{
    const auto want = super ? QTextCharFormat::AlignSuperScript : QTextCharFormat::AlignSubScript;
    QTextCharFormat f;
    f.setVerticalAlignment(currentCharFormat().verticalAlignment() == want ? QTextCharFormat::AlignNormal : want);
    mergeCharFormat(f, super ? tr("Superscript") : tr("Subscript"));
}

void Editor::setFontFamily(const QString &family)
{
    QTextCharFormat f;
    if (family.startsWith(QLatin1String("+"))) {
        // "+Heading" / "+Body": follow the font scheme.
        f.setProperty(tp::ThemeFont, family == QLatin1String("+Heading") ? QStringLiteral("major") : QStringLiteral("minor"));
        clearCharProperty(QTextFormat::FontFamilies, tr("Font"));
        mergeCharFormat(f, tr("Font"));
        return;
    }
    f.setFontFamilies(QStringList{family});
    mergeCharFormat(f, tr("Font"));
}

void Editor::setFontSize(double pt)
{
    if (pt <= 0) return;
    QTextCharFormat f;
    f.setFontPointSize(std::clamp(pt, 0.5, 999.0));
    mergeCharFormat(f, tr("Font Size"));
}

void Editor::growFont(int dir)
{
    auto targets = formatTargets();
    if (targets.isEmpty()) return;
    beginChange(dir > 0 ? tr("Grow Font") : tr("Shrink Font"));
    for (QTextCursor c : targets) {
        if (isEditingText()) selectWordIfCollapsed(c);
        const int s = c.selectionStart(), e = c.selectionEnd();
        QTextDocument *d = c.document();
        for (QTextBlock b = d->findBlock(s); b.isValid() && b.position() <= e; b = b.next())
            for (auto it = b.begin(); !it.atEnd(); ++it) {
                const QTextFragment fr = it.fragment();
                const int fs = std::max(s, fr.position()), fe = std::min(e, fr.position() + fr.length());
                if (fs >= fe) continue;
                const double cur = fr.charFormat().hasProperty(QTextFormat::FontPointSize) ? fr.charFormat().fontPointSize() : 11;
                const double next = nextFontSize(cur, dir);
                QTextCursor fc(d);
                fc.setPosition(fs);
                fc.setPosition(fe, QTextCursor::KeepAnchor);
                QTextCharFormat f;
                f.setFontPointSize(next);
                fc.mergeCharFormat(f);
                markDirectChars(fc, f);
            }
    }
    endChange();
    Q_EMIT textCursorChanged();
}

void Editor::clearFormatting()
{
    auto targets = formatTargets();
    if (targets.isEmpty()) return;
    beginChange(tr("Clear All Formatting"));
    for (QTextCursor c : targets) {
        if (isEditingText() && !c.hasSelection()) c.select(QTextCursor::BlockUnderCursor);
        const int s = c.selectionStart(), e = c.selectionEnd();
        QTextDocument *d = c.document();
        for (QTextBlock b = d->findBlock(s); b.isValid() && b.position() <= e; b = b.next()) {
            const QString st = b.blockFormat().stringProperty(tp::StyleName);
            const TextStyle ts = m_doc->resolvedStyle(st.isEmpty() ? QStringLiteral("Normal") : st);   // (with what its bases set)
            QTextCharFormat base = ts.chr;
            QTextCursor bc(d);
            bc.setPosition(std::max(s, b.position()));
            bc.setPosition(std::min(e, b.position() + b.length() - 1), QTextCursor::KeepAnchor);
            QTextCharFormat keepFields;
            // Keep fields working.
            for (auto it = b.begin(); !it.atEnd(); ++it) {
                const QTextFragment fr = it.fragment();
                const int fs = std::max(bc.selectionStart(), fr.position()), fe = std::min(bc.selectionEnd(), fr.position() + fr.length());
                if (fs >= fe) continue;
                QTextCursor fc(d);
                fc.setPosition(fs);
                fc.setPosition(fe, QTextCursor::KeepAnchor);
                QTextCharFormat nf = base;
                const QString field = fr.charFormat().stringProperty(tp::Field);
                if (!field.isEmpty()) nf.setProperty(tp::Field, field);
                fc.setCharFormat(nf);
            }
        }
    }
    endChange();
    Q_EMIT textCursorChanged();
}

void Editor::setTextColor(const ColorRef &c)
{
    if (c.isNone()) { clearCharProperty(tp::ColorRefP, tr("Font Color")); return; }
    QTextCharFormat f;
    f.setProperty(tp::ColorRefP, c.toString());
    f.setProperty(tp::TextFill, QString());   // a solid color replaces a gradient fill
    f.clearForeground();
    mergeCharFormat(f, tr("Font Color"));
}

void Editor::changeCase(int mode)
{
    auto targets = formatTargets();
    if (targets.isEmpty()) return;
    beginChange(tr("Change Case"));
    for (QTextCursor c : targets) {
        if (isEditingText()) selectWordIfCollapsed(c);
        const int s = c.selectionStart(), e = c.selectionEnd();
        QTextDocument *d = c.document();
        bool newSentence = true;
        // Replace character by character to keep formatting.
        for (int pos = s; pos < e; ++pos) {
            const QChar ch = d->characterAt(pos);
            if (ch == QChar::ObjectReplacementCharacter) continue;
            QChar out = ch;
            switch (mode) {
            case 0: out = newSentence ? ch.toUpper() : ch.toLower(); break;
            case 1: out = ch.toLower(); break;
            case 2: out = ch.toUpper(); break;
            case 3: {
                const QChar prev = pos > 0 ? d->characterAt(pos - 1) : QChar(' ');
                out = (prev.isSpace() || prev == QChar::ParagraphSeparator || pos == s && !prev.isLetter()) ? ch.toUpper() : ch.toLower();
                break;
            }
            case 4: out = ch.isUpper() ? ch.toLower() : ch.toUpper(); break;
            }
            if (ch.isLetter()) newSentence = false;
            if (ch == '.' || ch == '!' || ch == '?' || ch == QChar::ParagraphSeparator) newSentence = true;
            if (out != ch) {
                QTextCursor fc(d);
                fc.setPosition(pos);
                fc.setPosition(pos + 1, QTextCursor::KeepAnchor);
                const QTextCharFormat cf = fc.charFormat();
                fc.insertText(QString(out), cf);
            }
        }
    }
    endChange();
    Q_EMIT textCursorChanged();
}

void Editor::setAlignment(Qt::Alignment a)
{
    QTextBlockFormat f;
    f.setAlignment(a);
    f.setProperty(tp::Distribute, false);
    mergeBlockFormat(f, tr("Alignment"));
}

void Editor::setDirection(Qt::LayoutDirection d)
{
    QTextBlockFormat f;
    f.setLayoutDirection(d);
    const Qt::Alignment al = currentBlockFormat().alignment() & Qt::AlignHorizontal_Mask;
    if (d == Qt::RightToLeft && (al == 0 || al == Qt::AlignLeft || al == Qt::AlignLeading)) f.setAlignment(Qt::AlignRight);
    if (d == Qt::LeftToRight && (al == Qt::AlignRight || al == Qt::AlignTrailing)) f.setAlignment(Qt::AlignLeft);
    mergeBlockFormat(f, tr("Text Direction"));
}

void Editor::setLineSpacing(int type, double value)
{
    QTextBlockFormat f;
    f.setLineHeight(value, type);
    mergeBlockFormat(f, tr("Line Spacing"));
}

void Editor::setParagraphSpacing(double before, double after)
{
    QTextBlockFormat f;
    if (before >= 0) f.setTopMargin(before);
    if (after >= 0) f.setBottomMargin(after);
    mergeBlockFormat(f, tr("Paragraph Spacing"));
}

namespace {

// Whether two list formats look the same at a level (the bullet or the numbering, not where it starts).
bool sameListLook(const QTextListFormat &a, const QTextListFormat &b)
{
    return a.style() == b.style() && a.numberPrefix() == b.numberPrefix() && a.numberSuffix() == b.numberSuffix() && a.stringProperty(tp::BulletChar) == b.stringProperty(tp::BulletChar) &&
           a.stringProperty(tp::BulletFont) == b.stringProperty(tp::BulletFont) && a.intProperty(tp::ListLevel) == b.intProperty(tp::ListLevel);
}

// Puts the paragraph in the list of this look at this level: the list just above at that level when it looks the same (so the numbers go on), else a new one.
void putInList(QTextBlock b, QTextListFormat lf, int level)
{
    lf.setIndent(0);
    lf.setProperty(tp::ListLevel, level);
    QTextList *join = nullptr;
    for (QTextBlock p = b.previous(); p.isValid(); p = p.previous()) {
        QTextList *pl = p.textList();
        if (!pl) break;
        const int pv = pl->format().intProperty(tp::ListLevel);
        if (pv == level && sameListLook(pl->format(), lf)) {
            join = pl;
            break;
        }
        if (pv < level || pv == level) break;   // a shallower item, or another look at this level: a new list
    }
    if (QTextList *old = b.textList()) old->remove(b);
    if (join) join->add(b);
    else QTextCursor(b).createList(lf);
}

} // namespace

void Editor::setListFormat(const QTextListFormat &look, double indent)
{
    setMultilevelList({look}, indent);
}

void Editor::setMultilevelList(const QVector<QTextListFormat> &levels, double indent)
{
    if (levels.isEmpty()) return;
    auto targets = formatTargets();
    if (targets.isEmpty()) return;
    beginChange(levels.size() > 1 ? tr("Multilevel List") : levels.first().style() == QTextListFormat::ListDisc || levels.first().hasProperty(tp::BulletChar) ? tr("Bullets") : tr("Numbering"));
    for (QTextCursor c : targets) {
        QTextDocument *d = c.document();
        QTextBlock first = d->findBlock(c.selectionStart()), last = d->findBlock(c.selectionEnd());
        for (QTextBlock b = first; b.isValid(); b = b.next()) {
            const QTextList *was = b.textList();
            const int level = was ? std::clamp(was->format().intProperty(tp::ListLevel), 0, 8) : 0;
            if (!was) {
                QTextBlockFormat bf = b.blockFormat();
                bf.setLeftMargin(indent >= 0 ? indent : std::max(bf.leftMargin(), 18.0));
                bf.setTextIndent(indent >= 0 ? -indent : -18);
                bf.setIndent(0);
                setBlockDirect(b, bf);
            }
            putInList(b, levels.at(std::min(level, int(levels.size()) - 1)), level);
            if (b == last) break;
        }
    }
    endChange();
    Q_EMIT textCursorChanged();
}

void Editor::setListLevel(int level)
{
    auto targets = formatTargets();
    if (targets.isEmpty()) return;
    beginChange(tr("Change List Level"));
    for (QTextCursor c : targets) {
        QTextDocument *d = c.document();
        QTextBlock first = d->findBlock(c.selectionStart()), last = d->findBlock(c.selectionEnd());
        for (QTextBlock b = first; b.isValid(); b = b.next()) {
            if (QTextList *l = b.textList()) {
                const int was = l->format().intProperty(tp::ListLevel), to = std::clamp(level, 0, 8);
                if (was != to) {
                    QTextBlockFormat bf = b.blockFormat();
                    bf.setLeftMargin(std::max(0.0, bf.leftMargin() + (to - was) * 18.0));
                    QTextListFormat lf = l->format();
                    lf.setProperty(tp::ListLevel, to);
                    lf.setStart(1);
                    setBlockDirect(b, bf);
                    putInList(b, lf, to);
                }
            }
            if (b == last) break;
        }
    }
    endChange();
    Q_EMIT textCursorChanged();
}

void Editor::changeIndent(int dir)
{
    auto targets = formatTargets();
    if (targets.isEmpty()) return;
    beginChange(dir > 0 ? tr("Increase Indent") : tr("Decrease Indent"));
    for (QTextCursor c : targets) {
        QTextDocument *d = c.document();
        for (QTextBlock b = d->findBlock(c.selectionStart()); b.isValid() && b.position() <= c.selectionEnd(); b = b.next()) {
            QTextBlockFormat bf = b.blockFormat();
            bf.setLeftMargin(std::max(0.0, bf.leftMargin() + dir * 18.0));
            setBlockDirect(b, bf);
            // A list item moves a level in or out: into the list of that level
            // just above it (so its numbers go on), or a new one, numbered
            // 1. a. i. or bulleted • ◦ ▪ by level.
            QTextList *l = b.textList();
            if (!l) continue;
            const QTextListFormat lf = l->format();
            const int level = lf.intProperty(tp::ListLevel), to = std::clamp(level + dir, 0, 8);
            if (to == level) continue;
            QTextList *join = nullptr;
            for (QTextBlock p = b.previous(); p.isValid(); p = p.previous()) {
                QTextList *pl = p.textList();
                if (!pl) break;
                const int pv = pl->format().intProperty(tp::ListLevel);
                if (pv == to) { join = pl; break; }
                if (pv < to) break;   // a shallower item: this one starts a new list under it
            }
            l->remove(b);
            if (join) {
                join->add(b);
                continue;
            }
            QTextListFormat nf = lf;
            nf.setProperty(tp::ListLevel, to);
            nf.setStart(1);
            const bool bullets = lf.style() == QTextListFormat::ListDisc || lf.style() == QTextListFormat::ListCircle || lf.style() == QTextListFormat::ListSquare;
            if (bullets) {
                static const QTextListFormat::Style marks[] = {QTextListFormat::ListDisc, QTextListFormat::ListCircle, QTextListFormat::ListSquare};
                nf.setStyle(marks[to % 3]);
                nf.clearProperty(tp::BulletChar);   // the level's own mark
            } else {
                static const QTextListFormat::Style numbers[] = {QTextListFormat::ListDecimal, QTextListFormat::ListLowerAlpha, QTextListFormat::ListLowerRoman};
                static const int formats[] = {1, 2, 4};   // tp::NumberFormat: "1." "a." "i."
                nf.setStyle(numbers[to % 3]);
                nf.setProperty(tp::NumberFormat, formats[to % 3]);
                nf.setNumberPrefix(QString());
                nf.setNumberSuffix(QStringLiteral("."));
            }
            QTextCursor(b).createList(nf);
        }
    }
    endChange();
    Q_EMIT textCursorChanged();
}

void Editor::setList(int kind, int format, const QString &bullet, int start, double indent)
{
    auto targets = formatTargets();
    if (targets.isEmpty()) return;
    beginChange(kind == 0 ? tr("Remove List") : kind == 1 ? tr("Bullets") : tr("Numbering"));
    for (QTextCursor c : targets) {
        QTextDocument *d = c.document();
        QTextBlock first = d->findBlock(c.selectionStart()), last = d->findBlock(c.selectionEnd());
        if (kind == 0) {
            for (QTextBlock b = first; b.isValid(); b = b.next()) {
                if (QTextList *l = b.textList()) l->remove(b);
                QTextBlockFormat bf = b.blockFormat();
                bf.setIndent(0);
                bf.setTextIndent(0);
                bf.setLeftMargin(std::max(0.0, bf.leftMargin() - 18));
                setBlockDirect(b, bf);
                if (b == last) break;
            }
            continue;
        }
        QTextListFormat lf;
        if (kind == 1) {
            lf.setStyle(QTextListFormat::ListDisc);
            if (!bullet.isEmpty()) lf.setProperty(tp::BulletChar, bullet);
        } else {
            static const QTextListFormat::Style styles[] = {QTextListFormat::ListDecimal, QTextListFormat::ListDecimal, QTextListFormat::ListLowerAlpha,
                                                           QTextListFormat::ListUpperAlpha, QTextListFormat::ListLowerRoman, QTextListFormat::ListUpperRoman,
                                                           QTextListFormat::ListDecimal, QTextListFormat::ListDecimal};
            lf.setStyle(styles[std::clamp(format, 0, 7)]);
            lf.setNumberSuffix(format == 6 || format == 7 ? QStringLiteral(")") : QStringLiteral("."));
            if (format == 7) lf.setNumberPrefix(QStringLiteral("("));
            lf.setStart(std::max(1, start));
            lf.setProperty(tp::NumberFormat, format);
        }
        lf.setIndent(0);
        QTextList *list = nullptr;
        for (QTextBlock b = first; b.isValid(); b = b.next()) {
            if (QTextList *old = b.textList()) old->remove(b);
            QTextBlockFormat bf = b.blockFormat();
            bf.setLeftMargin(indent >= 0 ? indent : std::max(bf.leftMargin(), 18.0));
            bf.setTextIndent(indent >= 0 ? -indent : -18);
            bf.setIndent(0);
            QTextCursor bc(b);
            bc.setBlockFormat(bf);
            if (!list) list = bc.createList(lf);
            else list->add(b);
            if (b == last) break;
        }
    }
    endChange();
    Q_EMIT textCursorChanged();
}

void Editor::applyStyle(const QString &name)
{
    const TextStyle resolved = m_doc->resolvedStyle(name);   // with what its base style sets
    const TextStyle *s = resolved.name.isEmpty() ? nullptr : &resolved;
    if (!s) return;
    auto targets = formatTargets();
    if (targets.isEmpty()) return;
    beginChange(tr("Apply Style"));
    // A style applied takes the properties it gives out of what was set by hand.
    QSet<int> gives = tp::keysOf(s->chr);
    gives.insert(QTextFormat::FontFamilies);   // (a style with no font of its own puts the scheme font back)
    auto takeOver = [&](QTextCharFormat &f) { tp::removeDirect(f, gives); };
    for (QTextCursor c : targets) {
        if (s->charOnly) {
            if (isEditingText()) selectWordIfCollapsed(c);
            QTextCharFormat cf = s->chr;
            cf.setProperty(tp::CharStyle, name);   // (so a change to the style reaches the text)
            c.mergeCharFormat(cf);
            // (each run keeps what it was given by hand, less what the style sets)
            QTextDocument *d = c.document();
            for (QTextBlock b = d->findBlock(c.selectionStart()); b.isValid() && b.position() <= c.selectionEnd(); b = b.next())
                for (auto it = b.begin(); !it.atEnd(); ++it) {
                    const QTextFragment fr = it.fragment();
                    const int fs = std::max(c.selectionStart(), fr.position()), fe = std::min(c.selectionEnd(), fr.position() + fr.length());
                    QTextCharFormat ff = fr.charFormat();
                    if (fs >= fe || !tp::removeDirect(ff, gives)) continue;
                    QTextCursor fc(d);
                    fc.setPosition(fs);
                    fc.setPosition(fe, QTextCursor::KeepAnchor);
                    fc.setCharFormat(ff);
                }
            continue;
        }
        QTextDocument *d = c.document();
        const QSet<int> givesBlk = tp::keysOf(s->blk);
        for (QTextBlock b = d->findBlock(c.selectionStart()); b.isValid() && b.position() <= c.selectionEnd(); b = b.next()) {
            QTextCursor bc(b);
            QTextBlockFormat bf = b.blockFormat();
            tp::setProperties(bf, s->blk);
            bf.setProperty(tp::StyleName, name);
            tp::removeDirect(bf, givesBlk);
            bc.setBlockFormat(bf);
            bc.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
            QTextCharFormat cf = s->chr;
            cf.setProperty(tp::CharStyle, QString());
            bc.mergeCharFormat(cf);
            bc.mergeBlockCharFormat(cf);
            QTextCharFormat bcf = b.charFormat();
            takeOver(bcf);
            QTextCursor(b).setBlockCharFormat(bcf);
            // The runs: the style's look over theirs, the properties it sets no longer hand-set, and the scheme font where the style names none.
            for (auto it = b.begin(); !it.atEnd(); ++it) {
                const QTextFragment fr = it.fragment();
                QTextCursor fc(d);
                fc.setPosition(fr.position());
                fc.setPosition(fr.position() + fr.length(), QTextCursor::KeepAnchor);
                QTextCharFormat ff = fr.charFormat();
                if (!s->chr.hasProperty(QTextFormat::FontFamilies)) ff.clearProperty(QTextFormat::FontFamilies);
                takeOver(ff);
                fc.setCharFormat(ff);
            }
        }
    }
    if (isEditingText()) m_cursor.setCharFormat(m_cursor.block().charFormat());
    endChange();
    Q_EMIT textCursorChanged();
}

void Editor::setDropCap(int lines, int chars, const QString &font)
{
    QTextBlockFormat f;
    f.setProperty(tp::DropCapLines, lines);
    f.setProperty(tp::DropCapChars, chars);
    if (!font.isEmpty()) f.setProperty(tp::DropCapFont, font);
    // Drop caps apply to the first paragraph of the selection only.
    if (isEditingText()) {
        beginChange(tr("Drop Cap"));
        m_cursor.mergeBlockFormat(f);
        markDirectBlocks(QTextCursor(m_cursor.block()), f);
        endChange();
        Q_EMIT textCursorChanged();
        return;
    }
    auto targets = formatTargets();
    if (targets.isEmpty()) return;
    beginChange(tr("Drop Cap"));
    for (QTextCursor c : targets) {
        QTextCursor first(c.document()->begin());
        first.mergeBlockFormat(f);
        markDirectBlocks(first, f);
    }
    endChange();
}

void Editor::insertText(const QString &t)
{
    if (!isEditingText()) return;
    if (!m_typing) {
        beginChange(tr("Typing"));
        m_typing = true;
    }
    m_cursor.insertText(t);
    textEdited();
}

// The language that AutoCorrect and smart quotes follow in the text boxes: the one the program shows its words in.
static QString typingLanguage()
{
    const QString ui = Settings::get().value("ui/language").toString();
    return !ui.isEmpty() ? ui : QLocale().language() == QLocale::C ? QStringLiteral("en-US") : QLocale().name();
}

void Editor::autoCorrectWord()
{
    if (!isEditingText() || m_cursor.hasSelection() || !Settings::get().value("proof/autocorrect", true).toBool()) return;
    const QTextBlock b = m_cursor.block();
    const int rel = m_cursor.position() - b.position();
    const auto fix = autotext::correctWord(b.text(), rel, Settings::get().value("proof/smartQuotes", true).toBool(), autoCorrectConfig(typingLanguage()));
    if (!fix) return;
    if (!m_typing) {
        beginChange(tr("Typing"));
        m_typing = true;
    }
    QTextCursor c = m_cursor;
    c.setPosition(b.position() + fix->from);
    c.setPosition(b.position() + rel, QTextCursor::KeepAnchor);
    c.insertText(fix->with, c.charFormat());
    m_cursor.setPosition(b.position() + fix->from + int(fix->with.size()));
    textEdited();
}

// AutoFormat as you type: "word--word" becomes an em dash, "word - word" an
// en dash, 1/2 1/4 3/4 become fractions, and 1st 2nd 3rd 4th get raised
// suffixes, when the word after ends.
void Editor::autoFormatWord()
{
    if (!isEditingText() || m_cursor.hasSelection() || !Settings::get().value("proof/autoformat", true).toBool()) return;
    const QTextBlock b = m_cursor.block();
    const QString text = b.text().left(m_cursor.position() - b.position());
    const auto fix = autotext::formatWord(text);
    if (!fix) return;
    if (!m_typing) {
        beginChange(tr("Typing"));
        m_typing = true;
    }
    QTextCursor c = m_cursor;
    c.setPosition(b.position() + fix->from);
    c.setPosition(b.position() + fix->to, QTextCursor::KeepAnchor);
    if (fix->superscript) {
        QTextCharFormat sup;
        sup.setVerticalAlignment(QTextCharFormat::AlignSuperScript);
        c.mergeCharFormat(sup);
    } else {
        c.insertText(fix->with, c.charFormat());
    }
    textEdited();
    if (fix->superscript) {
        // Typing goes on in plain text.
        QTextCharFormat plain = m_cursor.charFormat();
        plain.setVerticalAlignment(QTextCharFormat::AlignNormal);
        m_cursor.setCharFormat(plain);
    } else if (fix->with.size() == 1 && fix->to - fix->from == 3) {
        m_cursor.setPosition(b.position() + fix->from + 1);
    }
}

// A web or email address just before the cursor, when a space ends it, becomes a link (one step of its own); what is typed next is not in the link.
void Editor::autoLinkWord()
{
    if (!isEditingText() || m_cursor.hasSelection() || !Settings::get().value("proof/autolink", true).toBool()) return;
    const QTextBlock b = m_cursor.block();
    const auto fix = autotext::linkWord(b.text().left(m_cursor.position() - b.position()));
    if (!fix) return;
    QTextCursor c = m_cursor;
    c.setPosition(b.position() + fix->from);
    c.setPosition(b.position() + fix->to, QTextCursor::KeepAnchor);
    if (c.charFormat().isAnchor()) return;
    flushTyping();
    beginChange(tr("Insert Hyperlink"));
    QTextCharFormat link;
    link.setAnchor(true);
    link.setAnchorHref(fix->with);
    c.mergeCharFormat(link);
    endChange();
    textEdited();
    // Typing goes on outside the link.
    QTextCharFormat plain = m_cursor.charFormat();
    plain.setAnchor(false);
    plain.setAnchorHref(QString());
    m_cursor.setCharFormat(plain);
}

void Editor::typeText(const QString &t)
{
    if (!isEditingText()) return;
    QString s = t;
    // AutoFormat: "* " or "- " starts a bulleted list, "1. " or "1) " a
    // numbered one, at the start of a paragraph.
    if (s == QLatin1String(" ") && !m_cursor.hasSelection() && !m_cursor.block().textList() && Settings::get().value("proof/autoformat", true).toBool()) {
        const QTextBlock b = m_cursor.block();
        const QString before = b.text().left(m_cursor.position() - b.position());
        const auto list = autotext::listStart(before);
        if (list) {
            flushTyping();
            beginChange(tr("AutoFormat List"));
            QTextCursor c = m_cursor;
            c.setPosition(b.position());
            c.setPosition(b.position() + list->length, QTextCursor::KeepAnchor);
            c.removeSelectedText();
            m_cursor.setPosition(b.position());
            if (list->numbered) setList(2, list->paren ? 6 : 0, QString(), list->number);
            else setList(1);
            endChange();
            textEdited();
            return;
        }
    }
    // Smart quotes: opening after a space, an opening bracket or the start
    // of the paragraph; closing (and apostrophes) otherwise.
    if ((s == QLatin1String("\"") || s == QLatin1String("'")) && Settings::get().value("proof/smartQuotes", true).toBool()) {
        const QTextBlock b = m_cursor.block();
        const int rel = m_cursor.selectionStart() - b.position();
        const QChar before = rel > 0 ? b.text().at(rel - 1) : QChar();
        s = autotext::smartQuote(s, before, typingLanguage());
    }
    // A word ends: fix it before going on.
    if (s.size() == 1 && (s[0].isSpace() || QStringLiteral(".,;:!?").contains(s[0]))) {
        if (s[0].isSpace()) autoLinkWord();
        autoCorrectWord();
        autoFormatWord();
    }
    insertText(s);
}

void Editor::editText(const std::function<void(QTextCursor &)> &fn)
{
    if (!isEditingText()) return;
    if (!m_typing) {
        beginChange(tr("Typing"));
        m_typing = true;
    }
    fn(m_cursor);
    textEdited();
}

void Editor::retargetText(const QString &itemId)
{
    if (!isEditingText() || itemId == m_text.itemId) return;
    m_text.itemId = itemId;
    m_sel = QStringList{itemId};
    Q_EMIT selectionChanged();
}

void Editor::insertField(const QString &code)
{
    if (!isEditingText()) return;
    beginChange(tr("Insert Field"));
    QTextCharFormat f = m_cursor.charFormat();
    f.setProperty(tp::Field, code);
    m_cursor.insertText(QString(QChar::ObjectReplacementCharacter), f);
    f.clearProperty(tp::Field);
    m_cursor.setCharFormat(f);
    endChange();
    textEdited();
}

void Editor::editTextAs(const QString &label, const std::function<void(QTextCursor &)> &fn)
{
    if (!isEditingText()) return;
    beginChange(label);
    fn(m_cursor);
    endChange();
    textEdited();
}

void Editor::insertTextBlock(const QString &text, const QString &label)
{
    if (!isEditingText()) return;
    beginChange(label);
    m_cursor.insertText(text);
    endChange();
    textEdited();
}

// ---------------- copying formatting ----------------
void Editor::copyFormatting()
{
    painterItem = QJsonObject();
    painterHasText = false;
    if (Item *it = single()) {
        const QJsonObject o = it->toJson();
        for (const char *k : {"fill", "stroke", "fx"})
            if (o.contains(k)) painterItem[k] = o[k];
        if (!o.contains("fill")) painterItem["fill"] = Fill().toJson();
        if (!o.contains("stroke")) painterItem["stroke"] = Stroke().toJson();
    }
    if (isEditingText() || (single() && single()->hasText())) {
        painterText = currentCharFormat();
        painterHasText = true;
    }
}

void Editor::applyCopiedFormatting(Item *it)
{
    const QJsonObject &f = painterItem;
    if (f.contains("fill")) it->fill = Fill::fromJson(f["fill"].toObject());
    if (f.contains("stroke")) it->stroke = Stroke::fromJson(f["stroke"].toObject());
    if (f.contains("fx")) it->fx = Effects::fromJson(f["fx"].toObject());
    if (!painterHasText) return;
    QString sid;
    if (it->type() == ItemType::Text) sid = m_doc->chainOf(it->id).value(0, static_cast<TextItem *>(it))->storyId;
    else if (it->type() == ItemType::Shape) sid = static_cast<ShapeItem *>(it)->storyId;
    if (QTextDocument *d = m_doc->storyDoc(sid)) {
        QTextCursor c(d);
        c.select(QTextCursor::Document);
        c.mergeCharFormat(painterText);
    }
}

void Editor::pasteFormattingTo(const QString &itemId)
{
    Item *it = m_doc->item(itemId);
    if (!it || !hasCopiedFormatting()) return;
    change(tr("Paste Formatting"), [&] { applyCopiedFormatting(it); });
}

void Editor::pasteFormatting()
{
    if (!hasCopiedFormatting()) return;
    if (isEditingText()) {
        if (painterHasText) mergeCharFormat(painterText, tr("Paste Formatting"));
        return;
    }
    const QVector<Item *> items = selectedItems();
    if (items.isEmpty()) return;
    change(tr("Paste Formatting"), [&] {
        for (Item *it : items) applyCopiedFormatting(it);
    });
}

} // namespace jp
