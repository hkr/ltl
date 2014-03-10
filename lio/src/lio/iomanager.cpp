#include "lio/iomanager.hpp"

#include "lio/socket.hpp"
#include "lio/server.hpp"

#include "ltl/promise.hpp"
#include "ltl/future.hpp"

#include "uv.h"

namespace lio {
    
namespace {
  
struct delete_loop
{
    void operator()(uv_loop_t* l) const
    {
        if (l)
        {
            //luv_oop_close(l);
            //delete l;
            uv_loop_delete(l);
        }
    }
};
    
std::shared_ptr<uv_loop_s> make_loop()
{
    auto l = uv_loop_new();
    //uv_loop_init(l);
    return std::shared_ptr<uv_loop_s>(l, delete_loop());
}
    
} // namespace
    
struct iomanager::impl
{
    impl(impl const&) = delete;
    impl& operator=(impl const&) = delete;
    
    explicit impl(iomanager* ptr)
    : back_(ptr)
    , loop_(make_loop())
    {
        loop_->data = back_;
    }
    
    ~impl()
    {
    }
    
    server create_server(char const* ip, int port, std::function<void(std::shared_ptr<socket> const&)> on_connection)
    {
        return server(back_->shared_from_this(), ip, port, std::move(on_connection));
    }
    
    ltl::future<std::shared_ptr<socket>> connect(char const* ip, int port)
    {
        
        return ltl::make_future(std::shared_ptr<socket>());
    }
    
    static void connection_established()
    {
        
    }
    
    std::shared_ptr<uv_loop_s> get_loop()
    {
        return loop_;
    }
    
    void run()
    {
        uv_run(loop_.get(), UV_RUN_DEFAULT);
    }
    
    void stop()
    {
        uv_stop(loop_.get());
    }
    
private:
    iomanager* const back_;
    std::shared_ptr<uv_loop_s> loop_;
};

server iomanager::create_server(char const* ip, int port, std::function<void(std::shared_ptr<socket> const&)> on_connection)
{
    return impl_->create_server(ip, port, std::move(on_connection));
}
    
ltl::future<std::shared_ptr<socket>> iomanager::connect(char const* ip, int port)
{
    return impl_->connect(ip, port);
}

iomanager::iomanager()
: impl_(new impl(this))
{
    
}
    
iomanager::~iomanager()
{
    
}

void iomanager::destroy(iomanager* ptr)
{
    delete ptr;
}
    
std::shared_ptr<iomanager> iomanager::create()
{
    return std::shared_ptr<iomanager>(new iomanager, &destroy);
}
    
std::shared_ptr<uv_loop_s> iomanager::get_loop()
{
    return impl_->get_loop();
}
    
void iomanager::run()
{
    impl_->run();
}
    
void iomanager::stop()
{
    impl_->stop();
}
    
} // namespace lio
