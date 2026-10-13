#pragma once
// How the slide show moves onto a slide, and when it moves on from it. It is
// kept in the slide's page props "transition":
//
//   {"type": "none" | "morph" | "fade" | "push" | "wipe" | "split" | "reveal" |
//            "cover" | "uncover" | "zoom" | "cut",
//    "dir": "l" | "r" | "u" | "d"  (split and zoom: "in" | "out"),
//    "by": "objects" | "words" | "characters"  (morph only),
//    "duration": seconds (0.7), "advanceAfter": seconds (absent: wait for the
//    presenter), "advanceOnClick": true}
//
// The direction is the way things move on the screen: a push "l" shoves the
// old slide off to the left as the new one comes in from the right. A cut
// goes through black. A morph moves the objects the two slides share from
// their places on the old slide to their places on the new one (app/morph.h);
// "by" says whether the unit that moves is the whole object, or the words or
// the characters of its text. The settings of a slide say how the show gets
// onto it; "advanceAfter" and "advanceOnClick" say how it gets off it.

#include "core/document.h"

#include <QJsonObject>
#include <QStringList>
#include <optional>

namespace jp::deck {

struct Transition {
    QString type = QStringLiteral("none");
    QString dir = QStringLiteral("l");
    QString by = QStringLiteral("objects");   // a morph's Effect Option
    double duration = 0.7;
    std::optional<double> advanceAfter;
    bool advanceOnClick = true;

    // Whether the type goes in a direction (push, wipe, reveal, cover, uncover)
    // or in or out (split, zoom); the others have neither.
    bool hasDirection() const;
    bool hasInOut() const;
    // Whether the type is a morph, which has Objects, Words, and Characters.
    bool isMorph() const;
    // What a morph moves: its Effect Options, in the order they are listed.
    static QStringList morphUnits();
    // How long a morph takes when it is chosen (the others take 0.7).
    static constexpr double kMorphSeconds = 2.0;
    bool operator==(const Transition &o) const;
    bool operator!=(const Transition &o) const { return !(*this == o); }
    QJsonObject toJson() const;
    // Anything missing or out of range takes its default.
    static Transition fromJson(const QJsonObject &o);
    // The direction a type starts with.
    static QString defaultDirection(const QString &type);
};

// The types, in the order the Transitions tab lists them.
QStringList transitionTypes();

Transition transitionOf(const Page &slide);
// Stores the transition in the slide's props; the default takes the key out.
void setTransition(Page &slide, const Transition &t);

} // namespace jp::deck
