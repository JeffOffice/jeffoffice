#pragma once
// Inflating the compressed metafiles of binary drawings, with a limit on
// what comes out: a small damaged or hostile stream can't grow without end.

#include <QByteArray>

namespace jp::office {

// The deflate data of a zlib stream (or of a stream without the zlib
// wrapper). `ok` is false, and the result empty, when the stream is damaged
// or would give more than `limit` bytes.
QByteArray odrawInflate(const QByteArray &in, qint64 limit, bool *ok = nullptr);

} // namespace jp::office
