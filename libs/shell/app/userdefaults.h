#pragma once
// "Set as Default", for every app: the question (this document only, or all new documents), and where an app keeps what the answer "all new documents" sets: a file in its data
// folder (JeffWrite's Normal template), or a small set of choices (JeffDeck's theme and slide size).

#include <QJsonObject>
#include <QString>
#include <functional>

class QWidget;

namespace jp {

enum class DefaultScope { Cancel, ThisDocument, AllNew };

// Asks "this document only, or all new documents?" about `what` (words for the choice, such as "the font"). Cancel when the box is closed.
DefaultScope askDefaultScope(QWidget *parent, const QString &what);
// Stands in for the box (a test sets it; the program never does). Set to an empty function, the box is shown again.
void setDefaultScopeStandIn(const std::function<DefaultScope(const QString &what)> &standIn);

// The folder of the files made by Set as Default (made when asked for), and the path of one of them.
QString userDefaultsFolder();
QString userDefaultFile(const QString &name);

// A small set of choices kept under a name, as the app's settings (empty when none was kept).
QJsonObject userDefault(const QString &name);
void setUserDefault(const QString &name, const QJsonObject &choices);
void clearUserDefault(const QString &name);

} // namespace jp
