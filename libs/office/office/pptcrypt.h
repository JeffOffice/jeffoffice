#pragma once
// An old binary presentation (.ppt) that is protected with a password: RC4 with a key made by the cryptographic provider (CryptoAPI), the only form the format has. The Current User stream
// says so (its header token), and the edit record that is the newest names, by persist id, the record that holds the description of the protection. Each persist object of the presentation
// stream is enciphered on its own, from the start of its record header, with the key of the block that is its persist id; the edit records, the persist directories, and the description
// itself are not. The Pictures stream is enciphered field by field (each restarts at block 0). decryptPpt gives the file as it was before, a compound file whose streams are plain.
// Read only: a .ppt is never written protected.

#include "office/officecrypt.h"

#include <QByteArray>
#include <QString>

#include <functional>

namespace jp::office::ppt {

// True if the bytes are a compound file whose Current User stream says the presentation is protected (or whose newest edit names a description of protection).
bool pptEncrypted(const QByteArray &file);

// Checks the password and, unless `verifyOnly`, gives the file with its streams deciphered and the mark of protection taken off. A file or its streams larger than `maxBytes` is TooLarge;
// a protection of another kind (a cipher other than RC4, a key held by a certificate) is Unsupported.
CryptResult decryptPpt(const QByteArray &file, const QString &password, qint64 maxBytes, bool verifyOnly, QByteArray *plain, const std::function<bool()> &cancelled = nullptr);

// What is bounded: the edits followed (1,000), the persist entries read (one for each 4 bytes of the stream), the description (8 KiB), and every offset and length against the stream's size.
constexpr int kMaxPptEdits = 1000;

} // namespace jp::office::ppt
