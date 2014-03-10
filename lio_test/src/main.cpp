#include "lio/iomanager.hpp"
#include "lio/server.hpp"
#include "lio/socket.hpp"

#include <iostream>

namespace {
    
    std::shared_ptr<lio::iomanager> iomgr;
    
    void connected(std::shared_ptr<lio::socket> const& s)
    {
        iomgr->stop();
    }
}

int main(int argc, char** argv)
{
    iomgr = lio::iomanager::create();
    lio::server srv = iomgr->create_server("0.0.0.0", 12345, connected);
    
    std::cout << "test" << std::endl;
    
    iomgr->run();
    
	return 0;
}
