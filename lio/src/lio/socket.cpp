#include "lio/socket.hpp"

#include "ltl/future.hpp"
#include "ltl/task_queue.hpp"

#include "lio/iomanager.hpp"

#include <assert.h>

#include "uv.h"

namespace lio {
    
namespace {

template <class T>
struct request
{
    request() : data(), promise(), buf(), write_req() {}
    
    request(request const&) =delete;
    request& operator=(request const&) =delete;
    
    request(request&& other)
    : data(std::move(other.data))
    , promise(std::move(other.promise))
    , buf(std::move(other.buf))
    , write_req(std::move(other.write_req)){}
    
    request& operator=(request&& other)
    {
        request tmp(std::move(other));
        data.swap(tmp.data);
        promise.swap(tmp.promise);
        buf = tmp.buf;
        write_req = tmp.write_req;
        return *this;
    }
    
    std::vector<std::uint8_t> data;
    ltl::promise<T> promise;
    uv_buf_t buf;
    uv_write_t write_req;
};

typedef request<void> write_request;
typedef request<std::vector<std::uint8_t>> read_request;
    
} // namespace

struct socket::impl : std::enable_shared_from_this<impl>
{
    explicit impl(std::shared_ptr<uv_tcp_s> const& tcp)
    : read_queue_()
    , write_queue_()
    , loop_(static_cast<iomanager*>(tcp->loop->data)->get_loop())
    , tcp_(tcp)
    , manager_(static_cast<iomanager*>(tcp->loop->data)->shared_from_this())
    , closing_(false)
    {
        tcp_->data = this;
    }
   
    static uv_buf_t alloc_read(uv_handle_t* handle, size_t suggested_size)
    {
        return static_cast<impl*>(handle->data)->on_alloc_read(handle, suggested_size);
    }
    
    uv_buf_t on_alloc_read(uv_handle_t*, size_t s)
    {
        auto& req = read_queue_.front();

        auto const old_s = req.data.size();
        auto new_s = old_s + s;
        if (new_s > req.data.capacity())
            new_s = req.data.capacity();
        
        req.data.resize(new_s);
        
        if (req.buf.base == nullptr)
        {
            req.buf.base = reinterpret_cast<char*>(req.data.data());
            req.buf.len = req.data.size();
        }
        else
        {
            req.buf.base += req.buf.len;
            req.buf.len = new_s - old_s;
        }

        return req.buf;
    }
    
    static void read_cb(uv_stream_t* stream, ssize_t nread, uv_buf_t buf)
    {
        static_cast<impl*>(stream->data)->on_read(stream, nread, buf);
    }
    
    void on_read(uv_stream_t* stream, ssize_t nread, uv_buf_t buf)
    {
        auto& front = read_queue_.front();
        
        bool done = false;
        
        if (nread < 0)
        {
            // TODO: do we really want an exception on EOF etc.?
            struct socket_read_error : virtual std::exception
            {
                virtual const char* what() const noexcept
                {
                    return "socket_read_error";
                }
            };
            uv_read_stop(stream);
            front.promise.set_exception(std::make_exception_ptr(socket_read_error()));
            done = true;
        }
        else
        {
            auto const old_size = buf.base - reinterpret_cast<char*>(front.data.data());
            auto const total = old_size + nread;
            assert(total <= front.data.capacity());
            
            front.data.resize(total);
            
            done = total == front.data.capacity() || nread == 0;
            
            if (done)
            {
                if (nread != 0)
                    uv_read_stop(stream);
                
                front.promise.set_value(std::move(front.data));
            }
        }

        if (done)
            read_queue_.pop_front();
    }
    
    ltl::future<std::vector<uint8_t>> read(std::size_t size)
    {
        read_request req;
        req.data.reserve(size);
        
        uv_read_start((uv_stream_t*)tcp_.get(), alloc_read, read_cb);
        read_queue_.push_back(std::move(req));
        return read_queue_.back().promise.get_future();
    }
    
    ltl::future<void> write(std::vector<uint8_t> data)
    {
        write_request req;
        req.data = std::move(data);
        
        req.buf.len = req.data.size();
        req.buf.base = reinterpret_cast<char*>(req.data.data());
        
        write_queue_.push_back(std::move(req));
        auto& r = write_queue_.back();
        r.write_req.data = this;
        uv_write(&r.write_req, (uv_stream_t*)tcp_.get(), &r.buf, 1, write_cb);
        
        return r.promise.get_future();
    }
    
    static void write_cb(uv_write_t* req, int status)
    {
        static_cast<impl*>(req->data)->on_write(req, status);
    }
    
    void on_write(uv_write_t* req, int status)
    {
        auto& front = write_queue_.front();
        if (status)
            front.promise.set_exception(std::make_exception_ptr(std::exception())); // TODO
        else
            front.promise.set_value();
        
        write_queue_.pop_front();
    }
    
    ltl::future<void> close()
    {
        assert(!closing_);
        closing_ = true;
        uv_close((uv_handle_t*) tcp_.get(), close_cb);
        keep_alive_ = shared_from_this();
        return closing_promise_.get_future();
    }
    
    static void close_cb(uv_handle_t* handle)
    {
        static_cast<impl*>(handle->data)->on_close();
    }
    
    void on_close()
    {
        tcp_.reset();
        closing_promise_.set_value();
        keep_alive_.reset();
    }
    
    impl(impl const&) = delete;
    impl& operator=(impl const&) = delete;
    
    std::deque<read_request> read_queue_;
    std::deque<write_request> write_queue_;
    std::shared_ptr<uv_loop_t> loop_;
    std::shared_ptr<uv_tcp_t> tcp_;
    std::shared_ptr<iomanager> manager_;
    
    ltl::promise<void> closing_promise_;
    std::shared_ptr<impl> keep_alive_;
    bool closing_;
};

socket::socket(std::shared_ptr<uv_tcp_s> const& tcp)
: impl_(std::make_shared<impl>(tcp))
{
    
}
    
socket::~socket()
{
    close();
}

ltl::future<void> socket::write(std::vector<uint8_t> const& data)
{
    auto i = impl_;
    return impl_->manager_->execute([=](){ return i->write(data); }).unwrap();
}
    
ltl::future<std::vector<uint8_t>> socket::read(std::size_t size)
{
    auto i = impl_;
    return impl_->manager_->execute([=](){ return i->read(size); }).unwrap();
}
    
ltl::future<void> socket::close()
{
    auto i = impl_;
    return impl_->manager_->execute([=](){ return i->close(); }).unwrap();
}
        
} // namespace lio
