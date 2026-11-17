#pragma once
// What the old binary forms of protection share (a workbook's FILEPASS record, a document's table stream): the obfuscation that XORs the bytes with a table made from the password, and the
// two kinds of RC4 (a 40-bit key from the password's MD5, and a key made by the cryptographic provider, CryptoAPI). The callers walk their own streams; this makes the keys.

#include "io/crypto.h"
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
    // The cipher for one block, started at its first byte (the key it was made from is wiped here).
    crypto::Rc4 cipher(quint32 block) const;
};

// `fp` is the form's header as a workbook's FILEPASS record holds it: two bytes (the kind of protection, not read here), the version (major, minor, two bytes each), and then the form's own
// fields. Ok if the password fits; the keys are made either way.
CryptResult rc4Setup(const QByteArray &fp, bool verifyOnly, const QString &password, Rc4Keys *keys);

// The other side, for a file to be written: a new header for a workbook's FILEPASS record in the form made by the cryptographic provider (RC4, SHA-1, a 128-bit key), from `password`
// (its first 255 letters), with a salt and a check value of its own (16 random bytes each; the tests may give them). `keys` is made to encipher the stream with. The body's first two bytes
// are the kind of protection (1: RC4), then the version (4.2) and the header the reader above takes, so that what this makes, rc4Setup opens. Empty if the password is empty.
QByteArray rc4CryptoApiHeader(const QString &password, Rc4Keys *keys, const QByteArray &salt = QByteArray(), const QByteArray &verifier = QByteArray());

} // namespace jp::office::bincrypt
