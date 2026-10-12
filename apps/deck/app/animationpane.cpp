#include "app/animationpane.h"

#include "app/editor.h"
#include "app/icons.h"
#include "app/slideshow.h"
#include "core/document.h"

#include <QAbstractItemView>
#include <QCoreApplication>
#include <QDropEvent>
#include <QFontMetricsF>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QItemSelectionModel>
#include <QKeyEvent>
#include <QLabel>
#include <QListWidget>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QStyledItemDelegate>
#include <QTextBlock>
#include <QTextDocument>
#include <QVBoxLayout>
#include <algorithm>
#include <functional>

namespace jp {

namespace {

QString say(const char *text) { return QCoreApplication::translate("Animation", text); }

constexpr int kBarWidth = 84;

// ---- pictures ----

// How a small shape stands halfway through an effect, on a slide of `size`.
struct MiniState {
    deck::TargetState state;
    QPainterPath path;      // a motion path: where it goes, on the slide
    bool lasts = false;     // it takes time (an appearing shape does not)
};

MiniState miniState(const deck::EffectKind &kind, const QString &variant, const QSizeF &size, const QRectF &box)
{
    MiniState out;
    deck::AnimSequence seq;
    deck::Anim a = deck::makeAnim(kind, 2);
    a.variant = variant;
    seq.add(a);
    Document doc;
    Page page;
    auto item = std::make_shared<ShapeItem>();
    item->sourceId = 2;
    item->rect = box;
    page.items.push_back(item);
    const deck::Timeline tl = deck::readTimeline(deck::animationsXml(doc, page, seq));
    if (tl.steps.isEmpty()) return out;
    deck::Geometry g;
    g.slide = size;
    g.shapes.insert(2, box);
    const deck::States states = deck::statesAt(tl, g, 0, tl.steps[0].duration() * 0.5);
    out.state = states.value(deck::Target{2});
    for (const deck::Effect &e : tl.steps[0].effects)
        for (const deck::Behavior &b : e.behaviors)
            if (b.kind == deck::Behavior::Kind::Motion && !b.path.isEmpty()) out.path = QTransform::fromScale(size.width(), size.height()).map(b.path);
    out.lasts = tl.steps[0].duration() >= 2;
    return out;
}

void paintShape(QPainter *p, const QRectF &box, const QColor &color)
{
    p->setPen(Qt::NoPen);
    p->setBrush(color);
    p->drawRoundedRect(box, box.height() * 0.18, box.height() * 0.18);
    // Something to see it turn by: a notch at the top left, and a line of "text".
    p->setBrush(QColor(255, 255, 255, 210));
    p->drawRect(QRectF(box.left() + box.width() * 0.14, box.top() + box.height() * 0.2, box.width() * 0.3, box.height() * 0.14));
    p->drawRect(QRectF(box.left() + box.width() * 0.14, box.top() + box.height() * 0.5, box.width() * 0.6, box.height() * 0.1));
}

QString shapeName(const Item &it) { return it.name.isEmpty() ? QCoreApplication::translate("Animation", "Shape %1").arg(it.sourceId) : it.name; }

// The first words of a paragraph of the shape's text.
QString paragraphText(const Document *d, const Item &it, int paragraph)
{
    QString story;
    if (it.type() == ItemType::Text) story = static_cast<const TextItem &>(it).storyId;
    else if (it.type() == ItemType::Shape) story = static_cast<const ShapeItem &>(it).storyId;
    const QTextDocument *doc = story.isEmpty() ? nullptr : d->storyDoc(story);
    if (!doc) return QString();
    const QString text = doc->findBlockByNumber(paragraph).text().simplified();
    return text.size() > 28 ? text.left(27) + QChar(0x2026) : text;
}

// A list that reorders by drag: the drop says where, and the rows are drawn again from the sequence.
class AnimList : public QListWidget {
public:
    using QListWidget::QListWidget;
    std::function<void(int, int)> moved;
    std::function<void(const QVector<int> &)> removed;

protected:
    void dropEvent(QDropEvent *e) override
    {
        const int from = currentRow();
        int to = indexAt(e->position().toPoint()).row();
        if (to < 0) to = count() - 1;
        e->ignore();
        if (from >= 0 && moved) moved(from, to);
    }
    void keyPressEvent(QKeyEvent *e) override
    {
        if (e->key() == Qt::Key_Delete || e->key() == Qt::Key_Backspace) {
            QVector<int> rows;
            for (const QModelIndex &i : selectionModel()->selectedRows()) rows << i.row();
            std::sort(rows.begin(), rows.end());
            if (!rows.isEmpty() && removed) removed(rows);
            return;
        }
        QListWidget::keyPressEvent(e);
    }
};

} // namespace

struct AnimationPane::Row {
    QString shape, effect, start;
    int click = 0, begin = 0, end = 0;
    deck::EffectClass cls = deck::EffectClass::Entrance;
    bool onClick = false, playing = false;
    QIcon icon;
};

namespace {

class RowDelegate : public QStyledItemDelegate {
public:
    RowDelegate(const QVector<AnimationPane::Row> *rows, const int *scale, QObject *parent) : QStyledItemDelegate(parent), m_rows(rows), m_scale(scale) {}
    QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const override { return QSize(100, 42); }

