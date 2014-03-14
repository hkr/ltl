#include "lio/iomanager.hpp"
#include "lio/server.hpp"
#include "lio/socket.hpp"
#include "ltl/future.hpp"
#include "ltl/await.hpp"
#include "ltl/task_queue.hpp"

#include <iostream>
#include <stdio.h>

namespace {
    ltl::task_queue server_tq;
    ltl::task_queue client_tq;
    
    std::string read(std::shared_ptr<lio::socket> const& s)
    {
        std::vector<std::uint8_t> buf = ltl::await |= s->read(sizeof(size_t));
        size_t size = 0;
        std::uninitialized_copy(std::begin(buf), std::end(buf), reinterpret_cast<std::uint8_t*>(&size));
        std::vector<std::uint8_t> data = ltl::await |= s->read(size);
        return std::string(data.begin(), data.end());
    }
   
    void write(std::shared_ptr<lio::socket> const& s, std::string const& data)
    {
        size_t size = data.size();
        ltl::await |= s->write(&size, sizeof(size_t));
        ltl::await |= s->write(data.data(), size);
    }
    
    std::shared_ptr<lio::iomanager> server_iomgr;
    std::shared_ptr<lio::iomanager> client_iomgr;
}

int main(int argc, char** argv)
{
    server_iomgr = lio::iomanager::create("server");
    client_iomgr = lio::iomanager::create("client");
    auto third_iomgr = lio::iomanager::create("third");
    
    std::shared_ptr<lio::server> srv = server_iomgr->create_server("0.0.0.0", 12345, [](std::shared_ptr<lio::socket> const& s){
        printf("server: client connected\n");
        server_tq.push_back_resumable([=](){
            while (true)
            {
                std::string msg = read(s);
                std::cout << msg << std::endl;
            }
        });
    }).get();
    
    std::thread clients([](){
        for (int i = 0; i < 1; ++i)
        {
            client_tq.push_back_resumable([](){
                printf("client: trying to connect\n");
                std::shared_ptr<lio::socket> s = ltl::await |= client_iomgr->connect("127.0.0.1", 12345);
                printf("client: connected\n");
                write(s, "Hello World0");
                write(s, "Hello World1");
                write(s, "Hello World2");
                write(s, "Hello World3");
            });
        }
        
        client_tq.push_back_resumable([](){
            //   server_iomgr->stop();
            //client_iomgr->stop();
        });
    });
    
    printf("joining clients\n");
    clients.join();
   
    third_iomgr->run();
    client_iomgr->run().wait();
    server_iomgr->run().wait();
    //.wait();
    
    
    
    
    printf("leaving main\n");
    
	return 0;
}
