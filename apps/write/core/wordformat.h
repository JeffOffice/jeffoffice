#pragma once
// Between the properties a word processing file states (paragraphs, runs) and the formats of the text engine, both ways. Properties are
// the "resolved" ones (the styles and the document's defaults under what the paragraph or run says) going in, and the whole set that a
// format holds coming out; the writer takes what differs from the style's to be what the paragraph or run says itself.

#include "office/theme.h"
#include "wml/wml.h"

#include <QColor>
#include <QTextBlockFormat>
#include <QTextCharFormat>

namespace jp::write {

// `theme` gives the colors that a run names by theme slot when the file has no value for them.
QTextCharFormat charFormatOf(const office::wml::RunProps &resolved, const office::Theme *theme = nullptr);
office::wml::RunProps runPropsOf(const QTextCharFormat &format);

QTextBlockFormat blockFormatOf(const office::wml::ParaProps &resolved);
office::wml::ParaProps paraPropsOf(const QTextBlockFormat &format);

// The fifteen highlight colors, by the names the file uses.
QColor highlightColor(const QString &name);
QString highlightName(const QColor &color);
QStringList highlightNames();

// Text the format marks as kept XML (a picture, a field's code, a bookmark...).
bool isAtom(const QTextCharFormat &format);
// The format with what makes it an atom taken away, for text typed beside one.
QTextCharFormat withoutAtom(QTextCharFormat format);

} // namespace jp::write
