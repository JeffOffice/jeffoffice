#include "sheet/externalbooks.h"

#include "office/package.h"
#include "sheet/cellref.h"
#include "sheet/sheetreader.h"

#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QUrl>
#include <QXmlStreamReader>

#include <algorithm>
#include <atomic>

namespace jp::office::sheet {

using jp::calc::ErrorKind;
using jp::calc::Value;

namespace {

constexpr int kMostLinks = 1000;
constexpr qint64 kMostCacheCells = 2000000;
constexpr size_t kMostReadRanges = 64;

// The file on this computer an address names (empty for a web address, or none).
QString localPathOf(const QString &target, const QString &baseDir)
{
    QString t = target.trimmed();
    if (t.isEmpty()) return {};
    if (t.startsWith(QLatin1String("http://"), Qt::CaseInsensitive) || t.startsWith(QLatin1String("https://"), Qt::CaseInsensitive) || t.startsWith(QLatin1String("ftp://"), Qt::CaseInsensitive)) return {};
    if (t.startsWith(QLatin1String("file:///"), Qt::CaseInsensitive)) {
        t.remove(0, 8);
        if (!(t.size() > 1 && t[1] == QLatin1Char(':'))) t.prepend(QLatin1Char('/'));
    }
    t = QUrl::fromPercentEncoding(t.toUtf8());
    if (!QDir::isAbsolutePath(t) && !baseDir.isEmpty()) t = QDir(baseDir).filePath(t);
    return t;
}

// What a source gives: its sheets and names, read once.
struct Source {
    Workbook book;
    std::vector<std::shared_ptr<Sheet>> sheets;   // read when first asked
    std::vector<std::pair<quint32, quint32>> extents;
    std::mutex lock;

