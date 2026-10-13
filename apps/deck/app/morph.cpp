#include "app/morph.h"

#include "app/slideshow.h"
#include "core/media.h"
#include "core/transition.h"
#include "render/renderer.h"

#include <QHash>
#include <QPainter>
#include <QSet>
#include <QTextDocument>
#include <algorithm>
#include <cmath>

namespace jp {

namespace {

// ---- numbers ----

double lerp(double a, double b, double t) { return a + (b - a) * t; }
QPointF lerp(const QPointF &a, const QPointF &b, double t) { return QPointF(lerp(a.x(), b.x(), t), lerp(a.y(), b.y(), t)); }
QSizeF lerp(const QSizeF &a, const QSizeF &b, double t) { return QSizeF(lerp(a.width(), b.width(), t), lerp(a.height(), b.height(), t)); }
QMarginsF lerp(const QMarginsF &a, const QMarginsF &b, double t)
{
    return QMarginsF(lerp(a.left(), b.left(), t), lerp(a.top(), b.top(), t), lerp(a.right(), b.right(), t), lerp(a.bottom(), b.bottom(), t));
}

// An angle on the way from one to another, the shorter way round.
double turned(double from, double to, double t)
{
    double d = std::fmod(to - from, 360.0);
    if (d > 180) d -= 360;
    else if (d < -180) d += 360;
    return from + d * t;
}

QColor mixed(const QColor &a, const QColor &b, double t)
{
    return QColor::fromRgbF(float(lerp(a.redF(), b.redF(), t)), float(lerp(a.greenF(), b.greenF(), t)), float(lerp(a.blueF(), b.blueF(), t)),
                            float(lerp(a.alphaF(), b.alphaF(), t)));
}

// ---- what an object is ----

QString storyIdOf(const Item &it)
{
    if (it.type() == ItemType::Text) return static_cast<const TextItem &>(it).storyId;
    if (it.type() == ItemType::Shape) return static_cast<const ShapeItem &>(it).storyId;
    return QString();
}

QString textOf(const Document &doc, const Item &it)
{
    if (it.type() == ItemType::TextArt) return static_cast<const TextArtItem &>(it).text;
    const QTextDocument *story = doc.storyDoc(storyIdOf(it));
    return story ? story->toPlainText() : QString();
}

// What the story looks like, for telling whether two are alike: its text with its fonts, sizes, and colors.
QString storyLook(const Document &doc, const Item &it)
{
    if (it.type() == ItemType::TextArt) return static_cast<const TextArtItem &>(it).text;
    const QTextDocument *story = doc.storyDoc(storyIdOf(it));
    return story ? story->toHtml() : QString();
}

bool hasStory(const Document &doc, const Item &it) { return !storyIdOf(it).isEmpty() && doc.storyDoc(storyIdOf(it)); }

// The key two objects of one kind of shape share (and none for the kinds that are matched by name, text, or picture only).
QString shapeKey(const Item &it)
{
    switch (it.type()) {
    case ItemType::Shape: {
        const auto &s = static_cast<const ShapeItem &>(it);
        if (s.isArt() || !s.customPath.isEmpty()) return QString();
        return QStringLiteral("shape|") + s.shape;
    }
    case ItemType::Line: return QStringLiteral("line");
    case ItemType::Table: return QStringLiteral("table");
    default: return QString();
    }
}

// ---- the pairs ----

constexpr qsizetype kMaxCandidates = 4096;   // the pairs of objects of one kind that are weighed for the nearest; more are paired in order

class Pairing {
public:
    Pairing(const Document &doc, const ItemList &from, const ItemList &to)
        : m_doc(doc), m_from(from), m_to(to), m_fromUsed(from.size(), 0), m_toUsed(to.size(), 0)
    {
    }

    MorphPlan run()
    {
        using Rule = MorphPair::Rule;
        pairBy(Rule::Forced, [](const Document &, const Item &it) { return it.name.startsWith(QLatin1String("!!")) ? it.name : QString(); });
        pairBy(Rule::Name, [](const Document &, const Item &it) {
            return it.name.isEmpty() ? QString() : QString::number(int(it.type())) + QLatin1Char('|') + it.name;
        });
        pairBy(Rule::Text, [](const Document &doc, const Item &it) {
            if (it.type() != ItemType::Text && it.type() != ItemType::Shape && it.type() != ItemType::TextArt) return QString();
            const QString text = textOf(doc, it).simplified();
            return text.isEmpty() ? QString() : QString::number(int(it.type())) + QLatin1Char('|') + text;
        });
        pairBy(Rule::Picture, [](const Document &, const Item &it) {
            if (it.type() != ItemType::Picture) return QString();
            return static_cast<const PictureItem &>(it).imageId;
        });
        pairBy(Rule::Shape, [](const Document &, const Item &it) { return shapeKey(it); });
        MorphPlan plan;
        plan.pairs = m_made;
        std::sort(plan.pairs.begin(), plan.pairs.end(), [](const MorphPair &a, const MorphPair &b) { return a.to < b.to; });
        for (qsizetype i = 0; i < m_from.size(); ++i)
            if (!m_fromUsed[i]) plan.leaving << int(i);
        for (qsizetype j = 0; j < m_to.size(); ++j)
            if (!m_toUsed[j]) plan.arriving << int(j);
        return plan;
    }

private:
    template <typename KeyOf>
    void pairBy(MorphPair::Rule rule, KeyOf keyOf)
    {
        QHash<QString, QVector<int>> a, b;
        for (qsizetype i = 0; i < m_from.size(); ++i)
            if (!m_fromUsed[i])
                if (const QString key = keyOf(m_doc, *m_from[i]); !key.isEmpty()) a[key].push_back(int(i));
        if (a.isEmpty()) return;
        for (qsizetype j = 0; j < m_to.size(); ++j)
            if (!m_toUsed[j])
                if (const QString key = keyOf(m_doc, *m_to[j]); !key.isEmpty() && a.contains(key)) b[key].push_back(int(j));
        // Each object has one key, so the order of the keys makes no difference.
        for (auto it = b.constBegin(); it != b.constEnd(); ++it) nearest(a.value(it.key()), it.value(), rule);
    }

