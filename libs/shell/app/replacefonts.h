#pragma once
// Replace Fonts (Home > Replace Fonts): a box that lists every font a document uses, with how many times, and replaces one of them with another everywhere it is used. The box and its
// words are the same in every app; each app gives it what is its own: the fonts its document uses, and the change of one for another over its own model.

#include <QMap>
#include <QString>
#include <QVector>
#include <functional>

class QWidget;

namespace jp {

// One font a document uses. `key` is what the app is told to replace (the family's name; "+Heading" and "+Body" for the theme's two fonts, which the app lists only when it can change
// them), `shown` the words in the list, and `uses` how many places in the content name it (text, cells, shapes: not styles). A font only styles name has no uses and is listed as "styles
// only"; replacing it still changes those styles. A theme font that is not the one picked is never changed by the replacement of another.
struct FontUse {
    QString key, shown;
    int uses = 0;
    bool theme = false;
};

struct ReplaceFontsHost {
    // The fonts the document uses now, most used first (asked again each time the box is shown).
    std::function<QVector<FontUse>()> fontsUsed;
    // Puts `to` in the place of the font `key` everywhere the document uses it, as one step of Undo; how many places changed (0 when none did: no step is made then).
    std::function<int(const QString &key, const QString &to)> replace;
};

struct ReplaceFontsResult {
    int uses = 0;
    QString from, to;
    // "Replaced 5 uses of Arial with Georgia." (nothing when nothing changed).
    QString message() const;
};

// Shows the box. The result says what was replaced (uses is 0 when the box was cancelled or nothing changed).
ReplaceFontsResult askReplaceFonts(QWidget *parent, const ReplaceFontsHost &host);

// The fonts as the box lists them, made from the places that name each in the content and those in styles (families that differ only in case are one font, spelled as the first of them is
// in the maps, `content` before `styles`): most used first, then by name; a font only `styles` name has no uses. One place for every app, so that the rule is the same in all.
QVector<FontUse> fontUsesOf(const QMap<QString, int> &content, const QMap<QString, int> &styles = QMap<QString, int>());

} // namespace jp
