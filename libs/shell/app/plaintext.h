#pragma once
// What is shown from a file is shown as the text it is: a note, a link's address or tip, a sheet's or a name's text, and a file's name are never read as markup (a label or a message box
// reads markup unless told not to, and markup can name a picture to fetch). These are the ways to show such text.

#include <QLabel>
#include <QMessageBox>
#include <QString>
#include <QTextBrowser>
#include <QUrl>
#include <QTextDocument>

namespace jp {

// A tooltip (and anything else that is given a text that may be taken for markup) that shows `text` as it is, line breaks and all. A text that Qt would not take for
// markup is returned as it is, so that code reading the tooltip back (for a name to speak, say) still gets the text.
inline QString plainToolTip(const QString &text)
{
    if (!Qt::mightBeRichText(text)) return text;
    return QStringLiteral("<p style='white-space:pre-wrap'>") + text.toHtmlEscaped() + QStringLiteral("</p>");
}

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

// The same for the static message boxes (QMessageBox::information and its kin read markup unless told not to).
inline QMessageBox::StandardButton plainInformation(QWidget *parent, const QString &title, const QString &text, QMessageBox::StandardButtons buttons = QMessageBox::Ok,
                                                    QMessageBox::StandardButton preferred = QMessageBox::NoButton)
{
    return QMessageBox::StandardButton(plainBox(parent, QMessageBox::Information, title, text, buttons, preferred));
}
inline QMessageBox::StandardButton plainWarning(QWidget *parent, const QString &title, const QString &text, QMessageBox::StandardButtons buttons = QMessageBox::Ok,
                                                QMessageBox::StandardButton preferred = QMessageBox::NoButton)
{
    return QMessageBox::StandardButton(plainBox(parent, QMessageBox::Warning, title, text, buttons, preferred));
}
inline QMessageBox::StandardButton plainQuestion(QWidget *parent, const QString &title, const QString &text, QMessageBox::StandardButtons buttons = QMessageBox::Yes | QMessageBox::No,
                                                 QMessageBox::StandardButton preferred = QMessageBox::NoButton)
{
    return QMessageBox::StandardButton(plainBox(parent, QMessageBox::Question, title, text, buttons, preferred));
}

// A text browser that never loads a picture or any other file the text it shows names (Markdown from the internet can name a picture on another computer).
class NoResourceBrowser : public QTextBrowser {
public:
    using QTextBrowser::QTextBrowser;
    QVariant loadResource(int, const QUrl &) override { return QVariant(); }
};

} // namespace jp
