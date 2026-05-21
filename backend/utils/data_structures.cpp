#include "data_structures.hpp"

namespace DataCollector {

u64 ProcessCPUData::getTotalTime() const
{
    return use_time + system_use_time;
}

u64 SystemCPUData::getTotalTime() const
{
    return getTotalSleepTime() + getTotalUseTime();
}

u64 SystemCPUData::getTotalUseTime() const
{
    return user + nice + system + irq + softirq + steal + guest + guest_nice;
}

u64 SystemCPUData::getTotalSleepTime() const
{
    return idle + iowait;
}
}
