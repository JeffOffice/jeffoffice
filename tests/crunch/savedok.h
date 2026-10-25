// Waits for the save a CrunchWindow::saved spy was made for, and says whether it worked. A test that only waits for the signal goes on
// with the old file when the save failed (on Windows, when the test still holds the file open); this one says the save failed.
#pragma once

#include <QSignalSpy>

inline bool savedOk(QSignalSpy &spy, int ms = 20000)
{
    return spy.wait(ms) && spy.last().value(1).toBool();
}
