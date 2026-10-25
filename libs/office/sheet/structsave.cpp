// The parts of a file as rows, columns, and cells are inserted and deleted: see structsave.h.
#include "sheet/structsave.h"
#include "sheet/namerefs.h"

#include "sheet/cellref.h"
#include "sheet/tables.h"
#include "sheet/xmlscan.h"

#include <QCoreApplication>
#include <QHash>
#include <QIODevice>
#include <QRegularExpression>

#include <algorithm>
#include <functional>

namespace jp::office::sheet {

namespace {

using xmlscan::Tag;
using K = StructEdit::Kind;

constexpr qsizetype kChunk = 1 << 20;

bool shiftsRows(K k) { return k == K::InsertRows || k == K::DeleteRows; }
bool shiftsColumns(K k) { return k == K::InsertColumns || k == K::DeleteColumns; }
bool shiftsCells(K k) { return k == K::InsertCellsDown || k == K::DeleteCellsUp || k == K::InsertCellsRight || k == K::DeleteCellsLeft; }
bool isSpace(char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }

// ---- where things are after the edits

// A row of the sheet's file after the edits that insert and delete rows; false if it was deleted.
bool mapRowOnly(const MoveContext &c, int sheet, quint32 *row)
{
    for (const StructEdit &e : c.edits) {
        if (e.sheet != sheet) continue;
        if (e.kind == K::InsertRows) {
            if (*row >= e.first) {
                *row += e.count;
                if (*row >= kMaxRows) return false;
            }
        } else if (e.kind == K::DeleteRows) {
            if (*row >= e.first) {
                if (*row < e.first + e.count) return false;
                *row -= e.count;
            }
        }
    }
    return true;
}

// A cell of the sheet's file after the edits; false if it was deleted, or is one that the workbook writes whole (a cell shifted into the place of another or out of its row or column).
bool mapStreamCell(const MoveContext &c, int sheet, quint32 *row, quint32 *col)
{
    for (const StructEdit &e : c.edits) {
        if (e.sheet != sheet) continue;
        switch (e.kind) {
        case K::InsertRows:
            if (*row >= e.first) {
                *row += e.count;
                if (*row >= kMaxRows) return false;
            }
            break;
        case K::DeleteRows:
            if (*row >= e.first) {
                if (*row < e.first + e.count) return false;
                *row -= e.count;
            }
            break;
        case K::InsertColumns:
            if (*col >= e.first) {
                *col += e.count;
                if (*col >= kMaxColumns) return false;
            }
            break;
        case K::DeleteColumns:
            if (*col >= e.first) {
                if (*col < e.first + e.count) return false;
                *col -= e.count;
            }
            break;
        case K::InsertCellsDown:
        case K::DeleteCellsUp:
            if (*row >= e.row1 && *col >= e.col1 && *col <= e.col2) return false;
            break;
        case K::InsertCellsRight:
        case K::DeleteCellsLeft:
            if (*col >= e.col1 && *row >= e.row1 && *row <= e.row2) return false;
            break;
        default: break;
        }
    }
    return true;
}

// A cell after all the edits (cells shifted too, and a Move): false if it is gone.
bool followCell(const MoveContext &c, int sheet, quint32 *row, quint32 *col)
{
    size_t next = 0;
    auto permutesBefore = [&](size_t edit) {
        // (cells put in another order after `edit` edits were made: what is tied to a cell goes with it)
        for (; next < c.permutes.size() && c.permutes[next].after <= edit; ++next) {
            const CellPermute &p = c.permutes[next];
            if (p.sheet != sheet) continue;
            quint32 &line = p.columns ? *col : *row;
            const quint32 other = p.columns ? *row : *col;
            if (other < p.otherFirst || other > p.otherLast || line < p.first || line - p.first >= p.to.size()) continue;
            line = p.to[line - p.first];
        }
    };
    size_t i = 0;
    for (const StructEdit &e : c.edits) {
        permutesBefore(i++);
        int s = sheet;
        if (adjustCell(e, &s, row, col) == RefFate::Gone) return false;
    }
    permutesBefore(i);
    return true;
}

// What names a sheet that was renamed or deleted, after the edits.
QString followSheetOps(QString f, const MoveContext &c, bool definedName)
{
    for (const SheetOp &op : c.sheetOps) {
        if (op.kind == SheetOp::Kind::Rename) f = renameSheetInFormula(f, op.name, op.newName, nullptr, definedName);
        else f = deleteSheetInFormula(f, op.name, nullptr, definedName);
    }
    return f;
}

// What uses a defined name that was given another name. `ownSheet` is the place of the sheet the formula is on (-1 for a name of the workbook).
QString followNameRenames(QString f, int ownSheet, const MoveContext &c)
{
    for (const NameRename &r : c.nameRenames) {
        if (!f.contains(r.oldName, Qt::CaseInsensitive)) continue;
        const QString scopeName = r.scope >= 0 && r.scope < c.sheetNames.size() ? c.sheetNames[r.scope] : QString();
        f = renameNameInFormula(f, r.oldName, r.newName, [&](const QString &written) {
            if (written.isEmpty()) return r.scope >= 0 ? ownSheet == r.scope : ownSheet < 0 || std::find(r.shadowed.begin(), r.shadowed.end(), ownSheet) == r.shadowed.end();
            return r.scope >= 0 && !scopeName.isEmpty() && written.compare(scopeName, Qt::CaseInsensitive) == 0;
        });
    }
    return f;
}

QString adjustAll(const QString &formula, int ownSheet, const MoveContext &c, bool definedName = false, int nameSheet = -2)
{
    QString f = formula;
    for (const StructEdit &e : c.edits) f = adjustFormula(f, ownSheet, c.sheetNames, e, nullptr, definedName);
    if (f.contains(QLatin1Char('[')))
        for (const ColumnRename &r : c.renames) f = renameColumnInFormula(f, r.table, r.oldName, r.newName, false);
    if (!c.nameRenames.empty()) f = followNameRenames(f, nameSheet == -2 ? ownSheet : nameSheet, c);
    return followSheetOps(f, c, definedName);
}

// The formula of a rule (a conditional format, a validation) after the edits, as the rule's first range (the first of `sqref`) goes through them: it is written for that range's first cell.
QString adjustRuleAll(const QString &formula, int ownSheet, const MoveContext &c, const QString &sqref)
{
    CellRange first;
    const QString firstText = sqref.section(QLatin1Char(' '), 0, 0);
    if (!parseRangeRef(firstText, &first)) first = CellRange();
    QString f = formula;
    for (const StructEdit &e : c.edits) {
        f = adjustRuleFormula(f, ownSheet, c.sheetNames, e, first);
        if (first.valid && !adjustRange(e, ownSheet, &first)) first = CellRange();
    }
    if (f.contains(QLatin1Char('[')))
        for (const ColumnRename &r : c.renames) f = renameColumnInFormula(f, r.table, r.oldName, r.newName, false);
    if (!c.nameRenames.empty()) f = followNameRenames(f, ownSheet, c);
    return followSheetOps(f, c, false);
}

QString adjustSqrefAll(const QString &sqref, int sheet, const MoveContext &c)
{
    QString s = sqref;
    for (const StructEdit &e : c.edits) {
        if (s.isEmpty()) break;
        s = adjustSqref(s, e, sheet);
    }
    return s;
}

// A range ("A1:B2", or a cell) after the edits; false if it is gone. `layoutOnly` follows only the edits of rows and columns (the range of an array or a data table is not cut by the others).
bool adjustRangeAll(const MoveContext &c, int sheet, QString *text, bool layoutOnly, bool dropOneCell = false)
{
    CellRange r;
    if (!parseRangeRef(*text, &r)) return true;
    for (const StructEdit &e : c.edits) {
        if (layoutOnly && !shiftsRows(e.kind) && !shiftsColumns(e.kind)) continue;
        if (!adjustRange(e, sheet, &r)) return false;
        if (dropOneCell && r.cellCount() <= 1) return false;
    }
    *text = r.row1 == r.row2 && r.col1 == r.col2 ? cellName(r.row1, r.col1) : rangeName(r);
    return true;
}

// ---- tags and elements

void setAttribute(QByteArray *tag, const char *name, const QByteArray &value)
{
    qsizetype vs = 0, ve = 0;
    if (xmlscan::attributeValue(*tag, name, &vs, &ve)) {
        tag->replace(vs, ve - vs, value);
        return;
    }
    qsizetype at = tag->size() - 1;   // the ">"
    if (at > 0 && (*tag)[at - 1] == '/') --at;
    tag->insert(at, QByteArray(" ") + name + "=\"" + value + "\"");
}

void removeAttribute(QByteArray *tag, const char *name)
{
    qsizetype vs = 0, ve = 0;
    if (!xmlscan::attributeValue(*tag, name, &vs, &ve)) return;
    qsizetype n = vs - 2;   // before the quote, the "="
    while (n >= 0 && isSpace((*tag)[n])) --n;
    if (n >= 0 && (*tag)[n] == '=') --n;
    while (n >= 0 && isSpace((*tag)[n])) --n;
    qsizetype nameStart = n;
    while (nameStart > 0 && !isSpace((*tag)[nameStart - 1])) --nameStart;
    qsizetype a = nameStart;
    while (a > 0 && isSpace((*tag)[a - 1])) --a;
    tag->remove(a, ve + 1 - a);
}

QString attributeText(const QByteArray &tag, const char *name, bool *found = nullptr)
{
    bool f = false;
    const QByteArrayView v = xmlscan::attribute(tag, name, &f);
    if (found) *found = f;
    return f ? xmlscan::unescape(v) : QString();
}

struct Elem {
    Tag open;
    qsizetype end = 0;        // after the element (its close tag, or its open tag if it closes itself)
    qsizetype closeStart = 0; // the start of its close tag (the end of its open tag if it closes itself)
};

// The element that begins with the tag (nested elements of the same name counted).
bool elementOf(const QByteArray &xml, const Tag &open, Elem *e)
{
    e->open = open;
    if (open.selfClosing) {
        e->end = e->closeStart = open.end;
        return true;
    }
    int depth = 1;
    qsizetype pos = open.end;
    Tag t;
    while (xmlscan::nextTag(xml, pos, &t)) {
        pos = t.end;
        if (t.local != open.local) continue;
        if (t.closing) {
            if (--depth == 0) {
                e->end = t.end;
                e->closeStart = t.start;
                return true;
            }
        } else if (!t.selfClosing) {
            ++depth;
        }
    }
    return false;
}

// The elements directly inside an element.
std::vector<Elem> childrenOf(const QByteArray &xml, const Elem &parent)
{
    std::vector<Elem> out;
    qsizetype pos = parent.open.end;
    Tag t;
    while (pos < parent.closeStart && xmlscan::nextTag(xml, pos, &t) && t.start < parent.closeStart) {
        if (t.closing) {
            pos = t.end;
            continue;
        }
        Elem e;
        if (!elementOf(xml, t, &e)) break;
        out.push_back(e);
        pos = e.end;
    }
    return out;
}

// The text of the elements with a local name inside the bytes, changed by `fn`.
QByteArray rewriteElementText(const QByteArray &xml, QByteArrayView local, const std::function<QString(const QString &)> &fn)
{
    QByteArray out;
    qsizetype copied = 0, pos = 0;
    Tag t;
    while (xmlscan::nextTag(xml, pos, &t)) {
        pos = t.end;
        if (t.closing || t.selfClosing || t.local != local) continue;
        Elem e;
        if (!elementOf(xml, t, &e)) break;
        const QString text = xmlscan::unescape(QByteArrayView(xml).sliced(t.end, e.closeStart - t.end));
        const QString now = fn(text);
        pos = e.end;
        if (now == text) continue;
        out += xml.mid(copied, t.end - copied) + xmlscan::escapeText(now);
        copied = e.closeStart;
    }
    if (copied == 0 && out.isEmpty()) return xml;
    out += xml.mid(copied);
    return out;
}

// The `val` of each value (`cfvo`) of a rule that is a formula (a cell such as $H$1, or a calculation) rewritten with `fn`; the others (a number, a percent, a percentile) are left.
QByteArray rewriteValueFormulas(const QByteArray &xml, const std::function<QString(const QString &)> &fn)
{
    QByteArray out;
    qsizetype copied = 0, pos = 0;
    Tag t;
    while (xmlscan::nextTag(xml, pos, &t)) {
        pos = t.end;
        if (t.closing || t.local != "cfvo") continue;
        QByteArray tag = xml.mid(t.start, t.end - t.start);
        bool hasType = false, hasVal = false;
        if (attributeText(tag, "type", &hasType) != QLatin1String("formula") || !hasType) continue;
        const QString val = attributeText(tag, "val", &hasVal);
        if (!hasVal) continue;
        const QString now = fn(val);
        if (now == val) continue;
        setAttribute(&tag, "val", xmlscan::escapeAttribute(now));
        out += xml.mid(copied, t.start - copied) + tag;
        copied = t.end;
    }
    if (copied == 0 && out.isEmpty()) return xml;
    out += xml.mid(copied);
    return out;
}

// ---- the part after a sheet's cells

QByteArray movedTop(const QByteArray &xml, const Elem &e, const MoveContext &c, int sheet);

// Replaces the bytes of the children of a container with the ones given (a child gone is an empty one), and says how many are left in `kept`.
QByteArray rebuild(const QByteArray &xml, const Elem &parent, const std::vector<Elem> &children, const std::vector<QByteArray> &now, qsizetype *kept)
{
    QByteArray out;
    qsizetype pos = parent.open.end;
    *kept = 0;
    for (size_t i = 0; i < children.size(); ++i) {
        out += xml.mid(pos, children[i].open.start - pos);
        out += now[i];
        if (!now[i].isEmpty()) ++*kept;
        pos = children[i].end;
    }
    out += xml.mid(pos, parent.closeStart - pos);
    return out;
}

// A container element (mergeCells, dataValidations, ...) with the children changed by `fn` (empty for gone), its count attribute set; the whole element is gone if no child is left.
QByteArray movedContainer(const QByteArray &xml, const Elem &e, const std::function<QByteArray(const QByteArray &, const Elem &)> &fn, const char *countAttribute)
{
    if (e.open.selfClosing) return xml.mid(e.open.start, e.end - e.open.start);
    const std::vector<Elem> children = childrenOf(xml, e);
    std::vector<QByteArray> now;
    for (const Elem &c : children) now.push_back(fn(xml, c));
    qsizetype kept = 0;
    const QByteArray inside = rebuild(xml, e, children, now, &kept);
    if (kept == 0 && !children.empty()) return QByteArray();
    QByteArray open = xml.mid(e.open.start, e.open.end - e.open.start);
    qsizetype vs = 0, ve = 0;
    if (countAttribute && !children.empty() && xmlscan::attributeValue(open, countAttribute, &vs, &ve)) setAttribute(&open, countAttribute, QByteArray::number(kept));
    return open + inside + xml.mid(e.closeStart, e.end - e.closeStart);
}

QByteArray movedSortState(const QByteArray &xml, const Elem &e, const MoveContext &c, int sheet)
{
    const QByteArray whole = xml.mid(e.open.start, e.end - e.open.start);
    QByteArray open = whole.left(e.open.end - e.open.start);
    bool found = false;
    QString ref = attributeText(open, "ref", &found);
    if (!found) return whole;
    if (!adjustRangeAll(c, sheet, &ref, false)) return QByteArray();
    setAttribute(&open, "ref", ref.toLatin1());
    if (e.open.selfClosing) return open;
    Tag t0;
    xmlscan::nextTag(whole, 0, &t0);
    Elem self;
    elementOf(whole, t0, &self);
    const std::vector<Elem> kids = childrenOf(whole, self);
    std::vector<QByteArray> kept;
    for (const Elem &k : kids) {
        QByteArray kb = whole.mid(k.open.start, k.end - k.open.start);
        QByteArray kopen = kb.left(k.open.end - k.open.start);
        bool has = false;
        QString r = attributeText(kopen, "ref", &has);
        if (has) {
            if (!adjustRangeAll(c, sheet, &r, false)) kb.clear();
            else {
                setAttribute(&kopen, "ref", r.toLatin1());
                kb = kopen + kb.mid(k.open.end - k.open.start);
            }
        }
        kept.push_back(kb);
    }
    qsizetype count = 0;
    const QByteArray inside = rebuild(whole, self, kids, kept, &count);
    return open + inside + whole.mid(self.closeStart, self.end - self.closeStart);
}

QByteArray movedAutoFilter(const QByteArray &xml, const Elem &e, const MoveContext &c, int sheet)
{
    const QByteArray whole = xml.mid(e.open.start, e.end - e.open.start);
    QByteArray open = whole.left(e.open.end - e.open.start);
    bool found = false;
    const QString ref = attributeText(open, "ref", &found);
    CellRange was;
    if (!found || !parseRangeRef(ref, &was)) return whole;
    QString now = ref;
    if (!adjustRangeAll(c, sheet, &now, false)) return QByteArray();
    CellRange nowRange;
    parseRangeRef(now, &nowRange);
    setAttribute(&open, "ref", now.toLatin1());
    if (e.open.selfClosing) return open;
    // a column of the filter deleted takes its conditions with it, and columns inserted or deleted inside move the ones after
    Tag t0;
    xmlscan::nextTag(whole, 0, &t0);
    Elem self;
    elementOf(whole, t0, &self);
    const std::vector<Elem> kids = childrenOf(whole, self);
    std::vector<QByteArray> kept;
    for (const Elem &k : kids) {
        QByteArray kb = whole.mid(k.open.start, k.end - k.open.start);
        if (k.open.local == "filterColumn") {
            quint32 col = was.col1 + attributeText(kb, "colId").toUInt();
            bool dead = false;
            for (const StructEdit &ed : c.edits) {
                if (ed.sheet != sheet || !shiftsColumns(ed.kind)) continue;
                if (ed.kind == K::InsertColumns) {
                    if (col >= ed.first) col += ed.count;
                } else if (col >= ed.first) {
                    if (col < ed.first + ed.count) {
                        dead = true;
                        break;
                    }
                    col -= ed.count;
                }
            }
            if (dead || col < nowRange.col1) {
                kb.clear();
            } else {
                Tag kt;
                xmlscan::nextTag(kb, 0, &kt);
                QByteArray kopen = kb.left(kt.end);
                setAttribute(&kopen, "colId", QByteArray::number(col - nowRange.col1));
                kb = kopen + kb.mid(kt.end);
            }
        } else if (k.open.local == "sortState") {
            kb = movedSortState(whole, k, c, sheet);
        }
        kept.push_back(kb);
    }
    qsizetype count = 0;
    const QByteArray inside = rebuild(whole, self, kids, kept, &count);
    return open + inside + whole.mid(self.closeStart, self.end - self.closeStart);
}

QByteArray movedTop(const QByteArray &xml, const Elem &e, const MoveContext &c, int sheet)
{
    const QByteArray whole = xml.mid(e.open.start, e.end - e.open.start);
    const QByteArrayView name = e.open.local;
    if (name == "mergeCells") {
        return movedContainer(xml, e, [&](const QByteArray &x, const Elem &m) {
            QByteArray open = x.mid(m.open.start, m.open.end - m.open.start);
            QString ref = attributeText(open, "ref");
            const QString was = ref;
            CellRange r;
            if (!parseRangeRef(ref, &r)) return x.mid(m.open.start, m.end - m.open.start);
            for (const StructEdit &ed : c.edits) {
                if (!adjustRange(ed, sheet, &r) || r.cellCount() <= 1) return QByteArray();
            }
            ref = rangeName(r);
            if (ref == was) return x.mid(m.open.start, m.end - m.open.start);
            setAttribute(&open, "ref", ref.toLatin1());
            return open + x.mid(m.open.end, m.end - m.open.end);
        }, "count");
    }
    if (name == "conditionalFormatting") {
        QByteArray open = whole.left(e.open.end - e.open.start);
        bool found = false;
        const QString sqref = attributeText(open, "sqref", &found);
        QString now = found ? adjustSqrefAll(sqref, sheet, c) : QString();
        if (found && now.isEmpty()) return QByteArray();
        if (found && now != sqref) setAttribute(&open, "sqref", xmlscan::escapeAttribute(now));
        QByteArray rest = whole.mid(e.open.end - e.open.start);
        rest = rewriteElementText(rest, "formula", [&](const QString &f) { return adjustRuleAll(f, sheet, c, sqref); });
        // (the values of a color scale, a data bar, or a set of icons that are formulas follow the cells too)
        rest = rewriteValueFormulas(rest, [&](const QString &f) { return adjustRuleAll(f, sheet, c, sqref); });
        return open + rest;
    }
    if (name == "dataValidations") {
        return movedContainer(xml, e, [&](const QByteArray &x, const Elem &v) {
            QByteArray open = x.mid(v.open.start, v.open.end - v.open.start);
            bool found = false;
            const QString sqref = attributeText(open, "sqref", &found);
            const QString now = found ? adjustSqrefAll(sqref, sheet, c) : QString();
            if (found && now.isEmpty()) return QByteArray();
            if (found && now != sqref) setAttribute(&open, "sqref", xmlscan::escapeAttribute(now));
            QByteArray rest = x.mid(v.open.end, v.end - v.open.end);
            rest = rewriteElementText(rest, "formula1", [&](const QString &f) { return adjustRuleAll(f, sheet, c, sqref); });
            rest = rewriteElementText(rest, "formula2", [&](const QString &f) { return adjustRuleAll(f, sheet, c, sqref); });
            return open + rest;
        }, "count");
    }
    if (name == "hyperlinks") {
        return movedContainer(xml, e, [&](const QByteArray &x, const Elem &h) {
            QByteArray open = x.mid(h.open.start, h.open.end - h.open.start);
            bool found = false;
            const QString ref = attributeText(open, "ref", &found);
            QString now = found ? adjustSqrefAll(ref, sheet, c) : QString();
            if (found && now.isEmpty()) return QByteArray();
            // (a link on one cell goes with the cell when cells were put in another order)
            quint32 linkRow = 0, linkCol = 0;
            if (found && !c.permutes.empty() && parseCellRef(ref, &linkRow, &linkCol)) {
                if (!followCell(c, sheet, &linkRow, &linkCol)) return QByteArray();
                now = cellName(linkRow, linkCol);
            }
            if (found && now != ref) setAttribute(&open, "ref", xmlscan::escapeAttribute(now));
            bool hasLocation = false;
            const QString location = attributeText(open, "location", &hasLocation);
            if (hasLocation) {
                const QString moved = adjustAll(location, sheet, c);
                if (moved != location) setAttribute(&open, "location", xmlscan::escapeAttribute(moved));
            }
            return open + x.mid(h.open.end, h.end - h.open.end);
        }, nullptr);
    }
    if (name == "rowBreaks" || name == "colBreaks") {
        const bool rows = name == "rowBreaks";
        QByteArray result = movedContainer(xml, e, [&](const QByteArray &x, const Elem &b) {
            QByteArray open = x.mid(b.open.start, b.open.end - b.open.start);
            bool found = false;
            quint32 id = attributeText(open, "id", &found).toUInt();
            if (!found) return x.mid(b.open.start, b.end - b.open.start);
            // (a break is before the row or column of that number counted from 1, which is the one of that number from 0)
            bool alive = true;
            for (const StructEdit &ed : c.edits) {
                if (ed.sheet != sheet) continue;
                if (rows && ed.kind == K::InsertRows) {
                    if (id >= ed.first) id += ed.count;
                } else if (rows && ed.kind == K::DeleteRows) {
                    if (id >= ed.first + ed.count) id -= ed.count;
                    else if (id > ed.first) alive = false;
                } else if (!rows && ed.kind == K::InsertColumns) {
                    if (id >= ed.first) id += ed.count;
                } else if (!rows && ed.kind == K::DeleteColumns) {
                    if (id >= ed.first + ed.count) id -= ed.count;
                    else if (id > ed.first) alive = false;
                }
            }
            if (!alive) return QByteArray();
            setAttribute(&open, "id", QByteArray::number(id));
            return open + x.mid(b.open.end, b.end - b.open.end);
        }, "count");
        if (!result.isEmpty()) {
            Tag t;
            if (xmlscan::nextTag(result, 0, &t)) {
                QByteArray open = result.left(t.end);
                bool found = false;
                const QString count = attributeText(open, "count", &found), manual = attributeText(open, "manualBreakCount", &found);
                if (found && manual.toInt() > count.toInt()) {
                    setAttribute(&open, "manualBreakCount", count.toLatin1());
                    result = open + result.mid(t.end);
                }
            }
        }
        return result;
    }
    if (name == "autoFilter") return movedAutoFilter(xml, e, c, sheet);
    if (name == "sortState") return movedSortState(xml, e, c, sheet);
    if (name == "ignoredErrors" || name == "protectedRanges") {
        return movedContainer(xml, e, [&](const QByteArray &x, const Elem &g) {
            QByteArray open = x.mid(g.open.start, g.open.end - g.open.start);
            bool found = false;
            const QString sqref = attributeText(open, "sqref", &found);
            const QString now = found ? adjustSqrefAll(sqref, sheet, c) : QString();
            if (found && now.isEmpty()) return QByteArray();
            if (found && now != sqref) setAttribute(&open, "sqref", xmlscan::escapeAttribute(now));
            return open + x.mid(g.open.end, g.end - g.open.end);
        }, nullptr);
    }
    if (name == "extLst") {
        QByteArray out = rewriteElementText(whole, "sqref", [&](const QString &s) { return adjustSqrefAll(s, sheet, c); });
        out = rewriteElementText(out, "f", [&](const QString &f) { return adjustAll(f, sheet, c); });
        return out;
    }
    return whole;
}

// What follows a sheet's cells (from after "</sheetData>" to the end), with the cells they name followed.
QByteArray movedTail(const QByteArray &tail, const MoveContext &c, int sheet, bool *changed)
{
    QByteArray out;
    qsizetype copied = 0, pos = 0;
    Tag t;
    while (xmlscan::nextTag(tail, pos, &t)) {
        if (t.closing) break;
        Elem e;
        if (!elementOf(tail, t, &e)) break;
        pos = e.end;
        const QByteArray was = tail.mid(e.open.start, e.end - e.open.start);
        const QByteArray now = movedTop(tail, e, c, sheet);
        if (now == was) continue;
        out += tail.mid(copied, e.open.start - copied) + now;
        copied = e.end;
        *changed = true;
    }
    if (copied == 0 && out.isEmpty()) return tail;
    out += tail.mid(copied);
    return out;
}

// ---- the sheet

struct SharedMaster {
    quint32 row = 0, col = 0;
    QString text;
};

class SheetMover {
public:
    SheetMover(QIODevice *in, PartSink *out, const MoveContext &ctx, int sheet, const std::atomic<bool> *cancel)
        : m_in(in), m_out(out), m_ctx(ctx), m_sheet(sheet), m_cancel(cancel)
    {
        for (const StructEdit &e : ctx.edits) {
            if (e.sheet != sheet) continue;
            if (shiftsColumns(e.kind)) m_columnEdits = true;
            if (shiftsRows(e.kind) || shiftsColumns(e.kind) || shiftsCells(e.kind)) m_positional = true;
            if (shiftsCells(e.kind) || shiftsColumns(e.kind)) m_cellLevel = true;
        }
    }

