#pragma once
// An OpenDocument file as a template (.ott, .ots, .otp): the same package with the type of its content named as a template's, in the "mimetype" part and in the manifest.

#include <QByteArray>

namespace jp::office::odt {

// The package `odf` (an .odt, .ods, or .odp as written) with its media type changed to the template's ("...text" to "...text-template"); every other part is as it was. The
// package unchanged when it is not a ZIP, has no "mimetype" part, or is a template already.
QByteArray odfAsTemplate(const QByteArray &odf);

} // namespace jp::office::odt
