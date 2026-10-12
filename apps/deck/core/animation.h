#pragma once
// What moves on a slide, and when, as the slide show plays it. A slide keeps
// its animations as the file wrote them (page props "timingXml", the slide's
// p:timing element); readTimeline() reads them into click steps. The main
// sequence's steps each start at a click (the first may start by itself when
// the slide is shown); an step has effects (entrance, exit, emphasis, motion
// path) that start with the click, with the effect before, or after it, and
// each effect is made of behaviors that change one thing about a shape:
// show or hide it (set), fade or wipe it (effect), move, resize, or turn it
// (animate), send it along a path, scale it, or rotate it. Behaviors are
// played as they are written, not by the preset they came from.
//
// Where the shapes are at a moment is a pure function of the timeline, the
// shapes' places, and the moment: statesAt(). The show paints a slide with
// those states (animate()), a step at a time: the steps before it at their
// ends, the one being played `ms` into it.
//
// Played: show and hide, fade and wipe (a wipe is a clip; every other filter
// is drawn as a fade), moving, resizing, turning, scaling, rotating, and
// motion paths, with each behavior's delay, duration, speeding up and slowing
// down, reversing, and repeats (a repeat "until the end" is played once);
// builds by paragraph. Read but not played: colors (animClr), commands
// (media calls), triggers (interactive sequences), and what changes text
// by letter or word (a shape's text range by characters moves the shape).

#include "core/document.h"
#include "render/renderer.h"

#include <QHash>
#include <QPainterPath>
#include <QPointF>
#include <QRectF>
#include <QSizeF>
#include <QString>
#include <QVector>

namespace jp::deck {

// What a behavior aims at: a shape by the number the file gave it
// (Item::sourceId), or a range of its paragraphs (a build by paragraph).
struct Target {
    int shape = 0;
    int firstParagraph = -1, lastParagraph = -1;   // -1: the whole shape
    bool paragraphs() const { return firstParagraph >= 0; }
    bool operator==(const Target &o) const { return shape == o.shape && firstParagraph == o.firstParagraph && lastParagraph == o.lastParagraph; }
    bool operator!=(const Target &o) const { return !(*this == o); }
};
size_t qHash(const Target &t, size_t seed = 0) noexcept;

// When a behavior runs. Times are milliseconds after the step starts.
struct Timing {
    double begin = 0;              // all the delays above it added
    double duration = -1;          // one run; -1 when the file says "indefinite" or nothing
    double accel = 0, decel = 0;   // the shares of one run spent speeding up and slowing down
    bool autoReverse = false;      // each run goes back after it goes
    double repeat = 1;             // runs; -1 for "until the end", which is played once
    double repeatDuration = -1;    // a cap on all the runs; -1 for none
    bool hold = false;             // what it did stays after it ends

