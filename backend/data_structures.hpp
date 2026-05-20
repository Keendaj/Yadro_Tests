#pragma once
#include "types.hpp"

namespace DataCollector
{
    struct SystemCPUData
    {
        public:
            u64 user = 0;
            u64 nice = 0;
            u64 system = 0;
            u64 idle = 0;
            u64 iowait = 0;
            u64 irq = 0;
            u64 softirq = 0;
            u64 steal = 0;
            u64 guest = 0;
            u64 guest_nice = 0;

        u64 getTotalTime() const;
        u64 getTotalUseTime() const;
        u64 getTotalSleepTime() const;
    };

    #ifdef DATA_STRUCTURE_REALISATION
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
    #endif

    struct SystemRAMData
    {
        public:
            u64 total = 0;
            u64 available = 0;
            u64 buffer = 0;
            u64 shmem = 0;
            u64 cached = 0;
            u64 used = 0;

            u64 swap_total = 0;
            u64 swap_free = 0;
            u64 swap_used = 0;
    };

    struct ProcessCPUData
    {
        public:
            u64 id = 0;
            u64 use_time = 0;
            u64 system_use_time = 0;

        u64 getTotalTime() const;
    };

    #ifdef DATA_STRUCTURE_REALISATION
        u64 ProcessCPUData::getTotalTime() const
        {
            return use_time + system_use_time;
        }

    #endif

    struct ProcessData
    {
        public:
            u64 _use_time = 0;
            u64 _system_use_time = 0;
            u64 _total_time = 0;

            u64 id = 0;
            str user = "";
            i8 priority = 0;
            u64 virtual_memory_size = 0;
            u64 resident_set_size = 0;
            u64 shared_memory = 0;
            float cpu_load = 0;
            float mem_load = 0;
            PS state = PS::Sleeping;
            str command = ""; //Command with args which starts this process
    };
};
