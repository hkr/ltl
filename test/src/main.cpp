#include "ltl/task_queue.hpp"

#include <stdio.h>
#include <string>

std::mutex m;
std::condition_variable cv;

namespace {
    std::string get_str1()
    {
        return "dsfsdf";
    }
    
    void print(std::string const& str)
    {
        printf("%s\n", str.c_str());

    }
    
}

int main(int argc, char** argv)
{
    {
        ltl::task_queue tq;
            
        tq.execute([]() { return get_str1(); })
            .then(&print);
        
        tq.enqueue([](){ printf("task2\n"); });
        tq.enqueue([&](){ cv.notify_one(); });
        
        std::unique_lock<std::mutex> lock(m);
        cv.wait(lock);
        
        printf("exiting...\n");
    }
    
    printf("...done\n");
	return 0;
}
