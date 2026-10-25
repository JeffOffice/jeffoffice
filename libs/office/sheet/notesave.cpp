#include "sheet/notesave.h"

#include "sheet/cellref.h"
#include "sheet/notes.h"
#include "sheet/xmlscan.h"

#include <QCoreApplication>
#include <QHash>

#include <algorithm>
#include <map>

namespace jp::office::sheet {

namespace {

using xmlscan::Tag;

// One element of the comments part, whole: where it starts and ends.
struct Span {
    qsizetype start = 0, end = 0;
};

// The element that begins with the open tag `open` (found by `nextTag`), up to and with its close tag; false if it is cut off.
bool wholeElement(const QByteArray &xml, const Tag &open, qsizetype *end)
{
    if (open.selfClosing) {
        *end = open.end;
        return true;
    }
    int depth = 1;
    Tag t;
    qsizetype pos = open.end;
    while (xmlscan::nextTag(xml, pos, &t)) {
        pos = t.end;
        if (t.closing) {
            if (--depth == 0) {
                *end = t.end;
                return true;
            }
        } else if (!t.selfClosing) {
            ++depth;
        }
    }
    return false;
}

QByteArray commentXml(const QByteArray &prefix, const NoteOp &op, int authorId)
{
    QByteArray x = "<" + prefix + "comment ref=\"" + cellName(op.row, op.col).toLatin1() + "\" authorId=\"" + QByteArray::number(authorId) + "\" shapeId=\"0\"><" + prefix + "text><" + prefix
                   + "t xml:space=\"preserve\">" + xmlscan::escapeText(op.text) + "</" + prefix + "t></" + prefix + "text></" + prefix + "comment>";
    return x;
}

QByteArray authorXml(const QByteArray &prefix, const QString &name) { return "<" + prefix + "author>" + xmlscan::escapeText(name) + "</" + prefix + "author>"; }

bool sameCell(const NoteOp &a, quint32 row, quint32 col) { return a.row == row && a.col == col; }

} // namespace

bool applyNoteOpsToComments(const QByteArray &xml, const std::vector<NoteOp> &ops, QByteArray *out, QString *why)
{
    auto fail = [&](const QString &text) {
        if (why) *why = text;
        return false;
    };
    if (qint64(ops.size()) > kMaxNoteOps) return fail(QCoreApplication::translate("jp::office::sheet::Save", "There are more changes to notes than a sheet's notes are saved with."));
    // the pieces: the root, the authors, the list of comments and each comment in it
    Tag t, root, authors, list;
    bool haveRoot = false, haveAuthors = false, haveList = false;
    qsizetype authorsEnd = 0, listEnd = 0;
    std::vector<Span> comments;
    std::vector<std::pair<quint32, quint32>> commentCells;
    QStringList names;
    qsizetype pos = 0;
    int depth = 0;
    while (xmlscan::nextTag(xml, pos, &t)) {
        pos = t.end;
        if (t.closing) {
            --depth;
            continue;
        }
        if (!haveRoot) {
            root = t;
            haveRoot = true;
        } else if (t.local == "authors" && !haveAuthors && depth == 1) {
            authors = t;
            haveAuthors = true;
            if (!wholeElement(xml, t, &authorsEnd)) return fail(QCoreApplication::translate("jp::office::sheet::Save", "The notes part of the file is cut off."));
            // the names
            Tag a;
            qsizetype p = t.end;
            while (!t.selfClosing && xmlscan::nextTag(xml, p, &a) && a.start < authorsEnd) {
                p = a.end;
                if (a.closing || a.local != "author") continue;
                qsizetype e = 0;
                if (!wholeElement(xml, a, &e)) break;
                const qsizetype closeStart = xml.lastIndexOf('<', e - 1);
                names << (a.selfClosing ? QString() : xmlscan::unescape(QByteArrayView(xml).sliced(a.end, closeStart - a.end)));
                p = e;
                if (names.size() > 100000) return fail(QCoreApplication::translate("jp::office::sheet::Save", "The notes part of the file has too many authors."));
            }
            pos = authorsEnd;
            continue;
        } else if (t.local == "commentList" && !haveList && depth == 1) {
            list = t;
            haveList = true;
            if (!wholeElement(xml, t, &listEnd)) return fail(QCoreApplication::translate("jp::office::sheet::Save", "The notes part of the file is cut off."));
            Tag c;
            qsizetype p = t.end;
            while (!t.selfClosing && xmlscan::nextTag(xml, p, &c) && c.start < listEnd) {
                p = c.end;
                if (c.closing || c.local != "comment") continue;
                qsizetype e = 0;
                if (!wholeElement(xml, c, &e)) return fail(QCoreApplication::translate("jp::office::sheet::Save", "The notes part of the file is cut off."));
                quint32 row = 0, col = 0;
                bool found = false;
                const QString ref = xmlscan::unescape(xmlscan::attribute(QByteArrayView(xml).sliced(c.start, c.end - c.start), "ref", &found));
                if (found && parseCellRef(ref, &row, &col)) {
                    comments.push_back({c.start, e});
                    commentCells.emplace_back(row, col);
                } else {
                    comments.push_back({c.start, e});
                    commentCells.emplace_back(0xFFFFFFFFu, 0xFFFFFFFFu);   // (one with no place is kept as it is)
                }
                p = e;
                if (comments.size() > size_t(kMaxNotes) + 1000) return fail(QCoreApplication::translate("jp::office::sheet::Save", "The notes part of the file has too many notes."));
            }
            pos = listEnd;
            continue;
        }
        if (!t.selfClosing) ++depth;
    }
    if (!haveRoot || !haveList) return fail(QCoreApplication::translate("jp::office::sheet::Save", "The notes part of the file is not in the form this program changes."));
    const QByteArray prefix(xmlscan::prefixOf(root.name));
    // the authors the changes need (by name, so that many changes do not each look through all the authors)
    QByteArray addedAuthors;
    QHash<QString, int> authorAt;
    for (int a = 0; a < names.size(); ++a)
        if (!authorAt.contains(names[a])) authorAt.insert(names[a], a);
    auto authorIndex = [&](const QString &name) {
        const auto found = authorAt.constFind(name);
        if (found != authorAt.constEnd()) return found.value();
        names << name;
        authorAt.insert(name, int(names.size()) - 1);
        addedAuthors += authorXml(prefix, name);
        return int(names.size()) - 1;
    };
    // the new comments list: the old ones in order, those that are changed replaced or dropped, and new ones where they sort
    QByteArray listBody;
    size_t next = 0;   // the next op not yet put
    auto putOps = [&](bool before, quint32 row, quint32 col) {
        // ops sorted by cell: those for cells before (row, col) that no comment has are put now
        while (next < ops.size() && (before ? (ops[next].row < row || (ops[next].row == row && ops[next].col < col)) : true)) {
            if (!ops[next].remove) listBody += commentXml(prefix, ops[next], authorIndex(ops[next].author));
            ++next;
        }
    };
    for (size_t i = 0; i < comments.size(); ++i) {
        const quint32 row = commentCells[i].first, col = commentCells[i].second;
        if (row == 0xFFFFFFFFu) {
            listBody += xml.mid(comments[i].start, comments[i].end - comments[i].start);
            continue;
        }
        putOps(true, row, col);
        if (next < ops.size() && sameCell(ops[next], row, col)) {
            if (!ops[next].remove) listBody += commentXml(prefix, ops[next], authorIndex(ops[next].author));
            ++next;
            continue;
        }
        listBody += xml.mid(comments[i].start, comments[i].end - comments[i].start);
    }
    putOps(false, 0, 0);
    // put together
    QByteArray result;
    QByteArray listOpen = xml.mid(list.start, list.end - list.start);
    if (list.selfClosing) {
        listOpen.chop(2);
        while (listOpen.endsWith(' ')) listOpen.chop(1);
        listOpen += ">";
    }
    const QByteArray listClose = list.selfClosing ? "</" + prefix + "commentList>" : xml.mid(xml.lastIndexOf('<', listEnd - 1), listEnd - xml.lastIndexOf('<', listEnd - 1));
    const qsizetype listAfter = listEnd;
    if (haveAuthors && authors.start < list.start) {
        result += xml.left(authors.start);
        if (authors.selfClosing) {
            QByteArray open = xml.mid(authors.start, authors.end - authors.start);
            open.chop(2);
            while (open.endsWith(' ')) open.chop(1);
            result += open + ">" + addedAuthors + "</" + prefix + "authors>";
        } else {
            const qsizetype closeAt = xml.lastIndexOf('<', authorsEnd - 1);
            result += xml.mid(authors.start, closeAt - authors.start) + addedAuthors + xml.mid(closeAt, authorsEnd - closeAt);
        }
        result += xml.mid(authorsEnd, list.start - authorsEnd);
    } else if (!haveAuthors) {
        result += xml.left(list.start);
        if (!addedAuthors.isEmpty()) result += "<" + prefix + "authors>" + addedAuthors + "</" + prefix + "authors>";
    } else {
        return fail(QCoreApplication::translate("jp::office::sheet::Save", "The notes part of the file is not in the form this program changes."));
    }
    result += listOpen + listBody + listClose + xml.mid(listAfter);
    *out = result;
    return true;
}

QByteArray newCommentsPart(const std::vector<NoteOp> &ops)
{
    QStringList names;
    QHash<QString, int> authorAt;
    QByteArray list;
    for (const NoteOp &op : ops) {
        if (op.remove) continue;
        auto found = authorAt.constFind(op.author);
        if (found == authorAt.constEnd()) {
            names << op.author;
            found = authorAt.insert(op.author, int(names.size()) - 1);
        }
        list += commentXml(QByteArray(), op, found.value());
    }
    QByteArray authors;
    for (const QString &n : names) authors += authorXml(QByteArray(), n);
    return "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<comments xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\"><authors>" + authors + "</authors><commentList>" + list
           + "</commentList></comments>";
}

namespace {

constexpr const char *kShapeType =
    "<v:shapetype id=\"_x0000_t202\" coordsize=\"21600,21600\" o:spt=\"202\" path=\"m,l,21600r21600,l21600,xe\"><v:stroke joinstyle=\"miter\"/><v:path gradientshapeok=\"t\" o:connecttype=\"rect\"/></v:shapetype>";

QByteArray shapeXml(const NoteOp &op, int id)
{
    const int row = int(op.row), col = int(op.col);
    return "<v:shape id=\"_x0000_s" + QByteArray::number(id) + "\" type=\"#_x0000_t202\" style=\"position:absolute;margin-left:80pt;margin-top:2pt;width:108pt;height:59.25pt;z-index:" + QByteArray::number(id % 1000000)
           + ";visibility:" + (op.shown ? "visible" : "hidden") + "\" fillcolor=\"#ffffe1\" o:insetmode=\"auto\"><v:fill color2=\"#ffffe1\"/><v:shadow on=\"t\" color=\"black\" obscured=\"t\"/><v:path o:connecttype=\"none\"/>"
           + "<v:textbox style=\"mso-direction-alt:auto\"><div style=\"text-align:left\"></div></v:textbox><x:ClientData ObjectType=\"Note\"><x:MoveWithCells/><x:SizeWithCells/><x:Anchor>"
           + QByteArray::number(col + 1) + ", 15, " + QByteArray::number(std::max(0, row - 1)) + ", 10, " + QByteArray::number(col + 3) + ", 15, " + QByteArray::number(std::max(0, row - 1) + 3) + ", 4</x:Anchor><x:AutoFill>False</x:AutoFill><x:Row>"
           + QByteArray::number(row) + "</x:Row><x:Column>" + QByteArray::number(col) + "</x:Column>" + (op.shown ? "<x:Visible/>" : "") + "</x:ClientData></v:shape>";
}

const char *kDrawingHead = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<xml xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:o=\"urn:schemas-microsoft-com:office:office\" xmlns:x=\"urn:schemas-microsoft-com:office:excel\">"
                           "<o:shapelayout v:ext=\"edit\"><o:idmap v:ext=\"edit\" data=\"1\"/></o:shapelayout>";

} // namespace

QByteArray newNotesDrawing(const std::vector<NoteOp> &ops)
{
    QByteArray x = kDrawingHead;
    x += kShapeType;
    int id = 1025;
    for (const NoteOp &op : ops)
        if (!op.remove) x += shapeXml(op, id++);
    x += "</xml>";
    return x;
}

bool applyNoteOpsToDrawing(const QByteArray &vml, const std::vector<NoteOp> &ops, QByteArray *out, QString *why)
{
    if (qint64(ops.size()) > kMaxNoteOps) {
        if (why) *why = QCoreApplication::translate("jp::office::sheet::Save", "There are more changes to notes than a sheet's notes are saved with.");
        return false;
    }
    std::map<quint64, const NoteOp *> byCell;
    for (const NoteOp &op : ops) byCell[noteKey(op.row, op.col)] = &op;
    std::map<quint64, bool> done;
    QByteArray result;
    qsizetype at = 0;
    int maxId = 1024;
    for (const DrawingShape &shape : drawingShapes(vml)) {
        const QByteArrayView body = QByteArrayView(vml).sliced(shape.start, shape.end - shape.start);
        // (the shape's id, so that a shape made now has one of its own)
        {
            Tag first;
            if (xmlscan::nextTag(body, 0, &first)) {
                const QByteArrayView id = xmlscan::attribute(body.sliced(first.start, first.end - first.start), "id");
                if (id.startsWith("_x0000_s")) maxId = std::max(maxId, QByteArray(id.sliced(8)).toInt());
            }
        }
        quint32 row = 0, col = 0;
        bool shown = false;
        if (!shapeNote(vml, shape, &row, &col, &shown)) continue;
        const quint64 key = noteKey(row, col);
        const auto op = byCell.find(key);
        if (op == byCell.end()) continue;
        done[key] = true;
        result += vml.mid(at, shape.start - at);
        at = shape.end;
        if (op->second->remove) continue;
        QByteArray piece = vml.mid(shape.start, shape.end - shape.start);
        if (shown != op->second->shown) {
            // kept as it is, but for whether it is shown: the shape's style, and the element that says so
            Tag open;
            xmlscan::nextTag(piece, 0, &open);
            QByteArray tag = piece.left(open.end);
            if (op->second->shown) tag.replace("visibility:hidden", "visibility:visible");
            else tag.replace("visibility:visible", "visibility:hidden");
            QByteArray rest = piece.mid(open.end);
            // the element
            Tag t;
            qsizetype pos = 0, visibleStart = -1, visibleEnd = -1, clientClose = -1;
            QByteArray prefix;
            while (xmlscan::nextTag(rest, pos, &t)) {
                pos = t.end;
                if (t.local == "Visible" && !t.closing) {
                    visibleStart = t.start;
                    visibleEnd = t.end;
                    if (!t.selfClosing) {
                        Tag c;
                        if (xmlscan::nextTag(rest, t.end, &c) && c.closing && c.local == "Visible") visibleEnd = c.end;
                    }
                } else if (t.local == "ClientData" && t.closing) {
                    clientClose = t.start;
                    prefix = QByteArray(xmlscan::prefixOf(t.name));
                }
            }
            if (op->second->shown) {
                if (visibleStart < 0 && clientClose >= 0) rest.insert(clientClose, "<" + prefix + "Visible/>");
            } else if (visibleStart >= 0) {
                rest.remove(visibleStart, visibleEnd - visibleStart);
            }
            piece = tag + rest;
        }
        result += piece;
    }
    // shapes for notes that were put where the drawing has none
    QByteArray added;
    int id = maxId + 1;
    for (const NoteOp &op : ops)
        if (!op.remove && !done.count(noteKey(op.row, op.col))) added += shapeXml(op, id++);
    result += vml.mid(at);
    if (!added.isEmpty()) {
        const qsizetype close = result.lastIndexOf("</xml>");
        if (close < 0) {
            if (why) *why = QCoreApplication::translate("jp::office::sheet::Save", "The drawing of the notes in the file is not in the form this program changes.");
            return false;
        }
        QByteArray piece = added;
        if (!result.contains("_x0000_t202")) piece = QByteArray(kShapeType) + piece;
        result.insert(close, piece);
    }
    *out = result;
    return true;
}

} // namespace jp::office::sheet
