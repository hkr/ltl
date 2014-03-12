#include "lio/socket.hpp"

#include "ltl/future.hpp"
#include "lio/iomanager.hpp"

#include "uv.h"

namespace lio {

struct socket::impl
{
    explicit impl(std::shared_ptr<uv_tcp_s> const& tcp)
    : in_progress_(false)
    , read_promise_()
    , read_buf_()
    , loop_(static_cast<iomanager*>(tcp->loop->data)->get_loop())
    , tcp_(tcp)
    {
        tcp_->data = this;
    }
    
    static uv_buf_t alloc_read(uv_handle_t* handle, size_t suggested_size)
    {
        return static_cast<impl*>(handle->data)->on_alloc_read(handle, suggested_size);
    }
    
    uv_buf_t on_alloc_read(uv_handle_t* handle, size_t suggested_size)
    {
        uv_buf_t buf;
        buf.len = read_buf_.size();
        buf.base = reinterpret_cast<char*>(read_buf_.data());
        return buf;
    }
    
    static void read_cb(uv_stream_t* stream, ssize_t nread, uv_buf_t buf)
    {
        static_cast<impl*>(stream->data)->on_read(stream, nread, buf);
    }
    
    static void write_cb(uv_write_t* req, int status)
    {
        
    }
    
    void on_read(uv_stream_t*, ssize_t nread, uv_buf_t)
    {
        in_progress_ = false;
        read_promise_.set_value(std::move(read_buf_));
    }
    
    ltl::future<std::vector<uint8_t>> read(std::size_t size)
    {
        if (in_progress_)
            return ltl::future<std::vector<uint8_t>>();
        
        in_progress_ = true;
        
        ltl::promise<std::vector<std::uint8_t>> new_read_promise;
        std::vector<std::uint8_t> new_read_buf(size);
        read_promise_.swap(new_read_promise);
        read_buf_.swap(new_read_buf);
        
        uv_read_start((uv_stream_t*)tcp_.get(), alloc_read, read_cb);
        
        return read_promise_.get_future();
    }
    
    ltl::future<void> write(void const* data, std::size_t size)
    {
        
        return ltl::make_ready_future();
    }
    
    void close()
    {
        uv_close((uv_handle_t*) tcp_.get(), nullptr);
    }
    
    impl(impl const&) = delete;
    impl& operator=(impl const&) = delete;
    
    bool in_progress_;
    ltl::promise<std::vector<std::uint8_t>> read_promise_;
    std::vector<std::uint8_t> read_buf_;
    std::shared_ptr<uv_loop_t> loop_;
    std::shared_ptr<uv_tcp_t> tcp_;
};

socket::socket(std::shared_ptr<uv_tcp_s> const& tcp)
: impl_(new impl(tcp))
{
    
}
    
socket::~socket()
{
    
}

ltl::future<void> socket::write(void const* data, std::size_t size)
{
    return impl_->write(data, size);
}
    
ltl::future<std::vector<uint8_t>> socket::read(std::size_t size)
{
    return impl_->read(size);
}
    
void socket::close()
{
    return impl_->close();
}
    
} // namespace lio
