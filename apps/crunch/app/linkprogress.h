#pragma once
// Reading the files that links point to takes as long as the files are big (and a file on another computer as long as the network): it is done on a thread of its own, behind a window that has
// Cancel, so that the window of the workbook does not stop.

#include "sheet/externalbooks.h"

#include <QString>

#include <functional>

class QWidget;

namespace jp {

// Runs `work` on another thread and returns when it is done (the window's events go on). The window with Cancel shows after a moment; Cancel stops the reads (ExternalBooks::cancelReads). False if it was cancelled.
bool runLinkReads(QWidget *parent, jp::office::sheet::ExternalBooks *links, const QString &text, const std::function<void()> &work);

} // namespace jp
