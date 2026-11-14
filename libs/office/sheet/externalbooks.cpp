#include "sheet/externalbooks.h"

#include "office/package.h"
#include "sheet/cellref.h"
#include "sheet/sheetreader.h"

#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QThread>
#include <QUrl>
#include <QXmlStreamReader>

#include <algorithm>
#include <atomic>

namespace jp::office::sheet {

using jp::calc::ErrorKind;
using jp::calc::Value;

namespace {

constexpr int kMostLinks = 1000;
constexpr qint64 kMostCacheCells = 2000000;           // the cells of one link's stored values
// What all the links' stored values may come to, and all the sources that are read (a test lowers them).
struct Caps {
    qint64 cacheCells = 5000000, cacheBytes = qint64(128) << 20, sourceCells = 5000000;
    qint64 readMillis = kMaxExternalReadMillis;
} g_caps;
constexpr size_t kMostReadRanges = 64;

struct Resolved {
    QString path;          // the file, as a path (empty for a web address, or none)
    bool network = false;  // on another computer: a path that begins with two slashes (or two backslashes), or a file: address with a host
    bool web = false;
};

// What an address names. Nothing is looked at here: no file is touched. A file on another computer (a share) is never read unless the person says so, since opening it sends their credentials to it.
Resolved resolveTargetRaw(const QString &target, const QString &baseDir)
{
    Resolved r;
    QString t = target.trimmed();
    if (t.isEmpty()) return r;
    if (t.startsWith(QLatin1String("http://"), Qt::CaseInsensitive) || t.startsWith(QLatin1String("https://"), Qt::CaseInsensitive) || t.startsWith(QLatin1String("ftp://"), Qt::CaseInsensitive)) {
        r.web = true;
        return r;
    }
    if (t.startsWith(QLatin1String("file:"), Qt::CaseInsensitive)) {
        QString rest = t.mid(5);
        if (rest.startsWith(QLatin1String("//"))) {
            rest.remove(0, 2);
            if (!rest.startsWith(QLatin1Char('/')) || rest.startsWith(QLatin1String("//"))) r.network = true;   // (file://host/share/x, and file://///host/share/x)
        }
        // (only a file: address is percent-encoded; a plain path is what it is)
        QString decoded = QUrl::fromPercentEncoding(rest.toUtf8());
        if (r.network) {
            while (decoded.startsWith(QLatin1Char('/'))) decoded.remove(0, 1);
            decoded.prepend(QLatin1String("//"));
        } else if (decoded.size() > 2 && decoded[0] == QLatin1Char('/') && decoded[2] == QLatin1Char(':')) {
            decoded.remove(0, 1);   // (file:///C:/x)
        }
        r.path = decoded;
        if (!r.network && !QDir::isAbsolutePath(decoded) && !baseDir.isEmpty()) r.path = QDir(baseDir).filePath(decoded);
        return r;
    }
    if (t.startsWith(QLatin1String("\\\\")) || t.startsWith(QLatin1String("//"))) {
        r.network = true;   // (\\host\share, \\?\UNC\host\share, \\host@SSL\share, //host/share)
        r.path = t;
        return r;
    }
    r.path = t;
    if (!QDir::isAbsolutePath(t) && !baseDir.isEmpty()) r.path = QDir(baseDir).filePath(t);
    return r;
}

// What an address names, with one last check on the path that comes out of any branch (a file: address, a plain path, a relative one joined to the folder): whatever way it was written, a path that
// Windows reads as a name on another computer (two slashes or backslashes, mixed or percent-encoded, in front) is on another computer.
Resolved resolveTarget(const QString &target, const QString &baseDir)
{
    Resolved r = resolveTargetRaw(target, baseDir);
    if (r.web || r.path.isEmpty()) return r;
    QString norm = r.path;
    norm.replace(QLatin1Char('\\'), QLatin1Char('/'));
    if (norm.startsWith(QLatin1String("//"))) r.network = true;   // ("/" and "//" in front: "///host/share" is one too)
    return r;
}

// The file on this computer an address names (empty for a web address, a file on another computer, or none).
QString localPathOf(const QString &target, const QString &baseDir)
{
    const Resolved r = resolveTarget(target, baseDir);
    return r.network ? QString() : r.path;
}

// A read that may be stopped: by the clock, and by the person (Cancel).
struct Control {
    std::atomic<qint64> deadline{0};
    std::shared_ptr<std::atomic<bool>> cancel;
    void start() { deadline.store(QDateTime::currentMSecsSinceEpoch() + g_caps.readMillis); }
    bool stopped() const { return (cancel && cancel->load()) || (deadline.load() > 0 && QDateTime::currentMSecsSinceEpoch() > deadline.load()); }
};

constexpr qint64 kMaxSourceSheetBytes = qint64(256) << 20;   // the XML of one sheet of a source

// What a source gives: its sheets and names, read once.
struct Source {
    Workbook book;
    std::vector<std::shared_ptr<Sheet>> sheets;   // read when first asked
    std::vector<std::pair<quint32, quint32>> extents;
    std::mutex lock;
    std::shared_ptr<Control> control = std::make_shared<Control>();
    std::shared_ptr<std::atomic<qint64>> total;   // the cells of every source that is read
    qint64 mine = 0;
    ~Source()
    {
        if (total) total->fetch_sub(mine);
    }

