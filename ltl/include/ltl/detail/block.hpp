#ifndef LTL_BLOCK_HPP
#define LTL_BLOCK_HPP

#include <mutex>
#include <condition_variable>

namespace ltl {
namespace detail {
    
class block
{
public:
    block() : mutex_(), cv_(), signalled_(false)
    {
    }
    
    ~block()
    {
        signal();
    }
    
    block(block const&) =delete;
    block& operator=(block const&) =delete;
    
    void wait()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [&](){ return signalled_; });
    }
    
    void signal()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (signalled_)
            return;
        
        signalled_ = true;
        cv_.notify_all();
    }
    
private:
    std::mutex mutex_;
    std::condition_variable cv_;
    bool signalled_;
};

} // namespace detail
} // namespace ltl

#endif // LTL_BLOCK_HPP
