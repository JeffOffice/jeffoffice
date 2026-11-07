#include "sheet/threadsave.h"

#include "sheet/cellref.h"
#include "sheet/notes.h"
#include "sheet/xmlscan.h"

#include <QCoreApplication>

#include <map>
#include <set>

namespace jp::office::sheet {

namespace {

using xmlscan::Tag;

const char *const kThreadNs = "http://schemas.microsoft.com/office/spreadsheetml/2018/threadedcomments";

// The end of the element that begins with `open`; false if the data is cut off first.
bool elementEnd(const QByteArray &xml, const Tag &open, qsizetype *end)
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

QByteArray commentXml(const QByteArray &prefix, const QString &ref, const ThreadComment &c)
{
    // (an attribute the file did not have is not made up: the time and the person are written when the comment has them)
    QByteArray x = "<" + prefix + "threadedComment ref=\"" + ref.toLatin1() + "\"";
    if (!c.time.isEmpty()) x += " dT=\"" + xmlscan::escapeAttribute(c.time) + "\"";
    if (!c.personId.isEmpty()) x += " personId=\"" + xmlscan::escapeAttribute(c.personId) + "\"";
    x += " id=\"" + xmlscan::escapeAttribute(c.id) + "\"";
    if (!c.parentId.isEmpty()) x += " parentId=\"" + xmlscan::escapeAttribute(c.parentId) + "\"";
    if (c.done) x += " done=\"1\"";
    x += "><" + prefix + "text>" + xmlscan::escapeText(c.text) + "</" + prefix + "text>" + c.extra.toUtf8() + "</" + prefix + "threadedComment>";
    return x;
}

QByteArray newOnes(const QByteArray &prefix, const std::vector<NoteOp> &ops)
{
    QByteArray out;
    std::map<quint64, const NoteOp *> byCell;
    for (const NoteOp &op : ops) byCell[noteKey(op.row, op.col)] = &op;
    for (const auto &entry : byCell) {
        const NoteOp &op = *entry.second;
        if (op.remove || !op.thread) continue;
        const QString ref = cellName(op.row, op.col);
        for (const ThreadComment &c : *op.thread) out += commentXml(prefix, ref, c);
    }
    return out;
}

const char kDeclaration[] = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";

QByteArray personXml(const QByteArray &prefix, const ThreadPerson &p)
{
    QByteArray x = "<" + prefix + "person displayName=\"" + xmlscan::escapeAttribute(p.name) + "\" id=\"" + xmlscan::escapeAttribute(p.id) + "\"";
    if (!p.userId.isEmpty()) x += " userId=\"" + xmlscan::escapeAttribute(p.userId) + "\"";
    if (!p.providerId.isEmpty()) x += " providerId=\"" + xmlscan::escapeAttribute(p.providerId) + "\"";
    return x + "/>";
}

} // namespace

bool opsHaveThreads(const std::vector<NoteOp> &ops)
{
    for (const NoteOp &op : ops)
        if (!op.remove && op.thread && !op.thread->empty()) return true;
    return false;
}

std::vector<ThreadPerson> threadPersons(const std::vector<NoteOp> &ops)
{
    std::vector<ThreadPerson> out;
    std::set<QString> seen;
    for (const NoteOp &op : ops) {
        if (op.remove || !op.thread) continue;
        for (const ThreadComment &c : *op.thread) {
            if (c.personId.isEmpty() || !seen.insert(c.personId).second) continue;
            out.push_back({c.personId, c.author, c.userId, c.providerId});
        }
    }
    return out;
}

bool applyThreadOpsToPart(const QByteArray &xml, const std::vector<NoteOp> &ops, QByteArray *out, bool *changed, QString *why)
{
    auto fail = [&](const QString &text) {
        if (why) *why = text;
        return false;
    };
    const QString notInForm = QCoreApplication::translate("jp::office::sheet::Save", "A part of the file that holds comments is not in the form this program changes.");
    if (qint64(ops.size()) > kMaxNoteOps) return fail(QCoreApplication::translate("jp::office::sheet::Save", "There are more changes to notes than a sheet's notes are saved with."));
    std::set<quint64> cells;
    for (const NoteOp &op : ops) cells.insert(noteKey(op.row, op.col));
    Tag root, t;
    bool haveRoot = false;
    qsizetype pos = 0;
    int depth = 0;
    std::vector<std::pair<qsizetype, qsizetype>> dropped;
    while (xmlscan::nextTag(xml, pos, &t)) {
        pos = t.end;
        if (t.closing) {
            --depth;
            continue;
        }
        if (!haveRoot) {
            root = t;
            haveRoot = true;
            if (!t.selfClosing) ++depth;
            continue;
        }
        if (depth == 1 && t.local == "threadedComment") {
            qsizetype e = 0;
            if (!elementEnd(xml, t, &e)) return fail(notInForm);
            quint32 row = 0, col = 0;
            bool found = false;
            const QString ref = xmlscan::unescape(xmlscan::attribute(QByteArrayView(xml).sliced(t.start, t.end - t.start), "ref", &found));
            if (found && parseCellRef(ref, &row, &col) && cells.count(noteKey(row, col))) dropped.emplace_back(t.start, e);
            if (dropped.size() > size_t(kMaxNoteOps) * 64) return fail(notInForm);
            pos = e;
            continue;
        }
        if (!t.selfClosing) ++depth;
    }
    if (!haveRoot || root.local != "ThreadedComments") return fail(notInForm);
    const QByteArray prefix(xmlscan::prefixOf(root.name));
    const QByteArray added = newOnes(prefix, ops);
    if (dropped.empty() && added.isEmpty()) {
        *out = xml;
        if (changed) *changed = false;
        return true;
    }
    QByteArray body;
    qsizetype from = 0;
    QByteArray head;
    // the part up to the root's end tag (or the root, opened, if it closes itself), without the comments that go
    qsizetype closeAt = xml.size();
    if (root.selfClosing) {
        head = xml.left(root.end);
        head.chop(2);
        while (head.endsWith(' ')) head.chop(1);
        head += ">";
        *out = head + added + "</" + QByteArray(root.name) + ">" + xml.mid(root.end);
        if (changed) *changed = true;
        return true;
    }
    qsizetype rootEnd = 0;
    if (!elementEnd(xml, root, &rootEnd)) return fail(notInForm);
    closeAt = xml.lastIndexOf('<', rootEnd - 1);
    for (const auto &d : dropped) {
        body += xml.mid(from, d.first - from);
        from = d.second;
    }
    body += xml.mid(from, closeAt - from);
    *out = body + added + xml.mid(closeAt);
    if (changed) *changed = true;
    return true;
}

QByteArray newThreadsPart(const std::vector<NoteOp> &ops)
{
    return QByteArray(kDeclaration) + "<ThreadedComments xmlns=\"" + kThreadNs + "\" xmlns:x=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\">" + newOnes(QByteArray(), ops) + "</ThreadedComments>";
}

bool applyPersonsToPart(const QByteArray &xml, const std::vector<ThreadPerson> &persons, QByteArray *out, bool *changed, QString *why)
{
    Tag root, t;
    bool haveRoot = false;
    qsizetype pos = 0;
    std::set<QString> have;
    while (xmlscan::nextTag(xml, pos, &t)) {
        pos = t.end;
        if (t.closing) continue;
        if (!haveRoot) {
            root = t;
            haveRoot = true;
            continue;
        }
        if (t.local == "person") {
            bool found = false;
            const QByteArrayView id = xmlscan::attribute(QByteArrayView(xml).sliced(t.start, t.end - t.start), "id", &found);
            if (found) have.insert(xmlscan::unescape(id));
            if (have.size() > 100000) break;
        }
    }
    if (!haveRoot || root.local != "personList") {
        if (why) *why = QCoreApplication::translate("jp::office::sheet::Save", "A part of the file that holds comments is not in the form this program changes.");
        return false;
    }
    const QByteArray prefix(xmlscan::prefixOf(root.name));
    QByteArray added;
    for (const ThreadPerson &p : persons)
        if (!p.id.isEmpty() && !have.count(p.id)) added += personXml(prefix, p);
    if (added.isEmpty()) {
        *out = xml;
        if (changed) *changed = false;
        return true;
    }
    if (changed) *changed = true;
    if (root.selfClosing) {
        QByteArray head = xml.left(root.end);
        head.chop(2);
        while (head.endsWith(' ')) head.chop(1);
        *out = head + ">" + added + "</" + QByteArray(root.name) + ">" + xml.mid(root.end);
        return true;
    }
    const qsizetype closeAt = xml.lastIndexOf("</");
    if (closeAt < root.end) {
        if (why) *why = QCoreApplication::translate("jp::office::sheet::Save", "A part of the file that holds comments is not in the form this program changes.");
        return false;
    }
    *out = xml.left(closeAt) + added + xml.mid(closeAt);
    return true;
}

QByteArray newPersonsPart(const std::vector<ThreadPerson> &persons)
{
    QByteArray body;
    for (const ThreadPerson &p : persons) body += personXml(QByteArray(), p);
    return QByteArray(kDeclaration) + "<personList xmlns=\"" + kThreadNs + "\" xmlns:x=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\">" + body + "</personList>";
}

} // namespace jp::office::sheet
