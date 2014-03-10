#ifndef LIO_SOCKET_HPP
#define LIO_SOCKET_HPP

#include "ltl/forward_declarations.hpp"

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
    
    void close();
    
    ltl::future<void> write(void const* data, std::size_t size);
    
    ltl::future<std::vector<uint8_t>> read(std::size_t size);
    
private:
    struct impl;
    std::unique_ptr<impl> impl_;
};
    
} // namespace lio

#endif // LIO_SOCKET_HPP
