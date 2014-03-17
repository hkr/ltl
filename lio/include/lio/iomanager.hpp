#ifndef LIO_IOMANAGER_HPP
#define LIO_IOMANAGER_HPP

#include "ltl/future.hpp"

#include <memory>
#include <functional>
#include <string>

struct uv_loop_s;

namespace lio {
    
class socket;
class server;
    
class iomanager : public std::enable_shared_from_this<iomanager>
{
public:
    static std::shared_ptr<iomanager> create(char const* name = nullptr);
    
    ltl::future<std::shared_ptr<server>> create_server(std::string const& ip, int port,
                                                       std::function<void(std::shared_ptr<socket> const&)> on_connection);
    
    ltl::future<std::shared_ptr<socket>> connect(std::string const& ip, int port);
    
    void run();
    void stop();
    
public:
    std::shared_ptr<uv_loop_s> get_loop();
    void exec(std::function<void()> task);
    
    template <typename R>
    ltl::future<R> execute_impl(std::function<R()> task, std::false_type)
    {
        auto promise = std::make_shared<ltl::promise<R>>();
        exec([=](){
            promise->set_value(task());
        });
        return promise->get_future();
    }
    
    template <typename R>
    ltl::future<R> execute_impl(std::function<R()> task, std::true_type)
    {
        auto promise = std::make_shared<ltl::promise<R>>();
        exec([=](){
            task();
            promise->set_value();
        });
        return promise->get_future();
    }
    
    template <typename Function>
    ltl::future<typename std::result_of<Function()>::type>
        execute(Function&& task)
    {
        typedef typename std::result_of<Function()>::type result_type;
        return execute_impl<result_type>(std::forward<Function>(task), std::is_void<result_type>());
    }
    

    
private:
    explicit iomanager(char const* name);
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
