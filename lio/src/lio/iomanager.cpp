#include "lio/iomanager.hpp"

#include "lio/socket.hpp"

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
    
typedef std::unique_ptr<uv_loop_t, delete_loop> unique_loop_ptr;
    
unique_loop_ptr make_loop()
{
    auto l = uv_loop_new();
    //uv_loop_init(l);
    return unique_loop_ptr(l);
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
        
    }
    
    ~impl()
    {
    }
    
    void listen(char const* ip, int port, std::function<void(socket)> on_connection)
    {
        
    }
    
    ltl::future<socket> connect(char const* ip, int port)
    {
        return ltl::make_future(socket());
    }
    
private:
    iomanager* back_;
    unique_loop_ptr loop_;
};

void iomanager::listen(char const* ip, int port, std::function<void(socket)> on_connection)
{
    impl_->listen(ip, port, std::move(on_connection));
}
    
ltl::future<socket> iomanager::connect(char const* ip, int port)
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
    
} // namespace lio
