#pragma once
// JeffDeck's small dialogs: Header & Footer, Format Background, and the
// size of a new table.

#include "core/color.h"
#include "core/deck.h"
#include "core/printheaderfooter.h"

#include "app/propertyeditors.h"
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QRadioButton>
#include <QDialog>
#include <functional>

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
    deck::FooterOptions m_start;   // as it was given: what the dialog doesn't show (a date's text, the field's code, ...) is given back as it was
    bool m_all = false;
};

// What to print at the top and bottom of notes pages and handouts: the date, a header, a footer, and the page number.
class PrintHeaderFooterDialog : public QDialog {
    Q_OBJECT
public:
    PrintHeaderFooterDialog(const deck::PrintHeaderFooter &start, QWidget *parent = nullptr);
    deck::PrintHeaderFooter choice() const;

private:
    QCheckBox *m_date, *m_header, *m_footer, *m_number;
    QLineEdit *m_dateText, *m_headerText, *m_footerText;
};

// A solid color for the background of the slide being edited (Apply) or of
// every slide (Apply to All).
class BackgroundDialog : public QDialog {
    Q_OBJECT
public:
    // `current`: what the page shows now. `images` shows a picture fill's picture; `pickPicture` asks for a picture and gives the id it is kept under (empty when none is chosen).
    BackgroundDialog(const Fill &current, const ColorScheme &scheme, const ImageLookup &images = {}, const std::function<QString()> &pickPicture = {}, QWidget *parent = nullptr);
    BackgroundDialog(const QColor &current, const ColorScheme &scheme, QWidget *parent = nullptr);
    // The fill chosen: solid, gradient (linear or radial, with its stops), picture, or pattern.
    Fill fill() const { return m_fill; }
    bool applyToAll() const { return m_all; }
    // The Hide background graphics box: the pictures and shapes of the slide masters under the page stop showing.
    void setHideGraphics(bool hide);
    bool hideGraphics() const;
    FillEditor *fillEditor() const { return m_editor; }

private:
    QCheckBox *m_hide;
    FillEditor *m_editor;
    Fill m_fill;
    std::function<QString()> m_pick;
    bool m_all = false;
};

// Insert > Link (Ctrl+K): a web page, an email address, a place in the presentation (the next, previous, first, or last slide, a numbered slide, or the end of the show), or a
// file. `current` is the link now there (see office/hyperlink.h), `slides` how many slides the presentation has.
class LinkDialog : public QDialog {
    Q_OBJECT
public:
    LinkDialog(const QString &current, int slides, QWidget *parent = nullptr);
    // The link chosen ("" with nothing typed), and whether Remove Link was pressed.
    QString link() const;
    bool removed() const { return m_removed; }
    // The words shown for a link with no selected words to carry it.
    QString displayText() const;
    void setDisplayShown(const QString &text, bool shown);

private:
    QComboBox *m_kind, *m_place;
    QLineEdit *m_address, *m_display;
    QLabel *m_displayLabel;
    bool m_removed = false;
};

// Design > Slide Size > Custom Slide Size: a size from the list or a width and height of its own (inches), and the slide's orientation.
class SlideSizeDialog : public QDialog {
    Q_OBJECT
public:
    SlideSizeDialog(const QSizeF &current, QWidget *parent = nullptr);
    // The size in points.
    QSizeF size() const;
    bool isPortrait() const;

private:
    void showPreset(int index);
    void orientationChosen(bool portrait);
    QComboBox *m_preset;
    QDoubleSpinBox *m_width, *m_height;
    QRadioButton *m_landscape, *m_portrait;
    bool m_loading = false;
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
