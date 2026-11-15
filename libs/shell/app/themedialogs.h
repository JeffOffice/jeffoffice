#pragma once
// The boxes and menus of themes that every app with themes shares (JeffDeck and JeffWrite): Create New Theme Colors (the twelve colors, a preview, and a name), Create New Theme Fonts
// (a heading and a body font, a preview, and a name), and the menus of the Colors and Fonts galleries (the sets a person made, the built-in ones, Create New, and Reset). The sets are
// those of libs/office/office/themelib.h; the apps apply a choice to their own documents.

#include "core/export.h"
#include "office/themelib.h"

#include <QDialog>
#include <QIcon>
#include <QMenu>
#include <QWidget>
#include <functional>

class QComboBox;
class QLineEdit;
class QPushButton;

namespace jp {

// The twelve colors as a row of swatches, over a line of sample text in the theme's two fonts.
class JO_COMMON_EXPORT ThemePreview : public QWidget {
    Q_OBJECT
public:
    explicit ThemePreview(QWidget *parent = nullptr);
    void setColors(const office::ThemeColors &colors);
    void setFonts(const QString &heading, const QString &body);
    const office::ThemeColors &colors() const { return m_colors; }
    QSize sizeHint() const override { return QSize(420, 96); }
    QSize minimumSizeHint() const override { return QSize(300, 80); }

protected:
    void paintEvent(QPaintEvent *) override;

private:
    office::ThemeColors m_colors;
    QString m_heading, m_body;
};

// Create New Theme Colors. It starts from `start`; the name is "Custom 1" and on unless the start is a custom set already. Accepting keeps the set in the user's folder (every app shows it).
class JO_COMMON_EXPORT ThemeColorsDialog : public QDialog {
    Q_OBJECT
public:
    explicit ThemeColorsDialog(const office::ThemeColors &start, QWidget *parent = nullptr);
    office::ThemeColors result() const;
    // For tests and the buttons: a slot's color, and the name.
    void setSlot(int slot, const QColor &color);
    QPushButton *slotButton(int slot) const { return m_buttons[slot]; }
    QLineEdit *nameEdit() const { return m_name; }
    ThemePreview *preview() const { return m_preview; }
    void accept() override;

private:
    void paintButton(int slot);
    office::ThemeColors m_colors;
    QPushButton *m_buttons[office::Theme::ColorCount] = {};
    QLineEdit *m_name;
    ThemePreview *m_preview;
};

// Create New Theme Fonts.
class JO_COMMON_EXPORT ThemeFontsDialog : public QDialog {
    Q_OBJECT
public:
    explicit ThemeFontsDialog(const office::ThemeFonts &start, QWidget *parent = nullptr);
    office::ThemeFonts result() const;
    QComboBox *headingBox() const { return m_heading; }
    QComboBox *bodyBox() const { return m_body; }
    QLineEdit *nameEdit() const { return m_name; }
    ThemePreview *preview() const { return m_preview; }
    void accept() override;

private:
    void updatePreview();
    QComboBox *m_heading, *m_body;
    QLineEdit *m_name;
    ThemePreview *m_preview;
};

// A name for a new set that no custom or built-in set has: "Custom 1", "Custom 2"...
QString newThemeSetName(const QStringList &taken);
// Four swatches in two rows of the set's colors, as a menu shows a scheme.
QIcon themeColorsIcon(const office::ThemeColors &colors);

// What a menu of sets does when a choice is made.
struct ThemeMenuHooks {
    std::function<void(const office::ThemeColors &)> chooseColors;
    std::function<void(const office::ThemeFonts &)> chooseFonts;
    std::function<void()> reset;   // empty: no Reset entry
    // Where the new box is shown over.
    QWidget *dialogParent = nullptr;
};

// The entries of the Theme Colors gallery's menu, after what the app puts first: the sets made by the person, `builtins`, Create New Theme Colors, and Reset. `current` is the colors the
// document has now (the set that has them is checked).
void addThemeColorsEntries(QMenu *menu, const QVector<office::ThemeColors> &builtins, const office::ThemeColors &current, const ThemeMenuHooks &hooks);
void addThemeFontsEntries(QMenu *menu, const QVector<office::ThemeFonts> &builtins, const office::ThemeFonts &current, const ThemeMenuHooks &hooks);

} // namespace jp
