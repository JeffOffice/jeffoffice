#include "core/mergeextras.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QMessageAuthenticationCode>
#include <QRandomGenerator>
#include <QSaveFile>
#include <QFileInfo>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSet>
#include <QStringList>
#include <algorithm>
#include <cmath>

namespace jp {

namespace {

int keyColumnOf(const MergeSource &m)
{
    if (m.keyColumn.isEmpty()) return -1;
    for (int c = 0; c < m.fields.size(); ++c)
        if (m.fields[c].trimmed().compare(m.keyColumn.trimmed(), Qt::CaseInsensitive) == 0) return c;
    return -1;
}

QString tr(const char *text, int n = -1) { return QCoreApplication::translate("MergeExclusions", text, nullptr, n); }

} // namespace

QString mergeKeyFingerprint(const QString &value, const QString &salt)
{
    const QByteArray mac = QMessageAuthenticationCode::hash(value.trimmed().toCaseFolded().toUtf8(), QByteArray::fromHex(salt.toLatin1()), QCryptographicHash::Sha256);
    return QString::fromLatin1(mac.toHex().left(16));
}

QString newExclusionSalt()
{
    quint32 words[8];
    QRandomGenerator::system()->fillRange(words);
    return QString::fromLatin1(QByteArray(reinterpret_cast<const char *>(words), sizeof words).toHex());
}

MergeExclusions exclusionsOf(const MergeSource &m, const QString &salt)
{
    MergeExclusions e;
    e.rows = m.rows.size();
    e.salt = salt.size() == kSaltDigits ? salt : newExclusionSalt();
    const int key = keyColumnOf(m);
    if (key >= 0) e.keyColumn = m.fields[key];
    for (int r = 0; r < m.rows.size() && e.keys.size() + e.places.size() < kMostSavedExclusions; ++r) {
        if (r >= m.include.size() || m.include[r]) continue;
        if (key >= 0) e.keys << mergeKeyFingerprint(m.rows[r].value(key), e.salt);
        else if (const int place = m.place.size() == m.rows.size() ? m.place[r] : r; place >= 0) e.places << place;
    }
    if (e.isEmpty()) e.salt.clear();
    return e;
}

QByteArray exclusionsToBytes(const MergeExclusions &e)
{
    if (e.isEmpty()) return QByteArray();
    QJsonObject o{{"version", 1}, {"rows", double(e.rows)}, {"salt", e.salt}};
    if (!e.keyColumn.isEmpty()) o["key"] = e.keyColumn.left(kMostKeyColumnChars);
    if (!e.keys.isEmpty()) o["keys"] = QJsonArray::fromStringList(e.keys);
    if (!e.places.isEmpty()) {
        QJsonArray a;
        for (int p : e.places) a.append(p);
        o["places"] = a;
    }
    return QJsonDocument(o).toJson(QJsonDocument::Compact);
}

bool exclusionsFromBytes(const QByteArray &bytes, MergeExclusions *out)
{
    if (bytes.isEmpty() || bytes.size() > kMostExclusionBytes) return false;
    const QJsonObject o = QJsonDocument::fromJson(bytes).object();
    if (o.value("version").toInt() != 1) return false;
    MergeExclusions e;
    // (a size is read as a number and checked before it is made a whole number: a file can say anything)
    const double rows = o.value("rows").toDouble(-1);
    if (!(rows >= 0 && rows <= double(kMostSavedListRows)) || rows != std::floor(rows)) return false;
    e.rows = qint64(rows);
    e.salt = o.value("salt").toString();
    e.keyColumn = o.value("key").toString().left(kMostKeyColumnChars);
    const QJsonArray keys = o.value("keys").toArray(), places = o.value("places").toArray();
    if (keys.size() + places.size() > kMostSavedExclusions) return false;
    for (const QJsonValue &k : keys) e.keys << k.toString().left(16);
    for (const QJsonValue &p : places) {
        const double place = p.toDouble(-1);
        if (!(place >= 0 && place < double(e.rows)) || place != std::floor(place)) return false;
        e.places << int(place);
    }
    if (e.keyColumn.isEmpty() && !e.keys.isEmpty()) return false;
    if (!e.keys.isEmpty() || !e.places.isEmpty()) {
        static const QRegularExpression digits(QStringLiteral("^[0-9a-f]{%1}$").arg(kSaltDigits));
        if (!digits.match(e.salt).hasMatch()) return false;
    }
    *out = e;
    return true;
}

ExclusionResult applyExclusions(MergeSource &m, const MergeExclusions &saved)
{
    ExclusionResult res;
    if (saved.isEmpty()) return res;
    m.keyColumn = saved.keyColumn;
    while (m.include.size() < m.rows.size()) m.include << true;
    if (!saved.keys.isEmpty()) {
        const int key = keyColumnOf(m);
        if (key < 0) {
            res.problem = tr("The list has no column named \"%1\" now, so the recipients you left out before could not be found. All recipients are included.").arg(saved.keyColumn);
            return res;
        }
        QHash<QString, int> needed;
        for (const QString &k : saved.keys) ++needed[k];
        QHash<QString, QVector<int>> found;
        for (int r = 0; r < m.rows.size(); ++r) {
            const QString f = mergeKeyFingerprint(m.rows[r].value(key), saved.salt);
            if (needed.contains(f)) found[f] << r;
        }
        int gone = 0, repeated = 0;
        for (auto it = needed.cbegin(); it != needed.cend(); ++it) {
            const QVector<int> rows = found.value(it.key());
            if (rows.isEmpty()) gone += it.value();
            else if (rows.size() != it.value()) repeated += it.value();
            else for (int r : rows) { m.include[r] = false; ++res.applied; }
        }
        QStringList said;
        if (gone > 0) said << tr("%n recipient you left out before is no longer in the list.", gone);
        if (repeated > 0) said << tr("%n recipient you left out before could not be matched, because other recipients have the same %1. That recipient is included.", repeated).arg(saved.keyColumn);
        res.problem = said.join(QLatin1Char(' '));
        return res;
    }
    if (saved.rows != m.rows.size()) {
        res.problem = tr("The list has %n recipient now. It had %1 when you chose which recipients to leave out, so all recipients are included.", int(m.rows.size())).arg(saved.rows);
        return res;
    }
    for (int p : saved.places)
        if (p >= 0 && p < m.rows.size()) { m.include[p] = false; ++res.applied; }
    return res;
}

// ---- one file for each recipient

QString safeMergeFileName(const QString &text, const QString &fallback)
{
    QString s;
    for (const QChar c : text.left(4 * kMostMergeNameChars)) {
        const ushort u = c.unicode();
        const bool bad = u < 32 || u == 127 || (u >= 0x80 && u < 0xA0) || u == '/' || u == '\\' || u == ':' || u == '*' || u == '?' || u == '"' || u == '<' || u == '>' || u == '|' ||
                         u == 0x2028 || u == 0x2029 || (u >= 0x202A && u <= 0x202E) || (u >= 0x2066 && u <= 0x2069) || u == 0xFEFF || u == 0xFFFE || u == 0xFFFF;
        s += bad ? QLatin1Char(' ') : c;
    }
    s = s.simplified();
    // (a high surrogate cut off from its pair is dropped, so the name stays valid text)
    if (s.size() > kMostMergeNameChars) {
        s.truncate(kMostMergeNameChars);
        if (s.back().isHighSurrogate()) s.chop(1);
    }
    while (!s.isEmpty() && (s.front() == QLatin1Char('.') || s.front() == QLatin1Char(' '))) s.remove(0, 1);
    while (!s.isEmpty() && (s.back() == QLatin1Char('.') || s.back() == QLatin1Char(' '))) s.chop(1);
    static const QRegularExpression reserved(QStringLiteral("^(con|prn|aux|nul|com[0-9]|lpt[0-9])(\\..*)?$"), QRegularExpression::CaseInsensitiveOption);
    if (reserved.match(s).hasMatch()) s.prepend(QLatin1Char('_'));
    // (a name is bounded in bytes too: every system takes 255)
    while (s.toUtf8().size() > kMostMergeNameBytes && !s.isEmpty()) s.chop(1);
    if (!s.isEmpty() && s.back().isHighSurrogate()) s.chop(1);
    return s.isEmpty() ? fallback : s;
}

QString defaultFileNamePattern(const MergeSource &m)
{
    auto find = [&](std::initializer_list<const char *> names) -> QString {
        for (const char *n : names)
            for (const QString &f : m.fields)
                if (f.trimmed().compare(QLatin1String(n), Qt::CaseInsensitive) == 0) return f;
        return QString();
    };
    const QString last = find({"Last Name", "Last", "Surname", "Family Name"}), first = find({"First Name", "First", "Given Name"});
    if (!last.isEmpty() && !first.isEmpty()) return QStringLiteral("{") + last + QStringLiteral("}, {") + first + QStringLiteral("}");
    if (!last.isEmpty()) return QStringLiteral("{") + last + QStringLiteral("}");
    return m.fields.isEmpty() ? QString() : QStringLiteral("{") + m.fields.first() + QStringLiteral("}");
}

QStringList mergeFileNames(const MergeSource &m, const QVector<int> &rows, const QString &pattern, const QString &extension)
{
    QHash<QString, int> columns;
    for (int c = 0; c < m.fields.size(); ++c) columns.insert(m.fields[c].trimmed().toLower(), c);
    static const QRegularExpression field(QStringLiteral("\\{([^{}]{1,%1})\\}").arg(kMostKeyColumnChars));
    QStringList out;
    QSet<QString> used;
    const QString ext = extension.left(16);
    const QString safeExt = ext.contains(QLatin1Char('/')) || ext.contains(QLatin1Char('\\')) ? QString() : ext;
    for (int row : rows) {
        QString name;
        int at = 0;
        const QString p = pattern.left(1000);
        for (auto it = field.globalMatch(p); it.hasNext();) {
            const QRegularExpressionMatch match = it.next();
            name += p.mid(at, match.capturedStart() - at);
            at = match.capturedEnd();
            const int c = columns.value(match.captured(1).trimmed().toLower(), -1);
            // (each value is cut to a name's size before it is joined, so a huge value cannot make a huge string)
            if (c >= 0) name += safeMergeFileName(m.rows.value(row).value(c).left(kMostMergeNameChars + 1), QString());
        }
        name += p.mid(at);
        QString base = safeMergeFileName(name, QString());
        // (a name of nothing but marks, what a pattern makes of empty values, is no name)
        if (std::none_of(base.cbegin(), base.cend(), [](QChar ch) { return ch.isLetterOrNumber(); })) base.clear();
        if (base.isEmpty()) base = QCoreApplication::translate("MergeFiles", "Recipient %1").arg(row + 1);
        QString candidate = base + safeExt;
        // (compared as a system compares: case folded, and an accented letter made of a letter and a mark is the one letter)
        auto same = [](const QString &name) { return name.normalized(QString::NormalizationForm_C).toCaseFolded().normalized(QString::NormalizationForm_C); };
        for (int n = 2; used.contains(same(candidate)); ++n) candidate = base + QStringLiteral(" (%1)").arg(n) + safeExt;
        used.insert(same(candidate));
        out << candidate;
    }
    return out;
}

bool nameStaysInFolder(const QString &folder, const QString &name)
{
    if (name.isEmpty() || name == QLatin1String(".") || name == QLatin1String("..")) return false;
    if (name.contains(QLatin1Char('/')) || name.contains(QLatin1Char('\\')) || name.contains(QLatin1Char(':'))) return false;
    const QString root = QDir::cleanPath(QFileInfo(folder).absoluteFilePath());
    const QString full = QDir::cleanPath(QFileInfo(root + QLatin1Char('/') + name).absoluteFilePath());
    return QFileInfo(full).absolutePath() == root;
}

int mergeFilesThatExist(const QString &folder, const QStringList &names)
{
    int n = 0;
    const QDir dir(folder);
    for (const QString &name : names)
        if (nameStaysInFolder(folder, name) && (QFileInfo::exists(dir.filePath(name)) || QFileInfo(dir.filePath(name)).isSymLink())) ++n;
    return n;
}

bool writeMergeFile(const QString &path, const QByteArray &bytes, QString *error)
{
    // (isSymLink looks at the link itself, so a link that leads nowhere is found too; QSaveFile would write through a link to what it leads to)
    if (QFileInfo(path).isSymLink() && !QFile::remove(path)) {
        if (error) *error = QCoreApplication::translate("MergeFiles", "A shortcut with that file's name could not be replaced.");
        return false;
    }
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size() || !file.commit()) {
        if (error) *error = file.errorString();
        return false;
    }
    return true;
}

