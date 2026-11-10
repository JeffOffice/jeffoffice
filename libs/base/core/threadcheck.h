#pragma once
// A check for code that keeps caches nothing locks (the text layout's and the renderer's: "main thread only, like all layout"). Those places call guiOnlyCache(); a test switches the
// watch on, and reads how many times it was called from a thread other than the application's own. Off, a call is one read of a flag.

#include <atomic>

namespace jp::threadcheck {

extern std::atomic<bool> g_watching;
void noteOffThread();   // (counts the call when this is not the application's thread)

inline void guiOnlyCache()
{
    if (g_watching.load(std::memory_order_relaxed)) noteOffThread();
}

// For tests: watching on or off, the number of calls from other threads, and back to none.
void watch(bool on);
int offThreadCalls();
void reset();

} // namespace jp::threadcheck
