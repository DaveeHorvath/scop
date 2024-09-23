#include "Logger.hpp"
#include <ctime>

std::ostream& operator<<(std::ostream& os, const Logger& log)
{
    if (log == Logger::reset)
    {
        os << "\033[0m";
        return os;    
    }
    std::time_t t = std::time(0);
    std::tm* now = std::localtime(&t);
    os << level[log] << "[" << now->tm_hour << ":" << now->tm_min << ":" << now->tm_sec << "] ";
    return os;
}