#ifndef __WAIT_STRATEGY_H
#define __WAIT_STRATEGY_H

#include "utility.h"
#include "ring_buffer.h"

namespace VariableDisrupt
{

enum class WaitStrategyOption
{
    Blocking = 0,
    Busyspin
};

class RingBuffer;

class WaitStrategy
{
public:
    virtual ~WaitStrategy() = default;

    virtual int64_t WaitFor(RingBuffer *lpRingBuffer, int64_t uSequence, 
                             uint32_t uTimeoutUs = uNoTimeout) = 0;

    /**
     * @brief 唤醒等待的所有comsumer
     *
     */
    virtual void NotifyAll() = 0;

    static WaitStrategy *NewInstance(WaitStrategyOption eOptin);

    static void DeleteInstance(WaitStrategy *lpStrategy);
};

class BlockingStrategy : public WaitStrategy
{
public:
    BlockingStrategy() = default;
    ~BlockingStrategy() override = default;

    int64_t WaitFor(RingBuffer *lpRingBuffer, int64_t uSequence,
                     uint32_t uTimeoutUs = uNoTimeout) override
    {
        int64_t uAvailableSequence = lpRingBuffer->GetCurSor();
        while (unlikely(uAvailableSequence < uSequence))
        {
            {
                std::unique_lock<std::mutex> lock(m_mutex);

                if (unlikely(uTimeoutUs == uNoTimeout))
                {
                    m_cond.wait(lock);
                }
                else
                {
                    m_cond.wait_for(lock, std::chrono::microseconds(uTimeoutUs));
                }
            }

            uAvailableSequence = lpRingBuffer->GetCurSor();
        }

        return uAvailableSequence;
    }

    void NotifyAll()
    {
        m_cond.notify_all();
    }

private:
    std::mutex m_mutex;
    std::condition_variable m_cond;
};

class BusySpinStrategy : public WaitStrategy
{
public:
    BusySpinStrategy() = default;
    ~BusySpinStrategy() override = default;

    int64_t WaitFor(RingBuffer *lpRingBuffer, int64_t uSequence,
                        uint32_t uTimeoutUs = uNoTimeout) override
    {
        int64_t uAvailableSequence = lpRingBuffer->GetCurSor();
        if (unlikely(uAvailableSequence < uSequence))
        {
            auto uTimeoutNs = uTimeoutUs * 1000;
            auto begin = Utility::GetTimeNs();
            while (uAvailableSequence < uSequence)
            {
                auto now = Utility::GetTimeNs();
                if (now - begin >= uTimeoutNs)
                {
                    break;
                }
                uAvailableSequence = lpRingBuffer->GetCurSor();
            }
        }

        return uAvailableSequence;
    }

    void NotifyAll()
    {
    }
};

WaitStrategy *WaitStrategy::NewInstance(WaitStrategyOption eOption)
{
    WaitStrategy *lpStrategy = nullptr;
    switch (eOption)
    {
        case WaitStrategyOption::Blocking:
            lpStrategy = NEW BlockingStrategy;
            break;

        case WaitStrategyOption::Busyspin:
            lpStrategy = NEW BusySpinStrategy;
            break;

        default:
            lpStrategy = nullptr;
            break;
    }

    return lpStrategy;
}

void WaitStrategy::DeleteInstance(WaitStrategy *lpStrategy)
{
    if (lpStrategy != nullptr)
    {
        delete lpStrategy;
    }
}

} // end namespace VariableDisrupt

#endif //__WAIT_STRATEGY_H
