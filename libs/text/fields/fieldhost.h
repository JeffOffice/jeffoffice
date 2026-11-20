#pragma once
// The field engine for an app with simple answers (a presentation, a publication, a sheet): the app says what only it knows (the page or slide, how many there are, the document's
// properties, the moment, a prompt's answer) by implementing FieldHost, and the engine (fields/fieldcode.h) works out the result. The word processor fills the engine's Env itself
// from its story; a host is the same bundle in a form that needs no story.

#include "fields/fieldcode.h"

#include <QDateTime>
#include <QLocale>
#include <QString>
#include <optional>

namespace jp::fields {

// The most characters a prompt's answer may have (ASK and FILLIN); a longer one is cut.
constexpr int kMostPromptChars = 1024;

class FieldHost {
public:
    virtual ~FieldHost() = default;
    // The page (or slide) the field is on, from 1, as a number and as it is shown ("iv"); 0 where there are none. The text may be empty: the number is shown.
    virtual int pageNumber() const { return 0; }
    virtual QString pageText() const { return QString(); }
    virtual int pageCount() const { return 0; }
    virtual int section() const { return 0; }
    virtual int sectionPageCount() const { return 0; }
    // The document's properties: author, title, file name and path, custom ones.
    virtual Properties properties() const { return Properties(); }
    // A bookmark's text and place; none where the host has no bookmarks.
    virtual std::optional<BookmarkInfo> bookmark(const QString &name) const { Q_UNUSED(name) return std::nullopt; }
    // The counters of SEQ fields by name (upper case); null where the host has none.
    virtual QHash<QString, qint64> *sequences() const { return nullptr; }
    virtual QDateTime now() const { return QDateTime::currentDateTime(); }
    virtual QLocale locale() const { return QLocale(); }
    // The answer to the prompt of an ASK or FILLIN field (`kind` is ASK or FILLIN); none where nobody answers (the field keeps its result).
    virtual std::optional<QString> prompt(const QString &kind, const QString &text, const QString &defaultAnswer) const { Q_UNUSED(kind) Q_UNUSED(text) Q_UNUSED(defaultAnswer) return std::nullopt; }
};

// The Env that answers from the host (it holds nothing of the host's but what it asks: keep the host alive while the Env is used).
Env makeEnv(const FieldHost &host);
// The result of the instruction ("PAGE \* roman"), or none where the engine leaves it as it is.
std::optional<QString> evaluate(const FieldHost &host, const QString &instruction);

// A date or time in a format of the Qt letters libs/text keeps its codes in ("M/d/yyyy h:mm AP"), worked out by the engine's date picture; none where the format has a
// letter the picture has no equal for (then the caller shows it as before). The one implementation of date pictures, for the field engine and for formatters like the merge's.
std::optional<QString> dateFromQtFormat(const QDateTime &when, const QString &format, const QLocale &locale);

} // namespace jp::fields
