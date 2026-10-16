// main() of every test program: see testrunner.h.
#include "testrunner.h"

#include <QFile>
#include <QMetaMethod>
#include <QtTest>

#include <algorithm>
#include <cstdio>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

#if defined(QT_WIDGETS_LIB)
#include <QApplication>
using TestApplication = QApplication;
#elif defined(QT_GUI_LIB)
#include <QGuiApplication>
using TestApplication = QGuiApplication;
#else
#include <QCoreApplication>
using TestApplication = QCoreApplication;
#endif

namespace {

struct Entry {
    std::string name;
    jptest::Factory make;
};

std::vector<Entry> &registry()
{
    static std::vector<Entry> classes;
    return classes;
}

bool startsWith(const std::string &s, const char *prefix)
{
    return s.compare(0, std::char_traits<char>::length(prefix), prefix) == 0;
}

// QtTest options that are followed by a value.
bool takesValue(const std::string &option)
{
    static const char *const names[] = {"-o", "-eventdelay", "-keydelay", "-mousedelay", "-maxwarnings", "-repeat",
                                        "-perfcounter", "-minimumvalue", "-minimumtotal", "-iterations", "-median"};
    return std::any_of(std::begin(names), std::end(names), [&](const char *n) { return option == n; });
}

// Whether the class has a test function of this name ("name" or "name:dataTag").
bool hasTestFunction(const QObject &object, const std::string &requested)
{
    const QByteArray name = QByteArray::fromStdString(requested).split(':').value(0);
    const QMetaObject *mo = object.metaObject();
    for (int i = mo->methodOffset(); i < mo->methodCount(); ++i) {
        const QMetaMethod m = mo->method(i);
        if (m.methodType() == QMetaMethod::Slot && m.parameterCount() == 0 && m.name() == name) return true;
    }
    return false;
}

// The file of a "-o file,format" (or old-style "-o file") option, or nothing when it names the screen.
std::string fileOf(const std::string &value)
{
    const size_t comma = value.rfind(',');
    const std::string file = comma == std::string::npos ? value : value.substr(0, comma);
    return file == "-" ? std::string() : file;
}

// Every run of QtTest opens its results file anew, so each class writes a file of its own that is
// added to the one the command line named as soon as the class is done.
void append(const std::string &from, const std::string &to)
{
    QFile in{QString::fromStdString(from)};
    if (!in.open(QIODevice::ReadOnly)) return;
    QFile out{QString::fromStdString(to)};
    if (out.open(QIODevice::WriteOnly | QIODevice::Append)) out.write(in.readAll());
    in.close();
    in.remove();
}

} // namespace

jptest::Registrar::Registrar(const char *name, Factory make)
{
    registry().push_back({name, make});
}

int main(int argc, char *argv[])
{
    std::vector<Entry> &classes = registry();
    std::sort(classes.begin(), classes.end(), [](const Entry &a, const Entry &b) { return a.name < b.name; });

    // Our own options first; the rest are QtTest's.
    std::vector<std::string> chosen, rest{argv[0]};
    bool list = false;
    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if (a == "--classes") list = true;
        else if (a == "--class" && i + 1 < argc) chosen.push_back(argv[++i]);
        else if (startsWith(a, "--class=")) chosen.push_back(a.substr(8));
        else rest.push_back(a);
    }
    if (list) {
        for (const Entry &c : classes) std::printf("%s\n", c.name.c_str());
        return 0;
    }
    for (const std::string &name : chosen) {
        if (std::none_of(classes.begin(), classes.end(), [&](const Entry &c) { return c.name == name; })) {
            std::fprintf(stderr, "Unknown test class: %s (--classes lists them)\n", name.c_str());
            return 1;
        }
    }

    std::vector<char *> restArgv;
    for (std::string &s : rest) restArgv.push_back(s.data());
    restArgv.push_back(nullptr);
    int restArgc = int(rest.size());
    TestApplication app(restArgc, restArgv.data());
    app.setAttribute(Qt::AA_Use96Dpi, true);

    // What QtTest was asked for: its options, the functions to run, and the results files.
    std::vector<std::string> options, functions;
    std::vector<std::string> results;   // the files of the -o options
    for (int i = 1; i < restArgc; ++i) {
        const std::string a = restArgv[i];
        if (a.empty() || a[0] != '-') {
            functions.push_back(a);
            continue;
        }
        options.push_back(a);
        if (takesValue(a) && i + 1 < restArgc) {
            std::string value = restArgv[++i];
            if (a == "-o") {
                const std::string file = fileOf(value);
                if (!file.empty()) {
                    results.push_back(file);
                    QFile truncate{QString::fromStdString(file)};
                    truncate.open(QIODevice::WriteOnly | QIODevice::Truncate);
                    value = file + ".part" + value.substr(file.size());
                }
            }
            options.push_back(value);
        }
    }
    const bool helpOnly = std::find(options.begin(), options.end(), "-help") != options.end();

    int failures = 0;
    std::vector<std::string> failed;
    std::vector<bool> found(functions.size(), functions.empty());
    for (const Entry &c : classes) {
        if (!chosen.empty() && std::find(chosen.begin(), chosen.end(), c.name) == chosen.end()) continue;
        std::unique_ptr<QObject> object(c.make());
        std::vector<std::string> args{rest[0]};
        args.insert(args.end(), options.begin(), options.end());
        bool any = functions.empty();
        for (size_t f = 0; f < functions.size(); ++f) {
            if (!hasTestFunction(*object, functions[f])) continue;
            args.push_back(functions[f]);
            found[f] = true;
            any = true;
        }
        if (!any) continue;
        std::vector<char *> cargv;
        for (std::string &s : args) cargv.push_back(s.data());
        cargv.push_back(nullptr);
        const int code = QTest::qExec(object.get(), int(args.size()), cargv.data());
        object.reset();
        for (const std::string &file : results) append(file + ".part", file);
        if (code != 0) {
            failures += code > 0 ? code : 1;
            failed.push_back(c.name);
        }
        if (helpOnly) break;
    }
    for (size_t f = 0; f < functions.size(); ++f) {
        if (found[f]) continue;
        std::fprintf(stderr, "Unknown test function: '%s'\n", functions[f].c_str());
        ++failures;
    }
    if (!failed.empty()) {
        std::fprintf(stderr, "Failed test classes:");
        for (const std::string &name : failed) std::fprintf(stderr, " %s", name.c_str());
        std::fprintf(stderr, "\n");
    }
    return std::min(failures, 127);
}