    void paint(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        const int row = index.row();
        if (row < 0 || row >= m_rows->size()) return;
        const AnimationPane::Row &d = (*m_rows)[row];
        p->save();
        p->setRenderHint(QPainter::Antialiasing);
        const QRect r = option.rect;
        const bool selected = option.state & QStyle::State_Selected;
        const QColor accent = uiAccent();
        if (selected) p->fillRect(r.adjusted(2, 1, -2, -1), QColor(accent.red(), accent.green(), accent.blue(), 46));
        else if (d.playing) p->fillRect(r.adjusted(2, 1, -2, -1), QColor(0xB6, 0x42, 0x0F, 30));
        p->setPen(QPen(uiLine(), 1));
        p->drawLine(r.bottomLeft() + QPoint(8, 0), r.bottomRight() - QPoint(8, 0));
        int x = r.left() + 8;
        const int mid = r.center().y();
        // The click it takes: a number for an effect that starts at a click; the others hang from the one before.
        const QRect badge(x, mid - 9, 20, 18);
        if (d.onClick) {
            p->setPen(Qt::NoPen);
            p->setBrush(accent);
            p->drawRoundedRect(badge, 5, 5);
            p->setPen(Qt::white);
            QFont f = option.font;
            f.setPixelSize(11);
            f.setBold(true);
            p->setFont(f);
            p->drawText(badge, Qt::AlignCenter, QString::number(d.click));
        } else {
            p->setPen(QPen(uiText(), 1.4));
            p->setBrush(Qt::NoBrush);
            p->drawLine(QPointF(x + 4, mid - 7), QPointF(x + 4, mid + 1));
            p->drawLine(QPointF(x + 4, mid + 1), QPointF(x + 12, mid + 1));
        }
        x += 26;
        d.icon.paint(p, QRect(x, mid - 12, 30, 24));
        x += 36;
        const int textRight = r.right() - kBarWidth - 14;
        QFont big = option.font;
        big.setBold(true);
        QFont small = option.font;
        small.setPointSizeF(std::max(7.0, option.font.pointSizeF() - 1.5));
        const QFontMetrics fm(big), sm(small);
        p->setPen(uiText());
        p->setFont(big);
        p->drawText(QRect(x, r.top() + 4, textRight - x, fm.height()), Qt::AlignLeft | Qt::AlignVCenter, fm.elidedText(d.shape, Qt::ElideRight, textRight - x));
        p->setFont(small);
        QColor dim = uiText();
        dim.setAlpha(150);
        p->setPen(dim);
        p->drawText(QRect(x, r.top() + 4 + fm.height(), textRight - x, sm.height() + 2), Qt::AlignLeft | Qt::AlignVCenter,
                    sm.elidedText(d.effect + QStringLiteral(" · ") + d.start, Qt::ElideRight, textRight - x));
        // When it plays after its click, against the longest of the slide's.
        const QRect track(r.right() - kBarWidth - 8, mid - 4, kBarWidth, 8);
        p->setPen(Qt::NoPen);
        QColor trackColor = uiLine();
        trackColor.setAlpha(120);
        p->setBrush(trackColor);
        p->drawRoundedRect(track, 4, 4);
        const double span = std::max(1, *m_scale);
        const double from = std::clamp(d.begin / span, 0.0, 1.0), to = std::clamp(d.end / span, 0.0, 1.0);
        p->setBrush(effectColor(d.cls));
        p->drawRoundedRect(QRectF(track.left() + from * track.width(), track.top(), std::max(4.0, (to - from) * track.width()), track.height()), 4, 4);
        p->restore();
    }

private:
    const QVector<AnimationPane::Row> *m_rows;
    const int *m_scale;
};

} // namespace

QColor effectColor(deck::EffectClass cls)
{
    switch (cls) {
    case deck::EffectClass::Entrance: return QColor(0x3B, 0x8F, 0x4E);
    case deck::EffectClass::Emphasis: return QColor(0xC4, 0x8A, 0x00);
    case deck::EffectClass::Exit: return QColor(0xC0, 0x39, 0x2B);
    case deck::EffectClass::MotionPath: return QColor(0x3A, 0x6E, 0xA5);
    default: return QColor(0x80, 0x84, 0x8C);
    }
}

QString effectClassLabel(deck::EffectClass cls)
{
    switch (cls) {
    case deck::EffectClass::Entrance: return say("Entrance");
    case deck::EffectClass::Emphasis: return say("Emphasis");
    case deck::EffectClass::Exit: return say("Exit");
    case deck::EffectClass::MotionPath: return say("Motion Path");
    default: return say("Other");
    }
}

QString startLabel(deck::EffectStart start)
{
    return start == deck::EffectStart::WithPrevious ? say("With Previous") : start == deck::EffectStart::AfterPrevious ? say("After Previous") : say("On Click");
}

QString effectLabel(const deck::Anim &a)
{
    const deck::EffectKind *kind = a.custom() ? nullptr : deck::effectKind(a.kind);
    if (!kind) return say("Custom");
    const deck::EffectVariant *v = kind->variant(a.variant);
    const bool named = v && kind->variants.size() > 1 && kind->amount == deck::EffectKind::Amount::None && (kind->cls == deck::EffectClass::MotionPath || v != &kind->variants.first());
    return named ? kind->name + QStringLiteral(": ") + v->name : kind->name;
}

namespace {

constexpr double kNameStrip = 12;   // the height of the name under a gallery picture

// An effect's name, centered in `strip`, in a size that fits the width (and cut with an ellipsis if even the smallest does not).
void paintName(QPainter *p, const QRectF &strip, const QString &name)
{
    QFont font = QGuiApplication::font();
    int size = 10;
    font.setPixelSize(size);
    QFontMetricsF fm(font);
    while (size > 7 && fm.horizontalAdvance(name) > strip.width() - 2) {
        font.setPixelSize(--size);
        fm = QFontMetricsF(font);
    }
    p->save();
    p->setFont(font);
    p->setPen(uiDark() ? QColor(0xE4, 0xE6, 0xEA) : QColor(0x2B, 0x2F, 0x36));
    p->drawText(strip, Qt::AlignCenter, fm.elidedText(name, Qt::ElideRight, strip.width() - 2));
    p->restore();
}

} // namespace

QIcon noEffectIcon()
{
    const QString name = QCoreApplication::translate("Animation", "None");
    return drawnIcon([name](QPainter *p, const QRectF &rc) {
        const QRectF picture = rc.adjusted(0, 0, 0, -kNameStrip);
        p->setRenderHint(QPainter::Antialiasing);
        p->setPen(QPen(QColor(0x80, 0x84, 0x8C), 1.4));
        p->setBrush(Qt::NoBrush);
        p->drawRoundedRect(picture.adjusted(1, 1, -1, -1), 3, 3);
        p->drawLine(picture.topLeft() + QPointF(picture.width() * 0.3, picture.height() * 0.3), picture.bottomRight() - QPointF(picture.width() * 0.3, picture.height() * 0.3));
        paintName(p, QRectF(rc.left(), rc.bottom() - kNameStrip, rc.width(), kNameStrip), name);
    });
}

QIcon effectIcon(const deck::EffectKind &kindIn, const QString &variantIn, bool named)
{
    const deck::EffectKind kind = kindIn;
    const QString variant = variantIn.isEmpty() ? kind.firstVariant().key : variantIn;
    return drawnIcon([kind, variant, named](QPainter *p, const QRectF &rcFull) {
        // Named, the picture is above a strip that holds the name.
        const QRectF rc = named ? rcFull.adjusted(0, 0, 0, -kNameStrip) : rcFull;
        if (named) paintName(p, QRectF(rcFull.left(), rcFull.bottom() - kNameStrip, rcFull.width(), kNameStrip), kind.name);
        p->save();
        p->translate(rc.topLeft());
        p->setRenderHint(QPainter::Antialiasing);
        const double w = rc.width(), h = rc.height();
        const QColor color = effectColor(kind.cls);
        // The slide, with a border in the class's color.
        const QRectF slide(1, 1, w - 2, h - 2);
        p->setPen(QPen(color, 1.4));
        p->setBrush(uiDark() ? QColor(0x2A, 0x2D, 0x33) : QColor(0xF6, 0xF7, 0xF9));
        p->drawRoundedRect(slide, 3, 3);
        const QRectF at(w * 0.34 - 1, h * 0.32 - 1, w * 0.32, h * 0.36);
        const MiniState m = miniState(kind, variant, slide.size(), at);
        p->translate(1, 1);
        if (kind.cls == deck::EffectClass::MotionPath && !m.path.isEmpty()) {
            // The path, as large as the picture holds, from a green dot to a red one.
            const QRectF b = m.path.boundingRect();
            const double s = std::min((slide.width() - 10) / std::max(b.width(), 1e-3), (slide.height() - 10) / std::max(b.height(), 1e-3));
            QTransform t;
            t.translate(slide.width() / 2 - b.center().x() * s, slide.height() / 2 - b.center().y() * s);
            t.scale(s, s);
            p->setPen(QPen(color, 1.5, Qt::DashLine));
            p->setBrush(Qt::NoBrush);
            p->drawPath(t.map(m.path));
            p->setPen(Qt::NoPen);
            p->setBrush(QColor(0x3B, 0x8F, 0x4E));
            p->drawEllipse(t.map(m.path.pointAtPercent(0)), 2.4, 2.4);
            p->setBrush(QColor(0xC0, 0x39, 0x2B));
            p->drawEllipse(t.map(m.path.pointAtPercent(1)), 2.4, 2.4);
        } else {
            // Where the shape is, dotted, and the shape halfway through.
            const bool disappears = !m.lasts && kind.cls == deck::EffectClass::Exit;
            if (m.lasts || disappears) {
                p->setPen(QPen(color, 1, Qt::DotLine));
                p->setBrush(Qt::NoBrush);
                p->drawRoundedRect(at, at.height() * 0.18, at.height() * 0.18);
            }
            const deck::TargetState &s = m.state;
            if (!disappears && s.visible) {
                p->save();
                QTransform t;
                t.translate(at.center().x() + s.offset.x(), at.center().y() + s.offset.y());
                t.rotate(s.rotation);
                t.scale(s.scaleX, s.scaleY);
                t.translate(-at.center().x(), -at.center().y());
                p->setTransform(t, true);
                if (!s.clip.isNull()) {
                    auto part = [&](const QRectF &q) { return QRectF(at.x() + at.width() * q.x(), at.y() + at.height() * q.y(), at.width() * q.width(), at.height() * q.height()); };
                    QPainterPath parts;
                    parts.addRect(part(s.clip));
                    if (!s.clip2.isNull()) parts.addRect(part(s.clip2));
                    p->setClipPath(parts, Qt::IntersectClip);
                }
                p->setOpacity(std::clamp(s.opacity, 0.15, 1.0));
                paintShape(p, at, color);
                p->restore();
            }
        }
        p->restore();
    });
}

// ---- the pane ----

AnimationPane::AnimationPane(Editor *ed, QWidget *parent) : QWidget(parent), m_ed(ed)
{
    setObjectName(QStringLiteral("animationPane"));
    setAccessibleName(tr("Animation Pane"));
    auto *column = new QVBoxLayout(this);
    column->setContentsMargins(8, 8, 8, 8);
    auto *play = new QHBoxLayout();
    m_playAll = new QPushButton(icon("play"), tr("Play All"), this);
    m_playAll->setObjectName(QStringLiteral("animPlayAll"));
    m_playAll->setToolTip(tr("Play all the effects of this slide"));
    m_playFrom = new QPushButton(icon("play"), tr("Play From"), this);
    m_playFrom->setObjectName(QStringLiteral("animPlayFrom"));
    m_playFrom->setToolTip(tr("Play the effects from the one picked"));
    play->addWidget(m_playAll);
    play->addWidget(m_playFrom);
    column->addLayout(play);

    auto *list = new AnimList(this);
    m_list = list;
    m_list->setObjectName(QStringLiteral("animList"));
    m_list->setAccessibleName(tr("Effects of the slide"));
    m_list->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_list->setDragDropMode(QAbstractItemView::InternalMove);
    m_list->setDefaultDropAction(Qt::MoveAction);
    m_list->setDropIndicatorShown(true);
    m_list->setContextMenuPolicy(Qt::CustomContextMenu);
    m_list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_list->setFrameShape(QFrame::NoFrame);
    m_list->setItemDelegate(new RowDelegate(&m_rows, &m_scale, m_list));
    column->addWidget(m_list, 1);

    m_empty = new QLabel(tr("No animation on this slide. Pick a shape, and an effect on the Animations tab."), m_list->viewport());
    m_empty->setObjectName(QStringLiteral("animEmpty"));
    m_empty->setWordWrap(true);
    m_empty->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_empty->setContentsMargins(10, 10, 10, 10);
    m_list->viewport()->installEventFilter(this);

    auto *order = new QHBoxLayout();
    order->addWidget(new QLabel(tr("Re-Order"), this));
    order->addStretch(1);
    m_earlier = new QPushButton(icon("arrow-up"), QString(), this);
    m_earlier->setObjectName(QStringLiteral("animEarlier"));
    m_earlier->setToolTip(tr("Move the effect earlier"));
    m_earlier->setAccessibleName(tr("Move Earlier"));
    m_later = new QPushButton(icon("arrow-down"), QString(), this);
    m_later->setObjectName(QStringLiteral("animLater"));
    m_later->setToolTip(tr("Move the effect later"));
    m_later->setAccessibleName(tr("Move Later"));
    order->addWidget(m_earlier);
    order->addWidget(m_later);
    column->addLayout(order);

    connect(m_playAll, &QPushButton::clicked, this, [this] { Q_EMIT playRequested(-1); });
    connect(m_playFrom, &QPushButton::clicked, this, [this] {
        const QVector<int> rows = selectedRows();
        Q_EMIT playRequested(rows.isEmpty() ? -1 : rows.first());
    });
    connect(m_earlier, &QPushButton::clicked, this, [this] { Q_EMIT reorderRequested(-1); });
    connect(m_later, &QPushButton::clicked, this, [this] { Q_EMIT reorderRequested(1); });
    list->moved = [this](int from, int to) { Q_EMIT moveRequested(from, to); };
    list->removed = [this](const QVector<int> &rows) { Q_EMIT removeRequested(rows); };
    connect(m_list, &QListWidget::itemSelectionChanged, this, [this] {
        const QVector<int> rows = selectedRows();
        m_playFrom->setEnabled(!rows.isEmpty());
        m_earlier->setEnabled(!rows.isEmpty() && rows.first() > 0);
        m_later->setEnabled(!rows.isEmpty() && rows.last() < rowCount() - 1);
        if (!m_quiet) Q_EMIT rowsChosen(rows);
    });
    connect(m_list, &QListWidget::customContextMenuRequested, this, [this](const QPoint &at) {
        const QModelIndex i = m_list->indexAt(at);
        if (!i.isValid()) return;
        if (!m_list->selectionModel()->isSelected(i)) m_list->setCurrentRow(i.row());
        Q_EMIT menuRequested(i.row(), m_list->viewport()->mapToGlobal(at));
    });
    connect(m_list, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *it) { Q_EMIT optionsRequested(m_list->row(it)); });
    connect(ed, &Editor::changed, this, &AnimationPane::reload);
    connect(ed, &Editor::pageChanged, this, &AnimationPane::reload);
    connect(ed, &Editor::documentReplaced, this, &AnimationPane::reload);
    reload();
}

