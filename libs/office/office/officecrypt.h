#pragma once
// Workbooks, documents, and presentations that are protected with a password in the form of the XML packages: a compound file holding the stream "EncryptionInfo" (how the key is made from
// the password) and the stream "EncryptedPackage" (the zip package, enciphered). Two forms are read: the agile one (AES with a 128-, 192-, or 256-bit key, and SHA-1, SHA-256, SHA-384, or
// SHA-512, with a check that nothing was changed), and the older standard one (AES in blocks with a SHA-1 key). A package is written in the first form, with AES-256 and SHA-512.
// A password is a string; it is turned into bytes here, used, and not kept.

#include <QByteArray>
#include <QString>

namespace jp::office {

enum class CryptResult {
    Ok,
    WrongPassword,
    Unsupported,   // a form of protection this program does not read (another cipher, or a key held by a certificate)
    Damaged,       // sizes that do not fit, a stream cut short, or an agile package whose check of changes fails
    TooLarge,      // more than the program reads
};

// The password that files protected only against changes by accident are made with: a file opens with it when no password is given.
QString defaultPassword();

// The most rounds of hashing a file may ask for to make a key (the form's own limit).
constexpr qint64 kMaxPackageSpinCount = 10000000;

// Decrypts the package. `info` and `package` are the two streams. With `verifyOnly` only the password is checked (a few thousand to a million hashes, no bulk work) and `zip` is left alone.
// A package that would come to more than `maxBytes` is TooLarge.
CryptResult decryptPackage(const QByteArray &info, const QByteArray &package, const QString &password, qint64 maxBytes, bool verifyOnly, QByteArray *zip);

// How a package is protected when it is written (the agile form): the key's size in bits (128, 192, or 256), the hash ("SHA1", "SHA256", "SHA384", or "SHA512"), and the rounds of hashing
// (the tests use a small number).
struct EncryptOptions {
    int keyBits = 256;
    QString hash = QStringLiteral("SHA512");
    int spinCount = 100000;
};

// The compound file (as bytes) that holds `zip` protected by `password`, in the agile form. Empty if the options are not any of the above.
QByteArray encryptPackage(const QByteArray &zip, const QString &password, const EncryptOptions &options = EncryptOptions());

// The same for the standard form, with AES-128 and SHA-1 (the tests make their files with it).
QByteArray encryptPackageStandard(const QByteArray &zip, const QString &password);

} // namespace jp::office
