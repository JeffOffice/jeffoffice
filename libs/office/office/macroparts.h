#pragma once
// The macro project of a macro-enabled file (a presentation, a workbook, or a text document): its bytes are kept as they were and written back, and
// nothing in them is ever read, parsed, shown, or run. The parts a project has are the project itself and its signature; they are kept only when the
// file is written as a macro-enabled kind, and left out of any other.
//
// What the package needs to be right, for each kind: the main part has the macro-enabled content type (macroMainType), the project is a part with the
// content type "application/vnd.ms-office.vbaProject" linked from the main part with the relationship type ".../2006/relationships/vbaProject", and
// the signature, if there is one, is a part of the type "application/vnd.ms-office.vbaProjectSignature" linked from the project with the relationship
// type ".../vbaProjectSignature".

#include "office/package.h"

#include <QByteArray>
#include <QJsonObject>
#include <QString>
#include <QVector>

namespace jp::office {

// The bounds on what is kept from a file (a project is read whole, and what a file says is never trusted): at most this many parts, none larger than
// the part bound, and no more than the total bound together. A project over a bound is left out whole (the file is then the plain kind), and `leftOut`
// says so.
constexpr int kMaxMacroParts = 8;
constexpr qint64 kMaxMacroPartBytes = qint64(32) << 20;
constexpr qint64 kMaxMacroBytes = qint64(64) << 20;

// Whether a part of that content type, or a relationship of that type, is part of a macro project (or the macro-enabled form of a main part).
bool isMacroContentType(const QString &contentType);
bool isMacroRelationshipType(const QString &relationshipType);

enum class MacroProgram { Document, Presentation, Workbook };
enum class MacroKind { Plain, Template, Show };
// The content type of a main part of a macro-enabled file; the plain kind's when `macros` is false. A workbook has no show, a text document neither.
QString macroMainType(MacroProgram program, MacroKind kind, bool macros);
// Whether a main part's content type is a macro-enabled kind's.
bool isMacroMainType(const QString &contentType);

struct MacroPart {
    QString name;            // as the file named it (absolute)
    QString contentType;
    QByteArray data;
    int from = -1;           // the part that links to it: -1 the main part, else the place of another in the list (always an earlier one)
    bool signature = false;  // a project's signature, linked from its project
};

struct MacroProject {
    QVector<MacroPart> parts;
    bool leftOut = false;    // there was a project, and it was over a bound: nothing of it is kept
    bool isEmpty() const { return parts.isEmpty(); }
};

// The project (and its signature) the main part links to, as the package has them. Nothing in the bytes is read.
MacroProject readMacroProject(const Package &package, const QString &mainPart);

// Adds the project to a package being written: the parts under the names they had, or under `folder` ("/ppt/", "/xl/") when that name is taken or
// not usable, each with its content type, and the links from the main part and from each project. Call it after the parts are added.
void writeMacroProject(PackageWriter *writer, const QString &mainPart, const MacroProject &project, const QString &folder);

// The project as a JSON object (the bytes in base64) to keep in a document, and back. Back gives nothing for an object that is not a project or is
// over a bound.
QJsonObject macroProjectToJson(const MacroProject &project);
MacroProject macroProjectFromJson(const QJsonObject &json);

} // namespace jp::office
