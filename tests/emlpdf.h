// The PDF attached to a message file (.eml) as the program writes one: the base64 body of the part of type application/pdf.
#pragma once

#include <QByteArray>

inline QByteArray pdfAttachmentOf(const QByteArray &eml)
{
    const qsizetype type = eml.indexOf("application/pdf");
    if (type < 0) return {};
    const qsizetype body = eml.indexOf("\r\n\r\n", type);
    if (body < 0) return {};
    qsizetype end = eml.indexOf("\r\n--", body + 4);
    if (end < 0) end = eml.size();
    return QByteArray::fromBase64(eml.mid(body + 4, end - body - 4), QByteArray::Base64Encoding | QByteArray::IgnoreBase64DecodingErrors);
}
