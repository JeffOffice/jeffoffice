#include "app/removebgprogress.h"

#include "app/editor.h"

#include <QProgressDialog>

namespace jp {

RemoveBackgroundProgress::RemoveBackgroundProgress(Editor *editor, QWidget *window) : QObject(window), m_editor(editor), m_window(window)
{
    connect(editor, &Editor::removeBackgroundProgress, this, &RemoveBackgroundProgress::progress);
    // (the mode ended while the box showed: Esc, or a click away)
    connect(editor, &Editor::viewChanged, this, [this] {
        if (m_box && m_editor->removeBgItem.isEmpty()) close();
    });
}

void RemoveBackgroundProgress::progress(int percent)
{
    if (percent >= 100 || m_editor->removeBgItem.isEmpty()) {
        close();
        return;
    }
    if (m_editor->removeBgPixels < kRemoveBgProgressPixels) return;
    if (!m_box) {
        auto *box = new QProgressDialog(tr("Finding the background…"), tr("Cancel"), 0, 100, m_window);
        box->setObjectName(QStringLiteral("removeBackgroundProgress"));
        box->setWindowModality(Qt::WindowModal);
        box->setMinimumDuration(400);
        box->setAutoClose(false);
        box->setAutoReset(false);
        connect(box, &QProgressDialog::canceled, this, [this] { m_editor->cancelRemoveBackgroundWork(); });
        m_box = box;
    }
    m_box->setValue(percent);
}

void RemoveBackgroundProgress::close()
{
    if (!m_box) return;
    m_box->hide();
    m_box->deleteLater();   // (it may be this box's own Cancel that ends the work)
}

} // namespace jp
