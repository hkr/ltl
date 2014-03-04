#include "ltl/task_queue.hpp"
#include "ltl/await.hpp"

#include "ltlcontext/ltlcontext.hpp"

#include <stdio.h>
#include <string>

#include <unistd.h>

std::mutex m;
int finished = 0;
std::condition_variable cv;

ltl::task_queue mainQueue("main");
ltl::task_queue otherQueue("other");

namespace {
    
    
std::string get_string()
{
    printf("get_string \n");
    return std::string("get_string");
}

void print(std::string const& str)
{
    printf("print %s\n", str.c_str());
    
}

void new_main()
{
    for (int i = 0; i < 1000; ++i)

    {
        std::string const str = ltl::await(otherQueue.execute(get_string));
        ltl::await (otherQueue.execute([=](){ print(str); }));
    }

    ltl::await(otherQueue.execute([&](){
        std::unique_lock<std::mutex> lock(m);
        ++finished;
        cv.notify_one();
    }));
}
    
} // namespace

int main(int argc, char** argv)
{
    {
        mainQueue.enqueue(&new_main);
        mainQueue.enqueue(&new_main);
        mainQueue.enqueue(&new_main);
        
        std::unique_lock<std::mutex> lock(m);
        cv.wait(lock, [&](){ return finished == 3; });
    }
    
    usleep(1000000);
    
    otherQueue.join();
    mainQueue.join();
    
	return 0;
}
