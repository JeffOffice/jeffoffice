// Putting changed cells into a sheet's XML without disturbing the rest of it: the XML is streamed through, every byte is passed on as it was, except in the rows that have a change,
// where the cells that changed are written anew and the others are copied. A shared formula whose first cell was changed is written out for the cells that shared it, so no cell
// loses its formula. See xlsxsave.h.

#include "sheet/cellref.h"
#include "sheet/xlsxbase.h"
#include "sheet/xlsxsave.h"
#include "sheet/xmlscan.h"

#include <QCoreApplication>
#include <QHash>
#include <QIODevice>
#include <QRegularExpression>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <unordered_map>

namespace jp::office::sheet {

namespace {

using xmlscan::Tag;

constexpr qsizetype kChunk = 1 << 20;


// ---- a cell, as the file writes it

QByteArray numberText(double v)
{
    if (!std::isfinite(v)) return QByteArray("0");
    if (v == std::floor(v) && std::fabs(v) < 1e15) return QByteArray::number(qlonglong(v));
    for (int digits = 15; digits <= 17; ++digits) {
        const QByteArray text = QByteArray::number(v, 'g', digits);
        if (text.toDouble() == v || digits == 17) return text;
    }
    return QByteArray::number(v, 'g', 17);
}

bool needsPreserve(const QString &text)
{
    return text.isEmpty() ? false : (text.front().isSpace() || text.back().isSpace() || text.contains(QLatin1Char('\n')) || text.contains(QLatin1Char('\t')) || text.contains(QLatin1String("  ")));
}

// The type attribute and the content of the value of a cell (what follows the formula): the attribute with its leading space, or nothing for a number.
struct ValueXml {
    QByteArray type;      // ` t="b"` ...
    QByteArray content;   // `<v>1</v>` or `<is>...</is>`; empty for none
};

ValueXml valueXml(const SavedValue &v, bool formula, bool hasShared, const QByteArray &p)
{
    const QByteArray vo = "<" + p + "v>", vc = "</" + p + "v>";
    ValueXml x;
    switch (v.kind) {
    case SavedValue::Kind::Empty:
        if (formula) x.content = vo + "0" + vc;
        break;
    case SavedValue::Kind::Number: x.content = vo + numberText(v.number) + vc; break;
    case SavedValue::Kind::Bool:
        x.type = " t=\"b\"";
        x.content = vo + (v.number != 0 ? "1" : "0") + vc;
        break;
    case SavedValue::Kind::Error:
        x.type = " t=\"e\"";
        x.content = vo + errorText(CellError(int(v.number))).toUtf8() + vc;
        break;
    case SavedValue::Kind::Text:
        if (formula) {
            x.type = " t=\"str\"";
            x.content = vo + xmlscan::escapeText(v.text) + vc;
        } else if (v.sharedString >= 0 && hasShared) {
            x.type = " t=\"s\"";
            x.content = vo + QByteArray::number(v.sharedString) + vc;
        } else {
            x.type = " t=\"inlineStr\"";
            x.content = "<" + p + "is><" + p + "t" + (needsPreserve(v.text) ? " xml:space=\"preserve\"" : "") + ">" + xmlscan::escapeText(v.text) + "</" + p + "t></" + p + "is>";
        }
        break;
    }
    return x;
}

QByteArray addressOf(quint32 row, quint32 col) { return columnName(col).toLatin1() + QByteArray::number(qulonglong(row) + 1); }

// A cell the person changed: nothing at all when it holds nothing and has no format of its own.
QByteArray cellXml(const QByteArray &prefix, const SavedCell &c, bool hasShared)
{
    const bool empty = !c.isFormula && c.value.kind == SavedValue::Kind::Empty;
    if (empty && c.style == 0) return {};
    QByteArray out = "<" + prefix + "c r=\"" + addressOf(c.row, c.col) + "\"";
    if (c.style != 0) out += " s=\"" + QByteArray::number(c.style) + "\"";
    if (empty) return out + "/>";
    const ValueXml v = valueXml(c.value, c.isFormula, hasShared, prefix);
    out += v.type + ">";
    if (c.isFormula) {
        QString f = c.formula;
        if (f.startsWith(QLatin1Char('='))) f.remove(0, 1);
        out += "<" + prefix + "f" + (c.arrayRef.isEmpty() ? QByteArray() : " t=\"array\" ref=\"" + c.arrayRef.toLatin1() + "\"") + ">" + xmlscan::escapeText(f) + "</" + prefix + "f>";
        if (c.hasResult) out += v.content;
    } else {
        out += v.content;
    }
    return out + "</" + prefix + "c>";
}

// ---- reading the cells of the old XML

// Where the first child element with the name is in a cell's bytes: [start, end).
bool findChild(QByteArrayView cell, qsizetype from, QByteArrayView local, qsizetype *start, qsizetype *end)
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

struct SharedMaster {
    quint32 row = 0, col = 0;
    QString text;
};

struct Item {
    quint32 col = 0;
    const SavedCell *cell = nullptr;
    const SavedResult *result = nullptr;
};

struct RowEdits {
    quint32 row = 0;
    std::vector<Item> items;   // sorted by column
    const RowProps *props = nullptr;   // its height or that it is hidden, changed
};

// Sets an attribute in a tag's bytes (a start tag): its value replaced, or the attribute put in before the end of the tag.
void setAttr(QByteArray *tag, const char *name, const QByteArray &value)
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

bool isSpaceByte(char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }

// Takes an attribute, and the white space before it, out of a tag's bytes.
void dropAttr(QByteArray *tag, const char *name)
{
    qsizetype vs = 0, ve = 0;
    if (!xmlscan::attributeValue(*tag, name, &vs, &ve)) return;
    qsizetype n = vs - 2;
    while (n >= 0 && isSpaceByte((*tag)[n])) --n;
    if (n >= 0 && (*tag)[n] == '=') --n;
    while (n >= 0 && isSpaceByte((*tag)[n])) --n;
    qsizetype nameStart = n;
    while (nameStart > 0 && !isSpaceByte((*tag)[nameStart - 1])) --nameStart;
    qsizetype a = nameStart;
    while (a > 0 && isSpaceByte((*tag)[a - 1])) --a;
    tag->remove(a, ve + 1 - a);
}

void setFlag(QByteArray *tag, const char *name, bool on)
{
    if (on) setAttr(tag, name, "1");
    else dropAttr(tag, name);
}

void applyRowProps(QByteArray *tag, const RowProps &p)
{
    if (p.height >= 0) setAttr(tag, "ht", numberText(p.height));
    setFlag(tag, "customHeight", p.customHeight && p.height >= 0);
    setFlag(tag, "hidden", p.hidden);
    if (p.styleKnown) {
        if (p.customFormat && p.style != 0) setAttr(tag, "s", QByteArray::number(p.style));
        else dropAttr(tag, "s");
        setFlag(tag, "customFormat", p.customFormat && p.style != 0);
    }
    if (p.outlineKnown) {
        if (p.outline > 0) setAttr(tag, "outlineLevel", QByteArray::number(int(p.outline)));
        else dropAttr(tag, "outlineLevel");
        setFlag(tag, "collapsed", p.collapsed);
    }
}

// The columns' element of a sheet's head (the bytes before sheetData) made from the runs of columns the sheet has: in place of the file's, or before sheetData if the file has none.
QByteArray columnsFromRuns(const QByteArray &head, const std::vector<ColumnSpan> &runs, const QByteArray &defaultPrefix)
{
    qsizetype colsStart = -1, colsEnd = -1;
    QByteArray prefix = defaultPrefix;
    Tag t;
    qsizetype pos = 0;
    while (xmlscan::nextTag(head, pos, &t)) {
        pos = t.end;
        if (t.closing || t.local != "cols") continue;
        colsStart = t.start;
        colsEnd = t.end;
        prefix = QByteArray(xmlscan::prefixOf(t.name));
        if (!t.selfClosing) {
            Tag inner;
            qsizetype p = t.end;
            while (xmlscan::nextTag(head, p, &inner)) {
                p = inner.end;
                if (inner.closing && inner.local == "cols") {
                    colsEnd = inner.end;
                    break;
                }
            }
        }
        break;
    }
    QByteArray out;
    if (!runs.empty()) {
        out = "<" + prefix + "cols>";
        for (const ColumnSpan &c : runs) {
            QByteArray tag = "<" + prefix + "col min=\"" + QByteArray::number(c.first + 1) + "\" max=\"" + QByteArray::number(c.last + 1) + "\"";
            if (c.width >= 0) tag += " width=\"" + numberText(c.width) + "\"";
            if (c.style != 0) tag += " style=\"" + QByteArray::number(c.style) + "\"";
            if (c.hidden) tag += " hidden=\"1\"";
            if (c.bestFit) tag += " bestFit=\"1\"";
            if (c.customWidth) tag += " customWidth=\"1\"";
            if (c.phonetic) tag += " phonetic=\"1\"";
            if (c.outline > 0) tag += " outlineLevel=\"" + QByteArray::number(int(c.outline)) + "\"";
            if (c.collapsed) tag += " collapsed=\"1\"";
            out += tag + "/>";
        }
        out += "</" + prefix + "cols>";
    }
    if (colsStart >= 0) return head.left(colsStart) + out + head.mid(colsEnd);
    return head + out;
}

// The head of a sheet with its `sheetFormatPr` made to say the sheet's default sizes: the file's element with those attributes changed (what else it says stays), or one made before the
// columns (or before the cells) if the file has none.
QByteArray formatFromChange(const QByteArray &head, const SheetFormat &f, const QByteArray &defaultPrefix)
{
    Tag t;
    qsizetype pos = 0, found = -1, foundEnd = -1, colsAt = -1;
    QByteArray prefix = defaultPrefix;
    while (xmlscan::nextTag(head, pos, &t)) {
        pos = t.end;
        if (t.closing) continue;
        if (t.local == "sheetFormatPr" && found < 0) {
            found = t.start;
            foundEnd = t.end;
            prefix = QByteArray(xmlscan::prefixOf(t.name));
        } else if (t.local == "cols" && colsAt < 0) {
            colsAt = t.start;
        }
    }
    auto apply = [&](QByteArray *tag) {
        setAttr(tag, "defaultRowHeight", numberText(f.defaultRowHeight));
        setFlag(tag, "customHeight", f.customHeight);
        if (f.defaultColWidth > 0) setAttr(tag, "defaultColWidth", numberText(f.defaultColWidth));
        else dropAttr(tag, "defaultColWidth");
    };
    if (found >= 0) {
        QByteArray tag = head.mid(found, foundEnd - found);
        apply(&tag);
        return head.left(found) + tag + head.mid(foundEnd);
    }
    QByteArray tag = "<" + prefix + "sheetFormatPr baseColWidth=\"" + QByteArray::number(f.baseColWidth) + "\"/>";
    apply(&tag);
    const qsizetype at = colsAt >= 0 ? colsAt : head.size();
    return head.left(at) + tag + head.mid(at);
}

// The head of a sheet with the color of its tab made: the file's `tabColor` replaced (or taken out, for no color), or one put first in `sheetPr` (which is made, first in the sheet, if the
// file has none).
QByteArray tabColorFromChange(const QByteArray &head, const ColorSpec &color, const QByteArray &defaultPrefix)
{
    Tag t;
    qsizetype pos = 0, rootEnd = -1, sheetPrStart = -1, sheetPrEnd = -1, closeStart = -1, closeEnd = -1, tabStart = -1, tabEnd = -1;
    bool sheetPrSelf = false;
    QByteArray prefix = defaultPrefix;
    int depth = 0;
    while (xmlscan::nextTag(head, pos, &t)) {
        pos = t.end;
        if (rootEnd < 0 && !t.closing) {
            rootEnd = t.end;
            prefix = QByteArray(xmlscan::prefixOf(t.name));
            if (t.selfClosing) break;
            continue;
        }
        if (t.closing) {
            --depth;
            if (sheetPrStart >= 0 && closeStart < 0 && t.local == "sheetPr") {
                closeStart = t.start;
                closeEnd = t.end;
            }
            continue;
        }
        if (t.local == "sheetPr" && sheetPrStart < 0) {
            sheetPrStart = t.start;
            sheetPrEnd = t.end;
            sheetPrSelf = t.selfClosing;
        } else if (t.local == "tabColor" && sheetPrStart >= 0 && closeStart < 0 && tabStart < 0) {
            tabStart = t.start;
            tabEnd = t.end;
        }
        if (!t.selfClosing) ++depth;
        if (t.local == "sheetData") break;
    }
    if (rootEnd < 0) return head;
    QByteArray element = colorXml("tabColor", color);
    if (!element.isEmpty() && !prefix.isEmpty()) element.replace("<tabColor", "<" + prefix + "tabColor");
    if (sheetPrStart < 0) {
        if (element.isEmpty()) return head;
        return head.left(rootEnd) + "<" + prefix + "sheetPr>" + element + "</" + prefix + "sheetPr>" + head.mid(rootEnd);
    }
    if (sheetPrSelf) {
        if (element.isEmpty()) return head;
        QByteArray open = head.mid(sheetPrStart, sheetPrEnd - sheetPrStart);
        open.chop(2);
        while (!open.isEmpty() && isSpaceByte(open.back())) open.chop(1);
        return head.left(sheetPrStart) + open + ">" + element + "</" + prefix + "sheetPr>" + head.mid(sheetPrEnd);
    }
    if (tabStart >= 0) return head.left(tabStart) + element + head.mid(tabEnd);
    if (element.isEmpty()) return head;
    return head.left(sheetPrEnd) + element + head.mid(sheetPrEnd);
}

// The head of a sheet with its first view said to be the one that is open (`tabSelected`), or not: the attribute put on or taken off the first `sheetView`; for a sheet that is to be
// open and has no view, one made before the sizes of its columns (or before the cells).
QByteArray tabSelectedFromChange(const QByteArray &head, bool selected, const QByteArray &defaultPrefix)
{
    Tag t;
    qsizetype pos = 0, viewAt = -1, viewEnd = -1, insertAt = -1;
    QByteArray prefix = defaultPrefix;
    bool first = true;
    while (xmlscan::nextTag(head, pos, &t)) {
        pos = t.end;
        if (t.closing) continue;
        if (first) {
            first = false;
            prefix = QByteArray(xmlscan::prefixOf(t.name));
            continue;
        }
        if (t.local == "sheetView" && viewAt < 0) {
            viewAt = t.start;
            viewEnd = t.end;
        } else if ((t.local == "sheetFormatPr" || t.local == "cols" || t.local == "sheetData") && insertAt < 0) {
            insertAt = t.start;
        }
    }
    if (viewAt >= 0) {
        QByteArray tag = head.mid(viewAt, viewEnd - viewAt);
        setFlag(&tag, "tabSelected", selected);
        return head.left(viewAt) + tag + head.mid(viewEnd);
    }
    if (!selected) return head;
    const qsizetype at = insertAt >= 0 ? insertAt : head.size();
    return head.left(at) + "<" + prefix + "sheetViews><" + prefix + "sheetView tabSelected=\"1\" workbookViewId=\"0\"/></" + prefix + "sheetViews>" + head.mid(at);
}

// The head of a sheet with its first view shown as `kind` says (0 normal: the `view` attribute is taken off; 1 the page breaks: pageBreakPreview; 2 the pages: pageLayout) at `zoom` percent
// (0: the file's zoom stays): the zoom is put in as the zoom of that kind of view too. A sheet with no view is given one before the sizes of its columns (or before the cells).
QByteArray viewKindFromChange(const QByteArray &head, int kind, int zoom, const QByteArray &defaultPrefix)
{
    Tag t;
    qsizetype pos = 0, viewAt = -1, viewEnd = -1, insertAt = -1;
    QByteArray prefix = defaultPrefix;
    bool first = true;
    while (xmlscan::nextTag(head, pos, &t)) {
        pos = t.end;
        if (t.closing) continue;
        if (first) {
            first = false;
            prefix = QByteArray(xmlscan::prefixOf(t.name));
            continue;
        }
        if (t.local == "sheetView" && viewAt < 0) {
            viewAt = t.start;
            viewEnd = t.end;
        } else if ((t.local == "sheetFormatPr" || t.local == "cols" || t.local == "sheetData") && insertAt < 0) {
            insertAt = t.start;
        }
    }
    auto apply = [&](QByteArray *tag) {
        if (kind == 1) setAttr(tag, "view", "pageBreakPreview");
        else if (kind == 2) setAttr(tag, "view", "pageLayout");
        else dropAttr(tag, "view");
        if (zoom > 0) {
            setAttr(tag, "zoomScale", QByteArray::number(zoom));
            if (kind == 0) setAttr(tag, "zoomScaleNormal", QByteArray::number(zoom));
            else setAttr(tag, kind == 1 ? "zoomScaleSheetLayoutView" : "zoomScalePageLayoutView", QByteArray::number(zoom));
        }
    };
    if (viewAt >= 0) {
        QByteArray tag = head.mid(viewAt, viewEnd - viewAt);
        apply(&tag);
        return head.left(viewAt) + tag + head.mid(viewEnd);
    }
    if (kind == 0) return head;
    QByteArray tag = "<" + prefix + "sheetView workbookViewId=\"0\"/>";
    apply(&tag);
    const qsizetype at = insertAt >= 0 ? insertAt : head.size();
    return head.left(at) + "<" + prefix + "sheetViews>" + tag + "</" + prefix + "sheetViews>" + head.mid(at);
}

// What follows a sheet's cells (from after "</sheetData>" to the end) with its merged ranges made from `merges`: in place of the element the file has, or where the format puts it.
QByteArray tailWithMerges(const QByteArray &after, const std::vector<CellRange> &merges, const QByteArray &prefix)
{
    QByteArray generated;
    if (!merges.empty()) {
        generated = "<" + prefix + "mergeCells count=\"" + QByteArray::number(qulonglong(merges.size())) + "\">";
        for (const CellRange &m : merges) generated += "<" + prefix + "mergeCell ref=\"" + rangeName(m).toLatin1() + "\"/>";
        generated += "</" + prefix + "mergeCells>";
    }
    static const QList<QByteArray> later = {"phoneticPr", "conditionalFormatting", "dataValidations", "hyperlinks", "printOptions", "pageMargins", "pageSetup", "headerFooter", "rowBreaks", "colBreaks",
                                            "customProperties", "cellWatches", "ignoredErrors", "smartTags", "drawing", "legacyDrawing", "legacyDrawingHF", "drawingHF", "picture", "oleObjects",
                                            "controls", "webPublishItems", "tableParts", "extLst"};
    Tag t;
    qsizetype pos = 0, existingStart = -1, existingEnd = -1, insertAt = -1;
    int depth = 0;
    while (xmlscan::nextTag(after, pos, &t)) {
        pos = t.end;
        if (t.closing) {
            if (--depth < 0) {
                if (insertAt < 0) insertAt = t.start;   // </worksheet>
                break;
            }
            if (depth == 0 && existingStart >= 0 && existingEnd < 0 && t.local == "mergeCells") existingEnd = t.end;
            continue;
        }
        if (depth == 0) {
            if (t.local == "mergeCells" && existingStart < 0) {
                existingStart = t.start;
                if (t.selfClosing) existingEnd = t.end;
            } else if (insertAt < 0 && later.contains(QByteArray(t.local))) {
                insertAt = t.start;
            }
        }
        if (!t.selfClosing) ++depth;
    }
    if (existingStart >= 0 && existingEnd >= 0) return after.left(existingStart) + generated + after.mid(existingEnd);
    if (generated.isEmpty() || insertAt < 0) return after;
    return after.left(insertAt) + generated + after.mid(insertAt);
}

// What follows a sheet's cells with the element that names its drawing put in, where the format has it (after the page setup and the header and footer, before the elements that come
// after the drawing); not if the sheet has one.
QByteArray tailWithDrawing(const QByteArray &after, const QByteArray &relId, const QByteArray &prefix)
{
    static const QList<QByteArray> later = {"legacyDrawing", "legacyDrawingHF", "drawingHF", "picture", "oleObjects", "controls", "webPublishItems", "tableParts", "extLst"};
    Tag t;
    qsizetype pos = 0, insertAt = -1;
    int depth = 0;
    while (xmlscan::nextTag(after, pos, &t)) {
        pos = t.end;
        if (t.closing) {
            if (--depth < 0) {
                if (insertAt < 0) insertAt = t.start;   // </worksheet>
                break;
            }
            continue;
        }
        if (depth == 0) {
            if (t.local == "drawing") {
                // the sheet names a drawing that its relationships do not (or whose part is not there): the element names the one made now
                QByteArray tag = after.mid(t.start, t.end - t.start);
                static const QRegularExpression idAttr(QStringLiteral("(\\s[A-Za-z_][\\w.-]*:id\\s*=\\s*)(\"[^\"]*\"|'[^']*')"));
                const QString text = QString::fromUtf8(tag);
                const QRegularExpressionMatch m = idAttr.match(text);
                if (!m.hasMatch()) return after;
                const QString fixed = text.left(m.capturedStart(2)) + QLatin1Char('"') + QString::fromLatin1(xmlscan::escapeAttribute(QString::fromLatin1(relId))) + QLatin1Char('"') + text.mid(m.capturedEnd(2));
                return after.left(t.start) + fixed.toUtf8() + after.mid(t.end);
            }
            if (insertAt < 0 && later.contains(QByteArray(t.local))) insertAt = t.start;
        }
        if (!t.selfClosing) ++depth;
    }
    if (insertAt < 0) return after;
    const QByteArray element = "<" + prefix + "drawing xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" r:id=\"" + xmlscan::escapeAttribute(QString::fromLatin1(relId)) + "\"/>";
    return after.left(insertAt) + element + after.mid(insertAt);
}

class Patcher {
public:
    Patcher(QIODevice *in, PartSink *out, const SheetChanges &changes, bool hasShared, const std::atomic<bool> *cancel) : m_in(in), m_out(out), m_hasShared(hasShared), m_cancel(cancel)
    {
        // by row
        for (const SavedCell &c : changes.cells) {
            RowEdits &r = rowFor(c.row);
            r.items.push_back({c.col, &c, nullptr});
            m_cells.push_back(&c);
        }
        for (const SavedResult &c : changes.results) {
            RowEdits &r = rowFor(c.row);
            r.items.push_back({c.col, nullptr, &c});
        }
        for (const RowProps &p : changes.rows) rowFor(p.row).props = &p;
        if (changes.replaceColumns) m_columns = &changes.columns;
        if (changes.replaceMerges) m_merges = &changes.merges;
        if (changes.setPage) m_page = &changes.page;
        m_drawingId = changes.drawingRelId.toLatin1();
        if (changes.replaceFormat) m_format = &changes.format;
        if (changes.replaceTabColor) m_tabColor = &changes.tabColor;
        m_tabSelected = changes.tabSelected;
        m_viewKind = changes.viewKind;
        m_viewZoom = changes.viewZoom;
        for (RowEdits &r : m_rows) {
            std::stable_sort(r.items.begin(), r.items.end(), [](const Item &a, const Item &b) { return a.col < b.col; });
            // (one item for a cell: a changed cell stands over a result)
            std::vector<Item> merged;
            for (const Item &it : r.items) {
                if (!merged.empty() && merged.back().col == it.col) {
                    if (it.cell) merged.back() = it;
                } else {
                    merged.push_back(it);
                }
            }
            r.items = std::move(merged);
        }
        std::sort(m_rows.begin(), m_rows.end(), [](const RowEdits &a, const RowEdits &b) { return a.row < b.row; });
    }