    void make(int i, int j, MorphPair::Rule rule)
    {
        m_fromUsed[i] = m_toUsed[j] = 1;
        MorphPair p;
        p.from = i;
        p.to = j;
        p.rule = rule;
        m_made << p;
    }

    // The pairs among the objects `as` of the old slide and `bs` of the new one, nearest first.
    void nearest(const QVector<int> &as, const QVector<int> &bs, MorphPair::Rule rule)
    {
        if (as.size() * bs.size() > kMaxCandidates) {
            for (qsizetype k = 0; k < std::min(as.size(), bs.size()); ++k) make(as[k], bs[k], rule);
            return;
        }
        struct Candidate { double distance; int i, j; };
        QVector<Candidate> all;
        all.reserve(as.size() * bs.size());
        for (const int i : as)
            for (const int j : bs) {
                const QPointF d = m_from[i]->bounds().center() - m_to[j]->bounds().center();
                all.push_back({std::hypot(d.x(), d.y()), i, j});
            }
        std::sort(all.begin(), all.end(), [](const Candidate &x, const Candidate &y) {
            if (x.distance != y.distance) return x.distance < y.distance;
            return x.i != y.i ? x.i < y.i : x.j < y.j;
        });
        for (const Candidate &c : all)
            if (!m_fromUsed[c.i] && !m_toUsed[c.j]) make(c.i, c.j, rule);
    }

