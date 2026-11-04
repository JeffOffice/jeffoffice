#include "core/insertops.h"

#include "core/editops.h"
#include "core/writedoc.h"

#include <QCoreApplication>
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
    if (text.isEmpty()) return fail(QCoreApplication::translate("jp::write::InsertText", "That file has no text."));
    insertText(doc, c, text);
    return true;
}

} // namespace jp::write