    bool run(QString *error)
    {
        m_error = error;
        if (!head()) return false;
        if (m_selfClosed) return tail(0);
        const bool ok = m_positional ? rows() : copyRows();
        if (!ok) return false;
        return tail(m_tailAt);
    }
    bool changed() const { return m_changed; }

private:
    bool fail(const QString &why)
    {
        if (m_error && m_error->isEmpty()) *m_error = why;
        return false;
    }

    bool fill()
    {
        if (m_eof) return false;
        const qsizetype old = m_buf.size();
        m_buf.resize(old + kChunk);
        const qint64 n = m_in->read(m_buf.data() + old, kChunk);
        if (n <= 0) {
            m_buf.resize(old);
            m_eof = true;
            if (n < 0) m_failed = true;
            return false;
        }
        m_buf.resize(old + qsizetype(n));
        return true;
    }

    void emit(const char *data, qsizetype size)
    {
        if (m_out && size > 0) m_out->write(data, size);
    }
    void emit(const QByteArray &bytes) { emit(bytes.constData(), bytes.size()); }

    bool nextTag(qsizetype from, Tag *tag)
    {
        for (;;) {
            if (xmlscan::nextTag(m_buf, from, tag)) return true;
            if (!fill()) return false;
        }
    }

    bool cancelled() const { return m_cancel && m_cancel->load(std::memory_order_relaxed); }

