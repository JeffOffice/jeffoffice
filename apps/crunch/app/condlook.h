#pragma once
// The look of a conditional-format rule that a person makes themselves: the Format Cells box limited to what a rule can carry, the look it starts from and gives back, and the small sample of
// how cells look (the lists of rules and the rule windows show it).

#include "app/formatdialog.h"
#include "sheet/styles.h"

#include <QWidget>
#include <functional>

class QComboBox;

namespace jp {

// What the windows that ask for a look need: the workbook's styles (to resolve its colors), what the Format Cells box starts with (locale, codes, theme colors), and a stand-in for the box (a test).
struct CondLookContext {
    const jp::office::sheet::Styles *styles = nullptr;
    FormatCellsDialog::Input base;
    std::function<bool(const jp::office::sheet::DifferentialFormat &start, jp::office::sheet::DifferentialFormat *out)> asker;
};

// A color as RGB (a theme or palette color through the styles when there are some).
quint32 condLookRgb(const jp::office::sheet::ColorSpec &color, const jp::office::sheet::Styles *styles, quint32 automatic);
// The look as the Format Cells box starts from it, and the look after the box's answer (what the answer leaves alone stays as the look had it, bytes and all).
FormatView viewOfCondLook(const jp::office::sheet::DifferentialFormat &look, const jp::office::sheet::Styles *styles);
jp::office::sheet::DifferentialFormat condLookAfter(const jp::office::sheet::DifferentialFormat &look, const FormatCellsDialog::Result &answer);
// Asks for the look: the context's stand-in, or the box. False when it is cancelled.
bool askCondLook(QWidget *parent, const CondLookContext &context, const jp::office::sheet::DifferentialFormat &start, jp::office::sheet::DifferentialFormat *out);

// A cell drawn with a look: its fill, lines, and text (the number format shows a number; no number format, a line of letters).
class CondLookSample : public QWidget {
    Q_OBJECT
public:
    explicit CondLookSample(QWidget *parent = nullptr);
    void setLook(const jp::office::sheet::DifferentialFormat &look, const jp::office::sheet::Styles *styles);
    jp::office::sheet::DifferentialFormat look() const { return m_look; }
    // The text the sample shows.
    QString text() const { return m_text; }
    QSize sizeHint() const override { return QSize(120, 28); }

protected:
    void paintEvent(QPaintEvent *) override;

private:
    jp::office::sheet::DifferentialFormat m_look;
    const jp::office::sheet::Styles *m_styles = nullptr;
    QString m_text;
};

// Whether two looks are the same in every part.
bool sameCondLook(const jp::office::sheet::DifferentialFormat &a, const jp::office::sheet::DifferentialFormat &b);

// The list "Cells look like this" with its sample: the ready-made looks, "Custom Format..." (which asks for the person's own), and for a rule that exists the look it has now.
class CondLookBox : public QWidget {
    Q_OBJECT
public:
    struct Choice {
        bool keep = false;     // the rule's own look stays as it is
        int preset = 0;        // a ready-made look (a place in condpresets)
        bool custom = false;   // the person's own look: `look`
        jp::office::sheet::DifferentialFormat look;
    };
    // `editing`: the rule is one that exists, with the look `current` (empty for none).
    CondLookBox(bool editing, const jp::office::sheet::DifferentialFormat &current, QWidget *parent = nullptr);
    void setContext(const CondLookContext &context);
    // The look shown at the start: the rule's own (`keep`), a ready-made one, or the person's own.
    void setChoice(const Choice &choice);
    Choice choice() const;
    QComboBox *list() const { return m_list; }
    CondLookSample *sample() const { return m_sample; }

private:
    void changed(bool again = false);
    jp::office::sheet::DifferentialFormat shown() const;

    bool m_editing = false;
    jp::office::sheet::DifferentialFormat m_current, m_custom;
    CondLookContext m_context;
    QComboBox *m_list = nullptr;
    CondLookSample *m_sample = nullptr;
    int m_before = 0;
    bool m_building = true;
};

} // namespace jp
