#pragma once
// The ciphers the protected files of other programs use, small and with no library behind them: AES (128, 192, and 256 bits) in ECB, CBC, and Galois/counter modes, RC4, and Blowfish in CFB mode,
// with BLAKE2b and Argon2id (the key stretching of the newest protected OpenDocument files).
// Hashes (SHA-1, SHA-256, SHA-512, MD5), HMAC, and PBKDF2 are Qt's own (QCryptographicHash and friends). Nothing here checks a password or keeps a key longer than its object lives.

#include <QByteArray>

#include <array>
#include <functional>

namespace jp::crypto {

// AES. A key of a size other than 16, 24, or 32 bytes, or data that is not a whole number of 16-byte blocks (and an iv that is not 16 bytes), gives an empty result.
QByteArray aesEcb(const QByteArray &key, const QByteArray &data, bool encrypt);
QByteArray aesCbc(const QByteArray &key, const QByteArray &iv, const QByteArray &data, bool encrypt);

// AES in Galois/counter mode (NIST SP 800-38D) with a tag of 16 bytes. aesGcmSeal gives the enciphered data followed by the tag. aesGcmOpen takes data in that form and gives the plain bytes;
// if the tag does not fit (or the key or the iv, which may be any length but not empty, is wrong) `*ok` is false and the result is empty.
QByteArray aesGcmSeal(const QByteArray &key, const QByteArray &iv, const QByteArray &aad, const QByteArray &plain);
QByteArray aesGcmOpen(const QByteArray &key, const QByteArray &iv, const QByteArray &aad, const QByteArray &sealed, bool *ok);

// BLAKE2b (RFC 7693) with an output of 1 to 64 bytes and, if wanted, a key of up to 64 bytes.
QByteArray blake2b(const QByteArray &data, int outLen = 64, const QByteArray &key = QByteArray());

// Argon2id (RFC 9106, version 0x13): `memoryKiB` blocks of 1 KiB, `iterations` passes, `lanes` of parallelism (run one after the other here), and a tag of `outLen` bytes (4 or more).
// `secret` and `ad` are the optional extra inputs. The result is empty if the numbers are out of range (memory under 8 per lane, or any of them zero), if memory cannot be had, or if
// `cancelled` (asked now and then) says stop. The callers bound the numbers a file may ask for.
QByteArray argon2id(const QByteArray &password, const QByteArray &salt, quint32 iterations, quint32 memoryKiB, quint32 lanes, int outLen, const QByteArray &secret = QByteArray(),
                    const QByteArray &ad = QByteArray(), const std::function<bool()> &cancelled = nullptr);

// RC4: the keystream starts at the key, and apply() xors the next bytes of it into the data (the same call encrypts and decrypts). The key is 1 to 256 bytes.
class Rc4 {
public:
    explicit Rc4(const QByteArray &key);
    ~Rc4();
    Rc4(const Rc4 &) = default;
    Rc4 &operator=(const Rc4 &) = default;
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

// Whether two byte strings are the same, taking the same time whatever the bytes are (for what a password is checked against).
bool equal(const QByteArray &a, const QByteArray &b);
// Zeroes memory with writes the compiler may not drop (a key schedule, a state of a cipher).
void secureZero(void *where, size_t size);

// Overwrites the bytes of a secret before it is let go (a password, a key): the block is zeroed with writes the compiler may not drop.
void wipe(QByteArray &bytes);
void wipe(QString &text);

} // namespace jp::crypto