    QString damaged() const { return QCoreApplication::translate("jp::office::sheet::Save", "The sheet's part is damaged."); }
    QString notForm() const { return QCoreApplication::translate("jp::office::sheet::Save", "The sheet's cells are not in the form this program changes."); }

    // ---- the part before the cells
    bool head()
    {
        qsizetype pos = 0;
        Tag t;
        for (;;) {
            if (!nextTag(pos, &t)) return fail(m_failed ? damaged() : QCoreApplication::translate("jp::office::sheet::Save", "The sheet's file has no place for cells in it, so it can't be saved."));
            pos = t.end;
            if (t.closing) continue;
            if (t.local == "sheetData") break;
        }
        m_prefix = QByteArray(xmlscan::prefixOf(t.name));
        QByteArray before = m_buf.left(t.start);
        if (m_positional) before = movedHead(before);
        emit(before);
        emit(m_buf.constData() + t.start, t.end - t.start);
        m_selfClosed = t.selfClosing;
        m_buf.remove(0, t.end);
        return true;
    }

    QByteArray movedHead(const QByteArray &before)
    {
        QByteArray out;
        qsizetype copied = 0, pos = 0;
        Tag t;
        while (xmlscan::nextTag(before, pos, &t)) {
            pos = t.end;
            if (t.closing) continue;
            if (t.local == "dimension") {
                QByteArray tag = before.mid(t.start, t.end - t.start);
                bool found = false;
                QString ref = attributeText(tag, "ref", &found);
                if (found && adjustRangeAll(m_ctx, m_sheet, &ref, true)) {
                    setAttribute(&tag, "ref", ref.toLatin1());
                    out += before.mid(copied, t.start - copied) + tag;
                    copied = t.end;
                }
            } else if (t.local == "cols" && m_columnEdits && !t.selfClosing) {
                Elem e;
                if (!elementOf(before, t, &e)) continue;
                QByteArray cols = before.mid(e.open.start, e.open.end - e.open.start);
                const std::vector<Elem> kids = childrenOf(before, e);
                std::vector<QByteArray> now;
                for (const Elem &k : kids) {
                    QByteArray tag = before.mid(k.open.start, k.open.end - k.open.start);
                    quint32 first = attributeText(tag, "min").toUInt(), last = attributeText(tag, "max").toUInt();
                    if (first == 0 || last < first) {
                        now.push_back(before.mid(k.open.start, k.end - k.open.start));
                        continue;
                    }
                    --first;
                    --last;
                    bool alive = true;
                    for (const StructEdit &ed : m_ctx.edits)
                        if (ed.sheet == m_sheet && alive) alive = adjustColumnRun(ed, &first, &last);
                    if (!alive) {
                        now.push_back(QByteArray());
                        continue;
                    }
                    setAttribute(&tag, "min", QByteArray::number(first + 1));
                    setAttribute(&tag, "max", QByteArray::number(last + 1));
                    now.push_back(tag + before.mid(k.open.end, k.end - k.open.end));
                }
                qsizetype kept = 0;
                const QByteArray inside = rebuild(before, e, kids, now, &kept);
                out += before.mid(copied, e.open.start - copied);
                if (kept > 0) out += cols + inside + before.mid(e.closeStart, e.end - e.closeStart);
                copied = e.end;
                pos = e.end;
            }
        }
        if (copied == 0 && out.isEmpty()) return before;
        out += before.mid(copied);
        return out;
    }

