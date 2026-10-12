#include "app/crash.h"

#include <QDate>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QRegularExpression>
#include <QStandardPaths>

#include <atomic>
#include <cstdint>
#include <ctime>

#ifdef Q_OS_WIN
#include <windows.h>
#include <csignal>
#else
#include <dlfcn.h>
#include <execinfo.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#endif

// The address sanitizer catches crashes itself, and says more.
#if defined(__SANITIZE_ADDRESS__)
#define JP_ASAN 1
#elif defined(__has_feature)
#if __has_feature(address_sanitizer)
#define JP_ASAN 1
#endif
#endif

namespace jp::crash {

namespace {

constexpr int kMaxFrames = 64;            // the collector takes no more
constexpr quint64 kMaxOffset = 1ull << 48;
const char kHeader[] = "jeffoffice-crash 1";

// Everything the handler uses is set up beforehand, in static memory: once
// the program has crashed its heap can't be trusted, so the handler builds
// the report in this buffer and writes it with the system's own calls.
struct Report {
    char text[8192];
    int size = 0;
    void put(const char *s)
    {
        while (*s && size < int(sizeof text)) text[size++] = *s++;
    }
    void putNumber(unsigned long long v, unsigned base, int width = 1)
    {
        char digits[24];
        int n = 0;
        do {
            digits[n++] = "0123456789abcdef"[v % base];
            v /= base;
        } while (v || n < width);
        while (n && size < int(sizeof text)) text[size++] = digits[--n];
    }
    void begin(const char *kind, long long code)
    {
        put(kHeader);
        put("\nversion " JP_VERSION "\nwhen ");
        // The day in UTC, worked out from the clock (gmtime() may lock).
        long long z = static_cast<long long>(std::time(nullptr)) / 86400 + 719468;
        const long long era = (z >= 0 ? z : z - 146096) / 146097;
        const unsigned doe = unsigned(z - era * 146097);
        const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
        const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100), mp = (5 * doy + 2) / 153;
        const unsigned day = doy - (153 * mp + 2) / 5 + 1, month = mp < 10 ? mp + 3 : mp - 9;
        putNumber(static_cast<unsigned long long>(yoe + era * 400 + (month <= 2)), 10, 4);
        put("-");
        putNumber(month, 10, 2);
        put("-");
        putNumber(day, 10, 2);
        put("\nkind ");
        put(kind);
        put("\ncode ");
        if (code < 0) put("-");
        putNumber(static_cast<unsigned long long>(code < 0 ? -code : code), 10);
        put("\n");
    }
    void frame(const char *module, unsigned long long offset)
    {
        put("frame ");
        put(*module ? module : "unknown");
        put(" ");
        putNumber(offset, 16);
        put("\n");
    }
};

Report g_report;
std::atomic<bool> g_installed{false}, g_crashed{false};

// The file name, after the last slash.
template <typename Char>
const Char *baseName(const Char *path)
{
    const Char *name = path;
    for (const Char *p = path; *p; ++p)
        if (*p == Char('/') || *p == Char('\\')) name = p + 1;
    return name;
}

#ifdef Q_OS_WIN

wchar_t g_path[1024];
LPTOP_LEVEL_EXCEPTION_FILTER g_previous = nullptr;
CONTEXT g_context;

// The module holding `pc`, and where it was loaded.
bool moduleOf(DWORD64 pc, char *name, int size, DWORD64 *base)
{
    HMODULE module = nullptr;
    wchar_t path[MAX_PATH];
    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                            reinterpret_cast<LPCWSTR>(pc), &module) ||
        !GetModuleFileNameW(module, path, MAX_PATH))
        return false;
    const wchar_t *file = baseName(path);
    int i = 0;
    for (; file[i] && i < size - 1; ++i) name[i] = file[i] < 128 ? char(file[i]) : '_';
    name[i] = 0;
    *base = reinterpret_cast<DWORD64>(module);
    return true;
}

void addFrame(DWORD64 pc)
{
    char name[MAX_PATH];
    DWORD64 base = 0;
    if (moduleOf(pc, name, MAX_PATH, &base)) g_report.frame(name, pc - base);
    else g_report.frame("unknown", 0);
}

