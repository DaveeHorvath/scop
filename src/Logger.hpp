#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <ostream>
enum loglevel {
    info, debug, warn, error, reset 
};

const std::string level[4] = {"\033[32m", "\033[36m", "\033[33m", "\033[1m\033[31m"};

using Logger = loglevel; 

std::ostream& operator<<(std::ostream& os, const Logger& log);

#endif