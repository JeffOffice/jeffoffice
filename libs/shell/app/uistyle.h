#pragma once
// The one dialog style of the four apps (docs/design/STYLE.md): the pieces a dialog, a message box, a File page, or a side pane is made of, so that most of them need no
// code of their own to look right. The colors come from icons.h (uiText, uiMuted, uiAccent, uiDanger, uiPanel); the style sheet is modernStyleSheet().

#include "core/export.h"

#include <QDialogButtonBox>
#include <QGroupBox>
#include <QMessageBox>
#include <QPixmap>
#include <QString>

class QFormLayout;
class QFrame;
class QLabel;
class QLayout;
class QPushButton;
class QWidget;

namespace jp {

// ---- pieces ----
// A small grey UPPERCASE heading that opens a group of rows (11 px, medium weight). The label's accessible name is the text as given.
JO_COMMON_EXPORT QLabel *sectionHeading(const QString &text, QWidget *parent = nullptr);
// A group of rows under such a heading, with no frame: a QGroupBox (so a layout is made on it as on any box) whose title the style sheet draws as the heading. The title is
// stored in capitals; `heading()` is the text as given.
class JO_COMMON_EXPORT Section : public QGroupBox {
public:
    explicit Section(const QString &heading, QWidget *parent = nullptr);
    QString heading() const { return m_heading; }

private:
    QString m_heading;
};
// The form layout of the style: one label column (labels left, centered on their fields), rows 8 apart, 12 between a label and its field.
JO_COMMON_EXPORT void styleForm(QFormLayout *form);
// Two fields on one row, each with a small muted label above it.
JO_COMMON_EXPORT QWidget *twoUp(const QString &labelA, QWidget *a, const QString &labelB, QWidget *b, QWidget *parent = nullptr);
// Two checkboxes (or any two small controls) on one line.
JO_COMMON_EXPORT QWidget *checkPair(QWidget *a, QWidget *b, QWidget *parent = nullptr);
// A button that looks like a link: accent text, no border.
JO_COMMON_EXPORT QPushButton *linkButton(const QString &text, QWidget *parent = nullptr);
// A soft grey panel for a preview (no sunken frame).
JO_COMMON_EXPORT QFrame *previewPanel(QWidget *parent = nullptr);
// A muted note under what it explains (12 px, wraps, shown as text).
JO_COMMON_EXPORT QLabel *noteLabel(const QString &text, QWidget *parent = nullptr);

// ---- buttons ----
// A button row in the order of the style: a destructive choice (DestructiveRole, or ResetRole) alone at the left; the rest at the right, Cancel before the primary; the primary
// (the default button, else the accept button) accent-filled; no icons. Every QDialogButtonBox of the program gets this when it is shown, this class does it at once too.
class JO_COMMON_EXPORT ButtonBox : public QDialogButtonBox {
public:
    explicit ButtonBox(QWidget *parent = nullptr);
    explicit ButtonBox(StandardButtons buttons, QWidget *parent = nullptr);
    ButtonBox(StandardButtons buttons, Qt::Orientation orientation, QWidget *parent = nullptr);
    // A button at the left, on its own, for a choice that throws something away (Don't Save, Delete, Discard).
    QPushButton *addDestructive(const QString &text);

protected:
    bool event(QEvent *e) override;
};
// Does to `box` what ButtonBox does (the program's filter calls this for every box shown).
JO_COMMON_EXPORT void arrangeButtons(QDialogButtonBox *box);

// Installs the filter that gives every other button box the style when it is shown (installUiPolish calls this).
JO_COMMON_EXPORT void installButtonBoxStyle();

// ---- message boxes ----
// The sign at the left of a message box, by kind: a red triangle (Warning), a red circle with an X (Critical), a blue circle with an "i" (Information), a blue circle with a
// "?" (Question). `size` is in device-independent pixels.
JO_COMMON_EXPORT QPixmap boxSign(QMessageBox::Icon kind, int size = 32, qreal pixelRatio = 1.0);
// A QMessageBox in the style: the sign, the first sentence of the text in bold with the rest (and the informative text) as a grey line under it, the buttons as ButtonBox
// puts them. text(), informativeText(), button(), and clickedButton() are those of a QMessageBox.
class JO_COMMON_EXPORT MessageBox : public QMessageBox {
public:
    explicit MessageBox(QWidget *parent = nullptr);
    MessageBox(Icon icon, const QString &title, const QString &text, StandardButtons buttons = NoButton, QWidget *parent = nullptr);
    // The first sentence of `text` and what follows it.
    static void splitText(const QString &text, QString *lead, QString *rest);

protected:
    void showEvent(QShowEvent *e) override;

private:
    void restyle();
    bool m_styled = false;
};

// The box that asks about unsaved changes when a window closes, the same in all four apps: a red triangle; "Save changes to "<name>"?"; the grey line "If you don't save, your
// changes since the last save are lost."; Don't Save at the left; Cancel and Save (the default) at the right. Returns Save, Discard, or Cancel.
JO_COMMON_EXPORT QString saveChangesText(const QString &name);
JO_COMMON_EXPORT QString saveChangesDetail();
JO_COMMON_EXPORT MessageBox *saveChangesBox(QWidget *parent, const QString &name);   // not shown yet; the caller owns it
JO_COMMON_EXPORT QMessageBox::StandardButton askSaveChanges(QWidget *parent, const QString &name);

} // namespace jp