    const Document &m_doc;
    const ItemList &m_from, &m_to;
    QVector<char> m_fromUsed, m_toUsed;
    QVector<MorphPair> m_made;
};

// ---- what a slide shows ----

// The slide's own objects as the states have them: those that are hidden left out, those that animation moved, scaled, or turned
// (and left so) as copies where they ended up. `opacity` gets the share each object shows.
ItemList shownObjects(const Page &page, const deck::States *states, QVector<double> *opacity)
{
    ItemList out;
    const QSet<QString> offScreen = deck::mediaHiddenWhenStopped(page);
    for (const ItemPtr &it : page.items) {
        if (offScreen.contains(it->id)) continue;
        double share = 1;
        ItemPtr use = it;
        if (states && it->sourceId > 0) {
            const auto found = states->constFind(deck::Target{it->sourceId, -1, -1});
            if (found != states->constEnd()) {
                const deck::TargetState &s = found.value();
                if (!s.visible || s.opacity <= 0 || s.scaleX == 0 || s.scaleY == 0) continue;
                share = std::min(1.0, s.opacity);
                if (!s.offset.isNull() || s.scaleX != 1 || s.scaleY != 1 || s.rotation != 0) {
                    use = it->clone();
                    const QPointF middle = it->bounds().center();
                    if (it->type() == ItemType::Group || it->type() == ItemType::Line) {
                        use->moveBy(s.offset.x(), s.offset.y());
                        if (s.rotation != 0) use->rotateAround(s.rotation, middle + s.offset);
                    } else {
                        const QSizeF size(it->rect.width() * s.scaleX, it->rect.height() * s.scaleY);
                        use->rect = QRectF(middle + s.offset - QPointF(size.width() / 2, size.height() / 2), size);
                        use->rotation = it->rotation + s.rotation;
                    }
                }
            }
        }
        out.push_back(use);
        if (opacity) opacity->push_back(share);
    }
    return out;
}

// ---- how one object becomes another ----

// Where an object is: its middle, size, and turn. A group or a line is its bounds, unturned.
struct Frame {
    QPointF middle;
    QSizeF size;
    double turn = 0;
};

Frame frameOf(const Item &it)
{
    const bool bounded = it.type() == ItemType::Group || it.type() == ItemType::Line;
    const QRectF r = bounded ? it.bounds() : it.rect;
    return Frame{r.center(), r.size(), bounded ? 0 : it.rotation};
}

Frame lerp(const Frame &a, const Frame &b, double t) { return Frame{lerp(a.middle, b.middle, t), lerp(a.size, b.size, t), turned(a.turn, b.turn, t)}; }

QRectF rectOf(const Frame &f) { return QRectF(f.middle - QPointF(f.size.width() / 2, f.size.height() / 2), f.size); }

// What turns the drawing of an object in frame `from` into the same drawing in frame `to`: about its middle, along its own axes.
QTransform carrying(const Frame &from, const Frame &to)
{
    auto share = [](double now, double then) { return now < 0.01 ? 1.0 : std::clamp(then / now, 0.02, 50.0); };
    QTransform m;
    m.translate(to.middle.x(), to.middle.y());
    m.rotate(to.turn);
    m.scale(share(from.size.width(), to.size.width()), share(from.size.height(), to.size.height()));
    m.rotate(-from.turn);
    m.translate(-from.middle.x(), -from.middle.y());
    return m;
}

bool fillsMix(const Fill &a, const Fill &b)
{
    auto plain = [](const Fill &f) { return f.type == Fill::NoFill || f.type == Fill::Solid; };
    if (plain(a) && plain(b)) return true;
    return a.type == Fill::Gradient && b.type == Fill::Gradient && a.gradType == b.gradType && a.stops.size() == b.stops.size();
}

// The fill part of the way from one to another. A fill that is none is the other's color, clear.
Fill mixFill(const Fill &a, const Fill &b, double t, const ColorScheme &cs)
{
    if (a.type == Fill::Gradient && b.type == Fill::Gradient) {
        Fill r = b;
        r.angle = turned(a.angle, b.angle, t);
        r.transparency = lerp(a.transparency, b.transparency, t);
        r.color = ColorRef::rgb(mixed(a.color.resolve(cs), b.color.resolve(cs), t));
        r.color2 = ColorRef::rgb(mixed(a.color2.resolve(cs), b.color2.resolve(cs), t));
        for (qsizetype i = 0; i < r.stops.size(); ++i) {
            r.stops[i].pos = lerp(a.stops[i].pos, b.stops[i].pos, t);
            r.stops[i].color = ColorRef::rgb(mixed(a.stops[i].color.resolve(cs), b.stops[i].color.resolve(cs), t));
            r.stops[i].transparency = lerp(a.stops[i].transparency, b.stops[i].transparency, t);
        }
        return r;
    }
    const bool an = a.isNone(), bn = b.isNone();
    if (an && bn) return b;
    const QColor ca = (an ? b : a).color.resolve(cs), cb = (bn ? a : b).color.resolve(cs);
    return Fill::solid(ColorRef::rgb(mixed(ca, cb, t)), lerp(an ? 1.0 : a.transparency, bn ? 1.0 : b.transparency, t));
}

// The line part of the way: its color, weight, and clearness; the style of the nearer.
Stroke mixStroke(const Stroke &a, const Stroke &b, double t, const ColorScheme &cs)
{
    Stroke r = t < 0.5 ? a : b;
    const bool an = a.isNone(), bn = b.isNone();
    if (an && bn) return r;
    const Stroke &ofA = an ? b : a, &ofB = bn ? a : b;
    r.color = ColorRef::rgb(mixed(ofA.color.resolve(cs), ofB.color.resolve(cs), t));
    r.width = lerp(ofA.width, ofB.width, t);
    r.transparency = lerp(an ? 1.0 : a.transparency, bn ? 1.0 : b.transparency, t);
    return r;
}

// Whether two objects can be one object that changes (see the top of morph.h).
bool alike(const Item &a, const Item &b)
{
    if (a.type() != b.type() || a.flipH != b.flipH || a.flipV != b.flipV || !fillsMix(a.fill, b.fill)) return false;
    switch (a.type()) {
    case ItemType::Shape: {
        const auto &x = static_cast<const ShapeItem &>(a), &y = static_cast<const ShapeItem &>(b);
        return x.shape == y.shape && x.customPath == y.customPath && x.adj.size() == y.adj.size();
    }
    case ItemType::Text: return static_cast<const TextItem &>(a).vertical == static_cast<const TextItem &>(b).vertical;
    case ItemType::Picture: {
        const auto &x = static_cast<const PictureItem &>(a), &y = static_cast<const PictureItem &>(b);
        return !x.imageId.isEmpty() && x.imageId == y.imageId && x.maskShape == y.maskShape && x.brightness == y.brightness && x.contrast == y.contrast &&
               x.recolor == y.recolor && x.recolorColor == y.recolorColor && x.hasTransparentColor == y.hasTransparentColor &&
               x.transparentColor == y.transparentColor && x.blackWhiteLevel == y.blackWhiteLevel && x.blipEffects == y.blipEffects && x.mediaKind == y.mediaKind;
    }
    case ItemType::Line: {
        const auto &x = static_cast<const LineItem &>(a), &y = static_cast<const LineItem &>(b);
        return x.route == LineItem::Straight && y.route == LineItem::Straight;
    }
    case ItemType::TextArt: {
        const auto &x = static_cast<const TextArtItem &>(a), &y = static_cast<const TextArtItem &>(b);
        return x.text == y.text && x.font == y.font && x.bold == y.bold && x.italic == y.italic && x.size == y.size && x.evenHeight == y.evenHeight &&
               x.vertical == y.vertical && x.spacing == y.spacing && x.align == y.align && x.transform_ == y.transform_ && x.transformAdj == y.transformAdj;
    }
    default: return false;
    }
}

// The copy of an object that paints only its text: no fill, no effects, and a line that is clear (the text keeps its place inside it).
ItemPtr textOnly(const Item &src)
{
    ItemPtr c = src.clone();
    c->fill = Fill::none();
    c->stroke.transparency = 1;
    c->fx = Effects();
    return c;
}

void setStory(Item &it, const QString &id)
{
    if (it.type() == ItemType::Text) static_cast<TextItem &>(it).storyId = id;
    else if (it.type() == ItemType::Shape) static_cast<ShapeItem &>(it).storyId = id;
}

// Puts `m` (a copy of `a` or `b`) where the two are a share `t` of the way: the frame and the parts of it that are not looks.
void place(Item &m, const Item &a, const Item &b, double t)
{
    if (m.type() == ItemType::Line) {
        auto &l = static_cast<LineItem &>(m);
        const auto &x = static_cast<const LineItem &>(a), &y = static_cast<const LineItem &>(b);
        l.p1 = lerp(x.p1, y.p1, t);
        l.p2 = lerp(x.p2, y.p2, t);
        l.start = l.end = LineItem::Glue();
        l.syncRect();
        return;
    }
    const Frame f = lerp(frameOf(a), frameOf(b), t);
    m.rect = rectOf(f);
    m.rotation = f.turn;
    if (m.type() == ItemType::Shape) {
        auto &s = static_cast<ShapeItem &>(m);
        const auto &x = static_cast<const ShapeItem &>(a), &y = static_cast<const ShapeItem &>(b);
        s.insets = lerp(x.insets, y.insets, t);
        if (x.adj.size() == y.adj.size() && x.adj.size() == s.adj.size())
            for (qsizetype i = 0; i < s.adj.size(); ++i) s.adj[i] = lerp(x.adj[i], y.adj[i], t);
    } else if (m.type() == ItemType::Text) {
        auto &s = static_cast<TextItem &>(m);
        const auto &x = static_cast<const TextItem &>(a), &y = static_cast<const TextItem &>(b);
        s.insets = lerp(x.insets, y.insets, t);
        s.fitScale = lerp(x.fitScale, y.fitScale, t);
        s.fitLineCut = lerp(x.fitLineCut, y.fitLineCut, t);
    } else if (m.type() == ItemType::Picture) {
        // The picture shows through the frame as it does in each: where it sits is a share of the frame, which moves as the frame does.
        auto &s = static_cast<PictureItem &>(m);
        const auto &x = static_cast<const PictureItem &>(a), &y = static_cast<const PictureItem &>(b);
        auto share = [](const PictureItem &p) {
            const double w = std::max(1e-6, p.rect.width()), h = std::max(1e-6, p.rect.height());
            return QRectF(p.imgRect.x() / w, p.imgRect.y() / h, p.imgRect.width() / w, p.imgRect.height() / h);
        };
        const QRectF u = share(x), v = share(y);
        const QSizeF size = m.rect.size();
        s.imgRect = QRectF(lerp(u.x(), v.x(), t) * size.width(), lerp(u.y(), v.y(), t) * size.height(), lerp(u.width(), v.width(), t) * size.width(),
                           lerp(u.height(), v.height(), t) * size.height());
        s.transparency = lerp(x.transparency, y.transparency, t);
    }
}


// ---- text that moves by the word or the character ----

constexpr double kTextRoom = 6;   // the points around a text box that the picture of its text holds, for what runs over

// A word or a character of a story, and where the layout puts it, in its box's points.
struct Token {
    QString text;
    int at = 0;       // its place in the story
    QRectF rect;
};

bool partOfWord(QChar c) { return c.isLetterOrNumber() || c == QLatin1Char('\'') || c == QChar(0x2019); }

// The words (a word is letters and numbers, with the apostrophes inside it; any other mark is a word of its own) or the characters of a story that
// the layout sets on one line, in the order they are read. What is left out stays in the text that fades: spaces, a word broken between two lines.
QVector<Token> tokensOf(const QTextDocument &story, const StoryLayout &layout, int frame, const QPointF &origin, bool byWord)
{
    const QString plain = story.toPlainText();
    QVector<Token> out;
    for (qsizetype i = 0; i < plain.size();) {
        const QChar c = plain[i];
        if (c.isSpace() || c == QChar::ObjectReplacementCharacter) {
            ++i;
            continue;
        }
        qsizetype end = i + 1;
        if (byWord) {
            if (partOfWord(c))
                while (end < plain.size() && partOfWord(plain[end])) ++end;
        } else {
            // A character with its marks (an accent set apart) and a pair of surrogates that are one.
            if (c.isHighSurrogate() && end < plain.size() && plain[end].isLowSurrogate()) ++end;
            while (end < plain.size() && (plain[end].category() == QChar::Mark_NonSpacing || plain[end].category() == QChar::Mark_SpacingCombining)) ++end;
        }
        const QVector<QRectF> rects = layout.rangeRects(frame, int(i), int(end));
        if (rects.size() == 1) out.push_back(Token{plain.mid(i, end - i), int(i), rects[0].translated(origin)});
        i = end;
    }
    return out;
}

// The words of the old text paired with those of the new: the first time a word is read in one with the first time it is read in the other, the
// second with the second, and so on.
QVector<QPair<int, int>> pairTokens(const QVector<Token> &a, const QVector<Token> &b)
{
    QHash<QString, QVector<int>> inB;
    for (qsizetype j = 0; j < b.size(); ++j) inB[b[j].text].push_back(int(j));
    QHash<QString, int> taken;
    QVector<QPair<int, int>> out;
    for (qsizetype i = 0; i < a.size(); ++i) {
        const auto found = inB.constFind(a[i].text);
        if (found == inB.constEnd()) continue;
        int &k = taken[a[i].text];
        if (k < found->size()) out.push_back({int(i), found.value()[k++]});
    }
    return out;
}

// Whether the character at a place in one story looks as the one at a place in the other: its font, size, and color.
bool looksAlike(const QTextDocument &sa, int pa, const QTextDocument &sb, int pb)
{
    QTextCursor ca(const_cast<QTextDocument *>(&sa)), cb(const_cast<QTextDocument *>(&sb));
    ca.setPosition(pa + 1);
    cb.setPosition(pb + 1);
    return ca.charFormat() == cb.charFormat();
}

} // namespace

MorphPlan planMorph(const Document &doc, const ItemList &from, const ItemList &to) { return Pairing(doc, from, to).run(); }

// ---- the frames ----

struct MorphFrames::Impl {
    // The words or characters that two texts have in common, moving from where one has them to where the other does. The text of each is
    // painted once into a picture the size of its box (and room for what runs over), at the frame's pixels; each frame is made of the
    // pictures' pieces, one for each word or character both have, at places between the two, and of what is left of both texts, fading.
    struct Motion {
        struct Piece {
            QRect from, to;   // in the two pictures
            bool alike;       // the same look: one moves; else the old one fades into the new
        };
        std::vector<Piece> pieces;
        QImage oldText, newText;           // the pictures
        QImage oldRest, newRest;           // and what is left of them when the pieces are cut out
        QImage sum, scratch;               // the text of a frame, and a place to weigh a picture
        double pad = 0;                    // the room around the box, in points
    };
    // What is painted for a pair.
    struct Work {
        ItemPtr a, b;            // the old slide's object and the new one's
        bool smooth = false;     // one object that changes, else two that dissolve
        ItemPtr body;            // smooth: the object, a copy of b that is changed for each frame
        bool textChanges = false;
        ItemPtr textA, textB;    // smooth, and the text of a and b is not alike: copies that paint only their text
        bool fillChanges = false, strokeChanges = false;
        std::shared_ptr<Motion> motion;   // and the words or characters they have in common move
    };
    // An object that fades, painted once.
    struct Layer {
        QImage image;
        QPointF at;
        bool made = false;
    };

