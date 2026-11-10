#include "app/netspy.h"

namespace jp {

namespace {
std::function<void(const QNetworkRequest &)> &spy()
{
    static std::function<void(const QNetworkRequest &)> f;
    return f;
}
} // namespace

void setNetworkSpy(const std::function<void(const QNetworkRequest &)> &s) { spy() = s; }

bool networkSpied(const QNetworkRequest &request)
{
    if (!spy()) return false;
    spy()(request);
    return true;
}

} // namespace jp
