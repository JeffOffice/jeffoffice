#pragma once
// The dialog of the Playback tab's Trim: where in a movie or sound it starts and stops.

#include <QDialog>

class QDoubleSpinBox;
class QLabel;

namespace jp {

// Start Time and End Time in seconds. `length` is how long the whole file
// is, in milliseconds (0 when it isn't known, and then the end can be any
// time). The end is shown as the length when the file plays to its end.
class MediaTrimDialog : public QDialog {
    Q_OBJECT
public:
    MediaTrimDialog(const QString &title, qint64 length, int startMs, int endMs, QWidget *parent = nullptr);
    int startMs() const;
    // 0 when the end is the end of the file.
    int endMs() const;

private:
    void check();
    QDoubleSpinBox *m_start, *m_end;
    QLabel *m_note;
    QPushButton *m_ok = nullptr;
    qint64 m_length;
};

} // namespace jp
