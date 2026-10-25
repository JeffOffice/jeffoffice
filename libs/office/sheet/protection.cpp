#include "sheet/protection.h"

#include <QCryptographicHash>
#include <QRandomGenerator>

namespace jp::office::sheet {

namespace {

bool algorithmOf(const QString &name, QCryptographicHash::Algorithm *out)
{
    const QString n = name.trimmed().toUpper();
    if (n == QLatin1String("SHA-512") || n == QLatin1String("SHA512")) *out = QCryptographicHash::Sha512;
    else if (n == QLatin1String("SHA-256") || n == QLatin1String("SHA256")) *out = QCryptographicHash::Sha256;
    else if (n == QLatin1String("SHA-384") || n == QLatin1String("SHA384")) *out = QCryptographicHash::Sha384;
    else if (n == QLatin1String("SHA-1") || n == QLatin1String("SHA1")) *out = QCryptographicHash::Sha1;
    else if (n == QLatin1String("MD5")) *out = QCryptographicHash::Md5;
    else return false;
    return true;
}

QByteArray utf16le(const QString &text)
{
    QByteArray out;
    out.reserve(text.size() * 2);
    for (const QChar c : text) {
        out.append(char(c.unicode() & 0xFF));
        out.append(char(c.unicode() >> 8));
    }
    return out;
}

// H(salt + password), then H(previous + the repeat's number as four bytes, little end first), `spin` times.
bool spinHash(QCryptographicHash::Algorithm algorithm, const QByteArray &salt, const QString &password, qint64 spin, const std::atomic<bool> *stop, QByteArray *out)
{
    QByteArray h = QCryptographicHash::hash(salt + utf16le(password), algorithm);
    for (qint64 i = 0; i < spin; ++i) {
        if ((i & 4095) == 0 && stop && stop->load()) return false;
        QByteArray n(4, 0);
        n[0] = char(i & 0xFF);
        n[1] = char((i >> 8) & 0xFF);
        n[2] = char((i >> 16) & 0xFF);
        n[3] = char((i >> 24) & 0xFF);
        h = QCryptographicHash::hash(h + n, algorithm);
    }
    *out = h;
    return true;
}

} // namespace

QString legacyPasswordHash(const QString &password)
{
    // (the older form takes the first 15 characters of a password, so no shift is past 15 places)
    const QString used = password.left(15);
    quint32 hash = 0;
    int index = 1;
    for (const QChar c : used) {
        quint32 value = quint32(c.unicode()) << index;
        const quint32 rotated = value >> 15;
        value &= 0x7FFF;
        hash ^= (value | rotated);
        ++index;
    }
    hash ^= quint32(used.size());
    hash ^= 0xCE4B;
    return QStringLiteral("%1").arg(hash & 0xFFFF, 4, 16, QLatin1Char('0')).toUpper();
}

PasswordResult checkPassword(const PasswordHash &hash, const QString &password, const std::atomic<bool> *stop)
{
    if (hash.hash.isEmpty()) {
        if (hash.legacy.isEmpty()) return PasswordResult::Match;   // (no password)
        bool ok = false;
        const uint stored = hash.legacy.trimmed().toUInt(&ok, 16);
        if (!ok) return PasswordResult::Unsupported;
        bool sameOk = false;
        const uint now = legacyPasswordHash(password).toUInt(&sameOk, 16);
        return sameOk && now == stored ? PasswordResult::Match : PasswordResult::Wrong;
    }
    QCryptographicHash::Algorithm algorithm;
    if (!algorithmOf(hash.algorithm, &algorithm)) return PasswordResult::Unsupported;
    if (hash.spin < 0 || hash.spin > kMaxSpinCount) return PasswordResult::TooSlow;
    const QByteArray salt = QByteArray::fromBase64(hash.salt.toLatin1());
    QByteArray h;
    if (!spinHash(algorithm, salt, password, hash.spin, stop, &h)) return PasswordResult::Stopped;
    return h == QByteArray::fromBase64(hash.hash.toLatin1()) ? PasswordResult::Match : PasswordResult::Wrong;
}

bool makePasswordHash(const QString &password, PasswordHash *out, const std::atomic<bool> *stop)
{
    QByteArray salt(16, 0);
    for (int i = 0; i < 16; i += 4) {
        const quint32 r = QRandomGenerator::system()->generate();
        for (int k = 0; k < 4; ++k) salt[i + k] = char((r >> (8 * k)) & 0xFF);
    }
    QByteArray h;
    if (!spinHash(QCryptographicHash::Sha512, salt, password, kNewSpinCount, stop, &h)) return false;
    out->algorithm = QStringLiteral("SHA-512");
    out->hash = QString::fromLatin1(h.toBase64());
    out->salt = QString::fromLatin1(salt.toBase64());
    out->spin = kNewSpinCount;
    out->legacy.clear();
    return true;
}

bool SheetProtection::operator==(const SheetProtection &o) const
{
    return enabled == o.enabled && objects == o.objects && scenarios == o.scenarios && allowSelectLocked == o.allowSelectLocked && allowSelectUnlocked == o.allowSelectUnlocked
           && allowFormatCells == o.allowFormatCells && allowFormatColumns == o.allowFormatColumns && allowFormatRows == o.allowFormatRows && allowInsertColumns == o.allowInsertColumns
           && allowInsertRows == o.allowInsertRows && allowInsertLinks == o.allowInsertLinks && allowDeleteColumns == o.allowDeleteColumns && allowDeleteRows == o.allowDeleteRows
           && allowSort == o.allowSort && allowFilter == o.allowFilter && allowPivot == o.allowPivot && password == o.password;
}

namespace {

QByteArray hashAttributes(const PasswordHash &p, const char *algorithmName, const char *hashName, const char *saltName, const char *spinName, const char *legacyName)
{
    QByteArray a;
    if (!p.hash.isEmpty()) {
        a += QByteArray(" ") + algorithmName + "=\"" + p.algorithm.toUtf8() + "\" " + hashName + "=\"" + p.hash.toLatin1() + "\" " + saltName + "=\"" + p.salt.toLatin1() + "\" " + spinName + "=\""
             + QByteArray::number(p.spin) + "\"";
    } else if (!p.legacy.isEmpty()) {
        a += QByteArray(" ") + legacyName + "=\"" + p.legacy.toLatin1() + "\"";
    }
    return a;
}

} // namespace

QByteArray sheetProtectionElement(const SheetProtection &p, const QByteArray &prefix)
{
    if (!p.enabled) return QByteArray();
    QByteArray e = "<" + prefix + "sheetProtection" + hashAttributes(p.password, "algorithmName", "hashValue", "saltValue", "spinCount", "password") + " sheet=\"1\"";
    if (p.objects) e += " objects=\"1\"";
    if (p.scenarios) e += " scenarios=\"1\"";
    // (set in the file is "not allowed"; the formats and the rest are not allowed unless the file says "0")
    auto locked = [&](const char *name, bool allowed) { e += QByteArray(" ") + name + (allowed ? "=\"0\"" : "=\"1\""); };
    locked("formatCells", p.allowFormatCells);
    locked("formatColumns", p.allowFormatColumns);
    locked("formatRows", p.allowFormatRows);
    locked("insertColumns", p.allowInsertColumns);
    locked("insertRows", p.allowInsertRows);
    locked("insertHyperlinks", p.allowInsertLinks);
    locked("deleteColumns", p.allowDeleteColumns);
    locked("deleteRows", p.allowDeleteRows);
    if (!p.allowSelectLocked) e += " selectLockedCells=\"1\"";
    locked("sort", p.allowSort);
    locked("autoFilter", p.allowFilter);
    locked("pivotTables", p.allowPivot);
    if (!p.allowSelectUnlocked) e += " selectUnlockedCells=\"1\"";
    e += "/>";
    return e;
}

QByteArray workbookProtectionElement(const WorkbookProtection &p, const QByteArray &prefix)
{
    if (!p.any()) return QByteArray();
    QByteArray e = "<" + prefix + "workbookProtection" + hashAttributes(p.password, "workbookAlgorithmName", "workbookHashValue", "workbookSaltValue", "workbookSpinCount", "workbookPassword");
    if (p.lockStructure) e += " lockStructure=\"1\"";
    if (p.lockWindows) e += " lockWindows=\"1\"";
    e += "/>";
    return e;
}

} // namespace jp::office::sheet
