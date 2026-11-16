#pragma once
// An old binary document (.doc) that is protected with a password. The header of the file says so (fEncrypted), and how: the obfuscation that XORs the bytes with a table made from the
// password (the verifier is in the header), RC4 with a 40-bit key, or RC4 with a key made by the cryptographic provider (CryptoAPI); the last two put their description at the start of the
// table stream. The bytes after the first 68 of the document stream, the table stream after its description, and the data stream are enciphered, in blocks of 512 bytes for RC4.
// decryptDoc gives the file as it was before, a compound file whose streams are plain, for the reader that reads one. Read only: a .doc is never written protected.

#include "office/officecrypt.h"

#include <QByteArray>
#include <QString>

#include <functional>

namespace jp::office::doc {

// True if the bytes are a compound file whose document stream says it is protected (and is of the 1997 form or newer: older ones are refused by the reader, with its own message).
bool docEncrypted(const QByteArray &file);

// Checks the password and, unless `verifyOnly`, gives the file with its streams deciphered and the mark of protection taken off. A file or its streams larger than `maxBytes` is TooLarge.
CryptResult decryptDoc(const QByteArray &file, const QString &password, qint64 maxBytes, bool verifyOnly, QByteArray *plain, const std::function<bool()> &cancelled = nullptr);

// The most bytes of the document stream that are never enciphered (the start of the header).
constexpr int kDocPlainPrefix = 68;

} // namespace jp::office::doc
