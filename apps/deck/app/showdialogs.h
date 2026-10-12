#pragma once
// The dialogs of the Slide Show tab: Set Up Show (the kind of show, which
// slides, how it moves on, the pen's and the laser pointer's colors), Custom
// Shows (the list: New, Edit, Copy, Remove, and Show), and the editor of one
// custom show (its name, and its slides picked from the presentation in any
// order). F1 in each opens its Help.

#include "core/color.h"
#include "core/showsettings.h"

#include <QDialog>

class QButtonGroup;
class QCheckBox;
class QComboBox;
class QDialogButtonBox;
class QLineEdit;
class QListWidget;
class QPushButton;
class QRadioButton;
class QSpinBox;

namespace jp {

class Document;

// How the show goes. The pen's and the laser pointer's colors are chosen from lists by name (the pen's ends with More Colors...; a color the
// lists lack is shown by its code). The settings it is given are the ones it starts from;
// `settings()` is them with what was changed. A kiosk loops and moves on by
// the times, so the choices that would say otherwise are held until another
// kind of show is chosen.
class SetUpShowDialog : public QDialog {
    Q_OBJECT
public:
    SetUpShowDialog(const deck::ShowSettings &settings, const QVector<deck::CustomShow> &shows, int slideCount, const ColorScheme &scheme, QWidget *parent = nullptr);
    deck::ShowSettings settings() const;

protected:
    void keyPressEvent(QKeyEvent *e) override;

private:
    void refresh();
    void fillPen(const QColor &current);
    deck::ShowSettings m_base;
    QColor m_pen;
    int m_penAt = 0;
    bool m_loopChosen = false;     // what the check box said before a kiosk held it
    bool m_timingsChosen = true;
    QRadioButton *m_speaker, *m_individual, *m_kiosk;
    QRadioButton *m_all, *m_range, *m_custom;
    QRadioButton *m_manual, *m_timings;
    QSpinBox *m_from, *m_to;
    QComboBox *m_shows, *m_laser, *m_penBox;
    QCheckBox *m_loop, *m_noAnimation;
};

// The editor of one custom show: its name, and the slides in it. `all` are the
// shows the presentation has (a name may not repeat another's), `editing` the
// show being changed (null: a new one).
class CustomShowEditDialog : public QDialog {
    Q_OBJECT
public:
    CustomShowEditDialog(const Document &doc, const QVector<deck::CustomShow> &all, const deck::CustomShow *editing, QWidget *parent = nullptr);
    deck::CustomShow show() const;

protected:
    void keyPressEvent(QKeyEvent *e) override;

private:
    void refresh();
    void addSlides();
    QVector<deck::CustomShow> m_all;
    deck::CustomShow m_show;
    bool m_new = true;
    QLineEdit *m_name;
    QListWidget *m_presentation, *m_mine;
    QPushButton *m_ok = nullptr, *m_add, *m_remove, *m_up, *m_down;
};

// The custom shows. Closing keeps the changes (shows()); Show closes it too and says which one to play (playId(), -1 for none).
class CustomShowsDialog : public QDialog {
    Q_OBJECT
public:
    CustomShowsDialog(const Document &doc, const QVector<deck::CustomShow> &shows, QWidget *parent = nullptr);
    QVector<deck::CustomShow> shows() const { return m_shows; }
    qint64 playId() const { return m_playId; }

public Q_SLOTS:
    // Esc closes it as Close does: the changes are kept.
    void reject() override { accept(); }

protected:
    void keyPressEvent(QKeyEvent *e) override;

private:
    void refill(int select = -1);
    void refresh();
    int current() const;
    void edit(int index);
    const Document &m_doc;
    QVector<deck::CustomShow> m_shows;
    qint64 m_playId = -1;
    QListWidget *m_list;
    QPushButton *m_new, *m_edit, *m_copy, *m_remove, *m_play;
};

} // namespace jp
