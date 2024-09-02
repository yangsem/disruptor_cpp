#ifndef __PRODUCER_H
#define __PRODUCER_H

#include "utility.h"
#include "ring_buffer.h"
#include "claim_strategy.h"
#include "wait_strategy.h"
#include "sequence_batch.h"
#include "consumer.h"

namespace FixedDisruptor
{

template <typename Entry> class RingBuffer;
template <typename Entry> class Consumer;

template <typename Entry>
class Producer
{
public:
    Producer(RingBuffer<Entry> *lpRingBuffer, 
            ClaimStrategy *lpClaimStrategy, 
            WaitStrategy<Entry> *lpWaitStrategy,
            std::map<Consumer<Entry> *, std::string> &mapConsumers)
        : m_lpRingBuffer(lpRingBuffer), m_lpClaimStrategy(lpClaimStrategy)
        , m_lpWaitStrategy(lpWaitStrategy) , m_mapConsumers(mapConsumers)
    {
    }

    ~Producer() = default;

    Entry *NewEntry()
    {
        auto uSequence = m_lpClaimStrategy->GetNextSequence();
        EnsureSequenceConsumed(uSequence);
        auto lpEntry = m_lpRingBuffer->GetEntry(uSequence);
        lpEntry->SetSequence(uSequence);
        return lpEntry;
    }

    void Commit(Entry *lpEntry)
    {
        Commit(lpEntry->GetSequence(), 1);
    }

    void NewEntries(SequenceBatch &sequenceBatch)
    {
        auto uSequence = m_lpClaimStrategy->GetBatchSequence(sequenceBatch.GetSize());
        sequenceBatch.SetEnd(uSequence);
        EnsureSequenceConsumed(uSequence);
        auto uEnd = sequenceBatch.GetEnd();
        for (uint64_t i = sequenceBatch.GetStart(); i < uEnd; i++)
        {
            auto lpEntry = m_lpRingBuffer->GetEntry(uSequence);
            lpEntry->SetEntry(uSequence);
        }
    }

    void Commit(SequenceBatch &sequenceBatch)
    {
        Commit(sequenceBatch.GetStart(), sequenceBatch.GetSize());
    }

private:
    void EnsureSequenceConsumed(uint64_t uSequence)
    {
        auto uWrapPoint = uSequence - m_lpRingBuffer->GetCapacity();
        while (uWrapPoint > m_uLastSequence)
        {
            std::this_thread::yield();
            m_uLastSequence = GetMinConsumedSequence();
        }
    }

    uint64_t GetMinConsumedSequence()
    {
        uint64_t uMinimum = UINT64_MAX;
        for (auto &consumer : m_mapConsumers)
        {
            auto uSequence = consumer.first->GetSequence();
            uMinimum = uMinimum < uSequence ? uMinimum : uSequence;
        }

        return uMinimum;
    }

    void Commit(uint64_t uSequence, uint32_t uBatchSize)
    {
        auto uExpectSequence = uSequence - uBatchSize;
        while (uExpectSequence != m_lpRingBuffer->GetCurSor())
        {
            std::this_thread::yield();
        }

        m_lpRingBuffer->SetCursor(uSequence);
        m_lpWaitStrategy->NotifyAll();
    }

private:
    RingBuffer<Entry> *m_lpRingBuffer{nullptr};
    ClaimStrategy *m_lpClaimStrategy{nullptr};
    WaitStrategy<Entry> *m_lpWaitStrategy{nullptr};
    uint64_t m_uLastSequence{uInvalidSequence};
    std::map<Consumer<Entry> *, std::string> &m_mapConsumers;
};

} // end namespace FixedDisruptor

#endif //__PRODUCER_H