    // The sheet at a place, read; null if it can't be.
    std::shared_ptr<Sheet> sheetAt(int index)
    {
        const std::lock_guard<std::mutex> guard(lock);
        if (index < 0 || size_t(index) >= sheets.size()) return nullptr;
        if (!sheets[size_t(index)]) {
            auto s = std::make_shared<Sheet>();
            if (!book.readSheet(index, s.get())) return nullptr;
            quint32 rows = 0, cols = 0;
            s->cells.forEach([&](quint32 r, quint32 c, const Cell &cell) {
                if (cell.type == CellType::Empty) return;
                rows = std::max(rows, r + 1);
                cols = std::max(cols, c + 1);
            });
            extents[size_t(index)] = {rows, cols};
            sheets[size_t(index)] = s;
        }
        return sheets[size_t(index)];
    }
    int sheetIndex(const QString &name) const
    {
        const auto &list = book.info().sheets;
        for (int i = 0; i < list.size(); ++i)
            if (!list[i].deleted && list[i].name.compare(name, Qt::CaseInsensitive) == 0) return i;
        return -1;
    }
    Value valueOf(const Sheet &s, const Cell &c) const
    {
        switch (c.type) {
        case CellType::Empty: return Value();
        case CellType::Number:
        case CellType::Date: return Value::number(c.number);
        case CellType::SharedString: return Value::text(book.sharedStrings().at(c.textIndex()));
        case CellType::InlineString:
        case CellType::FormulaString: return Value::text(s.texts.value(c.textIndex()));
        case CellType::Bool: return Value::boolean(c.number != 0);
        case CellType::Error: return Value::error(ErrorKind(std::max(0, std::min(int(ErrorKind::Unknown), int(c.number)))));
        }
        return Value();
    }
};

struct Link {
    QString target;
    bool changed = false;       // Change Source: the cache stays, the file is another
    bool live = false;
    // the cache the link's part holds
    QStringList sheetNames;
    std::map<std::tuple<int, quint32, quint32>, Value> cells;
    std::map<int, std::pair<quint32, quint32>> extents;   // by sheet: the rows and columns the cache has
    std::map<QString, QString> names;                      // by lower-case name: what it refers to
    // the source file, when read
    std::shared_ptr<Source> source;
    qint64 signSize = -1, signTime = 0;
    qint64 checkedAt = 0;
    bool unreadable = false;
    QDateTime readTime;
    // what the formulas read of it: by sheet name (lower), the blocks
    std::map<QString, std::pair<QString, std::vector<CellRange>>> reads;
};

// The ends of a reference as it follows the "!": "A1", "$A$1:$B$9", "A:C", "3:5". False if it is none of these.
bool parseSpan(QString text, CellRange *out)
{
    text.remove(QLatin1Char('$'));
    text.remove(QLatin1Char(' '));
    if (parseRangeRef(text, out)) return true;
    const int colon = text.indexOf(QLatin1Char(':'));
    if (colon <= 0 || colon >= text.size() - 1) return false;
    const QString a = text.left(colon), b = text.mid(colon + 1);
    bool digitsA = true, digitsB = true, lettersA = true, lettersB = true;
    for (const QChar ch : a) (ch.isDigit() ? lettersA : digitsA) = false;
    for (const QChar ch : b) (ch.isDigit() ? lettersB : digitsB) = false;
    if (digitsA && digitsB) {
        const quint32 r1 = a.toUInt(), r2 = b.toUInt();
        if (r1 < 1 || r2 < 1 || r1 > kMaxRows || r2 > kMaxRows) return false;
        out->row1 = std::min(r1, r2) - 1;
        out->row2 = std::max(r1, r2) - 1;
        out->col1 = 0;
        out->col2 = kMaxColumns - 1;
        out->valid = true;
        return true;
    }
    if (lettersA && lettersB) {
        quint32 r = 0, c1 = 0, c2 = 0;
        if (!parseCellRef(a + QLatin1Char('1'), &r, &c1) || !parseCellRef(b + QLatin1Char('1'), &r, &c2)) return false;
        out->col1 = std::min(c1, c2);
        out->col2 = std::max(c1, c2);
        out->row1 = 0;
        out->row2 = kMaxRows - 1;
        out->valid = true;
        return true;
    }
    return false;
}

struct Parsed {
    int link = 0;
    QString sheet;        // as written (a name; "A:B" style through several sheets is "Sheet1:Sheet3")
    QString what;         // the cells ("A1:B2") or, when `isName`, the name
    bool isName = false;
};

// `[1]Sheet1!A1`, `'[1]My Sheet'!$A$1:$B$2`, `[1]!Name`, `[1]!'a name'`, `'[1]!Name'`.
bool parseReference(const QString &text, Parsed *out)
{
    QString s = text.trimmed();
    QString inside, after;
    if (s.startsWith(QLatin1Char('\''))) {
        QString name;
        int i = 1;
        for (; i < s.size(); ++i) {
            if (s[i] == QLatin1Char('\'')) {
                if (i + 1 < s.size() && s[i + 1] == QLatin1Char('\'')) {
                    name += QLatin1Char('\'');
                    ++i;
                    continue;
                }
                break;
            }
            name += s[i];
        }
        if (i >= s.size() || i + 1 >= s.size() || s[i + 1] != QLatin1Char('!')) return false;
        inside = name;
        after = s.mid(i + 2);
        if (!inside.startsWith(QLatin1Char('['))) return false;
    } else {
        inside = s;
        const int bang = s.indexOf(QLatin1Char('!'));
        if (bang < 0) return false;
        inside = s.left(bang);
        after = s.mid(bang + 1);
    }
    const int close = inside.indexOf(QLatin1Char(']'));
    if (!inside.startsWith(QLatin1Char('[')) || close < 2) return false;
    bool ok = false;
    const int n = inside.mid(1, close - 1).toInt(&ok);
    if (!ok || n < 1) return false;
    out->link = n;
    out->sheet = inside.mid(close + 1);
    if (out->sheet.isEmpty()) {
        // [1]!Name or [1]!'a name'
        out->isName = true;
        if (after.startsWith(QLatin1Char('\'')) && after.endsWith(QLatin1Char('\'')) && after.size() >= 2) after = after.mid(1, after.size() - 2).replace(QStringLiteral("''"), QStringLiteral("'"));
        out->what = after;
        return !after.isEmpty();
    }
    if (after.startsWith(QLatin1Char('\'')) || after.isEmpty()) return false;
    // (a name in a name's place: [1]Sheet1!Name)
    CellRange probe;
    if (!parseSpan(after, &probe)) {
        out->isName = true;
    }
    out->what = after;
    return true;
}

QString unquoteSheet(QString s)
{
    s = s.trimmed();
    if (s.size() >= 2 && s.startsWith(QLatin1Char('\'')) && s.endsWith(QLatin1Char('\''))) s = s.mid(1, s.size() - 2).replace(QStringLiteral("''"), QStringLiteral("'"));
    return s;
}

// The place of "Sheet1!$A$1:$B$2" (a name's text) as a sheet's name and its cells; false for anything that is not one place.
bool splitPlace(const QString &ref, QString *sheet, CellRange *range)
{
    QString r = ref.trimmed();
    if (r.startsWith(QLatin1Char('='))) r.remove(0, 1);
    // (through an external book: "[1]Sheet1!$A$1")
    if (r.startsWith(QLatin1Char('['))) {
        const int close = r.indexOf(QLatin1Char(']'));
        if (close > 0) r.remove(1, close);
        r.remove(0, 1);
    } else if (r.startsWith(QLatin1String("'["))) {
        const int close = r.indexOf(QLatin1Char(']'));
        if (close > 0) r.remove(1, close);
    }
    const int bang = r.lastIndexOf(QLatin1Char('!'));
    if (bang <= 0) return false;
    *sheet = unquoteSheet(r.left(bang));
    return parseSpan(r.mid(bang + 1), range);
}

} // namespace

struct ExternalBooks::Impl {
    mutable std::mutex lock;
    std::vector<Link> links;   // the first is link 1
    QString baseDir;
    QDateTime fileTime;
    int checkMillis = 500;
    mutable std::atomic<int> reads{0};

