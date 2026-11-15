#pragma once
// Replace Fonts, for the text of a QTextDocument: the fonts its runs name outright, and swapping one for another. A run that follows the theme's font names none (tp::ThemeFont), so it
// is never counted or changed; a font a run names is the family of its format, or the families when it names more than one (a list of fallbacks).

#include <QMap>
#include <QString>
#include <QStringList>
#include <QTextCharFormat>

class QTextDocument;

namespace jp {

// The families a character format names (empty when it names none).
QStringList fontFamiliesOf(const QTextCharFormat &format);
// Adds to `uses` each family a run of `doc` names, and the paragraph marks of its blocks (what an empty paragraph is set in), with how many runs and marks name it. The first family of a
// run is the one that counts. With `major` and `minor`, the runs and marks that follow the theme's heading font and its body font are counted there.
void collectFontFamilies(const QTextDocument *doc, QMap<QString, int> *uses, int *major = nullptr, int *minor = nullptr);
// Puts `to` in the place of `from` (families are the same without regard to case) in every run and paragraph mark that names it; how many runs and marks changed. The change is one edit
// block of the document, so that one undo of the document's own history takes all of it back.
int replaceFontFamily(QTextDocument *doc, const QString &from, const QString &to);
// The same for one character format: whether it named `from`, and now names `to`.
bool replaceFontFamilyIn(QTextCharFormat *format, const QString &from, const QString &to);

} // namespace jp
