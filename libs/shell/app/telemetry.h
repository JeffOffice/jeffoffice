#pragma once
// Anonymous usage statistics. Once a day (and right away after an update),
// while JeffPub or JeffDeck runs and unless the person using them turned
// them off (in setup, when the first of them starts, or in File > Options),
// each sends which app it is, a random install id made up here (the same
// in both), its version, the operating system and language, how many times
// it was started, and how many times each command was used; and after a
// crash, its report (crash.h). Never files, their names or their text. The
// choice and the install id belong to the suite; the counts, the launches,
// and the day of the last ping to each app. The collector (server/telemetry)
// keeps no IP address.

#include <QJsonObject>
#include <QString>

class QWidget;

namespace jp::telemetry {

bool decided();                         // the person has chosen (or setup chose for them)
bool enabled();
void setEnabled(bool on);
void count(const QString &feature);     // a command's id, e.g. "file.open.pub" or "file.save.pptx"; nothing when off
QJsonObject payload();                  // what the next ping would send
QJsonObject crashPayload(const QJsonObject &report);   // what sending a crash report sends; {} for none
bool due();                             // on, and no ping yet today or from this version
void start(QWidget *window);            // asks once, then pings daily; only main() calls this

} // namespace jp::telemetry
