#include "unittest_common.h"

int main(int argc, const char *argv[])
{
    std::unique_ptr<FixedDisruptor::DisruptorFactory<MyEntry>> upFactory(
        FixedDisruptor::DisruptorFactory<MyEntry>::NewDisruptorFactory(
            1024, 
            FixedDisruptor::ClaimStrategyOption::SingleThread,
            FixedDisruptor::WaitStrategyOption::Blocking
        )
    );
    auto lpFactory =upFactory.get();
    if (lpFactory == nullptr)
    {
        std::cerr << "NewDisruptorFactory Failed" << std::endl;
        return -1;
    }

    constexpr uint32_t uSize = 2;
    MyProducer myProducer[uSize];
    MyConsumer myConsumer;

    for (uint32_t i = 0; i < uSize; i++)
    {
        auto iErrorNo = myProducer[i].Init(lpFactory, "single_producer", i, 1000000);
        if (iErrorNo != 0)
        {
            std::cerr << "MyProducer " << i << " Init Failed" << std::endl;
            return -1;
        }
    }
    

    auto iErrorNo = myConsumer.Init(lpFactory, "multi_blocking_consumer_1", 0);
    if (iErrorNo != 0)
    {
        std::cerr << "MyConsumer Init Failed" << std::endl;
        return -1;
    }

    for (uint32_t i = 0; i < uSize; i++)
    {
        iErrorNo = myProducer[i].Start();
        if (iErrorNo != 0)
        {
            std::cerr << "MyProducer " << i << " Start Failed" << std::endl;
            return -1;
        }
    }

    iErrorNo = myConsumer.Start();
    if (iErrorNo != 0)
    {
        std::cerr << "MyConsumer Start Failed" << std::endl;
        return -1;
    }

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}