    Impl(const Document *document, int fromSlide, int toSlide, const QString &unit, const QSize &viewSize, qreal ratio, const QColor &barColor,
         const deck::States *fromStates, const deck::States *toStates)
        : doc(document), from(fromSlide), to(toSlide), by(unit), size(viewSize), dpr(ratio), bars(barColor)
    {
        const QSizeF page = doc->pageSize();
        usable = from >= 0 && from < doc->pages.size() && to >= 0 && to < doc->pages.size() && page.width() > 0 && page.height() > 0 && !size.isEmpty();
        if (!usable) return;
        slide = slideRectIn(page, QSizeF(size));
        scale = slide.width() / page.width();
        view.translate(slide.left(), slide.top());
        view.scale(scale, scale);
        usable = doc->pages[from]->items.size() <= kMorphMaxObjects && doc->pages[to]->items.size() <= kMorphMaxObjects;
        px = QSize(std::max(1, qRound(size.width() * dpr)), std::max(1, qRound(size.height() * dpr)));
        baseFrom = base(from, usable);
        baseTo = base(to, usable);
        baseSame = baseFrom == baseTo;
        if (baseSame) baseTo = QImage();
        if (!usable) return;
        fromItems = shownObjects(*doc->pages[from], fromStates, &fromShare);
        toItems = shownObjects(*doc->pages[to], toStates, &toShare);
        plan = planMorph(*doc, fromItems, toItems);
        pairs.resize(plan.pairs.size());
        for (qsizetype k = 0; k < plan.pairs.size(); ++k) pairs[k] = work(plan.pairs[k]);
        leaving.resize(plan.leaving.size());
        arriving.resize(plan.arriving.size());
    }

