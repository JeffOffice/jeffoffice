#include "app/shapetextedit.h"

#include <QAbstractTextDocumentLayout>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QMimeData>
#include <QPalette>
#include <QScrollBar>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextList>

#include <algorithm>
#include <cmath>
#include <iterator>
#include <optional>

namespace jp {

SelectedShapeKey keyOnSelectedShape(const QKeyEvent *e, QString *typed)
{
    const bool ctrl = e->modifiers() & Qt::ControlModifier;
    if ((e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter || e->key() == Qt::Key_F2) && !(e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier))) return SelectedShapeKey::Begin;
    const QString text = e->text();
    if (!ctrl && !text.isEmpty() && text[0].isPrint() && e->key() != Qt::Key_Return && e->key() != Qt::Key_Enter) {
        if (typed) *typed = text;
        return SelectedShapeKey::BeginTyped;
    }
    return SelectedShapeKey::None;
}

double nextFontSize(double points, int direction)
{
    static const double steps[] = {6, 7, 8, 9, 10, 11, 12, 14, 16, 18, 20, 22, 24, 26, 28, 36, 48, 72, 96, 120, 144, 200, 288};
    double next = points;
    if (direction > 0) {
        for (double v : steps)
            if (v > points + 0.01) {
                next = v;
                break;
            }
        if (next == points) next = points + 12;
    } else {
        for (int i = int(std::size(steps)) - 1; i >= 0; --i)
            if (steps[i] < points - 0.01) {
                next = steps[i];
                break;
            }
        if (next == points) next = std::max(1.0, points - 1);
    }
    return next;
}

namespace {

// The sizes in a story times `factor` (to show it at a zoom, and back).
void scaleSizes(QTextDocument *story, double factor)
{
    if (qFuzzyCompare(factor, 1.0)) return;
    QFont f = story->defaultFont();
    if (f.pointSizeF() > 0) {
        f.setPointSizeF(std::round(f.pointSizeF() * factor * 100) / 100);
        story->setDefaultFont(f);
    }
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next())
        for (auto it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment fr = it.fragment();
            if (!fr.charFormat().hasProperty(QTextFormat::FontPointSize)) continue;
            QTextCursor c(story);
            c.setPosition(fr.position());
            c.setPosition(fr.position() + fr.length(), QTextCursor::KeepAnchor);
            QTextCharFormat cf;
            cf.setFontPointSize(std::round(fr.charFormat().fontPointSize() * factor * 100) / 100);
            c.mergeCharFormat(cf);
        }
}

} // namespace

ShapeTextEdit::ShapeTextEdit(QWidget *parent) : QTextEdit(parent)
{
    setFrameShape(QFrame::NoFrame);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setAcceptRichText(true);
    QPalette p = palette();
    p.setColor(QPalette::Base, QColor(255, 255, 255, 190));
    p.setColor(QPalette::Text, Qt::black);
    setPalette(p);
    document()->setDocumentMargin(0);
}

ShapeTextEdit::~ShapeTextEdit()
{
    // (the story is the editor's until take(): it is not the editor's own document, so the editor stops using it first)
    if (m_story) setDocument(new QTextDocument(this));
}

void ShapeTextEdit::begin(std::unique_ptr<QTextDocument> story, const QRect &frame, const ShapeTextLook &look, const QString &typed, bool replace)
{
    m_look = look;
    m_changed = false;
    m_done = false;
    m_story = std::move(story);
    m_story->setDocumentMargin(0);
    m_story->setUndoRedoEnabled(false);
    scaleSizes(m_story.get(), look.zoom);
    m_story->setUndoRedoEnabled(true);
    m_story->clearUndoRedoStacks();
    setDocument(m_story.get());
    setLineWrapMode(look.wrap ? QTextEdit::WidgetWidth : QTextEdit::NoWrap);
    setGeometry(frame);
    QTextCursor c(m_story.get());
    c.movePosition(QTextCursor::End);
    setTextCursor(c);
    connect(m_story.get(), &QTextDocument::contentsChanged, this, [this] {
        m_changed = true;
        refit();
        marksChanged();
        Q_EMIT formatChanged();
    });
    connect(this, &QTextEdit::currentCharFormatChanged, this, [this] { Q_EMIT formatChanged(); });
    if (!typed.isEmpty()) {
        if (replace) selectAll();
        textCursor().insertText(typed);
    }
    refit();
    marksChanged();
    m_story->setModified(false);
    if (!typed.isEmpty()) m_changed = true;
    show();
    setFocus(Qt::OtherFocusReason);
}

std::unique_ptr<QTextDocument> ShapeTextEdit::take()
{
    m_done = true;
    disconnect(m_story.get(), nullptr, this, nullptr);
    std::unique_ptr<QTextDocument> out = std::move(m_story);
    setDocument(new QTextDocument(this));
    out->setUndoRedoEnabled(false);
    scaleSizes(out.get(), 1 / m_look.zoom);
    hide();
    return out;
}

