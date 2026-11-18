#pragma once
// The Equation tab, for every program that has Insert > Equation: the structures (fraction, script, radical, integral, large operator, bracket, function, accent, limit, operator, and
// matrix) and the symbols to put in, as the text each puts in the equation's line in each of the typing formats (a line of our own syntax, UnicodeMath, LaTeX); the ribbon commands
// and menus the tab is built from; and the two choices a program that keeps no form for an equation remembers (how it is typed, and shown). The tab itself is in each program's
// ribbon.json; the programs show it while an equation is selected (the context kEquationContext). The dialog (equationdialog.h) has the same structures in a strip of its own.

#include <QAction>
#include <QString>
#include <QStringList>
#include <QVector>
#include <functional>

class QLineEdit;
class QMenu;
class QWidget;

namespace jp {

struct RibbonParts;

// The ribbon context the tab is shown under, and the color its header has (a name for RibbonParts::colors).
constexpr const char *kEquationContext = "Equation Tools";
constexpr const char *kEquationColor = "equation";

// How an equation is typed in the dialog: the line of our own syntax, UnicodeMath, or LaTeX. The value is also what the setting keeps.
enum class EquationSyntax { Line = 0, UnicodeMath = 1, Latex = 2 };

// One way to write a structure: in each typing format, with the places to fill marked by U+2039 and U+203A around their sample text (the first is selected when it is put in). The text
// without the marks always reads as an equation in its format.
struct EquationVariant {
    QString id, name;
    QString line, unicode, latex;
};
struct EquationCategory {
    QString id, name, icon;
    QVector<EquationVariant> variants;   // the first is what the button puts in
};
// The eleven structures, in the order of the tab.
const QVector<EquationCategory> &equationCategories();

// The symbols of the symbol menus, by group.
struct EquationSymbolGroup {
    QString id, name;
    QString symbols;   // each a single character, in the order shown
};
const QVector<EquationSymbolGroup> &equationSymbolGroups();

// What a pick puts in: `pick` is "structure:<category>.<variant>", "symbol:<character>", or "text" (words that are not math, in quotes or \text), written in `syntax`, with the
// marks of a place to fill; empty for a pick that is none.
QString equationPickText(const QString &pick, EquationSyntax syntax);
// The text without the marks.
QString equationUnmarked(const QString &marked);
// Puts `marked` (an equationPickText) into the line at its caret, in place of what is selected (which becomes the first place to fill), and selects the first place to fill. Counts nothing
// past the line's own limits: the text is cut at kMostEquationInput characters.
void insertIntoEquationLine(QLineEdit *line, const QString &marked);

// What the programs that keep no form of an equation remember between times: how it is typed (Settings "equation/syntax") and whether it starts shown Linear ("equation/linear").
EquationSyntax equationSyntaxSetting();
void setEquationSyntaxSetting(EquationSyntax syntax);
bool equationLinearSetting();
void setEquationLinearSetting(bool linear);

// What a program does for the tab. `edit` opens the equation's dialog (the selected equation's, or a new one) with `pick` put in at the end of its line (empty for none); `setLinear` shows or
// puts the selected equation Linear (true) or Professional (false); `setSyntax` makes a typing format the one the dialogs open in.
struct EquationTabHandlers {
    std::function<void(const QString &pick)> edit;
    std::function<void(bool linear)> setLinear;
    std::function<void(EquationSyntax)> setSyntax;
};
// Makes the tab's commands with `make` (the program's own way of making one: id, text, icon name, what it does, checkable) as: equation.edit, equation.professional, equation.linear,
// equation.normalText, equation.syntaxLine, equation.syntaxUnicode, equation.syntaxLatex (the last five checkable, as a choice), equation.structure.<category> for each structure (its first
// variant), and equation.pick.<category>.<variant> for every variant (they hold the menus' entries).
using EquationActionMaker = std::function<QAction *(const QString &id, const QString &text, const QString &icon, const std::function<void()> &fn, bool checkable)>;
void createEquationTabActions(const EquationActionMaker &make, const EquationTabHandlers &handlers);
// Adds the tab's menus to `parts`: "equationStructure.<category>" (its variants) and "equationSymbols.<group>"; `act` finds a command by its id.
void addEquationTabMenus(RibbonParts &parts, const std::function<QAction *(const QString &id)> &act, const std::function<void(const QString &pick)> &edit);
// Shows the form and the typing format the tab's choices stand at.
void showEquationTabChoices(const std::function<QAction *(const QString &id)> &act, bool linear, EquationSyntax syntax);

// A menu of the symbols of one group (a grid of buttons would not be one menu entry each: a symbol is one entry), whose picks are `pick("symbol:<character>")`. Used by the tab and the dialog.
QMenu *equationSymbolMenu(QWidget *owner, const EquationSymbolGroup &group, const std::function<void(const QString &pick)> &pick);

} // namespace jp
