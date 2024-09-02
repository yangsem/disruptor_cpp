#ifndef __SEQUENCE_BATCH_H
#define __SEQUENCE_BATCH_H

#include "utility.h"

namespace FixedDisruptor
{

class SequenceBatch
{
public:
    SequenceBatch(uint32_t uSize) : m_uSize(uSize) {}

    void SetEnd(uint64_t uEnd)
    {
        m_uEnd = uEnd;
    }

    uint64_t GetEnd()
    {
        return m_uEnd;
    }

    uint32_t GetSize()
    {
        return m_uSize;
    }

    uint64_t GetStart()
    {
        return m_uEnd - m_uSize + 1;
    }

private:
    const uint32_t m_uSize{0};
    uint64_t m_uEnd{uInvalidSequence};
};

} // end namespace FixedDisruptor

#endif //__SEQUENCE_BATCH_H
