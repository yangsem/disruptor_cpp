#ifndef __WAIT_STRATEGY_H
#define __WAIT_STRATEGY_H

#include "utility.h"

namespace FixedDisruptor
{

enum class WaitStrategyOption
{
    Blocking = 0,
    Busyspin
};

template <typename Entry> class RingBuffer;

template <typename Entry>
class WaitStrategy
{
public:
    /**
     * @brief 虚析构函数
     *
     */
    virtual ~WaitStrategy() = default;

    /**
     * @brief 等待可以消费的Entry
     *
     * @param consumers 需要等待的消费者集合
     * @param lpRingBuffer 等待哪一个RingBuffer
     * @param uSequence 等待的sequence
     * @return uint64_t 可以被消费的sequence
     */
    virtual uint64_t WaitFor(RingBuffer<Entry> *lpRingBuffer, uint64_t uSequence, 
                             uint32_t uTimeoutUs = uNoTimeout) = 0;

    /**
     * @brief 唤醒等待的所有comsumer
     *
     */
    virtual void NotifyAll() = 0;

    static WaitStrategy<Entry> *NewInstance(WaitStrategyOption eOptin);

    static void DeleteInstance(WaitStrategy *lpStrategy);
};

template <typename Entry>
class BlockingStrategy : public WaitStrategy<Entry>
{
public:
    BlockingStrategy() = default;
    ~BlockingStrategy() override = default;

    uint64_t WaitFor(RingBuffer<Entry> *lpRingBuffer, uint64_t uSequence,
                     uint32_t uTimeoutUs = uNoTimeout) override
    {
        uint64_t uAvailableSequence = lpRingBuffer->GetCurSor();
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

template <typename Entry>
class BusySpinStrategy : public WaitStrategy<Entry>
{
public:
    BusySpinStrategy() = default;
    ~BusySpinStrategy() override = default;

    uint64_t WaitFor(RingBuffer<Entry> *lpRingBuffer, uint64_t uSequence,
                        uint32_t uTimeoutUs = uNoTimeout) override
    {
        uint64_t uAvailableSequence = lpRingBuffer->GetCurSor();
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

template <typename Entry>
WaitStrategy<Entry> *WaitStrategy<Entry>::NewInstance(WaitStrategyOption eOption)
{
    WaitStrategy<Entry> *lpStrategy = nullptr;
    switch (eOption)
    {
        case WaitStrategyOption::Blocking:
            lpStrategy = NEW BlockingStrategy<Entry>;
            break;

        case WaitStrategyOption::Busyspin:
            lpStrategy = NEW BusySpinStrategy<Entry>;
            break;

        default:
            lpStrategy = nullptr;
            break;
    }

    return lpStrategy;
}

template <typename Entry>
void WaitStrategy<Entry>::DeleteInstance(WaitStrategy<Entry> *lpStrategy)
{
    if (lpStrategy != nullptr)
    {
        delete lpStrategy;
    }
}

} // end namespace FixedDisruptor

#endif //__WAIT_STRATEGY_H
