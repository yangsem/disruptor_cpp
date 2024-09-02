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

    MyProducer myProducer;
    auto iErrorNo = myProducer.Init(lpFactory, "single_producer", 0, 1000000);
    if (iErrorNo != 0)
    {
        std::cerr << "MyProducer Init Failed" << std::endl;
        return -1;
    }

    MyConsumer myConsumer[2];
    for (uint32_t i = 0; i < sizeof(myConsumer) / sizeof(MyConsumer); i++)
    {
        iErrorNo = myConsumer[i].Init(lpFactory, "multi_blocking_consumer_1", i);
        if (iErrorNo != 0)
        {
            std::cerr << "MyConsumer " << i << " Init Failed" << std::endl;
            return -1;
        }
    }

    iErrorNo = myProducer.Start();
    if (iErrorNo != 0)
    {
        std::cerr << "MyProducer Start Failed" << std::endl;
        return -1;
    }

    for (uint32_t i = 0; i < sizeof(myConsumer) / sizeof(MyConsumer); i++)
    {
        iErrorNo = myConsumer[i].Start();
        if (iErrorNo != 0)
        {
            std::cerr << "MyConsumer " << i << " Start Failed" << std::endl;
            return -1;
        }
    }
    

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}