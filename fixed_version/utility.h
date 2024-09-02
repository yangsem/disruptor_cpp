#ifndef __UTILITY_H
#define __UTILITY_H

#include <new>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <map>
#include <memory>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

namespace FixedDisruptor
{

#if defined(__WIN32) || defined(__WIN64)
#define OS_WIN
#else
#define OS_LINUX
#endif

// 分支优化
#ifdef OS_LINUX
#define likely(x) __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)
#else
#define likely(x) (x)
#define unlikely(x) (x)
#endif

#define NEW new(std::nothrow)

constexpr uint64_t uInvalidSequence = uint64_t(-1);
constexpr uint32_t uNoTimeout = uint32_t(-1);

class Utility
{
public:
    static uint64_t GetNextPowerOfTwo(uint64_t num)
    {
        // 如果n已经是2的幂，则直接返回
        if (num && !(num & (num - 1)))
        {
            return num;
        }

        uint64_t uResult = 1;
        while (uResult < num)
        {
            uResult <<= 1;
        }

        return uResult;
    }

    static uint64_t GetTimeNs()
    {
        timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return ts.tv_sec * (1000 * 1000 * 1000ULL) + ts.tv_nsec;
    }
};

} // end namespace FixedDisruptor

#endif //__UTILITY_H
