#ifndef LIO_IOMANAGER_HPP
#define LIO_IOMANAGER_HPP

#include "ltl/forward_declarations.hpp"

#include <memory>
#include <functional>

struct uv_loop_s;

namespace lio {
    
class socket;
class server;
    
class iomanager : public std::enable_shared_from_this<iomanager>
{
public:
    static std::shared_ptr<iomanager> create();
    
    server create_server(char const* ip, int port, std::function<void(std::shared_ptr<socket> const&)> on_connection);
    
    ltl::future<std::shared_ptr<socket>> connect(char const* ip, int port);
    
    void run();
    void stop();

public:
    std::shared_ptr<uv_loop_s> get_loop();
    
private:
    iomanager();
    ~iomanager();
    iomanager(iomanager const&) = delete;
    iomanager& operator=(iomanager const&) = delete;
    static void destroy(iomanager*);
    
private:
    struct impl;
    std::unique_ptr<impl> impl_;
};
    
} // namespace lio

#endif // LIO_IOMANAGER_HPP
