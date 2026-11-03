#include "sheet/sheetcrypt.h"

#include "io/crypto.h"
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
    case office::CryptResult::TooCostly: return QCoreApplication::translate("Sheet", "This file's password protection asks for more memory or time than this program allows, so it is not opened.");
    case office::CryptResult::Cancelled: return QCoreApplication::translate("Sheet", "The password was not checked: the read was stopped.");
    case office::CryptResult::TooLarge: return QCoreApplication::translate("jp::office::doc", "The file is larger than this program can open.");
    default: return QCoreApplication::translate("Sheet", "The file is damaged, or it was changed after it was protected: its protected contents can't be read.");
    }
}

} // namespace

namespace {
// The password the command-line tools were given: held in a static that wipes itself when the program ends.
struct Fallback {
    QString password;
    ~Fallback() { jp::crypto::wipe(password); }
};
Fallback &fallback()
{
    static Fallback f;
    return f;
}
} // namespace

void setFallbackPassword(const QString &password)
{
    jp::crypto::wipe(fallback().password);
    fallback().password = password;
}

bool passwordGate(const Limits &limits, QString *error, const std::function<office::CryptResult(const QString &)> &attempt)
{
    const QString &chosen = !limits.password.isEmpty() ? limits.password : fallback().password;
    const bool given = !chosen.isEmpty();
    // (the password every program knows is tried first: a file protected only against changes by accident opens with no question, whatever was typed)
    office::CryptResult r = attempt(office::defaultPassword());
    if (r == office::CryptResult::Ok) {
        if (limits.usedDefaultPassword) *limits.usedDefaultPassword = true;
        return true;
    }
    if (given && r == office::CryptResult::WrongPassword) {
        r = attempt(chosen);
        if (r == office::CryptResult::Ok) return true;
    }
    const PasswordNeed need = r != office::CryptResult::WrongPassword ? PasswordNeed::None : given ? PasswordNeed::Wrong : PasswordNeed::Needed;
    if (limits.passwordNeed) *limits.passwordNeed = need;
    if (error) *error = protectedMessage(r, need);
    return false;
}

} // namespace jp::office::sheet
