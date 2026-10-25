#pragma once
// What is shown from a file is shown as the text it is: a note, a link's address or tip, a sheet's or a name's text, and a file's name are never read as markup (a label or a message box
// reads markup unless told not to, and markup can name a picture to fetch). These are the ways to show such text.

#include <QLabel>
#include <QMessageBox>
#include <QString>

namespace jp {

// A tooltip (and anything else that is given a text that may be taken for markup) that shows `text` as it is, line breaks and all.
inline QString plainToolTip(const QString &text) { return QStringLiteral("<p style='white-space:pre-wrap'>") + text.toHtmlEscaped() + QStringLiteral("</p>"); }

// A label that shows its text as it is.
inline QLabel *plainLabel(QLabel *label)
{
    label->setTextFormat(Qt::PlainText);
    return label;
}

// A message box that shows its text (and its extra text) as it is; the button the person chose.
inline int plainBox(QWidget *parent, QMessageBox::Icon icon, const QString &title, const QString &text, QMessageBox::StandardButtons buttons = QMessageBox::Ok,
                    QMessageBox::StandardButton preferred = QMessageBox::NoButton)
{
    QMessageBox box(icon, title, text, buttons, parent);
    box.setTextFormat(Qt::PlainText);
    if (preferred != QMessageBox::NoButton) box.setDefaultButton(preferred);
    return box.exec();
}

} // namespace jp
