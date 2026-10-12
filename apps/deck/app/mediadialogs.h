#pragma once
// The dialog of the Playback tab's Trim: where in a movie or sound it starts and stops.

#include <QDialog>

class QDoubleSpinBox;
class QLabel;

namespace jp {

// Start Time and End Time in seconds: where in the file it plays from and to. The file holds how much is cut off each end, so the box takes
// and gives that (cutStartMs, cutEndMs). `length` is how long the whole file is, in milliseconds (0 when it isn't known: then the end can't
// be shown or set, and is given back as it came).
class MediaTrimDialog : public QDialog {
    Q_OBJECT
public:
    MediaTrimDialog(const QString &title, qint64 length, int cutStartMs, int cutEndMs, QWidget *parent = nullptr);
    int cutStartMs() const;
    int cutEndMs() const;

private:
    void check();
    QDoubleSpinBox *m_start, *m_end;
    QLabel *m_note;
    QPushButton *m_ok = nullptr;
    qint64 m_length;
    int m_cutEnd;
};

} // namespace jp
