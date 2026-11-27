#pragma once
// The text of a .docx for Insert File: its document part as simple HTML, read without a zip library (a compact inflater that refuses more than a part may hold).

#include <QByteArray>
#include <QString>

namespace jp {

// The most paragraphs a .docx, .doc, or .odt puts into the text (600,000 of them take the window about ten seconds to lay out); the rest is left out, and said.
constexpr int kMaxInsertParagraphs = 100000;
// The text of the document part of a .docx package as simple HTML; `left`, when given, is how many pictures, tables, lists, and notes it has that the text alone leaves out.
// ...and `cut`, when given, is set when it has more than kMaxInsertParagraphs paragraphs and the rest was not read.
QString docxToHtml(const QByteArray &docx, int *left = nullptr, bool *cut = nullptr);

} // namespace jp