    PaintContext contextFor(int slideIndex) const
    {
        PaintContext ctx;
        ctx.doc = doc;
        ctx.cache = const_cast<LayoutCache *>(&cache);
        ctx.pageNumber = slideIndex + 1;
        ctx.pageCount = int(doc->pages.size());
        return ctx;
    }

    void setHints(QPainter &p) const
    {
        p.setRenderHint(QPainter::Antialiasing);
        p.setRenderHint(QPainter::TextAntialiasing);
        p.setRenderHint(QPainter::SmoothPixmapTransform);
    }

    QImage blank() const
    {
        QImage img(px, QImage::Format_ARGB32_Premultiplied);
        img.setDevicePixelRatio(dpr);
        img.fill(Qt::transparent);
        return img;
    }

    // A slide's page without its own objects (with them, when `all`): the background and what the masters hold. Opaque.
    QImage base(int index, bool withoutObjects)
    {
        QImage img = blank();
        img.fill(bars);
        QPainter p(&img);
        setHints(p);
        p.setTransform(view);
        p.setClipRect(QRectF(QPointF(0, 0), doc->pageSize()));
        p.fillRect(QRectF(QPointF(0, 0), doc->pageSize()), Qt::white);
        PaintContext ctx = contextFor(index);
        if (withoutObjects)
            for (const ItemPtr &it : doc->pages[index]->items) ctx.opt.hidden.insert(it->id);
        Renderer::paintPage(&p, ctx, index);
        return img;
    }

    Work work(const MorphPair &pair)
    {
        Work w;
        w.a = fromItems[pair.from];
        w.b = toItems[pair.to];
        w.smooth = alike(*w.a, *w.b);
        if (!w.smooth) return w;
        w.body = w.b->clone();
        w.fillChanges = !(w.a->fill == w.b->fill);
        w.strokeChanges = !(w.a->stroke == w.b->stroke);
        w.textChanges = (hasStory(*doc, *w.a) || hasStory(*doc, *w.b)) && storyLook(*doc, *w.a) != storyLook(*doc, *w.b);
        if (w.textChanges) {
            w.textA = textOnly(*w.a);
            w.textB = textOnly(*w.b);
            setStory(*w.body, QString());
            w.motion = motionFor(w);
        }
        return w;
    }

    // The copy of an object that paints only its text, in a box of its own at the origin, turned and flipped no more.
    ItemPtr boxOf(const Item &src) const
    {
        ItemPtr c = textOnly(src);
        c->rect = QRectF(QPointF(0, 0), src.rect.size());
        c->rotation = 0;
        return c;
    }

    // The layout of the story of a text box or a shape, its frame in the chain, and where its lines start in the box.
    const StoryLayout *layoutOf(const Item &item, int slideIndex, QPointF *origin, int *frame)
    {
        const PaintContext ctx = contextFor(slideIndex);
        *origin = QPointF(0, 0);
        *frame = 0;
        if (item.type() == ItemType::Shape) return Renderer::shapeTextLayout(ctx, static_cast<const ShapeItem &>(item), origin);
        const LayoutCache::FrameLayout fl = cache.textFrame(*doc, static_cast<const TextItem &>(item), slideIndex + 1, ctx.opt);
        *frame = fl.frame;
        return fl.layout;
    }

