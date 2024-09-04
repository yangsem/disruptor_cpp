#ifndef __DISRUPTOR_FACTORY_H
#define __DISRUPTOR_FACTORY_H

#include "ring_buffer.h"
#include "claim_strategy.h"
#include "wait_strategy.h"
#include "producer.h"
#include "consumer.h"
#include "entry.h"

namespace VariableDisrupt
{

class DisruptorFactory
{
public:
    DisruptorFactory() = default;

    ~DisruptorFactory()
    {
        for (auto &consumer : m_mapConsumers)
        {
            delete consumer.first;
        }

        for (auto &producer : m_mapProducers)
        {
            delete producer.first;
        }

        if (m_lpWaitStrategt != nullptr)
        {
            delete m_lpWaitStrategt;
        }

        if (m_lpClaimStrategy != nullptr)
        {
            delete m_lpClaimStrategy;
        }

        if (m_lpRingBuffer != nullptr)
        {
            delete m_lpRingBuffer;
        }
    }

    RingBuffer *GetRingBuffer()
    {
        return m_lpRingBuffer;
    }

    Producer *NewProducer(const char *lpProducerName)
    {
        std::unique_ptr<Producer> upProducer(
            NEW Producer(m_lpRingBuffer, m_lpClaimStrategy, 
                                m_lpWaitStrategt, m_mapConsumers)
        );
        
        auto lpProducer = upProducer.get();
        if (lpProducer == nullptr)
        {
            return nullptr;
        }

        try
        {
            m_mapProducers.emplace(lpProducer, lpProducerName);
        }
        catch(...)
        {
            return nullptr;
        }

        upProducer.release();
        return lpProducer;
    }

    void DeleteProducer(Producer *lpProducer)
    {
        auto iter = m_mapProducers.find(lpProducer);
        if (iter != m_mapProducers.end())
        {
            delete lpProducer;
            m_mapProducers.erase(iter);
        }
    }

    Consumer *NewConsumer(const char *lpConsumerName)
    {
        std::unique_ptr<Consumer> upConsumer(
            NEW Consumer(m_lpRingBuffer, m_lpClaimStrategy, m_lpWaitStrategt)
        );

        auto lpConsumer = upConsumer.get();
        if (lpConsumer == nullptr)
        {
            return nullptr;
        }

        try
        {
            m_mapConsumers.emplace(lpConsumer, lpConsumerName);
        }
        catch(...)
        {
            return nullptr;
        }

        upConsumer.release();
        return lpConsumer;
    }

    void DeleteConsumer(Consumer *lpConsumer)
    {
        auto iter = m_mapConsumers.find(lpConsumer);
        if (iter != m_mapConsumers.end())
        {
            delete lpConsumer;
            m_mapConsumers.erase(iter);
        }
    }

    static DisruptorFactory *NewDisruptorFactory(
        uint32_t uRingBufferSize,
        ClaimStrategyOption eClaimStrategyOption,
        WaitStrategyOption eWaitStrategyOption)
    {
        std::unique_ptr<DisruptorFactory> upFactory(NEW DisruptorFactory);
        auto lpFactory = upFactory.get();
        if (lpFactory == nullptr)
        {
            return nullptr;
        }

        lpFactory->m_lpRingBuffer = NEW RingBuffer;
        if (lpFactory->m_lpRingBuffer == nullptr 
            || lpFactory->m_lpRingBuffer->Init(uRingBufferSize) != 0)
        {
            return nullptr;
        }

        lpFactory->m_lpClaimStrategy = ClaimStrategy::NewInstance(eClaimStrategyOption);
        if (lpFactory->m_lpClaimStrategy == nullptr)
        {
            return nullptr;
        }

        lpFactory->m_lpWaitStrategt = WaitStrategy::NewInstance(eWaitStrategyOption);
        if (lpFactory->m_lpWaitStrategt == nullptr)
        {
            return nullptr;
        }

        upFactory.release();
        return lpFactory;
    }

    static void DeleteDisruptorFactory(DisruptorFactory *lpFactory)
    {
        delete lpFactory;
    }

private:
    RingBuffer *m_lpRingBuffer{nullptr};
    ClaimStrategy *m_lpClaimStrategy{nullptr};
    WaitStrategy *m_lpWaitStrategt{nullptr};
    std::map<Producer *, std::string> m_mapProducers;
    std::map<Consumer *, std::string> m_mapConsumers;
};

} // end namespace VariableDisrupt

#endif //__DISRUPTOR_FACTORY_H