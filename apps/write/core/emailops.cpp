#include "core/emailops.h"

#include <QRandomGenerator>

namespace jp::write {

namespace {

// A header value as one line: a line break would start a header of its own, and a quote mark would end a quoted name.
QByteArray headerText(const QString &s)
{
    QString one = s;
    one.replace(QLatin1Char('\r'), QLatin1Char(' ')).replace(QLatin1Char('\n'), QLatin1Char(' ')).replace(QLatin1Char('"'), QLatin1Char('\''));
    for (const QChar c : one)
        if (c.unicode() >= 0x80 || c.unicode() < 0x20) return "=?UTF-8?B?" + one.toUtf8().toBase64() + "?=";
    return one.toLatin1();
}

QByteArray base64Lines(const QByteArray &data)
{
    const QByteArray b64 = data.toBase64();
    QByteArray out;
    for (int i = 0; i < b64.size(); i += 76) out += b64.mid(i, 76) + "\r\n";
    return out;
}

// A boundary no part holds: made again until it is in none of them.
QByteArray boundaryFor(const QVector<QByteArray> &parts)
{
    for (;;) {
        QByteArray b = "jo-";
        for (int i = 0; i < 20; ++i) b += "0123456789abcdefghijklmnopqrstuvwxyz"[QRandomGenerator::global()->bounded(36)];
        bool clear = true;
        for (const QByteArray &p : parts) clear = clear && !p.contains(b);
        if (clear) return b;
    }
}

QByteArray partOf(const EmailPart &p)
{
    const QByteArray name = headerText(p.fileName);
    QByteArray x = "Content-Type: " + p.mimeType.toLatin1() + "; name=\"" + name + "\"\r\n";
    x += p.contentId.isEmpty() ? "Content-Disposition: attachment; filename=\"" + name + "\"\r\n" : "Content-Disposition: inline; filename=\"" + name + "\"\r\nContent-ID: <" + p.contentId.toLatin1() + ">\r\n";
    return x + "Content-Transfer-Encoding: base64\r\n\r\n" + base64Lines(p.data);
}

QByteArray textPart(const char *type, const QString &text)
{
    return QByteArray("Content-Type: ") + type + "; charset=utf-8\r\nContent-Transfer-Encoding: base64\r\n\r\n" + base64Lines(text.toUtf8());
}

} // namespace

QByteArray emailMessage(const QString &subject, const QString &text, const QVector<EmailPart> &attachments, const QString &html, const QVector<EmailPart> &pictures)
{
    // The parts of the body: the words alone, or a page with its pictures (the words are kept in the page too).
    QByteArray body, bodyType;
    QVector<QByteArray> blocks;
    for (const EmailPart &p : attachments) blocks << p.data;
    for (const EmailPart &p : pictures) blocks << p.data;
    blocks << text.toUtf8() << html.toUtf8();
    const QByteArray mixed = boundaryFor(blocks);
    const QByteArray related = boundaryFor(blocks + QVector<QByteArray>{mixed});
    if (html.isEmpty()) {
        body = textPart("text/plain", text);
    } else {
        body = "Content-Type: multipart/related; boundary=\"" + related + "\"\r\n\r\n--" + related + "\r\n" + textPart("text/html", html);
        for (const EmailPart &p : pictures) body += "--" + related + "\r\n" + partOf(p);
        body += "--" + related + "--\r\n";
    }
    QByteArray x = "Subject: " + headerText(subject) + "\r\nX-Unsent: 1\r\nMIME-Version: 1.0\r\nContent-Type: multipart/mixed; boundary=\"" + mixed + "\"\r\n\r\n";
    x += "--" + mixed + "\r\n" + body;
    for (const EmailPart &p : attachments) x += "--" + mixed + "\r\n" + partOf(p);
    x += "--" + mixed + "--\r\n";
    return x;
}

} // namespace jp::write
