#pragma once
// The box that shows while the background of a big picture is being found, with Cancel (Editor::cancelRemoveBackgroundWork). Each window of a program that
// has Remove Background makes one for its editor.

#include "core/export.h"

#include <QObject>
#include <QPointer>
#include <QProgressDialog>

class QWidget;

namespace jp {

class Editor;

// A picture of more pixels than this has the box (a smaller one is found before the box would be seen).
constexpr qint64 kRemoveBgProgressPixels = 2'000'000;

class JO_COMMON_EXPORT RemoveBackgroundProgress : public QObject {
    Q_OBJECT
public:
    RemoveBackgroundProgress(Editor *editor, QWidget *window);
    // For tests: the box while it shows, else null.
    QProgressDialog *box() const { return m_box; }

private:
    void progress(int percent);
    void close();

    Editor *m_editor;
    QWidget *m_window;
    QPointer<QProgressDialog> m_box;
};

} // namespace jp