    // ---- the cells, as they are (a sheet the edits do not move cells on)
    bool copyRows()
    {
        const QByteArray closeTag = "</" + m_prefix + "sheetData>";
        for (;;) {
            if (cancelled()) return fail(QCoreApplication::translate("jp::office::sheet::Save", "The save was stopped."));
            const qsizetype i = m_buf.indexOf(closeTag);
            if (i >= 0) {
                emit(m_buf.constData(), i + closeTag.size());
                m_buf.remove(0, i + closeTag.size());
                m_tailAt = 0;
                return true;
            }
            // (all but the end, which may hold the first part of the tag, is done with)
            const qsizetype keep = closeTag.size() - 1;
            if (m_buf.size() > keep) {
                emit(m_buf.constData(), m_buf.size() - keep);
                m_buf.remove(0, m_buf.size() - keep);
            }
            if (!fill()) return fail(m_failed ? damaged() : QCoreApplication::translate("jp::office::sheet::Save", "The sheet's cells are cut short."));
        }
    }

    // ---- the cells, renumbered
    bool closeOfElement(const QByteArray &closeTag, QByteArrayView local, qsizetype from, qsizetype *at)
    {
        for (;;) {
            const qsizetype i = m_buf.indexOf(closeTag, from);
            if (i >= 0) {
                *at = i;
                break;
            }
            from = std::max(from, qsizetype(m_buf.size()) - closeTag.size() + 1);
            if (!fill()) return false;
        }
        if (QByteArrayView(m_buf).mid(from, *at - from).indexOf("<!") < 0) return true;
        qsizetype pos = from;
        Tag t;
        for (;;) {
            if (!nextTag(pos, &t)) return false;
            pos = t.end;
            if (t.closing && t.local == local) {
                *at = t.start;
                return true;
            }
        }
    }

