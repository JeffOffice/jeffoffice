#pragma once
// The dialog boxes of the Home and Review tabs that are not Font or Paragraph: Sort, Word Count.

#include "core/compareops.h"
#include "core/homeops.h"
#include "app/translate.h"
#include "core/proofops.h"
#include "text/textprops.h"

#include <QStringList>
#include <QWidget>
#include <functional>

class QWidget;
class QLabel;
class QListWidget;

namespace jp::write {

class PageView;

// Sort: (in a table the keys are columns, and there are no separators) up to three keys (the field, its type, ascending or descending), what separates the fields, and whether the first paragraph is a header. False if canceled; `options` is then as it was.
bool sortDialog(QWidget *parent, SortOptions *options, bool inTable = false);
// Word Count: the numbers of `stats` (for the selection or the whole document), with the pages, and a box for the notes that asks for the count again through `recount`.
void wordCountDialog(QWidget *parent, int pages, bool selection, const std::function<WordStats(bool notes)> &recount);

// Paste Special: the ways the clipboard can be pasted now (each a line), and which the person picks; -1 if canceled.
int pasteSpecialDialog(QWidget *parent, const QStringList &choices);

using jp::languageDialog;   // (libs/shell/app/translate.h)

// Thesaurus: synonyms of `word` (from the dictionary on this computer), a box to look up another word, and Replace. Returns the word chosen to replace `word`, or empty.
QString thesaurusDialog(QWidget *parent, const QString &word);
using jp::translateDialog;

// Restrict Editing. With none enforced: the kind of editing to allow (readOnly, comments, trackedChanges, or forms) and a password (twice), and Start Enforcing; `lockTracking` offers only tracked changes. With
// one enforced: its kind, and Stop Protection (the password asked for in the box when the protection has one). Returns 0 for canceled, 1 for start (`edit` and `password` set), 2 for stop (`password` set).
int restrictEditingDialog(QWidget *parent, bool enforced, const QString &currentEdit, bool hasPassword, bool lockTracking, QString *edit, QString *password);
// Track Changes Options: the name and initials that changes and comments are signed with, and how changes are drawn (`markup`: the looks and colors of insertions, deletions, moved text,
// and changes of formatting, the change bars, and whether moves and formatting are tracked as such).
bool trackOptionsDialog(QWidget *parent, QString *name, QString *initials, tp::RevisionMarkup *markup);

// Compare: the file of the revised version (the open document is the original), who the changes are signed with, and the choices. False if canceled.
bool compareDialog(QWidget *parent, QString *revisedPath, CompareOptions *options);

// The Accessibility pane: the problems of a document in a list (the place of one goes with a click), and a button to check again.
class AccessPane : public QWidget {
    Q_OBJECT
public:
    AccessPane(PageView *view, QWidget *parent = nullptr);
    void check();

private:
    PageView *m_view;
    QListWidget *m_list;
    QLabel *m_summary;
};

// Reveal Formatting: what is set where the caret is (the pairs of describeFormatting), in a box that only shows.
void formattingDialog(QWidget *parent, const QVector<QPair<QString, QString>> &facts);

} // namespace jp::write
