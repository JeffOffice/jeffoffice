#pragma once
// Open and Repair: what can be read of a damaged package (a .docx, .xlsx, or .pptx that is cut short, whose directory is gone, or some of whose parts are broken), made into a
// package the readers can open. The entries are found by their own headers, so a missing or wrong directory does not matter; a part that is XML and breaks part way is kept up to the
// break and closed properly; a part that can't be read is left out; and the list of types and the links that name the main part are made again when they are gone. Nothing in the
// damaged file is believed more than it can be checked: the work is bounded as reading a package is, and a part that holds a DOCTYPE is left out.

#include "office/xml.h"

#include <QByteArray>
#include <QString>
#include <QStringList>

namespace jp::office {

// What a repair holds and looks at, bounded as opening a package is (a package is refused at 2 GB inflated, an XML part at kMaxXmlBytes except a sheet, which is read as a stream). The repair
// holds one part at a time (the file is read twice: once to choose and check, once to write), so what it keeps is the package it writes, which is at most kRepairMaxKeptBytes inflated, half of
// what an open allows, so the package it makes always opens. Real files: the largest of 47 decks is 60 MB in all, and a workbook of a million rows is 100 MB; the entries are at most 10,000
// (real packages have under 500).
constexpr qint64 kRepairMaxKeptBytes = qint64(1) << 30;
constexpr qint64 kRepairMaxPartBytes = qint64(512) << 20;   // one part, inflated (a sheet part; any other XML part is held to kMaxXmlBytes, as an open does)
constexpr qint64 kRepairMaxWorkBytes = qint64(2) << 30;     // inflated from all the entries, in one reading of the file
constexpr int kRepairMaxEntries = 10000;
// The same, as a value, so a test can use small ones.
struct RepairLimits {
    qint64 maxKeptBytes = kRepairMaxKeptBytes;
    qint64 maxPartBytes = kRepairMaxPartBytes;
    qint64 maxXmlPartBytes = kMaxXmlBytes;   // an XML part that is not read as a stream
    qint64 maxWorkBytes = kRepairMaxWorkBytes;
    int maxEntries = kRepairMaxEntries;
};

struct RepairReport {
    QStringList kept;      // the parts that were read whole
    QStringList trimmed;   // XML parts that broke part way: kept up to the break
    QStringList dropped;   // parts that could not be read at all (each with why)
    QStringList notes;     // what was left out for want of room, in words (more entries than are looked at, more bytes than are held)
    bool typesMade = false, linksMade = false;   // the list of types, or the links to the main part, were gone and are made again
    bool damaged() const { return !trimmed.isEmpty() || !dropped.isEmpty() || !notes.isEmpty() || typesMade || linksMade; }
};

// The repaired package; empty, with a message in `error`, when no document can be found in `zip`.
QByteArray repairPackage(const QByteArray &zip, QString *error = nullptr, RepairReport *report = nullptr, const RepairLimits &limits = RepairLimits());

// The same for a ZIP that is not a package of parts (a .jpub): the entries that are whole, as they were, in a ZIP that reads; the others are left out and named in `report->dropped`. Empty,
// with a message in `error`, when none is whole.
QByteArray repairZip(const QByteArray &zip, QString *error = nullptr, RepairReport *report = nullptr, const RepairLimits &limits = RepairLimits());

// XML that breaks part way (cut short, or with a bad tag) kept up to the last whole piece, with every open element closed; the same bytes when it is well-formed; empty when nothing of it
// can be kept (no root element, or a DOCTYPE). `trimmed` says the text was cut. Nesting past kMaxXmlDepth (xml.h) is where it is cut too, and `stoppedBecause` says so in words (else it is left
// empty).
QByteArray repairXml(const QByteArray &xml, bool *trimmed = nullptr, QString *stoppedBecause = nullptr);

// What was done, in plain words, a line for each thing, for a message.
QString repairSummary(const RepairReport &report);

} // namespace jp::office