    bool rows()
    {
        quint32 lastRow = 0;   // the row number (from 1) of the last row element
        for (;;) {
            if (cancelled()) return fail(QCoreApplication::translate("jp::office::sheet::Save", "The save was stopped."));
            Tag t;
            if (!nextTag(0, &t)) return fail(m_failed ? damaged() : QCoreApplication::translate("jp::office::sheet::Save", "The sheet's cells are cut short."));
            if (t.closing) {
                if (t.local != "sheetData") return fail(notForm());
                emit(m_buf.constData(), t.end);
                m_buf.remove(0, t.end);
                m_tailAt = 0;
                return true;
            }
            if (t.local != "row") return fail(notForm());
            const QByteArray openTag = m_buf.mid(t.start, t.end - t.start);
            bool haveR = false;
            const QByteArrayView rAttr = xmlscan::attribute(openTag, "r", &haveR);
            quint32 rowIdx = lastRow;
            if (haveR) {
                bool ok = false;
                const qulonglong n = QByteArray(rAttr).toULongLong(&ok);
                if (ok && n >= 1 && n <= kMaxRows) rowIdx = quint32(n - 1);
            }
            lastRow = rowIdx + 1;
            qsizetype rowEnd = t.end;
            if (!t.selfClosing) {
                const QByteArray closeTag = "</" + m_prefix + "row>";
                qsizetype close = 0;
                if (!closeOfElement(closeTag, "row", t.end, &close)) return fail(m_failed ? damaged() : QCoreApplication::translate("jp::office::sheet::Save", "The sheet's cells are cut short."));
                Tag closing;
                if (!nextTag(close, &closing) || closing.start != close) return fail(QCoreApplication::translate("jp::office::sheet::Save", "The sheet's cells are cut short."));
                rowEnd = closing.end;
            }
            emit(m_buf.constData(), t.start);   // (the white space between rows)
            quint32 newRow = rowIdx;
            const bool alive = mapRowOnly(m_ctx, m_sheet, &newRow);
            // (a row that is gone may hold the formula that cells after it share)
            if (alive || m_hasShared) {
                const QByteArray row = m_buf.mid(t.start, rowEnd - t.start);
                if (alive || row.indexOf("shared") >= 0) {
                    QByteArray rewritten;
                    if (!rewriteRow(row, rowIdx, newRow, alive, &rewritten)) return false;
                    emit(rewritten);
                }
            }
            m_buf.remove(0, rowEnd);
        }
    }

    bool rewriteRow(const QByteArray &row, quint32 origRow, quint32 newRow, bool alive, QByteArray *out)
    {
        Tag open;
        if (!xmlscan::nextTag(row, 0, &open)) return fail(QCoreApplication::translate("jp::office::sheet::Save", "The sheet's cells are cut short."));
        QByteArray openTag = row.left(open.end);
        setAttribute(&openTag, "r", QByteArray::number(qulonglong(newRow) + 1));
        if (m_columnEdits) removeAttribute(&openTag, "spans");
        if (open.selfClosing) {
            *out = openTag;
            return true;
        }
        QByteArray body, rest;
        qsizetype pos = open.end;
        qint64 prevCol = -1;
        Tag t;
        for (;;) {
            if (!xmlscan::nextTag(row, pos, &t)) {
                rest = "</" + m_prefix + "row>";
                break;
            }
            if (t.closing || t.local != "c") {
                rest = row.mid(t.start);
                break;
            }
            qsizetype cellEnd = t.end;
            if (!t.selfClosing) {
                const QByteArray closeTag = "</" + m_prefix + "c>";
                qsizetype close = row.indexOf(closeTag, t.end);
                if (close >= 0 && QByteArrayView(row).mid(t.end, close - t.end).indexOf("<!") >= 0) {
                    close = -1;
                    qsizetype at = t.end;
                    Tag inner;
                    while (xmlscan::nextTag(row, at, &inner)) {
                        at = inner.end;
                        if (inner.closing && inner.local == "c") {
                            close = inner.start;
                            break;
                        }
                    }
                }
                if (close < 0) return fail(QCoreApplication::translate("jp::office::sheet::Save", "The sheet's cells are cut short."));
                cellEnd = close + closeTag.size();
            }
            const QByteArray cell = row.mid(t.start, cellEnd - t.start);
            pos = cellEnd;
            quint32 col = quint32(prevCol + 1);
            bool haveR = false;
            const QByteArray cellOpen = cell.left(t.end - t.start);   // (kept: the view of an attribute is of these bytes)
            const QByteArrayView r = xmlscan::attribute(cellOpen, "r", &haveR);
            if (haveR) {
                quint32 rr = 0, cc = 0;
                if (xmlscan::parseAddress(r, &rr, &cc)) col = cc;
            }
            prevCol = col;
            if (!selfClosingCell(t) && cell.indexOf("shared") >= 0) noteMaster(cell, t.end - t.start, origRow, col);
            if (!alive) continue;
            quint32 newCol = col, checkRow = origRow;
            if (m_cellLevel && !mapStreamCell(m_ctx, m_sheet, &checkRow, &newCol)) continue;
            body += movedCell(cell, t.end - t.start, t.selfClosing, origRow, col, newRow, newCol);
        }
        *out = alive ? openTag + body + rest : QByteArray();
        return true;
    }

