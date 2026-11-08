#pragma once
// The one door to qpdf (C++20): standard types only, so that the rest of the program (C++17, Qt) never includes its headers.
// libs/base/io/pdfcrypt.h is the program's own interface to it.

#include <cstddef>
#include <functional>
#include <string>

namespace jp::qpdfwrap {

struct EncryptOptions {
    std::string openPassword;    // required to read the file
    std::string ownerPassword;   // lifts the limits below; whoever has it can change them
    bool allowPrint = true, allowExtract = true, allowModify = true;
};

// The PDF in `data`, written again with AES-256 (V 5, R 6): the result goes to `sink` in pieces as qpdf makes it, so that it is never held whole here as well as by the
// caller. False with a reason in `error` when qpdf cannot read it.
using Sink = std::function<void(const char *piece, size_t size)>;
bool encrypt(const char *data, size_t size, const EncryptOptions &options, const Sink &sink, std::string *error);

// The same PDF without encryption (for checking what a protected file holds).
bool decrypt(const char *data, size_t size, const std::string &password, std::string *out, std::string *error);

// What a PDF says about its encryption, as qpdf reads it with `password`.
struct Info {
    bool encrypted = false;
    int V = 0, R = 0, P = 0;   // the /Encrypt dictionary's version and revision, and the permission bits
    bool ownerMatched = false;
    bool allowPrint = true, allowExtract = true, allowModify = true;   // as the password opened it gives them
};
// False when the file can't be opened with `password` (or at all).
bool inspect(const char *data, size_t size, const std::string &password, Info *info, std::string *error);

// Overwrites the text in place (the compiler is not to leave the writes out).
void wipe(std::string &text);

} // namespace jp::qpdfwrap
