#include "app.hpp"
#include <iostream>

int main()
{
    App app;

    try
    {
        std::cout << "\033[2J";
        app.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << "\033[1;31" << e.what() << "\033[0m" << '\n';
        exit(69);
    }
}
