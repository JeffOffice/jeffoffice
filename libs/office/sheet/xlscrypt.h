#pragma once
// A workbook of the old binary form that is protected with a password: the record FILEPASS (right after the first BOF) says how, and every record after it is enciphered except the
// ones that begin and describe the stream. Three forms are read (and the last is written): the obfuscation that XORs the bytes with a short table made from the password (the one of the oldest files and of BIFF5),
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

// The other way: protects a workbook stream (BIFF8, the way a save writes it) with `password`, in the form made by the cryptographic provider (RC4 with a 128-bit key from SHA-1; the
// obfuscation and the 40-bit form are old and weak, so they are only read). FILEPASS is put in right after the first BOF, the places of the sheets in the list of sheets move by its
// size, and every record after it is enciphered but the ones the format leaves in the clear (the ones that begin a stream, FILEPASS, those of a shared workbook, and where each sheet begins).
// False for an empty password, a stream that is not whole or is not BIFF8, or one that is protected already. The salt and the check value are random unless given (16 bytes each, for the tests).
bool encryptBiff(QByteArray *stream, const QString &password, const QByteArray &salt = QByteArray(), const QByteArray &verifier = QByteArray());

// The obfuscation's own numbers, for the tests: the key and the verifier made from a password (as ANSI bytes, 15 at most).
quint16 xorKeyForTest(const QByteArray &password);
quint16 xorVerifierForTest(const QByteArray &password);

} // namespace jp::office::sheet
