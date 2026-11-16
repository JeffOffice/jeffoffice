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

// True if the bytes are a compound file whose Current User stream says the presentation is protected. Only that small stream is read (a document, a workbook, or a presentation that is not
// protected is not parsed whole); `isPpt`, when given, says whether the file has the stream of a presentation at all.
bool pptEncrypted(const QByteArray &file, bool *isPpt = nullptr);

// Checks the password and, unless `verifyOnly`, gives the file with its streams deciphered and the mark of protection taken off. A file or its streams larger than `maxBytes` is TooLarge;
// a protection of another kind (a cipher other than RC4, the 40-bit form of a workbook, a key held by a certificate) is Unsupported, and that is said before any password is used: with `verifyOnly` and an empty password,
// Unsupported and Damaged come back as they would with the right one, and WrongPassword means the form is one that is read.
CryptResult decryptPpt(const QByteArray &file, const QString &password, qint64 maxBytes, bool verifyOnly, QByteArray *plain, const std::function<bool()> &cancelled = nullptr);

// What is bounded: the edits followed, the words of all the persist directories together (an eighth of the stream's bytes, headers and entries both), the description, the ids that name
// one offset, and every offset and length against the stream's size; atoms of edits and directories, and persist objects, may not overlap.
constexpr int kMaxPptEdits = 1000;
constexpr qint64 kMaxDescription = 8192;

} // namespace jp::office::ppt
