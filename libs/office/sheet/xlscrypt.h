#pragma once
// A workbook of the old binary form that is protected with a password: the record FILEPASS (right after the first BOF) says how, and every record after it is enciphered except the
// ones that begin and describe the stream. Three forms are read: the obfuscation that XORs the bytes with a short table made from the password (the one of the oldest files and of BIFF5),
// RC4 with a 40-bit key, and RC4 with a key made by the cryptographic provider (CryptoAPI). The records' own headers (number and length) are never enciphered, so the stream can be walked
// before it is deciphered; the key is taken from the place of the bytes in the stream.

#include "office/officecrypt.h"

#include <QByteArray>
#include <QString>

namespace jp::office::sheet {

// True if the stream has a FILEPASS record before the end of its first group of records (the workbook globals).
bool biffEncrypted(const QByteArray &stream);

// Checks `password` and, unless `verifyOnly`, deciphers every record after FILEPASS in place; the FILEPASS record stays where it is but is renamed to a record number no reader
// knows, so that the offsets in the stream don't change. WrongPassword if the password does not fit; Damaged for a record that is not whole; Unsupported for a form not read.
CryptResult decryptBiff(QByteArray *stream, const QString &password, bool verifyOnly);

// The obfuscation's own numbers, for the tests: the key and the verifier made from a password (as ANSI bytes, 15 at most).
quint16 xorKeyForTest(const QByteArray &password);
quint16 xorVerifierForTest(const QByteArray &password);

} // namespace jp::office::sheet
