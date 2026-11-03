#pragma once
// Open and Repair: what can be read of a damaged package (a .docx, .xlsx, or .pptx that is cut short, whose directory is gone, or some of whose parts are broken), made into a
// package the readers can open. The entries are found by their own headers, so a missing or wrong directory does not matter; a part that is XML and breaks part way is kept up to the
// break and closed properly; a part that can't be read is left out; and the list of types and the links that name the main part are made again when they are gone. Nothing in the
// damaged file is believed more than it can be checked: the work is bounded as reading a package is, and a part that holds a DOCTYPE is left out.

#include <QByteArray>
#include <QString>
#include <QStringList>

namespace jp::office {

struct RepairReport {
    QStringList kept;      // the parts that were read whole
    QStringList trimmed;   // XML parts that broke part way: kept up to the break
    QStringList dropped;   // parts that could not be read at all (each with why)
    bool typesMade = false, linksMade = false;   // the list of types, or the links to the main part, were gone and are made again
    bool damaged() const { return !trimmed.isEmpty() || !dropped.isEmpty() || typesMade || linksMade; }
};

// The repaired package; empty, with a message in `error`, when no document can be found in `zip`.
QByteArray repairPackage(const QByteArray &zip, QString *error = nullptr, RepairReport *report = nullptr);

// The same for a ZIP that is not a package of parts (a .jpub): the entries that are whole, as they were, in a ZIP that reads; the others are left out and named in `report->dropped`. Empty,
// with a message in `error`, when none is whole.
QByteArray repairZip(const QByteArray &zip, QString *error = nullptr, RepairReport *report = nullptr);

// XML that breaks part way (cut short, or with a bad tag) kept up to the last whole piece, with every open element closed; the same bytes when it is well-formed; empty when nothing of it
// can be kept (no root element, or a DOCTYPE). `trimmed` says the text was cut.
QByteArray repairXml(const QByteArray &xml, bool *trimmed = nullptr);

// What was done, in plain words, a line for each thing, for a message.
QString repairSummary(const RepairReport &report);

} // namespace jp::office
