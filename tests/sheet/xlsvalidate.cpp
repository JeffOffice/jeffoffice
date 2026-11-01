#include "xlsvalidate.h"

#include "io/cfb.h"
#include "sheet/xlsfunctions.h"

#include <QHash>
#include <QSet>
#include <QVector>

#include <map>

using jp::office::sheet::xlsFunction;

namespace xlstest {

namespace {

// The records whose length is always the same in the files of a real program (those that 3,000 workbooks written by it show, 5 times or more, all of one length): a record of one of
// these ids in a copy has that length. (A PANE of 9 bytes was in the copies of two workbooks that the program refused to open.)
const QHash<int, int> &fixedLengths()
{
    static const QHash<int, int> t = [] {
        const int rows[][2] = {{0x0000, 0}, {0x000A, 0}, {0x000C, 2}, {0x000D, 2}, {0x000E, 2}, {0x000F, 2}, {0x0010, 8}, {0x0011, 2}, {0x0012, 2}, {0x0013, 2}, {0x0019, 2}, {0x0022, 2}, {0x0026, 8},
                               {0x0027, 8}, {0x0028, 8}, {0x0029, 8}, {0x002A, 2}, {0x002B, 2}, {0x0033, 2}, {0x003D, 18}, {0x0040, 2}, {0x0041, 10}, {0x0042, 2}, {0x0050, 8}, {0x0055, 2}, {0x0059, 4},
                               {0x005E, 2}, {0x005F, 2}, {0x0063, 2}, {0x0080, 8}, {0x0081, 2}, {0x0082, 2}, {0x0083, 2}, {0x0084, 2}, {0x008C, 4}, {0x008D, 2}, {0x0092, 226}, {0x0097, 4}, {0x0099, 2},
                               {0x009C, 2}, {0x009D, 2}, {0x00A0, 4}, {0x00A1, 34}, {0x00AE, 8}, {0x00B0, 61}, {0x00B1, 10}, {0x00B2, 8}, {0x00C1, 2}, {0x00D3, 0}, {0x00D5, 2}, {0x00DA, 2}, {0x00DC, 12},
                               {0x00DD, 2}, {0x00E0, 20}, {0x00E1, 2}, {0x00E2, 0}, {0x00E3, 2}, {0x00F0, 8}, {0x00F1, 24}, {0x00F2, 8}, {0x00F5, 2}, {0x00FB, 4}, {0x00FD, 10}, {0x0160, 2}, {0x0161, 2},
                               {0x01AA, 64}, {0x01AB, 2}, {0x01AF, 2}, {0x01B2, 18}, {0x01B7, 2}, {0x01BC, 2}, {0x01BD, 0}, {0x01C0, 0}, {0x01C1, 8}, {0x0200, 14}, {0x0201, 6}, {0x0203, 14}, {0x0205, 8},
                               {0x0208, 16}, {0x0225, 4}, {0x027E, 10}, {0x0862, 20}};
        QHash<int, int> h;
        for (const auto &r : rows) h.insert(r[0], r[1]);
        return h;
    }();
    return t;
}

struct Rec {
    qsizetype at;
    int id;
    QByteArray data;
};

int u16(const QByteArray &b, int at) { return at + 2 <= b.size() ? quint8(b[at]) | (quint8(b[at + 1]) << 8) : -1; }
quint32 u32(const QByteArray &b, int at) { return at + 4 <= b.size() ? quint32(u16(b, at)) | (quint32(u16(b, at + 2)) << 16) : 0; }

QString at(const Rec &r, const QString &what) { return QStringLiteral("record %1 at %2: %3").arg(r.id, 4, 16, QLatin1Char('0')).arg(r.at).arg(what); }

// The string table: every string parses, in the records it is spread over, to the count the table gives.
bool checkStringTable(const QList<QByteArray> &chunks, int *unique, QString *why)
{
    if (chunks.isEmpty() || chunks[0].size() < 8) {
        *why = QStringLiteral("the string table is too short");
        return false;
    }
    const int total = int(u32(chunks[0], 0));
    *unique = int(u32(chunks[0], 4));
    int ci = 0;
    int pos = 8;
    int seen = 0;
    auto need = [&](int n) {
        if (pos + n > chunks[ci].size()) return false;
        pos += n;
        return true;
    };
    Q_UNUSED(total);
    while (seen < *unique) {
        if (pos == chunks[ci].size()) {
            if (++ci >= chunks.size()) {
                *why = QStringLiteral("the string table ends before its %1 strings (%2 found)").arg(*unique).arg(seen);
                return false;
            }
            pos = 0;
        }
        const int start = pos;
        if (!need(3)) {
            *why = QStringLiteral("a string of the table begins at the end of a record (string %1)").arg(seen);
            return false;
        }
        const int cch = u16(chunks[ci], start), flags = quint8(chunks[ci][start + 2]);
        int runs = 0;
        quint32 ext = 0;
        if (flags & 8) {
            runs = u16(chunks[ci], pos);
            if (!need(2)) return false;
        }
        if (flags & 4) {
            ext = u32(chunks[ci], pos);
            if (!need(4)) return false;
        }
        bool wide = flags & 1;
        int remain = cch;
        while (remain > 0) {
            if (pos == chunks[ci].size()) {
                if (++ci >= chunks.size()) {
                    *why = QStringLiteral("a string of the table is cut off by the end of the table (string %1)").arg(seen);
                    return false;
                }
                wide = quint8(chunks[ci][0]) & 1;
                pos = 1;
            }
            const int room = (chunks[ci].size() - pos) / (wide ? 2 : 1);
            const int take = std::min(room, remain);
            if (take <= 0) {
                *why = QStringLiteral("a string of the table has a character cut in two (string %1)").arg(seen);
                return false;
            }
            pos += take * (wide ? 2 : 1);
            remain -= take;
        }
        pos += runs * 4 + int(ext);
        if (pos > chunks[ci].size()) {
            *why = QStringLiteral("the formatting runs of a string go past its record (string %1)").arg(seen);
            return false;
        }
        ++seen;
    }
    return true;
}

} // namespace

bool validateTokens(const QByteArray &rg, int externalSheets, int names, QString *why)
{
    int i = 0, stack = 0;
    QVector<int> classes;   // the class of the reference each operand on the stack is (0 for any other operand)
    bool memory = false;
    const int n = int(rg.size());
    auto b = [&](int k) { return k < n ? quint8(rg[k]) : 0; };
    while (i < n) {
        const int p = b(i);
        const int base = p >= 0x20 ? ((p & 0x1F) | 0x20) : p;
        int len = 0, pop = 0, push = 0;
        if (p == 0x01 || p == 0x02) { len = 5; push = 1; }
        else if (p >= 0x03 && p <= 0x11) { len = 1; pop = 2; push = 1; }
        else if (p >= 0x12 && p <= 0x14) { len = 1; pop = 1; push = 1; }
        else if (p == 0x15) len = 1;
        else if (p == 0x16) { len = 1; push = 1; }
        else if (p == 0x17) {
            if (i + 3 > n) { *why = QStringLiteral("a string token is cut off"); return false; }
            len = 3 + b(i + 1) * ((b(i + 2) & 1) ? 2 : 1);
            push = 1;
        } else if (p == 0x19) {
            const int t = b(i + 1);
            len = t == 0x04 ? 4 + 2 * (u16(rg, i + 2) + 1) : 4;
            if (t == 0x10) { pop = 1; push = 1; }
        } else if (p == 0x1C || p == 0x1D) { len = 2; push = 1; }
        else if (p == 0x1E) { len = 3; push = 1; }
        else if (p == 0x1F) { len = 9; push = 1; }
        else if (base == 0x20) { len = 8; push = 1; }
        else if (base == 0x21) {
            const int f = u16(rg, i + 1);
            const int fixed = xlsFunction(f).fixedArgs;
            if (fixed < 0) { *why = QStringLiteral("function %1 is called without a count of its arguments").arg(f); return false; }
            len = 3; pop = fixed; push = 1;
        } else if (base == 0x22) { len = 4; pop = b(i + 1) & 0x7F; push = 1; }
        else if (base == 0x23) {
            len = 5; push = 1;
            const int x = u16(rg, i + 1);
            if (names >= 0 && (x < 1 || x > names)) { *why = QStringLiteral("a name token names the name %1 of %2").arg(x).arg(names); return false; }
        } else if (base == 0x24 || base == 0x2A || base == 0x2C) { len = 5; push = 1; }
        else if (base == 0x25 || base == 0x2B || base == 0x2D) { len = 9; push = 1; }
        else if (base == 0x26 || base == 0x27 || base == 0x28) { len = 7; memory = true; }
        else if (base == 0x29) { len = 3; memory = true; }
        else if (base == 0x39 || base == 0x3A || base == 0x3C) {
            len = 7; push = 1;
            const int x = u16(rg, i + 1);
            if (base != 0x39 && externalSheets >= 0 && x >= externalSheets) { *why = QStringLiteral("a reference names the external sheet %1 of %2").arg(x).arg(externalSheets); return false; }
        } else if (base == 0x3B || base == 0x3D) {
            len = 11; push = 1;
            const int x = u16(rg, i + 1);
            if (externalSheets >= 0 && x >= externalSheets) { *why = QStringLiteral("a reference names the external sheet %1 of %2").arg(x).arg(externalSheets); return false; }
        } else {
            *why = QStringLiteral("a token (%1) is not one the format has").arg(p, 2, 16, QLatin1Char('0'));
            return false;
        }
        if (i + len > n) { *why = QStringLiteral("a token (%1) runs past the end of the formula").arg(p, 2, 16, QLatin1Char('0')); return false; }
        if (!memory && stack < pop) { *why = QStringLiteral("an operator (%1) has too few operands").arg(p, 2, 16, QLatin1Char('0')); return false; }
        // a call of an add-in function (the call with function 255) is given its references as references: of the value class, the program the format is named for works a range out as #VALUE!
        if (base == 0x22 && u16(rg, i + 2) == 255 && !memory) {
            for (int k = 0; k < pop - 1 && k < classes.size(); ++k) {
                const int c = classes[classes.size() - 1 - k];
                if (c == 0x40 || c == 0x60) { *why = QStringLiteral("an add-in function is given a reference of the value class (%1)").arg(c, 2, 16); return false; }
            }
        }
        for (int k = 0; k < pop && !classes.isEmpty(); ++k) classes.removeLast();
        for (int k = 0; k < push; ++k) classes.append((base == 0x23 || base == 0x24 || base == 0x25 || base == 0x2C || base == 0x2D || base == 0x3A || base == 0x3B) && p >= 0x20 ? (p & 0x60) : 0);
        stack = stack - pop + push;
        i += len;
    }
    if (!memory && n > 0 && stack != 1 && !(rg.size() > 0 && (quint8(rg[0]) == 0x01 || quint8(rg[0]) == 0x02))) {
        *why = QStringLiteral("the formula leaves %1 values, not one").arg(stack);
        return false;
    }
    return true;
}

QString validateXls(const QByteArray &file)
{
    jp::cfb::File cf;
    QString why;
    if (!jp::cfb::read(file, &cf, &why)) return QStringLiteral("not a compound file: ") + why;
    const QByteArray d = cf.stream(QStringLiteral("Workbook"));
    if (d.isEmpty()) return QStringLiteral("no Workbook stream");
    if (d.size() < 4096) return QStringLiteral("the Workbook stream is under 4,096 bytes (%1)").arg(d.size());
    QList<Rec> recs;
    qsizetype pos = 0;
    while (pos + 4 <= d.size()) {
        const int id = quint8(d[pos]) | (quint8(d[pos + 1]) << 8), len = quint8(d[pos + 2]) | (quint8(d[pos + 3]) << 8);
        if (id == 0 && len == 0) {   // (the zeros that fill the stream out)
            bool zeros = true;
            for (qsizetype k = pos; k < d.size(); ++k)
                if (d[k] != 0) zeros = false;
            if (zeros) break;
        }
        if (len > 8224) return QStringLiteral("record %1 at %2 is %3 bytes (the most is 8,224)").arg(id, 4, 16, QLatin1Char('0')).arg(pos).arg(len);
        if (pos + 4 + len > d.size()) return QStringLiteral("record %1 at %2 runs past the end of the stream").arg(id, 4, 16, QLatin1Char('0')).arg(pos);
        recs.append({pos, id, d.mid(pos + 4, len)});
        pos += 4 + len;
    }
    // the sizes records always have
    for (const Rec &r : recs) {
        const auto fixed = fixedLengths().constFind(r.id);
        if (fixed != fixedLengths().cend() && fixed.value() != r.data.size()) return at(r, QStringLiteral("is %1 bytes, and the program that writes the format makes it %2").arg(r.data.size()).arg(fixed.value()));
    }
    // streams: each begins with a BOF and ends with an EOF; the sheets are where the list of sheets says
    QSet<qsizetype> bofs;
    int depth = 0;
    for (const Rec &r : recs) {
        if (r.id == 0x0809) {
            if (depth == 0) bofs.insert(r.at);
            ++depth;
        } else if (r.id == 0x000A) {
            --depth;
            if (depth < 0) return at(r, QStringLiteral("an EOF with no BOF"));
        }
    }
    if (depth != 0) return QStringLiteral("the BOF and EOF records do not pair (%1 left open)").arg(depth);
    int sheets = 0, xfs = 0, fonts = 0, externalSheets = -1, names = 0, uniqueStrings = -1;
    QSet<int> formats;
    QList<QByteArray> sstChunks;
    for (int k = 0; k < recs.size(); ++k) {
        const Rec &r = recs[k];
        if (r.id == 0x0085) {
            ++sheets;
            if (!bofs.contains(qsizetype(u32(r.data, 0)))) return at(r, QStringLiteral("a sheet's offset %1 is not where a BOF is").arg(u32(r.data, 0)));
        } else if (r.id == 0x00E0) ++xfs;
        else if (r.id == 0x0031) ++fonts;
        else if (r.id == 0x041E) formats.insert(u16(r.data, 0));
        else if (r.id == 0x0017) externalSheets = u16(r.data, 0);
        else if (r.id == 0x0018) ++names;
        else if (r.id == 0x00FC) {
            sstChunks << r.data;
            for (int j = k + 1; j < recs.size() && recs[j].id == 0x003C; ++j) sstChunks << recs[j].data;
        }
    }
    if (sheets == 0) return QStringLiteral("no sheet in the list of sheets");
    if (bofs.size() != sheets + 1) return QStringLiteral("%1 sheets in the list, %2 streams").arg(sheets).arg(bofs.size() - 1);
    for (int k = 0; k < recs.size(); ++k) {
        const Rec &r = recs[k];
        if (r.id == 0x003C) {
            int j = k - 1;
            while (j >= 0 && recs[j].id == 0x003C) --j;
            if (j < 0 || (recs[j].id != 0x00FC && recs[j].id != 0x0207)) return at(r, QStringLiteral("follows a record that may not be continued"));
        }
    }
    if (!sstChunks.isEmpty()) {
        QString problem;
        if (!checkStringTable(sstChunks, &uniqueStrings, &problem)) return problem;
    }
    for (const Rec &r : recs) {
        if (r.id == 0x00E0) {
            const int font = u16(r.data, 0), format = u16(r.data, 2);
            if (font == 4 || font > fonts) return at(r, QStringLiteral("names the font %1 of %2").arg(font).arg(fonts));
            if (format >= 164 && !formats.contains(format)) return at(r, QStringLiteral("names the number format %1, which has no record").arg(format));
        }
    }
    // the sheets
    int sheetNo = -1;
    bool inSheet = false;
    int panes = -1, selections = 0, activePane = -1;
    int lastColInfo = -1;
    QSet<int> rowsSeen;
    int prevRow = -1, prevCol = -1;
    int dimLastRow = -1, dimLastCol = -1, maxRow = -1, maxCol = -1;
    depth = 0;
    auto endOfSheet = [&](const Rec &r) -> QString {
        if (panes >= 0 && selections != panes) return at(r, QStringLiteral("a sheet that is split has %1 panes and %2 selections").arg(panes).arg(selections));
        if (maxRow > dimLastRow || maxCol > dimLastCol) return at(r, QStringLiteral("a cell lies outside the used area the sheet states"));
        return QString();
    };
    for (const Rec &r : recs) {
        if (r.id == 0x0809) {
            if (depth++ == 0 && r.at != 0) {
                ++sheetNo;
                inSheet = true;
                panes = -1;
                selections = 0;
                activePane = -1;
                lastColInfo = -1;
                rowsSeen.clear();
                prevRow = prevCol = -1;
                dimLastRow = dimLastCol = 1 << 20;
                maxRow = maxCol = -1;
            }
            continue;
        }
        if (r.id == 0x000A) {
            if (--depth == 0 && inSheet) {
                const QString problem = endOfSheet(r);
                if (!problem.isEmpty()) return problem;
                inSheet = false;
            }
            continue;
        }
        if (!inSheet || depth != 1) continue;
        switch (r.id) {
        case 0x0200:
            dimLastRow = int(u32(r.data, 4));
            dimLastCol = u16(r.data, 10);
            break;
        case 0x007D: {
            const int first = u16(r.data, 0), last = u16(r.data, 2);
            if (first <= lastColInfo || last < first || last > 255) return at(r, QStringLiteral("column widths overlap or are out of order (%1 to %2 after %3)").arg(first).arg(last).arg(lastColInfo));
            lastColInfo = last;
            break;
        }
        case 0x0208:
            rowsSeen.insert(u16(r.data, 0));
            prevRow = -1;
            break;
        case 0x0041:
            panes = 0;
            activePane = u16(r.data, 8);
            if (u16(r.data, 0) > 0 || u16(r.data, 2) > 0) panes = (u16(r.data, 0) > 0 && u16(r.data, 2) > 0) ? 4 : 2;
            break;
        case 0x001D:
            ++selections;
            break;
        case 0x00E5:
            if (u16(r.data, 0) > 1026 || r.data.size() != 2 + 8 * u16(r.data, 0)) return at(r, QStringLiteral("holds %1 merged ranges (1,026 at most, and its size must agree)").arg(u16(r.data, 0)));
            break;
        case 0x00FD: case 0x0201: case 0x0203: case 0x027E: case 0x0006: case 0x0205: case 0x00BE: {
            const int row = u16(r.data, 0), col = u16(r.data, 2);
            int last = col;
            if (r.id == 0x00BE) {
                last = u16(r.data, int(r.data.size()) - 2);
                if (r.data.size() != 6 + 2 * (last - col + 1) || last < col) return at(r, QStringLiteral("a run of blank cells whose size does not fit its columns"));
                for (int x = 4; x + 2 <= r.data.size() - 2; x += 2)
                    if (u16(r.data, x) >= xfs) return at(r, QStringLiteral("names a cell format that is not there"));
            } else if (u16(r.data, 4) >= xfs) {
                return at(r, QStringLiteral("names the cell format %1 of %2").arg(u16(r.data, 4)).arg(xfs));
            }
            if (!rowsSeen.contains(row)) return at(r, QStringLiteral("a cell in a row that has no ROW record"));
            if (col <= prevCol && row == prevRow) return at(r, QStringLiteral("cells are out of order or overlap"));
            prevRow = row;
            prevCol = last;
            maxRow = std::max(maxRow, row + 1);
            maxCol = std::max(maxCol, last + 1);
            if (r.id == 0x00FD && uniqueStrings >= 0 && int(u32(r.data, 6)) >= uniqueStrings) return at(r, QStringLiteral("names a string that is not in the table"));
            if (r.id == 0x0006) {
                const int cce = u16(r.data, 20);
                if (22 + cce > r.data.size()) return at(r, QStringLiteral("its tokens run past the record"));
                QString problem;
                if (!validateTokens(r.data.mid(22, cce), externalSheets, names, &problem)) return at(r, problem);
            }
            break;
        }
        case 0x0221: {
            const int cce = u16(r.data, 12);
            if (14 + cce > r.data.size()) return at(r, QStringLiteral("its tokens run past the record"));
            QString problem;
            if (!validateTokens(r.data.mid(14, cce), externalSheets, names, &problem)) return at(r, problem);
            break;
        }
        default: break;
        }
        Q_UNUSED(activePane);
    }
    // the names
    for (const Rec &r : recs) {
        if (r.id != 0x0018) continue;
        const int cch = quint8(r.data[3]), cce = u16(r.data, 4);
        const bool wide = cch > 0 && (quint8(r.data[14]) & 1);
        const int tokens = 15 + cch * (wide ? 2 : 1);
        if (tokens + cce > r.data.size()) return at(r, QStringLiteral("a name's tokens run past the record"));
        QString problem;
        if (cce > 0 && quint8(r.data[tokens]) == 0x1C) return at(r, QStringLiteral("a name's tokens are an error value (a name for a reference that is gone has the token for that)"));
        if (!validateTokens(r.data.mid(tokens, cce), externalSheets, names, &problem)) return at(r, QStringLiteral("a name: ") + problem);
    }
    return QString();
}

} // namespace xlstest
