#include "app/listmarkers.h"

#include "core/fonts.h"
#include "text/textengine.h"

#include <QAbstractTextDocumentLayout>
#include <QEvent>
#include <QFontDatabase>
#include <QFontMetricsF>
#include <QPaintEvent>
#include <QPainter>
#include <QScrollBar>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextEdit>
#include <QTextLayout>
#include <QTextList>
#include <algorithm>
#include <cmath>
#include <cstring>

namespace jp {

// What is laid over the text view: nothing of its own, only the markers (and, under each, the plain mark the view's own document draws where a level is indented, which this
// puts out of sight). It lets the mouse through.
class ListMarkers::Layer : public QWidget {
public:
    Layer(QTextEdit *edit, ListMarkers *owner) : QWidget(edit), m_owner(owner)
    {
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setAttribute(Qt::WA_NoSystemBackground);
        setFocusPolicy(Qt::NoFocus);
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        m_owner->paint(p);
    }

private:
    ListMarkers *m_owner;
};

ListMarkers::ListMarkers(QTextEdit *edit, std::function<void(int, int)> margins, std::function<const ColorScheme *()> colors)
    : QObject(edit), m_edit(edit), m_margins(std::move(margins)), m_colors(std::move(colors))
{
    m_layer = new Layer(edit, this);
    m_layer->setGeometry(edit->rect());
    m_layer->raise();
    m_layer->show();
    edit->installEventFilter(this);
    edit->viewport()->installEventFilter(this);
    connect(edit, &QTextEdit::textChanged, this, [this] { refresh(); });
    connect(edit->verticalScrollBar(), &QScrollBar::valueChanged, this, [this] { refresh(); });
    connect(edit->horizontalScrollBar(), &QScrollBar::valueChanged, this, [this] { refresh(); });
}

ListMarkers::~ListMarkers() = default;

// (The view and its viewport are told of a new size before they have taken it, and the text is laid out for it after: the paragraphs in view are gone through when those have run.)
bool ListMarkers::eventFilter(QObject *watched, QEvent *e)
{
    if ((watched == m_edit && (e->type() == QEvent::Resize || e->type() == QEvent::Show || e->type() == QEvent::LayoutRequest)) || (watched == m_edit->viewport() && (e->type() == QEvent::Resize || e->type() == QEvent::Show))) {
        if (watched == m_edit) {
            m_layer->setGeometry(m_edit->rect());
            m_layer->raise();
        }
        if (!m_pending) {
            m_pending = true;
            QMetaObject::invokeMethod(this, [this] {
                m_pending = false;
                refresh();
            }, Qt::QueuedConnection);
        }
    }
    return QObject::eventFilter(watched, e);
}

void ListMarkers::setScale(double scale)
{
    if (qFuzzyCompare(scale, m_scale)) return;
    m_scale = scale;
    refresh();
}

// The look of a paragraph's marker: its text from the page text engine (what the printed page shows), in the font, size, and color the list gives it (the paragraph mark's own
// where it gives none).
const ListMarkers::Look &ListMarkers::lookOf(const QTextBlock &b)
{
    const QTextDocument *doc = b.document();
    const std::array<qint64, 4> key = {qint64(reinterpret_cast<quintptr>(doc)), qint64(doc->revision()), qint64(std::llround(m_scale * 1000)), qint64(m_edit->viewport()->logicalDpiY())};
    if (key != m_lookKey) {
        m_lookKey = key;
        m_looks.clear();
    }
    auto it = m_looks.constFind(b.position());
    if (it != m_looks.constEnd()) return *it;
    ++s_built;
    Look look;
    QTextList *list = b.textList();
    const QTextListFormat lf = list ? list->format() : QTextListFormat();
    look.text = listMarkerText(b);
    // (As the printed page sets it: the size and look of the item's first words, and the paragraph mark's for an item with none.)
    QTextCharFormat mark = b.charFormat();
    if (b.begin() != b.end()) {
        const QTextCharFormat first = b.begin().fragment().charFormat();
        for (int property : {QTextFormat::FontPointSize, QTextFormat::FontFamilies, QTextFormat::FontFamily, QTextFormat::FontWeight, QTextFormat::FontItalic})
            if (first.hasProperty(property)) mark.setProperty(property, first.property(property));
            else mark.clearProperty(property);
    }
    QFont font = mark.font().resolve(doc->defaultFont());
    double points = font.pointSizeF() > 0 ? font.pointSizeF() : 11.0;
    QString custom = b.blockFormat().stringProperty(tp::BulletChar);
    if (custom.isEmpty()) custom = lf.stringProperty(tp::BulletChar);
    if (lf.hasProperty(tp::BulletSize)) points = std::max(1.0, lf.property(tp::BulletSize).toDouble()) * fontPointFactor();
    const QString bfont = lf.stringProperty(tp::BulletFont);
    if (!bfont.isEmpty() && !custom.isEmpty()) {
        // (A bullet in its own font is that font's regular face.)
        font.setWeight(QFont::Normal);
        font.setItalic(false);
        if (isSymbolFont(bfont) && !QFontDatabase::hasFamily(bfont)) {
            const QString uni = symbolToUnicode(bfont, custom.at(0).unicode());
            look.text = uni.isEmpty() ? QStringLiteral("•") : uni;
        } else {
            font.setFamily(bfont);
            if (isSymbolFont(bfont) && custom.size() == 1 && custom.at(0).unicode() >= 0x20 && custom.at(0).unicode() <= 0xFF) look.text = QString(QChar(char16_t(0xF000 + custom.at(0).unicode())));
        }
    }
    font.setPointSizeF(points * m_scale);
    look.font = font;
    QColor color = mark.foreground().style() != Qt::NoBrush ? mark.foreground().color() : m_edit->palette().color(QPalette::Text);
    const ColorScheme *scheme = nullptr;
    static const ColorScheme standard;
    auto resolve = [&](const QString &ref) {
        if (!scheme) scheme = m_colors ? m_colors() : nullptr;
        return ColorRef::fromString(ref).resolve(scheme ? *scheme : standard);
    };
    if (const QString ref = mark.stringProperty(tp::ColorRefP); !ref.isEmpty()) color = resolve(ref);
    if (const QString bc = lf.stringProperty(tp::BulletColor); !bc.isEmpty() && bc != QLatin1String("auto")) color = resolve(bc);
    look.color = color;
    const QFontMetricsF fm(font);
    look.ascent = fm.ascent();
    look.height = fm.height();
    look.width = fm.horizontalAdvance(look.text);
    if (lf.hasProperty(tp::BulletPicture)) {
        look.picture = bulletPictureImage(lf.property(tp::BulletPicture).toByteArray());
        if (!look.picture.isNull()) look.width = look.height * 0.9;
    }
    return *m_looks.insert(b.position(), look);
}

// The list items in view, and the gutter they need: the markers' widths and gaps less what the text's own left margin and indent give them. It is gone through once for a state of the
// words, the scrolling, and the size.
void ListMarkers::run(bool margins)
{
    if (m_busy) return;
    QTextDocument *doc = m_edit->document();
    QWidget *vp = m_edit->viewport();
    auto stateKey = [&] {
        return std::array<qint64, 8>{qint64(reinterpret_cast<quintptr>(doc)), qint64(doc->revision()), qint64(m_edit->verticalScrollBar()->value()), qint64(m_edit->horizontalScrollBar()->value()),
                                     qint64(vp->width()), qint64(vp->height()), qint64(std::llround(m_scale * 1000)), 0};
    };
    if (stateKey() == m_viewKey && (!margins || stateKey() == m_gutterKey)) return;
    struct Guard {
        bool &b;
        ~Guard() { b = false; }
    } guard{m_busy};
    m_busy = true;
    ++s_runs;
    for (int pass = 0; pass < 2; ++pass) {
        m_view.clear();
        int left = 0, right = 0;
        if (m_edit->isVisible() || vp->width() > 0) {
            const QTextBlock first = m_edit->cursorForPosition(QPoint(0, 0)).block();
            const QTextBlock last = m_edit->cursorForPosition(QPoint(std::max(0, vp->width() - 1), std::max(0, vp->height() - 1))).block();
            const double gap = 4.0 * vp->logicalDpiY() / 72.0 * m_scale;
            for (QTextBlock b = first; b.isValid(); b = b.next()) {
                if (b.textList() && b.layout() && b.layout()->lineCount() > 0) {
                    m_view.push_back(b.position());
                    const Look &look = lookOf(b);
                    const QTextLine line = b.layout()->lineAt(0);
                    const QRectF text = line.naturalTextRect();
                    const double start = b.layout()->position().x() + text.left(), end = b.layout()->position().x() + text.right();
                    const double hang = hangOf(b);
                    if (b.length() <= 1) {}   // (no marker is drawn: none needs room)
                    else if (hang < 0) left = std::max(left, int(std::ceil(-(start + hang) - 1.0)));   // (the marker hangs left of the text, where the page has it)
                    else if (b.textDirection() == Qt::RightToLeft) right = std::max(right, int(std::ceil(look.width + gap - ((doc->textWidth() > 0 ? doc->textWidth() : double(vp->width())) - end))));
                    else left = std::max(left, int(std::ceil(look.width + gap - start)));
                }
                if (b == last) break;
            }
        }
        if (!margins && (left != m_left || right != m_right) && !m_pending) {
            // (a paint or a read came before the gutter followed the words: it does, once the event loop has run)
            m_pending = true;
            QMetaObject::invokeMethod(this, [this] {
                m_pending = false;
                refresh();
            }, Qt::QueuedConnection);
        }
        if (!margins || (left == m_left && right == m_right)) break;
        m_left = left;
        m_right = right;
        m_margins(m_left, m_right);   // (the text's width changes: the paragraphs in view may too, so they are gone through again)
    }
    m_viewKey = stateKey();
    if (margins) m_gutterKey = m_viewKey;
    m_layer->update();
}

void ListMarkers::refresh() { run(true); }

void ListMarkers::setShown(bool shown) { m_layer->setVisible(shown); }

double ListMarkers::hangOf(const QTextBlock &b)
{
    const QVariant v = b.blockFormat().property(HangProperty);
    return v.isValid() ? std::min(0.0, v.toDouble()) : 0.0;
}

QVector<ListMarkers::Marker> ListMarkers::visibleMarkers() const
{
    QVector<Marker> out;
    for (const Marker &m : allMarkers())
        if (!m.empty) out.push_back(m);
    return out;
}

QVector<ListMarkers::Marker> ListMarkers::allMarkers() const
{
    QVector<Marker> out;
    auto *self = const_cast<ListMarkers *>(this);
    self->run(false);   // (a paint that came before the words' change reached the markers: the paragraphs in view are made again, the gutter waits for its own call)
    QTextDocument *doc = m_edit->document();
    const QPointF origin = QPointF(m_edit->viewport()->pos()) - QPointF(m_edit->horizontalScrollBar()->value(), m_edit->verticalScrollBar()->value());
    const double gap = 4.0 * m_edit->viewport()->logicalDpiY() / 72.0 * m_scale;
    for (int pos : m_view) {
        const QTextBlock b = doc->findBlock(pos);
        if (!b.isValid() || !b.textList() || !b.layout() || b.layout()->lineCount() == 0) continue;
        const Look &look = self->lookOf(b);
        const QTextLine line = b.layout()->lineAt(0);
        const QRectF text = line.naturalTextRect();
        const QPointF at = b.layout()->position();
        Marker m;
        m.blockPos = pos;
        m.empty = b.length() <= 1;
        m.text = look.text;
        m.rightToLeft = b.textDirection() == Qt::RightToLeft;
        m.baseline = origin.y() + at.y() + line.y() + line.ascent();
        const double hang = hangOf(b);
        const double x = hang < 0 ? origin.x() + at.x() + text.left() + hang : m.rightToLeft ? origin.x() + at.x() + text.right() + gap : origin.x() + at.x() + text.left() - gap - look.width;
        m.rect = QRectF(x, m.baseline - look.ascent, look.width, look.height);
        out.push_back(m);
    }
    return out;
}

void ListMarkers::paint(QPainter &p)
{
    if (m_view.isEmpty()) return;
    const QVector<Marker> markers = allMarkers();
    const QRect vp = m_edit->viewport()->geometry();
    const QColor ground = m_edit->viewport()->palette().color(m_edit->viewport()->backgroundRole());
    p.setRenderHint(QPainter::TextAntialiasing);
    QTextDocument *doc = m_edit->document();
    for (const Marker &m : markers) {
        const QTextBlock b = doc->findBlock(m.blockPos);
        const Look &look = lookOf(b);
        // The plain mark the document draws itself, left of the first line's text (or right of it), is covered: the gap before the text and what Qt draws in it is as wide as a space and the mark.
        const QFontMetricsF fm(look.font);
        const double room = fm.horizontalAdvance(QLatin1Char(' ')) + std::max(fm.horizontalAdvance(look.text), fm.lineSpacing()) + 3;
        const QTextLine line = b.layout()->lineAt(0);
        const QPointF origin = QPointF(m_edit->viewport()->pos()) - QPointF(m_edit->horizontalScrollBar()->value(), m_edit->verticalScrollBar()->value());
        const QRectF text = line.naturalTextRect().translated(b.layout()->position() + origin);
        const double top = origin.y() + b.layout()->position().y() + line.y();
        QRectF cover = m.rightToLeft ? QRectF(text.right() + 1, top, room, line.height()) : QRectF(text.left() - 1 - room, top, room, line.height());
        p.save();
        p.setClipRect(vp);
        p.fillRect(cover, ground);
        p.restore();
        if (m.empty) continue;
        if (m.rect.right() < 0 || m.rect.left() > m_layer->width() || m.rect.bottom() < 0 || m.rect.top() > m_layer->height()) continue;
        if (!look.picture.isNull()) {
            p.drawImage(QRectF(m.rect.left(), m.baseline - look.height * 0.9, look.width, look.height * 0.9), look.picture);
            continue;
        }
        p.setFont(look.font);
        p.setPen(look.color);
        p.drawText(QPointF(m.rect.left(), m.baseline), look.text);
    }
}

} // namespace jp