    // What moves when the text of two objects is moved by the word or the character: none when they have no word (or character) in common, when a
    // box is turned on its side or flipped, when a text is longer than a morph holds, or when the pictures of the texts would take too much.
    std::shared_ptr<Motion> motionFor(const Work &w)
    {
        if (by != QLatin1String("words") && by != QLatin1String("characters")) return nullptr;
        const Item &a = *w.a, &b = *w.b;
        if ((a.type() != ItemType::Text && a.type() != ItemType::Shape) || a.flipH || a.flipV) return nullptr;
        if (a.type() == ItemType::Text && (static_cast<const TextItem &>(a).vertical || static_cast<const TextItem &>(b).vertical)) return nullptr;
        const QTextDocument *sa = doc->storyDoc(storyIdOf(a)), *sb = doc->storyDoc(storyIdOf(b));
        if (!sa || !sb || sa->characterCount() > kMorphMaxTextCharacters || sb->characterCount() > kMorphMaxTextCharacters) return nullptr;
        const ItemPtr boxA = boxOf(a), boxB = boxOf(b);
        QPointF originA, originB;
        int frameA = 0, frameB = 0;
        const StoryLayout *layoutA = layoutOf(*boxA, from, &originA, &frameA), *layoutB = layoutOf(*boxB, to, &originB, &frameB);
        if (!layoutA || !layoutB) return nullptr;
        const bool byWord = by == QLatin1String("words");
        const QVector<Token> oldTokens = tokensOf(*sa, *layoutA, frameA, originA, byWord), newTokens = tokensOf(*sb, *layoutB, frameB, originB, byWord);
        const QVector<QPair<int, int>> pairs = pairTokens(oldTokens, newTokens);
        if (pairs.isEmpty()) return nullptr;

        auto m = std::make_shared<Motion>();
        m->pad = kTextRoom;
        const double perPoint = scale * dpr;   // device pixels to a point of the slide
        const QSizeF room(std::max(a.rect.width(), b.rect.width()) + 2 * m->pad, std::max(a.rect.height(), b.rect.height()) + 2 * m->pad);
        const double wide = room.width() * perPoint, high = room.height() * perPoint;
        if (!(wide >= 1 && high >= 1) || wide > 32768 || high > 32768) return nullptr;   // a box that size (or no size, or a number that is not one) is not text on a slide
        const QSize px(int(std::ceil(wide)), int(std::ceil(high)));
        const qint64 pixels = qint64(px.width()) * px.height();
        if (textPixels + 6 * pixels > kMorphMaxTextPixels) return nullptr;
        textPixels += 6 * pixels;
        auto paint = [&](const ItemPtr &box, int slideIndex) {
            QImage img(px, QImage::Format_ARGB32_Premultiplied);
            img.fill(Qt::transparent);
            QPainter lp(&img);
            setHints(lp);
            lp.scale(perPoint, perPoint);
            lp.translate(m->pad, m->pad);
            Renderer::paintItem(&lp, contextFor(slideIndex), *box);
            return img;
        };
        m->oldText = paint(boxA, from);
        m->newText = paint(boxB, to);
        // A token's cell: the pixels its place covers.
        auto cell = [&](const QRectF &r) {
            return QRectF((r.x() + m->pad) * perPoint, (r.y() + m->pad) * perPoint, r.width() * perPoint, r.height() * perPoint).toAlignedRect() & QRect(QPoint(0, 0), px);
        };
        for (const QPair<int, int> &pair : pairs) {
            const Token &x = oldTokens[pair.first], &y = newTokens[pair.second];
            Motion::Piece piece;
            piece.from = cell(x.rect);
            piece.to = cell(y.rect);
            if (piece.from.isEmpty() || piece.to.isEmpty()) continue;
            piece.alike = std::abs(x.rect.height() - y.rect.height()) < 0.02 * y.rect.height() && std::abs(x.rect.width() - y.rect.width()) < 0.02 * y.rect.width() &&
                          looksAlike(*sa, x.at, *sb, y.at);
            m->pieces.push_back(piece);
        }
        if (m->pieces.empty()) return nullptr;
        // What is left of each text when the pieces are cut out.
        m->oldRest = m->oldText;
        m->newRest = m->newText;
        for (QImage *rest : {&m->oldRest, &m->newRest}) {
            QPainter cp(rest);
            cp.setCompositionMode(QPainter::CompositionMode_Clear);
            for (const Motion::Piece &piece : m->pieces) cp.fillRect(rest == &m->oldRest ? piece.from : piece.to, Qt::transparent);
        }
        m->sum = QImage(px, QImage::Format_ARGB32_Premultiplied);
        m->scratch = QImage(px, QImage::Format_ARGB32_Premultiplied);
        return m;
    }

    // The text of a pair at a share `t` of the way, drawn over its object.
    void drawMotion(QPainter &p, Work &w, double t)
    {
        Motion &m = *w.motion;
        m.sum.setDevicePixelRatio(1);
        m.sum.fill(Qt::transparent);
        const QRect all(QPoint(0, 0), m.sum.size());
        QPainter sp(&m.sum);
        sp.setRenderHint(QPainter::SmoothPixmapTransform);
        // What is left of the two texts: one fades out and the other in, to the whole of what both have where they have the same.
        if (t < 1) {
            sp.setOpacity(1 - t);
            sp.drawImage(QPointF(0, 0), m.oldRest);
            sp.setOpacity(1);
        }
        if (t > 0) {
            QPainter wp(&m.scratch);
            wp.setCompositionMode(QPainter::CompositionMode_Source);
            wp.drawImage(QPointF(0, 0), m.newRest);
            wp.setCompositionMode(QPainter::CompositionMode_DestinationIn);
            wp.fillRect(all, QColor(0, 0, 0, std::clamp(qRound(t * 255), 0, 255)));
            wp.end();
            sp.setCompositionMode(QPainter::CompositionMode_Plus);
            sp.drawImage(QPointF(0, 0), m.scratch);
            sp.setCompositionMode(QPainter::CompositionMode_SourceOver);
        }
        for (const Motion::Piece &piece : m.pieces) {
            const QRectF at(lerp(piece.from.x(), piece.to.x(), t), lerp(piece.from.y(), piece.to.y(), t), lerp(piece.from.width(), piece.to.width(), t),
                            lerp(piece.from.height(), piece.to.height(), t));
            if (piece.alike) {
                // One piece, moved whole pixels so that it stays as sharp as it is.
                const bool old = t < 0.5;
                const QRect source = old ? piece.from : piece.to;
                sp.drawImage(QPoint(qRound(at.x()), qRound(at.y())), old ? m.oldText : m.newText, source);
            } else {
                sp.setOpacity(1 - t);
                sp.drawImage(at, m.oldText, QRectF(piece.from));
                sp.setOpacity(t);
                sp.drawImage(at, m.newText, QRectF(piece.to));
                sp.setOpacity(1);
            }
        }
        sp.end();
        // Where the box is, the picture's top left in the frame.
        const Frame f = lerp(frameOf(*w.a), frameOf(*w.b), t);
        const double turn = qDegreesToRadians(f.turn);
        const QPointF corner(-m.pad - f.size.width() / 2, -m.pad - f.size.height() / 2);
        QPointF at = view.map(f.middle + QPointF(corner.x() * std::cos(turn) - corner.y() * std::sin(turn), corner.x() * std::sin(turn) + corner.y() * std::cos(turn)));
        if (f.turn == 0) at = QPointF(std::round(at.x() * dpr) / dpr, std::round(at.y() * dpr) / dpr);   // a plain move keeps the letters sharp
        m.sum.setDevicePixelRatio(dpr);
        p.save();
        p.resetTransform();
        p.translate(at);
        p.rotate(f.turn);
        p.drawImage(QPointF(0, 0), m.sum);
        p.restore();
    }

