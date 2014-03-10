#ifndef LIO_SERVER_HPP
#define LIO_SERVER_HPP

#include "ltl/forward_declarations.hpp"

#include <memory>
#include <functional>

namespace lio {
    
class iomanager;
class socket;
    
class server
{
public:
    explicit server(std::shared_ptr<iomanager> const& x,
                    char const* ip, int port,
                    std::function<void(std::shared_ptr<socket> const&)> on_connection);
    ~server();
    server(server&& other);
    server& operator=(server&& other);
    
    void close();
    
private:
    struct impl;
    std::unique_ptr<impl> impl_;
};
    
} // namespace lio

#endif // LIO_SERVER_HPP
