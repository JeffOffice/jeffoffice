// The helpers declared in pubhelpers.h.
#include "pubhelpers.h"

void initPubTestCase()
{
    static QTemporaryDir settingsDir;
    jp::setAppInfo(jp::pubAppInfo());
    QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, settingsDir.path());
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settingsDir.path());
    initCore();
    // Measurements in inches, whatever the region of the computer running the tests.
    jp::Settings::get().setUnit(jp::Unit::Inch);
}
