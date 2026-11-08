#include "io/pdfcrypt.h"

#include "io/crypto.h"

#include <QCoreApplication>
#include <QRandomGenerator>

#include <qpdfwrap.h>

namespace jp {

void PdfProtection::wipe()
{
    crypto::wipe(openPassword);
    crypto::wipe(ownerPassword);
}

namespace {

// A string the wrapper takes, to be wiped by whoever made it.
std::string utf8Of(const QString &text)
{
    QByteArray bytes = text.toUtf8();
    std::string out(bytes.constData(), size_t(bytes.size()));
    crypto::wipe(bytes);
    return out;
}

} // namespace

QByteArray encryptPdf(const QByteArray &pdf, const PdfProtection &protection, QString *error)
{
    auto fail = [&](const QString &why) {
        if (error) *error = why;
        return QByteArray();
    };
    if (protection.openPassword.isEmpty()) return fail(QCoreApplication::translate("PdfCrypt", "There is no password."));
    if (pdf.size() > kMostPdfEncryptBytes) return fail(QCoreApplication::translate("PdfCrypt", "The PDF is too large to protect with a password."));
    if (protection.limited && protection.ownerPassword.isEmpty()) return fail(QCoreApplication::translate("PdfCrypt", "The limits need a password of their own."));
    if (protection.limited && protection.ownerPassword == protection.openPassword)
        return fail(QCoreApplication::translate("PdfCrypt", "The password for the limits must not be the one that opens the file."));
    qpdfwrap::EncryptOptions options;
    options.openPassword = utf8Of(protection.openPassword);
    if (protection.limited) {
        options.ownerPassword = utf8Of(protection.ownerPassword);
        options.allowPrint = protection.allowPrint;
        options.allowExtract = protection.allowCopy;
        options.allowModify = protection.allowChange;
    } else {
        // Nobody holds the owner password: the file has no limits, and no second way in.
        QByteArray random(32, Qt::Uninitialized);
        QRandomGenerator::system()->fillRange(reinterpret_cast<quint32 *>(random.data()), random.size() / int(sizeof(quint32)));
        options.ownerPassword = random.toHex().toStdString();
        crypto::wipe(random);
    }
    std::string out, why;
    const bool ok = qpdfwrap::encrypt(pdf.constData(), size_t(pdf.size()), options, &out, &why);
    qpdfwrap::wipe(options.openPassword);
    qpdfwrap::wipe(options.ownerPassword);
    if (!ok) return fail(QString::fromStdString(why));
    return QByteArray(out.data(), qsizetype(out.size()));
}

QByteArray decryptPdf(const QByteArray &pdf, const QString &password, QString *error)
{
    std::string word = utf8Of(password), out, why;
    const bool ok = qpdfwrap::decrypt(pdf.constData(), size_t(pdf.size()), word, &out, &why);
    qpdfwrap::wipe(word);
    if (!ok) {
        if (error) *error = QString::fromStdString(why);
        return {};
    }
    return QByteArray(out.data(), qsizetype(out.size()));
}

bool readPdfEncryption(const QByteArray &pdf, const QString &password, PdfEncryptionInfo *info, QString *error)
{
    std::string word = utf8Of(password), why;
    qpdfwrap::Info found;
    const bool ok = qpdfwrap::inspect(pdf.constData(), size_t(pdf.size()), word, &found, &why);
    qpdfwrap::wipe(word);
    if (!ok) {
        if (error) *error = QString::fromStdString(why);
        return false;
    }
    info->encrypted = found.encrypted;
    info->version = found.V;
    info->revision = found.R;
    info->permissions = found.P;
    info->ownerMatched = found.ownerMatched;
    info->allowPrint = found.allowPrint;
    info->allowCopy = found.allowExtract;
    info->allowChange = found.allowModify;
    return true;
}

} // namespace jp