AnimationPane::~AnimationPane()
{
    // The list clears itself as it goes, and says its selection changed: nobody is left to hear it.
    m_quiet = true;
    m_list->disconnect(this);
}

bool AnimationPane::eventFilter(QObject *o, QEvent *e)
{
    if (o == m_list->viewport() && e->type() == QEvent::Resize) m_empty->setGeometry(m_list->viewport()->rect());
    return QWidget::eventFilter(o, e);
}

int AnimationPane::rowCount() const { return m_list->count(); }

QVector<int> AnimationPane::selectedRows() const
{
    QVector<int> rows;
    for (const QModelIndex &i : m_list->selectionModel()->selectedRows()) rows << i.row();
    std::sort(rows.begin(), rows.end());
    return rows;
}

void AnimationPane::selectRows(const QVector<int> &rows)
{
    const bool was = m_quiet;
    m_quiet = true;
    m_list->clearSelection();
    for (int r : rows)
        if (r >= 0 && r < m_list->count()) m_list->item(r)->setSelected(true);
    if (!rows.isEmpty() && rows.first() < m_list->count()) m_list->setCurrentRow(rows.first(), QItemSelectionModel::NoUpdate);
    m_quiet = was;
}

void AnimationPane::selectShapes(const QSet<int> &numbers)
{
    QVector<int> rows;
    for (int i = 0; i < m_seq.effects.size(); ++i)
        if (numbers.contains(m_seq.effects[i].shape)) rows << i;
    selectRows(rows);
}