    // The source of a link that is live (read now if it is not, or if its file changed): null if it can't be had.
    std::shared_ptr<Source> sourceOf(int index, bool *tooBig = nullptr) const
    {
        QString path;
        {
            const std::lock_guard<std::mutex> guard(lock);
            Link &l = const_cast<Link &>(links[size_t(index)]);
            path = localPathOf(l.target, baseDir);
            if (path.isEmpty()) return nullptr;
            const QFileInfo info(path);
            const qint64 now = QDateTime::currentMSecsSinceEpoch();
            if (l.source && now - l.checkedAt < checkMillis) return l.source;
            l.checkedAt = now;
            if (!info.isFile()) {
                l.source.reset();
                l.signSize = -1;
                return nullptr;
            }
            const qint64 time = info.lastModified().toMSecsSinceEpoch();
            if ((l.source || l.unreadable) && info.size() == l.signSize && time == l.signTime) return l.source;
            l.source.reset();
            l.unreadable = false;
            if (info.size() > kMaxExternalFileBytes) {
                l.unreadable = true;
                l.signSize = info.size();
                l.signTime = time;
                if (tooBig) *tooBig = true;
                return nullptr;
            }
        }
        // (read with the lock let go of: a big file takes a while, and the window asks for the state of its links meanwhile)
        auto source = std::make_shared<Source>();
        QString error;
        QElapsedTimer timer;
        timer.start();
        Limits limits;
        limits.cancelled = [&] { return timer.elapsed() > kMaxExternalReadMillis; };
        const QFileInfo before(path);
        const bool ok = before.isFile() && source->book.openFile(path, &error, limits);
        reads.fetch_add(1);
        const std::lock_guard<std::mutex> guard(lock);
        Link &l = const_cast<Link &>(links[size_t(index)]);
        l.signSize = before.size();
        l.signTime = before.lastModified().toMSecsSinceEpoch();
        if (!ok) {
            l.unreadable = true;
            return nullptr;
        }
        const size_t n = size_t(source->book.info().sheets.size());
        source->sheets.assign(n, nullptr);
        source->extents.assign(n, {0, 0});
        l.source = source;
        l.readTime = QDateTime::currentDateTime();
        l.reads.clear();
        return source;
    }

