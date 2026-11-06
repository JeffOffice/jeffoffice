#pragma once

namespace jp {

// With JP_STARTUP_TRACE set, prints the milliseconds since JP_STARTUP_T0 (a steady-clock reading in nanoseconds, given by whatever launched the
// program) next to a name, to stderr. Nothing happens without it.
void startupMark(const char *what);

} // namespace jp
