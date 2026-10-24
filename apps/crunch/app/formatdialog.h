#pragma once
// The Format Cells dialog: six tabs (Number, Alignment, Font, Border, Fill, Protection) for the cells selected. It is shown the format of the cell with the cursor, and gives back only what the
// person changed (so that cells that differ from it are left as they are in what was not touched): a FormatChange for the font, number format, alignment, fill, and protection, a list of
// border lines for the edges of the block (the outside, the lines between its cells, the diagonals), and whether to merge or unmerge.

#include "calc/dates.h"
#include "calc/numfmt.h"
#include "sheet/styles.h"

#include <QColor>
#include <QDialog>
#include <QLocale>
#include <QStringList>
#include <optional>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QSpinBox;
class QStackedWidget;
class QTabWidget;
class QToolButton;

namespace jp {

class ColorButton;

// The format of the cell with the cursor, as the dialog starts from it (colors resolved to RGB).
struct FormatView {
    QString numberFormat = QStringLiteral("General");
    QString fontName;                 // the typeface the cell is set in
    double fontSize = 11;
    bool bold = false, italic = false, strike = false;
    jp::office::sheet::Underline underline = jp::office::sheet::Underline::None;
    quint8 vertAlign = 0;
    QColor fontColor;                 // invalid for automatic
    jp::office::sheet::PatternKind pattern = jp::office::sheet::PatternKind::None;
    QColor fillColor, patternColor;   // the background color (invalid for none) and the color of the pattern
    jp::office::sheet::BorderSpec border;   // with colors as RGB
    jp::office::sheet::HAlign horizontal = jp::office::sheet::HAlign::General;
    jp::office::sheet::VAlign vertical = jp::office::sheet::VAlign::Bottom;
    bool wrap = false, shrink = false;
    int indent = 0;
    int rotation = 0;                 // as the file has it: 0 to 90 up, 91 to 180 down, 255 one letter under another
    int readingOrder = 0;
    bool locked = true, formulaHidden = false;
    bool merged = false;              // the cell with the cursor is in a merged range
};

class FormatCellsDialog : public QDialog {
    Q_OBJECT
public:
    enum Tab { NumberTab, AlignmentTab, FontTab, BorderTab, FillTab, ProtectionTab };

    struct Input {
        FormatView view;
        QLocale locale;
        jp::calc::DateSystem dateSystem = jp::calc::DateSystem::Date1900;
        // the value the Number tab's sample shows: a number, else a text
        bool sampleIsNumber = true;
        double sampleNumber = 1234.5;
        QString sampleText = QStringLiteral("Text");
        QStringList customCodes;      // the codes the workbook defines, offered under Custom
        bool oneCell = true;          // the selection is one cell (the lines between cells are not offered)
        QVector<QColor> palette;      // the workbook's theme colors for the color buttons (the first is the text color, then six accents); empty for the usual ones
    };

    // One line to put on some of the edges of the block (the bits of EditSession::BorderEdge); a line of no style takes them away.
    struct BorderOp {
        int edges = 0;
        jp::office::sheet::BorderSide line;
    };

    struct Result {
        jp::office::sheet::FormatChange change;
        QVector<BorderOp> borders;
        std::optional<bool> merge;    // true to merge the cells, false to take the merge away
        bool isEmpty() const;
    };

    explicit FormatCellsDialog(const Input &input, QWidget *parent = nullptr);
    void showTab(int tab);
    Result result() const;
    // The number format code the Number tab has now.
    QString code() const { return m_code; }

private:
    void buildNumber(QWidget *page);
    void buildAlignment(QWidget *page);
    void buildFont(QWidget *page);
    void buildBorder(QWidget *page);
    void buildFill(QWidget *page);
    void buildProtection(QWidget *page);
    void setCode(const QString &code, bool fromControls);
    void numberControlsChanged();
    void categoryChanged(int row);
    void updateSample();
    void updateFontPreview();
    void updateFillSample();
    void alignmentChanged();
    void toggleEdge(int edge);
    void setEdge(int edge, const jp::office::sheet::BorderSide &line);
    void refreshBorderButtons();
    jp::office::sheet::BorderSide currentLine() const;
    jp::office::sheet::ColorSpec specOf(const QColor &c) const;
    QColor colorOf(ColorButton *b) const;

    Input m_in;
    QTabWidget *m_tabs = nullptr;
    // number
    QString m_code;
    bool m_building = false;
    QListWidget *m_categories = nullptr;
    QStackedWidget *m_pages = nullptr;
    QSpinBox *m_decimals = nullptr;
    QCheckBox *m_thousands = nullptr;
    QListWidget *m_negatives = nullptr;
    QComboBox *m_symbol = nullptr;
    QListWidget *m_types = nullptr;
    QLineEdit *m_custom = nullptr;
    QLabel *m_sample = nullptr, *m_description = nullptr;
    QWidget *m_optionsBox = nullptr, *m_negativeBox = nullptr, *m_symbolBox = nullptr, *m_typesBox = nullptr, *m_customBox = nullptr, *m_thousandsBox = nullptr;
    // alignment
    QComboBox *m_horizontal = nullptr, *m_vertical = nullptr, *m_direction = nullptr;
    QSpinBox *m_indent = nullptr, *m_degrees = nullptr;
    QCheckBox *m_wrap = nullptr, *m_shrink = nullptr, *m_merge = nullptr, *m_verticalText = nullptr;
    // font
    QComboBox *m_fontName = nullptr, *m_fontStyle = nullptr, *m_underline = nullptr, *m_fontSize = nullptr;
    ColorButton *m_fontColor = nullptr;
    QCheckBox *m_strike = nullptr, *m_superscript = nullptr, *m_subscript = nullptr;
    QLabel *m_fontPreview = nullptr;
    bool m_fontColorTouched = false;
    // border
    QListWidget *m_lineStyles = nullptr;
    ColorButton *m_lineColor = nullptr;
    QToolButton *m_edgeButtons[8] = {};
    QWidget *m_borderPreview = nullptr;
    jp::office::sheet::BorderSide m_edgeLines[8];
    bool m_edgeChanged[8] = {};
    // fill
    ColorButton *m_fillColor = nullptr, *m_patternColor = nullptr;
    QComboBox *m_patternStyle = nullptr;
    QLabel *m_fillSample = nullptr;
    bool m_fillTouched = false;
    // protection
    QCheckBox *m_locked = nullptr, *m_hidden = nullptr;
    friend class BorderPreview;
};

} // namespace jp
