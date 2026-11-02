#pragma once
// The field engine, the part that needs no document: a field's code is read into its name, arguments, and switches; its result is worked out from what the document tells (the
// Env) and shown in the formats the switches ask for (\* roman, \# "0.00", \@ "MMMM d, yyyy"). Nothing here touches the story; core/fieldops.h walks the story and asks.
// A field this engine does not know, or one it cannot answer (a bookmark it has not been told of), has no result here, and the field keeps the result it has.

#include <QDateTime>
#include <QHash>
#include <QString>
#include <QStringList>
#include <QVector>
#include <functional>
#include <optional>

namespace jp::write::fields {

// The most characters of a code that are read, the most words in it, and the deepest nesting the formulas and the conditions are followed (a hostile file can say more).
constexpr int kMostCodeChars = 4096;
constexpr int kMostCodeWords = 256;
constexpr int kMostFormulaDepth = 48;
constexpr int kMostPictureChars = 255;

struct Switch {
    QChar letter;        // as written, except that the letter of a switch is compared without regard to case
    QString argument;    // the word after it that it takes (a quoted one without its quotes), else empty
};

struct Code {
    QString name;                // upper case: PAGE, REF, TOC...
    QStringList arguments;       // the words before the first switch, a quoted one without its quotes (an empty quoted one is kept)
    QString expression;          // the text after the name up to the first switch, as written (a formula's)
    QVector<Switch> switches;
    QStringList general;         // the arguments of \* (MERGEFORMAT, roman, Upper...)
    QString numberPicture, datePicture;   // the arguments of \# and \@
    bool hasNumberPicture = false, hasDatePicture = false;
    bool has(QChar letter) const;
    QString argumentOf(QChar letter) const;   // the argument of the first such switch
    bool keepsFormat() const;                 // \* MERGEFORMAT or \* CHARFORMAT: the result takes the look of the result it replaces
};

// The code of an instruction (leading and trailing blanks do not matter). Never fails: what it cannot read is left out.
Code parseCode(const QString &instruction);

// ---- formats
// The number as the general-format switches show it (roman, ROMAN, alphabetic, ALPHABETIC, arabic, Ordinal, CardText, OrdText, Hex, ArabicDash, then Upper, Lower, FirstCap, Caps
// on the text); the plain digits where none asks for a number format.
QString formatNumber(qint64 n, const QStringList &general);
// Text in the case the switches ask for (Upper, Lower, FirstCap, Caps).
QString formatText(const QString &text, const QStringList &general);
// A number in a picture: 0 and # for digits, x to drop digits, . and , for the point and the groups, ; between the pictures for positive, negative, and zero, 'text' and other
// characters as they are, - and + for the sign.
QString numberInPicture(double value, const QString &picture);
// A date and time in a picture: M, d, y, h, H, m, s, am/pm, 'text', \ before a character.
QString dateInPicture(const QDateTime &when, const QString &picture);
// A number as a formula without a picture shows it (up to six places, no trailing zeros).
QString plainNumber(double value);

// ---- formulas and conditions
// The value of an expression of a formula field (= 2 + 3 * 4, SUM(1,2,3), (7 - 2) ^ 2, 50% ...). Names that are not functions are asked of `name` (a bookmark's number).
// Nothing for text that is no formula, or one deeper or longer than the most.
std::optional<double> evaluateFormula(const QString &expression, const std::function<std::optional<double>(const QString &)> &name = nullptr);

// The pieces of the document a field's result may be made of. Each function says nothing (null) where the document cannot answer, and the field keeps its result.
struct BookmarkInfo {
    QString text;          // what the bookmark holds
    QString page;          // the page it is on, as a page number shows it (empty where the pages are not laid out)
    QString number;        // the number of its paragraph without trailing periods (REF \n), the one with the numbers of the levels above (\w), and the one relative to the field (\r)
    QString fullNumber, relativeNumber;
    int position = 0;      // where it begins in the text: whether it is above or below the field
    int pageIndex = 0;     // the page, from 1, for "on page N" (0 where unknown)
};

struct Properties {
    QString author, title, subject, keywords, comments, lastSavedBy, template_, company, manager, category, fileName, filePath, revision;
    QDateTime created, saved, printed;
    qint64 fileSize = -1;
    qint64 totalMinutes = 0;
    int pages = 0, words = 0, characters = 0, charactersWithSpaces = 0, lines = 0, paragraphs = 0;
    bool statistics = false;   // whether the counts are known
};

struct Env {
    QDateTime now;
    Properties properties;
    // Where the field is: its page (from 1) as a number and as the page number shows it, the pages in all and in its section, its section's number, and the place it stands at.
    int page = 0, pages = 0, sectionPages = 0, section = 0, position = 0;
    QString pageText;
    std::function<std::optional<BookmarkInfo>(const QString &)> bookmark;
    std::function<std::optional<QString>(const QString &name, const Code &code)> noteNumber;    // NOTEREF: the number of the note the bookmark holds the reference of
    std::function<std::optional<QString>(const QString &style, const Code &code)> styleRef;     // STYLEREF
    // The counters of SEQ fields, by name (upper case): the pass that walks the document keeps them and resets those that count by chapter.
    QHash<QString, qint64> *sequences = nullptr;
    // Used with a bookmark name read by a formula, and by a condition of IF.
    std::function<std::optional<double>(const QString &)> number;
};

// The result of a field with this code, or null where the engine leaves it as it is (a field it does not know, a document that cannot say). TOC is built by the caller.
std::optional<QString> compute(const Code &code, Env &env);

// The names of the fields that depend on the machine or the moment, not on the document (a yardstick leaves them out).
bool dependsOnTheMachine(const QString &name);
// The fields this engine works out.
bool isKnown(const QString &name);
// The errors a reference shows as the other program does.
QString referenceSourceNotFound();
QString bookmarkNotDefined();

} // namespace jp::write::fields