void ShapeTextEdit::refit()
{
    if (m_refitting || !m_story) return;
    m_refitting = true;
    const QMarginsF in = m_look.insets;
    const int left = int(in.left()), right = int(in.right()), top = int(in.top()), bottom = int(in.bottom());
    const int availWidth = std::max(1, width() - left - right), avail = std::max(1, height() - top - bottom);
    if (m_look.wrap) m_story->setTextWidth(availWidth);
    else m_story->setTextWidth(-1);
    const double h = m_story->documentLayout()->documentSize().height();
    int offset = 0;
    if (h < avail) {
        if (m_look.anchor & Qt::AlignVCenter) offset = int((avail - h) / 2);
        else if (m_look.anchor & Qt::AlignBottom) offset = int(avail - h);
    }
    setViewportMargins(left, top + offset, right, bottom);
    m_refitting = false;
    if (h > avail) ensureCursorVisible();
}

void ShapeTextEdit::resizeEvent(QResizeEvent *e)
{
    QTextEdit::resizeEvent(e);
    refit();
}

void ShapeTextEdit::focusOutEvent(QFocusEvent *e)
{
    QTextEdit::focusOutEvent(e);
    if (m_done || !m_story) return;
    // (A pop-up, as the menu of the spelling marks, or another window coming forward does not end the edit: the host decides about the rest.)
    if (e->reason() != Qt::PopupFocusReason && e->reason() != Qt::ActiveWindowFocusReason) Q_EMIT focusLeft();
}

void ShapeTextEdit::mergeFormat(const QTextCharFormat &f)
{
    QTextCursor c = textCursor();
    c.mergeCharFormat(f);
    mergeCurrentCharFormat(f);
}

void ShapeTextEdit::setBold(bool on)
{
    QTextCharFormat f;
    f.setFontWeight(on ? QFont::Bold : QFont::Normal);
    mergeFormat(f);
}

void ShapeTextEdit::setItalic(bool on)
{
    QTextCharFormat f;
    f.setFontItalic(on);
    mergeFormat(f);
}

void ShapeTextEdit::setUnderline(bool on)
{
    QTextCharFormat f;
    f.setFontUnderline(on);
    mergeFormat(f);
}

void ShapeTextEdit::setFontFamilyName(const QString &family)
{
    QTextCharFormat f;
    f.setFontFamilies({family});
    mergeFormat(f);
}

double ShapeTextEdit::fontSizePt() const
{
    const QTextCharFormat f = currentCharFormat();
    return f.hasProperty(QTextFormat::FontPointSize) ? std::round(f.fontPointSize() / m_look.zoom * 100) / 100 : 0;
}

void ShapeTextEdit::setFontSizePt(double points)
{
    QTextCharFormat f;
    f.setFontPointSize(points * m_look.zoom);
    mergeFormat(f);
}

void ShapeTextEdit::growFont(int direction)
{
    QTextCursor c = textCursor();
    if (!c.hasSelection()) {
        const double cur = fontSizePt() > 0 ? fontSizePt() : document()->defaultFont().pointSizeF() / m_look.zoom;
        setFontSizePt(nextFontSize(cur > 0 ? cur : 11, direction));
        return;
    }
    const int from = c.selectionStart(), to = c.selectionEnd();
    c.beginEditBlock();
    for (QTextBlock b = document()->findBlock(from); b.isValid() && b.position() < to; b = b.next())
        for (auto it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment fr = it.fragment();
            const int fs = std::max(from, fr.position()), fe = std::min(to, fr.position() + fr.length());
            if (fs >= fe) continue;
            const double cur = fr.charFormat().hasProperty(QTextFormat::FontPointSize) ? fr.charFormat().fontPointSize() / m_look.zoom : 11;
            QTextCursor fc(document());
            fc.setPosition(fs);
            fc.setPosition(fe, QTextCursor::KeepAnchor);
            QTextCharFormat f;
            f.setFontPointSize(nextFontSize(cur, direction) * m_look.zoom);
            fc.mergeCharFormat(f);
        }
    c.endEditBlock();
}

void ShapeTextEdit::setColor(const QColor &color)
{
    QTextCharFormat f;
    f.setForeground(color);
    mergeFormat(f);
}

void ShapeTextEdit::setAlign(Qt::Alignment alignment)
{
    setAlignment(alignment);
}

bool ShapeTextEdit::hasBullets() const
{
    return textCursor().block().textList() != nullptr;
}

