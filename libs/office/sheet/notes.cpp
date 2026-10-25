#include "sheet/notes.h"

#include "sheet/cellref.h"
#include "sheet/xmlscan.h"

#include <QIODevice>
#include <QXmlStreamReader>

#include <atomic>
#include <algorithm>
#include <memory>

namespace jp::office::sheet {

namespace {
std::atomic<qint64> g_noteReads{0};

// The name of each person a thread's comments are from, by the id the thread names.
std::map<QString, QString> readPersons(const Workbook &book)
{
    std::map<QString, QString> out;
    for (const Relationship &rel : book.package().relationshipsOfType(book.mainPart(), QStringLiteral("/person"))) {
        const qint64 size = rel.external ? 0 : book.package().partSize(rel.target);
        if (size <= 0 || size > (qint64(4) << 20)) continue;
        const QByteArray bytes = book.package().part(rel.target);
        QXmlStreamReader r(bytes);
        int count = 0;
        while (!r.atEnd() && count < 10000) {
            if (r.readNext() == QXmlStreamReader::DTD) break;
            if (r.isStartElement() && r.name() == QLatin1String("person")) {
                ++count;
                out[r.attributes().value(QLatin1String("id")).toString()] = r.attributes().value(QLatin1String("displayName")).toString();
            }
        }
    }
    return out;
}

// The text of each thread of a sheet: by cell, its comments (the first, then the replies) with who wrote each, one under the other.
std::map<quint64, QString> readThreads(const Workbook &book, const QString &sheetPart, bool *truncated)
{
    std::map<quint64, QString> out;
    const auto parts = book.package().relationshipsOfType(sheetPart, QStringLiteral("/threadedComment"));
    if (parts.isEmpty()) return out;
    const std::map<QString, QString> persons = readPersons(book);
    qint64 chars = 0;
    for (const Relationship &rel : parts) {
        const qint64 size = rel.external ? 0 : book.package().partSize(rel.target);
        if (size <= 0 || size > kMaxNotesPartBytes) continue;
        const std::unique_ptr<QIODevice> device = book.package().openPart(rel.target);
        if (!device) continue;
        QXmlStreamReader r(device.get());
        int depth = 0;
        bool inComment = false, inText = false;
        quint32 row = 0, col = 0;
        QString who, text;
        while (!r.atEnd()) {
            switch (r.readNext()) {
            case QXmlStreamReader::DTD: return out;
            case QXmlStreamReader::StartElement:
                if (++depth > 40) return out;
                if (r.name() == QLatin1String("threadedComment")) {
                    row = col = 0;
                    inComment = parseCellRef(r.attributes().value(QLatin1String("ref")), &row, &col);
                    text.clear();
                    const auto p = persons.find(r.attributes().value(QLatin1String("personId")).toString());
                    who = p == persons.end() ? QString() : p->second;
                } else if (inComment && r.name() == QLatin1String("text")) {
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
                if (r.name() == QLatin1String("text")) inText = false;
                if (r.name() == QLatin1String("threadedComment") && inComment) {
                    inComment = false;
                    if (qint64(out.size()) >= kMaxNotes) {
                        if (truncated) *truncated = true;
                        return out;
                    }
                    QString &into = out[noteKey(row, col)];
                    if (!into.isEmpty()) into += QStringLiteral("\n\n");
                    xmlscan::decodeEscapes(&text);
                    into += who.isEmpty() ? text : who + QStringLiteral(": ") + text;
                }
                break;
            default: break;
            }
        }
    }
    return out;
}

} // namespace

qint64 noteReads() { return g_noteReads.load(); }

std::vector<DrawingShape> drawingShapes(const QByteArray &vml, qint64 most)
{
    std::vector<DrawingShape> out;
    xmlscan::Tag t;
    qsizetype pos = 0;
    int depth = 0;   // (how many `shape` elements we are inside: those inside another are part of it)
    qsizetype start = 0;
    qint64 tags = 0;
    while (xmlscan::nextTag(vml, pos, &t)) {
        pos = t.end;
        if (++tags > most * 64) break;
        if (t.local != "shape") continue;
        if (t.closing) {
            if (depth > 0 && --depth == 0) {
                out.push_back({start, t.end});
                if (qint64(out.size()) >= most) break;
            }
        } else if (t.selfClosing) {
            if (depth == 0) out.push_back({t.start, t.end});
            if (qint64(out.size()) >= most) break;
        } else if (depth++ == 0) {
            start = t.start;
        }
    }
    return out;
}

bool shapeNote(const QByteArray &vml, const DrawingShape &shape, quint32 *row, quint32 *col, bool *shown)
{
    const QByteArrayView body = QByteArrayView(vml).sliced(shape.start, shape.end - shape.start);
    xmlscan::Tag t;
    qsizetype pos = 0;
    bool isNote = false, haveRow = false, haveCol = false, visible = false, first = true;
    quint32 r = 0, c = 0;
    auto number = [&](qsizetype from, quint32 *value) {
        qsizetype e = from;
        while (e < body.size() && body[e] == ' ') ++e;
        quint64 v = 0;
        int digits = 0;
        while (e < body.size() && body[e] >= '0' && body[e] <= '9' && digits < 10) {
            v = v * 10 + quint64(body[e] - '0');
            ++e;
            ++digits;
        }
        *value = quint32(std::min<quint64>(v, 0xFFFFFFFFu));
        return digits > 0;
    };
    qint64 tags = 0;
    while (xmlscan::nextTag(body, pos, &t)) {
        pos = t.end;
        if (++tags > 4096) break;
        if (t.closing) continue;
        if (first) {
            first = false;
            bool found = false;
            const QByteArrayView style = xmlscan::attribute(body.sliced(t.start, t.end - t.start), "style", &found);
            if (found && style.contains("visibility") ) {
                const qsizetype at = style.indexOf("visibility");
                qsizetype e = at + 10;
                while (e < style.size() && (style[e] == ' ' || style[e] == ':')) ++e;
                visible = style.sliced(e).startsWith("visible");
            }
            continue;
        }
        if (t.local == "ClientData") {
            const QByteArrayView type = xmlscan::attribute(body.sliced(t.start, t.end - t.start), "ObjectType");
            isNote = type == "Note";
        } else if (t.local == "Row") {
            haveRow = number(t.end, &r);
        } else if (t.local == "Column") {
            haveCol = number(t.end, &c);
        } else if (t.local == "Visible") {
            visible = true;
        }
    }
    if (!isNote || !haveRow || !haveCol) return false;
    *row = r;
    *col = c;
    *shown = visible;
    return true;
}

std::map<quint64, bool> noteShapes(const QByteArray &vml)
{
    std::map<quint64, bool> out;
    for (const DrawingShape &s : drawingShapes(vml)) {
        quint32 row = 0, col = 0;
        bool shown = false;
        if (shapeNote(vml, s, &row, &col, &shown)) {
            out[noteKey(row, col)] = shown;
            if (qint64(out.size()) >= kMaxNotes) break;
        }
    }
    return out;
}

QString notesDrawingPart(const Package &package, const QString &sheetPart, QByteArray *bytes)
{
    if (bytes) bytes->clear();
    QString spare;
    QByteArray spareBytes;
    for (const Relationship &rel : package.relationshipsOfType(sheetPart, QStringLiteral("/vmlDrawing"))) {
        if (rel.external) continue;
        const qint64 size = package.partSize(rel.target);
        if (size <= 0 || size > kMaxNotesDrawingBytes) continue;
        const QByteArray data = package.part(rel.target);
        if (data.contains("Note") && !noteShapes(data).empty()) {
            if (bytes) *bytes = data;
            return rel.target;
        }
        // (a drawing of the sheet's form controls can hold the notes' shapes too; the header and footer pictures' drawing, whose shapes are named for where they go, is not it)
        static const char *const picturePlaces[] = {"id=\"LH\"", "id=\"CH\"", "id=\"RH\"", "id=\"LF\"", "id=\"CF\"", "id=\"RF\""};
        bool forPictures = false;
        for (const char *place : picturePlaces) forPictures = forPictures || data.contains(place);
        if (!forPictures && spare.isEmpty()) {
            spare = rel.target;
            spareBytes = data;
        }
    }
    if (bytes) *bytes = spareBytes;
    return spare;
}

std::map<quint64, NoteInfo> readSheetNoteInfos(const Workbook &book, int sheet, bool *truncated, qint64 most)
{
    std::map<quint64, NoteInfo> out;
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
    g_noteReads.fetch_add(1);
    {
        // (read as a stream: the part is not held whole)
        const std::unique_ptr<QIODevice> device = book.package().openPart(commentsPart);
        if (!device) return out;
        QXmlStreamReader r(device.get());
        int depth = 0;
        bool inComment = false, inText = false, inAuthors = false, inAuthor = false, inPhonetic = false;
        quint32 row = 0, col = 0;
        int authorId = 0;
        QString text, authorText;
        QStringList authors;
        qint64 chars = 0;
        bool stopped = false;
        while (!r.atEnd() && !stopped) {
            switch (r.readNext()) {
            case QXmlStreamReader::DTD: stopped = true; break;
            case QXmlStreamReader::StartElement:
                if (++depth > 40) {
                    stopped = true;
                    break;
                }
                if (r.name() == QLatin1String("authors")) {
                    inAuthors = true;
                } else if (inAuthors && r.name() == QLatin1String("author")) {
                    inAuthor = true;
                    authorText.clear();
                } else if (r.name() == QLatin1String("comment")) {
                    inComment = true;
                    text.clear();
                    row = col = 0;
                    authorId = r.attributes().value(QLatin1String("authorId")).toInt();
                    if (!parseCellRef(r.attributes().value(QLatin1String("ref")), &row, &col)) inComment = false;
                } else if (inComment && r.name() == QLatin1String("rPh")) {
                    inPhonetic = true;
                } else if (inComment && !inPhonetic && r.name() == QLatin1String("t")) {
                    inText = true;
                }
                break;
            case QXmlStreamReader::Characters:
                if (inAuthor) {
                    authorText += r.text();
                    if (authorText.size() > 1000) authorText.truncate(1000);
                } else if (inComment && inText) {
                    chars += r.text().size();
                    if (chars > kMaxNoteChars) {
                        if (truncated) *truncated = true;
                        stopped = true;
                        break;
                    }
                    text += r.text();
                }
                break;
            case QXmlStreamReader::EndElement:
                --depth;
                if (r.name() == QLatin1String("t")) inText = false;
                if (r.name() == QLatin1String("rPh")) inPhonetic = false;
                if (r.name() == QLatin1String("authors")) inAuthors = false;
                if (r.name() == QLatin1String("author") && inAuthor) {
                    inAuthor = false;
                    xmlscan::decodeEscapes(&authorText);
                    if (authors.size() < 100000) authors << authorText;
                }
                if (r.name() == QLatin1String("comment") && inComment) {
                    inComment = false;
                    if (qint64(out.size()) >= std::min(most, kMaxNotes)) {
                        if (truncated) *truncated = true;
                        stopped = true;
                        break;
                    }
                    if (!text.isEmpty()) {
                        NoteInfo &n = out[noteKey(row, col)];
                        xmlscan::decodeEscapes(&text);
                        n.text = text;
                        n.author = authorId >= 0 && authorId < authors.size() ? authors[authorId] : QString();
                        // (a comment of a thread has an author named "tc=" and the id of the thread)
                        n.threaded = n.author.startsWith(QLatin1String("tc="));
                        if (n.threaded) n.author.clear();
                    }
                }
                break;
            default: break;
            }
        }
    }
    bool anyThreaded = false;
    for (const auto &n : out) anyThreaded = anyThreaded || n.second.threaded;
    if (anyThreaded) {
        const std::map<quint64, QString> threads = readThreads(book, info.sheets[sheet].part, truncated);
        for (auto &n : out) {
            if (!n.second.threaded) continue;
            const auto t = threads.find(n.first);
            if (t != threads.end()) n.second.text = t->second;
        }
    }
    QByteArray vml;
    if (!notesDrawingPart(book.package(), info.sheets[sheet].part, &vml).isEmpty()) {
        const std::map<quint64, bool> shown = noteShapes(vml);
        for (auto &n : out) {
            const auto s = shown.find(n.first);
            n.second.shown = s != shown.end() && s->second;
        }
    }
    return out;
}

std::map<quint64, QString> readSheetNotes(const Workbook &book, int sheet, bool *truncated)
{
    std::map<quint64, QString> out;
    for (const auto &n : readSheetNoteInfos(book, sheet, truncated)) out[n.first] = n.second.text;
    return out;
}

} // namespace jp::office::sheet
