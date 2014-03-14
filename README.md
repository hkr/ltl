simple tasks library
====================


Overview
--------

Simple tasks library tries to implement C# `await`, also known as *resumable functions* ([N3650](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3650.pdf)).

The library is in very very alpha state.

Because C++11 `std::future` is not composable and does not provide `std::future::then` as proposed in [N3634](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3634.pdf), this library provides its own implementations. 
These are very similar to the C++11 or existing proposals. 


Platform support
----------------

The current implementation depends on many C++11 features. 
It compiles with the Xcode 5 version of Clang, GCC 4.8 and Visual C++ 2013.
The implementation of `await` uses multiple *contexts*/*stacks*. This is implemented using Windows Fibers, *ucontext* and *Boost.Context*.
It should therefore work on many current platforms, but only x64 Mac OS X, Win32, Win64 and x64 Linux have been tested.


Dependencies
------------

Optionally *Boost.Context* and *CXX11* (a C++11 compiler detection module for *CMake*). 
Both are included as *git submodules*, no complete installation of *boost* required.


Usage example
-------------

    using namespace ltl;
    
    task_queue queue1; // task queues with a single worker thread each.
    task_queue queue2;
    
    std::string get_string(int idx) {
        std::stringstream ss;
        ss << "get_string: " << idx;
        return ss.str();
    }
    
    void print(std::string const& str) {
        printf("print %s\n", str.c_str());
    }
    
    void run() {
        std::vector<future<void>> fdone;
    
        for (int i = 0; i < 10; ++i) {
            future<std::string> fs = queue1.push_back_resumable([=](){ 
                return get_string(idx); 
            });
        
            std::string const str = await(fs);
        
            future<void> fprinted = queue2.push_back_resumable([=]() {
                print(str);
            }
        
            fdone.push_back(fprinted);
        }
        
        future<void> f_all_done = when_all_ready(std::begin(fdone), std::end(fdone));   
        await(f_all_done);
    }
    
    int main(int argc, char** argv) {
        async(run).wait();
        return 0;
    }
    
    
    
    










