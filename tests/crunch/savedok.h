// Waits for the save a CrunchWindow::saved spy was made for, and says whether it worked. A test that only waits for the signal goes on
// with the old file when the save failed (on Windows, when the test still holds the file open); this one says the save failed.
#pragma once

#include <QSignalSpy>

// (60 s, not 20: on a loaded Linux CI machine one class took 70 s where it takes 0.6 s here, and a save ran past 20 s; Nov 15 2026)
inline bool savedOk(QSignalSpy &spy, int ms = 60000)
{
    return spy.wait(ms) && spy.last().value(1).toBool();
}
