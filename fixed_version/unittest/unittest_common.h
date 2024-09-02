#ifndef __UNITTEST_COMMON_H
#define __UNITTEST_COMMON_H

#include <disruptor_factory.h>
#include <iostream>
#include <pthread.h>

// 终端打印颜色宏
#define RESET "\033[0m"
#define RED "\033[0;31m"
#define YELLOW "\033[0;33m"
#define BLUE "\033[0;34m"
#define GREEN "\033[0;32m"

class ColorSelector
{
public:
    static const char *GetColor(uint32_t i)
    {
        static const char *Colocrs[] = {
            RED, YELLOW, BLUE, GREEN
        };

        return Colocrs[i % (sizeof(Colocrs) / sizeof(const char *))];
    }
};

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

    int32_t Init(FixedDisruptor::DisruptorFactory<MyEntry> *lpFactory, 
                 const char *lpConsumerName, uint32_t uIndex)
    {
        m_lpFactory = lpFactory;
        m_lpConsumer = m_lpFactory->NewConsumer(lpConsumerName);
        if (m_lpConsumer == nullptr)
        {
            return -1;
        }
        
        m_uIndex = uIndex;
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
        char szThreadName[16];
        snprintf(szThreadName, sizeof(szThreadName), "consumer_%u", m_uIndex);
        pthread_setname_np(pthread_self(), szThreadName);

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
                    
                    printf("%sconsumer %u pop  %024lu, time = %024lu\n" RESET, 
                            ColorSelector::GetColor(lpMyEntry->uEntryNo), 
                            m_uIndex, lpMyEntry->uEntryNo, lpMyEntry->uPop);
                    
                    printf("%sconsumer %u consumed count = %lu, wait time = %lu.%lu s\n" RESET, 
                            ColorSelector::GetColor(lpMyEntry->uEntryNo), 
                            m_uIndex, m_uConsumedCount, 
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
    uint32_t m_uIndex{0};
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

    int32_t Init(FixedDisruptor::DisruptorFactory<MyEntry> *lpFactory, 
                 const char *lpProducerName, uint32_t uIndex,  uint32_t uSleepTImeUs)
    {
        m_lpFactory = lpFactory;
        m_lpProducer = m_lpFactory->NewProducer(lpProducerName);
        if (m_lpProducer == nullptr)
        {
            return -1;
        }

        m_uIndex = uIndex;
        m_uSleepTImeUs = uSleepTImeUs;
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
        char szThreadName[16];
        snprintf(szThreadName, sizeof(szThreadName), "producer_%u", m_uIndex);
        pthread_setname_np(pthread_self(), szThreadName);

        MyEntry *lpMyEntry = nullptr;

        while (m_bRunning)
        {
            lpMyEntry = m_lpProducer->NewEntry();
            if (lpMyEntry != nullptr)
            {
                lpMyEntry->uEntryNo = m_uProductedCount;
                lpMyEntry->uPush = FixedDisruptor::Utility::GetTimeNs();
                printf("%sproducer %u push %024lu, time = %024lu\n" RESET, 
                        ColorSelector::GetColor(lpMyEntry->uEntryNo), 
                        m_uIndex, m_uProductedCount, lpMyEntry->uPush);
                m_lpProducer->Commit(lpMyEntry);

                m_uProductedCount++;

                std::this_thread::sleep_for(std::chrono::microseconds(m_uSleepTImeUs));
            }
        }
    }

private:
    FixedDisruptor::DisruptorFactory<MyEntry> *m_lpFactory{nullptr};
    FixedDisruptor::Producer<MyEntry> *m_lpProducer{nullptr};

    std::thread m_thProducer;
    bool m_bRunning{false};
    uint32_t m_uSleepTImeUs{0};
    uint32_t m_uIndex{0};
    uint64_t m_uProductedCount{0};
};

#endif //__UNITTEST_COMMON_H
