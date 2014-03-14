#include "lio/iomanager.hpp"

#include "lio/socket.hpp"
#include "lio/server.hpp"

#include "ltl/promise.hpp"
#include "ltl/future.hpp"
#include "ltl/task_queue.hpp"

#include "uv.h"

#include <stdio.h>

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
    
    
    explicit impl(iomanager* ptr, char const* name)
    : back_(ptr)
    , loop_()
    , name_(name ? name : "unnamed")
    , task_queue_(name_)
    {
        loop_ = task_queue_.push_back([&]() { return make_loop(); }).get();
        loop_->data = back_;
    }
    
    ~impl()
    {
    }
    
    struct connection_request
    {
        connection_request(connection_request const&) =delete;
        connection_request& operator=(connection_request const&) =delete;
        
        explicit connection_request(uv_loop_t* loop)
        : prm()
        , conn()
        , sock(std::make_shared<uv_tcp_t>())
        {
            uv_tcp_init(loop, sock.get());
            conn.data = this;
        }
        
        ltl::promise<std::shared_ptr<socket>> prm;
        uv_connect_t conn;
        std::shared_ptr<uv_tcp_t> sock;
    };
    
    static void connection_established(uv_connect_t* conn, int status)
    {
        printf("connection_established\n");
        auto req = static_cast<connection_request*>(conn->data);
        req->prm.set_value(std::make_shared<socket>(req->sock));
        delete req;
    }
    
    std::shared_ptr<uv_loop_s> get_loop()
    {
        return loop_;
    }
    
    void run()
    {
        printf("Entered run %s\n", name_);
        uv_ref((uv_handle_t*)loop_.get());
        uv_run(loop_.get(), UV_RUN_DEFAULT);
        printf("Left run %s\n", name_);
    }
    
    void stop()
    {
        uv_stop(loop_.get());
        uv_unref((uv_handle_t*)loop_.get());
    }
    
    ltl::future<std::shared_ptr<socket>> connect(char const* ip, int port)
    {
        auto req = new connection_request(loop_.get());
        printf("uv_tcp_connect1\n");
        uv_tcp_connect(&req->conn, req->sock.get(), uv_ip4_addr(ip, port), connection_established);
        printf("uv_tcp_connect2\n");
        return req->prm.get_future();
    }
    
    std::shared_ptr<server> create_server(char const* ip, int port, std::function<void(std::shared_ptr<socket> const&)> on_connection)
    {
        return std::make_shared<server>(back_->shared_from_this(), ip, port, std::move(on_connection));
    }
    
    iomanager* const back_;
    std::shared_ptr<uv_loop_s> loop_;
    char const* name_;
    ltl::task_queue task_queue_;
};

ltl::future<std::shared_ptr<server>> iomanager::create_server(char const* ip, int port, std::function<void(std::shared_ptr<socket> const&)> on_connection)
{
    return impl_->task_queue_.push_back([=](){ return impl_->create_server(ip, port, std::move(on_connection)); });
}
    
ltl::future<std::shared_ptr<socket>> iomanager::connect(char const* ip, int port)
{
    return impl_->task_queue_.push_back([=](){ return impl_->connect(ip, port); }).unwrap();
}

iomanager::iomanager(char const* name)
: impl_(new impl(this, name))
{
    
}
    
iomanager::~iomanager()
{
    
}

void iomanager::destroy(iomanager* ptr)
{
    delete ptr;
}
    
std::shared_ptr<iomanager> iomanager::create(char const* name)
{
    return std::shared_ptr<iomanager>(new iomanager(name), &destroy);
}
    
std::shared_ptr<uv_loop_s> iomanager::get_loop()
{
    return impl_->get_loop();
}
    
ltl::future<void> iomanager::run()
{
    return impl_->task_queue_.push_back([=](){ impl_->run(); });
}
    
ltl::future<void> iomanager::stop()
{
    return impl_->task_queue_.push_back([=](){ impl_->stop(); });
}
    
ltl::task_queue& iomanager::get_queue()
{
    return impl_->task_queue_;
}
    
} // namespace lio
