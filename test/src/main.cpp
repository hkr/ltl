#include "ltl/task_queue.hpp"
#include "ltl/await.hpp"

#include "ltlcontext/ltlcontext.hpp"

#include <stdio.h>
#include <string>
#include <sstream>

#include <unistd.h>
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
        ltl::await (otherQueues[(i + i%2) % 2].execute([=](){ print(str); }));
    }

    ltl::await(mainQueue.execute([&](){
        std::unique_lock<std::mutex> lock(m);
        ++finished;
        cv.notify_one();
    }));
}
    
} // namespace

int main(int argc, char** argv)
{
    {
        mainQueue.enqueue([](){ new_main(1); });
        mainQueue.enqueue([](){ new_main(2); });
        mainQueue.enqueue([](){ new_main(3); });
        
        std::unique_lock<std::mutex> lock(m);
        cv.wait(lock, [&](){ return finished == 3; });
    }
    
    assert(loopCount == 3 * loopEnd);
    usleep(1000000);
    
    for (auto&& q : otherQueues)
        q.join();

    mainQueue.join();
    
	return 0;
}
