#pragma once
// The default locale the program takes numbers and messages in, set for the length of a test and put back after it: a test of what a number looks like says which language it is in, and
// does not depend on the machine's.

#include <QLocale>

struct PinnedLocale {
    QLocale before;
    explicit PinnedLocale(const QLocale &locale = QLocale(QLocale::English, QLocale::UnitedStates)) : before(QLocale())
    {
        QLocale::setDefault(locale);
    }
    ~PinnedLocale() { QLocale::setDefault(before); }
    PinnedLocale(const PinnedLocale &) = delete;
    PinnedLocale &operator=(const PinnedLocale &) = delete;
};