void ShapeTextEdit::setBullets(bool on)
{
    if (m_bullets) {
        QTextCursor own = textCursor();
        m_bullets(own, on);
        setTextCursor(own);
        return;
    }
    QTextCursor c = textCursor();
    const int from = c.selectionStart(), to = c.selectionEnd();
    c.beginEditBlock();
    QTextList *list = nullptr;
    for (QTextBlock b = document()->findBlock(from); b.isValid() && b.position() <= to; b = b.next()) {
        if (b.position() == to && to > from) break;
        QTextCursor bc(b);
        if (on) {
            if (b.textList()) {
                list = b.textList();
                continue;
            }
            if (list) {
                list->add(b);
            } else {
                QTextListFormat lf;
                lf.setStyle(QTextListFormat::ListDisc);
                lf.setIndent(1);
                list = bc.createList(lf);
            }
        } else if (QTextList *l = b.textList()) {
            l->remove(b);
            QTextBlockFormat bf = b.blockFormat();
            bf.setIndent(0);
            bf.setTextIndent(0);
            bc.setBlockFormat(bf);
        }
    }
    c.endEditBlock();
}

void ShapeTextEdit::indent(int direction)
{
    QTextCursor c = textCursor();
    const int from = c.selectionStart(), to = c.selectionEnd();
    c.beginEditBlock();
    for (QTextBlock b = document()->findBlock(from); b.isValid() && b.position() <= to; b = b.next()) {
        if (b.position() == to && to > from) break;
        if (QTextList *l = b.textList()) {
            QTextListFormat lf = l->format();
            lf.setIndent(std::max(1, lf.indent() + direction));
            l->setFormat(lf);
        } else {
            QTextCursor bc(b);
            QTextBlockFormat bf = b.blockFormat();
            bf.setIndent(std::max(0, bf.indent() + direction));
            bc.setBlockFormat(bf);
        }
    }
    c.endEditBlock();
}

void ShapeTextEdit::setSpellCheck(SpellCheck check)
{
    m_check = std::move(check);
    marksChanged();
}

void ShapeTextEdit::marksChanged()
{
    m_marks = 0;
    if (!m_check || !m_story) {
        if (!extraSelections().isEmpty()) setExtraSelections({});
        return;
    }
    QList<QTextEdit::ExtraSelection> marks;
    QTextCharFormat under;
    under.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
    under.setUnderlineColor(Qt::red);
    for (const QPair<int, int> &r : m_check(m_story.get())) {
        QTextEdit::ExtraSelection s;
        s.cursor = QTextCursor(m_story.get());
        s.cursor.setPosition(r.first);
        s.cursor.setPosition(r.second, QTextCursor::KeepAnchor);
        s.format = under;
        marks << s;
    }
    m_marks = int(marks.size());
    setExtraSelections(marks);
}

void ShapeTextEdit::insertFromMimeData(const QMimeData *source)
{
    // (Paste into an empty paragraph brings the pasted paragraph's alignment with it: Qt keeps the paragraph's own.)
    const QTextCursor at = textCursor();
    const bool intoEmpty = at.block().text().isEmpty() || (at.hasSelection() && at.selectionStart() <= at.block().position() && at.selectionEnd() >= at.block().position() + at.block().length() - 1);
    const int start = at.selectionStart();
    std::optional<Qt::Alignment> align;
    if (intoEmpty && source->hasHtml()) {
        QTextDocument tmp;
        // (the fragment marks make the importer drop the paragraph's own look)
        tmp.setHtml(QString(source->html()).remove(QStringLiteral("<!--StartFragment-->")).remove(QStringLiteral("<!--EndFragment-->")));
        align = tmp.firstBlock().blockFormat().alignment();
    }
    QTextEdit::insertFromMimeData(source);
    if (align) {
        QTextCursor b(document()->findBlock(start));
        QTextBlockFormat bf = b.blockFormat();
        bf.setAlignment(*align);
        b.setBlockFormat(bf);
    }
}

void ShapeTextEdit::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape && !(e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier | Qt::AltModifier))) {
        e->accept();
        Q_EMIT finished(true);
        return;
    }
    if ((e->modifiers() & ~Qt::KeypadModifier) == Qt::ControlModifier) {
        switch (e->key()) {
        case Qt::Key_B: toggleBold(); e->accept(); return;
        case Qt::Key_I: toggleItalic(); e->accept(); return;
        case Qt::Key_U: toggleUnderline(); e->accept(); return;
        case Qt::Key_E: setAlign(Qt::AlignHCenter); e->accept(); return;
        case Qt::Key_L: setAlign(Qt::AlignLeft); e->accept(); return;
        case Qt::Key_R: setAlign(Qt::AlignRight); e->accept(); return;
        case Qt::Key_J: setAlign(Qt::AlignJustify); e->accept(); return;
        case Qt::Key_BracketRight: growFont(1); e->accept(); return;
        case Qt::Key_BracketLeft: growFont(-1); e->accept(); return;
        default: break;
        }
    }
    QTextEdit::keyPressEvent(e);
}

} // namespace jp
