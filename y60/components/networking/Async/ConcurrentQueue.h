#ifndef CONCURRENT_QUEUE
#define CONCURRENT_QUEUE

#include <boost/thread.hpp> 
#include <queue>

namespace y60 {
namespace async {

template<typename Data>
class ConcurrentQueue
{
private:
    std::queue<Data> the_queue;
    mutable boost::mutex the_mutex;
    boost::condition_variable the_condition_variable;
public:
    void push(Data const& data)
    {
        boost::mutex::scoped_lock lock(the_mutex);
        the_queue.push(data);
        lock.unlock();
        the_condition_variable.notify_one();
    }

    bool empty() const
    {
        boost::mutex::scoped_lock lock(the_mutex);
        return the_queue.empty();
    }


    bool try_pop(Data& popped_value)
    {
        boost::mutex::scoped_lock lock(the_mutex);
        if(the_queue.empty())
        {
            return false;
        }
        
        popped_value=the_queue.front();
        the_queue.pop();
        return true;
    }

    void wait_and_front(Data& front_value) 
    {
        boost::mutex::scoped_lock lock(the_mutex);
        while(the_queue.empty())
        {
            the_condition_variable.wait(lock);
        }
        
        front_value=the_queue.front();
    }

    void wait_and_pop(Data& popped_value)
    {
        wait_and_front(popped_value);
        the_queue.pop();
    }

};
}; // namespace async
}; //namespace y60
#endif // CONCURRENT_QUEUE
