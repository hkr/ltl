#include "lio/socket.hpp"

#include "ltl/future.hpp"
#include "lio/iomanager.hpp"

#include "uv.h"

namespace lio {

struct socket::impl
{
    explicit impl(std::shared_ptr<uv_tcp_s> const& tcp)
    : loop_(static_cast<iomanager*>(tcp->loop->data)->get_loop())
    , tcp_(tcp)
    {
        
    }
    
    ltl::future<void> write(void const* data, std::size_t size)
    {
        return ltl::make_future();
    }
    
    ltl::future<std::vector<uint8_t>> read(std::size_t size)
    {
        return ltl::make_future(std::vector<uint8_t>());
    }
    
    void close()
    {
        uv_close((uv_handle_t*) tcp_.get(), nullptr);
    }
    
    impl(impl const&) = delete;
    impl& operator=(impl const&) = delete;
    
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
