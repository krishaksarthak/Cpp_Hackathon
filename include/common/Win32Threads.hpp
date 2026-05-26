#pragma once

#ifdef _WIN32
#ifndef _GLIBCXX_HAS_GTHREADS

// MinGW Win32 thread model lacks std::mutex and std::thread.
// This is a minimal shim to provide them using Windows API.

#include <windows.h>
#include <process.h>
#include <functional>
#include <memory>
#include <chrono>
#include <exception>

namespace std {

class mutex {
    CRITICAL_SECTION cs;
public:
    mutex() { InitializeCriticalSection(&cs); }
    ~mutex() { DeleteCriticalSection(&cs); }
    void lock() { EnterCriticalSection(&cs); }
    void unlock() { LeaveCriticalSection(&cs); }
    mutex(const mutex&) = delete;
    mutex& operator=(const mutex&) = delete;
};

// lock_guard is provided by <mutex> even when std::mutex is not

class thread {
    HANDLE hThread;
    unsigned threadId;
    
    struct ThreadData {
        std::function<void()> func;
    };
    
    static unsigned __stdcall threadFunc(void* arg) {
        std::unique_ptr<ThreadData> data(static_cast<ThreadData*>(arg));
        data->func();
        return 0;
    }
    
public:
    thread() : hThread(nullptr), threadId(0) {}
    
    template<typename Callable, typename... Args>
    explicit thread(Callable&& f, Args&&... args) {
        // std::bind is used to bind the function and arguments
        auto data = new ThreadData{std::bind(std::forward<Callable>(f), std::forward<Args>(args)...)};
        hThread = (HANDLE)_beginthreadex(nullptr, 0, &threadFunc, data, 0, &threadId);
    }
    
    ~thread() {
        if (joinable()) {
            std::terminate();
        }
    }
    
    bool joinable() const { return hThread != nullptr; }
    
    void join() {
        if (hThread) {
            WaitForSingleObject(hThread, INFINITE);
            CloseHandle(hThread);
            hThread = nullptr;
        }
    }
    
    thread(thread&& other) noexcept : hThread(other.hThread), threadId(other.threadId) {
        other.hThread = nullptr;
    }
    thread& operator=(thread&& other) noexcept {
        if (joinable()) std::terminate();
        hThread = other.hThread;
        threadId = other.threadId;
        other.hThread = nullptr;
        return *this;
    }
    thread(const thread&) = delete;
    thread& operator=(const thread&) = delete;
};

} // namespace std

#endif // _GLIBCXX_HAS_GTHREADS
#endif // _WIN32
