#pragma once
// The ciphers the protected files of other programs use, small and with no library behind them: AES (128, 192, and 256 bits) in ECB and CBC modes, RC4, and Blowfish in CFB mode.
// Hashes (SHA-1, SHA-256, SHA-512, MD5), HMAC, and PBKDF2 are Qt's own (QCryptographicHash and friends). Nothing here checks a password or keeps a key longer than its object lives.

#include <QByteArray>

#include <array>

namespace jp::crypto {

// AES. A key of a size other than 16, 24, or 32 bytes, or data that is not a whole number of 16-byte blocks (and an iv that is not 16 bytes), gives an empty result.
QByteArray aesEcb(const QByteArray &key, const QByteArray &data, bool encrypt);
QByteArray aesCbc(const QByteArray &key, const QByteArray &iv, const QByteArray &data, bool encrypt);

// RC4: the keystream starts at the key, and apply() xors the next bytes of it into the data (the same call encrypts and decrypts). The key is 1 to 256 bytes.
class Rc4 {
public:
    explicit Rc4(const QByteArray &key);
    void apply(char *data, qsizetype size);
    QByteArray apply(const QByteArray &data);
    // Passes over `count` bytes of the keystream without using them.
    void skip(qsizetype count);
private:
    std::array<quint8, 256> m_s{};
    quint8 m_i = 0, m_j = 0;
};

// Blowfish with a key of 4 to 56 bytes, in the cipher-feedback mode with a whole block fed back (64 bits). The last block of data may be short. An iv that is not 8 bytes, or a key
// that is out of range, gives an empty result.
QByteArray blowfishCfb(const QByteArray &key, const QByteArray &iv, const QByteArray &data, bool encrypt);
// One block, for the tests of the cipher against the published values.
QByteArray blowfishEcbBlock(const QByteArray &key, const QByteArray &block, bool encrypt);

// Overwrites the bytes of a secret before it is let go (a password, a key): the block is zeroed with writes the compiler may not drop.
void wipe(QByteArray &bytes);
void wipe(QString &text);

} // namespace jp::crypto
