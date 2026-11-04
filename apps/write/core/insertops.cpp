#include "core/insertops.h"

#include "core/editops.h"
#include "core/fieldops.h"
#include "core/refops.h"
#include "core/tableops.h"
#include "core/writedoc.h"

#include <QCoreApplication>
#include <QDate>
#include <QLocale>
#include <QTextCursor>
#include <memory>

namespace jp::write {

void insertSymbol(WriteDoc &doc, QTextCursor &c, const QString &symbol, const QString &font)
{
    if (symbol.isEmpty()) return;
    const WriteDoc::StepGroup group(doc);
    insertText(doc, c, symbol);
    if (font.isEmpty()) return;
    // (The symbol just typed is the text before the caret.)
    QTextCursor made(c);
    made.setPosition(c.position() - int(symbol.size()));
    made.setPosition(c.position(), QTextCursor::KeepAnchor);
    setFontFamily(doc, made, font);
}

void insertBlankPage(WriteDoc &doc, QTextCursor &c)
{
    const WriteDoc::StepGroup group(doc);
    insertPageBreak(doc, c);
    insertPageBreak(doc, c);
}

namespace {
const QString kCoverMark = QStringLiteral("_JeffWriteCoverPage");

BookmarkSpan coverSpan(WriteDoc &doc)
{
    const StoryScan scan = scanStory(doc.story());
    const auto found = scan.bookmarkByName.constFind(kCoverMark.toLower());
    return found == scan.bookmarkByName.constEnd() ? BookmarkSpan() : scan.bookmarks[found.value()];
}
}

bool hasCoverPage(WriteDoc &doc)
{
    const BookmarkSpan s = coverSpan(doc);
    return s.start >= 0 && s.end > s.start;
}

bool removeCoverPage(WriteDoc &doc)
{
    const BookmarkSpan s = coverSpan(doc);
    if (s.start < 0 || s.end <= s.start) return false;
    const WriteDoc::StepGroup group(doc);
    QTextCursor c(doc.story());
    c.setPosition(s.start);
    // (To the start of the paragraph after the one the cover ends in: its paragraph mark goes with it.)
    const QTextBlock last = doc.story()->findBlock(s.end);
    c.setPosition(std::min(last.position() + last.length(), editableEnd(doc.story())), QTextCursor::KeepAnchor);
    removeSelection(doc, c);
    return true;
}

bool insertCoverPage(WriteDoc &doc, CoverDesign design, QString *problem)
{
    QTextDocument *story = doc.story();
    if (inTable(story->begin())) {
        if (problem) *problem = QCoreApplication::translate("jp::write::CoverPage", "A cover page can't go before a table at the very start of the document. Put a paragraph before the table first.");
        return false;
    }
    const WriteDoc::StepGroup group(doc);
    removeCoverPage(doc);
    struct Look {
        double size, before, after;
        bool bold;
        QColor color;
        Qt::Alignment align;
    };
    const QColor accent(0x1D, 0x55, 0xC4), gray(0x59, 0x59, 0x59), ink(0x20, 0x20, 0x20);
    Look looks[4];
    switch (design) {
    case CoverDesign::Classic:
        looks[0] = {36, 180, 12, true, accent, Qt::AlignHCenter};
        looks[1] = {18, 0, 160, false, gray, Qt::AlignHCenter};
        looks[2] = {14, 0, 4, false, ink, Qt::AlignHCenter};
        looks[3] = {12, 0, 0, false, gray, Qt::AlignHCenter};
        break;
    case CoverDesign::Band:
        looks[0] = {44, 200, 6, true, accent, Qt::AlignLeft};
        looks[1] = {20, 0, 200, false, gray, Qt::AlignLeft};
        looks[2] = {13, 0, 2, true, ink, Qt::AlignLeft};
        looks[3] = {12, 0, 0, false, gray, Qt::AlignLeft};
        break;
    case CoverDesign::Plain:
        looks[0] = {28, 72, 6, true, ink, Qt::AlignLeft};
        looks[1] = {14, 0, 72, false, gray, Qt::AlignLeft};
        looks[2] = {12, 0, 2, false, ink, Qt::AlignLeft};
        looks[3] = {11, 0, 0, false, gray, Qt::AlignLeft};
        break;
    }
    const QString lines[4] = {QCoreApplication::translate("jp::write::CoverPage", "Document Title"), QCoreApplication::translate("jp::write::CoverPage", "Subtitle"),
                              QCoreApplication::translate("jp::write::CoverPage", "Author Name"), QLocale().toString(QDate::currentDate(), QStringLiteral("MMMM d, yyyy"))};
    QTextCursor c(story);
    c.setPosition(0);
    // (The page break is the last character of the date's paragraph: what follows goes to the next page, with no empty paragraph before it.)
    insertText(doc, c, lines[0] + QLatin1Char('\n') + lines[1] + QLatin1Char('\n') + lines[2] + QLatin1Char('\n') + lines[3] + QString(flow::kPageBreak) + QLatin1Char('\n'));
    for (int i = 0; i < 4; ++i) {
        QTextCursor sel(story);
        sel.setPosition(story->findBlockByNumber(i).position());
        sel.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        setFontSize(doc, sel, looks[i].size);
        if (looks[i].bold) toggleBold(doc, sel);
        setTextColor(doc, sel, looks[i].color);
        setAlignment(doc, sel, looks[i].align);
        setSpacing(doc, sel, looks[i].before, looks[i].after);
    }
    QTextCursor all(story);
    all.setPosition(0);
    all.setPosition(story->findBlockByNumber(3).position() + story->findBlockByNumber(3).length() - 1, QTextCursor::KeepAnchor);
    return addBookmark(doc, all, kCoverMark, problem, true);
}

bool insertTextFromFile(WriteDoc &doc, QTextCursor &c, const QByteArray &bytes, QString *problem)
{
    auto fail = [&](const QString &why) {
        if (problem) *problem = why;
        return false;
    };
    if (bytes.size() > kMostInsertedFile) return fail(QCoreApplication::translate("jp::write::InsertText", "That file is too large to put in a document."));
    if (c.document() != doc.story()) return fail(QCoreApplication::translate("jp::write::InsertText", "Text from a file goes in the text, not in a header, a footer, or a note."));
    QString err;
    std::unique_ptr<WriteDoc> other = WriteDoc::fromFile(bytes, &err);
    const WriteDoc::StepGroup group(doc);
    if (other) {
        QTextCursor all(other->story());
        all.setPosition(0);
        all.setPosition(editableEnd(other->story()), QTextCursor::KeepAnchor);
        if (other->story()->blockCount() > kMostInsertedParagraphs)
            return fail(QCoreApplication::translate("jp::write::InsertText", "That file has more than %1 paragraphs, which is more than can be put in at once.").arg(QLocale().toString(kMostInsertedParagraphs)));
        if (!all.hasSelection()) return fail(QCoreApplication::translate("jp::write::InsertText", "That file has no text."));
        const Clip clip = copySelection(*other, all);
        pasteClip(doc, c, clip);
        return true;
    }
    // A plain text file: a paragraph for each line.
    if (bytes.contains('\0')) return fail(QCoreApplication::translate("jp::write::InsertText", "JeffWrite can't read that file as a document or as text."));
    QString text = QString::fromUtf8(bytes);
    text.replace(QStringLiteral("\r\n"), QStringLiteral("\n")).replace(QLatin1Char('\r'), QLatin1Char('\n'));
    if (text.endsWith(QLatin1Char('\n'))) text.chop(1);
    // (A control character is not text: a page or column break comes from the Insert and Layout commands, never from a byte of a file.)
    QString kept;
    kept.reserve(text.size());
    for (const QChar ch : std::as_const(text)) {
        const ushort u = ch.unicode();
        if ((u < 0x20 && u != '\t' && u != '\n') || (u >= 0x7F && u <= 0x9F) || u == 0x2028 || u == 0x2029) continue;
        kept += ch;
    }
    text = kept;
    if (text.count(QLatin1Char('\n')) + 1 > kMostInsertedParagraphs)
        return fail(QCoreApplication::translate("jp::write::InsertText", "That file has more than %1 lines, which is more than can be put in at once.").arg(QLocale().toString(kMostInsertedParagraphs)));
    if (text.isEmpty()) return fail(QCoreApplication::translate("jp::write::InsertText", "That file has no text."));
    insertText(doc, c, text);
    return true;
}

} // namespace jp::write
