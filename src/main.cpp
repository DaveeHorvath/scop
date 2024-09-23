#include "app.hpp"
#include <iostream>

int main()
{
    App app;

    try
    {
        app.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << "\033[1;31" << e.what() << "\033[0m" << '\n';
        exit(69);
    }
}
