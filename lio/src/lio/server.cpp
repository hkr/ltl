#include "lio/server.hpp"

#include "ltl/future.hpp"
#include "lio/iomanager.hpp"
#include "lio/socket.hpp"

#include <assert.h>

#include "uv.h"

namespace lio {
    
struct server::impl
{
    explicit impl(std::shared_ptr<iomanager> const& x,
                  std::string const& ip, int port,
                  std::function<void(std::shared_ptr<socket> const&)> on_connection)
    : on_connection_(std::move(on_connection))
    , server_()
    , loop_(x->get_loop())
    , manager_(x)
    , closing_(false)
    {
        uv_tcp_init(loop_.get(), &server_);
        server_.data = this;
        auto bind_addr = uv_ip4_addr(ip.c_str(), port);
        uv_tcp_bind(&server_, bind_addr);
        int r = uv_listen((uv_stream_t*) &server_, 128, &connection);
        // TODO
    }
    
    static void connection(uv_stream_t* server, int status)
    {
        static_cast<impl*>(server->data)->connection_established(status);
    }
    
    void connection_established(int status)
    {
        if (status)
        {
            on_connection_(std::shared_ptr<socket>()); // TODO
            return;
        }
        
        auto client = std::make_shared<uv_tcp_t>();
        uv_tcp_init(loop_.get(), client.get());
        
        if (uv_accept((uv_stream_t*) &server_, (uv_stream_t*) client.get()) == 0)
        {
            on_connection_(std::make_shared<socket>(client));
        }
        else
        {
            on_connection_(std::shared_ptr<socket>());
            uv_close((uv_handle_t*) client.get(), nullptr);
        }
    }
    
    ~impl()
    {
    }
    
    impl(impl const&) =delete;
    impl& operator=(impl const&) =delete;
    
    ltl::future<void> close()
    {
        if (closing_)
            return ltl::make_ready_future();
        
        closing_ = true;
        uv_close((uv_handle_t*) &server_, close_cb);
        return closing_promise_.get_future();
    }
    
    static void close_cb(uv_handle_t* handle)
    {
        static_cast<impl*>(handle->data)->on_close();
    }
    
    void on_close()
    {
        closing_promise_.set_value();
    }
    
    std::function<void(std::shared_ptr<socket> const&)> on_connection_;
    uv_tcp_t server_;
    std::shared_ptr<uv_loop_s> loop_;
    std::shared_ptr<iomanager> manager_;
    ltl::promise<void> closing_promise_;
    bool closing_;
    
};
    
server::server(std::shared_ptr<iomanager> const& x,
               std::string const& ip, int port,
               std::function<void(std::shared_ptr<socket> const&)> on_connection)
: impl_(std::make_shared<impl>(x, ip, port, std::move(on_connection)))
{
    
}
    
server::~server()
{
    try {
        close();
    } catch(...) {
        
    }
}
    
server::server(server&& other)
: impl_(std::move(other.impl_))
{
    
}
    
server& server::operator=(server&& other)
{
    impl_ = std::move(other.impl_);
    return *this;
}
    
ltl::future<void> server::close()
{
    auto i = impl_;
    return impl_->manager_->execute([=](){ return i->close(); }).unwrap();
}
    
} // namespace lio
