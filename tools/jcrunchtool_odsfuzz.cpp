// The sweep for crashes and hangs of the readers of OpenDocument spreadsheets and of delimited text (docs/jeffcrunch.md, "damaged and deliberately broken files").
//   jcrunchtool convert-fuzz <file.ods|file.fods|file.csv> [count=500] [seed=1] [seconds=60]   the file's text damaged in a few places, `count` times: for an .ods its XML parts (the
//                                                bytes of content.xml, styles.xml, and settings.xml, inflated and put in a package again), for the others the file itself. Bytes
//                                                changed, a run set to 0 or 0xFF, the end cut off, a piece repeated, a number given an extreme value (repeat counts, spans, sizes),
//                                                a tag taken out or repeated, a stretch wrapped in tags without end, a DOCTYPE put in, a line repeated, a quote or delimiter run put
//                                                in. Each damaged copy is opened, every sheet read (and the first rows alone), every formula's text asked for, and (for one copy
//                                                in three) the calculation loaded and worked out with the engine's own bounds. Prints how many opened, how many were refused, how
//                                                many sheets stopped short, and the longest time one copy took. It stops, with what it has counted, when `seconds` have passed.

#include "io/zip.h"
#include "sheet/calculation.h"
#include "sheet/sheetreader.h"

#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
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

constexpr qsizetype kMaxBytes = qsizetype(64) << 20;

// Damages text in a few places.
void damage(QByteArray &d, Rng &rng, bool xml)
{
    if (d.isEmpty()) return;
    const int steps = 1 + int(rng.below(3));
    for (int step = 0; step < steps && !d.isEmpty(); ++step) {
        const int kind = int(rng.below(xml ? 11 : 8));
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
        case 4: {   // a line (or a stretch) repeated many times
            const qsizetype at = qsizetype(rng.below(d.size()));
            const qsizetype end = std::min<qsizetype>(d.size(), at + qsizetype(1 + rng.below(300)));
            const QByteArray piece = d.mid(at, end - at);
            const int times = 2 + int(rng.below(2000));
            QByteArray many;
            for (int i = 0; i < times && many.size() < (8 << 20); ++i) many += piece;
            d.insert(at, many);
            break;
        }
        case 5: {   // quotes, delimiters, and line breaks put in
            static const char *const bits[] = {"\"", "\"\"\"", ",", ";", "\t", "\r\n", "\n", "=", "'", "\"\n\"", "\xEF\xBB\xBF", "sep=;\n"};
            const QByteArray bit = bits[rng.below(sizeof(bits) / sizeof(bits[0]))];
            QByteArray many;
            const int times = 1 + int(rng.below(rng.below(3) == 0 ? 20000 : 5));
            for (int i = 0; i < times; ++i) many += bit;
            d.insert(qsizetype(rng.below(d.size() + 1)), many);
            break;
        }
        case 6: {   // a number given an extreme value
            static const char *const values[] = {"99999999999999999999", "2147483648", "4294967296", "-1", "0", "1048577", "16385", "1e308", "NaN", "-9223372036854775808", "65536"};
            QVector<qsizetype> digits;
            for (int tries = 0; tries < 50 && digits.size() < 1; ++tries) {
                const qsizetype at = qsizetype(rng.below(d.size()));
                if (d[at] >= '0' && d[at] <= '9') digits << at;
            }
            if (digits.isEmpty()) break;
            qsizetype from = digits[0], to = digits[0];
            while (from > 0 && d[from - 1] >= '0' && d[from - 1] <= '9') --from;
            while (to + 1 < d.size() && d[to + 1] >= '0' && d[to + 1] <= '9') ++to;
            d.replace(from, to - from + 1, values[rng.below(sizeof(values) / sizeof(values[0]))]);
            break;
        }
        case 7:   // the start cut off
            d.remove(0, qsizetype(rng.below(std::min<qint64>(d.size(), 256))));
            break;
        default: {
            // XML: a tag taken out (8), repeated (9), or a stretch wrapped in tags without end and a DOCTYPE put in (10)
            const qsizetype start = d.indexOf('<', qsizetype(rng.below(d.size())));
            if (start < 0) break;
            const qsizetype end = d.indexOf('>', start);
            if (end < 0) break;
            if (kind == 8) {
                d.remove(start, end - start + 1);
            } else if (kind == 9) {
                const QByteArray tag = d.mid(start, end - start + 1);
                QByteArray many;
                const int times = 2 + int(rng.below(rng.below(2) ? 100000 : 50));
                for (int i = 0; i < times && many.size() < (8 << 20); ++i) many += tag;
                d.insert(start, many);
            } else if (rng.below(2)) {
                QByteArray open;
                const int times = 1 + int(rng.below(100000));
                for (int i = 0; i < times; ++i) open += "<text:span>";
                d.insert(start, open);
            } else {
                d.insert(0, "<!DOCTYPE x [<!ENTITY a \"aaaa\">]>");
            }
            break;
        }
        }
        if (d.size() > kMaxBytes) d.truncate(kMaxBytes);
    }
}

} // namespace