const deck::Anim *AnimationPane::effect(int row) const { return row >= 0 && row < m_seq.effects.size() ? &m_seq.effects[row] : nullptr; }

void AnimationPane::setPlaying(int row)
{
    for (int i = 0; i < m_rows.size(); ++i) m_rows[i].playing = i == row;
    m_list->viewport()->update();
}

void AnimationPane::reload()
{
    const Document *d = m_ed->doc();
    const int slide = m_ed->currentPage();
    if (!d || slide < 0 || slide >= d->pages.size()) return;
    const Page &page = *d->pages[slide];
    // The rows show the shapes' names, so they are drawn again when those or the animations are not what they show.
    const quint64 signature = deck::animationSignature(*d, page);
    if (page.id == m_page && signature == m_signature) return;
    m_page = page.id;
    m_signature = signature;
    m_seq = signature ? deck::readAnimations(page, d) : deck::AnimSequence();
    rebuild();
}

void AnimationPane::rebuild()
{
    const Document *d = m_ed->doc();
    const Page &page = *d->pages[m_ed->currentPage()];
    const QVector<int> picked = selectedRows();
    QHash<int, const Item *> shapes;
    walkItems(page.items, [&](const ItemPtr &it) {
        if (it->sourceId > 0 && !shapes.contains(it->sourceId)) shapes.insert(it->sourceId, it.get());
    });
    m_rows.clear();
    const bool was = m_quiet;
    m_quiet = true;
    m_list->clear();
    int longest = 1000;
    for (int i = 0; i < m_seq.effects.size(); ++i) {
        const deck::Anim &a = m_seq.effects[i];
        Row r;
        const Item *it = shapes.value(a.shape, nullptr);
        const QString name = it ? shapeName(*it) : tr("Shape %1").arg(a.shape);
        // A paragraph effect is named by its paragraph, as the shape's name and the effect are on the second line.
        const QString text = it && a.paragraphs() ? paragraphText(d, *it, a.firstParagraph) : QString();
        r.shape = text.isEmpty() ? name : text;
        r.effect = effectLabel(a);   // its class shows in the picture's color, and in the tip
        r.start = startLabel(a.start);
        r.click = m_seq.clickOf(i);
        r.begin = m_seq.beginOf(i);
        r.end = m_seq.endOf(i);
        r.cls = a.cls;
        r.onClick = a.start == deck::EffectStart::OnClick;
        const deck::EffectKind *kind = a.custom() ? nullptr : deck::effectKind(a.kind);
        if (kind) {
            r.icon = effectIcon(*kind, a.variant);
        } else {
            const deck::EffectClass cls = a.cls;
            r.icon = drawnIcon([cls](QPainter *p, const QRectF &rc) {
                p->setRenderHint(QPainter::Antialiasing);
                p->setPen(QPen(effectColor(cls), 1.4));
                p->setBrush(Qt::NoBrush);
                p->drawRoundedRect(rc.adjusted(1, 1, -1, -1), 3, 3);
                p->setPen(effectColor(cls));
                p->drawText(rc, Qt::AlignCenter, QStringLiteral("?"));
            });
        }
        longest = std::max(longest, r.end);
        m_rows.push_back(r);
        auto *item = new QListWidgetItem(QStringLiteral("%1. %2, %3: %4, %5").arg(r.click).arg(r.shape, effectClassLabel(a.cls), r.effect, r.start), m_list);
        item->setData(Qt::AccessibleTextRole, item->text());
        item->setToolTip(QStringLiteral("%1\n%2\n%3").arg(name, effectClassLabel(a.cls) + QStringLiteral(": ") + effectLabel(a), r.start));
    }
    m_scale = std::min(longest, 30000);
    for (int r : picked)
        if (r < m_list->count()) m_list->item(r)->setSelected(true);
    m_quiet = was;
    m_empty->setVisible(m_seq.effects.isEmpty());
    m_empty->setGeometry(m_list->viewport()->rect());
    m_playAll->setEnabled(!m_seq.effects.isEmpty());
    m_playFrom->setEnabled(!selectedRows().isEmpty());
    m_list->viewport()->update();
}

