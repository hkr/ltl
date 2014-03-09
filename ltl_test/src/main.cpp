#include "ltl/task_queue.hpp"
#include "ltl/await.hpp"
#include "ltl/when_all.hpp"
#include "ltl/when_any.hpp"


#include "ltlcontext/ltlcontext.hpp"

#include <stdio.h>
#include <string>
#include <sstream>

#include "assert.h"

std::mutex m;
int finished = 0;
std::condition_variable cv;

ltl::task_queue mainQueue("main");
ltl::task_queue otherQueues[2] = { ltl::task_queue("other1") , ltl::task_queue("other2") };

namespace {
   
int const loopEnd = 1000;
int loopCount = 0;
    
std::string get_string(int idx, int cnt)
{
    std::stringstream ss;
    ss << "get_string: " << idx << " / " << cnt;
    printf("%s\n", ss.str().c_str());
    return ss.str();
}

void print(std::string const& str)
{
    printf("print %s\n", str.c_str());
    
}
    
void new_main(int idx)
{
    for (int i = 0; i < loopEnd; ++i)
    {
        int c = ++loopCount;
        std::string const str = ltl::await |= otherQueues[i%2] <<= [=](){ return get_string(idx, c); };
        ltl::await (otherQueues[(i + i%2) % 2].push_back_resumable([=](){ print(str); }));
    }

    ltl::await(ltl::async([&](){
        std::unique_lock<std::mutex> lock(m);
        ++finished;
        cv.notify_one();
    }));
}
    
} // namespace

int main(int argc, char** argv)
{
    {
        mainQueue.push_back_resumable([](){ new_main(1); });
        mainQueue.push_back_resumable([](){ new_main(2); });
        mainQueue.push_back_resumable([](){ new_main(3); });
        
        std::unique_lock<std::mutex> lock(m);
        cv.wait(lock, [&](){ return finished == 3; });
    }
    
    ltl::future<void> f = mainQueue.push_back_resumable([](){});
    f.get();
    
    assert(loopCount == 3 * loopEnd);

    for (auto&& q : otherQueues)
        q.join();

    mainQueue.join();
    
    
    ltl::future<ltl::future<int>> xff;
    ltl::future<int> xf = xff.unwrap();
    
    std::vector<ltl::future<int>> vfi;
    when_all(std::begin(vfi), std::end(vfi));
    when_all_ready(std::begin(vfi), std::end(vfi));
    when_any_ready(std::begin(vfi), std::end(vfi));
    
    std::vector<ltl::future<void>> vfv;
    when_all_ready(std::begin(vfv), std::end(vfv));
    when_any_ready(std::begin(vfv), std::end(vfv));
    
    std::tuple<ltl::future<int>, ltl::future<std::string>> tfis;

    ltl::future<std::tuple<int, std::string>> r = when_all(tfis);
    ltl::future<void> fr = when_all_ready(tfis);
    

    
	return 0;
}
