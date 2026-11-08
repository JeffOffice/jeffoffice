#pragma once
// Passwords for PDFs. Qt's PDF writer cannot encrypt, so a PDF it wrote (in memory) is read by qpdf and written again with AES-256 (R6): the open
// password is required to read the file, and the optional owner password lifts the limits on printing, copying, and changing. The limits are a request
// that most PDF readers honor and none must: only the open password really protects the file.

#include <QByteArray>
#include <QString>

namespace jp {

struct PdfProtection {
    QString openPassword;            // empty: the PDF is not protected
    QString ownerPassword;           // used with `limited`
    bool limited = false;           // the three switches below apply
    bool allowPrint = true, allowCopy = true, allowChange = true;

    bool isSet() const { return !openPassword.isEmpty(); }
    // Overwrites both passwords before they are let go.
    void wipe();
};

// The most bytes of PDF that are encrypted (as the most of a picture file read: this reads the program's own output).
constexpr qint64 kMostPdfEncryptBytes = 256 * 1024 * 1024;

// `pdf` (as Qt wrote it) with the protection applied, all in memory; empty with the reason in `error` when it can't be done (no open password, a
// restriction without an owner password, an owner password the same as the open one, a file past the cap, or a file that is not a PDF).
QByteArray encryptPdf(const QByteArray &pdf, const PdfProtection &protection, QString *error = nullptr);

// The PDF without its encryption, opened with `password` (to check what a protected file holds); empty when it can't be opened.
QByteArray decryptPdf(const QByteArray &pdf, const QString &password, QString *error = nullptr);

// What qpdf reads of a PDF's encryption with `password` (for tests, and for checking a file made): false when the file can't be opened with it.
struct PdfEncryptionInfo {
    bool encrypted = false;
    int version = 0, revision = 0, permissions = 0;   // /V, /R, and /P of the /Encrypt dictionary
    bool ownerMatched = false;
    bool allowPrint = true, allowCopy = true, allowChange = true;
};
bool readPdfEncryption(const QByteArray &pdf, const QString &password, PdfEncryptionInfo *info, QString *error = nullptr);

} // namespace jp
