#ifndef __RING_BUFFER_H
#define __RING_BUFFER_H

#include "utility.h"
#include "entry.h"
#include "claim_strategy.h"
#include "wait_strategy.h"

namespace VariableDisrupt
{

class RingBuffer
{
public:
    RingBuffer() = default;
    ~RingBuffer()
    {
        if (m_lpBuffer != nullptr)
        {
            delete[] m_lpBuffer;
        }
    }

    int32_t Init(const uint32_t uSize)
    {
        m_uSize = Utility::GetNextPowerOfTwo(uSize);
        m_uRingModMask = m_uSize - 1;

        m_lpBuffer = NEW uint8_t[m_uSize];
        if (m_lpBuffer == nullptr)
        {
            return -1;
        }

        return 0;
    }

    int64_t GetCurSor()
    {
        return m_uCursor;
    }

    void SetCursor(int64_t uCursor)
    {
        m_uCursor = uCursor;
    }

    uint32_t GetCapacity()
    {
        return m_uSize;
    }

    Entry *GetEntry(int64_t uSequence)
    {
        auto lpEntry = (Entry *)(&m_lpBuffer[uSequence & m_uRingModMask]);
        return lpEntry;
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
    int64_t m_uCursor{0};
    uint8_t m_padding2[64]; // cache line padding
    uint8_t *m_lpBuffer{nullptr};
    uint32_t m_uRingModMask{0};
    uint32_t m_uSize{0};
    bool m_bAlerted{false};
};

} // end namespace VariableDisrupt

#endif //__RING_BUFFER_H