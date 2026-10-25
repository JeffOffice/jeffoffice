#include "sheet/notes.h"

#include "sheet/cellref.h"

#include <QIODevice>
#include <QXmlStreamReader>

#include <memory>

namespace jp::office::sheet {

std::map<quint64, QString> readSheetNotes(const Workbook &book, int sheet, bool *truncated)
{
    std::map<quint64, QString> out;
    if (truncated) *truncated = false;
    const WorkbookInfo &info = book.info();
    if (sheet < 0 || sheet >= info.sheets.size() || info.sheets[sheet].deleted || info.sheets[sheet].isNew || info.sheets[sheet].part.isEmpty()) return out;
    if (book.isConverted()) return out;
    QString commentsPart;
    for (const Relationship &rel : book.package().relationships(info.sheets[sheet].part))
        if (!rel.external && rel.type.endsWith(QLatin1String("/comments"))) {
            commentsPart = rel.target;
            break;
        }
    if (commentsPart.isEmpty()) return out;
    const qint64 size = book.package().partSize(commentsPart);
    if (size <= 0 || size > kMaxNotesPartBytes) {
        if (truncated && size > kMaxNotesPartBytes) *truncated = true;
        return out;
    }
    // (read as a stream: the part is not held whole)
    const std::unique_ptr<QIODevice> device = book.package().openPart(commentsPart);
    if (!device) return out;
    QXmlStreamReader r(device.get());
    int depth = 0;
    bool inComment = false, inText = false;
    quint32 row = 0, col = 0;
    QString text;
    qint64 chars = 0;
    while (!r.atEnd()) {
        switch (r.readNext()) {
        case QXmlStreamReader::DTD: return out;
        case QXmlStreamReader::StartElement:
            if (++depth > 40) return out;
            if (r.name() == QLatin1String("comment")) {
                inComment = true;
                text.clear();
                row = col = 0;
                if (!parseCellRef(r.attributes().value(QLatin1String("ref")), &row, &col)) inComment = false;
            } else if (inComment && r.name() == QLatin1String("t")) {
                inText = true;
            }
            break;
        case QXmlStreamReader::Characters:
            if (inComment && inText) {
                chars += r.text().size();
                if (chars > kMaxNoteChars) {
                    if (truncated) *truncated = true;
                    return out;
                }
                text += r.text();
            }
            break;
        case QXmlStreamReader::EndElement:
            --depth;
            if (r.name() == QLatin1String("t")) inText = false;
            if (r.name() == QLatin1String("comment") && inComment) {
                inComment = false;
                if (qint64(out.size()) >= kMaxNotes) {
                    if (truncated) *truncated = true;
                    return out;
                }
                if (!text.isEmpty()) out[(quint64(row) << 32) | col] = text;
            }
            break;
        default: break;
        }
    }
    return out;
}

} // namespace jp::office::sheet