MergeFilesResult makeMergeFiles(const QString &folder, const QVector<int> &rows, const QStringList &names, const MakeMergeFile &make, const std::function<bool(int, int)> &progress)
{
    MergeFilesResult res;
    if (rows.size() > kMostMergeFiles || names.size() != rows.size()) {
        res.error = QCoreApplication::translate("MergeFiles", "A merge makes at most %n file at a time. Use a shorter list.", nullptr, kMostMergeFiles);
        return res;
    }
    if (!QDir().mkpath(folder)) {
        res.error = QCoreApplication::translate("MergeFiles", "The folder can't be made.");
        return res;
    }
    const QDir dir(folder);
    for (int i = 0; i < rows.size(); ++i) {
        if (progress && !progress(i, int(rows.size()))) {
            res.stopped = true;
            return res;
        }
        // (the last guard: a name never leaves the folder, whatever made it)
        if (!nameStaysInFolder(folder, names[i])) {
            res.error = QCoreApplication::translate("MergeFiles", "A file name is not allowed: %1").arg(safeMergeFileName(names[i], QStringLiteral("?")));
            return res;
        }
        QString why;
        if (!make(rows[i], dir.filePath(names[i]), &why)) {
            res.error = QCoreApplication::translate("MergeFiles", "%1 can't be saved.\n%2").arg(names[i], why);
            return res;
        }
        ++res.written;
    }
    return res;
}

} // namespace jp
