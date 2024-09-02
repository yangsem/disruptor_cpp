#ifndef __RING_BUFFER_H
#define __RING_BUFFER_H

#include "utility.h"
#include "claim_strategy.h"
#include "wait_strategy.h"

namespace FixedDisruptor
{

template <typename Entry>
class RingBuffer
{
public:
    RingBuffer() = default;
    ~RingBuffer()
    {
        if (m_lpEntries != nullptr)
        {
            delete[] m_lpEntries;
        }
    }

    int32_t Init(const uint32_t uSize)
    {
        m_uSize = Utility::GetNextPowerOfTwo(uSize);
        m_uRingModMask = m_uSize - 1;

        m_lpEntries = NEW Entry[m_uSize];
        if (m_lpEntries == nullptr)
        {
            return -1;
        }

        return 0;
    }

    uint64_t GetCurSor()
    {
        return m_uCursor;
    }

    void SetCursor(uint64_t uCursor)
    {
        m_uCursor = uCursor;
    }

    uint32_t GetCapacity()
    {
        return m_uSize;
    }

    Entry *GetEntry(uint64_t uSequence)
    {
        return &m_lpEntries[uSequence & m_uRingModMask];
    }

    void SetAlert()
    {
        m_bAlerted = true;
    }

    void ClearAlert()
    {
        m_bAlerted = false;
    }

    bool IsAlerted()
    {
        return m_bAlerted;
    }

private:
    uint8_t m_padding1[64]; // cache line padding
    uint64_t m_uCursor{uInvalidSequence};
    uint8_t m_padding2[64]; // cache line padding
    Entry *m_lpEntries{nullptr};
    uint32_t m_uRingModMask{0};
    uint32_t m_uSize{0};
    bool m_bAlerted{false};
};

} // end namespace FixedDisruptor

#endif //__RING_BUFFER_H