// Each step of the call stack, unwound with the tables the compiler keeps
// in the program for exceptions, from where `context` stopped.
void walk(CONTEXT &context)
{
#if defined(_M_X64) || defined(__x86_64__)
    const NT_TIB *tib = reinterpret_cast<const NT_TIB *>(NtCurrentTeb());
    const DWORD64 low = reinterpret_cast<DWORD64>(tib->StackLimit), high = reinterpret_cast<DWORD64>(tib->StackBase);
    for (int i = 0; i < kMaxFrames && context.Rip; ++i) {
        addFrame(context.Rip);
        const DWORD64 sp = context.Rsp;
        DWORD64 imageBase = 0;
        if (PRUNTIME_FUNCTION fn = RtlLookupFunctionEntry(context.Rip, &imageBase, nullptr)) {
            PVOID data = nullptr;
            DWORD64 frame = 0;
            RtlVirtualUnwind(UNW_FLAG_NHANDLER, imageBase, context.Rip, fn, &context, &data, &frame, nullptr);
        } else {
            // A function that calls nothing: its return address is on top.
            if (sp < low || sp + 8 > high) break;
            context.Rip = *reinterpret_cast<const DWORD64 *>(sp);
            context.Rsp += 8;
        }
        if (context.Rsp <= sp || context.Rsp < low || context.Rsp >= high) break;
    }
#else
    addFrame(static_cast<DWORD64>(context.Eip));
#endif
}

void save()
{
    const HANDLE file = CreateFileW(g_path, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) return;
    DWORD written = 0;
    WriteFile(file, g_report.text, DWORD(g_report.size), &written, nullptr);
    CloseHandle(file);
}

LONG WINAPI onException(EXCEPTION_POINTERS *e)
{
    if (!g_crashed.exchange(true)) {
        g_report.begin("exception", static_cast<long long>(static_cast<unsigned long>(e->ExceptionRecord->ExceptionCode)));
        g_context = *e->ContextRecord;
        walk(g_context);
        save();
    }
    return g_previous ? g_previous(e) : EXCEPTION_CONTINUE_SEARCH;
}

// abort(), which an unhandled C++ exception and a fatal Qt error end in.
// The C runtime ends the program when this returns.
void onAbort(int sig)
{
    if (!g_crashed.exchange(true)) {
        g_report.begin("signal", sig);
        RtlCaptureContext(&g_context);
        walk(g_context);
        save();
    }
    std::signal(sig, SIG_DFL);
}

#else

char g_path[4096];
alignas(16) char g_altStack[64 * 1024];   // a stack that has overflowed can't run the handler

bool moduleOf(const void *pc, char *name, int size, uintptr_t *base)
{
    Dl_info info;
    if (!dladdr(pc, &info) || !info.dli_fname || !info.dli_fbase) return false;
    const char *file = baseName(info.dli_fname);
    int i = 0;
    for (; file[i] && i < size - 1; ++i) name[i] = file[i];
    name[i] = 0;
    *base = reinterpret_cast<uintptr_t>(info.dli_fbase);
    return true;
}

void addFrame(const void *pc)
{
    char name[256];
    uintptr_t base = 0;
    if (moduleOf(pc, name, sizeof name, &base)) g_report.frame(name, reinterpret_cast<uintptr_t>(pc) - base);
    else g_report.frame("unknown", 0);
}

#ifdef __APPLE__

// Older SDKs read the registers directly.
#ifndef __darwin_arm_thread_state64_get_pc
#define __darwin_arm_thread_state64_get_pc(ts) ((ts).__pc)
#define __darwin_arm_thread_state64_get_lr(ts) ((ts).__lr)
#define __darwin_arm_thread_state64_get_fp(ts) ((ts).__fp)
#endif

