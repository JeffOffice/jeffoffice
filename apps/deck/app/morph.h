#pragma once
// The Morph transition. The objects the two slides have in common move, grow,
// turn, and change their colors from where the old slide has them to where the
// new slide has them, and the others fade out (the old slide's) and in (the
// new one's).
//
// An object of the old slide is paired with at most one of the new slide, in
// this order (planMorph):
//   1. both names start with "!!" and are the same: the way to make a pair
//      whatever the objects are;
//   2. the same kind of object with the same name;
//   3. the same kind of object with the same text;
//   4. the same picture;
//   5. the same kind of shape: the same preset (a pair of circles), a line with
//      a line, a table with a table.
// Where several could pair, the nearest pairs are made first.
//
// A pair of objects of one kind and one geometry is one object that moves,
// grows, turns, and changes its fill and line; a pair that differs (a circle and
// a square, a flipped one, two lines of text that are not alike) is two that move
// to the same places while one fades into the other. Text that is not the same
// fades into the other text; with "words" or "characters" the words (or
// characters) that both have move to their new places and the rest fade.
//
// A frame is drawn from the objects of the two slides as the show has them (the
// old slide with its steps played, the new one at its start), over the two
// slides' backgrounds, which fade into each other.

#include "core/animation.h"
#include "core/document.h"

#include <QColor>
#include <QImage>
#include <QSize>
#include <QStringList>
#include <QVector>
#include <memory>

class QPainter;

namespace jp {

struct MorphPair {
    enum class Rule { Forced, Name, Text, Picture, Shape };
    int from = -1, to = -1;   // places in the two lists
    Rule rule = Rule::Name;
};

struct MorphPlan {
    QVector<MorphPair> pairs;   // in the order of the new slide's objects
    QVector<int> leaving;       // the old slide's objects that have no pair, bottom first
    QVector<int> arriving;      // the new slide's
};

// The pairs between two slides' objects (the page's own, not what is inside a group).
MorphPlan planMorph(const Document &doc, const ItemList &from, const ItemList &to);

// What a morph is held to. A slide of the reference presentations has at most 49 objects, and a shape at most 1,258 characters (214 words) of text,
// so each limit is at least ten times that. A slide with more objects is not morphed: it fades. A story with more characters is not moved by
// the word or the character: it fades. The pictures that moving text and fading objects are painted into, which a morph keeps for its frames, take
// at most as many pixels as a 4K screen's frame has sixteen times (more is painted for each frame, or not moved).
constexpr int kMorphMaxObjects = 1000;
constexpr int kMorphMaxTextCharacters = 16000;
constexpr qint64 kMorphMaxTextPixels = 16LL * 3840 * 2160;
constexpr qint64 kMorphMaxLayerPixels = 16LL * 3840 * 2160;

class MorphFrames {
public:
    // The morph from slide `from` to slide `to` of `doc`, drawn `size` device-independent pixels across at `dpr`, the
    // slides centered and as large as they go with `bars` around them. `by` is "objects", "words", or "characters"
    // (core/transition.h). The states are the slides' as the show has them (core/animation.h); none: as they are.
    // `doc` must outlive this.
    MorphFrames(const Document *doc, int from, int to, const QString &by, const QSize &size, qreal dpr, const QColor &bars = Qt::black,
                const deck::States *fromStates = nullptr, const deck::States *toStates = nullptr);
    ~MorphFrames();
    MorphFrames(const MorphFrames &) = delete;
    MorphFrames &operator=(const MorphFrames &) = delete;

    // Whether the two slides can be morphed (neither holds more than kMorphMaxObjects).
    bool isUsable() const;
    const MorphPlan &plan() const;
    // The pairs and the objects without one, a line each, by name (for the command line tool).
    QStringList describe() const;
    // The frame at `progress`, 0 (the old slide) to 1 (the new one), eased by the caller. Opaque, `size` at `dpr`.
    QImage frame(double progress);
    // The frame drawn at the painter's top left.
    void paint(QPainter *p, double progress);

private:
    struct Impl;
    std::unique_ptr<Impl> d;
};

} // namespace jp