int odsFuzzMain(const QStringList &args)
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
    file.close();
    // an .ods: its parts; any other file is its own text
    QMap<QString, QByteArray> parts;
    QStringList order;
    const bool zipped = original.startsWith("PK") && jp::readZip(original, parts);
    if (zipped) order = parts.keys();
    QTemporaryDir dir;
    const QString suffix = QFileInfo(path).suffix().toLower();
    const QString copy = dir.filePath(QStringLiteral("copy.") + (suffix.isEmpty() ? QStringLiteral("csv") : suffix));
    Rng rng(seed);
    QElapsedTimer total, each;
    total.start();
    qint64 opened = 0, refused = 0, partial = 0, calculated = 0, ran = 0, longestMs = 0;
    for (int i = 0; i < count && total.elapsed() < qint64(seconds * 1000); ++i) {
        QByteArray bytes;
        if (zipped) {
            QMap<QString, QByteArray> damaged = parts;
            // (the XML of one part, now and then of two; the package itself, one copy in ten)
            const QStringList xml = {QStringLiteral("content.xml"), QStringLiteral("styles.xml"), QStringLiteral("settings.xml"), QStringLiteral("META-INF/manifest.xml"), QStringLiteral("mimetype")};
            const int n = 1 + int(rng.below(2));
            for (int k = 0; k < n; ++k) {
                const QString name = rng.below(3) == 0 ? xml[int(rng.below(xml.size()))] : QStringLiteral("content.xml");
                if (damaged.contains(name)) damage(damaged[name], rng, true);
            }
            jp::ZipWriter zip;
            for (const QString &name : std::as_const(order)) zip.add(name, damaged.value(name), name != QLatin1String("mimetype"));
            bytes = zip.finish();
            if (i % 10 == 9) damage(bytes, rng, false);
        } else {
            bytes = original;
            damage(bytes, rng, original.trimmed().startsWith('<'));
        }
        QFile f(copy);
        if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) continue;
        f.write(bytes);
        f.close();
        each.start();
        Workbook book;
        QString why;
        ++ran;
        if (!book.openFile(copy, &why)) {
            ++refused;
        } else {
            ++opened;
            const bool calc = i % 3 == 0;
            for (int s = 0; s < book.info().sheets.size() && s < 40; ++s) {
                Sheet sheet;
                book.readSheet(s, &sheet, nullptr, -1, 100);   // (the first screen alone)
                if (!book.readSheet(s, &sheet)) ++partial;
                qint64 asked = 0;
                sheet.cells.forEach([&](const Cell &c) {
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
    out() << "convert-fuzz\t" << path << '\t' << ran << " copies\t" << opened << " opened\t" << refused << " refused\t" << partial << " sheets stopped short\t" << calculated << " calculated\t"
          << longestMs << " ms longest\n";
    return 0;
}

} // namespace jp::crunchtool
