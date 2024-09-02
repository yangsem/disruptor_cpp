#ifndef __CLAIM_STRATEGY_H
#define __CLAIM_STRATEGY_H

#include "utility.h"

namespace FixedDisruptor
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

    virtual uint64_t GetNextSequence() = 0;

    virtual uint64_t GetBatchSequence(uint64_t uBatchSize) = 0;

    virtual void SetSequence(uint64_t uSequence) = 0;

    static ClaimStrategy *NewInstance(ClaimStrategyOption eOption);

    static void DeleteInstance(ClaimStrategy *lpStrategy);
};

class SingleThreadStrategy : public ClaimStrategy
{
public:
    SingleThreadStrategy() = default;

    ~SingleThreadStrategy() override = default;

    uint64_t GetNextSequence() override
    {
        return ++m_uSequence;
    }

    uint64_t GetBatchSequence(uint64_t uBatchSize) override
    {
        m_uSequence += uBatchSize;
        return m_uSequence;
    }

    void SetSequence(uint64_t uSequence) override
    {
        m_uSequence = uSequence;
    }

private:
    uint64_t m_uSequence{uInvalidSequence};
};

class MultiThreadStrategy : public ClaimStrategy
{
public:
    MultiThreadStrategy() = default;

    ~MultiThreadStrategy() override = default;

    uint64_t GetNextSequence() override
    {
        return ++m_uSequence;
    }

    uint64_t GetBatchSequence(uint64_t uBatchSize) override
    {
        return m_uSequence += uBatchSize;
    }

    void SetSequence(uint64_t uSequence) override
    {
        m_uSequence.store(uSequence);
    }

private:
    std::atomic<uint64_t> m_uSequence{uInvalidSequence};
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

} // end namespace FixedDisruptor

#endif //__CLAIM_STRATEGY_H