    static bool selfClosingCell(const Tag &t) { return t.selfClosing; }

    // The first cell of a shared formula holds its text: the cells that share it are written out from it.
    void noteMaster(const QByteArray &cell, qsizetype openEnd, quint32 origRow, quint32 origCol)
    {
        qsizetype fs = 0, fe = 0;
        if (!findChild(cell, openEnd, "f", &fs, &fe)) return;
        const QByteArray f = cell.mid(fs, fe - fs);
        Tag fo;
        if (!xmlscan::nextTag(f, 0, &fo) || fo.selfClosing) return;
        const QByteArray fTag = f.left(fo.end);
        bool found = false;
        if (xmlscan::attribute(fTag, "t", &found) != "shared" || !found) return;
        const QByteArray si = QByteArray(xmlscan::attribute(fTag, "si", &found));
        const qsizetype close = f.lastIndexOf('<');
        if (!found || close <= fo.end) return;
        const QString text = xmlscan::unescape(QByteArrayView(f).sliced(fo.end, close - fo.end));
        if (!text.isEmpty()) m_masters.insert(si, SharedMaster{origRow, origCol, text});
    }

    QByteArray movedCell(const QByteArray &cell, qsizetype openEnd, bool selfClosing, quint32 origRow, quint32 origCol, quint32 newRow, quint32 newCol)
    {
        QByteArray open = cell.left(openEnd);
        QByteArray rest = cell.mid(openEnd);
        if (!selfClosing && rest.indexOf("f") >= 0) rest = movedFormula(cell, openEnd, origRow, origCol);
        setAttribute(&open, "r", (columnName(newCol) + QString::number(qulonglong(newRow) + 1)).toLatin1());
        return open + rest;
    }

    // The part of a cell after its open tag, with its formula written out if it shares one and its ranges followed if it is an array's or a data table's.
    QByteArray movedFormula(const QByteArray &cell, qsizetype openEnd, quint32 origRow, quint32 origCol)
    {
        qsizetype fs = 0, fe = 0;
        if (!findChild(cell, openEnd, "f", &fs, &fe)) return cell.mid(openEnd);
        const QByteArray f = cell.mid(fs, fe - fs);
        Tag fo;
        if (!xmlscan::nextTag(f, 0, &fo)) return cell.mid(openEnd);
        QByteArray fTag = f.left(fo.end);
        bool found = false;
        const QByteArrayView typeView = xmlscan::attribute(fTag, "t", &found);
        const QByteArray type = found ? QByteArray(typeView) : QByteArray();
        QByteArray replacement;
        if (type == "shared") {
            const QByteArray si = QByteArray(xmlscan::attribute(fTag, "si", &found));
            QString text;
            if (!fo.selfClosing) {
                const qsizetype close = f.lastIndexOf('<');
                if (close > fo.end) text = xmlscan::unescape(QByteArrayView(f).sliced(fo.end, close - fo.end));
            }
            if (text.isEmpty()) {
                const auto master = m_masters.constFind(si);
                if (master == m_masters.constEnd()) return cell.mid(openEnd);
                text = shiftFormula(master->text, int(origRow) - int(master->row), int(origCol) - int(master->col));
            }
            QByteArray tag = "<" + m_prefix + "f";
            bool ca = false;
            const QByteArrayView caValue = xmlscan::attribute(fTag, "ca", &ca);
            if (ca) tag += " ca=\"" + QByteArray(caValue) + "\"";
            replacement = tag + ">" + xmlscan::escapeText(text) + "</" + m_prefix + "f>";
        } else if (type == "array" || type == "dataTable") {
            QByteArray tag = fTag;
            for (const char *name : {"ref", "r1", "r2"}) {
                bool has = false;
                QString v = attributeText(tag, name, &has);
                if (!has) continue;
                if (adjustRangeAll(m_ctx, m_sheet, &v, true)) setAttribute(&tag, name, v.toLatin1());
            }
            replacement = tag + f.mid(fo.end);
        } else {
            return cell.mid(openEnd);
        }
        return cell.mid(openEnd, fs - openEnd) + replacement + cell.mid(fe);
    }

    static bool findChild(const QByteArray &cell, qsizetype from, QByteArrayView local, qsizetype *start, qsizetype *end)
    {
        Tag t;
        int depth = 0;
        qsizetype pos = from;
        while (xmlscan::nextTag(cell, pos, &t)) {
            pos = t.end;
            if (t.closing) {
                if (--depth < 0) return false;
                continue;
            }
            if (depth == 0 && t.local == local) {
                *start = t.start;
                if (t.selfClosing) {
                    *end = t.end;
                    return true;
                }
                Tag close;
                qsizetype p = t.end;
                while (xmlscan::nextTag(cell, p, &close)) {
                    p = close.end;
                    if (close.closing && close.local == local) {
                        *end = close.end;
                        return true;
                    }
                }
                return false;
            }
            if (!t.selfClosing) ++depth;
        }
        return false;
    }

    // ---- what follows the cells
    bool tail(qsizetype from)
    {
        while (fill()) {
            if (cancelled()) return fail(QCoreApplication::translate("jp::office::sheet::Save", "The save was stopped."));
        }
        if (m_failed) return fail(damaged());
        const QByteArray rest = m_buf.mid(from);
        const QByteArray now = movedTail(rest, m_ctx, m_sheet, &m_changed);
        emit(now);
        m_buf.clear();
        if (m_positional) m_changed = true;
        return true;
    }

