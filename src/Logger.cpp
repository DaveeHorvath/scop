#include "Logger.hpp"
#include <ctime>

std::ostream& operator<<(std::ostream& os, const Logger& log)
{
    if (log == Logger::reset)
    {
        os << "\033[0m\n";
        return os;    
    }
    std::time_t t = std::time(0);
    std::tm* now = std::localtime(&t);
    os << level[log] << "[" << now->tm_hour << ":" << (now->tm_min < 10 ? "0" : "") <<now->tm_min << ":" << (now->tm_sec < 10 ? "0" : "") << now->tm_sec << "] ";
    return os;
}