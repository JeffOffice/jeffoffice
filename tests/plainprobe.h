#pragma once
// What the hostile-text tests share: would Qt take this text for markup that names a picture (which it then reads, a UNC path or a file: address on this computer)?
// A tooltip, a label in Auto format, and a message box decide this with Qt::mightBeRichText, then show the text as a QTextDocument reads HTML.

#include <QLabel>
#include <QMessageBox>
#include <QString>
#include <QTextBlock>
#include <QTextCharFormat>
#include <QTextDocument>

namespace plainprobe {

// The text a person would have for a name or a comment that tries this.
inline QString hostile() { return QStringLiteral("<img src=\"file:///etc/hostname\"><img src=\"\\\\probe.invalid\\share\\x.png\"><b>bold</b>"); }

// True when `text`, shown with `format`, makes Qt read a picture.
inline bool readsAPicture(const QString &text, Qt::TextFormat format = Qt::AutoText)
{
    if (format == Qt::PlainText) return false;
    if (format == Qt::AutoText && !Qt::mightBeRichText(text)) return false;
    QTextDocument doc;
    doc.setHtml(text);
    for (QTextBlock b = doc.begin(); b.isValid(); b = b.next())
        for (auto it = b.begin(); !it.atEnd(); ++it)
            if (it.fragment().charFormat().isImageFormat()) return true;
    return false;
}

inline bool readsAPicture(const QLabel *label) { return readsAPicture(label->text(), label->textFormat()); }
inline bool readsAPicture(const QMessageBox *box) { return readsAPicture(box->text(), box->textFormat()) || readsAPicture(box->informativeText(), box->textFormat()); }

// The probe itself must be able to see: the hostile text, shown as an Auto label would show it, does read a picture.
inline bool probeSeesTheTrap() { return readsAPicture(hostile()); }

} // namespace plainprobe
