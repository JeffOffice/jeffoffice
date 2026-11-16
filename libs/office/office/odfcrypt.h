#pragma once
// An OpenDocument package (.odt, .odp, .ods, and their templates) that is protected with a password: its manifest gives each enciphered part its own way (the cipher: AES-256 in CBC mode, or the older Blowfish in CFB mode;
// the key: the password hashed with SHA-1 or SHA-256, then stretched by PBKDF2 with a salt; and a checksum of the part's first 1,024 bytes once deciphered). The parts are stored deflated and
// then enciphered. decryptOdf gives the package as it was before, a zip of the plain parts, for the reader that reads one; encryptOdf is the other way, in the form of the 1.2 and 1.3
// standards (AES-256 in CBC mode, the key from the SHA-256 of the password stretched by PBKDF2 with HMAC-SHA-1, one salt for the file and an iv for each part). One implementation for all three programs.

#include "office/officecrypt.h"

#include <QByteArray>
#include <QString>

#include <functional>

namespace jp::office {

// True if the manifest of the zip says any part is enciphered.
bool odfEncrypted(const QByteArray &zip);

// Checks the password against every enciphered part (or, with `verifyOnly`, the first one) and, unless `verifyOnly`, gives the zip with those parts deciphered and inflated.
// More than `maxBytes` of parts is TooLarge.
CryptResult decryptOdf(const QByteArray &zip, const QString &password, qint64 maxBytes, bool verifyOnly, QByteArray *plain, const std::function<bool()> &cancelled = nullptr);

// The newest files stretch the key with Argon2id (memory in KiB, passes, and lanes) and use AES-256 in Galois/counter mode, the whole package being the one enciphered part. A file may ask
// for at most ten times what the original program writes by default (64 MiB, 3 passes, 4 lanes); more is TooCostly.
constexpr qint64 kMaxOdfArgonMemoryKiB = 655360;
constexpr qint64 kMaxOdfArgonIterations = 30;
constexpr qint64 kMaxOdfArgonLanes = 40;

// The most rounds of key stretching that one part may ask for (twenty times the 100,000 that the original program writes), and the most that all the parts of a package may make keys with
// together (a file from another program has a salt for each part, so each is stretched on its own: 400 parts at the usual number; the keys are made on up to eight threads, and the file
// this program writes has one salt, so one key). More is TooCostly.
constexpr qint64 kMaxOdfKeyRounds = 2000000;
constexpr qint64 kMaxOdfTotalKeyRounds = 40000000;
// The most parts a protected package may have.
constexpr int kMaxOdfParts = 20000;

// How a package is protected when it is written: the rounds of key stretching for each part (the tests use a small number).
struct OdfEncryptOptions {
    int rounds = 100000;
};

// A smaller most for encryptOdf than its caller's, for tests of a refused protection (none: -1).
void setOdfProtectLimitForTest(qint64 maxBytes);

// The package `zip` with every part but "mimetype" and the manifest deflated and enciphered with `password`, and the manifest saying how. Empty if `zip` is not a package that can be
// protected (no manifest, an entry that will not inflate, more than `maxBytes` in all, more than kMaxOdfParts parts, a package already protected) or if `cancelled` said stop.
QByteArray encryptOdf(const QByteArray &zip, const QString &password, qint64 maxBytes, const OdfEncryptOptions &options = OdfEncryptOptions(), const std::function<bool()> &cancelled = nullptr);

} // namespace jp::office
