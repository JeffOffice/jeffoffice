#pragma once
// The Normal template: a per-user template file in the app's data folder that every new blank document starts from. "Set as Default" (Font, Paragraph, and Page Setup) with "all new
// documents" changes it; it is made on first use, from a blank document.

#include <QString>
#include <functional>
#include <memory>

namespace jp::write {

class WriteDoc;

// Where the file is.
QString normalTemplatePath();
// A new blank document: the Normal template when there is one (and it can be read), else a plain blank document.
std::unique_ptr<WriteDoc> newBlankDocument();
// Changes the Normal template with `change` (a blank document stands in when there is no file yet) and saves it. False, with `error` told, when the file can't be written.
bool changeNormalTemplate(const std::function<void(WriteDoc &)> &change, QString *error = nullptr);

} // namespace jp::write