// Each step of the call stack from where the signal stopped, by the chain of
// frame pointers Apple's systems always keep. (backtrace() stops at the edge
// of the thread's stack, and this handler runs on a stack of its own.)
int stackAt(void *context, const void **frames, int max)
{
    const auto *state = &static_cast<ucontext_t *>(context)->uc_mcontext->__ss;
#if defined(__aarch64__) || defined(__arm64__)
    // The system's own code signs the return addresses it saves (pointer
    // authentication) in the bits above the address; they're cleared here.
    constexpr uintptr_t kAddress = (uintptr_t(1) << 47) - 1;
    const uintptr_t pc = uintptr_t(__darwin_arm_thread_state64_get_pc(*state)) & kAddress;
    const uintptr_t lr = uintptr_t(__darwin_arm_thread_state64_get_lr(*state)) & kAddress;
    uintptr_t fp = uintptr_t(__darwin_arm_thread_state64_get_fp(*state));
#else
    constexpr uintptr_t kAddress = ~uintptr_t(0);
    const uintptr_t pc = state->__rip, lr = 0;
    uintptr_t fp = state->__rbp;
#endif
    const pthread_t self = pthread_self();
    const uintptr_t top = reinterpret_cast<uintptr_t>(pthread_get_stackaddr_np(self)), bottom = top - pthread_get_stacksize_np(self);
    auto onStack = [&](uintptr_t p) { return p >= bottom && p + 2 * sizeof(uintptr_t) <= top && p % sizeof(uintptr_t) == 0; };
    int n = 0;
    frames[n++] = reinterpret_cast<const void *>(pc);
    // A function that calls nothing may not have saved its return address:
    // it's still in the link register.
    if (lr && max > 1 && (!onStack(fp) || (reinterpret_cast<const uintptr_t *>(fp)[1] & kAddress) != lr))
        frames[n++] = reinterpret_cast<const void *>(lr);
    while (n < max && onStack(fp)) {
        const uintptr_t *record = reinterpret_cast<const uintptr_t *>(fp);   // {caller's frame, return address}
        const uintptr_t ret = record[1] & kAddress;
        if (!ret) break;
        frames[n++] = reinterpret_cast<const void *>(ret);
        if (record[0] <= fp) break;   // frames only go up the stack
        fp = record[0];
    }
    return n;
}

#else

// The instruction the signal stopped.
const void *stoppedAt(void *context)
{
#if defined(__linux__) && defined(__x86_64__)
    return reinterpret_cast<const void *>(static_cast<ucontext_t *>(context)->uc_mcontext.gregs[REG_RIP]);
#elif defined(__linux__) && defined(__aarch64__)
    return reinterpret_cast<const void *>(static_cast<ucontext_t *>(context)->uc_mcontext.pc);
#else
    Q_UNUSED(context);
    return nullptr;
#endif
}

// Each step of the call stack from where the signal stopped.
int stackAt(void *context, const void **frames, int max)
{
    void *all[kMaxFrames + 8];
    const int n = backtrace(all, kMaxFrames + 8);
    // Start at the stopped instruction, past this handler and the system's
    // code that called it; without it, after this handler.
    const void *pc = stoppedAt(context);
    int first = -1, out = 0;
    for (int i = 0; i < n && first < 0; ++i)
        if (all[i] == pc) first = i;
    if (first < 0) {
        first = 1;
        if (pc) frames[out++] = pc;
    }
    for (int i = first; i < n && out < max; ++i) frames[out++] = all[i];
    return out;
}

#endif

void onSignal(int sig, siginfo_t *, void *context)
{
    if (!g_crashed.exchange(true)) {
        g_report.begin("signal", sig);
        const void *frames[kMaxFrames];
        const int n = stackAt(context, frames, kMaxFrames);
        for (int i = 0; i < n && g_report.size < int(sizeof g_report.text) - 300; ++i) addFrame(frames[i]);
        const int file = open(g_path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
        if (file >= 0) {
            [[maybe_unused]] const ssize_t written = write(file, g_report.text, size_t(g_report.size));
            close(file);
        }
    }
    signal(sig, SIG_DFL);
    raise(sig);
}

#endif

// Only the characters the collector takes, cut to its length.
QString clean(QString s, const QString &bad, int max)
{
    s.replace(QRegularExpression(bad), QStringLiteral("-"));
    return s.left(max);
}

} // namespace

QString reportPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/crash-report.txt");
}