    // How long it lasts, repeats and going back included; 0 when its duration is indefinite.
    double activeDuration() const;
    double end() const { return begin + activeDuration(); }
    enum class Phase { Before, Running, After };
    // Where it is `ms` after the step started: how far through a run (0 to 1,
    // eased) while it is running, and at its end after it. A duration that is
    // indefinite counts as done the moment it begins.
    Phase phaseAt(double ms, double *progress) const;
};

struct Keyframe {
    double at = 0;      // 0 to 1 of one run
    QString value;      // a number, or a formula over #ppt_x, #ppt_y, #ppt_w, #ppt_h, +, -, *, /, and parentheses
};

struct Behavior {
    enum class Kind { Set, Effect, Animate, Motion, Scale, Rotate, Color, Command };
    Kind kind = Kind::Set;
    Target target;
    Timing timing;
    // Set, Animate, Color: what it changes ("style.visibility", "ppt_x", "ppt_w", "style.rotation", "style.opacity", ...).
    QString attribute;
    // Set: the value it takes. Animate, when it has no keyframes: where it goes from and to, or by how much.
    QString to, from, by;
    QVector<Keyframe> keys;          // Animate
    QString additive;                // Animate: "base" (the default), "sum", "repl", "mult", or "none"
    bool discrete = false;           // Animate: the value jumps from one keyframe to the next
    QString filter;                  // Effect: "fade", "wipe(left)", ...
    bool entering = true;            // Effect: the shape comes in (false: it goes out)
    QPainterPath path;               // Motion: fractions of the slide, from where the shape is
    QString origin, pathEditMode;    // Motion: as the file says ("layout", "relative", "fixed"); the path is played from where the shape is either way
    QPointF scaleFrom{1, 1}, scaleTo{1, 1};   // Scale: 1 is the size it has
    double rotateFrom = 0, rotateTo = 0;      // Rotate: degrees clockwise
    QString command;                 // Command
};

enum class EffectClass { Entrance, Exit, Emphasis, MotionPath, Media, Verb, Other };
enum class EffectStart { OnClick, WithPrevious, AfterPrevious };

struct Effect {
    EffectClass cls = EffectClass::Other;
    EffectStart start = EffectStart::OnClick;
    int presetId = 0, presetSubtype = 0;
    double begin = 0;                // ms after its step starts
    QVector<Behavior> behaviors;
    // ms after its step starts, when its last behavior with an end has ended.
    double end() const;
    // Whether it changes anything on the slide (a media call and a color change don't).
    bool visual() const;
};

struct Step {
    QVector<Effect> effects;
    bool automatic = false;          // starts by itself when the slide is shown, not at a click
    double duration() const;         // ms from the start to the last effect's end
    bool visual() const;             // some effect shows something
};

// An interactive sequence: effects that play when a shape is clicked. Read, not played.
struct Trigger {
    int shape = 0;
    QVector<Effect> effects;
};

struct Timeline {
    QVector<Step> steps;             // the main sequence
    QVector<Trigger> triggers;
    bool isEmpty() const { return steps.isEmpty(); }
};

// Reads a p:timing element (with xmlns:p declared). Nothing wrong with it
// stops it: what can't be read is left out. The steps keep the file's order.
Timeline readTimeline(const QString &timingXml);
// The slide's own: its page props "timingXml".
Timeline timelineOf(const Page &slide);
// The timeline as the show plays it: steps that show nothing (only a media
// call) take no click.
Timeline playableTimeline(const Page &slide);

// How a shape is, partway through its animations.
struct TargetState {
    bool visible = true;
    double opacity = 1;
    QPointF offset;                  // how far it is from where the slide has it, in points
    double scaleX = 1, scaleY = 1;   // about its middle
    double rotation = 0;             // degrees clockwise, about its middle
    QRectF clip;                     // the part shown, as shares of its width and height; null for all of it
    bool operator==(const TargetState &o) const;
    bool operator!=(const TargetState &o) const { return !(*this == o); }
};
using States = QHash<Target, TargetState>;

// Where the shapes are on the slide, by the number the file gave each, for
// the formulas (#ppt_x is the middle's share of the slide's width...).
struct Geometry {
    QSizeF slide;
    QHash<int, QRectF> shapes;
};
Geometry geometryOf(const Document &doc, const Page &slide);

// The state of every target an effect touches after `done` steps have played,
// and, with `ms` of 0 or more, the next step `ms` into it. A target whose
// first effect is an entrance starts hidden; the others start in place.
States statesAt(const Timeline &timeline, const Geometry &geometry, int done, double ms = -1);

// The value of a formula over numbers, #names from `variables`, + - * /, and
// parentheses. `ok` is false (and NaN comes back) for what it can't read.
double evaluateFormula(const QString &formula, const QHash<QString, double> &variables, bool *ok = nullptr);

// Has `ctx` paint the page with each shape as `states` have it: hidden,
// faded, wiped, moved, scaled, or turned, and the paragraphs of a text box
// that are targets one by one. `doc` must outlive the context.
void animate(PaintContext &ctx, const Document &doc, const States &states);

} // namespace jp::deck
