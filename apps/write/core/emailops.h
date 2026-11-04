#pragma once
// A message for an email program, made as a file (.eml) so that nothing is sent through an account: the person opens it, adds who it is for, and sends it.

#include <QByteArray>
#include <QString>
#include <QVector>

namespace jp::write {

struct EmailPart {
    QString fileName;
    QString mimeType;
    QByteArray data;
    QString contentId;   // for a picture shown in the message's page: its name there ("cid:" and this); empty for an attachment
};

// The message: a subject (one line, encoded when it has more than ASCII), the words, the attachments, and, with `html`, a page of the message's own that shows `pictures` (by their content ids).
QByteArray emailMessage(const QString &subject, const QString &text, const QVector<EmailPart> &attachments, const QString &html = QString(), const QVector<EmailPart> &pictures = {});

} // namespace jp::write
