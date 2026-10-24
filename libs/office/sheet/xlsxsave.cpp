// Saving a workbook as the old file with only what changed put in anew: see xlsxsave.h.

#include "sheet/xlsxsave.h"

#include "io/zip.h"
#include "io/zipindex.h"
#include "sheet/structsave.h"
#include "sheet/xlsxbase.h"
#include "sheet/xmlscan.h"

#include <QCoreApplication>
#include <QBuffer>
#include <QDir>
#include <QFileInfo>
#include <QHash>
#include <QSaveFile>
#include <QRegularExpression>
#include <QSet>

#include <algorithm>

namespace jp::office::sheet {

namespace {

using xmlscan::Tag;


bool isSpace(char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }

// A name the way two spellings of one entry compare equal: slashes, no leading slash, lower case.
QString plain(const QString &name)
{
    QString n = name;
    n.replace(QLatin1Char('\\'), QLatin1Char('/'));
    while (n.startsWith(QLatin1Char('/'))) n.remove(0, 1);
    return n.toLower();
}

// Sets an attribute in a tag's bytes (a start tag, `<xf ...>` or `<xf .../>`): its value replaced, or the attribute put in before the end of the tag.
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

// The first start tag of an element with the local name (anywhere); false if there is none.
bool findStart(const QByteArray &xml, QByteArrayView local, Tag *tag, qsizetype from = 0)
{
    qsizetype pos = from;
    while (xmlscan::nextTag(xml, pos, tag)) {
        pos = tag->end;
        if (!tag->closing && tag->local == local) return true;
    }
    return false;
}

// The close tag that matches a start tag of the same local name (no element of that name is nested in the ones this is used for); false if there is none.
bool findEnd(const QByteArray &xml, const Tag &start, Tag *close)
{
    qsizetype pos = start.end;
    while (xmlscan::nextTag(xml, pos, close)) {
        pos = close->end;
        if (close->closing && close->local == start.local) return true;
    }
    return false;
}

QByteArray prefixOfRoot(const QByteArray &xml)
{
    Tag t;
    return xmlscan::nextTag(xml, 0, &t) && !t.closing ? QByteArray(xmlscan::prefixOf(t.name)) : QByteArray();
}

void addToCount(QByteArray *tag, const char *name, qint64 more)
{
    qsizetype vs = 0, ve = 0;
    if (!xmlscan::attributeValue(*tag, name, &vs, &ve)) return;
    bool ok = false;
    const qint64 now = tag->mid(vs, ve - vs).toLongLong(&ok);
    if (ok) tag->replace(vs, ve - vs, QByteArray::number(now + more));
}

} // namespace

// ---- the shared strings

bool patchSharedStrings(const QByteArray &xml, const QStringList &added, QByteArray *out, QString *error)
{
    Tag root;
    if (!findStart(xml, "sst", &root)) {
        if (error) *error = QCoreApplication::translate("jp::office::sheet::Save", "The shared strings part is not in the form this program changes.");
        return false;
    }
    const QByteArray p(xmlscan::prefixOf(root.name));
    QByteArray items;
    for (const QString &s : added) {
        const bool preserve = !s.isEmpty() && (s.front().isSpace() || s.back().isSpace() || s.contains(QLatin1Char('\n')) || s.contains(QLatin1Char('\t')) || s.contains(QLatin1String("  ")));
        items += "<" + p + "si><" + p + "t" + (preserve ? " xml:space=\"preserve\"" : "") + ">" + xmlscan::escapeText(s) + "</" + p + "t></" + p + "si>";
    }
    QByteArray rootTag = xml.mid(root.start, root.end - root.start);
    addToCount(&rootTag, "uniqueCount", added.size());
    addToCount(&rootTag, "count", added.size());
    if (root.selfClosing) {
        rootTag.chop(2);
        while (!rootTag.isEmpty() && isSpace(rootTag.back())) rootTag.chop(1);
        *out = xml.left(root.start) + rootTag + ">" + items + "</" + p + "sst>" + xml.mid(root.end);
        return true;
    }
    const QByteArray closeTag = "</" + p + "sst>";
    const qsizetype close = xml.lastIndexOf(closeTag);
    if (close < root.end) {
        if (error) *error = QCoreApplication::translate("jp::office::sheet::Save", "The shared strings part is not in the form this program changes.");
        return false;
    }
    *out = xml.left(root.start) + rootTag + xml.mid(root.end, close - root.end) + items + xml.mid(close);
    return true;
}

// ---- the styles

bool patchStyles(const QByteArray &xml, const SaveChanges &changes, QByteArray *out, QString *error)
{
    auto bad = [&] {
        if (error) *error = QCoreApplication::translate("jp::office::sheet::Save", "The styles part is not in the form this program changes.");
        return false;
    };
    Tag root;
    if (!findStart(xml, "styleSheet", &root)) return bad();
    const QByteArray p(xmlscan::prefixOf(root.name));
    QByteArray result = xml;
    // the cell formats first (they are later in the part, so the places of the number formats' change do not move them)
    if (!changes.newCellFormats.isEmpty()) {
        Tag open;
        if (!findStart(result, "cellXfs", &open, root.end)) return bad();
        if (open.selfClosing) return bad();
        Tag close;
        if (!findEnd(result, open, &close)) return bad();
        // the xf elements the new ones are copies of
        QSet<quint32> needed;
        for (const SaveChanges::CellFormatAdded &f : changes.newCellFormats)
            if (qint64(f.base) < changes.cellFormatsBefore) needed.insert(f.base);
        QHash<quint32, QByteArray> raw;
        qsizetype pos = open.end;
        quint32 index = 0;
        Tag t;
        int depth = 0;
        while (xmlscan::nextTag(result, pos, &t) && t.start < close.start) {
            pos = t.end;
            if (t.closing) {
                --depth;
                continue;
            }
            if (depth == 0 && t.local == "xf") {
                qsizetype end = t.end;
                if (!t.selfClosing) {
                    Tag xfClose;
                    if (!findEnd(result, t, &xfClose)) return bad();
                    end = xfClose.end;
                }
                if (needed.contains(index)) raw.insert(index, result.mid(t.start, end - t.start));
                ++index;
                pos = end;
                continue;
            }
            if (!t.selfClosing) ++depth;
        }
        if (qint64(index) != changes.cellFormatsBefore) return bad();
        QByteArray added;
        QVector<QByteArray> made;
        for (const SaveChanges::CellFormatAdded &f : changes.newCellFormats) {
            QByteArray base;
            if (qint64(f.base) < changes.cellFormatsBefore) base = raw.value(f.base);
            else if (qint64(f.base) - changes.cellFormatsBefore < made.size()) base = made[int(qint64(f.base) - changes.cellFormatsBefore)];
            if (base.isEmpty()) return bad();
            Tag baseTag;
            if (!xmlscan::nextTag(base, 0, &baseTag)) return bad();
            QByteArray tag = base.left(baseTag.end);
            setAttribute(&tag, "numFmtId", QByteArray::number(f.numFmtId));
            setAttribute(&tag, "applyNumberFormat", "1");
            const QByteArray one = tag + base.mid(baseTag.end);
            made.append(one);
            added += one;
        }
        QByteArray openTag = result.mid(open.start, open.end - open.start);
        addToCount(&openTag, "count", changes.newCellFormats.size());
        result = result.left(open.start) + openTag + result.mid(open.end, close.start - open.end) + added + result.mid(close.start);
    }
    if (!changes.newNumberFormats.isEmpty()) {
        QByteArray added;
        for (const SaveChanges::NumberFormat &f : changes.newNumberFormats)
            added += "<" + p + "numFmt numFmtId=\"" + QByteArray::number(f.id) + "\" formatCode=\"" + xmlscan::escapeAttribute(f.code) + "\"/>";
        Tag open;
        if (findStart(result, "numFmts", &open, root.end)) {
            QByteArray openTag = result.mid(open.start, open.end - open.start);
            addToCount(&openTag, "count", changes.newNumberFormats.size());
            if (open.selfClosing) {
                openTag.chop(2);
                while (!openTag.isEmpty() && isSpace(openTag.back())) openTag.chop(1);
                result = result.left(open.start) + openTag + ">" + added + "</" + p + "numFmts>" + result.mid(open.end);
            } else {
                Tag close;
                if (!findEnd(result, open, &close)) return bad();
                result = result.left(open.start) + openTag + result.mid(open.end, close.start - open.end) + added + result.mid(close.start);
            }
        } else {
            const QByteArray element = "<" + p + "numFmts count=\"" + QByteArray::number(changes.newNumberFormats.size()) + "\">" + added + "</" + p + "numFmts>";
            Tag again;
            if (!findStart(result, "styleSheet", &again)) return bad();
            result.insert(again.end, element);
        }
    }
    *out = result;
    return true;
}

// ---- the package's own parts

bool removeContentTypeOverride(const QByteArray &xml, const QString &partName, QByteArray *out)
{
    Tag t;
    qsizetype pos = 0;
    const QString want = plain(partName);
    while (xmlscan::nextTag(xml, pos, &t)) {
        pos = t.end;
        if (t.closing || t.local != "Override") continue;
        bool found = false;
        const QByteArrayView name = xmlscan::attribute(QByteArrayView(xml).sliced(t.start, t.end - t.start), "PartName", &found);
        if (!found || plain(xmlscan::unescape(name)) != want) continue;
        qsizetype from = t.start;
        while (from > 0 && isSpace(xml[from - 1])) --from;
        *out = xml.left(from) + xml.mid(t.end);
        return true;
    }
    return false;
}

bool removeRelationshipOfType(const QByteArray &xml, const QString &typeSuffix, QByteArray *out)
{
    Tag t;
    qsizetype pos = 0;
    while (xmlscan::nextTag(xml, pos, &t)) {
        pos = t.end;
        if (t.closing || t.local != "Relationship") continue;
        bool found = false;
        const QByteArrayView type = xmlscan::attribute(QByteArrayView(xml).sliced(t.start, t.end - t.start), "Type", &found);
        if (!found || !xmlscan::unescape(type).endsWith(typeSuffix)) continue;
        qsizetype from = t.start;
        while (from > 0 && isSpace(xml[from - 1])) --from;
        *out = xml.left(from) + xml.mid(t.end);
        return true;
    }
    return false;
}

bool setFullCalcOnLoad(const QByteArray &xml, QByteArray *out)
{
    Tag calc;
    if (findStart(xml, "calcPr", &calc)) {
        QByteArray tag = xml.mid(calc.start, calc.end - calc.start);
        setAttribute(&tag, "fullCalcOnLoad", "1");
        *out = xml.left(calc.start) + tag + xml.mid(calc.end);
        return true;
    }
    // (the element goes after the last of the ones the format puts before it)
    const QByteArray p = prefixOfRoot(xml);
    qsizetype at = -1;
    Tag t;
    qsizetype pos = 0;
    while (xmlscan::nextTag(xml, pos, &t)) {
        pos = t.end;
        const bool before = t.local == "sheets" || t.local == "functionGroups" || t.local == "externalReferences" || t.local == "definedNames";
        if (before && (t.closing || t.selfClosing)) at = t.end;
    }
    if (at < 0) return false;
    *out = xml.left(at) + "<" + p + "calcPr fullCalcOnLoad=\"1\"/>" + xml.mid(at);
    return true;
}

// ---- the whole file

namespace {

class ZipSink : public PartSink {
public:
    explicit ZipSink(ZipWriter *zip) : m_zip(zip) {}
    void write(const char *data, qsizetype size) override { m_zip->write(data, size); }

private:
    ZipWriter *m_zip;
};

class BufferSink : public PartSink {
public:
    void write(const char *data, qsizetype size) override { bytes.append(data, size); }
    QByteArray bytes;
};

} // namespace

bool saveXlsx(const Workbook &book, const SaveChanges &changes, QByteArray *out, QString *error, SaveReport *report, const std::atomic<bool> *cancel)
{
    SaveReport local;
    SaveReport &rep = report ? *report : local;
    rep = SaveReport();
    QString why;
    auto fail = [&](const QString &text) {
        if (error) *error = text;
        return false;
    };
    // A workbook read from the older binary form has no package yet: the first save writes one from what was read (xlsxbase.h), and the changes are put in that.
    Package baseFile;
    const Package *base = &book.package();
    if (!base->isLazy() && book.isConverted()) {
        QByteArray bytes;
        if (!writeBaseXlsx(book, &bytes, &why, cancel)) return fail(why);
        if (!baseFile.readLazy(bytes, &why)) return fail(why);
        base = &baseFile;
    }
    const Package &package = *base;
    if (!package.isLazy()) return fail(QCoreApplication::translate("jp::office::sheet::Save", "The workbook was not opened from a file this program can save over."));
    const QByteArray bytes = package.fileBytes();
    QVector<ZipEntry> directory;
    if (!readZipDirectory(bytes, &directory)) return fail(QCoreApplication::translate("jp::office::sheet::Save", "The file this workbook was opened from is damaged."));
    QHash<QString, int> entryAt;   // by the plain name, the first entry of it
    for (int i = 0; i < directory.size(); ++i) {
        const QString key = plain(directory[i].name);
        if (!entryAt.contains(key)) entryAt.insert(key, i);
    }
    auto entryOfPart = [&](const QString &part) -> int {
        const QString name = package.entryName(part);
        return name.isEmpty() ? -1 : entryAt.value(plain(name), -1);
    };
    auto inflate = [&](int index, QByteArray *data) { return index >= 0 && inflateZipEntry(bytes, directory[index], data); };

    QHash<int, QByteArray> replaced;   // entries written anew from bytes in memory
    QHash<int, int> sheetOfEntry;      // entries streamed through the sheet patcher: the sheet
    QSet<int> dropped;

    // ---- the sheets
    bool anyCellChanged = false;
    for (const auto &[sheet, sc] : changes.sheets) {
        if (sc.empty()) continue;
        if (sheet < 0 || sheet >= book.info().sheets.size()) return fail(QCoreApplication::translate("jp::office::sheet::Save", "A change is to a sheet the workbook does not have."));
        const int index = entryOfPart(book.info().sheets[sheet].part);
        if (index < 0) return fail(QCoreApplication::translate("jp::office::sheet::Save", "A sheet that changed has no part in the file."));
        sheetOfEntry.insert(index, sheet);
        if (!sc.cells.empty()) anyCellChanged = true;
        rep.cellsChanged += qint64(sc.cells.size());
        rep.resultsChanged += qint64(sc.results.size());
    }
    // ---- rows, columns, and cells inserted and deleted: the sheets they move cells on are renumbered, and any sheet (and chart, drawing, note, name, ...) that refers to cells follows
    MoveContext ctx;
    ctx.edits = changes.edits;
    ctx.renames = changes.renames;
    for (const SheetEntry &se : book.info().sheets) ctx.sheetNames << se.name;
    const bool moves = !ctx.edits.empty() || !ctx.renames.empty();
    QSet<int> moveSheets;   // the sheets (by their place) whose part is made by the mover
    if (moves) {
        anyCellChanged = true;
        for (int sheet = 0; sheet < book.info().sheets.size(); ++sheet) {
            if (book.info().sheets[sheet].kind != SheetKind::Worksheet) continue;
            const int index = entryOfPart(book.info().sheets[sheet].part);
            if (index < 0) continue;
            if (cancel && cancel->load()) return fail(QCoreApplication::translate("jp::office::sheet::Save", "The save was stopped."));
            bool needed = movesCellsOf(ctx, sheet);
            if (!needed) {
                // (a sheet the edits do not move cells on may still have what refers to cells that moved: looked at, and left as it is if not)
                ZipEntryDevice device(bytes, directory[index]);
                if (!device.open(QIODevice::ReadOnly)) return fail(QCoreApplication::translate("jp::office::sheet::Save", "A sheet of the file could not be read to be saved."));
                if (!moveSheetPart(&device, nullptr, ctx, sheet, &needed, &why, cancel)) return fail(why);
            }
            if (!needed) continue;
            moveSheets.insert(sheet);
            sheetOfEntry.insert(index, sheet);
        }
        // the parts that name cells
        auto replacePart = [&](const QString &part, const std::function<bool(const QByteArray &, QByteArray *, bool *)> &fn) {
            const int index = entryOfPart(part);
            if (index < 0 || replaced.contains(index)) return;
            QByteArray xml, now;
            bool changed = false;
            if (!inflate(index, &xml) || !fn(xml, &now, &changed) || !changed) return;
            replaced.insert(index, now);
        };
        static const QRegularExpression chartPart(QStringLiteral("/charts/(chart|chartEx)\\d*\\.xml$")), cachePart(QStringLiteral("/pivotCacheDefinition\\d*\\.xml$"));
        for (const QString &part : package.partNames()) {
            if (chartPart.match(part).hasMatch()) replacePart(part, [&](const QByteArray &x, QByteArray *o, bool *c) { return moveChartPart(x, ctx, o, c); });
            else if (cachePart.match(part).hasMatch()) replacePart(part, [&](const QByteArray &x, QByteArray *o, bool *c) { return movePivotCachePart(x, ctx, o, c); });
        }
        for (int sheet = 0; sheet < book.info().sheets.size(); ++sheet) {
            if (book.info().sheets[sheet].kind != SheetKind::Worksheet) continue;
            const QString sheetPart = book.info().sheets[sheet].part;
            for (const Relationship &r : package.relationshipsOfType(sheetPart, QStringLiteral("/drawing")))
                if (!r.external) replacePart(r.target, [&](const QByteArray &x, QByteArray *o, bool *c) { return moveDrawingPart(x, ctx, sheet, o, c); });
            for (const Relationship &r : package.relationshipsOfType(sheetPart, QStringLiteral("/comments")))
                if (!r.external) replacePart(r.target, [&](const QByteArray &x, QByteArray *o, bool *c) { return moveCommentsPart(x, ctx, sheet, o, c); });
            for (const Relationship &r : package.relationshipsOfType(sheetPart, QStringLiteral("/threadedComment")))
                if (!r.external) replacePart(r.target, [&](const QByteArray &x, QByteArray *o, bool *c) { return moveThreadedCommentsPart(x, ctx, sheet, o, c); });
            for (const Relationship &r : package.relationshipsOfType(sheetPart, QStringLiteral("/ctrlProp")))
                if (!r.external) replacePart(r.target, [&](const QByteArray &x, QByteArray *o, bool *c) { return moveControlPart(x, ctx, sheet, o, c); });
            for (const Relationship &r : package.relationshipsOfType(sheetPart, QStringLiteral("/vmlDrawing")))
                if (!r.external) replacePart(r.target, [&](const QByteArray &x, QByteArray *o, bool *c) { return moveNotesDrawing(x, ctx, sheet, o, c); });
        }
    }
    // ---- the shared strings
    if (!changes.newSharedStrings.isEmpty()) {
        const int index = entryOfPart(book.sharedStringsPart());
        QByteArray xml, patched;
        if (index < 0 || !inflate(index, &xml)) return fail(QCoreApplication::translate("jp::office::sheet::Save", "The shared strings part is not in the file."));
        if (!patchSharedStrings(xml, changes.newSharedStrings, &patched, &why)) return fail(why);
        replaced.insert(index, patched);
    }
    // ---- the styles
    if (!changes.newCellFormats.isEmpty() || !changes.newNumberFormats.isEmpty()) {
        const int index = entryOfPart(book.stylesPart());
        QByteArray xml, patched;
        if (index < 0 || !inflate(index, &xml)) return fail(QCoreApplication::translate("jp::office::sheet::Save", "The styles part is not in the file."));
        if (!patchStyles(xml, changes, &patched, &why)) return fail(why);
        replaced.insert(index, patched);
    }
    // ---- the table parts whose columns were renamed
    for (const auto &[part, data] : changes.replacedParts) {
        const int index = entryOfPart(part);
        if (index < 0) return fail(QCoreApplication::translate("jp::office::sheet::Save", "A table that changed has no part in the file."));
        replaced.insert(index, data);
    }
    // ---- the calculation chain: left out when a cell changed (it may name a cell that no longer holds a formula)
    if (anyCellChanged) {
        const QVector<Relationship> chains = package.relationshipsOfType(book.mainPart(), QStringLiteral("/calcChain"));
        if (!chains.isEmpty() && !chains.first().external) {
            const QString part = chains.first().target;
            const int chainEntry = entryOfPart(part);
            // the three places that name it (the part, its type in [Content_Types].xml, and the workbook's link to it) all go, or none does
            int typesEntry = -1;
            for (int i = 0; i < directory.size(); ++i)
                if (plain(directory[i].name) == QLatin1String("[content_types].xml")) {
                    typesEntry = i;
                    break;
                }
            const QString mainEntryName = package.entryName(book.mainPart());
            const int slash = mainEntryName.lastIndexOf(QLatin1Char('/'));
            const QString relsName = (slash < 0 ? QString() : mainEntryName.left(slash + 1)) + QStringLiteral("_rels/") + (slash < 0 ? mainEntryName : mainEntryName.mid(slash + 1)) + QStringLiteral(".rels");
            const int relsEntry = entryAt.value(plain(relsName), -1);
            QByteArray types, rels, newTypes, newRels;
            if (chainEntry >= 0 && typesEntry >= 0 && relsEntry >= 0 && inflate(typesEntry, &types) && inflate(relsEntry, &rels) && removeContentTypeOverride(types, part, &newTypes)
                && removeRelationshipOfType(rels, QStringLiteral("/calcChain"), &newRels)) {
                replaced.insert(typesEntry, newTypes);
                replaced.insert(relsEntry, newRels);
                dropped.insert(chainEntry);
                rep.calcChainDropped = true;
            }
        }
    }
    // ---- the workbook part: the names follow the cells, and the other program is asked to work the formulas out again when it is to
    if (changes.fullCalcOnLoad || moves) {
        const int index = entryOfPart(book.mainPart());
        QByteArray xml, patched;
        if (index >= 0 && inflate(index, &xml)) {
            bool changed = false;
            if (moves && moveWorkbookPart(xml, ctx, &patched, &changed) && changed) xml = patched;
            if (changes.fullCalcOnLoad && setFullCalcOnLoad(xml, &patched)) {
                xml = patched;
                changed = true;
            }
            if (changed) replaced.insert(index, xml);
        }
    }

    // ---- the new ZIP, entry by entry in the old order
    ZipWriter zip;
    ZipSink sink(&zip);
    QSet<QString> written;
    const bool hasShared = !book.sharedStringsPart().isEmpty();
    for (int i = 0; i < directory.size(); ++i) {
        const ZipEntry &e = directory[i];
        if (cancel && cancel->load()) return fail(QCoreApplication::translate("jp::office::sheet::Save", "The save was stopped."));
        const QString key = plain(e.name);
        if (entryAt.value(key) != i && written.contains(key)) continue;   // (of two entries of one name the first stands)
        if (dropped.contains(i)) continue;
        written.insert(key);
        if (replaced.contains(i)) {
            zip.add(e.name, replaced.value(i), true);
            rep.changedParts << e.name;
            ++rep.partsChanged;
        } else if (sheetOfEntry.contains(i)) {
            const int sheet = sheetOfEntry.value(i);
            const auto changed = changes.sheets.find(sheet);
            const bool patch = changed != changes.sheets.end() && !changed->second.empty();
            const bool mover = moveSheets.contains(sheet);
            zip.beginStream(e.name, true);
            ZipEntryDevice device(bytes, e);
            bool ok = device.open(QIODevice::ReadOnly);
            if (ok && mover && patch) {
                // (the file's cells are put in the places the edits leave them, and then the cells that changed are put in)
                BufferSink moved;
                ok = moveSheetPart(&device, &moved, ctx, sheet, nullptr, &why, cancel);
                QBuffer in(&moved.bytes);
                ok = ok && in.open(QIODevice::ReadOnly) && patchSheet(&in, &sink, changed->second, hasShared, &why, cancel);
            } else if (ok && mover) {
                ok = moveSheetPart(&device, &sink, ctx, sheet, nullptr, &why, cancel);
            } else if (ok) {
                ok = patchSheet(&device, &sink, changes.sheets.at(sheet), hasShared, &why, cancel);
            }
            if (!ok) {
                zip.endStream();
                return fail(why.isEmpty() ? QCoreApplication::translate("jp::office::sheet::Save", "A sheet of the file could not be read to be saved.") : why);
            }
            zip.endStream();
            rep.changedParts << e.name;
            ++rep.partsChanged;
            ++rep.sheetsRewritten;
        } else {
            if (e.dataAt < 0 || e.dataAt + e.csize > bytes.size()) return fail(QCoreApplication::translate("jp::office::sheet::Save", "The file this workbook was opened from is damaged."));
            zip.addRaw(e.name, e.method, e.crc, quint32(e.csize), quint32(e.usize), bytes.constData() + e.dataAt);
            ++rep.partsCopied;
        }
    }
    *out = zip.finish();
    return true;
}

bool writeFileAtomically(const QString &path, const QByteArray &bytes, QString *error)
{
    QSaveFile file(path);
    file.setDirectWriteFallback(false);
    if (!file.open(QIODevice::WriteOnly)) {
        if (error) *error = QCoreApplication::translate("jp::office::sheet::Save", "The file couldn't be written: %1").arg(file.errorString());
        return false;
    }
    if (file.write(bytes) != bytes.size() || !file.commit()) {
        if (error) *error = QCoreApplication::translate("jp::office::sheet::Save", "The file couldn't be written: %1").arg(file.errorString());
        return false;
    }
    return true;
}

} // namespace jp::office::sheet
