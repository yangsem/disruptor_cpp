#ifndef __PRODUCER_H
#define __PRODUCER_H

#include "utility.h"
#include "ring_buffer.h"
#include "claim_strategy.h"
#include "wait_strategy.h"
#include "consumer.h"

namespace VariableDisrupt
{

class RingBuffer;
class Consumer;

class Producer
{
public:
    Producer(RingBuffer *lpRingBuffer,
            ClaimStrategy *lpClaimStrategy,
            WaitStrategy *lpWaitStrategy,
            std::map<Consumer *, std::string> &mapConsumers)
        : m_lpRingBuffer(lpRingBuffer), m_lpClaimStrategy(lpClaimStrategy)
        , m_lpWaitStrategy(lpWaitStrategy) , m_mapConsumers(mapConsumers)
    {
    }

    ~Producer() = default;

    Entry *NewEntry(uint32_t uSize)
    {
        auto uEntrySize = sizeof(Entry) + ALIGN8(uSize);
        auto uSequence = m_lpClaimStrategy->GetSequence(uEntrySize);
        EnsureSequenceConsumed(uSequence + uEntrySize);
        auto lpEntry = m_lpRingBuffer->GetEntry(uSequence);
        lpEntry->SetSequence(uSequence);
        return lpEntry;
    }

    void Commit(Entry *lpEntry)
    {
        auto uExpectSequence = lpEntry->GetSequence();
        while (lpEntry->GetSequence() != m_lpRingBuffer->GetCurSor())
        {
            std::this_thread::yield();
        }

        m_lpRingBuffer->SetCursor(lpEntry->GetSequence() + lpEntry->GetDataLength());
        m_lpWaitStrategy->NotifyAll();
    }

private:
    void EnsureSequenceConsumed(int64_t uSequence)
    {
        auto uWrapPoint = uSequence - m_lpRingBuffer->GetCapacity();
        while (uWrapPoint >= m_uNextConsume)
        {
            std::this_thread::yield();
            m_uNextConsume = GetMinNextConsume();
        }
    }

    int64_t GetMinNextConsume()
    {
        int64_t uMinimum = INT64_MAX;
        for (auto &consumer : m_mapConsumers)
        {
            auto uSequence = consumer.first->GetSequence();
            uMinimum = uMinimum < uSequence ? uMinimum : uSequence;
        }

        return uMinimum;
    }

private:
    RingBuffer *m_lpRingBuffer{nullptr};
    ClaimStrategy *m_lpClaimStrategy{nullptr};
    WaitStrategy *m_lpWaitStrategy{nullptr};
    int64_t m_uNextConsume{0};
    std::map<Consumer *, std::string> &m_mapConsumers;
};

} // end namespace VariableDisrupt

#endif //__PRODUCER_H