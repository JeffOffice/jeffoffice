#include "io/pdfcrypt.h"

#include "io/crypto.h"

#include <QCoreApplication>
#include <QRandomGenerator>
#include <QScopeGuard>

#include <qpdfwrap.h>

namespace jp {

void PdfProtection::wipe()
{
    crypto::wipe(openPassword);
    crypto::wipe(ownerPassword);
}

PdfProtection PdfProtection::copy() const
{
    PdfProtection out = *this;
    out.openPassword = QString(openPassword.constData(), openPassword.size());
    out.ownerPassword = QString(ownerPassword.constData(), ownerPassword.size());
    return out;
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

bool prohibited(char32_t c)
{
    switch (QChar::category(c)) {
    case QChar::Other_Control:
    case QChar::Other_Surrogate:
    case QChar::Other_PrivateUse:
    case QChar::Other_NotAssigned: return true;
    default: break;
    }
    // The bidirectional controls (and the tag characters).
    return c == 0x200E || c == 0x200F || (c >= 0x202A && c <= 0x202E) || (c >= 0x2066 && c <= 0x2069) || (c >= 0xE0000 && c <= 0xE007F);
}

} // namespace

PdfPasswordProblem normalizePdfPassword(const QString &typed, QString *normalized)
{
    normalized->clear();
    QString made = typed.normalized(QString::NormalizationForm_KC);
    for (qsizetype i = 0; i < made.size(); ++i) {
        const QChar c = made.at(i);
        if (c.isHighSurrogate() && i + 1 < made.size() && made.at(i + 1).isLowSurrogate()) {
            if (prohibited(QChar::surrogateToUcs4(c, made.at(i + 1)))) {
                crypto::wipe(made);
                return PdfPasswordProblem::Forbidden;
            }
            ++i;
            continue;
        }
        if (prohibited(c.unicode())) {
            crypto::wipe(made);
            return PdfPasswordProblem::Forbidden;
        }
        if (c.unicode() > 0x7F && c.category() == QChar::Separator_Space) made[i] = QLatin1Char(' ');
    }
    if (made.toUtf8().size() > kMostPdfPasswordBytes) {
        crypto::wipe(made);
        return PdfPasswordProblem::TooLong;
    }
    *normalized = QString(made.constData(), made.size());
    crypto::wipe(made);
    return PdfPasswordProblem::None;
}

QString pdfPasswordProblemText(PdfPasswordProblem problem)
{
    switch (problem) {
    case PdfPasswordProblem::Forbidden: return QCoreApplication::translate("PdfCrypt", "The password has a character that can't be used in a PDF password, such as a control character.");
    case PdfPasswordProblem::TooLong: return QCoreApplication::translate("PdfCrypt", "The password is too long. A PDF password can be at most 127 bytes: about 127 letters without accents, and fewer with other characters.");
    case PdfPasswordProblem::None: break;
    }
    return {};
}

QString pdfOpeningPassword(const QString &typed)
{
    QString made = typed.normalized(QString::NormalizationForm_KC);
    QByteArray bytes = made.toUtf8();
    if (bytes.size() > kMostPdfPasswordBytes) {
        int cut = kMostPdfPasswordBytes;
        while (cut > 0 && (static_cast<unsigned char>(bytes[cut]) & 0xC0) == 0x80) --cut;   // (not in the middle of a character)
        bytes.truncate(cut);
        made = QString::fromUtf8(bytes);
    }
    crypto::wipe(bytes);
    return made;
}

// A note on wiping: the passwords here are wiped wherever this code holds them, but qpdf keeps copies of its own in std::strings (the encryption data of its writer)
// that nothing outside it can reach, so they stay in memory until the allocator reuses them.
//
// Memory: the PDF is read where it is (qpdf is given its address, not a copy), and the result is appended to one QByteArray reserved at the size of the PDF, so that the peak
// is about twice the PDF (the input and the output) plus qpdf's small tables of objects. The streams pass through as they are (they are not inflated).
QByteArray encryptPdf(const QByteArray &pdf, const PdfProtection &protection, QString *error)
{
    auto fail = [&](const QString &why) {
        if (error) *error = why;
        return QByteArray();
    };
    if (protection.openPassword.isEmpty()) return fail(QCoreApplication::translate("PdfCrypt", "There is no password."));
    if (pdf.size() > kMostPdfEncryptBytes) return fail(QCoreApplication::translate("PdfCrypt", "The PDF is too large to protect with a password."));
    if (protection.limited && protection.ownerPassword.isEmpty()) return fail(QCoreApplication::translate("PdfCrypt", "The limits need a password of their own."));
    QString open, owner;
    const auto wipeNormalized = qScopeGuard([&] {
        crypto::wipe(open);
        crypto::wipe(owner);
    });
    PdfPasswordProblem problem = normalizePdfPassword(protection.openPassword, &open);
    if (problem == PdfPasswordProblem::None && protection.limited) problem = normalizePdfPassword(protection.ownerPassword, &owner);
    if (problem != PdfPasswordProblem::None) return fail(pdfPasswordProblemText(problem));
    // (compared as they will be written: two spellings of one password are one password)
    if (protection.limited && owner == open) return fail(QCoreApplication::translate("PdfCrypt", "The password for the limits must not be the one that opens the file."));
    qpdfwrap::EncryptOptions options;
    options.openPassword = utf8Of(open);
    if (protection.limited) {
        options.ownerPassword = utf8Of(owner);
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
    QByteArray made;
    made.reserve(pdf.size() + pdf.size() / 8 + 4096);
    std::string why;
    const bool ok = qpdfwrap::encrypt(pdf.constData(), size_t(pdf.size()), options, [&made](const char *piece, size_t n) { made.append(piece, qsizetype(n)); }, &why);
    qpdfwrap::wipe(options.openPassword);
    qpdfwrap::wipe(options.ownerPassword);
    if (!ok) return fail(QString::fromStdString(why));
    return made;
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
