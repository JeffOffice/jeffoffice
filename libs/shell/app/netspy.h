#pragma once
// For tests: a look at each request the updater or the statistics are about to send. While a spy is set, it is called with
// the request, and nothing goes out (the request is answered as if it had been sent). With none set, nothing changes.

#include <QNetworkRequest>
#include <functional>

namespace jp {

void setNetworkSpy(const std::function<void(const QNetworkRequest &)> &spy);
// True when a spy took the request, so the caller sends nothing.
bool networkSpied(const QNetworkRequest &request);

} // namespace jp
