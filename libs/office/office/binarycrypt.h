#pragma once
// What the old binary forms of protection share (a workbook's FILEPASS record, a document's table stream): the obfuscation that XORs the bytes with a table made from the password, and the
// two kinds of RC4 (a 40-bit key from the password's MD5, and a key made by the cryptographic provider, CryptoAPI). The callers walk their own streams; this makes the keys.

#include "office/officecrypt.h"

#include <QByteArray>
#include <QString>

#include <array>

namespace jp::office::bincrypt {

// The password as the obfuscation takes it: the first 15 characters, one byte each. False for a password with a letter outside the Western set (Latin-1).
bool ansiPassword(const QString &password, QByteArray *out);
quint16 xorKey(const QByteArray &pw);
quint16 xorVerifier(const QByteArray &pw);
std::array<quint8, 16> xorArray(const QByteArray &pw, quint16 key);

// Wipes the secrets it was given when it goes out of scope, whatever way the function leaves.
struct Wiper {
    QByteArray *items[8] = {};
    int count = 0;
    void add(QByteArray *b) { items[count++] = b; }
    ~Wiper();
};

// How the keys of the blocks are made, for the two forms.
struct Rc4Keys {
    bool cryptoApi = false;
    QByteArray base;      // the hash that the block number is added to
    int keyBytes = 16;
    QByteArray forBlock(quint32 block) const;
};

// `fp` is the form's header as a workbook's FILEPASS record holds it: two bytes (the kind of protection, not read here), the version (major, minor, two bytes each), and then the form's own
// fields. Ok if the password fits; the keys are made either way.
CryptResult rc4Setup(const QByteArray &fp, bool verifyOnly, const QString &password, Rc4Keys *keys);

} // namespace jp::office::bincrypt
