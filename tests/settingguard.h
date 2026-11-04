#pragma once
// For the test programs that link the apps' shell: a setting with a value for one test, and its own value back after.
#include "isolatesettings.h"
#include "app/settings.h"

namespace jptest {

// A setting the test needs to have a value, and the value it had back after however the test ends.
struct SettingGuard {
    QString key;
    QVariant before;
    SettingGuard(const QString &k, const QVariant &value, const QVariant &normally) : key(k), before(jp::Settings::get().value(k, normally)) { jp::Settings::get().setValue(key, value); }
    ~SettingGuard() { jp::Settings::get().setValue(key, before); }
    SettingGuard(const SettingGuard &) = delete;
    SettingGuard &operator=(const SettingGuard &) = delete;
};

} // namespace jptest
