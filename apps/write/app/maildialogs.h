#pragma once
// The dialog boxes of the Mailings tab: the address block and greeting line, Match Fields, the rules, Envelopes, Labels, and the choices for sending messages.

#include "core/labelops.h"
#include "core/mergesource.h"

#include <QHash>
#include <QString>
#include <QStringList>

class QWidget;

namespace jp::write {

// Address Block: the format of the lines, the country that is left out of the address, and what the first record's address block would be (a preview). `data` may be empty (no preview).
bool addressBlockDialog(QWidget *parent, const MergeSource &data, const QHash<QString, QString> &map, QString *format, QString *homeCountry);
// Greeting Line: the greeting, and what it says where the record has no name, with the first record's greeting as the preview.
bool greetingLineDialog(QWidget *parent, const MergeSource &data, const QHash<QString, QString> &map, QString *format, QString *fallback);
// Match Fields: for each part of an address, the column of the list that holds it.
bool matchFieldsDialog(QWidget *parent, const MergeSource &data, QHash<QString, QString> *map);
// A rule (IF, ASK, FILLIN, SET, SKIPIF, NEXTIF) with the words it needs, made into the code of a field. NEXT, MERGEREC, and MERGESEQ need none.
bool ruleDialog(QWidget *parent, const QString &rule, const QStringList &columns, QString *code);
// Where the messages are to go: the column that holds the addresses, the subject, and a folder for the files.
bool messageDialog(QWidget *parent, const QStringList &columns, QString *column, QString *subject, QString *folder);

// What the Envelopes and Labels dialogs ask for.
struct EnvelopeChoice {
    QString delivery, returnAddress;
    bool useBlock = false;   // the delivery address is the address block of the merge
    int size = 0;            // the place in envelopeSizes()
    bool print = false;      // else added to the document
};
bool envelopeDialog(QWidget *parent, bool merging, EnvelopeChoice *choice);
struct LabelChoice {
    QString address;
    bool useBlock = false;
    int sheet = 0;           // the place in labelSheets(), -1 for the one described by `custom`
    mail::LabelSheet custom;
    bool single = false;     // one label of the sheet (its row and column, from 1), else a full page of the same label
    int row = 1, column = 1;
    bool print = false;      // else made into a document
};
bool labelsDialog(QWidget *parent, bool merging, LabelChoice *choice);

} // namespace jp::write