    void noteRead(int index, const QString &sheetName, const CellRange &r) const
    {
        const std::lock_guard<std::mutex> guard(lock);
        Link &l = const_cast<Link &>(links[size_t(index)]);
        auto &entry = l.reads[sheetName.toCaseFolded()];
        entry.first = sheetName;
        auto &list = entry.second;
        for (const CellRange &x : list)
            if (x == r) return;
        list.push_back(r);
        if (list.size() > kMostReadRanges) {
            CellRange all = list.front();
            for (const CellRange &x : list) {
                all.row1 = std::min(all.row1, x.row1);
                all.col1 = std::min(all.col1, x.col1);
                all.row2 = std::max(all.row2, x.row2);
                all.col2 = std::max(all.col2, x.col2);
            }
            list.assign(1, all);
        }
    }

    // The values of a place, from the live source: false if it can't be had.
    bool fromSource(int index, const Parsed &p, Value *out) const
    {
        const std::shared_ptr<Source> src = sourceOf(index);
        if (!src) return false;
        QString sheetName;
        CellRange range;
        QString what = p.what;
        QString nameSheet = unquoteSheet(p.sheet);
        if (p.isName) {
            // a defined name of the source: by its sheet when it names one, else the workbook's
            QString text;
            const auto &sheets = src->book.info().sheets;
            int local = nameSheet.isEmpty() ? -1 : src->sheetIndex(nameSheet);
            for (int pass = 0; pass < 2 && text.isEmpty(); ++pass)
                for (const DefinedName &n : src->book.info().definedNames) {
                    if (n.name.compare(what, Qt::CaseInsensitive) != 0) continue;
                    if (pass == 0 && local >= 0 && n.localSheet == local) text = n.ref;
                    else if (pass == 1 && n.localSheet < 0) text = n.ref;
                    if (!text.isEmpty()) break;
                }
            Q_UNUSED(sheets)
            if (text.isEmpty()) return false;
            bool number = false;
            const double d = text.trimmed().toDouble(&number);
            if (number) {
                *out = Value::number(d);
                return true;
            }
            if (!splitPlace(text, &sheetName, &range)) return false;
            return arrayFrom(index, src, sheetName, sheetName, range, out, true);
        }
        if (!parseSpan(what, &range)) return false;
        const QString first = nameSheet;
        QString last = nameSheet;
        if (src->sheetIndex(nameSheet) < 0) {
            const int colon = nameSheet.indexOf(QLatin1Char(':'));
            if (colon <= 0) return false;
            return arrayFrom(index, src, nameSheet.left(colon), nameSheet.mid(colon + 1), range, out, true);
        }
        return arrayFrom(index, src, first, last, range, out, true);
    }

