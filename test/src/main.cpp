#include "ltl/task_queue.hpp"

#include "ltlcontext/ltlcontext.hpp"

#include <stdio.h>
#include <string>

#include <unistd.h>

std::mutex m;
bool finished = false;
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
        std::string const str = ltl::await <= ltl::async(otherQueue, get_string);
        ltl::await <= ltl::async(otherQueue, [=](){
            print(str);
        });
    }

    ltl::await <= ltl::async(otherQueue, [&](){
        std::unique_lock<std::mutex> lock(m);
        finished = true;
        cv.notify_one();
    });
}
    
} // namespace

int main(int argc, char** argv)
{
    {
        mainQueue.enqueue(&new_main);
        
        std::unique_lock<std::mutex> lock(m);
        cv.wait(lock, [&](){ return finished; });
    }
    
    usleep(1000000);
    
    otherQueue.join();
    mainQueue.join();
    
	return 0;
}
