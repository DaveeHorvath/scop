#include <iostream>
#include "Logger.hpp"
int main()
{
    std::cout << "<===  LOGGER TESTING FACILITY  ===>\n";
    std::cout << Logger::info << "Hello there\n" << Logger::reset;
    std::cout << Logger::debug << "This is \n" << Logger::reset;
    std::cout << Logger::warn << "something evil\n" << Logger::reset;
    std::cout << Logger::error << "that i made!\n" << Logger::reset;

    std::cout << "and the stdout isnt ruined hopefully\n";
}