    QIODevice *m_in;
    PartSink *m_out;
    const MoveContext &m_ctx;
    int m_sheet;
    const std::atomic<bool> *m_cancel;
    QString *m_error = nullptr;
    bool m_columnEdits = false, m_positional = false, m_cellLevel = false, m_hasShared = true;
    bool m_eof = false, m_failed = false, m_selfClosed = false, m_changed = false;
    QByteArray m_buf, m_prefix;
    qsizetype m_tailAt = 0;
    QHash<QByteArray, SharedMaster> m_masters;
};

} // namespace

bool movesCellsOf(const MoveContext &ctx, int sheet)
{
    for (const StructEdit &e : ctx.edits)
        if (e.sheet == sheet && (shiftsRows(e.kind) || shiftsColumns(e.kind) || shiftsCells(e.kind))) return true;
    return false;
}

bool moveSheetPart(QIODevice *in, PartSink *out, const MoveContext &ctx, int sheet, bool *changed, QString *error, const std::atomic<bool> *cancel)
{
    if (error) error->clear();
    SheetMover mover(in, out, ctx, sheet, cancel);
    QString why;
    const bool ok = mover.run(&why);
    if (changed) *changed = mover.changed();
    if (!ok && error) *error = why.isEmpty() ? QCoreApplication::translate("jp::office::sheet::Save", "The sheet could not be saved.") : why;
    return ok;
}

// ---- the other parts

bool moveWorkbookPart(const QByteArray &xml, const MoveContext &ctx, QByteArray *out, bool *changed)
{
    QByteArray result;
    qsizetype copied = 0, pos = 0;
    Tag t;
    bool any = false;
    while (xmlscan::nextTag(xml, pos, &t)) {
        pos = t.end;
        if (t.closing || t.selfClosing || t.local != "definedName") continue;
        Elem e;
        if (!elementOf(xml, t, &e)) break;
        pos = e.end;
        const QByteArray open = xml.mid(t.start, t.end - t.start);
        bool hasScope = false;
        const int scope = attributeText(open, "localSheetId", &hasScope).toInt();
        const QString text = xmlscan::unescape(QByteArrayView(xml).sliced(t.end, e.closeStart - t.end));
        const QString now = adjustAll(text, hasScope ? scope : 0, ctx, true, hasScope ? scope : -1);
        if (now == text) continue;
        result += xml.mid(copied, t.end - copied) + xmlscan::escapeText(now);
        copied = e.closeStart;
        any = true;
    }
    if (!any) {
        *out = xml;
        *changed = false;
        return true;
    }
    result += xml.mid(copied);
    *out = result;
    *changed = true;
    return true;
}

bool moveChartPart(const QByteArray &xml, const MoveContext &ctx, QByteArray *out, bool *changed, int ownSheet)
{
    *out = rewriteElementText(xml, "f", [&](const QString &f) { return adjustAll(f, ownSheet, ctx, true); });
    *changed = *out != xml;
    return true;
}

bool movePivotCachePart(const QByteArray &xml, const MoveContext &ctx, QByteArray *out, bool *changed)
{
    // <worksheetSource ref="A1:D100" sheet="Data"/> names the cells a PivotTable is made from
    QByteArray result;
    qsizetype copied = 0, pos = 0;
    Tag t;
    while (xmlscan::nextTag(xml, pos, &t)) {
        pos = t.end;
        if (t.closing || t.local != "worksheetSource") continue;
        QByteArray tag = xml.mid(t.start, t.end - t.start);
        bool hasRef = false, hasSheet = false;
        QString ref = attributeText(tag, "ref", &hasRef);
        const QString sheetName = attributeText(tag, "sheet", &hasSheet);
        if (!hasRef || !hasSheet) continue;
        int sheet = -1;
        for (int i = 0; i < ctx.sheetNames.size(); ++i)
            if (ctx.sheetNames[i].compare(sheetName, Qt::CaseInsensitive) == 0) sheet = i;
        if (sheet < 0) continue;
        const QString was = ref;
        if (!adjustRangeAll(ctx, sheet, &ref, false)) ref = was;
        // (a sheet renamed: the cache names it as it is called now. A sheet deleted: the cache keeps its records and the name of the sheet it was made from, as the programs people know leave
        // it, so that a PivotTable still shows what it did and can't be refreshed until the data is put back; a name given later to another sheet is not the source's)
        QString nowSheet = sheetName;
        for (const SheetOp &op : ctx.sheetOps) {
            if (op.name.compare(nowSheet, Qt::CaseInsensitive) != 0) continue;
            if (op.kind == SheetOp::Kind::Delete) break;
            nowSheet = op.newName;
        }
        if (ref == was && nowSheet == sheetName) continue;
        setAttribute(&tag, "ref", ref.toLatin1());
        if (nowSheet != sheetName) setAttribute(&tag, "sheet", xmlscan::escapeAttribute(nowSheet));
        result += xml.mid(copied, t.start - copied) + tag;
        copied = t.end;
    }
    if (copied == 0 && result.isEmpty()) {
        *out = xml;
        *changed = false;
        return true;
    }
    result += xml.mid(copied);
    *out = result;
    *changed = true;
    return true;
}

namespace {

// A point of a drawing (a row or column counted from 0) after the edits that insert and delete rows (`rows`) or columns.
quint32 movedPoint(const MoveContext &c, int sheet, bool rows, quint32 v)
{
    for (const StructEdit &e : c.edits) {
        if (e.sheet != sheet) continue;
        if (rows && e.kind == K::InsertRows) {
            if (v >= e.first) v += e.count;
        } else if (rows && e.kind == K::DeleteRows) {
            if (v >= e.first + e.count) v -= e.count;
            else if (v > e.first) v = e.first;
        } else if (!rows && e.kind == K::InsertColumns) {
            if (v >= e.first) v += e.count;
        } else if (!rows && e.kind == K::DeleteColumns) {
            if (v >= e.first + e.count) v -= e.count;
            else if (v > e.first) v = e.first;
        }
    }
    return v;
}

} // namespace

bool moveDrawingPart(const QByteArray &xml, const MoveContext &ctx, int sheet, QByteArray *out, bool *changed)
{
    // Each anchor: <xdr:twoCellAnchor editAs="oneCell"><xdr:from><xdr:col>1</xdr:col><xdr:colOff/><xdr:row>2</xdr:row><xdr:rowOff/></xdr:from><xdr:to>...</xdr:to>...</xdr:twoCellAnchor>.
    // It moves and sizes with the cells (editAs "twoCell", the usual), moves and keeps its size ("oneCell": both ends move as far as the first), or stays where it is ("absolute").
    static const QRegularExpression anchorRe(QStringLiteral("<(?:\\w+:)?(?:twoCellAnchor|oneCellAnchor)\\b.*?</(?:\\w+:)?(?:twoCellAnchor|oneCellAnchor)>"), QRegularExpression::DotMatchesEverythingOption);
    static const QRegularExpression openRe(QStringLiteral("^<(?:\\w+:)?(twoCellAnchor|oneCellAnchor)\\b[^>]*>")), editAsRe(QStringLiteral("\\seditAs=\"(\\w+)\""));
    static const QRegularExpression fromRe(QStringLiteral("<(?:\\w+:)?from>.*?</(?:\\w+:)?from>"), QRegularExpression::DotMatchesEverythingOption);
    static const QRegularExpression toRe(QStringLiteral("<(?:\\w+:)?to>.*?</(?:\\w+:)?to>"), QRegularExpression::DotMatchesEverythingOption);
    static const QRegularExpression colRe(QStringLiteral("(<(?:\\w+:)?col>)(\\d+)(</(?:\\w+:)?col>)")), rowRe(QStringLiteral("(<(?:\\w+:)?row>)(\\d+)(</(?:\\w+:)?row>)"));
    const QString text = QString::fromUtf8(xml);
    QString result;
    int at = 0;
    // a block's column and row numbers changed by `change(rows, value)`
    auto remap = [&](const QString &block, const std::function<quint32(bool, quint32)> &change) {
        QString out = block;
        for (int pass = 0; pass < 2; ++pass) {
            const QRegularExpression &re = pass == 0 ? colRe : rowRe;
            QString next;
            int pos = 0;
            QRegularExpressionMatchIterator it = re.globalMatch(out);
            while (it.hasNext()) {
                const QRegularExpressionMatch m = it.next();
                next += out.mid(pos, m.capturedStart() - pos) + m.captured(1) + QString::number(change(pass == 1, m.captured(2).toUInt())) + m.captured(3);
                pos = m.capturedEnd();
            }
            out = next + out.mid(pos);
        }
        return out;
    };
    auto numberOf = [&](const QString &block, bool rows) {
        const QRegularExpressionMatch m = (rows ? rowRe : colRe).match(block);
        return m.hasMatch() ? qint64(m.captured(2).toUInt()) : qint64(-1);
    };
    QRegularExpressionMatchIterator it = anchorRe.globalMatch(text);
    while (it.hasNext()) {
        const QRegularExpressionMatch m = it.next();
        QString anchor = m.captured();
        const QRegularExpressionMatch open = openRe.match(anchor);
        const QRegularExpressionMatch editAs = open.hasMatch() ? editAsRe.match(open.captured()) : QRegularExpressionMatch();
        const QString how = editAs.hasMatch() ? editAs.captured(1) : QStringLiteral("twoCell");
        const bool two = open.hasMatch() && open.captured(1) == QLatin1String("twoCellAnchor");
        if (two && how == QLatin1String("absolute")) continue;
        const QRegularExpressionMatch from = fromRe.match(anchor), to = toRe.match(anchor);
        if (!from.hasMatch()) continue;
        QString now = anchor;
        if (two && how == QLatin1String("oneCell") && to.hasMatch()) {
            const qint64 row0 = numberOf(from.captured(), true), col0 = numberOf(from.captured(), false);
            const QString newFrom = remap(from.captured(), [&](bool rows, quint32 v) { return movedPoint(ctx, sheet, rows, v); });
            const qint64 dRow = numberOf(newFrom, true) - row0, dCol = numberOf(newFrom, false) - col0;
            const QString newTo = remap(to.captured(), [&](bool rows, quint32 v) { return quint32(std::max<qint64>(0, qint64(v) + (rows ? dRow : dCol))); });
            // (the later block first, so that the places of the earlier one hold)
            if (to.capturedStart() > from.capturedStart()) {
                now.replace(to.capturedStart(), to.capturedLength(), newTo);
                now.replace(from.capturedStart(), from.capturedLength(), newFrom);
            } else {
                now.replace(from.capturedStart(), from.capturedLength(), newFrom);
                now.replace(to.capturedStart(), to.capturedLength(), newTo);
            }
        } else {
            QString mapped = anchor;
            const auto change = [&](bool rows, quint32 v) { return movedPoint(ctx, sheet, rows, v); };
            // (the `from` and the `to` of the anchor; the later block first)
            QVector<QRegularExpressionMatch> blocks;
            blocks.append(from);
            if (to.hasMatch()) blocks.append(to);
            std::sort(blocks.begin(), blocks.end(), [](const QRegularExpressionMatch &a, const QRegularExpressionMatch &b) { return a.capturedStart() > b.capturedStart(); });
            for (const QRegularExpressionMatch &b : blocks) mapped.replace(b.capturedStart(), b.capturedLength(), remap(b.captured(), change));
            now = mapped;
        }
        if (now == anchor) continue;
        result += text.mid(at, m.capturedStart() - at) + now;
        at = m.capturedEnd();
    }
    if (at == 0 && result.isEmpty()) {
        *out = xml;
        *changed = false;
        return true;
    }
    result += text.mid(at);
    *out = result.toUtf8();
    *changed = *out != xml;
    return true;
}

namespace {

// The elements (of that local name) whose `ref` is a cell of the sheet: the cell goes where the edits take it, and an element on a deleted cell goes.
bool moveElementsByRef(const QByteArray &xml, const MoveContext &ctx, int sheet, QByteArrayView element, QByteArray *out, bool *changed)
{
    QByteArray result;
    qsizetype copied = 0, pos = 0;
    Tag t;
    bool any = false;
    // the comments that are on deleted cells go
    while (xmlscan::nextTag(xml, pos, &t)) {
        pos = t.end;
        if (t.closing || t.local != element) continue;
        Elem e;
        if (!elementOf(xml, t, &e)) break;
        pos = e.end;
        QByteArray tag = xml.mid(t.start, t.end - t.start);
        bool has = false;
        const QString ref = attributeText(tag, "ref", &has);
        quint32 row = 0, col = 0;
        if (!has || !parseCellRef(ref, &row, &col)) continue;
        quint32 r = row, cc = col;
        const bool alive = followCell(ctx, sheet, &r, &cc);
        if (alive && r == row && cc == col) continue;
        result += xml.mid(copied, t.start - copied);
        if (alive) {
            setAttribute(&tag, "ref", cellName(r, cc).toLatin1());
            result += tag + xml.mid(t.end, e.end - t.end);
        }
        copied = e.end;
        any = true;
    }
    if (!any) {
        *out = xml;
        *changed = false;
        return true;
    }
    result += xml.mid(copied);
    *out = result;
    *changed = true;
    return true;
}

} // namespace

bool moveCommentsPart(const QByteArray &xml, const MoveContext &ctx, int sheet, QByteArray *out, bool *changed) { return moveElementsByRef(xml, ctx, sheet, "comment", out, changed); }

bool moveThreadedCommentsPart(const QByteArray &xml, const MoveContext &ctx, int sheet, QByteArray *out, bool *changed) { return moveElementsByRef(xml, ctx, sheet, "threadedComment", out, changed); }

bool moveControlPart(const QByteArray &xml, const MoveContext &ctx, int sheet, QByteArray *out, bool *changed)
{
    // <formControlPr objectType="Drop" fmlaLink="$A$5" fmlaRange="Sheet1!$A$1:$A$5"/>: the cell a control sets and the range it lists (and the text box it reads) are references
    Tag t;
    qsizetype pos = 0;
    *out = xml;
    *changed = false;
    while (xmlscan::nextTag(xml, pos, &t)) {
        pos = t.end;
        if (t.closing || t.local != "formControlPr") continue;
        QByteArray tag = xml.mid(t.start, t.end - t.start);
        const QByteArray before = tag;
        for (const char *name : {"fmlaLink", "fmlaRange", "fmlaTxbx"}) {
            bool has = false;
            const QString value = attributeText(tag, name, &has);
            if (!has) continue;
            const QString now = adjustAll(value, sheet, ctx);
            if (now != value) setAttribute(&tag, name, xmlscan::escapeAttribute(now));
        }
        if (tag != before) {
            *out = xml.left(t.start) + tag + xml.mid(t.end);
            *changed = true;
        }
        break;
    }
    return true;
}

bool moveNotesDrawing(const QByteArray &vml, const MoveContext &ctx, int sheet, QByteArray *out, bool *changed)
{
    // each note's shape holds <x:Row>1</x:Row> <x:Column>2</x:Column> (from 0) and an <x:Anchor> of eight numbers (column, offset, row, offset, twice)
    // (the prefixes are the usual ones in the files of the programs people know, but the format does not fix them: whatever prefix an element has, it is kept)
    static const QRegularExpression shape(QStringLiteral("<((?:[A-Za-z_][\\w.\\-]*:)?)shape\\b.*?</\\1shape>"), QRegularExpression::DotMatchesEverythingOption);
    static const QRegularExpression rowRe(QStringLiteral("<((?:[A-Za-z_][\\w.\\-]*:)?)Row>(\\d+)</\\1Row>")), colRe(QStringLiteral("<((?:[A-Za-z_][\\w.\\-]*:)?)Column>(\\d+)</\\1Column>")),
        anchorRe(QStringLiteral("<((?:[A-Za-z_][\\w.\\-]*:)?)Anchor>([^<]*)</\\1Anchor>"));
    const QString text = QString::fromUtf8(vml);
    QString result;
    int at = 0;
    bool any = false;
    QRegularExpressionMatchIterator it = shape.globalMatch(text);
    while (it.hasNext()) {
        const QRegularExpressionMatch m = it.next();
        QString body = m.captured();
        const QRegularExpressionMatch rm = rowRe.match(body), cm = colRe.match(body);
        if (!rm.hasMatch() || !cm.hasMatch()) continue;
        const quint32 row = rm.captured(2).toUInt(), col = cm.captured(2).toUInt();
        quint32 r = row, c = col;
        const bool alive = followCell(ctx, sheet, &r, &c);
        if (alive && r == row && c == col) continue;
        result += text.mid(at, m.capturedStart() - at);
        at = m.capturedEnd();
        any = true;
        if (!alive) continue;
        body.replace(rowRe, QStringLiteral("<\\1Row>%1</\\1Row>").arg(r));
        body.replace(colRe, QStringLiteral("<\\1Column>%1</\\1Column>").arg(c));
        const QRegularExpressionMatch am = anchorRe.match(body);
        if (am.hasMatch()) {
            QStringList parts = am.captured(2).split(QLatin1Char(','));
            if (parts.size() >= 8) {
                for (QString &part : parts) part = part.trimmed();
                const int dc = int(c) - int(col), dr = int(r) - int(row);
                parts[0] = QString::number(parts[0].trimmed().toInt() + dc);
                parts[2] = QString::number(parts[2].trimmed().toInt() + dr);
                parts[4] = QString::number(parts[4].trimmed().toInt() + dc);
                parts[6] = QString::number(parts[6].trimmed().toInt() + dr);
                body.replace(anchorRe, QStringLiteral("<\\1Anchor>%1</\\1Anchor>").arg(parts.join(QStringLiteral(", "))));
            }
        }
        result += body;
    }
    if (!any) {
        *out = vml;
        *changed = false;
        return true;
    }
    result += text.mid(at);
    *out = result.toUtf8();
    *changed = true;
    return true;
}

} // namespace jp::office::sheet
