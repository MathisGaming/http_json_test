#pragma once

#include <mutex>
#include <thread>
#include <exception>
#include <queue>

template <typename T>
class threadsafe_queue
{
private:
    mutable std::mutex mutex;
    std::queue<T> queue;
    std::condition_variable cond;
public:
    threadsafe_queue()
    {}
    threadsafe_queue(const threadsafe_queue & other)
    {
        std::lock_guard<std::mutex> lk(other.mutex);
        queue = other.queue;
    }
    ~threadsafe_queue() {}

    void push(const T & new_value)
    {
        std::lock_guard<std::mutex> lk(mutex);
        queue.push(std::move(new_value));
        cond.notify_one();
    }

    void push(T && new_value)
    {
        std::lock_guard<std::mutex> lk(mutex);
        queue.push(new_value);
        cond.notify_one();
    }
    
    void wait_and_pop(T& value)
    {
        std::lock_guard<std::mutex> lk(mutex);
        cond.wait(lk, [this] {return !queue.empty(); });
        value = std::move(queue.front());
        queue.pop();
    }
    
    std::shared_ptr<T> wait_and_pop()
    {
        std::lock_guard<std::mutex> lk(mutex);
        cond.wait(lk, [this] {return !queue.empty(); });
        std::shared_ptr<T> res(std::make_shared<T>(std::move(queue.front())));
        queue.pop();
        return res;
    }
    
    bool try_and_pop(T & value)
    {
        std::lock_guard<std::mutex> lk(mutex);
        if (queue.empty())
        {
            return false;
        }
        value = queue.front();
        queue.pop();
        return true;
    }
    
    std::shared_ptr<T> try_and_pop()
    {
        std::lock_guard<std::mutex> lk(mutex);
        if (queue.empty())
        {
            return std::shared_ptr<T>();
        }
        std::shared_ptr<T> result(std::make_shared<T>(queue.front()));
        queue.pop();
        return result;
    }
    
    bool empty() const
    {
        std::lock_guard<std::mutex> lk(mutex);
        return queue.empty();
    }
};