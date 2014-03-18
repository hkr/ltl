#include "lio/iomanager.hpp"

#include "lio/socket.hpp"
#include "lio/server.hpp"

#include "ltl/promise.hpp"
#include "ltl/future.hpp"

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
    , async_signal_task_()
    , thread_()
    {
        loop_ = make_loop();
        loop_->data = back_;
        uv_async_init(loop_.get(), &async_signal_task_, async_new_task_cb);
        async_signal_task_.data = this;
    }
    
    static void async_new_task_cb(uv_async_t* handle, int)
    {
        static_cast<impl*>(handle->data)->on_async_new_task();
    }
    
    void on_async_new_task()
    {
        while (true)
        {
            std::deque<std::function<void()>> tasks;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                tasks.swap(task_queue_);
            }
            
            if (tasks.empty())
                return;
            
            while (!tasks.empty())
            {
                tasks.front()();
                tasks.pop_front();
            }
        }
    }
    
    ~impl()
    {
        if (thread_.joinable())
            thread_.join();
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
        if (thread_.joinable())
            return;
        
        thread_  = std::thread([&](){
            printf("Entered run %s\n", name_);
            uv_run(loop_.get(), UV_RUN_DEFAULT);
            printf("Left run %s\n", name_);
        });
        
    }
    
    void stop()
    {
        uv_stop(loop_.get());
        uv_unref((uv_handle_t*)loop_.get());
    }
    
    ltl::future<std::shared_ptr<socket>> connect(std::string const& ip, int port)
    {
        auto req = new connection_request(loop_.get());
        uv_tcp_connect(&req->conn, req->sock.get(), uv_ip4_addr(ip.c_str(), port), connection_established);
        return req->prm.get_future();
    }
    
    std::shared_ptr<server> create_server(std::string const& ip, int port, std::function<void(std::shared_ptr<socket> const&)> on_connection)
    {
        return std::make_shared<server>(back_->shared_from_this(), ip, port, std::move(on_connection));
    }
    
    void execute(std::function<void()> task)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        task_queue_.push_back(std::move(task));
        uv_async_send(&async_signal_task_);
    }
    
    iomanager* const back_;
    std::shared_ptr<uv_loop_s> loop_;
    char const* name_;
    
    std::mutex mutex_;
    std::deque<std::function<void()>> task_queue_;
    uv_async_t async_signal_task_;
    std::thread thread_;
    
};

ltl::future<std::shared_ptr<server>> iomanager::create_server(std::string const& ip, int port,
                                                              std::function<void(std::shared_ptr<socket> const&)> const& on_connection)
{
    return execute([=](){
        return impl_->create_server(ip, port, on_connection);
    });
}
    
ltl::future<std::shared_ptr<socket>> iomanager::connect(std::string const& ip, int port)
{
    auto promise = std::make_shared<ltl::promise<ltl::future<std::shared_ptr<socket>>>>();
    execute([=](){
        promise->set_value(impl_->connect(ip, port));
    });
    return promise->get_future().unwrap();
}

void iomanager::run()
{
    return impl_->run();
}

void iomanager::stop()
{
    if (!impl_->thread_.joinable())
        return;
    
    auto i = impl_;
    auto promise = std::make_shared<ltl::promise<void>>();
    execute([=](){
        i->stop();
        promise->set_value();
    });
    promise->get_future().wait();
    impl_->thread_.join();
}
    
iomanager::iomanager(char const* name)
: impl_(std::make_shared<impl>(this, name))
{
    
}
    
iomanager::~iomanager()
{
    try {
        stop();
    } catch(...) {
        
    }
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
    
void iomanager::exec(std::function<void()> task)
{
    impl_->execute(std::move(task));
}
    
} // namespace lio