    bool arrayFrom(int index, const std::shared_ptr<Source> &src, const QString &first, const QString &last, CellRange range, Value *out, bool note) const
    {
        const int a = src->sheetIndex(first), b = src->sheetIndex(last);
        if (a < 0 || b < 0) return false;
        const int lo = std::min(a, b), hi = std::max(a, b);
        std::vector<int> sheets;
        for (int i = lo; i <= hi; ++i)
            if (!src->book.info().sheets[i].deleted) sheets.push_back(i);
        // (a whole column or row, or a block past what the sheet holds, is only as big as what the sheet holds)
        std::vector<std::shared_ptr<Sheet>> read;
        quint32 rows = range.row2 - range.row1 + 1, cols = range.col2 - range.col1 + 1;
        const bool single = range.row1 == range.row2 && range.col1 == range.col2 && sheets.size() == 1;
        for (int s : sheets) {
            std::shared_ptr<Sheet> sh = src->sheetAt(s);
            if (!sh) return false;
            read.push_back(sh);
        }
        auto clipped = [&](int s) {
            const auto ext = src->extents[size_t(s)];
            CellRange r = range;
            if (range.row2 >= ext.first) r.row2 = ext.first > 0 ? std::min(range.row2, ext.first - 1) : range.row1;
            if (range.col2 >= ext.second) r.col2 = ext.second > 0 ? std::min(range.col2, ext.second - 1) : range.col1;
            return r;
        };
        quint32 useRows = rows, useCols = cols;
        if (rows > 100000 || cols > 1000) {
            quint32 maxR = 0, maxC = 0;
            for (int s : sheets) {
                const CellRange r = clipped(s);
                maxR = std::max(maxR, r.row2 - r.row1 + 1);
                maxC = std::max(maxC, r.col2 - r.col1 + 1);
            }
            if (rows > 100000) useRows = std::max<quint32>(maxR, 1);
            if (cols > 1000) useCols = std::max<quint32>(maxC, 1);
        }
        if (single) {
            if (note) noteRead(index, src->book.info().sheets[sheets[0]].name, range);
            const Cell *c = read[0]->cells.find(range.row1, range.col1);
            *out = c ? src->valueOf(*read[0], *c) : Value();
            return true;
        }
        const qint64 total = qint64(useRows) * qint64(useCols) * qint64(sheets.size());
        if (total > kMaxExternalRefCells || total > jp::calc::kMaxArrayCells) return false;
        Value arr = Value::array(int(useRows * quint32(sheets.size())), int(useCols));
        if (!arr.isArray() || arr.rows() != int(useRows * quint32(sheets.size()))) return false;
        for (size_t k = 0; k < sheets.size(); ++k) {
            const Sheet &sh = *read[k];
            if (note) {
                CellRange r = range;
                r.row2 = std::min(range.row2, range.row1 + useRows - 1);
                r.col2 = std::min(range.col2, range.col1 + useCols - 1);
                noteRead(index, src->book.info().sheets[sheets[k]].name, r);
            }
            for (quint32 r = 0; r < useRows; ++r)
                for (quint32 c = 0; c < useCols; ++c) {
                    const Cell *cell = sh.cells.find(range.row1 + r, range.col1 + c);
                    if (cell && cell->type != CellType::Empty) arr.set(int(quint32(k) * useRows + r), int(c), src->valueOf(sh, *cell));
                }
        }
        *out = arr;
        return true;
    }

