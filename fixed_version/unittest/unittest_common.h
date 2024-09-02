#ifndef __UNITTEST_COMMON_H
#define __UNITTEST_COMMON_H

#include <disruptor_factory.h>
#include <iostream>

class MyEntry : public FixedDisruptor::AbstractEntry
{
public:
    uint64_t uEntryNo;
    uint64_t uPush;
    uint64_t uPop;
};

class MyConsumer
{
public:
    MyConsumer() = default;
    ~MyConsumer()
    {
        if (m_lpFactory != nullptr && m_lpConsumer != nullptr)
        {
            m_lpFactory->DeleteConsumer(m_lpConsumer);
        }
    }

    int32_t Init(FixedDisruptor::DisruptorFactory<MyEntry> *lpFactory, const char *lpConsumerName)
    {
        m_lpFactory = lpFactory;
        m_lpConsumer = m_lpFactory->NewConsumer(lpConsumerName);
        if (m_lpConsumer == nullptr)
        {
            return -1;
        }

        return 0;
    }

    int32_t Start()
    {
        try
        {
            m_bRunning = true;
            m_thConsumer = std::thread(&MyConsumer::Run, this);
        }
        catch(...)
        {
            return -1;
        }

        return 0;
    }

    void Stop()
    {
        m_bRunning = false;
        if (m_thConsumer.joinable())
        {
            m_thConsumer.join();
        }
    }

private:
    void Run()
    {
        MyEntry *lpMyEntry = nullptr;
        uint64_t uCurSequence = 0;

        while (m_bRunning)
        {
            auto uSequence = m_lpConsumer->WaitFor(uCurSequence, FixedDisruptor::uNoTimeout);
            if (uSequence >= uCurSequence)
            {
                lpMyEntry = m_lpConsumer->GetEntry(uCurSequence);
                if (lpMyEntry != nullptr)
                {
                    lpMyEntry->uPop = FixedDisruptor::Utility::GetTimeNs();
                    
                    printf("consumer pop  %024lu, time = %024lu\n", lpMyEntry->uEntryNo, lpMyEntry->uPop);
                    
                    printf("consumed count = %lu, wait time = %lu.%lu s\n", 
                            m_uConsumedCount, 
                            (lpMyEntry->uPop - lpMyEntry->uPush) / (1000 * 1000 * 1000ULL),
                            (lpMyEntry->uPop - lpMyEntry->uPush) % (1000 * 1000 * 1000ULL)
                    );
                    
                    m_lpConsumer->Commit(uCurSequence);
                    uCurSequence++;
                    
                    m_uConsumedCount++;
                }
            }
        }
    }

private:
    FixedDisruptor::DisruptorFactory<MyEntry> *m_lpFactory{nullptr};
    FixedDisruptor::Consumer<MyEntry> *m_lpConsumer{nullptr};

    std::thread m_thConsumer;
    bool m_bRunning{false};
    uint64_t m_uConsumedCount{0};
};

class MyProducer
{
public:
    MyProducer() = default;
    ~MyProducer()
    {
        if (m_lpFactory != nullptr && m_lpProducer != nullptr)
        {
            m_lpFactory->DeleteProducer(m_lpProducer);
        }
    }

    int32_t Init(FixedDisruptor::DisruptorFactory<MyEntry> *lpFactory, const char *lpProducerName)
    {
        m_lpFactory = lpFactory;
        m_lpProducer = m_lpFactory->NewProducer(lpProducerName);
        if (m_lpProducer == nullptr)
        {
            return -1;
        }

        return 0;
    }

    int32_t Start()
    {
        try
        {
            m_bRunning = true;
            m_thProducer = std::thread(&MyProducer::Run, this);
        }
        catch(...)
        {
            return -1;
        }

        return 0;
    }

    void Stop()
    {
        m_bRunning = false;
        if (m_thProducer.joinable())
        {
            m_thProducer.join();
        }
    }

private:
    void Run()
    {
        MyEntry *lpMyEntry = nullptr;

        while (m_bRunning)
        {
            lpMyEntry = m_lpProducer->NewEntry();
            if (lpMyEntry != nullptr)
            {
                lpMyEntry->uEntryNo = m_uProductedCount;
                lpMyEntry->uPush = FixedDisruptor::Utility::GetTimeNs();
                m_lpProducer->Commit(lpMyEntry);
                printf("producer push %024lu, time = %024lu\n", m_uProductedCount, lpMyEntry->uPush);

                m_uProductedCount++;

                std::this_thread::sleep_for(std::chrono::seconds(10));
            }
        }
    }

private:
    FixedDisruptor::DisruptorFactory<MyEntry> *m_lpFactory{nullptr};
    FixedDisruptor::Producer<MyEntry> *m_lpProducer{nullptr};

    std::thread m_thProducer;
    bool m_bRunning{false};
    uint64_t m_uProductedCount{0};
};

#endif //__UNITTEST_COMMON_H
