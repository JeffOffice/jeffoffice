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
// Played: show and hide, fade, wipe, and split (a clip; every other filter
// is drawn as a fade), moving, resizing, turning, scaling, rotating, and
// motion paths, with each behavior's delay, duration, speeding up and slowing
// down, reversing, and repeats (a repeat "until the end" is played once);
// builds by paragraph; hiding a shape after its effect, or at the next click. Read but not played: colors (animClr), commands
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
#include <QStringList>
#include <QVector>
#include <functional>

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
    QVector<QPointF> filter;       // how far through a run maps to how far it shows (tmFilter), as points from 0 to 1; empty for none

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
    bool afterwards = false;         // hides its shape when its effect is over: not part of the effect's own time
};

enum class EffectClass { Entrance, Exit, Emphasis, MotionPath, Media, Verb, Other };
enum class EffectStart { OnClick, WithPrevious, AfterPrevious };

struct Effect {
    EffectClass cls = EffectClass::Other;
    EffectStart start = EffectStart::OnClick;
    int presetId = 0, presetSubtype = 0;
    double begin = 0;                // ms after its step starts
    double repeat = 1;               // times the whole effect plays (a repeat "until the end" counts once)
    bool remove = false;             // it takes its work with it when it ends (rewind when done playing)
    bool hideOnNextClick = false;    // its shape is hidden when the next click step starts
    bool implied = false;            // not in the file: the hiding another effect asks for at this step's click
    QVector<Behavior> behaviors;
    // ms after its step starts, when its last behavior with an end has ended
    // (once() is how long one play takes); end() counts the repeats.
    double once() const;
    // How many times it plays: its repeat, but not for longer than twenty seconds (a file that asks for thousands would stall the show).
    double repeats() const;
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
// The same without the effects of shapes the slide no longer has (or paragraphs it no longer has): they take no click.
Timeline playableTimeline(const Document &doc, const Page &slide);

// How a shape is, partway through its animations.
struct TargetState {
    bool visible = true;
    double opacity = 1;
    QPointF offset;                  // how far it is from where the slide has it, in points
    double scaleX = 1, scaleY = 1;   // about its middle
    double rotation = 0;             // degrees clockwise, about its middle
    QRectF clip;                     // the part shown, as shares of its width and height; null for all of it
    QRectF clip2;                    // with clip, a second part shown (a split shows two strips); null for none
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


// ---- Making and changing effects ----
//
// The animation pane lists the slide's main sequence as Anim: an effect JeffDeck
// makes (a kind from effectKinds(), a way it goes, a shape or some of its
// paragraphs, when it starts, how long it takes) or, for what the file holds
// and JeffDeck has no form for, one it keeps as the file wrote it ("Custom":
// it can be moved, removed, and made to start another way, nothing more).
// readAnimations() reads the slide's timing this way; setAnimations() writes
// the sequence back as the slide's timing, as the presentation program writes
// it (click steps, effects that start with or after the one before, each
// effect's preset number and behaviors, and the builds), so that program lists
// each effect by its own name. An effect is "known" only when the program
// would write the very XML that was read for it; any other stays Custom, so
// nothing a file holds is changed. Interactive sequences and what else the
// timing holds are kept as they were.

// One way an effect goes: a direction, or one motion path.
struct EffectVariant {
    int presetId = 0, subtype = 0;
    QString key;     // "bottom", "in", "p42": stable, for actions
    QString name;    // "From Bottom"
};

struct EffectKind {
    enum class Amount { None, Size, Spin };
    QString id;      // "fade", "fly", "growshrink"
    QString name;
    QString group;   // motion paths: "Lines", "Arcs", "Turns", "Shapes", "Loops", "Curves"
    EffectClass cls = EffectClass::Entrance;
    QVector<EffectVariant> variants;   // the first is the one a new effect has
    int duration = 0;                  // milliseconds a new effect takes; 0 when it has no length (an appearing shape)
    Amount amount = Amount::None;      // Size: a percentage and a direction (both, horizontal, vertical); Spin: degrees and a turning way
    bool smooth = false;               // starts and ends smoothly unless told not to (motion paths)
    const EffectVariant *variant(const QString &key) const;
    const EffectVariant &firstVariant() const { return variants.first(); }
};
// Entrance, then Exit, Emphasis, and Motion Path kinds, each in the order the ribbon lists them.
const QVector<EffectKind> &effectKinds();
const EffectKind *effectKind(const QString &id);
// Where the kinds of a class that come in groups end (motion paths), in order.
QStringList motionPathGroups();

// What an effect does to its shape when it is over: nothing, hide it at once, or hide it at the next click.
enum class AfterEffect { None, Hide, HideOnNextClick };

struct Anim {
    QString kind;                    // EffectKind::id; empty: kept as written
    EffectClass cls = EffectClass::Entrance;
    QString variant;                 // EffectVariant::key
    int shape = 0;                   // the shape's number (Item::sourceId)
    int firstParagraph = -1, lastParagraph = -1;   // the paragraphs it aims at; -1 for the whole shape
    EffectStart start = EffectStart::OnClick;
    int duration = 0, delay = 0;     // ms
    bool smoothStart = false, smoothEnd = false;
    bool rewind = false;             // goes back to how it was when it ends
    AfterEffect after = AfterEffect::None;
    double repeat = 1;
    double amount = 0;               // growshrink: percent (150); spin: degrees (360)
    int group = -1;                  // effects of one shape with the same number are one build; -1 until written
    QString xml;                     // a kept effect: the p:par as the file wrote it
    bool custom() const { return kind.isEmpty(); }
    bool paragraphs() const { return firstParagraph >= 0; }
    // How long it lasts, delay and repeats included; 0 for an effect with no length.
    int span() const;
};

struct AnimSequence {
    QVector<Anim> effects;           // the main sequence, in order
    QStringList kept;                // the other nodes under the timing's root (sequences that wait for a click on a shape, a movie's or sound's node), as the file wrote them
    int keptBeforeMain = 0;          // how many of them come before the main sequence
    QString declarations;            // the namespaces the timing declares, as XML attributes
    QString extension;               // what follows the root node in the timing, as written
    QStringList builds;              // builds of what the file keeps (charts, diagrams), as written
    bool editable = true;            // false when the sequence is too long to be changed safely
    bool isEmpty() const { return effects.isEmpty() && kept.isEmpty(); }
    // The click that starts effect `i`: 1 for the first, 0 for those that play when the slide shows.
    int clickOf(int i) const;
    // How many clicks the effects take.
    int clicks() const;
    // Milliseconds after its click that effect `i` starts and ends (as the program draws them).
    int beginOf(int i) const;
    int endOf(int i) const;
    // New effects: `kind` aimed at `shape`, at the end, or at `at`; returns its index.
    int add(const Anim &anim, int at = -1);
    void remove(int i);
    // The effect at `from` moves to `to` (the others keep their order).
    void move(int from, int to);
    // The effects aimed at `shape`, in order.
    QVector<int> effectsOf(int shape) const;
};
// An effect of `kind` (its first way) aimed at `shape`, with the kind's own length, smoothness, and amount.
Anim makeAnim(const EffectKind &kind, int shape);
Anim makeAnim(const QString &kind, int shape);
// The effect with another kind: it keeps what it is aimed at and how it starts and when; the rest is the new kind's.
void changeKind(Anim &anim, const EffectKind &kind);

// With the document, the effects of shapes the slide no longer has, and of paragraphs it no longer has, are left out.
// A timing that can't be read is not editable.
AnimSequence readAnimations(const Page &slide, const Document *doc = nullptr);
// The XML of the slide's timing for the sequence, aiming only at shapes that are on the slide (an effect aimed at one that isn't is dropped);
// empty when nothing is left.
QString animationsXml(const Document &doc, const Page &slide, const AnimSequence &seq);
// Stores it as the slide's timing (page props "timingXml"; none when nothing is left).
void setAnimations(const Document &doc, Page &slide, const AnimSequence &seq);
// The number the file would know the item by (Item::sourceId); one that has none gets the next free one. An effect aims at a shape by it. 0 if the slide has no such item.
int shapeNumber(Page &slide, const QString &itemId);

// Builds by paragraph: the paragraphs of a shape's text that animate together (a paragraph, and the ones below it that are indented more).
QVector<QPair<int, int>> paragraphGroups(const Document &doc, const Page &slide, int shape);
// Aims effect `i` at the whole shape, or splits it into one effect per group of paragraphs (the first keeps how it starts; the others start on a click, or after it when it did not).
void setByParagraph(AnimSequence &seq, int i, bool byParagraph, const Document &doc, const Page &slide);
// Copies the effects aimed at `from` onto `to`.
void copyEffects(AnimSequence &seq, int from, const QVector<int> &to);

// A number that changes when what the animation rows show changes: the slide's timing, and the shapes (their numbers, names, and paragraphs)
// it aims at; 0 for a slide with no timing. It is cheap, for being asked after every change.
quint64 animationSignature(const Document &doc, const Page &slide);

// The slide's timing for saving: the effects aimed at shapes `present` says are not in the file are left out, the rest as the sequence writes them;
// the timing as it is when none is, or when it can't be read as a sequence. `dropped` says how many effects went.
QString timingForSave(const Document &doc, const Page &slide, const std::function<bool(int shape)> &present, int *dropped = nullptr);

} // namespace jp::deck
