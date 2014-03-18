#ifndef LIO_SOCKET_HPP
#define LIO_SOCKET_HPP

#include "ltl/future.hpp"

#include <memory>
#include <stdint.h>
#include <vector>

struct uv_tcp_s;

namespace lio {
    
class iomanager;
    
class socket
{
public:
    explicit socket(std::shared_ptr<uv_tcp_s> const& tcp);
    ~socket();
    socket(socket const&) =delete;
    socket& operator=(socket const&) =delete;
    
    ltl::future<void> close();
    
    ltl::future<void> write(std::vector<uint8_t> const& data);
    ltl::future<void> write(std::vector<uint8_t>&& data);
    
    template <typename Byte>
    ltl::future<void> write(Byte const* data, size_t size)
    {
        static_assert(sizeof(Byte) == 1, "size mismatch");
        auto d = reinterpret_cast<uint8_t const*>(data);
        return write(std::vector<uint8_t>(d, d + size));
    }
    
    ltl::future<std::vector<uint8_t>> read(std::size_t size);
    
private:
    struct impl;
    std::shared_ptr<impl> impl_;
};
    
} // namespace lio

#endif // LIO_SOCKET_HPP
