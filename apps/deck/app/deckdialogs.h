#pragma once
// JeffDeck's small dialogs: Header & Footer, Format Background, and the
// size of a new table.

#include "core/color.h"
#include "core/deck.h"

#include <QDialog>

class QCheckBox;
class QLineEdit;
class QSpinBox;

namespace jp {

class ColorButton;

// What to show on the slides (the date, the slide number, a line of text),
// for the slide being edited (Apply) or for all of them (Apply to All).
class HeaderFooterDialog : public QDialog {
    Q_OBJECT
public:
    HeaderFooterDialog(const deck::FooterOptions &options, QWidget *parent = nullptr);
    deck::FooterOptions options() const;
    // Whether Apply to All was chosen (else Apply, to the slide being edited).
    bool applyToAll() const { return m_all; }

private:
    QCheckBox *m_date, *m_number, *m_footer, *m_skipTitle;
    QLineEdit *m_text;
    bool m_all = false;
};

// A solid color for the background of the slide being edited (Apply) or of
// every slide (Apply to All).
class BackgroundDialog : public QDialog {
    Q_OBJECT
public:
    BackgroundDialog(const QColor &current, const ColorScheme &scheme, QWidget *parent = nullptr);
    QColor color() const { return m_color; }
    bool applyToAll() const { return m_all; }
    // The Hide background graphics box: the pictures and shapes of the slide masters under the page stop showing.
    void setHideGraphics(bool hide);
    bool hideGraphics() const;

private:
    QCheckBox *m_hide;
    ColorButton *m_button;
    QColor m_color;
    ColorScheme m_scheme;
    bool m_all = false;
};

// Rows and columns for Insert > Table > Insert Table.
class TableSizeDialog : public QDialog {
    Q_OBJECT
public:
    explicit TableSizeDialog(QWidget *parent = nullptr);
    int rows() const;
    int columns() const;

private:
    QSpinBox *m_rows, *m_cols;
};

} // namespace jp
