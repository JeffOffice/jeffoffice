#pragma once
// The property editors of the Format pane: a number with a slider, the stops of a gradient, a fill, and a line. They show the values of
// all the objects selected (what the objects don't agree on is blank) and say what the person changed as a function that changes a Fill or
// a Stroke, so each object keeps what the person didn't touch. They know nothing of pages or documents: an app's pane connects them to
// its objects and to its undo.
//
// Every editor says a change in two signals: a change (mid-drag too), and finished(), which comes when the person lets go of the control
// (at once, for a control with nothing to drag). The pane makes one step that undoes from the first change to finished().

#include "core/color.h"
#include "core/style.h"

#include <QDoubleSpinBox>
#include <QFrame>
#include <QSlider>
#include <QWidget>
#include <functional>


class QButtonGroup;
class QCheckBox;
class QComboBox;
class QFormLayout;
class QLabel;
class QRadioButton;
class QStackedWidget;
class QToolButton;

namespace jp {

class ColorButton;

// Lets the wheel scroll the pane instead of changing a box or a slider the pointer happens to be over: the wheel counts only on the
// control that has the keyboard.
void ignoreWheelUnlessFocused(QWidget *w);

// A number in a box, with a slider if asked, for a size, a percent, or an angle.
class NumberField : public QWidget {
    Q_OBJECT
public:
    explicit NumberField(QWidget *parent = nullptr);
    void setRange(double lo, double hi);
    void setDecimals(int n);
    void setStep(double step);
    void setSuffix(const QString &suffix);
    // The value is in points and shows in the unit of Options (inches, centimeters...); a typed unit is read.
    void setMeasure(bool on);
    // A slider of the same range beside the box.
    void setSlider(bool on);
    void setAccessibleLabel(const QString &name);
    double value() const;
    // Sets the box and the slider without a signal. Ignored while the person is working in the field.
    void setValue(double v);
    // The objects don't agree: the box is blank. A change by the person ends it.
    void setMixed(bool on);
    bool isMixed() const { return m_mixed; }
    bool isBusy() const;
    QDoubleSpinBox *box() const;
    QSlider *slider() const { return m_slider; }

Q_SIGNALS:
    void edited(double value);
    void finished();

protected:
    bool eventFilter(QObject *o, QEvent *e) override;

private:
    class Spin;
    void spinChanged(double v);
    void blankIfMixed();
    Spin *m_spin;
    QSlider *m_slider = nullptr;
    int m_sliderScale = 1;
    bool m_mixed = false;
    bool m_loading = false;
    bool m_pressed = false;   // the pointer is down on the arrows
    bool m_keyStep = false;   // an arrow key is held
    bool m_typing = false;
    bool m_changedSinceTyping = false;
    bool m_unfinished = false;   // edits were said, and finished() has not been
};

// A row of colored markers on a gradient: the stops. Click one to choose it, drag it (or press Left and Right) to move it, Up and Down to
// choose another, Delete to take it away, and double-click on the bar to add one.
class GradientBar : public QWidget {
    Q_OBJECT
public:
    explicit GradientBar(QWidget *parent = nullptr);
    void setScheme(const ColorScheme &s);
    // Shows the stops (not signalled); the current stop stays if it still exists.
    void setStops(const QVector<GradientStop> &stops);
    QVector<GradientStop> stops() const { return m_stops; }
    int current() const { return m_current; }
    void setCurrent(int i);
    QSize sizeHint() const override { return QSize(220, 44); }
    QSize minimumSizeHint() const override { return QSize(120, 44); }
    // Where a stop's marker is on the bar, for a pointer at the position (0 to 1) of a bar of this width.
    QRectF markerRect(int i) const;
    QRectF barRect() const;
    // A new stop at `pos`, in the color the bar has there.
    void addStopAt(double pos);
    void removeCurrent();

Q_SIGNALS:
    void stopsEdited(const QVector<GradientStop> &stops);
    void currentChanged(int index);
    void finished();

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;

private:
    double posAt(double x) const;
    int markerAt(const QPointF &p) const;
    void move(int i, double pos);
    QVector<GradientStop> m_stops;
    ColorScheme m_scheme;
    int m_current = 0;
    bool m_dragging = false;
    bool m_moved = false;
    bool m_keyHeld = false;
};

using FillChange = std::function<void(Fill &)>;

// The colors of a gradient: its kind and angle, the bar of stops, and the color, place, and transparency of the stop chosen. Used by the
// fill editor and by the line editor (a gradient line).
class GradientEditor : public QWidget {
    Q_OBJECT
public:
    explicit GradientEditor(QWidget *parent = nullptr);
    void setScheme(const ColorScheme &s);
    // The gradients of the objects (each a Fill of type Gradient).
    void setGradients(const QVector<Fill> &fills);
    GradientBar *bar() const { return m_bar; }

Q_SIGNALS:
    void changed(const FillChange &change);
    void finished();

private:
    void showStop();
    void showAngle(bool linear);
    void stopsEdited(const QVector<GradientStop> &stops);
    void editStop(const std::function<void(GradientStop &)> &change, bool last);
    QComboBox *m_kind;
    NumberField *m_angle, *m_position, *m_transparency;
    GradientBar *m_bar;
    ColorButton *m_color;
    QToolButton *m_add, *m_remove;
    QVector<GradientStop> m_shown;
    ColorScheme m_scheme;
    bool m_loading = false;
};

// A fill: none, solid, gradient, picture or texture, or pattern; and what each has.
class FillEditor : public QWidget {
    Q_OBJECT
public:
    explicit FillEditor(QWidget *parent = nullptr);
    void setScheme(const ColorScheme &s);
    // How a picture fill's picture is shown (a thumbnail).
    void setImageProvider(const ImageLookup &lookup) { m_images = lookup; }
    // The fills of the objects now. None: nothing is selected, and the editor is turned off.
    void setFills(const QVector<Fill> &fills);
    QVector<Fill> fills() const { return m_fills; }

Q_SIGNALS:
    void changed(const FillChange &change);
    void finished();
    // The person chose a picture (or texture) fill without a picture to show for it, or pressed Insert: the app asks for the file and
    // then changes the fills (see Fill::imageId).
    void pictureRequested();

private:
    void showFills();
    void typeChosen(Fill::Type type);
    QRadioButton *m_radios[5] = {};
    QButtonGroup *m_group;
    QStackedWidget *m_pages;
    ColorButton *m_color, *m_fore, *m_back;
    NumberField *m_transparency, *m_tileScale, *m_pictureTransparency;
    GradientEditor *m_gradient;
    QCheckBox *m_tile;
    QLabel *m_thumb;
    QVector<QToolButton *> m_patterns;
    QVector<Fill> m_fills;
    ColorScheme m_scheme;
    ImageLookup m_images;
    bool m_loading = false;
};

using StrokeChange = std::function<void(Stroke &)>;

// A line: none, solid, or gradient; its color, transparency, width, compound type, dash type, cap, join, and the ends' arrows.
class LineEditor : public QWidget {
    Q_OBJECT
public:
    explicit LineEditor(QWidget *parent = nullptr);
    void setScheme(const ColorScheme &s);
    void setStrokes(const QVector<Stroke> &strokes);
    // A line with no ends (a shape's outline) has no arrows to choose.
    void setArrowsShown(bool shown);
    QVector<Stroke> strokes() const { return m_strokes; }

Q_SIGNALS:
    void changed(const StrokeChange &change);
    void finished();

private:
    void showStrokes();
    void typeChosen(int type);
    QRadioButton *m_radios[3] = {};
    QButtonGroup *m_group;
    QFormLayout *m_form;
    QWidget *m_details, *m_arrows;
    GradientEditor *m_gradient;
    ColorButton *m_color;
    NumberField *m_transparency, *m_width;
    QComboBox *m_compound, *m_dash, *m_cap, *m_join, *m_beginType, *m_beginSize, *m_endType, *m_endSize;
    QVector<Stroke> m_strokes;
    ColorScheme m_scheme;
    bool m_loading = false;
};

} // namespace jp