    bool run(QString *error)
    {
        m_error = error;
        if (!head()) return false;
        if (!rows()) return false;
        return tail();
    }

private:
    RowEdits &rowFor(quint32 row)
    {
        if (!m_rows.empty() && m_rows.back().row == row) return m_rows.back();
        const auto found = m_rowAt.find(row);
        if (found != m_rowAt.end()) return m_rows[found->second];
        m_rowAt.emplace(row, m_rows.size());
        m_rows.push_back(RowEdits{row, {}});
        return m_rows.back();
    }

    bool fail(const QString &why)
    {
        if (m_error && m_error->isEmpty()) *m_error = why;
        return false;
    }

    // ---- the buffer: [0, m_written) is done with, [m_written, m_cur) is passed on as it is (and not yet written), the rest is not yet looked at
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

    void flushPending()
    {
        if (m_cur > m_written) {
            m_out->write(m_buf.constData() + m_written, m_cur - m_written);
            m_written = m_cur;
        }
    }

    // Lets go of what is done with (at a place where no position into the buffer is kept).
    void compact()
    {
        if (m_written < kChunk) return;
        m_buf.remove(0, m_written);
        m_cur -= m_written;
        m_written = 0;
    }

    void put(const QByteArray &bytes)
    {
        flushPending();
        if (!bytes.isEmpty()) m_out->write(bytes.constData(), bytes.size());
    }

