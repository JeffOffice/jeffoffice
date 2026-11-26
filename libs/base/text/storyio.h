#pragma once
// Lossless JSON serialization of a QTextDocument, including JeffPub's custom
// properties (Qt's HTML export drops those).

#include <QJsonObject>
#include <QTextFormat>

class QTextDocument;

namespace jp {

QJsonObject formatToJson(const QTextFormat &f);
void formatFromJson(QTextFormat &f, const QJsonObject &o);

QJsonObject storyToJson(const QTextDocument *doc);
void storyFromJson(QTextDocument *doc, const QJsonObject &o);

// Plain text with paragraphs separated by '\n'.
void setStoryText(QTextDocument *doc, const QString &text);
// The paragraphs' text joined by '\n' with a line break kept as its own character (QTextDocument::toPlainText turns it into a '\n'), so a line break and a
// paragraph mark can be told apart: the text replaceStoryText compares with.
QString storyTextKeepingBreaks(const QTextDocument *doc);
// The same words as setStoryText leaves, changing only what differs (the text both share at the start and at the end is not touched), so the formatting, lists, and
// links of the unchanged text stay.
void replaceStoryText(QTextDocument *doc, const QString &text);

} // namespace jp