    // The frame's area, in view pixels, that an object in slide units reaches: its bounds and room for what spills out of it.
    QRect reach(const QRectF &bounds) const
    {
        const QRectF r = view.mapRect(bounds.adjusted(-40, -40, 40, 40)).intersected(QRectF(QPointF(0, 0), QSizeF(size)));
        return r.toAlignedRect() & QRect(QPoint(0, 0), size);
    }

    // The painter for a layer image over `area`, set so that slide units paint as they do in the frame.
    void begin(QPainter &lp, QImage &img, const QRect &area, const QTransform &extra) const
    {
        lp.begin(&img);
        setHints(lp);
        lp.setCompositionMode(QPainter::CompositionMode_Source);
        lp.fillRect(area, Qt::transparent);
        lp.setCompositionMode(QPainter::CompositionMode_SourceOver);
        lp.setClipRect(area);
        lp.setTransform(view);
        lp.setClipRect(QRectF(QPointF(0, 0), doc->pageSize()), Qt::IntersectClip);
        lp.setTransform(extra, true);
    }

    // The pixels of the frame image that an area of it (in device-independent units) covers.
    QRectF pixelsOf(const QRect &area) const
    {
        return QRectF(area.x() * dpr, area.y() * dpr, area.width() * dpr, area.height() * dpr);
    }

    QImage &scratch(QImage &img) const
    {
        if (img.isNull()) img = blank();
        return img;
    }

    // An object painted alone in a layer: the layer image, and where it goes.
    Layer paintLayer(const Item &it, int slideIndex)
    {
        Layer layer;
        layer.made = true;
        const QRect area = reach(it.bounds());
        if (area.isEmpty()) return layer;
        QImage &one = scratch(scratchOne);
        QPainter lp;
        begin(lp, one, area, QTransform());
        Renderer::paintItem(&lp, contextFor(slideIndex), it);
        lp.end();
        layer.image = one.copy(pixelsOf(area).toAlignedRect());
        layer.image.setDevicePixelRatio(dpr);
        layer.at = area.topLeft();
        return layer;
    }

    // An object that fades in or out, painted once and then as an image.
    void fade(QPainter &p, Layer &layer, const ItemPtr &it, int slideIndex, double share)
    {
        if (share <= 0.0005) return;
        // Kept for the next frames unless the objects that fade have taken what is allowed; then painted each time.
        Layer *use = &layer, temporary;
        if (!layer.made) {
            temporary = paintLayer(*it, slideIndex);
            const qint64 pixels = qint64(temporary.image.width()) * temporary.image.height();
            if (layerPixels + pixels <= kMorphMaxLayerPixels) {
                layerPixels += pixels;
                layer = std::move(temporary);
            } else {
                use = &temporary;
            }
        }
        if (use->image.isNull()) return;
        p.save();
        p.resetTransform();
        p.setOpacity(share);
        p.drawImage(use->at, use->image);
        p.restore();
    }

    struct Dissolving {
        const Item *item;
        int slide;          // the slide it is on
        QTransform extra;   // from the slide to where the item is drawn
        double share;
    };

    // Some objects painted so that each shows its share and the shares add up (a dissolve: where two alike are painted, they stay as
    // they are, instead of each dimming the other).
    void dissolve(QPainter &p, const std::vector<Dissolving> &parts)
    {
        QRectF bounds;
        for (const Dissolving &d : parts)
            if (d.share > 0.0005) bounds = bounds.united(d.extra.mapRect(d.item->bounds()));
        const QRect area = reach(bounds);
        if (bounds.isNull() || area.isEmpty()) return;
        QImage &sum = scratch(scratchSum), &one = scratch(scratchOne);
        QPainter sp;
        sp.begin(&sum);
        sp.setCompositionMode(QPainter::CompositionMode_Source);
        sp.fillRect(area, Qt::transparent);
        sp.setCompositionMode(QPainter::CompositionMode_Plus);
        for (const Dissolving &d : parts) {
            if (d.share <= 0.0005) continue;
            QPainter lp;
            begin(lp, one, area, d.extra);
            Renderer::paintItem(&lp, contextFor(d.slide), *d.item);
            // Its share of what is painted, all of it as one: each of its parts keeps what it covers of the others. (A painter's own
            // opacity would take the share of each part separately, and Qt's Plus with an opacity is not an add.)
            lp.resetTransform();
            lp.setClipRect(QRectF(area));
            lp.setCompositionMode(QPainter::CompositionMode_DestinationIn);
            lp.fillRect(QRectF(area), QColor(0, 0, 0, std::clamp(qRound(d.share * 255), 0, 255)));
            lp.end();
            sp.drawImage(QRectF(area), one, pixelsOf(area));
        }
        sp.end();
        p.save();
        p.resetTransform();
        p.drawImage(QRectF(area), sum, pixelsOf(area));
        p.restore();
    }

