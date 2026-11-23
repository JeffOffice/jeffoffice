#pragma once
// The windows of conditional formatting: one rule (New Rule, and Edit Rule in Manage Rules) and the list of a sheet's rules (Manage Rules).

#include "app/condlook.h"
#include "core/editsession.h"
#include "sheet/sheetmodel.h"

#include <QDialog>
#include <QPushButton>
#include <QVector>

#include <functional>

class QCheckBox;
class QComboBox;
class QFormLayout;
class QLineEdit;
class QSpinBox;
class QStackedWidget;
class QTableWidget;

namespace jp {

// What a rule is to be, as the window asks for it: the kind, what it compares with (as typed), and for data bars, color scales, and icon sets the settings of the rule itself.
struct CondSpec {
    enum class Kind {
        CellValue, TextContains, TextNotContains, TextBegins, TextEnds, DateOccurring, Blanks, NoBlanks, Errors, NoErrors, Top, Bottom, TopPercent, BottomPercent, Above, Below, Duplicate, Unique,
        Formula, ColorScale, DataBar, IconSet
    } kind = Kind::CellValue;
    jp::office::sheet::CondRule::Operator op = jp::office::sheet::CondRule::Operator::GreaterThan;
    QString v1, v2;          // the values (or the formula), as typed
    QString text;            // the text a text rule looks for
    QString period;          // the dates a rule looks for, as the file names them
    int count = 10;
    jp::office::sheet::CondRule visual;   // the settings of a color scale, data bar, or icon set
    int look = 0;            // a ready-made look (a place in condpresets)
    bool keepLook = false;   // the rule's own look stays as it is
    bool useCustom = false;  // the person's own look (Custom Format): `custom`
    jp::office::sheet::DifferentialFormat custom;
    jp::office::sheet::DifferentialFormat currentLook;   // the look of the rule that is being edited (what the list shows for "keep", and where a custom look starts)
    bool stopIfTrue = false;
    // Whether the kind has a look of font and fill (a color scale, data bar, or icon set has none).
    bool hasLook() const { return kind != Kind::ColorScale && kind != Kind::DataBar && kind != Kind::IconSet; }
};

// The spec of a rule that exists (what Edit Rule starts from; its look stays unless another is chosen).
CondSpec specOfRule(const jp::office::sheet::CondRule &rule);

// A button that shows a color and asks for another when it is pressed.
class CondColorButton : public QPushButton {
    Q_OBJECT
public:
    explicit CondColorButton(const jp::office::sheet::ColorSpec &color, QWidget *parent = nullptr);
    jp::office::sheet::ColorSpec color() const { return m_color; }
    void setColor(const jp::office::sheet::ColorSpec &color);

private:
    jp::office::sheet::ColorSpec m_color;
};

class CondRuleDialog : public QDialog {
    Q_OBJECT
public:
    // `editing`: the rule is one that exists (its look may stay as it is).
    CondRuleDialog(const CondSpec &start, bool editing, QWidget *parent = nullptr);
    CondSpec spec() const;
    // What Custom Format needs to open its box (the workbook's styles and colors); a test sets a stand-in for the box here.
    void setLookContext(const CondLookContext &context);
    CondLookBox *lookBox() const { return m_lookBox; }
    // The kind the person is making (for a test, and for the pages).
    void setKind(CondSpec::Kind kind);

private:
    void build(const CondSpec &start, bool editing);
    void showPage();
    QWidget *scalePage(const jp::office::sheet::CondRule &rule);
    QWidget *barPage(const jp::office::sheet::CondRule &rule);
    QWidget *iconPage(const jp::office::sheet::CondRule &rule);
    void fillIconRows();

    CondSpec m_start;
    QComboBox *m_kind = nullptr;
    CondLookBox *m_lookBox = nullptr;
    QFormLayout *m_looksForm = nullptr;
    QStackedWidget *m_pages = nullptr;
    QComboBox *m_op = nullptr, *m_period = nullptr;
    QLineEdit *m_v1 = nullptr, *m_v2 = nullptr, *m_text = nullptr, *m_formula = nullptr;
    QSpinBox *m_count = nullptr;
    QCheckBox *m_stop = nullptr;
    // color scale
    QCheckBox *m_threeColors = nullptr;
    QComboBox *m_scaleType[3] = {};
    QLineEdit *m_scaleValue[3] = {};
    CondColorButton *m_scaleColor[3] = {};
    // data bar
    QComboBox *m_barMin = nullptr, *m_barMax = nullptr, *m_barFill = nullptr, *m_barAxis = nullptr, *m_barDirection = nullptr;
    QLineEdit *m_barMinValue = nullptr, *m_barMaxValue = nullptr;
    CondColorButton *m_barColor = nullptr, *m_barNegative = nullptr, *m_barBorder = nullptr;
    QCheckBox *m_barBorderOn = nullptr, *m_barOnly = nullptr, *m_barNegativeSame = nullptr;
    // icon set
    QComboBox *m_iconSet = nullptr;
    QCheckBox *m_iconReverse = nullptr, *m_iconOnly = nullptr;
    QWidget *m_iconRows = nullptr;
    QVector<QComboBox *> m_iconType;
    QVector<QLineEdit *> m_iconValue;
    QVector<QComboBox *> m_iconGte;
    jp::office::sheet::CondRule m_iconStart;
};

// Manage Rules: the rules of a sheet in the order they work, each with the cells it is for and Stop If True; New Rule, Edit Rule, Delete Rule, and the two buttons that move a rule.
class CondManageDialog : public QDialog {
    Q_OBJECT
public:
    struct Item {
        jp::office::sheet::CondRule rule;
        jp::office::sheet::DifferentialFormat look;
        bool changed = false, newLook = false;
    };
    // Makes a rule from the person's answers (null when they give it up): `old` is the rule being changed, if any.
    using Editor = std::function<bool(const jp::office::sheet::CondRule *old, const jp::office::sheet::DifferentialFormat &oldLook, Item *out)>;
    CondManageDialog(const QVector<Item> &items, const Editor &editor, QWidget *parent = nullptr);
    QVector<crunch::EditSession::CondManaged> result() const;
    // For a test and for the buttons: what the list holds now, and the actions on the selected row.
    int count() const { return int(m_items.size()); }
    const Item &item(int row) const { return m_items[row]; }
    void select(int row);
    void editSelected();
    void addRule();
    void deleteSelected();
    void moveSelected(int by);
    // The cells of a rule as typed ("A1:B5 D2"); false when they are not cells.
    bool setRangesOf(int row, const QString &text);
    void setStopIfTrue(int row, bool stop);

private:
    void refill();
    int selectedRow() const;
    QVector<Item> m_items;
    Editor m_editor;
    QTableWidget *m_table = nullptr;
    bool m_filling = false;
};

} // namespace jp
