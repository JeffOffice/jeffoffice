#pragma once
// An OpenDocument spreadsheet that is protected with a password: its manifest gives each enciphered part its own way (the cipher: AES-256 in CBC mode, or the older Blowfish in CFB mode;
// the key: the password hashed with SHA-1 or SHA-256, then stretched by PBKDF2 with a salt; and a checksum of the part's first 1,024 bytes once deciphered). The parts are stored deflated and
// then enciphered. decryptOds gives the package as it was before, a zip of the plain parts, for the reader that reads one.

#include "office/officecrypt.h"

#include <QByteArray>
#include <QString>

namespace jp::office::sheet {

// True if the manifest of the zip says any part is enciphered.
bool odsEncrypted(const QByteArray &zip);

// Checks the password against every enciphered part (or, with `verifyOnly`, the first one) and, unless `verifyOnly`, gives the zip with those parts deciphered and inflated.
// More than `maxBytes` of parts is TooLarge.
CryptResult decryptOds(const QByteArray &zip, const QString &password, qint64 maxBytes, bool verifyOnly, QByteArray *plain);

// The most rounds of key stretching that all the parts of a package may ask for together.
constexpr qint64 kMaxOdsKeyRounds = 2000000;

} // namespace jp::office::sheet
