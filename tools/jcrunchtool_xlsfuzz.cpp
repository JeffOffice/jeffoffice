// The sweep for crashes and hangs of the reader of the old binary workbook format (docs/jeffcrunch.md, "damaged and deliberately broken files").
//   jcrunchtool xls-fuzz <file.xls> [count=500] [seed=1] [seconds=60]   the file's workbook stream (and its container) damaged in a few places, `count` times: bytes changed, a
//                                                length field set to an extreme, records cut, repeated, moved, or given another number, the end cut off; each damaged copy is
//                                                opened, every sheet read, every formula's text asked for, and (for one copy in three) the calculation loaded and worked out
//                                                with the engine's own bounds. Prints how many opened, how many were refused, how many sheets stopped short, and the longest
//                                                time one copy took. It stops, with what it has counted, when `seconds` have passed.

#include "io/cfb.h"
#include "sheet/calculation.h"
#include "sheet/sheetreader.h"
#include "sheet/xls.h"

#include <QElapsedTimer>
#include <QFile>
#include <QTextStream>

#include <algorithm>
#include <random>

using namespace jp::office::sheet;

namespace jp::crunchtool {

namespace {

QTextStream &out()
{
    static QTextStream s(stdout);
    return s;
}

struct Rng {
    std::mt19937_64 gen;
    explicit Rng(quint64 seed) : gen(seed) {}
    qint64 below(qint64 n) { return n <= 0 ? 0 : qint64(gen() % quint64(n)); }
};

// The place of each record's header in a stream.
QVector<qsizetype> recordStarts(const QByteArray &d)
{
    QVector<qsizetype> starts;
    qsizetype o = 0;
    while (o + 4 <= d.size() && starts.size() < 2000000) {
        starts << o;
        const quint16 len = quint16(quint8(d[o + 2])) | (quint16(quint8(d[o + 3])) << 8);
        o += 4 + len;
    }
    return starts;
}

void put16(QByteArray &d, qsizetype o, quint16 v)
{
    if (o >= 0 && o + 2 <= d.size()) {
        d[o] = char(v & 0xFF);
        d[o + 1] = char(v >> 8);
    }
}

// Damages the bytes of a stream (records) or of a container in a few places.
void damage(QByteArray &d, Rng &rng, bool records)
{
    if (d.isEmpty()) return;
    const int steps = 1 + int(rng.below(3));
    for (int step = 0; step < steps; ++step) {
        const int kind = int(rng.below(records ? 8 : 4));
        switch (kind) {
        case 0: {   // bytes changed
            const int n = 1 + int(rng.below(16));
            for (int i = 0; i < n; ++i) d[qsizetype(rng.below(d.size()))] = char(rng.below(256));
            break;
        }
        case 1: {   // a run set to 0 or 0xFF
            const qsizetype at = qsizetype(rng.below(d.size()));
            const qsizetype n = qsizetype(1 + rng.below(64));
            const char fill = rng.below(2) ? char(0xFF) : char(0);
            for (qsizetype i = at; i < std::min(d.size(), at + n); ++i) d[i] = fill;
            break;
        }
        case 2:   // the end cut off
            d.truncate(qsizetype(rng.below(d.size())));
            break;
        case 3: {   // a piece repeated where another was
            const qsizetype at = qsizetype(rng.below(d.size()));
            const qsizetype n = qsizetype(1 + rng.below(std::min<qint64>(4096, d.size())));
            const QByteArray piece = d.mid(at, n);
            const qsizetype to = qsizetype(rng.below(d.size()));
            d.replace(to, std::min<qsizetype>(piece.size(), d.size() - to), piece.left(d.size() - to));
            break;
        }
        default: {
            const QVector<qsizetype> starts = recordStarts(d);
            if (starts.isEmpty()) break;
            const qsizetype at = starts[int(rng.below(starts.size()))];
            if (kind == 4) {   // a length set to an extreme
                static const quint16 lengths[] = {0, 1, 2, 3, 7, 8, 9, 15, 16, 255, 256, 8224, 8225, 32767, 32768, 65534, 65535};
                put16(d, at + 2, lengths[rng.below(sizeof(lengths) / sizeof(lengths[0]))]);
            } else if (kind == 5) {   // another record's number
                static const quint16 ids[] = {0x0809, 0x000A, 0x003C, 0x0006, 0x00FC, 0x0018, 0x0017, 0x01AE, 0x0085, 0x00E0, 0x0031, 0x041E, 0x00BD, 0x00BE, 0x027E, 0x04BC, 0x0221, 0x0236,
                                              0x0207, 0x00E5, 0x007D, 0x0208, 0x0225, 0x023E, 0x0041, 0x00A0, 0x001D, 0x002F, 0x0200, 0x0204, 0x00FD, 0x0203, 0x0205, 0x0201, 0x0092};
                put16(d, at, ids[rng.below(sizeof(ids) / sizeof(ids[0]))]);
            } else if (kind == 6) {   // a record taken out
                const quint16 len = quint16(quint8(d[at + 2])) | (quint16(quint8(d[at + 3])) << 8);
                d.remove(at, std::min<qsizetype>(4 + len, d.size() - at));
            } else {   // a record repeated
                const quint16 len = quint16(quint8(d[at + 2])) | (quint16(quint8(d[at + 3])) << 8);
                const QByteArray piece = d.mid(at, 4 + len);
                d.insert(starts[int(rng.below(starts.size()))], piece);
            }
            break;
        }
        }
        if (d.size() > (qsizetype(256) << 20)) d.truncate(qsizetype(256) << 20);
    }
}

} // namespace

int xlsFuzzMain(const QStringList &args)
{
    if (args.isEmpty() || args.size() > 4) return -1;
    const QString path = args[0];
    const int count = args.size() > 1 ? args[1].toInt() : 500;
    const quint64 seed = args.size() > 2 ? args[2].toULongLong() : 1;
    const double seconds = args.size() > 3 ? args[3].toDouble() : 60;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        out() << "!" << path << ": " << file.errorString() << "\n";
        return 1;
    }
    const QByteArray original = file.readAll();
    QByteArray stream;
    if (jp::cfb::File container; startsCompoundFile(original) && jp::cfb::read(original, &container)) {
        stream = container.stream(QStringLiteral("Workbook"));
        if (stream.isEmpty()) stream = container.stream(QStringLiteral("Book"));
    }
    if (stream.isEmpty()) stream = original;
    Rng rng(seed);
    QElapsedTimer total, each;
    total.start();
    qint64 opened = 0, refused = 0, partial = 0, calculated = 0, ran = 0, longestMs = 0;
    for (int i = 0; i < count && total.elapsed() < qint64(seconds * 1000); ++i) {
        // one copy in five has its container damaged and not the stream
        const bool container = i % 5 == 4 && startsCompoundFile(original);
        QByteArray bytes = container ? original : stream;
        damage(bytes, rng, !container);
        each.start();
        Workbook book;
        QString why;
        ++ran;
        if (!book.openBinary(bytes, &why)) {
            ++refused;
        } else {
            ++opened;
            const bool calc = i % 3 == 0;
            for (int s = 0; s < book.info().sheets.size() && s < 40; ++s) {
                Sheet sheet;
                if (!book.readSheet(s, &sheet)) ++partial;
                qint64 asked = 0;
                sheet.cells.forEach([&](quint32, quint32, const Cell &c) {
                    if (c.hasFormula() && asked++ < 20000) (void)sheet.formulaText(c);
                    if (c.type == CellType::SharedString || c.type == CellType::InlineString || c.type == CellType::FormulaString) (void)sheet.text(c, book.sharedStrings());
                });
                (void)book.styles().formatCode(sheet.cells.cellCount() ? 0 : 1);
            }
            if (calc) {
                CalcOptions options;
                options.keepApart = true;
                options.workOutWhatHasNoResult = true;
                Calculation calculation(book, options);
                calculation.load();
                calculation.recalculate(20000000);
                ++calculated;
            }
        }
        longestMs = std::max<qint64>(longestMs, each.elapsed());
    }
    out() << "xls-fuzz\t" << path << '\t' << ran << " copies\t" << opened << " opened\t" << refused << " refused\t" << partial << " sheets stopped short\t" << calculated << " calculated\t"
          << longestMs << " ms longest\n";
    return 0;
}

} // namespace jp::crunchtool
