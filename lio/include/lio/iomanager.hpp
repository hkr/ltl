#ifndef LIO_IOMANAGER_HPP
#define LIO_IOMANAGER_HPP

#include "ltl/forward_declarations.hpp"

#include <memory>
#include <functional>

namespace lio {
    
class socket;
    
class iomanager : public std::enable_shared_from_this<iomanager>
{
public:
    static std::shared_ptr<iomanager> create();
    
    void listen(char const* ip, int port, std::function<void(socket)> on_connection);
    
    ltl::future<socket> connect(char const* ip, int port);
    
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
