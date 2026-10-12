#pragma once
// Crash reports. When the program crashes, a handler notes where it stopped:
// for each step of the call stack, the file name of the program or library
// it was in and the position within that file, plus the version, the date,
// and the signal or exception. Never memory, documents, or anything typed.
// On the next start the report is sent with the usage statistics, only if
// those are on (telemetry.cpp), and deleted either way.

#include <QJsonObject>
#include <QString>

namespace jp::crash {

// Where a crash leaves its report: in the app's data folder.
QString reportPath();

// Catches crashes from now on and writes their report to `path`. Only
// main() calls this, early; a second call changes nothing.
void install(const QString &path);

// The report a crash left at `path`: {"version", "when", "kind", "code",
// "frames": [{"module", "offset"}]}, with every value cut to what the
// collector takes; {} when there's no report there or it isn't one.
QJsonObject readReport(const QString &path);

// The file name of the program or library holding `address`, and the
// address's offset within it, as a report gives them (for tests).
bool locate(const void *address, QString *module, quint64 *offset);

} // namespace jp::crash
