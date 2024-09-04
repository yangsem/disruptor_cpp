#ifndef __CONSUMER_H
#define __CONSUMER_H

#include "utility.h"
#include "ring_buffer.h"
#include "entry.h"

namespace VariableDisrupt
{

class RingBuffer;

class Consumer
{
public:
    Consumer(RingBuffer *lpRingBuffer, 
             ClaimStrategy *lpClaimStrategy, 
             WaitStrategy *lpWaitStrategy)
        : m_lpRingBuffer(lpRingBuffer)
        , m_lpClaimStrategy(lpClaimStrategy)
        , m_lpWaitStrategy(lpWaitStrategy)
    {
    }

    ~Consumer() = default;

    Entry *GetEntry(int64_t uSequence)
    {
        return m_lpRingBuffer->GetEntry(uSequence);
    }

    void Commit(Entry *lpEntry)
    {
        m_uNextConsume = lpEntry->GetSequence() + lpEntry->GetDataLength();
    }

    int64_t WaitFor(int64_t uSequence, uint32_t uTimeoutUs)
    {
        return m_lpWaitStrategy->WaitFor(m_lpRingBuffer, uSequence, uTimeoutUs);
    }

    int64_t GetSequence()
    {
        return m_uNextConsume;
    }

private:
    RingBuffer *m_lpRingBuffer{nullptr};
    ClaimStrategy *m_lpClaimStrategy{nullptr};
    WaitStrategy *m_lpWaitStrategy{nullptr};
    int64_t m_uNextConsume{0};
};

} // end namespace VariableDisrupt

#endif //__CONSUMER_H
