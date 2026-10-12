#pragma once
// How the slide show plays (Set Up Slide Show), and the custom slide shows.
// Both are the presentation's own and are saved with it: the settings in the
// document's appData "showSettings" (ppt/presProps.xml, p:showPr, when
// saved), the custom shows in appData "customShows" (ppt/presentation.xml,
// p:custShowLst).
//
//   showSettings: {"type": "speaker" | "individual" | "kiosk",
//                  "slides": "all" | "range" | "custom", "from": 1, "to": 1,
//                  "customShow": id (-1 for none), "loop": false,
//                  "narration": false, "animation": true, "useTimings": true,
//                  "pen": "#ff0000", "laser": "#ff0000",
//                  "scrollbar": true, "kioskRestart": 300000}
//   customShows:  [{"id": 0, "name": "...", "slides": [page id, ...]}, ...]
//
// A custom show lists slides by their page ids, in the order it plays them
// (a slide may be listed twice). Slides that are no longer in the
// presentation are gone from it when it is read, and a show left with none
// is gone too, so deleting a slide needs nothing more. What is missing or
// makes no sense takes its default. A presentation without the keys plays as
// before: every slide that is not hidden, by a speaker, full screen.

#include "core/document.h"

#include <QColor>
#include <QHash>
#include <QJsonObject>
#include <QList>
#include <QString>
#include <QStringList>
#include <QVector>

namespace jp::deck {

struct ShowSettings {
    // By a speaker (full screen), by an individual (in a window), or at a kiosk
    // (full screen, looping, moving on by the slides' timings only).
    enum class Type { Speaker, Individual, Kiosk };
    // Which slides play: all, slides `from` to `to` (counting from 1), or a custom show.
    enum class Slides { All, Range, Custom };

    Type type = Type::Speaker;
    Slides slides = Slides::All;
    int from = 1, to = 1;
    qint64 customShow = -1;
    bool loop = false;         // starts again after the last slide, until Esc
    bool narration = false;    // kept for the file; narration is not played
    bool animation = true;     // false: the slides show without their animations
    bool useTimings = true;    // false: the show moves on only when the presenter says
    QColor penColor{255, 0, 0};
    QColor laserColor{255, 0, 0};
    bool scrollbar = true;     // kept for the file (a show in a window)
    qint64 kioskRestart = 300000;   // kept for the file (milliseconds)

    // Whether the show starts again after its last slide: a kiosk always does.
    bool looping() const { return loop || type == Type::Kiosk; }
    // Whether the slides' timings move the show on: a kiosk has nothing else.
    bool usesTimings() const { return useTimings || type == Type::Kiosk; }
    bool operator==(const ShowSettings &o) const;
    bool operator!=(const ShowSettings &o) const { return !(*this == o); }
    QJsonObject toJson() const;
    static ShowSettings fromJson(const QJsonObject &o);
};

ShowSettings showSettingsOf(const Document &d);
// Stores the settings; the defaults take the key out.
void setShowSettings(Document &d, const ShowSettings &s);

struct CustomShow {
    qint64 id = 0;
    QString name;
    QStringList slides;   // page ids, in the order they play
    bool operator==(const CustomShow &o) const { return id == o.id && name == o.name && slides == o.slides; }
};

// The custom shows, their slides as the presentation has them now.
QVector<CustomShow> customShowsOf(const Document &d);
// Stores the shows (none: the key goes). Ids that repeat or are out of range
// are made new, names are cut to what a file allows.
void setCustomShows(Document &d, const QVector<CustomShow> &shows);
const CustomShow *customShowById(const QVector<CustomShow> &shows, qint64 id);
// An id no show has, and a name no show has ("Custom Show 1"...).
qint64 newCustomShowId(const QVector<CustomShow> &shows);
QString newCustomShowName(const QVector<CustomShow> &shows);
// The slides (indices) of a show, in order, for the presentation as it is.
// For many shows, the index of the pages by id is made once.
QList<int> customShowSlides(const Document &d, const CustomShow &show);
QHash<QString, int> pageIndexById(const Document &d);
QList<int> customShowSlides(const QHash<QString, int> &pages, const CustomShow &show);
// The longest name a show has.
constexpr int kMaxCustomShowName = 255;

// Which slides a show plays: `slides` in the order they go (empty: every
// slide, in order), and whether hidden slides play (a custom show plays the
// slides it lists, as it says; the others skip hidden ones).
struct ShowPlan {
    QList<int> slides;
    bool hidden = false;
};
// The plan the settings ask for; `honorSlides` false plays every slide (From Current Slide).
ShowPlan planOf(const Document &d, const ShowSettings &s, bool honorSlides = true);
ShowPlan planOfCustomShow(const Document &d, const CustomShow &show);

} // namespace jp::deck
