#ifndef __CONSUMER_H
#define __CONSUMER_H

#include "utility.h"
#include "ring_buffer.h"
#include "sequence_batch.h"

namespace FixedDisruptor
{
template <typename Entry> class RingBuffer;

template <typename Entry>
class Consumer
{
public:
    Consumer(RingBuffer<Entry> *lpRingBuffer, 
             ClaimStrategy *lpClaimStrategy, 
             WaitStrategy<Entry> *lpWaitStrategy)
        : m_lpRingBuffer(lpRingBuffer)
        , m_lpClaimStrategy(lpClaimStrategy)
        , m_lpWaitStrategy(lpWaitStrategy)
    {
    }

    ~Consumer() = default;

    Entry *GetEntry(uint64_t uSequence)
    {
        return m_lpRingBuffer->GetEntry(uSequence);
    }

    void Commit(uint64_t uSequence)
    {
        m_uLastSequence = uSequence;
    }

    uint64_t WaitFor(uint64_t uSequence, uint32_t uTimeoutUs)
    {
        return m_lpWaitStrategy->WaitFor(m_lpRingBuffer, uSequence, uTimeoutUs);
    }

    uint64_t GetSequence()
    {
        return m_uLastSequence;
    }

private:
    RingBuffer<Entry> *m_lpRingBuffer{nullptr};
    ClaimStrategy *m_lpClaimStrategy{nullptr};
    WaitStrategy<Entry> *m_lpWaitStrategy{nullptr};
    uint64_t m_uLastSequence{uInvalidSequence};
};

} // end namespace FixedDisruptor

#endif //__CONSUMER_H
