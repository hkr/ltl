#include "lio/iomanager.hpp"

#include <iostream>

int main(int argc, char** argv)
{
    auto mgr = lio::iomanager::create();
    
    std::cout << "test" << std::endl;
    
	return 0;
}
