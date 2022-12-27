#include "host.hpp"
#include <iostream>
#include <stdexcept>

int main()
{
    std::cout << "Starting game" << std::endl << std::endl;
    Host host;
    try
    {
        host.startGame();
    }
    catch (const std::runtime_error &err)
    {
        std::cout << "ERROR: " << err.what() << std::endl;
        return 1;
    }
    std::cout << "Exiting game" << std::endl;
    return 0;
}