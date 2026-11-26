#pragma once
// Legacy form fields (FORMTEXT, FORMCHECKBOX, FORMDROPDOWN): reading what a field says of itself (its w:ffData, kept as written in the field's begin mark), and filling one in:
// typing in a text field, ticking a check box, choosing an entry of a drop-down. Nothing here makes a new field. A fill changes only the field's result (or the state its begin
// mark keeps), in one step of the history, and is never a tracked change (as an update of a field is not); a field nobody fills is written as it was read.

#include "core/writedoc.h"

#include <QString>
#include <QStringList>
#include <QVector>

class QTextCursor;
class QTextDocument;

namespace jp::write {

enum class FormKind { Text, CheckBox, DropDown };

constexpr int kMostFormFields = 20000;       // the fields looked at in one story
constexpr int kMostFormEntries = 1000;       // the entries of a drop-down that are read
constexpr int kMostFormChars = 32767;        // the text of one field (and the default text and format that are read from the file)
constexpr int kMostFormDataBytes = 262144;   // the part of a field's data that is read

struct FormField {
    FormKind kind = FormKind::Text;
    int begin = -1, separate = -1, end = -1;   // the places of its marks in the story
    QString name;                              // w:name, the name of the field (and of the bookmark round it, in files made by the usual program)
    QString bookmark;                          // the bookmark that holds the field (empty for none)
    QString help, status;                      // the help text, and the text for the status bar
    bool enabled = true;                       // false: the field is protected against being filled in (w:enabled off)
    bool calcOnExit = false;                   // w:calcOnExit: the file asks for the calculated fields to be worked out when the field is left
    // A text field: its kind of text (regular, number, date, currentDate, currentTime, calculated), default text, longest text (0: none), and format (a case, or a picture).
    QString textType = QStringLiteral("regular"), defaultText, format;
    int maxLength = 0;
    QString result;                            // what the field holds now
    // A check box: whether it is ticked, and its size in half points (0: as large as the text).
    bool checked = false;
    int size = 0;
    // A drop-down: its entries (at most kMostFormEntries), and the one chosen (-1: it has none).
    QStringList entries;
    int chosen = -1;
    bool valid() const { return begin >= 0; }
    // Whether a person can fill it in: it is enabled, and a text field is one whose text is typed (not one the program works out, or a date that is the day's).
    bool fillable() const;
};

// What the data of a form field's begin mark (the XML it keeps) says; `kind` of the result is only right when `ok` is set (the data names one kind of field).
FormField formFieldFromData(const QByteArray &beginRaw, bool *ok = nullptr);

// The form fields of a story in the order they begin (at most kMostFormFields), and the one that holds a place (the caret) or begins at it; none valid where there is none.
QVector<FormField> formFields(QTextDocument *story);
FormField formFieldAt(QTextDocument *story, int position);

// The field whose begin mark is at the place, none valid where there is none.
FormField formFieldBeginningAt(QTextDocument *story, int begin);

// The text a text field takes for what is typed in it: cut to its longest text, in its case (upper, lower, first capital, title), and, for a number, with only the characters a number has.
QString enteredFormText(const FormField &field, const QString &typed);
// The text a text field shows once it is left: a number or date put in the picture the field has. `ok` is false where the text is not a number (or date) and is left as typed.
QString finishedFormText(const FormField &field, const QString &typed, bool *ok = nullptr);

// Fills a text field in: its result becomes `text` as entered (enteredFormText), and, with `leaving`, as finished (finishedFormText); `*shown` gets what it came to. One step. False
// (nothing changed) where the field can't be filled, or the text is what it holds already. The references to its bookmark are updated.
bool fillFormText(WriteDoc &doc, QTextDocument *story, int begin, const QString &text, bool leaving = false, QString *shown = nullptr);
// Ticks or clears a check box. One step. False where it can't, or it is so already.
bool setFormChecked(WriteDoc &doc, QTextDocument *story, int begin, bool checked);
// Chooses the entry of a drop-down. One step. False where it can't, or the entry is chosen already.
bool chooseFormEntry(WriteDoc &doc, QTextDocument *story, int begin, int index);

// The field after (or before, with `backward`) a place that can be filled in, going round past the ends; none valid where there is no field.
FormField nextFormField(QTextDocument *story, int from, bool backward);
// What the caret takes when it goes to a field: a text field's result selected, else the place of its begin mark.
void selectFormField(QTextCursor &c, const FormField &field);

// Whether a place may be changed in a document protected for filling in forms: inside a form field that can be filled, or in a section the protection leaves out. Only the text of the
// document has sections; the other stories have their form fields alone.
bool formsEditableAt(WriteDoc &doc, QTextDocument *story, int position);
// Whether the section of the text that holds the place is one the protection leaves out (w:formProt off).
bool formsSectionOpenAt(WriteDoc &doc, int position);
// For each section of the text, whether the protection leaves it out.
QVector<bool> formsOpenSections(WriteDoc &doc);
// THE answer to "may text here be edited in a document protected for forms?": the place (or range) is in the text, and every section it lies in is one the protection leaves out. A range that
// reaches from a section that is left out into one that is not is refused. `sections` is formsOpenSections(doc) when the caller asks again and again.
bool formsRangeOpen(WriteDoc &doc, QTextDocument *story, int from, int to, const QVector<bool> *sections = nullptr);

} // namespace jp::write
