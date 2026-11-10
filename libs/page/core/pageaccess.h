#pragma once
// What on the pages of a publication or a presentation is hard for some readers (or a screen reader) to use: a picture or a shape with no description, and, where the app asks,
// a page with no title. A list, each with the page and the object, so that the window can go there.

#include <QString>
#include <QVector>
#include <functional>

namespace jp {

class Document;

struct PageAccessIssue {
    enum Kind { MissingAltText, NoPageTitle };
    enum Level { Error, Warning, Tip };
    Kind kind = MissingAltText;
    Level level = Warning;
    int page = 0;
    QString itemId;   // empty for a whole page
    QString message;
};

constexpr int kMaxPageAccessIssues = 500;

// The problems found, page by page (at most `most`). A picture with no alt text is an error, and so is a shape with no words in it (an object marked as decoration is left alone).
// `titleOf` (may be empty) gives the title of a page; with it, a page with an empty title is a warning. `slides` says "slide" where the messages say "page".
QVector<PageAccessIssue> checkPageAccessibility(const Document &doc, bool slides = false, const std::function<QString(int)> &titleOf = {}, int most = kMaxPageAccessIssues);

} // namespace jp
