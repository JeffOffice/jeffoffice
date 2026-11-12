#pragma once
// Mail merge: what a document is merged as and where its list of recipients comes from (the mail merge of the settings part, kept with the document), the fields of a merge (MERGEFIELD,
// ADDRESSBLOCK, GREETINGLINE, and the rules IF, SKIPIF, NEXT, NEXTIF, SET, ASK, FILLIN, MERGEREC, MERGESEQ), and the merge itself: one new document with a copy of the text for each recipient.
// The list is read by the shared reader of libs/page/core/mergesource.h.

#include "core/fieldcode.h"
#include "core/mergesource.h"
#include "core/writedoc.h"
#include "wml/wml.h"

#include <QHash>
#include <QJsonObject>
#include <QString>
#include <QTextCursor>
#include <QStringList>
#include <QVector>
#include <functional>
#include <memory>
#include <optional>

namespace jp::write::mail {

// The most records one merge makes (a larger list is merged in its first records), and the most characters of a prompt or an answer.
constexpr int kMostRecords = 100000;
// The most pages one merge makes in all (a page of a record is counted from its text, at least one for each record), and the most different questions (ASK and FILLIN) it asks. Layout and memory grow with the
// pages, and a list times a long letter would run for hours; past the questions' limit the answers are the defaults.
constexpr int kMostPages = 10000;
constexpr int kMostPrompts = 20;
// The most barcodes one merge makes (the number a document honors as barcodes when it is opened again, office/barcodemarker.h; labels have a few dozen to a page, so this is over 10 times a long run of labels).
constexpr int kMostBarcodes = 2000;

// A list longer than kMostRecords keeps its first records.
void limitRecords(MergeSource &data);
// A record whose every column is empty is not a recipient (JeffPub keeps them, as a list of its own makes them; the letters and labels here do not).
void dropBlankRecords(MergeSource &data);

enum class Kind { None, Letters, Email, Envelopes, Labels, Directory };
// The value of w:mainDocumentType for a kind ("formLetters", "email", "envelopes", "mailingLabels", "catalog"), and the kind a value says (None for one that is not known).
QString kindValue(Kind kind);
Kind kindOfValue(const QString &value);

// The merge of a document: the settings as the file had them (or as chosen), the list, and which column holds each field the address block and greeting line ask for.
struct State {
    office::wml::MailMergeSettings settings;
    QString sourcePath;                  // the file the list is read from (empty: a typed list that is not saved yet)
    MergeSource data;                    // the list, once it is read or made
    bool loaded = false;                 // `data` is the list
    bool changed = false;                // what the file has differs: it is written when the document is saved
    QHash<QString, QString> fieldMap;    // a field of the address block (see logicalFields()) -> the column that holds it (as Match Fields set it)
    QHash<QString, QString> fieldMapRead;   // what the file said of it (written again only where it differs)
    Kind kind() const { return kindOfValue(settings.type); }
};

// The merge of a document, made if it has none.
State &stateOf(WriteDoc &doc);
// The merge of a document, null if it has none.
State *stateIfAny(const WriteDoc &doc);

// Reads the list the document's merge names, from the file; where it is not (it may be on another computer). Returns whether the list is read.
bool loadSource(State &state, const QString &fromPath);

// ---- the fields of the address block and greeting line
// The parts of a name and an address the formats name: Title, First, Last, Suffix, Company, Street1, Street2, City, State, Postal, Country (and Name, the whole name where the list has it in one column).
QStringList logicalFields();
// The column of the list for a field of those (the one Match Fields chose, else the one with a usual name), empty for none.
QString columnFor(const MergeSource &data, const QHash<QString, QString> &map, const QString &logical);
// The formats of an address block the dialog offers, and the format a document's field has by default. A format is text with {Field} in it, and line breaks as \n.
QStringList addressBlockFormats();
// The address block of a record by a format: a line whose fields are all empty is left out, and so are the commas and spaces round them. `country`: where it is not empty, a record whose country is
// this one has no country line (the address is in the country the letter is posted in).
QString addressBlockText(const MergeSource &data, int row, const QHash<QString, QString> &map, const QString &format, const QString &homeCountry = QString());
// The greeting line of a record by a format ("Dear {Title} {Last},"), or `fallback` ("Dear Sir or Madam,") where the record has none of the name's parts the format asks for.
QString greetingText(const MergeSource &data, int row, const QHash<QString, QString> &map, const QString &format, const QString &fallback);
QStringList greetingFormats();

// ---- fields
// The code of a field, as it is put in a document, and the result it shows before the merge.
QString mergeFieldCode(const QString &column);
// Puts a field at the caret whose code may hold fields: "{ code }" in `code` is a field of its own (as in IF { MERGEFIELD State } = "VA" "yes" "no"). `result` is what it shows until the merge.
bool insertRule(WriteDoc &doc, QTextCursor &c, const QString &code, const QString &result = QString());
// A barcode whose value is a merge field, as the field DISPLAYBARCODE "{ MERGEFIELD column }" TYPE switches that the file saves, with the types and switches the field syntax defines (type: QR, CODE128,
// CODE39, EAN13, EAN8, UPCA; \s the size in percent, \h the height drawn in twips, \q the QR level, \t print the digits; a book's ISBN is EAN13 with its 13
// digits; \r, \b, and \f are read past). `settings` (the barcode dialog's) gives the type and look. The merge makes a barcode picture of each record's value, or a message where the code can't hold
// the value; a field with a value written out in it, no type, or a type not drawn here is left as it is.
bool insertBarcodeField(WriteDoc &doc, QTextCursor &c, const QString &column, const QJsonObject &settings);
QString addressBlockCode(const QString &format, const QString &homeCountry = QString());
QString greetingCode(const QString &format, const QString &fallback);

// The settings of the barcode a DISPLAYBARCODE field asks for with this value, as the barcode dialog keeps them; empty for a type that is none or not drawn here.
QJsonObject barcodeSettingsOfField(const fields::Code &code, const QString &value);
// The text that stands for the nth barcode of a record until the picture is put there.
QString barcodeMark(const QString &nonce, int n);

// Answers the fields of a merge, for one record at a time. The rules that move to another record (NEXT, NEXTIF) move `position`; SKIPIF marks the record skipped.
class Evaluator {
public:
    Evaluator(const MergeSource &data, const QHash<QString, QString> &fieldMap, const QVector<int> &rows) : m_data(data), m_map(fieldMap), m_rows(rows) {}
    std::optional<QString> answer(const fields::Code &code);
    // Where in `rows` the record is that fields show (a rule moves it).
    int position = 0;
    int sequence = 0;                 // MERGESEQ: how many records the merge has made, counting the one being made
    bool skipped = false;             // SKIPIF was true for the record
    int mostPrompts = kMostPrompts;   // the questions asked at most; the rest are answered by their defaults
    bool *cancelled = nullptr;        // the person cancelled: nothing more is asked
    // The answer to ASK and FILLIN (a prompt and a default): the person's, or the default where there is none.
    std::function<QString(const QString &prompt, const QString &defaultAnswer)> ask;
    QHash<QString, QString> variables;
    // The barcodes the record being merged asks for (DISPLAYBARCODE): the field's result is a mark (barcodeMark) for each, which the merge swaps for the picture once the text is in the new document;
    // with `barcodePictures` false (the text of an email) the result is the value. `barcodesMade` counts all the records', and `tooManyBarcodes` is set past `mostBarcodes`.
    QVector<QJsonObject> barcodes;
    bool barcodePictures = true;
    QString barcodeNonce;             // in the marks of this merge (made at its start), so that no value can hold one
    int barcodesMade = 0;
    int mostBarcodes = kMostBarcodes;
    bool tooManyBarcodes = false;
    bool atEnd() const { return position >= m_rows.size(); }
    int row() const { return position >= 0 && position < m_rows.size() ? m_rows[position] : -1; }
    // Whether a field of this name is the merge's to work out (the merge freezes these).
    bool isMergeField(const fields::Code &code) const;

private:
    QString value(const QString &column) const;
    bool test(const fields::Code &code) const;
    const MergeSource &m_data;
    const QHash<QString, QString> &m_map;
    const QVector<int> &m_rows;
    QHash<QString, QString> m_answers;
};

struct MergeOptions {
    QVector<int> rows;                // the records merged, as rows of the list (empty: those the list includes)
    std::function<QString(const QString &prompt, const QString &defaultAnswer)> ask;
    std::function<bool(int done, int total)> progress;   // false stops the merge
    bool pageBreaks = true;           // each record begins a page (letters); not for a directory
    int mostBarcodes = kMostBarcodes; // the merge stops (with an error) where it would make more barcodes than this
    int mostPages = kMostPages;       // the merge stops (with an error) where it would make more pages than this
    bool *cancelled = nullptr;        // set by `ask` (or the progress) when the person cancels: the merge stops
};

// A new document with a copy of the text of `source` for each record: its fields are the answers, its page setup, styles, headers, and footers are the source's. Null, with the reason, where it
// can't be made. The source is not changed.
std::unique_ptr<WriteDoc> mergeToDocument(WriteDoc &source, const MergeSource &data, const QHash<QString, QString> &fieldMap, const MergeOptions &options, QString *error = nullptr);
// The plain text of each merged record, for email messages (one string per text). `sourceRows` gets, for each text, the row of the list its first record is (a skipped record has no text, and a rule may
// take several records into one).
QStringList mergeToTexts(WriteDoc &source, const MergeSource &data, const QHash<QString, QString> &fieldMap, const MergeOptions &options, QString *error = nullptr, QVector<int> *sourceRows = nullptr);
// The text of `page` (an envelope, say) put first in `doc` as a section of its own, with the page setup of `page`, as one step of the history. False where it can't be done.
bool addAsFirstSection(WriteDoc &doc, WriteDoc &page);
// The records, in order, that fields show in an error-free merge: a record whose field names a column the list does not have is reported (the columns that are missing).
QStringList missingColumns(WriteDoc &doc, const MergeSource &data);

// An email message for each record as a .eml file in `folder` (named by the record's number and its address): the merged text as the message, `subject` as its subject, from the column `addressColumn`.
// Returns how many were written; the reason in `error` if it stops. A record whose address is not one person's (several, a name with it, a line break, no domain) has no message, and is counted in `refused`.
int writeEmailFiles(const QStringList &texts, const MergeSource &data, const QVector<int> &rows, const QString &addressColumn, const QString &subject, const QString &folder, QString *error = nullptr, int *refused = nullptr);

} // namespace jp::write::mail
