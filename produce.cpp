/*
 * 有一家生产奶酪的厂家，每天需要生产100000份奶酪卖给超市.
 * 通过一辆送货车发货，送货车辆每次送100份。
 * 厂家有一个容量为1000份的冷库，用于奶酪保鲜，生产的奶酪需要先存放在冷库。
 * 运输车辆从冷库取货。
 * 厂家有三条生产线，分别是牛奶供应生产线、发酵剂制作生产线、奶酪生产线。生产每份奶酪需要2份牛奶和1份发酵剂。
 *
 * 请设计生产系统。

 * 作者：sunyelw
 * 链接：https://www.jianshu.com/p/f53ab3821d85
 * 来源：简书
 * 著作权归作者所有。商业转载请联系作者获得授权，非商业转载请注明出处。
 */

/*
 * 三条生产线加一个送货车就是四个线程
 * 奶酪生产需要等待牛奶与发酵剂
 * 送货车需要等待奶酪数量满足一次运送的量才开始运送
 *
 * 奶酪两种情况下需要停止生产:
 * 1. 达到冷藏库数量
 * 2. 达到当天奶酪生产目标
 */

#include <atomic>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <thread>
#include <condition_variable>

template <typename T>
class SyncQueue
{
public:
    SyncQueue(int maxsize) : maxsize_(maxsize) {}
    ~SyncQueue() {}

    void Put(const T &x)
    {
        std::lock_guard<std::mutex> locker(mutex_);
        while (IsFull())
        {
            std::cout << "Buffer is full, waiting..." << std::endl;
            not_full_.wait(mutex_);
        }

        queue_.push_back(x);
        not_empty_.notify_one();
    }

    void Take(T &x)
    {
        std::lock_guard<std::mutex> locker(mutex_);
        while (IsEmpty())
        {
            std::cout << "Buffer is empty, waiting..." << std::endl;
            not_empty_.wait(mutex_);
        }

        x = queue_.front();
        queue_.pop_front();
        not_full_.notify_one();
    }

private:
    bool IsFull() const
    {
        return queue_.size() == maxsize_;
    }

    bool IsEmpty() const
    {
        return queue_.empty();
    }

private:
    std::list<T> queue_;
    std::mutex mutex_;
    std::condition_variable_any not_empty_;
    std::condition_variable_any not_full_;
    int maxsize_;
};

class ProduceOnlineBus
{
public:
    ProduceOnlineBus(uint64_t productNum)
        : productNum_(productNum),
          trucked(0),
          milkProded(0),
          fjjProded(0),
          cheeseProded(0)
    {
        milkCount_ = milkMultiple_ * productNum_;
        fjjCount_ = fjjMultiple_ * productNum_;
        needTruckNum_ = productNum_ / truckCapacity_;
        milkNodeBlockingDeque = new SyncQueue<int>(milkCount_);
        fjjNodeBlockingDeque = new SyncQueue<int>(fjjCount_);
        cheeseNodeBlockingDeque = new SyncQueue<int>(chessCapacity_);
    }
    ~ProduceOnlineBus()
    {
    }

    void start();

private:
    uint64_t productNum_;           // 生产奶酪的数量
    uint64_t fjjMultiple_ = 1;      // 生产每块奶酪需要的发酵剂的数量
    uint64_t milkMultiple_ = 2;     // 生产每块奶酪需要的牛奶的数量
    uint64_t chessCapacity_ = 1000; // 奶酪仓库容量
    uint64_t truckCapacity_ = 100;  // 单次运输的奶酪数量
    uint64_t needTruckNum_;         // 总共需要运送的次数
    uint32_t milkCount_;            // 需要牛奶的总数量
    uint32_t fjjCount_;             // 需要的发酵剂总数量

    std::atomic<int> trucked; // 实际运送的次数

    /**
     * 各生产线生产次数
     **/
    std::atomic<int> milkProded;   // 牛奶供应生产线生产数量
    std::atomic<int> fjjProded;    // 发酵剂制作生产线生产数量
    std::atomic<int> cheeseProded; // 奶酪生产线生产数量

    SyncQueue<int> *milkNodeBlockingDeque;   // 保存牛奶的队列
    SyncQueue<int> *fjjNodeBlockingDeque;    // 保存发酵剂的队列
    SyncQueue<int> *cheeseNodeBlockingDeque; // 保存奶酪的队列
};

void ProduceOnlineBus::start()
{
    // 牛奶生产线
    std::thread th1([&]()
                    {
                        for (size_t i = 0; i < milkCount_; ++i)
                        {
                            milkNodeBlockingDeque->Put(i);
                            ++milkProded;
                        }
                    });

    // 发酵剂生产线
    std::thread th2([&]()
                    {
                        for (size_t i = 0; i < fjjCount_; ++i)
                        {
                            fjjNodeBlockingDeque->Put(i);
                            ++fjjProded;
                        }
                    });

    // 奶酪生产线
    std::thread th3([&]()
                    {
                        for (size_t i = 0; i < productNum_; ++i)
                        {
                            int x;
                            for (size_t j = 0; j < milkMultiple_; ++j)
                            {
                                milkNodeBlockingDeque->Take(x);
                            }

                            for (size_t j = 0; j < fjjMultiple_; ++j)
                            {
                                fjjNodeBlockingDeque->Take(x);
                            }

                            cheeseNodeBlockingDeque->Put(i);
                            ++cheeseProded;
                        }
                    });

    // 运输车辆线程
    std::thread th4([&]()
                    {
                        while (trucked < needTruckNum_)
                        {
                            int x;
                            for (size_t i = 0; i < truckCapacity_; ++i)
                            {
                                cheeseNodeBlockingDeque->Take(x);
                            }
                            ++trucked;
                        }
                    });

    if (th1.joinable())
        th1.join();
    if (th2.joinable())
        th2.join();
    if (th3.joinable())
        th3.join();
    if (th4.joinable())
        th4.join();

    std::cout << "生产牛奶的数量：" << milkProded << " .生产发酵剂的数量：" << fjjProded << " .生产奶酪的数量：" << cheeseProded << " .车辆运输的次数：" << trucked << std::endl;
}

int main()
{
    ProduceOnlineBus *pb = new ProduceOnlineBus(100000);
    pb->start();
    return 0;
}
