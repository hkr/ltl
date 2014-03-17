#include "lio/iomanager.hpp"
#include "lio/server.hpp"
#include "lio/socket.hpp"
#include "ltl/future.hpp"
#include "ltl/await.hpp"
#include "ltl/task_queue.hpp"

#include <iostream>
#include <stdio.h>

namespace {
    ltl::task_queue server_tq("server");
    ltl::task_queue client_tq("client");
    ltl::task_queue client_tq2("client2");
    
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
    
    ltl::detail::block block;
    const std::string terminate_connection("terminate");
    
    int msgcount = 0;
    
}

int main(int argc, char** argv)
{
    server_iomgr = lio::iomanager::create("server");
    client_iomgr = lio::iomanager::create("client");
    auto third_iomgr = lio::iomanager::create("third");
    
    third_iomgr->run();
    client_iomgr->run();
    server_iomgr->run();
    
    std::shared_ptr<lio::server> srv = server_iomgr->create_server("0.0.0.0", 12345, [](std::shared_ptr<lio::socket> const& s){
        server_tq.push_back_resumable([=](){
            for (int i = 0; i < 5; ++i)
            {
                try {
                    std::string msg = read(s);
                    std::cout << msg << std::endl;
                    
                    if (msg == terminate_connection)
                    {
                        block.signal();
                        return;
                    }
                    else
                    {
                        ++msgcount;
                    }
                    
                } catch(...)
                {
                    return;
                }
            }
        });
    }).get();
    

    for (int i = 0; i < 1; ++i)
    {
        client_tq.push_back_resumable([=](){
            std::shared_ptr<lio::socket> s = ltl::await |= client_iomgr->connect("127.0.0.1", 12345);         
            std::string msg("XX-Hello World-0");
            msg[0] = '0' + (char) i / 10;
            msg[1] = '0' + (char) i % 10;
            
            msg[msg.size()-1] = '0';
            write(s, msg);
            
            msg[msg.size()-1] = '1';
            write(s, msg);

            msg[msg.size()-1] = '2';
            write(s, msg);
            
            msg[msg.size()-1] = '3';
            write(s, msg);
        });
    }
   
    client_tq.push_back([](){}).wait();
    client_tq.join();
    
    client_tq2.push_back_resumable([](){
        std::shared_ptr<lio::socket> s = ltl::await |= client_iomgr->connect("127.0.0.1", 12345);
        write(s, terminate_connection);
    });
    
    
    block.wait();
    
    printf("received %d messages total\n", msgcount);
    
    printf("srv->close()...\n");
    srv->close();
    
    printf("server_tq.join...\n");
    server_tq.join();
    
    printf("leaving main\n");
    
	return 0;
}
