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
    auto iErrorNo = myProducer.Init(lpFactory, "singleton_producer");
    if (iErrorNo != 0)
    {
        std::cerr << "MyProducer Init Failed" << std::endl;
        return -1;
    }

    MyConsumer myConsumer;
    iErrorNo = myConsumer.Init(lpFactory, "singleton_blocking_consumer");
    if (iErrorNo != 0)
    {
        std::cerr << "MyConsumer Init Failed" << std::endl;
        return -1;
    }

    iErrorNo = myProducer.Start();
    if (iErrorNo != 0)
    {
        std::cerr << "MyProducer Start Failed" << std::endl;
        return -1;
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