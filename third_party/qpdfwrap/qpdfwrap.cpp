#include "qpdfwrap.h"

#include <qpdf/Buffer.hh>
#include <qpdf/QPDF.hh>
#include <qpdf/QPDFWriter.hh>

#include <exception>
#include <memory>

namespace jp::qpdfwrap {

bool encrypt(const char *data, size_t size, const EncryptOptions &options, std::string *out, std::string *error)
{
    try {
        QPDF pdf;
        pdf.processMemoryFile("pdf", data, size);
        QPDFWriter writer(pdf);
        writer.setOutputMemory();
        // The streams are kept as Qt wrote them: only their encryption is new.
        writer.setStreamDataMode(qpdf_s_preserve);
        writer.setObjectStreamMode(qpdf_o_preserve);
        // Screen readers may always read the text out; the other limits are the caller's.
        writer.setR6EncryptionParameters(options.openPassword.c_str(), options.ownerPassword.c_str(), true, options.allowExtract, options.allowModify,
                                         options.allowModify, options.allowModify, options.allowModify, options.allowPrint ? qpdf_r3p_full : qpdf_r3p_none, true);
        writer.write();
        const std::shared_ptr<Buffer> made = writer.getBufferSharedPointer();
        out->assign(reinterpret_cast<const char *>(made->getBuffer()), made->getSize());
        return true;
    } catch (const std::exception &e) {
        if (error) *error = e.what();
        return false;
    }
}

bool decrypt(const char *data, size_t size, const std::string &password, std::string *out, std::string *error)
{
    try {
        QPDF pdf;
        pdf.processMemoryFile("pdf", data, size, password.c_str());
        QPDFWriter writer(pdf);
        writer.setOutputMemory();
        writer.setStreamDataMode(qpdf_s_preserve);
        writer.setObjectStreamMode(qpdf_o_preserve);
        writer.write();
        const std::shared_ptr<Buffer> made = writer.getBufferSharedPointer();
        out->assign(reinterpret_cast<const char *>(made->getBuffer()), made->getSize());
        return true;
    } catch (const std::exception &e) {
        if (error) *error = e.what();
        return false;
    }
}

bool inspect(const char *data, size_t size, const std::string &password, Info *info, std::string *error)
{
    try {
        QPDF pdf;
        pdf.processMemoryFile("pdf", data, size, password.c_str());
        Info found;
        QPDF::encryption_method_e streams, strings, file;
        found.encrypted = pdf.isEncrypted(found.R, found.P, found.V, streams, strings, file);
        found.ownerMatched = found.encrypted && pdf.ownerPasswordMatched();
        found.allowPrint = pdf.allowPrintHighRes();
        found.allowExtract = pdf.allowExtractAll();
        found.allowModify = pdf.allowModifyAnnotation() && pdf.allowModifyAssembly() && pdf.allowModifyForm() && pdf.allowModifyOther();
        // The owner's password lifts the limits.
        if (found.ownerMatched) found.allowPrint = found.allowExtract = found.allowModify = true;
        *info = found;
        return true;
    } catch (const std::exception &e) {
        if (error) *error = e.what();
        return false;
    }
}

void wipe(std::string &text)
{
    volatile char *p = text.empty() ? nullptr : &text[0];
    for (size_t i = 0; i < text.size(); ++i) p[i] = 0;
    text.clear();
}

} // namespace jp::qpdfwrap