    void paintPair(QPainter &p, Work &w, double t)
    {
        const PaintContext ctx = contextFor(to);
        if (!w.smooth) {
            const Frame a = frameOf(*w.a), b = frameOf(*w.b), now = lerp(a, b, t);
            dissolve(p, {{w.a.get(), from, carrying(a, now), 1 - t}, {w.b.get(), to, carrying(b, now), t}});
            return;
        }
        Item &m = *w.body;
        place(m, *w.a, *w.b, t);
        if (w.fillChanges) m.fill = mixFill(w.a->fill, w.b->fill, t, doc->colors);
        if (w.strokeChanges) m.stroke = mixStroke(w.a->stroke, w.b->stroke, t, doc->colors);
        if (t < 0.5) m.fx = w.a->fx;
        else m.fx = w.b->fx;
        Renderer::paintItem(&p, ctx, m);
        if (!w.textChanges) return;
        if (w.motion) {
            drawMotion(p, w, t);
            return;
        }
        place(*w.textA, *w.a, *w.b, t);
        place(*w.textB, *w.a, *w.b, t);
        dissolve(p, {{w.textA.get(), from, QTransform(), 1 - t}, {w.textB.get(), to, QTransform(), t}});
    }

    QImage frame(double progress)
    {
        const double t = std::clamp(progress, 0.0, 1.0);
        if (last.isNull()) last = blank();
        QPainter p(&last);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.drawImage(QPointF(0, 0), baseFrom);
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
        if (!baseSame && t > 0) {
            p.setOpacity(t);
            p.drawImage(QPointF(0, 0), baseTo);
            p.setOpacity(1);
        }
        if (usable) {
            setHints(p);
            p.setClipRect(slide);
            p.setTransform(view);
            for (qsizetype k = 0; k < plan.leaving.size(); ++k) fade(p, leaving[k], fromItems[plan.leaving[k]], from, (1 - t) * fromShare[plan.leaving[k]]);
            // The new slide's objects in its order: those that arrive fade in, and the pairs are painted.
            qsizetype pairAt = 0, arriveAt = 0;
            for (qsizetype j = 0; j < toItems.size(); ++j) {
                if (pairAt < plan.pairs.size() && plan.pairs[pairAt].to == j) {
                    paintPair(p, pairs[pairAt], t);
                    ++pairAt;
                } else if (arriveAt < plan.arriving.size() && plan.arriving[arriveAt] == j) {
                    fade(p, arriving[arriveAt], toItems[j], to, t * toShare[j]);
                    ++arriveAt;
                }
            }
        }
        p.end();
        return last;
    }

    const Document *doc;
    int from, to;
    QString by;
    QSize size;
    qreal dpr;
    QColor bars;
    bool usable = false;
    QRectF slide;
    double scale = 1;
    QTransform view;   // slide units to frame pixels (device-independent)
    QSize px;
    QImage baseFrom, baseTo;
    bool baseSame = false;
    ItemList fromItems, toItems;
    QVector<double> fromShare, toShare;
    MorphPlan plan;
    std::vector<Work> pairs;
    std::vector<Layer> leaving, arriving;
    LayoutCache cache;
    qint64 textPixels = 0;    // what the pictures of moving text take, counted in pixels (six of the size of each box)
    qint64 layerPixels = 0;   // what the objects that fade take
    QImage last, scratchOne, scratchSum;
};

MorphFrames::MorphFrames(const Document *doc, int from, int to, const QString &by, const QSize &size, qreal dpr, const QColor &bars,
                         const deck::States *fromStates, const deck::States *toStates)
    : d(new Impl(doc, from, to, by, size, dpr, bars, fromStates, toStates))
{
    // The first frame lays out the text and paints the objects that fade: done before the show's clock starts.
    if (d->usable) d->frame(0.001);
}

MorphFrames::~MorphFrames() = default;

bool MorphFrames::isUsable() const { return d->usable; }

const MorphPlan &MorphFrames::plan() const { return d->plan; }

QStringList MorphFrames::describe() const
{
    static const char *rules[] = {"forced name", "name", "text", "picture", "shape"};
    auto nameOf = [](const ItemPtr &it) { return it->name.isEmpty() ? QStringLiteral("(%1)").arg(itemTypeName(it->type())) : it->name; };
    QStringList out;
    for (qsizetype k = 0; k < d->plan.pairs.size(); ++k) {
        const MorphPair &p = d->plan.pairs[k];
        const Impl::Work &w = d->pairs[k];
        QString how = w.smooth ? QStringLiteral("one object") : QStringLiteral("dissolve");
        if (w.motion) how += QStringLiteral(", %1 of its %2 move").arg(w.motion->pieces.size()).arg(d->by);
        out << QStringLiteral("pair\t%1\t%2\t%3\t%4").arg(nameOf(d->fromItems[p.from]), nameOf(d->toItems[p.to]), QString::fromLatin1(rules[int(p.rule)]), how);
    }
    for (const int i : std::as_const(d->plan.leaving)) out << QStringLiteral("leaves\t%1").arg(nameOf(d->fromItems[i]));
    for (const int j : std::as_const(d->plan.arriving)) out << QStringLiteral("arrives\t%1").arg(nameOf(d->toItems[j]));
    return out;
}

QImage MorphFrames::frame(double progress)
{
    if (!d->usable && d->baseFrom.isNull()) return QImage();
    return d->frame(progress);
}

void MorphFrames::paint(QPainter *p, double progress)
{
    const QImage img = frame(progress);
    if (img.isNull()) return;
    p->save();
    p->setCompositionMode(QPainter::CompositionMode_Source);
    p->drawImage(QPointF(0, 0), img);
    p->restore();
}

} // namespace jp
