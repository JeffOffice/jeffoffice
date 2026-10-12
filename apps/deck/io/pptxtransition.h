#pragma once
// A slide's transition as presentation files write it (p:transition, ECMA-376
// Part 1, 19.3.1.50, and the newer effects in mc:AlternateContent), and the
// slide show's own form of it (core/transition.h).

#include "core/transition.h"

#include <QString>

namespace jp::deck {

// What a slide's p:transition (or an mc:AlternateContent holding one) says, in
// the show's terms: the effect and its direction, how long it takes (p14:dur,
// else its speed), and how the show moves on from the slide (advClick,
// advTm). An effect the show has no form for is a fade.
Transition transitionFromXml(const QString &xml);
// The p:transition for a transition, as the presentation program writes it
// (Reveal, a newer effect, inside mc:AlternateContent with a fade for programs
// that don't know it); empty for the default, which writes nothing.
QString transitionXml(const Transition &t);

} // namespace jp::deck
