#pragma once
// The boxes for setting up a list that every app with lists shares: Define New Bullet, Define New Number Format, and Set Numbering Value. They work on plain values (a look for a level), so each
// app puts them against its own text model (JeffWrite's numbering definitions, the paragraph bullets of JeffPub and JeffDeck).

#include <QColor>
#include <QString>
#include <functional>

class QWidget;

namespace jp {

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
};

// Picks a symbol for a bullet (the app's Symbol box): true when one was chosen, with its character and font.
using SymbolPicker = std::function<bool(QWidget *parent, QString *character, QString *font)>;

// Define New Bullet: a symbol (from `pick`, or typed), its font, size, and color. False when canceled; `look` is then as it was.
bool defineBulletDialog(QWidget *parent, ListLevelLook *look, const SymbolPicker &pick);
// Define New Number Format: the style (1, 2, 3 / A, B, C / a, b, c / I, II, III / i, ii, iii), the text round the number (with %1 for the number), its alignment, and where it starts.
bool defineNumberFormatDialog(QWidget *parent, ListLevelLook *look);
// Set Numbering Value: start a new list or continue the previous one (`canContinue`: the item has a list of its kind above it), with a value to give the item. False when canceled.
struct NumberingValueChoice {
    bool continuePrevious = false;
    bool setValue = true;
    int value = 1;
};
bool setNumberingValueDialog(QWidget *parent, bool canContinue, NumberingValueChoice *choice);

// The number format names the boxes list, as "decimal" and the others are written, with what each shows.
struct NumberStyleName {
    const char *id;
    const char *example;
};
const NumberStyleName *numberStyleNames(int *count);

} // namespace jp
