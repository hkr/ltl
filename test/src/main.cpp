#include "ltl/task_queue.hpp"

#include "stdio.h"

std::mutex m;
std::condition_variable cv;

int main(int argc, char** argv)
{
    {
        ltl::task_queue tq;
            
        tq.enqueue([](){ printf("task1\n"); });
        tq.enqueue([](){ printf("task2\n"); });
        tq.enqueue([&](){ cv.notify_one(); });
        
        std::unique_lock<std::mutex> lock(m);
        cv.wait(lock);
        
        printf("exiting...\n");
    }
    
    printf("...done\n");
	return 0;
}