    // The same from the link's cache.
    bool fromCache(int index, const Parsed &p, Value *out) const
    {
        const std::lock_guard<std::mutex> guard(lock);
        const Link &l = links[size_t(index)];
        QString sheetName;
        CellRange range;
        QString nameSheet = unquoteSheet(p.sheet);
        QString firstName, lastName;
        if (p.isName) {
            const auto it = l.names.find(p.what.toCaseFolded());
            if (it == l.names.end()) return false;
            bool number = false;
            const double d = it->second.trimmed().toDouble(&number);
            if (number) {
                *out = Value::number(d);
                return true;
            }
            if (!splitPlace(it->second, &sheetName, &range)) return false;
            firstName = lastName = sheetName;
        } else {
            if (!parseSpan(p.what, &range)) return false;
            firstName = lastName = nameSheet;
        }
        auto indexOfSheet = [&](const QString &name) {
            for (int i = 0; i < l.sheetNames.size(); ++i)
                if (l.sheetNames[i].compare(name, Qt::CaseInsensitive) == 0) return i;
            return -1;
        };
        int a = indexOfSheet(firstName), b = indexOfSheet(lastName);
        if (a < 0 && !p.isName) {
            const int colon = nameSheet.indexOf(QLatin1Char(':'));
            if (colon > 0) {
                a = indexOfSheet(nameSheet.left(colon));
                b = indexOfSheet(nameSheet.mid(colon + 1));
            }
        }
        if (a < 0 || b < 0) return false;
        const int lo = std::min(a, b), hi = std::max(a, b);
        const quint32 sheetCount = quint32(hi - lo + 1);
        quint32 rows = range.row2 - range.row1 + 1, cols = range.col2 - range.col1 + 1;
        const bool single = rows == 1 && cols == 1 && sheetCount == 1;
        if (single) {
            const auto it = l.cells.find({lo, range.row1, range.col1});
            *out = it == l.cells.end() ? Value() : it->second;
            return true;
        }
        quint32 maxR = 0, maxC = 0;
        for (int s = lo; s <= hi; ++s) {
            const auto e = l.extents.find(s);
            if (e == l.extents.end()) continue;
            maxR = std::max(maxR, e->second.first);
            maxC = std::max(maxC, e->second.second);
        }
        if (rows > 100000) rows = std::max<quint32>(1, std::min<quint32>(rows, maxR > range.row1 ? maxR - range.row1 : 1));
        if (cols > 1000) cols = std::max<quint32>(1, std::min<quint32>(cols, maxC > range.col1 ? maxC - range.col1 : 1));
        const qint64 total = qint64(rows) * qint64(cols) * qint64(sheetCount);
        if (total > kMaxExternalRefCells || total > jp::calc::kMaxArrayCells) return false;
        Value arr = Value::array(int(rows * sheetCount), int(cols));
        if (!arr.isArray() || arr.rows() != int(rows * sheetCount)) return false;
        for (int s = lo; s <= hi; ++s) {
            const quint32 base = quint32(s - lo) * rows;
            auto it = l.cells.lower_bound({s, range.row1, 0});
            for (; it != l.cells.end() && std::get<0>(it->first) == s && std::get<1>(it->first) < range.row1 + rows; ++it) {
                const quint32 r = std::get<1>(it->first), c = std::get<2>(it->first);
                if (c < range.col1 || c >= range.col1 + cols) continue;
                arr.set(int(base + r - range.row1), int(c - range.col1), it->second);
            }
        }
        *out = arr;
        return true;
    }
};

ExternalBooks::ExternalBooks() : d(new Impl) {}
ExternalBooks::~ExternalBooks() = default;

namespace {

Value cachedValue(const QString &type, const QString &text)
{
    if (type == QLatin1String("str") || type == QLatin1String("s")) return Value::text(text);
    if (type == QLatin1String("b")) return Value::boolean(text.trimmed() == QLatin1String("1") || text.trimmed().compare(QLatin1String("true"), Qt::CaseInsensitive) == 0);
    if (type == QLatin1String("e")) return Value::error(jp::calc::errorKindFromText(text));
    bool ok = false;
    const double v = text.trimmed().toDouble(&ok);
    return ok ? Value::number(v) : Value::error(ErrorKind::Value);
}

// What a link's own part holds: the names of the sheets, the cells it stored, and the names.
void readCache(const QByteArray &xml, Link *l)
{
    QXmlStreamReader r(xml);
    int sheetId = -1;
    quint32 row = 0;
    bool inCell = false, inV = false;
    QString ref, type, text;
    qint64 count = 0;
    while (!r.atEnd() && count < kMostCacheCells) {
        const QXmlStreamReader::TokenType t = r.readNext();
        if (t == QXmlStreamReader::StartElement) {
            const QStringView n = r.name();
            if (n == QLatin1String("sheetName")) {
                if (l->sheetNames.size() < 10000) l->sheetNames << r.attributes().value(QLatin1String("val")).toString();
            } else if (n == QLatin1String("definedName")) {
                const QString name = r.attributes().value(QLatin1String("name")).toString();
                const QString refers = r.attributes().value(QLatin1String("refersTo")).toString();
                if (!name.isEmpty() && l->names.size() < 100000) l->names[name.toCaseFolded()] = refers;
            } else if (n == QLatin1String("sheetData")) {
                sheetId = r.attributes().value(QLatin1String("sheetId")).toInt();
            } else if (n == QLatin1String("row")) {
                row = r.attributes().value(QLatin1String("r")).toUInt();
            } else if (n == QLatin1String("cell")) {
                inCell = true;
                ref = r.attributes().value(QLatin1String("r")).toString();
                type = r.attributes().value(QLatin1String("t")).toString();
                text.clear();
            } else if (n == QLatin1String("v") && inCell) {
                inV = true;
                text.clear();
            }
        } else if (t == QXmlStreamReader::Characters && inV) {
            text += r.text().toString();
        } else if (t == QXmlStreamReader::EndElement) {
            const QStringView n = r.name();
            if (n == QLatin1String("v")) inV = false;
            else if (n == QLatin1String("cell") && inCell) {
                inCell = false;
                quint32 rr = 0, cc = 0;
                if (parseCellRef(ref, &rr, &cc)) {
                    (void)row;
                    l->cells[{sheetId, rr, cc}] = cachedValue(type, text);
                    auto &e = l->extents[sheetId];
                    e.first = std::max(e.first, rr + 1);
                    e.second = std::max(e.second, cc + 1);
                    ++count;
                }
            }
        }
    }
}

} // namespace

std::shared_ptr<ExternalBooks> ExternalBooks::read(const Workbook &book, const QString &baseDir, const QDateTime &fileTime)
{
    const jp::office::Package &package = book.package();
    const QString main = book.mainPart();
    if (main.isEmpty()) return nullptr;
    const QByteArray xml = package.part(main);
    if (!xml.contains("externalReference")) return nullptr;
    auto out = std::make_shared<ExternalBooks>();
    out->d->baseDir = baseDir;
    out->d->fileTime = fileTime;
    QXmlStreamReader reader(xml);
    while (!reader.atEnd() && int(out->d->links.size()) < kMostLinks) {
        if (reader.readNext() != QXmlStreamReader::StartElement || reader.name() != QLatin1String("externalReference")) continue;
        QString id;
        for (const QXmlStreamAttribute &a : reader.attributes())
            if (a.name() == QLatin1String("id")) id = a.value().toString();
        Link l;
        const jp::office::Relationship rel = package.relationship(main, id);
        if (!rel.id.isEmpty()) {
            const QVector<jp::office::Relationship> paths = package.relationshipsOfType(rel.target, QStringLiteral("/externalLinkPath"));
            if (!paths.isEmpty()) l.target = paths.first().target.left(8192);
            if (package.partSize(rel.target) < (64 << 20)) readCache(package.part(rel.target), &l);
        }
        out->d->links.push_back(std::move(l));
    }
    if (out->d->links.empty()) return nullptr;
    return out;
}

int ExternalBooks::addLink(const QString &target, const QStringList &sheetNames)
{
    const std::lock_guard<std::mutex> guard(d->lock);
    Link l;
    l.target = target;
    l.sheetNames = sheetNames;
    d->links.push_back(std::move(l));
    return int(d->links.size());
}

int ExternalBooks::count() const
{
    const std::lock_guard<std::mutex> guard(d->lock);
    return int(d->links.size());
}

QString ExternalBooks::target(int index) const
{
    const std::lock_guard<std::mutex> guard(d->lock);
    return index >= 1 && index <= int(d->links.size()) ? d->links[size_t(index - 1)].target : QString();
}

QString ExternalBooks::localPath(int index) const
{
    const std::lock_guard<std::mutex> guard(d->lock);
    return index >= 1 && index <= int(d->links.size()) ? localPathOf(d->links[size_t(index - 1)].target, d->baseDir) : QString();
}

void ExternalBooks::setBaseDir(const QString &dir)
{
    const std::lock_guard<std::mutex> guard(d->lock);
    d->baseDir = dir;
}

void ExternalBooks::setTarget(int index, const QString &target)
{
    const std::lock_guard<std::mutex> guard(d->lock);
    if (index < 1 || index > int(d->links.size())) return;
    Link &l = d->links[size_t(index - 1)];
    l.target = target;
    l.changed = true;
    l.source.reset();
    l.unreadable = false;
    l.signSize = -1;
}

ExternalBooks::State ExternalBooks::state(int index) const
{
    const std::lock_guard<std::mutex> guard(d->lock);
    if (index < 1 || index > int(d->links.size())) return State::Missing;
    const Link &l = d->links[size_t(index - 1)];
    const QString path = localPathOf(l.target, d->baseDir);
    const QFileInfo info(path);
    if (path.isEmpty() || !info.isFile()) return State::Missing;
    if (l.unreadable) return State::Unreadable;
    const QDateTime ref = l.readTime.isValid() ? l.readTime : d->fileTime;
    if (ref.isValid() && info.lastModified().toMSecsSinceEpoch() > ref.toMSecsSinceEpoch() + 2000) return State::Newer;
    return State::Ok;
}

bool ExternalBooks::live(int index) const
{
    const std::lock_guard<std::mutex> guard(d->lock);
    return index >= 1 && index <= int(d->links.size()) && d->links[size_t(index - 1)].live;
}

void ExternalBooks::setLive(int index, bool on)
{
    const std::lock_guard<std::mutex> guard(d->lock);
    if (index >= 1 && index <= int(d->links.size())) d->links[size_t(index - 1)].live = on;
}

void ExternalBooks::setCheckInterval(int millis)
{
    const std::lock_guard<std::mutex> guard(d->lock);
    d->checkMillis = millis;
}

void ExternalBooks::invalidate(int index)
{
    const std::lock_guard<std::mutex> guard(d->lock);
    if (index < 1 || index > int(d->links.size())) return;
    Link &l = d->links[size_t(index - 1)];
    l.source.reset();
    l.unreadable = false;
    l.signSize = -1;
    l.checkedAt = 0;
}

bool ExternalBooks::probe(int index, QString *why)
{
    if (index < 1 || index > count()) {
        if (why) *why = QStringLiteral("no such link");
        return false;
    }
    invalidate(index);
    const bool ok = d->sourceOf(index - 1) != nullptr;
    if (!ok && why) *why = state(index) == State::Missing ? QStringLiteral("not found") : QStringLiteral("could not be read");
    return ok;
}

int ExternalBooks::linkNumber(const QString &text)
{
    Parsed p;
    return parseReference(text, &p) ? p.link : 0;
}

bool ExternalBooks::resolve(const QString &text, Value *out, bool *fromFile) const
{
    Parsed p;
    if (!parseReference(text, &p)) return false;
    int n;
    bool live;
    {
        const std::lock_guard<std::mutex> guard(d->lock);
        n = int(d->links.size());
        live = p.link >= 1 && p.link <= n && d->links[size_t(p.link - 1)].live;
    }
    if (p.link < 1 || p.link > n) return false;
    if (live && d->fromSource(p.link - 1, p, out)) {
        if (fromFile) *fromFile = true;
        return true;
    }
    if (fromFile) *fromFile = false;
    return d->fromCache(p.link - 1, p, out);
}

std::vector<SaveChanges::ExternalCachedCell> ExternalBooks::readCells(int index) const
{
    std::vector<SaveChanges::ExternalCachedCell> out;
    std::shared_ptr<Source> src;
    std::map<QString, std::pair<QString, std::vector<CellRange>>> reads;
    {
        const std::lock_guard<std::mutex> guard(d->lock);
        if (index < 1 || index > int(d->links.size())) return out;
        const Link &l = d->links[size_t(index - 1)];
        src = l.source;
        reads = l.reads;
    }
    if (!src) return out;
    constexpr size_t kMostCells = 500000;
    for (const auto &[key, entry] : reads) {
        const int s = src->sheetIndex(entry.first);
        if (s < 0) continue;
        const std::shared_ptr<Sheet> sh = src->sheetAt(s);
        if (!sh) continue;
        sh->cells.forEach([&](quint32 r, quint32 c, const Cell &cell) {
            if (cell.type == CellType::Empty || out.size() >= kMostCells) return;
            bool inside = false;
            for (const CellRange &x : entry.second)
                if (x.contains(r, c)) {
                    inside = true;
                    break;
                }
            if (!inside) return;
            SaveChanges::ExternalCachedCell e;
            e.sheet = src->book.info().sheets[s].name;
            e.row = r;
            e.col = c;
            const Value v = src->valueOf(*sh, cell);
            switch (v.kind()) {
            case Value::Kind::Number: e.number = v.number(); break;
            case Value::Kind::Bool: e.kind = 'b'; e.number = v.boolean() ? 1 : 0; break;
            case Value::Kind::Text: e.kind = 's'; e.text = v.text(); break;
            case Value::Kind::Error: e.kind = 'e'; e.text = jp::calc::errorKindText(v.errorKind()); break;
            default: return;
            }
            out.push_back(e);
        });
    }
    return out;
}

QStringList ExternalBooks::sheetsRead(int index) const
{
    QStringList out;
    const std::lock_guard<std::mutex> guard(d->lock);
    if (index < 1 || index > int(d->links.size())) return out;
    for (const auto &[key, entry] : d->links[size_t(index - 1)].reads) out << entry.first;
    return out;
}

int ExternalBooks::fileReads() const { return d->reads.load(); }

} // namespace jp::office::sheet
