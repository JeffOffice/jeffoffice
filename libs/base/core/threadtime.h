#pragma once
// The processor time the calling thread has used, in microseconds: what a stretch of work cost, apart from the time it was waiting or another program had the machine. A window's
// work on its own thread is measured with this (and a test asserts on it), since the clock on the wall includes everything else the computer was doing.

#include <QtGlobal>

namespace jp {

// Microseconds of processor time (user and system) this thread has used since it began; 0 if the system can't say.
qint64 threadCpuMicros();

} // namespace jp