    bool nextTag(qsizetype from, Tag *tag)
    {
        for (;;) {
            if (xmlscan::nextTag(m_buf, from, tag)) return true;
            if (!fill()) return false;
        }
    }

    bool find(const QByteArray &what, qsizetype from, qsizetype *at)
    {
        for (;;) {
            const qsizetype i = m_buf.indexOf(what, from);
            if (i >= 0) {
                *at = i;
                return true;
            }
            from = std::max(from, qsizetype(m_buf.size()) - what.size() + 1);
            if (!fill()) return false;
        }
    }

    // The closing tag of an element that begins (its open tag ends) at `from`, the first that is a tag: one inside a comment or a CDATA section is not. The plain search is exact unless
    // a comment or a CDATA section begins before the tag it found, and then the tags are walked (the pieces the sections hold are passed over). `at` is where the tag begins.
    bool closeOfElement(const QByteArray &closeTag, QByteArrayView local, qsizetype from, qsizetype *at)
    {
        if (!find(closeTag, from, at)) return false;
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

    bool cancelled() const { return m_cancel && m_cancel->load(std::memory_order_relaxed); }

    // ---- the part before the rows
    bool head()
    {
        qsizetype pos = 0;
        Tag t;
        qsizetype dimStart = -1, dimEnd = -1;
        for (;;) {
            if (!nextTag(pos, &t)) return fail(m_failed ? QCoreApplication::translate("jp::office::sheet::Save", "The sheet's part is damaged.") : QCoreApplication::translate("jp::office::sheet::Save", "The sheet's file has no place for cells in it, so it can't be saved."));
            pos = t.end;
            if (t.closing) continue;
            if (t.local == "dimension" && dimStart < 0) {
                dimStart = t.start;
                dimEnd = t.end;
            }
            if (t.local == "sheetData") break;
        }
        m_prefix = QByteArray(xmlscan::prefixOf(t.name));
        const qsizetype dataStart = t.start;
        const bool selfClosed = t.selfClosing;
        const qsizetype dataEnd = t.end;
        QByteArray before = m_buf.left(dataStart);
        if (dimStart >= 0 && !m_cells.empty()) before = withDimension(before, dimStart, dimEnd);
        if (m_tabColor) before = tabColorFromChange(before, *m_tabColor, m_prefix);
        if (m_tabSelected >= 0) before = tabSelectedFromChange(before, m_tabSelected != 0, m_prefix);
        if (m_viewKind >= 0) before = viewKindFromChange(before, m_viewKind, m_viewZoom, m_prefix);
        if (m_format) before = formatFromChange(before, *m_format, m_prefix);
        if (m_columns) before = columnsFromRuns(before, *m_columns, m_prefix);
        if (m_page) before = patchFitToPageHead(before, *m_page);
        put(before);
        m_written = m_cur = dataStart;
        if (selfClosed) {
            // an empty sheetData: opened, given its rows, and closed
            QByteArray open = m_buf.mid(dataStart, dataEnd - dataStart);
            open.chop(2);
            while (!open.isEmpty() && (open.back() == ' ' || open.back() == '\t' || open.back() == '\n' || open.back() == '\r')) open.chop(1);
            put(open + ">");
            m_written = m_cur = dataEnd;
            if (!newRowsFrom(0, 0xFFFFFFFFu)) return false;
            put("</" + m_prefix + "sheetData>");
            m_afterData = true;
            m_dataClosed = true;
        } else {
            m_cur = dataEnd;
        }
        return true;
    }

    QByteArray withDimension(const QByteArray &before, qsizetype tagStart, qsizetype tagEnd)
    {
        const QByteArray tag = before.mid(tagStart, tagEnd - tagStart);
        qsizetype vs = 0, ve = 0;
        if (!xmlscan::attributeValue(tag, "ref", &vs, &ve)) return before;
        const QString ref = QString::fromLatin1(tag.mid(vs, ve - vs));
        CellRange range;
        if (!parseRangeRef(ref, &range)) return before;
        CellRange now = range;
        for (const SavedCell *c : m_cells) {
            if (c->value.kind == SavedValue::Kind::Empty && !c->isFormula && c->style == 0) continue;
            now.row1 = std::min(now.row1, c->row);
            now.row2 = std::max(now.row2, c->row);
            now.col1 = std::min<quint32>(now.col1, c->col);
            now.col2 = std::max<quint32>(now.col2, c->col);
        }
        if (now == range) return before;
        QByteArray out = before;
        out.replace(tagStart + vs, ve - vs, rangeName(now).toLatin1());
        return out;
    }

    // ---- the rows
    bool rows()
    {
        if (m_afterData) return true;
        quint32 lastRow = 0;   // the row number (from 1) of the last row element
        size_t next = 0;       // the first of the changed rows not yet written
        for (;;) {
            if (cancelled()) return fail(QCoreApplication::translate("jp::office::sheet::Save", "The save was stopped."));
            if (m_cur - m_written > kChunk) flushPending();
            compact();
            Tag t;
            if (!nextTag(m_cur, &t)) return fail(m_failed ? QCoreApplication::translate("jp::office::sheet::Save", "The sheet's part is damaged.") : QCoreApplication::translate("jp::office::sheet::Save", "The sheet's cells are cut short."));
            if (t.closing) {
                if (t.local == "sheetData") {
                    m_cur = t.start;
                    if (!newRowsFrom(next, 0xFFFFFFFFu)) return false;
                    m_afterData = true;
                    return true;
                }
                return fail(QCoreApplication::translate("jp::office::sheet::Save", "The sheet's cells are not in the form this program changes."));
            }
            if (t.local != "row") return fail(QCoreApplication::translate("jp::office::sheet::Save", "The sheet's cells are not in the form this program changes."));
            // the row's number
            const QByteArray tagBytes = m_buf.mid(t.start, t.end - t.start);
            bool haveR = false;
            const QByteArrayView rAttr = xmlscan::attribute(tagBytes, "r", &haveR);
            quint32 rowIdx = lastRow;   // (from 0: the next row)
            if (haveR) {
                bool ok = false;
                const qulonglong n = QByteArray(rAttr).toULongLong(&ok);
                if (ok && n >= 1 && n <= kMaxRows) rowIdx = quint32(n - 1);
            }
            lastRow = rowIdx + 1;
            const qsizetype start = t.start;
            const bool selfClosing = t.selfClosing;
            qsizetype rowEnd = t.end;
            // rows that did not exist, which come before this one
            if (next < m_rows.size() && m_rows[next].row < rowIdx) {
                m_cur = start;
                if (!newRowsFrom(next, rowIdx)) return false;
                while (next < m_rows.size() && m_rows[next].row < rowIdx) ++next;
            }
            const RowEdits *edits = nullptr;
            if (next < m_rows.size() && m_rows[next].row == rowIdx) edits = &m_rows[next++];
            const bool slow = edits || (!m_dropped.isEmpty() && rowIdx <= m_sharedUntil);
            if (!selfClosing) {
                const QByteArray closeTag = "</" + m_prefix + "row>";
                qsizetype close = 0;
                if (!closeOfElement(closeTag, "row", rowEnd, &close)) return fail(m_failed ? QCoreApplication::translate("jp::office::sheet::Save", "The sheet's part is damaged.") : QCoreApplication::translate("jp::office::sheet::Save", "The sheet's cells are cut short."));
                // (the end of the closing tag: it may have a space before its ">")
                Tag closing;
                if (!nextTag(close, &closing) || closing.start != close) return fail(QCoreApplication::translate("jp::office::sheet::Save", "The sheet's cells are cut short."));
                rowEnd = closing.end;
            }
            if (!slow) {
                m_cur = rowEnd;
                continue;
            }
            m_cur = start;
            const QByteArray row = m_buf.mid(start, rowEnd - start);
            QByteArray rewritten;
            if (!rewriteRow(row, rowIdx, edits, &rewritten)) return false;
            put(rewritten);
            m_cur = m_written = rowEnd;
        }
    }

    // Writes the rows from m_rows[from] up to (not including) the row `before`, which are new, at m_cur.
    bool newRowsFrom(size_t from, quint32 before)
    {
        QByteArray out;
        for (size_t i = from; i < m_rows.size() && m_rows[i].row < before; ++i) {
            QByteArray cells;
            for (const Item &it : m_rows[i].items)
                if (it.cell) cells += cellXml(m_prefix, *it.cell, m_hasShared);
            if (cells.isEmpty() && !m_rows[i].props) continue;
            QByteArray open = "<" + m_prefix + "row r=\"" + QByteArray::number(qulonglong(m_rows[i].row) + 1) + "\">";
            if (m_rows[i].props) applyRowProps(&open, *m_rows[i].props);
            out += open + cells + "</" + m_prefix + "row>";
        }
        put(out);
        return true;
    }

    // One row, with the changes: the cells that changed written anew, the new cells put in their places, the others as they were.
    bool rewriteRow(const QByteArray &row, quint32 rowIdx, const RowEdits *edits, QByteArray *out)
    {
        Tag open;
        if (!xmlscan::nextTag(row, 0, &open)) return fail(QCoreApplication::translate("jp::office::sheet::Save", "The sheet's cells are cut short."));
        QByteArray body;
        QByteArray rest;
        quint32 minCol = 0xFFFFFFFFu, maxCol = 0;
        auto widen = [&](quint32 col) {
            minCol = std::min(minCol, col);
            maxCol = std::max(maxCol, col);
        };
        size_t e = 0;
        const size_t n = edits ? edits->items.size() : 0;
        auto writeNewUntil = [&](quint32 col, bool inclusive) {
            while (e < n && (edits->items[e].col < col || (inclusive && edits->items[e].col == col))) {
                const Item &it = edits->items[e++];
                if (it.cell) {
                    const QByteArray x = cellXml(m_prefix, *it.cell, m_hasShared);
                    if (!x.isEmpty()) {
                        body += x;
                        widen(it.col);
                    }
                }
                // (a result for a cell that is not in the file has nowhere to go)
            }
        };
        if (!open.selfClosing) {
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
                // the cell: its bytes, and where it is
                qsizetype cellEnd = t.end;
                if (!t.selfClosing) {
                    // (the first closing tag of the cell that is a tag: a "</c>" in a CDATA section or a comment in it is not)
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
                                cellEnd = inner.end - closeTag.size();   // (kept as the end below)
                                break;
                            }
                        }
                    }
                    if (close < 0) return fail(QCoreApplication::translate("jp::office::sheet::Save", "The sheet's cells are cut short."));
                    cellEnd = close + closeTag.size();
                }
                const QByteArray cell = row.mid(t.start, cellEnd - t.start);
                pos = cellEnd;
                const QByteArray openTag = row.mid(t.start, t.end - t.start);
                quint32 col = quint32(prevCol + 1);
                bool haveR = false;
                const QByteArrayView r = xmlscan::attribute(openTag, "r", &haveR);
                if (haveR) {
                    quint32 rr = 0, cc = 0;
                    if (xmlscan::parseAddress(r, &rr, &cc)) col = cc;
                }
                prevCol = col;
                writeNewUntil(col, false);
                const Item *item = nullptr;
                if (e < n && edits->items[e].col == col) item = &edits->items[e++];
                if (item && item->cell) {
                    noteSharedMaster(cell, rowIdx, col, true);
                    const QByteArray x = cellXml(m_prefix, *item->cell, m_hasShared);
                    if (!x.isEmpty()) {
                        body += x;
                        widen(col);
                    }
                } else if (item && item->result) {
                    // (a cell that shares the formula of a cell that was changed has its own formula written out, and then its result)
                    const bool shared = !m_dropped.isEmpty() && rowIdx <= m_sharedUntil;
                    body += resultCell(shared ? sharedChild(cell, rowIdx, col) : cell, openTag, t.selfClosing, *item->result);
                } else if (!m_dropped.isEmpty() && rowIdx <= m_sharedUntil) {
                    body += sharedChild(cell, rowIdx, col);
                } else {
                    body += cell;
                }
            }
        } else {
            rest = "</" + m_prefix + "row>";
        }
        writeNewUntil(0xFFFFFFFFu, true);
        // the row's tag, its spans widened to the cells put in
        QByteArray openTag = row.left(open.end);
        if (edits && edits->props) applyRowProps(&openTag, *edits->props);
        if (open.selfClosing) {
            openTag.chop(2);
            while (!openTag.isEmpty() && (openTag.back() == ' ' || openTag.back() == '\t' || openTag.back() == '\n' || openTag.back() == '\r')) openTag.chop(1);
            if (body.isEmpty()) {
                *out = openTag + "/>";
                return true;
            }
            openTag += ">";
        }
        if (minCol != 0xFFFFFFFFu) {
            qsizetype vs = 0, ve = 0;
            if (xmlscan::attributeValue(openTag, "spans", &vs, &ve)) {
                const QList<QByteArray> parts = openTag.mid(vs, ve - vs).split(':');
                bool ok1 = false, ok2 = false;
                const int a = parts.size() == 2 ? parts[0].toInt(&ok1) : 0, b = parts.size() == 2 ? parts[1].toInt(&ok2) : 0;
                if (ok1 && ok2) {
                    const int na = std::min<int>(a, int(minCol) + 1), nb = std::max<int>(b, int(maxCol) + 1);
                    if (na != a || nb != b) openTag.replace(vs, ve - vs, QByteArray::number(na) + ":" + QByteArray::number(nb));
                }
            }
        }
        *out = openTag + body + rest;
        return true;
    }

    static bool isSpace(char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }

    // Takes an attribute, and the white space before it, out of a tag's bytes.
    static void removeAttribute(QByteArray *tag, const char *name)
    {
        qsizetype vs = 0, ve = 0;
        if (!xmlscan::attributeValue(*tag, name, &vs, &ve)) return;
        qsizetype n = vs - 2;   // (before the quote, the "=")
        while (n >= 0 && isSpace((*tag)[n])) --n;
        // n is the "=" or the white space before it; the name ends before it
        if (n >= 0 && (*tag)[n] == '=') --n;
        while (n >= 0 && isSpace((*tag)[n])) --n;
        qsizetype nameStart = n;
        while (nameStart > 0 && !isSpace((*tag)[nameStart - 1])) --nameStart;
        qsizetype a = nameStart;
        while (a > 0 && isSpace((*tag)[a - 1])) --a;
        tag->remove(a, ve + 1 - a);
    }

    // A formula cell with another result: its tag with the new type, its formula as it was, and the new value.
    QByteArray resultCell(const QByteArray &cell, const QByteArray &openTag, bool selfClosing, const SavedResult &r)
    {
        QByteArray tag = openTag;
        tag.chop(selfClosing ? 2 : 1);
        removeAttribute(&tag, "t");
        while (!tag.isEmpty() && (tag.back() == ' ' || tag.back() == '\t' || tag.back() == '\n' || tag.back() == '\r')) tag.chop(1);
        QByteArray formula;
        if (!selfClosing) {
            qsizetype fs = 0, fe = 0;
            const qsizetype from = openTag.size();
            if (findChild(QByteArrayView(cell), from, "f", &fs, &fe)) formula = cell.mid(fs, fe - fs);
        }
        const ValueXml v = valueXml(r.value, true, m_hasShared, m_prefix);
        return tag + v.type + ">" + formula + v.content + "</" + m_prefix + "c>";
    }

    // If the cell (being replaced) holds the text of a shared formula, the cells that share it are to get theirs written out.
    void noteSharedMaster(const QByteArray &cell, quint32 row, quint32 col, bool)
    {
        if (cell.indexOf("shared") < 0) return;
        qsizetype fs = 0, fe = 0;
        Tag open;
        Tag t;
        if (!xmlscan::nextTag(cell, 0, &t)) return;
        if (!findChild(QByteArrayView(cell), t.end, "f", &fs, &fe)) return;
        const QByteArray f = cell.mid(fs, fe - fs);
        if (!xmlscan::nextTag(f, 0, &open)) return;
        const QByteArray fTag = f.left(open.end);
        bool found = false;
        const QByteArrayView type = xmlscan::attribute(fTag, "t", &found);
        if (!found || type != "shared" || open.selfClosing) return;
        const QByteArrayView si = xmlscan::attribute(fTag, "si", &found);
        if (!found) return;
        const qsizetype close = f.lastIndexOf('<');
        const QString text = xmlscan::unescape(QByteArrayView(f).sliced(open.end, close - open.end));
        if (text.isEmpty()) return;
        SharedMaster m;
        m.row = row;
        m.col = col;
        m.text = text;
        m_dropped.insert(QByteArray(si), m);
        const QByteArrayView ref = xmlscan::attribute(fTag, "ref", &found);
        CellRange range;
        if (found && parseRangeRef(QString::fromLatin1(ref.data(), ref.size()), &range)) m_sharedUntil = std::max(m_sharedUntil, range.row2);
        else m_sharedUntil = std::max(m_sharedUntil, row);
    }

    // A cell that shares a formula whose first cell was changed: its formula written out.
    QByteArray sharedChild(const QByteArray &cell, quint32 row, quint32 col)
    {
        if (cell.indexOf("shared") < 0) return cell;
        Tag t;
        if (!xmlscan::nextTag(cell, 0, &t) || t.selfClosing) return cell;
        qsizetype fs = 0, fe = 0;
        if (!findChild(QByteArrayView(cell), t.end, "f", &fs, &fe)) return cell;
        const QByteArray f = cell.mid(fs, fe - fs);
        Tag open;
        if (!xmlscan::nextTag(f, 0, &open)) return cell;
        const QByteArray fTag = f.left(open.end);
        bool found = false;
        const QByteArrayView type = xmlscan::attribute(fTag, "t", &found);
        if (!found || type != "shared") return cell;
        const QByteArrayView si = xmlscan::attribute(fTag, "si", &found);
        if (!found) return cell;
        const auto master = m_dropped.constFind(QByteArray(si));
        if (master == m_dropped.constEnd()) return cell;
        if (!open.selfClosing) {
            const qsizetype close = f.lastIndexOf('<');
            if (close > open.end) return cell;   // (it has its own text: it is not a child)
        }
        const QString text = shiftFormula(master->text, int(row) - int(master->row), int(col) - int(master->col));
        const QByteArray explicitF = "<" + m_prefix + "f>" + xmlscan::escapeText(text) + "</" + m_prefix + "f>";
        QByteArray out = cell;
        out.replace(fs, fe - fs, explicitF);
        return out;
    }

    // ---- what follows the rows
    bool tail()
    {
        if (m_merges || m_page || !m_drawingId.isEmpty()) {
            // (what follows the cells is held whole to change its merged ranges, its page setup, and its drawing)
            while (fill()) {
                if (cancelled()) return fail(QCoreApplication::translate("jp::office::sheet::Save", "The save was stopped."));
            }
            if (m_failed) return fail(QCoreApplication::translate("jp::office::sheet::Save", "The sheet's part is damaged."));
            m_cur = m_written;
            const QByteArray rest = m_buf.mid(m_written);
            Tag close;
            auto tailOf = [&](const QByteArray &rest) {
                QByteArray tail = m_merges ? tailWithMerges(rest, *m_merges, m_prefix) : rest;
                if (m_page) tail = patchPageSetupTail(tail, *m_page, m_prefix);
                if (!m_drawingId.isEmpty()) tail = tailWithDrawing(tail, m_drawingId, m_prefix);
                return tail;
            };
            if (m_dataClosed) put(tailOf(rest));   // (an empty sheetData was given its closing tag with its rows: what is left follows it)
            else if (xmlscan::nextTag(rest, 0, &close) && close.closing && close.local == "sheetData") put(rest.left(close.end) + tailOf(rest.mid(close.end)));
            else put(rest);
            m_buf.clear();
            m_cur = m_written = 0;
            return true;
        }
        for (;;) {
            m_cur = m_buf.size();
            flushPending();
            m_buf.clear();
            m_cur = m_written = 0;
            if (cancelled()) return fail(QCoreApplication::translate("jp::office::sheet::Save", "The save was stopped."));
            if (!fill()) break;
        }
        if (m_failed) return fail(QCoreApplication::translate("jp::office::sheet::Save", "The sheet's part is damaged."));
        return true;
    }

    QIODevice *m_in;
    PartSink *m_out;
    bool m_hasShared;
    const std::atomic<bool> *m_cancel;
    QString *m_error = nullptr;
    std::vector<RowEdits> m_rows;
    std::unordered_map<quint32, size_t> m_rowAt;
    std::vector<const SavedCell *> m_cells;
    QByteArray m_buf;
    qsizetype m_written = 0;   // [0, m_written) has been written, or dropped
    qsizetype m_cur = 0;       // [m_written, m_cur) is passed on as it is, and not yet written
    bool m_eof = false, m_failed = false, m_afterData = false, m_dataClosed = false;
    QByteArray m_prefix;
    QHash<QByteArray, SharedMaster> m_dropped;
    quint32 m_sharedUntil = 0;
    const std::vector<ColumnSpan> *m_columns = nullptr;
    const std::vector<CellRange> *m_merges = nullptr;
    const PageSetup *m_page = nullptr;
    QByteArray m_drawingId;
    const SheetFormat *m_format = nullptr;
    const ColorSpec *m_tabColor = nullptr;
    int m_tabSelected = -1;
    int m_viewKind = -1, m_viewZoom = 0;
};

} // namespace

bool patchSheet(QIODevice *in, PartSink *out, const SheetChanges &changes, bool hasSharedStrings, QString *error, const std::atomic<bool> *cancel)
{
    if (error) error->clear();
    QString why;
    Patcher p(in, out, changes, hasSharedStrings, cancel);
    if (p.run(&why)) return true;
    if (error) *error = why.isEmpty() ? QCoreApplication::translate("jp::office::sheet::Save", "The sheet could not be saved.") : why;
    return false;
}

} // namespace jp::office::sheet
