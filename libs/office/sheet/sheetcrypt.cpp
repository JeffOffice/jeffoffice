#include "sheet/sheetcrypt.h"

#include "io/cfb.h"
#include "office/package.h"
#include "sheet/odscrypt.h"
#include "sheet/xlscrypt.h"
#include "sheet/xls.h"

#include <QCoreApplication>
#include <QFile>

namespace jp::office::sheet {

namespace {

// What a protected file says when it is refused, and why (see Limits::passwordNeed): no password given, a wrong one, or a protection that is not read.
QString protectedMessage(office::CryptResult why, PasswordNeed need)
{
    switch (why) {
    case office::CryptResult::WrongPassword:
        return need == PasswordNeed::Needed ? QCoreApplication::translate("Sheet", "This file needs a password: it is protected with a password, and none was given.")
                                            : QCoreApplication::translate("Sheet", "The password is not correct.");
    case office::CryptResult::Unsupported: return QCoreApplication::translate("Sheet", "This file is protected in a way this program can't open (not a password, or a kind of protection it does not read).");
    case office::CryptResult::TooLarge: return QCoreApplication::translate("jp::office::doc", "The file is larger than this program can open.");
    default: return QCoreApplication::translate("Sheet", "The file is damaged, or it was changed after it was protected: its protected contents can't be read.");
    }
}

} // namespace

namespace {
QString &fallback()
{
    static QString password;
    return password;
}
} // namespace

void setFallbackPassword(const QString &password) { fallback() = password; }

bool passwordGate(const Limits &limits, QString *error, const std::function<office::CryptResult(const QString &)> &attempt)
{
    const QString &chosen = !limits.password.isEmpty() ? limits.password : fallback();
    const bool given = !chosen.isEmpty();
    const office::CryptResult r = attempt(given ? chosen : office::defaultPassword());
    if (r == office::CryptResult::Ok) return true;
    const PasswordNeed need = r != office::CryptResult::WrongPassword ? PasswordNeed::None : given ? PasswordNeed::Wrong : PasswordNeed::Needed;
    if (limits.passwordNeed) *limits.passwordNeed = need;
    if (error) *error = protectedMessage(r, need);
    return false;
}

PasswordNeed passwordCheck(const QString &path, const QString &password)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return PasswordNeed::None;
    const QByteArray head = file.read(16384);
    file.close();
    const bool given = !password.isEmpty();
    auto verdict = [&](CryptResult r) {
        if (r == CryptResult::Ok) return PasswordNeed::None;
        if (r == CryptResult::WrongPassword) return given ? PasswordNeed::Wrong : PasswordNeed::Needed;
        return PasswordNeed::None;   // (a protection that is not read, or damage: opening says so)
    };
    const QString pw = given ? password : defaultPassword();
    if (startsCompoundFile(head)) {
        const QByteArray info = jp::cfb::readStream(path, QStringLiteral("EncryptionInfo"));
        if (!info.isEmpty()) return verdict(decryptPackage(info, QByteArray(), pw, 0, true, nullptr));
        QByteArray stream = jp::cfb::readStream(path, QStringLiteral("Workbook"));
        if (stream.isEmpty()) stream = jp::cfb::readStream(path, QStringLiteral("Book"));
        if (!stream.isEmpty() && biffEncrypted(stream)) return verdict(decryptBiff(&stream, pw, true));
        return PasswordNeed::None;
    }
    // (only an OpenDocument package can be protected: its first part is the one named mimetype)
    if (head.startsWith(QByteArrayView("PK\x03\x04", 4)) && head.mid(30, 8) == "mimetype") {
        QByteArray bytes;
        if (!readWholeFile(path, &bytes, nullptr, kMaxOpenFileBytes) || !odsEncrypted(bytes)) return PasswordNeed::None;
        return verdict(decryptOds(bytes, pw, 0, true, nullptr));
    }
    return PasswordNeed::None;
}

} // namespace jp::office::sheet