    // The sheet at a place, read; null if it can't be.
    std::shared_ptr<Sheet> sheetAt(int index)
    {
        const std::lock_guard<std::mutex> guard(lock);
        if (index < 0 || size_t(index) >= sheets.size()) return nullptr;
        if (!sheets[size_t(index)]) {
            auto s = std::make_shared<Sheet>();
            control->start();
            const auto progress = [c = control](qint64) { return !c->stopped(); };
            if (!book.readSheet(index, s.get(), progress)) return nullptr;
            const qint64 cellsHere = s->cells.cellCount();
            if (total && total->load() + cellsHere > g_caps.sourceCells) return nullptr;   // (more than the sources may hold: the values the link stores are used)
            if (total) total->fetch_add(cellsHere);
            mine += cellsHere;
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
    bool allowNetwork = false;   // the person asked for this source to be read (it is on another computer)
    bool capped = false;         // the cache was left unread (the file's links hold more than is read)
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

    mutable std::shared_ptr<std::atomic<qint64>> liveCells = std::make_shared<std::atomic<qint64>>(0);
    std::shared_ptr<std::atomic<bool>> cancelFlag = std::make_shared<std::atomic<bool>>(false);
    mutable std::atomic<int> touches{0};   // every time a file was asked about or opened
    mutable std::atomic<int> hits{0};      // answers that were given again
    mutable std::atomic<quintptr> loadThread{0};   // the thread that read a file last
    struct Prefetched {
        std::shared_ptr<Source> source;
        qint64 size = -1, time = 0;
    };
    mutable std::map<QString, Prefetched> prefetched;   // sources read ahead (off the window's thread) for a link that is to point at them
    // What a formula's reference gave, by link and text (a column of formulas that sum the same range share one array): `tag` says which source it was from (null for the link's stored values).
    struct Answer {
        const void *tag = nullptr;
        Value value;
    };
    mutable std::map<QString, Answer> answers;
    mutable qint64 answerCells = 0;

    void forgetAnswers() const
    {
        answers.clear();
        answerCells = 0;
    }

    // Reads a file: null, with the reason, if it can't be (too big, damaged, too slow, stopped).
    std::shared_ptr<Source> load(const QString &path, QString *error) const
    {
        auto source = std::make_shared<Source>();
        source->total = liveCells;
        source->control->cancel = cancelFlag;
        source->control->start();
        Limits limits;
        limits.cancelled = [c = source->control] { return c->stopped(); };
        limits.cellsPerSheet = g_caps.sourceCells;
        limits.sheetBytes = kMaxSourceSheetBytes;
        touches.fetch_add(1);
        loadThread.store(quintptr(QThread::currentThreadId()));
        const bool ok = source->book.openFile(path, error, limits);
        reads.fetch_add(1);
        if (!ok) return nullptr;
        const size_t n = size_t(source->book.info().sheets.size());
        source->sheets.assign(n, nullptr);
        source->extents.assign(n, {0, 0});
        return source;
    }

    // Reads a file ahead, for a link that is about to point at it (what the person chose: a file on another computer is read too).
    bool prefetch(const QString &path, QString *why) const
    {
        touches.fetch_add(1);
        const QFileInfo info(path);
        if (!info.isFile()) {
            if (why) *why = QStringLiteral("not found");
            return false;
        }
        if (info.size() > kMaxExternalFileBytes) {
            if (why) *why = QStringLiteral("too big");
            return false;
        }
        QString error;
        std::shared_ptr<Source> source = load(path, &error);
        if (!source) {
            if (why) *why = error;
            return false;
        }
        const std::lock_guard<std::mutex> guard(lock);
        if (prefetched.size() >= 4) prefetched.clear();
        prefetched[path] = Prefetched{source, info.size(), info.lastModified().toMSecsSinceEpoch()};
        return true;
    }

    // The source of a link that is live (read now if it is not, or if its file changed): null if it can't be had. A file on another computer is not looked at unless the person allowed it.
    std::shared_ptr<Source> sourceOf(int index) const
    {
        QString path;
        {
            const std::lock_guard<std::mutex> guard(lock);
            Link &l = const_cast<Link &>(links[size_t(index)]);
            const Resolved r = resolveTarget(l.target, baseDir);
            if (r.path.isEmpty() || (r.network && !l.allowNetwork)) return nullptr;
            path = r.path;
            const qint64 now = QDateTime::currentMSecsSinceEpoch();
            if (l.source && now - l.checkedAt < checkMillis) return l.source;
            l.checkedAt = now;
        }
        // (the file is asked about with the lock let go of: a share that is slow to answer must not hold up the window)
        touches.fetch_add(1);
        const QFileInfo info(path);
        const bool isFile = info.isFile();
        const qint64 size = info.size(), time = info.lastModified().toMSecsSinceEpoch();
        {
            const std::lock_guard<std::mutex> guard(lock);
            Link &l = const_cast<Link &>(links[size_t(index)]);
            if (!isFile) {
                l.source.reset();
                l.signSize = -1;
                return nullptr;
            }
            if ((l.source || l.unreadable) && size == l.signSize && time == l.signTime) return l.source;
            l.source.reset();
            l.unreadable = false;
            if (size > kMaxExternalFileBytes) {
                l.unreadable = true;
                l.signSize = size;
                l.signTime = time;
                return nullptr;
            }
            const auto ahead = prefetched.find(path);
            if (ahead != prefetched.end() && ahead->second.size == size && ahead->second.time == time) {
                l.source = ahead->second.source;
                l.signSize = size;
                l.signTime = time;
                l.readTime = QDateTime::currentDateTime();
                prefetched.erase(ahead);
                forgetAnswers();
                return l.source;
            }
        }
        // (read with the lock let go of: a big file takes a while, and the window asks for the state of its links meanwhile)
        QString error;
        std::shared_ptr<Source> source = load(path, &error);
        const std::lock_guard<std::mutex> guard(lock);
        Link &l = const_cast<Link &>(links[size_t(index)]);
        l.signSize = size;
        l.signTime = time;
        forgetAnswers();
        if (!source) {
            l.unreadable = true;
            return nullptr;
        }
        l.source = source;
        l.readTime = QDateTime::currentDateTime();
        // (what the formulas read of the source before stays listed: the cache the next save writes does not shrink because a cell was not read this time)
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
void readCache(const QByteArray &xml, Link *l, qint64 *cellBudget)
{
    QXmlStreamReader r(xml);
    int sheetId = -1;
    quint32 row = 0;
    bool inCell = false, inV = false;
    QString ref, type, text;
    qint64 count = 0;
    while (!r.atEnd() && count < kMostCacheCells && *cellBudget > 0) {
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
                    --*cellBudget;
                }
            }
        }
    }
    // (a link whose values do not all fit is left out whole: part of its values would be a wrong answer)
    if (*cellBudget <= 0 && !r.atEnd()) {
        l->cells.clear();
        l->extents.clear();
        l->names.clear();
        l->sheetNames.clear();
        l->capped = true;
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
    qint64 cellBudget = g_caps.cacheCells, byteBudget = g_caps.cacheBytes;   // (what all the links' stored values may come to: the links past it are not read, and their formulas keep what the file stored)
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
            const qint64 size = package.partSize(rel.target);
            if (size >= 0 && size < (64 << 20) && size <= byteBudget && cellBudget > 0) {
                byteBudget -= size;
                readCache(package.part(rel.target), &l, &cellBudget);
            } else {
                l.capped = true;
            }
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

QString ExternalBooks::filePath(int index) const
{
    const std::lock_guard<std::mutex> guard(d->lock);
    return index >= 1 && index <= int(d->links.size()) ? resolveTarget(d->links[size_t(index - 1)].target, d->baseDir).path : QString();
}

bool ExternalBooks::isNetwork(int index) const
{
    const std::lock_guard<std::mutex> guard(d->lock);
    return index >= 1 && index <= int(d->links.size()) && resolveTarget(d->links[size_t(index - 1)].target, d->baseDir).network;
}

bool ExternalBooks::allowNetwork(int index) const
{
    const std::lock_guard<std::mutex> guard(d->lock);
    return index >= 1 && index <= int(d->links.size()) && d->links[size_t(index - 1)].allowNetwork;
}

void ExternalBooks::setAllowNetwork(int index, bool on)
{
    const std::lock_guard<std::mutex> guard(d->lock);
    if (index >= 1 && index <= int(d->links.size())) d->links[size_t(index - 1)].allowNetwork = on;
}

bool ExternalBooks::prefetch(const QString &path, QString *why) { return d->prefetch(path, why); }

void ExternalBooks::cancelReads() { d->cancelFlag->store(true); }

void ExternalBooks::resetCancel() { d->cancelFlag->store(false); }

int ExternalBooks::fileTouches() const { return d->touches.load(); }

int ExternalBooks::answersShared() const { return d->hits.load(); }

quintptr ExternalBooks::lastReadThread() const { return d->loadThread.load(); }

void ExternalBooks::setCapsForTest(qint64 cacheCells, qint64 cacheBytes, qint64 sourceCells, qint64 readMillis)
{
    g_caps.cacheCells = cacheCells;
    g_caps.cacheBytes = cacheBytes;
    g_caps.sourceCells = sourceCells;
    g_caps.readMillis = readMillis;
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
    l.allowNetwork = false;
    d->forgetAnswers();
}

ExternalBooks::State ExternalBooks::state(int index) const
{
    QString path;
    bool unreadable = false;
    QDateTime ref;
    {
        const std::lock_guard<std::mutex> guard(d->lock);
        if (index < 1 || index > int(d->links.size())) return State::Missing;
        const Link &l = d->links[size_t(index - 1)];
        const Resolved r = resolveTarget(l.target, d->baseDir);
        if (r.path.isEmpty()) return State::Missing;
        if (r.network && !l.allowNetwork) return State::Network;   // (not asked about: that would send the person's credentials to another computer)
        path = r.path;
        unreadable = l.unreadable;
        ref = l.readTime.isValid() ? l.readTime : d->fileTime;
    }
    d->touches.fetch_add(1);
    const QFileInfo info(path);
    if (!info.isFile()) return State::Missing;
    if (unreadable) return State::Unreadable;
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
    d->forgetAnswers();
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
    d->forgetAnswers();
}

bool ExternalBooks::probe(int index, QString *why, bool refresh)
{
    if (index < 1 || index > count()) {
        if (why) *why = QStringLiteral("no such link");
        return false;
    }
    if (refresh) invalidate(index);
    const bool ok = d->sourceOf(index - 1) != nullptr;
    if (!ok && why) *why = state(index) == State::Missing ? QStringLiteral("not found") : state(index) == State::Network ? QStringLiteral("on another computer") : QStringLiteral("could not be read");
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
    // (the answer a formula had for this reference is given to the next that asks the same, from the same source: a column of formulas that sum one range share its array)
    const void *tag = live ? d->sourceOf(p.link - 1).get() : nullptr;
    const QString key = QString::number(p.link) + QLatin1Char('|') + text;
    {
        const std::lock_guard<std::mutex> guard(d->lock);
        const auto it = d->answers.find(key);
        if (it != d->answers.end() && it->second.tag == tag) {
            *out = it->second.value;
            d->hits.fetch_add(1);
            if (fromFile) *fromFile = tag != nullptr;
            return true;
        }
    }
    bool ok = false, file = false;
    if (live && d->fromSource(p.link - 1, p, out)) {
        ok = file = true;
    } else {
        ok = d->fromCache(p.link - 1, p, out);
    }
    if (fromFile) *fromFile = file;
    if (ok && out->isArray()) {
        const std::lock_guard<std::mutex> guard(d->lock);
        const qint64 cells = qint64(out->rows()) * out->cols();
        if (d->answerCells + cells > 4000000) d->forgetAnswers();
        d->answers[key] = Impl::Answer{file ? tag : nullptr, *out};
        d->answerCells += cells;
    }
    return ok;
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
