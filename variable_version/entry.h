#ifndef __ENTRY_H
#define __ENTRY_H

#include "utility.h"

namespace VariableDisrupt
{

constexpr char  MAGIC[] = "disr";

class Entry
{
public:
    int64_t GetSequence()
    {
        return m_uSequence;
    }

    void SetSequence(int64_t uSequence)
    {
        m_uSequence = uSequence;
    }

    void SetData(const void *lpData, uint32_t uLength)
    {
        *(uint32_t*)m_szMagic = *(uint32_t*)MAGIC;
        m_uDataLength = uLength;
        memcmp(m_Data, lpData, uLength);
    }

    uint32_t GetDataLength()
    {
        return m_uDataLength;
    }

    uint8_t *GetData()
    {
        assert(*(uint32_t*)m_szMagic == *(uint32_t*)MAGIC);
        return m_Data;
    }

public:
    int64_t m_uSequence{0};
    uint8_t m_szMagic[4];
    uint32_t m_uDataLength;
    uint8_t m_Data[];
};

} // end namespace VariableDisrupt

#endif //__ENTRY_H
