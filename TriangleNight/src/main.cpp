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
        std::cerr << e.what() << '\n';
        exit(69);
    }
}
