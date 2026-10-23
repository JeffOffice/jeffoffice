#include "calc/functions.h"

#include "calc/machine.h"

#include <QHash>

#include <vector>

namespace jp::calc {

// Each file of functions adds its own.
void registerLogicFunctions(std::vector<FunctionInfo> &r);
void registerMathFunctions(std::vector<FunctionInfo> &r);
void registerStatFunctions(std::vector<FunctionInfo> &r);
void registerTextFunctions(std::vector<FunctionInfo> &r);
void registerDateFunctions(std::vector<FunctionInfo> &r);
void registerLookupFunctions(std::vector<FunctionInfo> &r);
void registerFinanceFunctions(std::vector<FunctionInfo> &r);

namespace {

struct Registry {
    std::vector<FunctionInfo> functions;
    QHash<QString, int> byName;

    Registry()
    {
        registerLogicFunctions(functions);
        registerMathFunctions(functions);
        registerStatFunctions(functions);
        registerTextFunctions(functions);
        registerDateFunctions(functions);
        registerLookupFunctions(functions);
        registerFinanceFunctions(functions);
        for (size_t i = 0; i < functions.size(); ++i) byName.insert(QString::fromLatin1(functions[i].name), int(i));
    }
};

const Registry &registry()
{
    static const Registry r;
    return r;
}

QString canonical(const QString &name)
{
    QString n = name;
    for (bool stripped = true; stripped;) {
        stripped = false;
        for (const char *prefix : {"_xlfn.", "_xlws."})
            if (n.startsWith(QLatin1String(prefix), Qt::CaseInsensitive)) {
                n.remove(0, int(strlen(prefix)));
                stripped = true;
            }
    }
    return n.toUpper();
}

} // namespace

int functionIndex(const QString &name)
{
    const Registry &r = registry();
    const auto it = r.byName.constFind(canonical(name));
    return it == r.byName.constEnd() ? -1 : it.value();
}

const FunctionInfo *findFunction(const QString &name)
{
    const int i = functionIndex(name);
    return i < 0 ? nullptr : &registry().functions[size_t(i)];
}

const FunctionInfo &functionAt(int index) { return registry().functions[size_t(index)]; }
int functionCount() { return int(registry().functions.size()); }

} // namespace jp::calc
