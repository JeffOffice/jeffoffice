// Insert > Content Control: the menu that puts a rich text box, a plain text box, a picture, a combo box, a drop-down list, a date picker, a check box, or a repeating section in the text, and the Properties box of the one the caret is in; and what a
// click on a list or a date shows (core/contentcontrolops.h).

#include "app/controldialogs.h"
#include "app/pageview.h"
#include "app/writewindow.h"
#include "core/contentcontrolops.h"
#include "core/formfieldops.h"

#include <QAction>
#include <QFileDialog>
#include <QFileInfo>
#include <QMenu>
#include <QStatusBar>

namespace jp {

void WriteWindow::createControlActions()
{
    auto insert = [this](write::ControlKind kind) {
        if (!m_doc || refuseIfReadOnly()) return;
        QString why;
        if (!write::insertContentControl(*m_doc, m_view->cursor(), kind, &why) && !why.isEmpty()) statusBar()->showMessage(why, 6000);
        m_view->setFocus(Qt::OtherFocusReason);
        m_view->viewport()->update();
    };
    mk("insert.control.richText", tr("Rich Text"), "text", QKeySequence(), [insert] { insert(write::ControlKind::RichText); });
    mk("insert.control.text", tr("Plain Text"), "text-cursor-input", QKeySequence(), [insert] { insert(write::ControlKind::PlainText); });
    mk("insert.control.picture", tr("Picture"), "image", QKeySequence(), [insert] { insert(write::ControlKind::Picture); });
    mk("insert.control.comboBox", tr("Combo Box"), "chevrons-up-down", QKeySequence(), [insert] { insert(write::ControlKind::ComboBox); });
    mk("insert.control.dropDown", tr("Drop-Down List"), "list", QKeySequence(), [insert] { insert(write::ControlKind::DropDown); });
    mk("insert.control.date", tr("Date Picker"), "calendar", QKeySequence(), [insert] { insert(write::ControlKind::Date); });
    mk("insert.control.checkBox", tr("Check Box"), "square-check", QKeySequence(), [insert] { insert(write::ControlKind::CheckBox); });
    mk("insert.control.repeatingSection", tr("Repeating Section"), "repeat", QKeySequence(), [insert] { insert(write::ControlKind::RepeatingSection); });
    mk("insert.control.addItem", tr("Add Item"), "square-plus", QKeySequence(), [this] {
        if (!m_doc || refuseIfReadOnly()) return;
        QString why;
        if (!write::addRepeatingItem(*m_doc, m_view->cursor().position(), &why)) statusBar()->showMessage(why.isEmpty() ? tr("The caret is not in a repeating section.") : why, 6000);
        m_view->setFocus(Qt::OtherFocusReason);
        m_view->viewport()->update();
    });
    mk("insert.control.removeItem", tr("Remove Item"), "square-minus", QKeySequence(), [this] {
        if (!m_doc || refuseIfReadOnly()) return;
        QString why;
        if (!write::removeRepeatingItem(*m_doc, m_view->cursor().position(), &why)) statusBar()->showMessage(why.isEmpty() ? tr("The caret is not in a repeating section.") : why, 6000);
        m_view->setFocus(Qt::OtherFocusReason);
        m_view->viewport()->update();
    });
    mk("insert.control.clearPicture", tr("Clear Picture"), "image-minus", QKeySequence(), [this] {
        if (!m_doc || refuseIfReadOnly()) return;
        const write::ContentControl c = write::contentControlAt(*m_doc, m_view->cursor().position());
        if (!c.valid() || c.props.kind != write::ControlKind::Picture || !write::clearControlPicture(*m_doc, c.start)) statusBar()->showMessage(tr("The caret is not in a picture control that holds a picture."), 6000);
        m_view->setFocus(Qt::OtherFocusReason);
        m_view->viewport()->update();
    });
    connect(m_view, &write::PageView::controlPictureWanted, this, [this](int start) {
        if (!m_doc || refuseIfReadOnly()) return;
        const QString path = QFileDialog::getOpenFileName(this, tr("Choose a Picture"), QString(), tr("Pictures (*.png *.jpg *.jpeg *.gif *.bmp *.tif *.tiff *.svg)"));
        if (path.isEmpty()) return;
        QByteArray bytes;
        const write::PictureResult read = write::readPictureFile(path, &bytes);
        if (read != write::PictureResult::Ok) {
            statusBar()->showMessage(read == write::PictureResult::TooLarge ? tr("That picture is too large to put in a document.") : tr("%1 can't be opened as a picture.").arg(QFileInfo(path).fileName()), 8000);
            return;
        }
        const write::PictureResult made = write::setControlPicture(*m_doc, start, bytes, QFileInfo(path).completeBaseName());
        if (made == write::PictureResult::TooLarge) statusBar()->showMessage(tr("That picture is too large to put in a document."), 8000);
        else if (made != write::PictureResult::Ok) statusBar()->showMessage(tr("%1 can't be opened as a picture.").arg(QFileInfo(path).fileName()), 8000);
        m_view->setFocus(Qt::OtherFocusReason);
        m_view->viewport()->update();
    });
    mk("insert.control.properties", tr("Properties…"), "settings-2", QKeySequence(), [this] {
        if (!m_doc || refuseIfReadOnly()) return;
        const write::ContentControl c = write::controlAtCaret(*m_doc, m_view->cursor());
        if (!c.valid()) return;
        write::contentControlDialog(this, m_doc.get(), c);
        m_view->setFocus(Qt::OtherFocusReason);
        m_view->viewport()->update();
    });
    connect(m_view, &write::PageView::formChoiceWanted, this, [this](int begin, const QPoint &global) {
        if (!m_doc) return;
        QTextDocument *story = m_view->cursor().document();
        const write::FormField field = write::formFieldBeginningAt(story, begin);
        if (!field.valid() || field.kind != write::FormKind::DropDown || !field.fillable()) return;
        auto *menu = new QMenu(this);
        menu->setAttribute(Qt::WA_DeleteOnClose);
        for (int i = 0; i < field.entries.size(); ++i) {
            QAction *a = menu->addAction(field.entries.at(i));
            a->setCheckable(true);
            a->setChecked(i == field.chosen);
            connect(a, &QAction::triggered, this, [this, story, begin, i] {
                if (m_doc) write::chooseFormEntry(*m_doc, story, begin, i);
                m_view->setFocus(Qt::OtherFocusReason);
                m_view->viewport()->update();
            });
        }
        menu->popup(global);
    });
    connect(m_view, &write::PageView::controlChoiceWanted, this, [this](int start, const QPoint &global) {
        if (!m_doc || refuseIfReadOnly()) return;
        QMenu *menu = write::controlChoicesMenu(this, m_doc.get(), start, [this] {
            m_view->setFocus(Qt::OtherFocusReason);
            m_view->viewport()->update();
        });
        if (menu) menu->popup(global);
    });
}

void WriteWindow::refreshControlUi()
{
    const bool body = m_doc && !m_view->inHeaderFooter() && !m_view->inNote();
    for (const char *id : {"insert.control.richText", "insert.control.text", "insert.control.picture", "insert.control.comboBox", "insert.control.dropDown", "insert.control.date", "insert.control.checkBox", "insert.control.repeatingSection"})
        act(QString::fromLatin1(id))->setEnabled(body);
    bool inControl = false, inSection = false, inPicture = false;
    if (body) {
        const QTextCursor &c = m_view->cursor();
        const write::ContentControl ctl = write::controlAtCaret(*m_doc, c);
        inControl = ctl.valid() && ctl.editable;
        inSection = write::repeatingItemAt(*m_doc, c.position()).valid();
        inPicture = ctl.valid() && ctl.props.kind == write::ControlKind::Picture && !ctl.placeholder;
    }
    act("insert.control.properties")->setEnabled(inControl);
    act("insert.control.addItem")->setEnabled(inSection);
    act("insert.control.removeItem")->setEnabled(inSection);
    act("insert.control.clearPicture")->setEnabled(inPicture);
}

} // namespace jp
