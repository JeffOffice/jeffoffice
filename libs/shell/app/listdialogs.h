#pragma once
// The boxes for setting up a list that every app with lists shares: Define New Bullet, Define New Number Format, and Set Numbering Value. They work on plain values (a look for a level), so each
// app puts them against its own text model (JeffWrite's numbering definitions, the paragraph bullets of JeffPub and JeffDeck).

#include <QByteArray>
#include <QColor>
#include <QString>
#include <QVector>
#include <functional>

class QWidget;

namespace jp {

// Which boxes an app offers: Full (JeffWrite: everything its files hold), Plain (formats that keep only a bullet character with its font, size, and color, and five counting styles with three
// punctuation forms: no text of one's own, no picture, no alignment), and Jpub (JeffPub's own files: Plain's styles, but the text round the number as typed, a picture for a bullet, and all nine
// levels; still no alignment).
enum class ListBoxes { Full, Plain, Jpub };

// The look of one level of a list, as the boxes say it.
struct ListLevelLook {
    bool bullet = true;
    QString text;                 // a bullet: its character; a number: the text round it ("Step %1:")
    QString numFmt = QStringLiteral("decimal");   // decimal, upperRoman, lowerRoman, upperLetter, lowerLetter
    QString font;                 // empty: the text's own
    double sizePoints = 0;        // 0: the text's own
    QColor color;                 // invalid: the text's own
    QString align = QStringLiteral("left");       // left, center, right
    int start = 1;
    QByteArray picture;           // a picture bullet: the picture's file bytes (a bullet that is a picture; `text` is then the character that stands for it)
};

// Picks a symbol for a bullet (the app's Symbol box): true when one was chosen, with its character and font.
using SymbolPicker = std::function<bool(QWidget *parent, QString *character, QString *font)>;

// Define New Bullet: a symbol (from `pick`, or typed), its font, size, and color. False when canceled; `look` is then as it was.
bool defineBulletDialog(QWidget *parent, ListLevelLook *look, const SymbolPicker &pick, ListBoxes mode = ListBoxes::Full);
// Define New Number Format: the style (1, 2, 3 / A, B, C / a, b, c / I, II, III / i, ii, iii), the text round the number (with %1 for the number), its alignment, and where it starts.
// With `plain` (JeffPub and JeffDeck, whose files hold only a bullet character with its font, size, and color, and the five counting styles with three punctuation forms) there is no alignment, and the
// number's text is chosen from the forms ("1.", "1)", "(1)", "1") rather than typed; only the first five styles are offered.
bool defineNumberFormatDialog(QWidget *parent, ListLevelLook *look, ListBoxes mode = ListBoxes::Full);
// Define New Multilevel List for the apps whose files hold a bullet or a counting style for each of nine levels: per level, a bullet (its character) or a number (style, punctuation, start at).
bool defineMultilevelPlainDialog(QWidget *parent, QVector<ListLevelLook> *levels, const SymbolPicker &pick, ListBoxes mode = ListBoxes::Plain);
// Set Numbering Value: start a new list or continue the previous one (`canContinue`: the item has a list of its kind above it), with a value to give the item. False when canceled.
struct NumberingValueChoice {
    bool continuePrevious = false;
    bool setValue = true;
    int value = 1;
};
bool setNumberingValueDialog(QWidget *parent, bool canContinue, NumberingValueChoice *choice);

// The number format names the boxes list, as "decimal" and the others are written, with what each shows. The first five are the styles every app counts; the rest (leading zero, ordinal, the numbers in words, and the formats of other scripts and systems) are for the apps whose files hold them (JeffWrite). The examples are UTF-8.
struct NumberStyleName {
    const char *id;
    const char *example;
};
const NumberStyleName *numberStyleNames(int *count);

} // namespace jp
