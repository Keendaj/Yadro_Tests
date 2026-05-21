#include "logger.hpp"
#include <chrono>
#include <ctime>

namespace DataCollector::Utils {

void log(const str& component, const str& level, const str& msg)
{
    const auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);

    std::tm tm_struct;
    localtime_r(&time, &tm_struct);

    char time_buf[32];
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", &tm_struct);

    auto& stream = (level == "ERROR") ? std::cerr : std::cout;
    stream << "[" << time_buf << "][" << component << "] " << level << ": " << msg << "\n";
}
}