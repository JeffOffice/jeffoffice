// What the test programs share: each test class registers itself here (JP_TEST_CLASS, after the class),
// and the program's main() (in testrunner.cpp) runs the classes one after another in one process.
//   program --classes          lists the classes
//   program --class Name       runs only that class (more than one --class are allowed)
// Every other argument goes to QtTest, which runs each class as if it were the whole program: the
// options (-v1, -silent, -o file,txt) apply to every class, and test function names run in the class
// that has them. tools/runtests.sh runs the classes of a program as parallel processes.
#pragma once

#include <QObject>

namespace jptest {

using Factory = QObject *(*)();

struct Registrar {
    Registrar(const char *name, Factory make);
};

} // namespace jptest

#define JP_TEST_CLASS(Class) \
    static jptest::Registrar jpTestRegistrar##Class(#Class, []() -> QObject * { return new Class; });