// ---- the preview ----

AnimationPreview::AnimationPreview(const Document *doc, LayoutCache *cache, QWidget *parent) : QWidget(parent), m_doc(doc), m_cache(cache)
{
    setObjectName(QStringLiteral("animationPreview"));
    setFocusPolicy(Qt::StrongFocus);
    m_timer.setInterval(16);
    connect(&m_timer, &QTimer::timeout, this, &AnimationPreview::tick);
    hide();
}

bool AnimationPreview::play(int slide, const QRect &area, int fromStep)
{
    if (slide < 0 || slide >= m_doc->pages.size()) return false;
    const Page &page = *m_doc->pages[slide];
    m_timeline = deck::playableTimeline(*m_doc, page);
    if (m_timeline.steps.isEmpty()) return false;
    m_geometry = deck::geometryOf(*m_doc, page);
    m_slide = slide;
    m_area = area;
    m_first = std::clamp(fromStep, 0, int(m_timeline.steps.size()) - 1);
    m_total = 0;
    for (int k = m_first; k < m_timeline.steps.size(); ++k) m_total += m_timeline.steps[k].duration();
    m_step = -1;
    setGeometry(parentWidget()->rect());
    raise();
    show();
    setFocus();
    m_clock.start();
    m_timer.start();
    tick();
    return true;
}

