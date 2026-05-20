#pragma once
#include "types.hpp"
#include "data_structures.hpp"
#include <unordered_map>

#ifdef DATA_COLLECTOR_REALISATION
    #include <fstream>
    #include <chrono>
    #include <iomanip>
    #include <sstream>
    #include <unistd.h>
#endif

namespace DataCollector
{
    class DataCollector
    {
        private:
            SystemCPUData _previous_system_cpu_tick;
            std::unordered_map<u64, ProcessCPUData> _previous_processes_cpu_tick;

        protected:
            SystemCPUData collectCPU();

            SystemRAMData collectRAM();

            std::unordered_map<u64, ProcessData> collectProcesses();
        public:
            
            bool CollectData();
    };

    #ifdef DATA_COLLECTOR_REALISATION
        str get_username_by_uid(u64 uid) 
        {
            struct passwd *pw = getpwuid(static_cast<uid_t>(uid));
            
            if (pw) {
                return str(pw->pw_name);
            }

            return std::to_string(uid);
        }

        SystemCPUData DataCollector::collectCPU()
        {
            SystemCPUData cpu_result;
            std::ifstream cpu_file("/proc/stat");

            if (!cpu_file.is_open()) {
                auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

                std::cerr << "[" << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S") 
                          << "] [Error]: Cannot open /proc/stat" << std::endl;
                return cpu_result;
            }

            str name;
            cpu_file >> name;
            
            cpu_file >> cpu_result.user 
                    >> cpu_result.nice 
                    >> cpu_result.system 
                    >> cpu_result.idle 
                    >> cpu_result.iowait 
                    >> cpu_result.irq 
                    >> cpu_result.softirq 
                    >> cpu_result.steal;

            return cpu_result;
        }

        SystemRAMData collectRAM()
        {
            SystemRAMData ram_result;
            std::ifstream ram_file("/proc/meminfo");

            if (!ram_file.is_open()) {
                auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

                std::cerr << "[" << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S") 
                          << "] [Error]: Cannot open /proc/meminfo" << std::endl;
                return ram_result;
            }
            str name;
            u64 value;
            str t;

            while(ram_file >> name >> value)
            {
                std::getline(ram_file, t);

                if (name == "MemTotal:") 
                {
                    ram_result.total = value;
                } 
                else if (name == "MemAvailable:")
                {
                    ram_result.available = value;
                } 
                else if (name == "Buffers:")
                {
                    ram_result.buffer = value;
                } 
                else if (name == "Cached:") 
                {
                    ram_result.cached = value;
                } 
                else if (name == "SwapTotal:") 
                {
                    ram_result.swap_total = value;
                } 
                else if (name == "SwapFree:")
                {
                    ram_result.swap_free = value;
                } 
                else if (name == "Shmem:")
                {
                    ram_result.shmem = value;
                }
            }

            if (ram_result.total >= ram_result.available) {
                ram_result.used = ram_result.total - ram_result.available;
            }
            
            if (ram_result.swap_total >= ram_result.swap_free) {
                ram_result.swap_used = ram_result.swap_total - ram_result.swap_free;
            }

            return ram_result;
        }

