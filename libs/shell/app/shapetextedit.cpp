#include "app/shapetextedit.h"

#include <QAbstractTextDocumentLayout>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QMimeData>
#include <QPalette>
#include <QScrollBar>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextList>

#include <QVector>
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

// Every size a story holds (its default font, each run's, and each paragraph's end mark: what typing in an empty paragraph takes), changed by `map`. The places are collected first, and
// the sizes put in afterward, so that nothing changes under the walk.
void mapSizes(QTextDocument *story, const std::function<double(double)> &map)
{
    struct Run {
        int from, to;
        double size;
    };
    QVector<Run> runs;
    QVector<QPair<int, double>> marks;
    for (QTextBlock b = story->begin(); b.isValid(); b = b.next()) {
        if (b.charFormat().hasProperty(QTextFormat::FontPointSize)) marks << qMakePair(b.position(), b.charFormat().fontPointSize());
        for (auto it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment fr = it.fragment();
            if (fr.charFormat().hasProperty(QTextFormat::FontPointSize)) runs << Run{fr.position(), fr.position() + fr.length(), fr.charFormat().fontPointSize()};
        }
    }
    QFont f = story->defaultFont();
    if (f.pointSizeF() > 0) {
        f.setPointSizeF(map(f.pointSizeF()));
        story->setDefaultFont(f);
    }
    for (const Run &r : runs) {
        QTextCursor c(story);
        c.setPosition(r.from);
        c.setPosition(r.to, QTextCursor::KeepAnchor);
        QTextCharFormat cf;
        cf.setFontPointSize(map(r.size));
        c.mergeCharFormat(cf);
    }
    for (const auto &m : marks) {
        QTextCursor c(story->findBlock(m.first));
        QTextCharFormat cf = c.blockCharFormat();
        cf.setFontPointSize(map(m.second));
        c.setBlockCharFormat(cf);
    }
}

} // namespace

// The back of the editor: the words of the shape that the page still draws under it do not show through.
class ShapeTextBack : public QWidget {
public:
    explicit ShapeTextBack(QWidget *parent) : QWidget(parent)
    {
        setAttribute(Qt::WA_TransparentForMouseEvents);
        hide();
    }

protected:
    void paintEvent(QPaintEvent *) override { QPainter(this).fillRect(rect(), QColor(255, 255, 255, 190)); }
};

ShapeTextEdit::ShapeTextEdit(QWidget *parent) : QTextEdit(parent)
{
    setFrameShape(QFrame::NoFrame);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setAcceptRichText(true);
    QPalette p = palette();
    // (the whole frame is backed, by a widget of its own under the editor: the viewport has no back, for the room round the words and the room the words are set down by)
    p.setColor(QPalette::Base, Qt::transparent);
    p.setColor(QPalette::Text, Qt::black);
    setPalette(p);
    viewport()->setAutoFillBackground(false);
    document()->setDocumentMargin(0);
}

ShapeTextEdit::~ShapeTextEdit()
{
    blockSignals(true);   // (a host that is going away is told nothing)
    delete m_back.data();
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
    m_originals.clear();
    if (!qFuzzyCompare(look.zoom, 1.0)) mapSizes(m_story.get(), [this](double v) { return scaled(v); });
    m_story->setUndoRedoEnabled(true);
    m_story->clearUndoRedoStacks();
    setDocument(m_story.get());
    setLineWrapMode(look.wrap ? QTextEdit::WidgetWidth : QTextEdit::NoWrap);
    if (!m_back) m_back = new ShapeTextBack(parentWidget());
    setGeometry(frame);
    m_back->setGeometry(frame);
    m_back->show();
    m_back->stackUnder(this);
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
    connect(this, &QTextEdit::cursorPositionChanged, this, [this] { Q_EMIT formatChanged(); });
    QPalette pal = palette();
    pal.setColor(QPalette::Text, look.textColor.isValid() ? look.textColor : QColor(Qt::black));
    setPalette(pal);
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
    if (m_back) m_back->hide();
    out->setUndoRedoEnabled(false);
    if (!qFuzzyCompare(m_look.zoom, 1.0)) mapSizes(out.get(), [this](double v) { return unscaled(v); });
    hide();
    return out;
}

double ShapeTextEdit::scaled(double points)
{
    if (qFuzzyCompare(m_look.zoom, 1.0)) return points;
    const double shown = std::round(points * m_look.zoom * 1000) / 1000;
    m_originals.insert(qRound64(shown * 1000), points);   // (a size is given back as it was, to its last digit)
    return shown;
}

double ShapeTextEdit::unscaled(double shown) const
{
    if (qFuzzyCompare(m_look.zoom, 1.0)) return shown;
    const auto it = m_originals.constFind(qRound64(shown * 1000));
    return it != m_originals.cend() ? it.value() : std::round(shown / m_look.zoom * 100) / 100;
}

std::unique_ptr<QTextDocument> ShapeTextEdit::snapshot() const
{
    if (!m_story) return nullptr;
    std::unique_ptr<QTextDocument> copy(m_story->clone());
    copy->setUndoRedoEnabled(false);
    if (!qFuzzyCompare(m_look.zoom, 1.0)) mapSizes(copy.get(), [this](double v) { return unscaled(v); });
    return copy;
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
    if (m_back) m_back->setGeometry(geometry());
    refit();
}

void ShapeTextEdit::moveEvent(QMoveEvent *e)
{
    QTextEdit::moveEvent(e);
    if (m_back) m_back->setGeometry(geometry());
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
    return f.hasProperty(QTextFormat::FontPointSize) ? unscaled(f.fontPointSize()) : 0;
}

void ShapeTextEdit::setFontSizePt(double points)
{
    QTextCharFormat f;
    f.setFontPointSize(scaled(points));
    mergeFormat(f);
}

void ShapeTextEdit::growFont(int direction)
{
    QTextCursor c = textCursor();
    if (!c.hasSelection()) {
        const double cur = fontSizePt() > 0 ? fontSizePt() : unscaled(document()->defaultFont().pointSizeF());
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
            const double cur = fr.charFormat().hasProperty(QTextFormat::FontPointSize) ? unscaled(fr.charFormat().fontPointSize()) : 11;
            QTextCursor fc(document());
            fc.setPosition(fs);
            fc.setPosition(fe, QTextCursor::KeepAnchor);
            QTextCharFormat f;
            f.setFontPointSize(scaled(nextFontSize(cur, direction)));
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

bool ShapeTextEdit::event(QEvent *e)
{
    // The keys of the words are the editor's, whatever a window has them for: they reach it as keys, not as the window's shortcuts.
    if (e->type() == QEvent::ShortcutOverride) {
        auto *k = static_cast<QKeyEvent *>(e);
        if ((k->modifiers() & ~Qt::KeypadModifier) == Qt::ControlModifier) {
            switch (k->key()) {
            case Qt::Key_B: case Qt::Key_I: case Qt::Key_U: case Qt::Key_E: case Qt::Key_L: case Qt::Key_R: case Qt::Key_J: case Qt::Key_BracketLeft: case Qt::Key_BracketRight:
                e->accept();
                return true;
            default: break;
            }
        }
        if (k->key() == Qt::Key_Escape && !(k->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier | Qt::AltModifier))) {
            e->accept();
            return true;
        }
    }
    return QTextEdit::event(e);
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
