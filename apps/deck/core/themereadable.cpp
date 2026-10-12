#include "core/backdrop.h"
#include "core/color.h"
#include "core/themes_p.h"
#include "text/textprops.h"

#include <QSet>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextList>
#include <algorithm>

namespace jp::deck::detail {

namespace {

// WCAG AA: text reads at 4.5 to 1; a bullet or other mark that carries meaning at 3 to 1.
constexpr double kTextContrast = 4.5, kMarkContrast = 3.0;
// A thing's backdrop is looked at in nine places, a tenth of the way in from each edge and in the middle.
constexpr double kAcross[3] = {0.1, 0.5, 0.9};
// The most colors kept of one thing's backdrop.
constexpr int kMaxBackdropColors = 24;

// What stands behind one story of a page: the nine points' colors together.
struct Reach {
    QVector<QColor> colors;   // everywhere
    QVector<QColor> middle;   // in the middle
    bool picture = false;
    bool known = true;
};

// A story of a page, the object it is in, and where.
struct Place {
    const Item *item = nullptr;
    QString key;       // the object's id (and the cell, for a table)
    QString storyId;
    QRectF frame;      // frame coordinates of the object: the room the text has
    const Fill *own = nullptr;
};

void places(const PageBase &page, const std::function<void(const Place &)> &fn)
{
    walkItems(page.items, [&](const ItemPtr &it) {
        Place p;
        p.item = it.get();
        p.key = it->id;
        p.frame = QRectF(QPointF(0, 0), it->rect.size());
        switch (it->type()) {
        case ItemType::Text:
            p.storyId = static_cast<const TextItem &>(*it).storyId;
            p.own = &it->fill;
            if (!p.storyId.isEmpty()) fn(p);
            break;
        case ItemType::Shape:
            p.storyId = static_cast<const ShapeItem &>(*it).storyId;
            p.own = &it->fill;
            if (!p.storyId.isEmpty()) fn(p);
            break;
        case ItemType::Table: {
            const auto &t = static_cast<const TableItem &>(*it);
            if (t.rows <= 0 || t.cols <= 0 || t.colW.size() < t.cols || t.rowH.size() < t.rows || t.cells.size() < t.rows * t.cols) break;
            for (int r = 0; r < t.rows; ++r)
                for (int c = 0; c < t.cols; ++c) {
                    const TableCell &cell = t.cell(r, c);
                    if (cell.covered) continue;
                    p.frame = t.cellRect(r, c);
                    p.own = Backdrop::paints(cell.fill) ? &cell.fill : &t.fill;
                    for (int half = 0; half < 2; ++half) {
                        p.storyId = cell.halfStory(half);
                        p.key = QStringLiteral("%1#%2,%3,%4").arg(it->id).arg(r).arg(c).arg(half);
                        if (!p.storyId.isEmpty()) fn(p);
                    }
                }
            break;
        }
        default: break;
        }
    });
}

Reach reachOf(const Backdrop &backdrop, const Place &p)
{
    Reach out;
    const QTransform to = p.item->transform();
    for (double fy : kAcross)
        for (double fx : kAcross) {
            const QPointF at = to.map(QPointF(p.frame.left() + p.frame.width() * fx, p.frame.top() + p.frame.height() * fy));
            const Behind b = backdrop.at(p.item, at, p.own);
            if (!b.known) {
                out.known = false;
                return out;
            }
            // A picture counts where it is in the middle of the thing; at a corner of its frame, it is a picture's edge crossing it.
            if (fx == 0.5 && fy == 0.5) {
                out.middle = b.colors;
                out.picture = b.picture;
            }
            for (const QColor &c : b.colors) {
                bool have = false;
                for (const QColor &x : std::as_const(out.colors)) have = have || x.rgb() == c.rgb();
                if (!have && out.colors.size() < kMaxBackdropColors) out.colors.push_back(c);
            }
        }
    return out;
}

bool readsOn(const QColor &fg, const QVector<QColor> &behind, double need)
{
    if (behind.isEmpty()) return false;
    for (const QColor &c : behind)
        if (contrastRatio(fg, c) < need) return false;
    return true;
}

// `from` moved toward white or black, as little as will do, until it reads on all of `behind`; invalid if no step reaches it.
QColor nearestReading(const QColor &from, const QVector<QColor> &behind, double need)
{
    QColor best;
    double bestStep = 2;
    for (const QColor &toward : {QColor(Qt::white), QColor(Qt::black)}) {
        for (int step = 0; step <= 20; ++step) {
            const double t = step / 20.0;
            const QColor c(mix(from, toward, t).rgb());
            if (readsOn(c, behind, need)) {
                if (t < bestStep) {
                    bestStep = t;
                    best = c;
                }
                break;
            }
        }
    }
    return best;
}

// The color a run or bullet should have now, given what it was and what is behind it; `now` itself if it is fine. A color that reads all
// over the thing's backdrop is looked for first; where none can (the thing lies over two colors), one that reads in the middle of it.
QColor colorNow(const QColor &now, const QColor &was, bool readBefore, const Reach &behind, const ColorSwap &plain, double need)
{
    if (behind.picture) return QColor(plain(was).rgb());   // a picture doesn't turn with the theme
    if (!behind.known || behind.colors.isEmpty() || !readBefore || readsOn(now, behind.colors, need)) return now;
    const QColor swapped = plain(was);
    // The old color swapped for the new scheme's, else as near to it as reads, else the old color itself.
    auto choose = [&](const QVector<QColor> &on) {
        if (readsOn(swapped, on, need)) return QColor(swapped.rgb());
        const QColor near = nearestReading(swapped, on, need);
        if (near.isValid()) return near;
        return readsOn(was, on, need) ? QColor(was.rgb()) : QColor();
    };
    const QColor everywhere = choose(behind.colors);
    if (everywhere.isValid()) return everywhere;
    if (behind.middle.isEmpty() || readsOn(now, behind.middle, need)) return now;
    const QColor inTheMiddle = choose(behind.middle);
    return inTheMiddle.isValid() ? inTheMiddle : now;
}

template <typename T>
const T *recordAt(const QVector<T> &records, int pos)
{
    auto it = std::upper_bound(records.cbegin(), records.cend(), pos, [](int p, const T &r) { return p < r.pos; });
    return it == records.cbegin() ? nullptr : &*(it - 1);
}

} // namespace

QColor shownRunColor(const Document &d, const QTextCharFormat &cf)
{
    const QString ref = cf.stringProperty(tp::ColorRefP);
    if (!ref.isEmpty()) {
        const ColorRef r = ColorRef::fromString(ref);
        return r.kind() == ColorRef::None ? QColor() : r.resolve(d.colors);
    }
    if (cf.hasProperty(QTextFormat::ForegroundBrush) && cf.foreground().style() == Qt::SolidPattern) return cf.foreground().color();
    return QColor();
}

void setRunColor(QTextCharFormat &cf, const QColor &c)
{
    cf.setForeground(QBrush(c));
    cf.setProperty(tp::ColorRefP, ColorRef::rgb(c).toString());
}

void editRunsAt(QTextDocument *sd, const std::function<bool(QTextCharFormat &, int, bool)> &fn)
{
    struct Edit {
        int from, to;
        QTextCharFormat format;
        bool end;
    };
    QVector<Edit> edits;
    for (QTextBlock b = sd->begin(); b.isValid(); b = b.next()) {
        QTextCharFormat end = b.charFormat();
        if (fn(end, b.position(), true)) edits << Edit{b.position(), b.position(), end, true};
        for (auto it = b.begin(); !it.atEnd(); ++it) {
            QTextCharFormat cf = it.fragment().charFormat();
            if (fn(cf, it.fragment().position(), false)) edits << Edit{it.fragment().position(), it.fragment().position() + it.fragment().length(), cf, false};
        }
    }
    for (const Edit &e : edits) {
        QTextCursor c(sd);
        c.setPosition(e.from);
        if (e.end) {
            c.setBlockCharFormat(e.format);
        } else {
            c.setPosition(e.to, QTextCursor::KeepAnchor);
            c.setCharFormat(e.format);
        }
    }
}

void TextReadable::note(const PageBase &page, const MasterPage *base)
{
    const Backdrop backdrop(m_d, page, base);
    places(page, [&](const Place &p) {
        QTextDocument *sd = m_d.storyDoc(p.storyId);
        if (!sd) return;
        const Reach behind = reachOf(backdrop, p);
        Story story;
        auto take = [&](QVector<Run> &into, int pos, const QColor &c, double need) {
            if (!c.isValid()) return;
            into.push_back({pos, c, behind.known && (behind.picture || readsOn(c, behind.middle.isEmpty() ? behind.colors : behind.middle, need))});
        };
        QSet<QTextList *> seen;
        for (QTextBlock b = sd->begin(); b.isValid(); b = b.next()) {
            take(story.ends, b.position(), shownRunColor(m_d, b.charFormat()), kTextContrast);
            for (auto it = b.begin(); !it.atEnd(); ++it) take(story.runs, it.fragment().position(), shownRunColor(m_d, it.fragment().charFormat()), kTextContrast);
            QTextList *list = b.textList();
            if (!list || seen.contains(list)) continue;
            seen.insert(list);
            const ColorRef ref = ColorRef::fromString(list->format().stringProperty(tp::BulletColor));
            if (ref.kind() != ColorRef::None) take(story.bullets, b.position(), ref.resolve(m_d.colors), kMarkContrast);
        }
        if (!story.runs.isEmpty() || !story.ends.isEmpty() || !story.bullets.isEmpty()) m_stories.insert(page.id + QLatin1Char('/') + p.key, story);
    });
}

void TextReadable::keep(PageBase &page, const MasterPage *base, const ColorSwap &plain)
{
    const Backdrop backdrop(m_d, page, base);
    places(page, [&](const Place &p) {
        const auto found = m_stories.constFind(page.id + QLatin1Char('/') + p.key);
        if (found == m_stories.cend()) return;
        QTextDocument *sd = m_d.storyDoc(p.storyId);
        if (!sd) return;
        const Reach behind = reachOf(backdrop, p);
        if (!behind.known) return;
        const Story &story = found.value();
        editRunsAt(sd, [&](QTextCharFormat &cf, int pos, bool end) {
            const QColor now = shownRunColor(m_d, cf);
            const Run *was = recordAt(end ? story.ends : story.runs, pos);
            if (!now.isValid() || !was) return false;
            const QColor want = colorNow(now, was->was, was->read, behind, plain, kTextContrast);
                if (want == now) return false;
            setRunColor(cf, want);
            return true;
        });
        // The bullets, as the lists say them.
        QSet<QTextList *> seen;
        for (QTextBlock b = sd->begin(); b.isValid(); b = b.next()) {
            QTextList *list = b.textList();
            if (!list || seen.contains(list)) continue;
            seen.insert(list);
            QTextListFormat lf = list->format();
            const ColorRef ref = ColorRef::fromString(lf.stringProperty(tp::BulletColor));
            const Run *was = recordAt(story.bullets, b.position());
            if (ref.kind() == ColorRef::None || !was) continue;
            const QColor now = ref.resolve(m_d.colors);
            const QColor want = colorNow(now, was->was, was->read, behind, plain, kMarkContrast);
            if (want == now) continue;
            lf.setProperty(tp::BulletColor, ColorRef::rgb(want).toString());
            list->setFormat(lf);
        }
    });
}

} // namespace jp::deck::detail