void install(const QString &path)
{
#ifndef JP_ASAN
    if (path.isEmpty() || g_installed.exchange(true)) return;
    QDir().mkpath(QFileInfo(path).path());
#ifdef Q_OS_WIN
    const QString native = QDir::toNativeSeparators(path);
    if (native.size() >= int(sizeof g_path / sizeof g_path[0])) return;
    g_path[native.toWCharArray(g_path)] = 0;
    g_previous = SetUnhandledExceptionFilter(onException);
    std::signal(SIGABRT, onAbort);
#else
    const QByteArray native = QFile::encodeName(path);
    if (native.size() >= int(sizeof g_path)) return;
    qstrcpy(g_path, native.constData());
    void *warm[2];
    backtrace(warm, 2);   // loads what it needs now, not in the handler
    stack_t alt{};
    alt.ss_sp = g_altStack;
    alt.ss_size = sizeof g_altStack;
    sigaltstack(&alt, nullptr);
    struct sigaction action {};
    action.sa_sigaction = onSignal;
    action.sa_flags = SA_SIGINFO | SA_ONSTACK | SA_RESETHAND;
    sigemptyset(&action.sa_mask);
    for (int sig : {SIGSEGV, SIGBUS, SIGFPE, SIGILL, SIGABRT}) sigaction(sig, &action, nullptr);
#endif
#else
    Q_UNUSED(path);
#endif
}

QJsonObject readReport(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return {};
    const QList<QByteArray> lines = file.read(64 * 1024).split('\n');
    if (lines.value(0) != kHeader) return {};
    QJsonObject report;
    QJsonArray frames;
    for (const QByteArray &line : lines.mid(1)) {
        const QList<QByteArray> parts = line.split(' ');
        const QByteArray key = parts.value(0);
        const QString value = QString::fromLatin1(parts.value(1));
        if (key == "frame" && parts.size() == 3) {
            bool ok = false;
            const quint64 offset = parts[2].toULongLong(&ok, 16);
            if (frames.size() >= kMaxFrames) continue;
            QString module = clean(value, QStringLiteral("[^A-Za-z0-9._+-]"), 64);
            if (!ok || offset >= kMaxOffset || module.isEmpty()) frames.append(QJsonObject{{QStringLiteral("module"), QStringLiteral("unknown")}, {QStringLiteral("offset"), 0}});
            else frames.append(QJsonObject{{QStringLiteral("module"), module}, {QStringLiteral("offset"), qint64(offset)}});
        } else if (key == "version" && parts.size() == 2) {
            report.insert(QStringLiteral("version"), clean(value, QStringLiteral("[^0-9A-Za-z.+-]"), 20));
        } else if (key == "when" && parts.size() == 2 && QDate::fromString(value, Qt::ISODate).isValid()) {
            report.insert(QStringLiteral("when"), QDate::fromString(value, Qt::ISODate).toString(Qt::ISODate));
        } else if (key == "kind" && parts.size() == 2 && (value == QLatin1String("signal") || value == QLatin1String("exception"))) {
            report.insert(QStringLiteral("kind"), value);
        } else if (key == "code" && parts.size() == 2) {
            bool ok = false;
            const qint64 code = parts[1].toLongLong(&ok);
            if (ok && code >= -(1ll << 31) && code < (1ll << 32)) report.insert(QStringLiteral("code"), code);
        }
    }
    for (const char *needed : {"version", "when", "kind", "code"})
        if (!report.contains(QLatin1String(needed))) return {};
    report.insert(QStringLiteral("frames"), frames);
    return report;
}

bool locate(const void *address, QString *module, quint64 *offset)
{
#ifdef Q_OS_WIN
    char name[MAX_PATH];
    DWORD64 base = 0;
    if (!moduleOf(reinterpret_cast<DWORD64>(address), name, MAX_PATH, &base)) return false;
    *offset = reinterpret_cast<DWORD64>(address) - base;
#else
    char name[256];
    uintptr_t base = 0;
    if (!moduleOf(address, name, sizeof name, &base)) return false;
    *offset = reinterpret_cast<uintptr_t>(address) - base;
#endif
    *module = QString::fromLatin1(name);
    return true;
}

} // namespace jp::crash