void AnimationPreview::tick()
{
    // A moment after the last step ends, it is over.
    const double ms = m_clock.nsecsElapsed() / 1e6;
    if (ms > m_total + 250) {
        stop();
        return;
    }
    int step = m_first;
    double at = 0;
    for (int k = m_first; k < m_timeline.steps.size(); ++k) {
        const double length = m_timeline.steps[k].duration();
        step = k;
        if (ms < at + length || k == m_timeline.steps.size() - 1) break;
        at += length;
    }
    const double inside = std::clamp(ms - at, 0.0, m_timeline.steps[step].duration());
    if (step != m_step) {
        m_step = step;
        Q_EMIT stepChanged(step);
    }
    const deck::States states = deck::statesAt(m_timeline, m_geometry, step, inside);
    m_frame = renderSlideImage(m_doc, m_cache, m_slide, m_area.size(), devicePixelRatioF(), Qt::white, &states);
    update();
}

void AnimationPreview::stop()
{
    if (!m_timer.isActive() && !isVisible()) return;
    m_timer.stop();
    m_step = -1;
    hide();
    Q_EMIT stepChanged(-1);
    Q_EMIT finished();
}

void AnimationPreview::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape) stop();
    else QWidget::keyPressEvent(e);
}

void AnimationPreview::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    if (!m_frame.isNull()) p.drawImage(m_area.topLeft(), m_frame);
}

} // namespace jp
