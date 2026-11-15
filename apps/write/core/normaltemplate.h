#pragma once
// The Normal template: a per-user template file in the app's data folder that every new blank document starts from. "Set as Default" (Font, Paragraph, and Page Setup) with "all new
// documents" changes it; it is made on first use, from a blank document.

#include <QString>
#include <functional>
#include <memory>

namespace jp::write {

class WriteDoc;

// The largest file that is read as the template: a larger one (or one that can't be read) is not used, and a new document is made with the program's own settings.
constexpr qint64 kMaxNormalTemplateBytes = qint64(50) << 20;

// Where the file is.
QString normalTemplatePath();

enum class NormalTemplateState { Missing, Usable, Unreadable };
// Whether there is a template, and whether it can be used (not too large, and a document that reads).
NormalTemplateState normalTemplateState();

// A new blank document: the Normal template when there is one (and it can be used), else a plain blank document, and `note` says why when the file was there and could not be
// used. What the template carries stays (styles, page setup, headers and footers, theme, body text), as with any template; the document's properties (author, title, dates, and so
// on) start again, its comments are dropped, and its tracked changes are accepted.
std::unique_ptr<WriteDoc> newBlankDocument(QString *note = nullptr);

// Changes the Normal template with `change` (a blank document stands in when there is no file yet) and saves it. The file is locked while it is changed, so that two windows (or two
// programs) do not lose each other's change. False, with `error` told, when the change is not saved: the file can't be written, is in use, or can't be read and `replaceUnreadable` is
// not set (the caller asks the person first: nothing is replaced unasked).
bool changeNormalTemplate(const std::function<void(WriteDoc &)> &change, QString *error = nullptr, bool replaceUnreadable = false);

} // namespace jp::write
