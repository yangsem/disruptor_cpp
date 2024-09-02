#ifndef __ABSTRACT_ENTRY_H
#define __ABSTRACT_ENTRY_H

#include "utility.h"

namespace FixedDisruptor
{

class AbstractEntry
{
public:
    uint64_t GetSequence()
    {
        return m_uSequence;
    }

    void SetSequence(uint64_t uSequence)
    {
        m_uSequence = uSequence;
    }

private:
    uint64_t m_uSequence{uInvalidSequence};
};

} // end namespace FixedDisruptor

#endif //__ABSTRACT_ENTRY_H
