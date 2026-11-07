#include "sheet/notes.h"

#include "sheet/cellref.h"
#include "sheet/xmlscan.h"

#include <QIODevice>
#include <QXmlStreamReader>

#include <atomic>
#include <algorithm>
#include <memory>
#include <set>

namespace jp::office::sheet {

namespace {
std::atomic<qint64> g_noteReads{0};
std::atomic<qint64> g_threadPartReads{0};

// A token of the reader written out again as XML text (an element is written with its own prefix and the namespaces it declares).
void appendToken(QString *out, const QXmlStreamReader &r)
{
    auto escape = [](const QString &t) { return t.toHtmlEscaped().replace(QLatin1Char('"'), QStringLiteral("&quot;")); };
    switch (r.tokenType()) {
    case QXmlStreamReader::StartElement:
        *out += QLatin1Char('<') + r.qualifiedName().toString();
        for (const QXmlStreamNamespaceDeclaration &d : r.namespaceDeclarations())
            *out += QStringLiteral(" xmlns") + (d.prefix().isEmpty() ? QString() : QLatin1Char(':') + d.prefix().toString()) + QStringLiteral("=\"") + escape(d.namespaceUri().toString()) + QLatin1Char('"');
        for (const QXmlStreamAttribute &a : r.attributes()) *out += QLatin1Char(' ') + a.qualifiedName().toString() + QStringLiteral("=\"") + escape(a.value().toString()) + QLatin1Char('"');
        *out += QLatin1Char('>');
        break;
    case QXmlStreamReader::EndElement: *out += QStringLiteral("</") + r.qualifiedName().toString() + QLatin1Char('>'); break;
    case QXmlStreamReader::Characters: *out += r.text().toString().toHtmlEscaped(); break;
    default: break;
    }
}

// The name of each person a thread's comments are from, by the id the thread names: read once for the workbook, each name cut to kMaxPersonChars and counted in the budget.
const std::map<QString, QString> &readPersons(const Workbook &book, NoteReadContext &ctx)
{
    if (ctx.personsRead) return ctx.persons;
    ctx.personsRead = true;
    for (const Relationship &rel : book.package().relationshipsOfType(book.mainPart(), QStringLiteral("/person"))) {
        const qint64 size = rel.external ? 0 : book.package().partSize(rel.target);
        if (size <= 0 || size > (qint64(4) << 20)) continue;
        const QByteArray bytes = book.package().part(rel.target);
        QXmlStreamReader r(bytes);
        while (!r.atEnd() && ctx.persons.size() < kMaxPersons) {
            if (r.readNext() == QXmlStreamReader::DTD) break;
            if (r.isStartElement() && r.name() == QLatin1String("person")) {
                QString name = r.attributes().value(QLatin1String("displayName")).toString().left(kMaxPersonChars);
                QString id = r.attributes().value(QLatin1String("id")).toString().left(kMaxPersonChars);
                const QString userId = r.attributes().value(QLatin1String("userId")).toString().left(kMaxPersonChars);
                const QString providerId = r.attributes().value(QLatin1String("providerId")).toString().left(kMaxPersonChars);
                ctx.chars += name.size() + id.size() + userId.size() + providerId.size();
                if (ctx.chars > kMaxNoteCharsPerBook) {
                    ctx.exhausted = true;
                    return ctx.persons;
                }
                ctx.persons[id] = name;
                ctx.personIds[id] = {userId, providerId};
            }
        }
    }
    return ctx.persons;
}

// The comments of each thread of one thread part: by cell, in the order the file has them (the first, then the replies), each with its person. A thread with more than kMaxThreadComments has
// the first of them and is told in `cut`.
void parseThreadPart(const Workbook &book, const Relationship &rel, const std::map<QString, QString> &persons, NoteReadContext &ctx, std::map<quint64, ThreadList> &out, bool *truncated,
                     std::set<quint64> *cut)
{
    qint64 &chars = ctx.chars;
    const qint64 size = rel.external ? 0 : book.package().partSize(rel.target);
    if (size <= 0 || size > kMaxNotesPartBytes) return;
    const std::unique_ptr<QIODevice> device = book.package().openPart(rel.target);
    if (!device) return;
    QXmlStreamReader r(device.get());
    int depth = 0, commentDepth = 0;
    bool inComment = false, inText = false;
    quint32 row = 0, col = 0;
    ThreadComment c;
    QString extra;
    bool extraTooBig = false, capturing = false;
    while (!r.atEnd()) {
        const QXmlStreamReader::TokenType token = r.readNext();
        switch (token) {
        case QXmlStreamReader::DTD: return;
        case QXmlStreamReader::StartElement:
            if (++depth > 40) return;
            if (!inComment && r.name() == QLatin1String("threadedComment")) {
                row = col = 0;
                inComment = parseCellRef(r.attributes().value(QLatin1String("ref")), &row, &col);
                commentDepth = depth;
                c = ThreadComment();
                extra.clear();
                extraTooBig = false;
                capturing = false;
                const QXmlStreamAttributes a = r.attributes();
                c.id = a.value(QLatin1String("id")).toString().left(kMaxPersonChars);
                c.parentId = a.value(QLatin1String("parentId")).toString().left(kMaxPersonChars);
                c.personId = a.value(QLatin1String("personId")).toString().left(kMaxPersonChars);
                c.time = a.value(QLatin1String("dT")).toString().left(64);
                const QStringView done = a.value(QLatin1String("done"));
                c.done = done == QLatin1String("1") || done == QLatin1String("true");
                const auto p = persons.find(c.personId);
                if (p != persons.end()) {
                    c.author = p->second;
                    const auto ids = ctx.personIds.find(c.personId);
                    if (ids != ctx.personIds.end()) {
                        c.userId = ids->second.first;
                        c.providerId = ids->second.second;
                    }
                }
            } else if (inComment && depth == commentDepth + 1 && r.name() == QLatin1String("text")) {
                inText = true;
            } else if (inComment && depth == commentDepth + 1) {
                capturing = true;   // (what follows the words, such as who is mentioned, is kept as it is written)
            }
            if (capturing && !extraTooBig) {
                appendToken(&extra, r);
                if (extra.size() > kMaxThreadExtraChars) {
                    extraTooBig = true;
                    extra.clear();
                }
            }
            break;
        case QXmlStreamReader::Characters:
            if (capturing && !extraTooBig) appendToken(&extra, r);
            if (inComment && inText) {
                chars += r.text().size();
                if (chars > kMaxNoteCharsPerBook) {
                    ctx.exhausted = true;
                    if (truncated) *truncated = true;
                    return;
                }
                c.text += r.text();
            }
            break;
        case QXmlStreamReader::EndElement:
            --depth;
            if (capturing && !extraTooBig) appendToken(&extra, r);
            if (capturing && depth == commentDepth) capturing = false;
            if (inComment && depth == commentDepth && r.name() == QLatin1String("text")) inText = false;
            if (inComment && depth == commentDepth - 1 && r.name() == QLatin1String("threadedComment")) {
                inComment = false;
                if (qint64(out.size()) >= kMaxNotes && !out.count(noteKey(row, col))) {
                    if (truncated) *truncated = true;
                    return;
                }
                ThreadList &into = out[noteKey(row, col)];
                if (ctx.threadComments >= kMaxThreadCommentsPerBook) {
                    if (truncated) *truncated = true;
                    if (cut) cut->insert(noteKey(row, col));
                    break;
                }
                if (into.size() >= size_t(kMaxThreadComments)) {
                    if (cut) cut->insert(noteKey(row, col));
                    break;
                }
                // (what a comment holds besides its words is charged to the workbook's budget too: the extra and the ids)
                chars += extra.size() + c.id.size() + c.parentId.size() + c.personId.size();
                if (chars > kMaxNoteCharsPerBook) {
                    ctx.exhausted = true;
                    if (truncated) *truncated = true;
                    return;
                }
                ++ctx.threadComments;
                xmlscan::decodeEscapes(&c.text);
                c.extra = extra;
                into.push_back(std::move(c));
                c = ThreadComment();
            }
            break;
        default: break;
        }
    }
}

// The comments of each thread of a sheet. A part is parsed once for the workbook, whatever number of sheets name it.
std::map<quint64, ThreadList> readThreads(const Workbook &book, const QString &sheetPart, bool *truncated, NoteReadContext &ctx, std::set<quint64> *cut)
{
    std::map<quint64, ThreadList> out;
    const auto parts = book.package().relationshipsOfType(sheetPart, QStringLiteral("/threadedComment"));
    if (parts.isEmpty()) return out;
    const std::map<QString, QString> &persons = readPersons(book, ctx);
    for (const Relationship &rel : parts) {
        auto found = ctx.threadParts.find(rel.target);
        if (found == ctx.threadParts.end()) {
            g_threadPartReads.fetch_add(1);
            NoteReadContext::ThreadPart &part = ctx.threadParts[rel.target];
            if (!ctx.exhausted) parseThreadPart(book, rel, persons, ctx, part.threads, &part.truncated, &part.cut);
            else part.truncated = true;
            found = ctx.threadParts.find(rel.target);
        }
        const NoteReadContext::ThreadPart &part = found->second;
        if (part.truncated && truncated) *truncated = true;
        for (const auto &cell : part.threads) {
            if (qint64(out.size()) >= kMaxNotes && !out.count(cell.first)) {
                if (truncated) *truncated = true;
                break;
            }
            ThreadList &into = out[cell.first];
            for (const ThreadComment &c : cell.second) {
                if (into.size() >= size_t(kMaxThreadComments)) {
                    if (cut) cut->insert(cell.first);
                    break;
                }
                into.push_back(c);
            }
            if (part.cut.count(cell.first) && cut) cut->insert(cell.first);
        }
    }
    return out;
}

} // namespace

qint64 noteReads() { return g_noteReads.load(); }
qint64 threadPartReads() { return g_threadPartReads.load(); }

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

// The notes of a comments part as the file has them (by cell: words, author, whether it is a thread's), read once for the workbook whatever number of sheets name the part.
const std::map<quint64, NoteInfo> &commentsOf(const Workbook &book, const QString &commentsPart, bool *truncated, NoteReadContext &ctx)
{
    const auto cached = ctx.parts.find(commentsPart);
    if (cached != ctx.parts.end()) return cached->second;
    std::map<quint64, NoteInfo> &out = ctx.parts[commentsPart];
    const qint64 size = book.package().partSize(commentsPart);
    if (size <= 0 || size > kMaxNotesPartBytes) {
        if (truncated && size > kMaxNotesPartBytes) *truncated = true;
        return out;
    }
    if (ctx.exhausted) {
        if (truncated) *truncated = true;
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
                ctx.chars += r.text().size();
                if (ctx.chars > kMaxNoteCharsPerBook) {
                    ctx.exhausted = true;
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
                if (qint64(out.size()) >= kMaxNotes) {
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
    return out;
}

std::map<quint64, NoteInfo> readSheetNoteInfos(const Workbook &book, int sheet, bool *truncated, qint64 most, NoteReadContext *context)
{
    NoteReadContext local;
    NoteReadContext &c = context ? *context : local;
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
    const bool hasThreads = !book.package().relationshipsOfType(info.sheets[sheet].part, QStringLiteral("/threadedComment")).isEmpty();
    if (commentsPart.isEmpty() && !hasThreads) return out;
    if (!commentsPart.isEmpty()) {
        const std::map<quint64, NoteInfo> &raw = commentsOf(book, commentsPart, truncated, c);
        qint64 taken = 0;
        for (const auto &n : raw) {
            if (taken >= std::min(most, kMaxNotes)) {
                if (truncated) *truncated = true;
                break;
            }
            out.insert(out.end(), n);
            ++taken;
        }
    }
    // (a thread's older note is only its stand-in: a note and a thread on one cell are one thread, and editing it writes over the note, as the format's main producer does)
    // the threads: each one's cell shows its comments (an older note on the cell stands for it; a thread with no older note has a note made for it)
    std::set<quint64> cut;
    if (hasThreads) {
        std::map<quint64, ThreadList> threads = readThreads(book, info.sheets[sheet].part, truncated, c, &cut);
        for (auto &t : threads) {
            if (t.second.empty()) continue;
            if (!out.count(t.first) && qint64(out.size()) >= std::min(most, kMaxNotes)) {
                if (truncated) *truncated = true;
                continue;
            }
            NoteInfo &n = out[t.first];
            n.threaded = true;
            n.author.clear();
            n.text = threadWords(t.second);
            n.threadCut = cut.count(t.first) > 0;
            n.thread = std::make_shared<const ThreadList>(std::move(t.second));
            n.fileThread = n.thread;
        }
    }
    // (an older note made for a thread that is not in the file is only a note)
    for (auto &n : out)
        if (n.second.threaded && !n.second.thread) n.second.threaded = false;
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
