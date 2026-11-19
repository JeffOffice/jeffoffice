#pragma once
// Threads for our own work. Never std::thread, std::jthread, or std::async: on Windows (MinGW) a thread the C++ library starts frees the storage of its
// thread_local objects before their destructors run, and a worker that had used one (a renderer's cache, the color filter of libs/base/core/color.cpp)
// crashed as it ended, the more often the more processors a machine has. Found November 19, 2026, on a 16-processor machine: the pictures of JeffPub's
// templates, made on one thread for each processor, crashed jptests in ~std::function, called from the library's tls_callback while the thread shut down.
// A QThread ends in the right order (its storage is freed after the destructors). tools/check_threads.py keeps the C++ library's threads out of libs/ and apps/.

#include <QObject>
#include <QThread>

#include <memory>
#include <utility>

namespace jp {

// A thread that runs `f` and is waited for: join(), or the destructor, waits for it to end.
class WorkThread {
public:
    template <typename F>
    explicit WorkThread(F &&f) : m_thread(QThread::create(std::forward<F>(f))) { m_thread->start(); }
    WorkThread(WorkThread &&) noexcept = default;
    WorkThread &operator=(WorkThread &&) = delete;   // (the thread assigned over could still be running)
    ~WorkThread() { join(); }

    // False when the system had no thread to give: `f` did not run and will not.
    bool started() const { return m_thread && (m_thread->isRunning() || m_thread->isFinished()); }
    void join()
    {
        if (!m_thread) return;
        m_thread->wait();
        m_thread.reset();
    }

private:
    std::unique_ptr<QThread> m_thread;
};

// Runs `f` on a thread of its own that nothing waits for. (The thread's object goes when the thread has ended and the thread that started it next handles
// its events; when that never happens, at the end of the program, the small object is left.)
template <typename F>
void runDetached(F &&f)
{
    QThread *thread = QThread::create(std::forward<F>(f));
    QObject::connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    thread->start();
}

} // namespace jp
