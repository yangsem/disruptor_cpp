#ifndef __CLAIM_STRATEGY_H
#define __CLAIM_STRATEGY_H

#include "utility.h"

namespace VariableDisrupt
{

enum class ClaimStrategyOption
{
    SingleThread = 0,
    MultiThread
};

class ClaimStrategy
{
public:
    virtual ~ClaimStrategy() = default;

    virtual int64_t GetSequence(uint32_t uEntrySize) = 0;

    static ClaimStrategy *NewInstance(ClaimStrategyOption eOption);

    static void DeleteInstance(ClaimStrategy *lpStrategy);
};

class SingleThreadStrategy : public ClaimStrategy
{
public:
    SingleThreadStrategy() = default;

    ~SingleThreadStrategy() override = default;

    int64_t GetSequence(uint32_t uEntrySize) override
    {
        auto uSequence = m_uSequence;
        m_uSequence += uEntrySize;
        return uSequence;
    }

private:
    int64_t m_uSequence{0};
};

class MultiThreadStrategy : public ClaimStrategy
{
public:
    MultiThreadStrategy() = default;

    ~MultiThreadStrategy() override = default;

    int64_t GetSequence(uint32_t uEntrySize) override
    {
        return m_uSequence.fetch_add(uEntrySize);
    }

private:
    std::atomic<int64_t> m_uSequence{0};
};

ClaimStrategy *ClaimStrategy::NewInstance(ClaimStrategyOption eOption)
{
    ClaimStrategy *lpStrategy = nullptr;
    switch (eOption)
    {
        case ClaimStrategyOption::SingleThread:
            lpStrategy = NEW SingleThreadStrategy;
            break;

        case ClaimStrategyOption::MultiThread:
            lpStrategy = NEW MultiThreadStrategy;
            break;

        default:
            lpStrategy = nullptr;
            break;
    }

    return lpStrategy;
}

void ClaimStrategy::DeleteInstance(ClaimStrategy *lpStrategy)
{
    if (lpStrategy != nullptr)
    {
        delete lpStrategy;
    }
}

} // end namespace VariableDisrupt

#endif //__CLAIM_STRATEGY_H

