#pragma once
// The OpenType choices of a run: ligatures, number spacing and forms, stylistic sets, and contextual alternates. The text engine shapes with them, the Font
// box sets them, and the files that can hold them read and write them, all by the numbers here.

#include "text/textprops.h"

#include <QByteArray>
#include <QFont>
#include <QMap>
#include <QString>
#include <QStringView>
#include <QTextCharFormat>
#include <utility>

namespace jp::tp::ot {

// tp::Ligatures: 0 the standard and contextual ones (the font's own), 1 none, 2 all (those and the historical and discretionary ones), or 16 plus the kinds on:
// the sum of these. (A value from 16 up names each kind on or off; 0 and 2 are what older files hold.)
inline constexpr int kLigStandard = 1, kLigContextual = 2, kLigHistorical = 4, kLigDiscretionary = 8;
inline constexpr int kLigKinds = 15;
inline constexpr int kLigNone = 1, kLigAllOlder = 2, kLigBase = 16;
// A set of kinds as the number tp::Ligatures holds.
inline int ligatureValueOfKinds(int kinds)
{
    kinds &= kLigKinds;
    return kinds == 0 ? kLigNone : kLigBase + kinds;
}
// The kinds a tp::Ligatures number turns on.
inline int ligatureKindsOf(int value)
{
    if (value == kLigNone) return 0;
    if (value == kLigAllOlder) return kLigKinds;
    if (value >= kLigBase && value < kLigBase + 16) return value - kLigBase;
    return kLigStandard | kLigContextual;
}

// tp::NumberStyle: 0 default, 1 lining, 2 old-style. tp::NumberSpacing: 0 default, 1 proportional, 2 tabular. tp::ContextualAlts: 0 default, 1 on, 2 off.
// tp::StylisticSet: 0 default, 1 to 20.
inline constexpr int kMostStylisticSet = 20;   // (a font has ss01 to ss20)

// The names a file gives a set of kinds (a word processor's w14:ligatures): none, all, or the kinds in order joined as in standardContextual. A name that is none of
// these (or is far too long) has no kinds: -1.
inline QByteArray ligatureNameOfKinds(int kinds)
{
    kinds &= kLigKinds;
    if (kinds == 0) return "none";
    if (kinds == kLigKinds) return "all";
    QByteArray name;
    for (const auto &[kind, word] : {std::pair<int, const char *>{kLigStandard, "standard"}, {kLigContextual, "contextual"}, {kLigHistorical, "historical"}, {kLigDiscretionary, "discretional"}})
        if (kinds & kind) {
            QByteArray w(word);
            if (!name.isEmpty()) w[0] = char(w[0] - 'a' + 'A');
            name += w;
        }
    return name;
}
inline int ligatureKindsOfName(QStringView name)
{
    if (name == QLatin1String("none")) return 0;
    if (name == QLatin1String("all")) return kLigKinds;
    if (name.size() > 64) return -1;
    int kinds = 0;
    if (name.contains(QLatin1String("tandard"))) kinds |= kLigStandard;
    if (name.contains(QLatin1String("ontextual"))) kinds |= kLigContextual;
    if (name.contains(QLatin1String("istorical"))) kinds |= kLigHistorical;
    if (name.contains(QLatin1String("iscretional"))) kinds |= kLigDiscretionary;
    return kinds ? kinds : -1;
}

// The text of one feature tag: "ss07" for the seventh set.
inline QFont::Tag stylisticTag(int set)
{
    return QFont::Tag::fromString(QStringLiteral("ss%1").arg(set, 2, 10, QLatin1Char('0'))).value_or(QFont::Tag());
}

// The features a run's format asks for, each with 1 (on) or 0 (off); `noLigatures` is the layout's own wish for no standard ones where the run says nothing.
inline QMap<QFont::Tag, quint32> features(const QTextCharFormat &cf, bool noLigatures)
{
    QMap<QFont::Tag, quint32> out;
    const int num = cf.intProperty(NumberStyle), sp = cf.intProperty(NumberSpacing), lig = cf.intProperty(Ligatures);
    if (num == 1) out.insert(QFont::Tag("lnum"), 1);
    if (num == 2) out.insert(QFont::Tag("onum"), 1);
    if (sp == 1) out.insert(QFont::Tag("pnum"), 1);
    if (sp == 2) out.insert(QFont::Tag("tnum"), 1);
    if (lig == kLigNone || noLigatures) {
        out.insert(QFont::Tag("liga"), 0);
        out.insert(QFont::Tag("clig"), 0);
    }
    if (lig == kLigAllOlder) {
        out.insert(QFont::Tag("dlig"), 1);
        out.insert(QFont::Tag("hlig"), 1);
    } else if (lig >= kLigBase && lig < kLigBase + 16) {
        const int kinds = lig - kLigBase;
        out.insert(QFont::Tag("liga"), kinds & kLigStandard ? 1 : 0);
        out.insert(QFont::Tag("clig"), kinds & kLigContextual ? 1 : 0);
        out.insert(QFont::Tag("hlig"), kinds & kLigHistorical ? 1 : 0);
        out.insert(QFont::Tag("dlig"), kinds & kLigDiscretionary ? 1 : 0);
    }
    const int ss = cf.intProperty(StylisticSet);
    if (ss > 0 && ss <= kMostStylisticSet) out.insert(stylisticTag(ss), 1);
    if (cf.boolProperty(Swash)) out.insert(QFont::Tag("swsh"), 1);
    if (cf.boolProperty(Alternates)) out.insert(QFont::Tag("salt"), 1);
    const int alts = cf.intProperty(ContextualAlts);
    if (alts == 1) out.insert(QFont::Tag("calt"), 1);
    if (alts == 2) out.insert(QFont::Tag("calt"), 0);
    return out;
}

} // namespace jp::tp::ot
