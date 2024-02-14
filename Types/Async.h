#pragma once
#include <future>
#include <mutex>
#include <thread>
using SLockGuard = std::lock_guard<std::mutex>;
using SUniqueLock = std::unique_lock<std::mutex>;
using SMutex = std::mutex;