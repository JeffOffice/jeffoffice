#pragma once
// The dialog boxes of the Home and Review tabs that are not Font or Paragraph: Sort, Word Count.

#include "core/homeops.h"
#include "core/proofops.h"

#include <QStringList>
#include <functional>

class QWidget;

namespace jp::write {

// Sort: up to three keys (the field, its type, ascending or descending), what separates the fields, and whether the first paragraph is a header. False if canceled; `options` is then as it was.
bool sortDialog(QWidget *parent, SortOptions *options);
// Word Count: the numbers of `stats` (for the selection or the whole document), with the pages, and a box for the notes that asks for the count again through `recount`.
void wordCountDialog(QWidget *parent, int pages, bool selection, const std::function<WordStats(bool notes)> &recount);

// Paste Special: the ways the clipboard can be pasted now (each a line), and which the person picks; -1 if canceled.
int pasteSpecialDialog(QWidget *parent, const QStringList &choices);

// Language: the languages that can be checked, with the one the text is in chosen, and "Do not check spelling". False if canceled; else `tag` and `noProof` are the choice.
bool languageDialog(QWidget *parent, const QString &current, bool noProof, QString *tag, bool *noProofOut);

} // namespace jp::write
