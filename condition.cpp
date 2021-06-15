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

int main()
{
    SyncQueue<int> sync_queue(5);
    std::thread PutDatas([&]() {
        for (int i = 0; i < 20; ++i)
        {
            sync_queue.Put(i);
        }
    });

    std::thread TakeDatas([&]() {
        int x = 0;
        for (int i = 0; i < 20; ++i)
        {
            sync_queue.Take(x);
            std::cout << x << std::endl;
        }
    });

    if (PutDatas.joinable())
    {
        PutDatas.join();
    }

    if (TakeDatas.joinable())
    {
        TakeDatas.join();
    }
    return 0;
}