        std::unordered_map<u64, ProcessData> collectProcesses()
        {
            std::unordered_map<u64, ProcessData> processes_result;

            static const long page_size_kb = sysconf(_SC_PAGESIZE) / 1024;
            std::error_code ec;
            for (const auto& entry : fs::directory_iterator("/proc", ec)) {
                if (!entry.is_directory(ec))
                {
                    continue;
                }

                str dir_name = entry.path().filename().string();
                if (!std::all_of(dir_name.begin(), dir_name.end(), ::isdigit)) {
                    continue;
                }

                ProcessData process_result;
                process_result.id = std::stoull(dir_name);
                str base_path = entry.path().string();

                std::ifstream status_file(base_path + "/status");
                if (!status_file.is_open()) {
                    continue;
                    
                } 

                str key, value;
                while (status_file >> key) {
                    if (key == "Uid:") {
                        u64 uid;
                        status_file >> uid;
                        process_result.user = get_username_by_uid(uid);
                        break;
                    }
                    std::getline(status_file, value);
                }

                std::ifstream statm_file(base_path + "/statm");
                if (statm_file.is_open()) {
                    u64 size_pages, resident_pages, shared_pages;
                    if (statm_file >> size_pages >> resident_pages >> shared_pages) {
                        process_result.virtual_memory_size = size_pages * page_size_kb;
                        process_result.resident_set_size = resident_pages * page_size_kb;
                        process_result.shared_memory = shared_pages * page_size_kb;
                    }
                }

                std::ifstream stat_file(base_path + "/stat");
                if (stat_file.is_open()) {
                    str line;
                    if (std::getline(stat_file, line)) {
                        size_t rparen_pos = line.find_last_of(')');
                        if (rparen_pos != str::npos && rparen_pos + 2 < line.size()) {
                            str remaining = line.substr(rparen_pos + 2);
                            std::istringstream iss(remaining);
                            
                            char state_char;
                            iss >> state_char;

                            switch (state_char) {
                                case 'R': process_result.state = PS::Running; break;
                                case 'S': process_result.state = PS::Sleeping; break;
                                case 'D': process_result.state = PS::WaitingDisk; break;
                                case 'Z': process_result.state = PS::Zombie; break;
                                case 'T': process_result.state = PS::Stopped; break;
                                default:  process_result.state = PS::Sleeping; break;
                            }

                            str t;
                            for (int i = 0; i < 10; ++i) {
                                iss >> t;
                            } 

                            iss >> process_result._use_time >> process_result._system_use_time;

                            iss >> t >> t;

                            int priority_val;
                            iss >> priority_val;
                            process_result.priority = static_cast<i8>(priority_val);
                            
                            process_result._total_time = process_result._use_time + process_result._system_use_time;
                        }
                    }
                }
                std::ifstream cmd_file(base_path + "/cmdline");
                if (cmd_file.is_open()) {
                    std::getline(cmd_file, process_result.command);
                    if (!process_result.command.empty()) {
                        for (char& c : process_result.command) {
                            if (c == '\0') {
                                c = ' ';
                            }
                        }
                        if (process_result.command.back() == ' ') {
                            process_result.command.pop_back();
                        }
                    }
                }
                processes_result[process_result.id] = process_result;
            }

            return processes_result;
        }

        bool DataCollector::CollectData()
        {
            SystemCPUData current_cpu = collectCPU();
            SystemRAMData current_ram = collectRAM();
            const u64 current_total_cpu_time = current_cpu.getTotalTime();
            const u64 previous_total_cpu_time = _previous_system_cpu_tick.getTotalTime();
            const u64 total_delta_time = current_total_cpu_time - previous_total_cpu_time;
            if (total_delta_time == 0)
            {
                total_delta_time = 1;
            }

            auto current_processes = collectProcesses();
            static const int num_cores = sysconf(_SC_NPROCESSORS_ONLN);

            for (auto& [pid, process] : current_processes) 
            {
                if (current_ram.total > 0) {
                    process.mem_load = (static_cast<float>(process.resident_set_size) / current_ram.total) * 100.0f;
                }

                auto it = previous_processes_cpu_tick.find(pid);
                if (it != previous_processes_cpu_tick.end()) 
                {
                    const u64 proc_time_delta = process._total_time - it->second.getTotalTime();

                    process.cpu_load = (static_cast<float>(proc_time_delta) / total_delta) * 100.0f * num_cores;
                }
                else 
                {
                    process.cpu_load = 0.0f; 
                }
            }
            _previous_system_cpu_tick = current_cpu;

            _previous_processes_cpu_tick.clear();
            for (const auto& [pid, process] : current_processes) {
                ProcessCPUData cpu_data;
                cpu_data.id = process.id;
                cpu_data.use_time = process._use_time;
                cpu_data.system_use_time = process._system_use_time;
                _previous_processes_cpu_tick[pid] = cpu_data;
            }

            return true;
        }
    #endif

};