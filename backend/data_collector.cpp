#include "data_collector.hpp"
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace DataCollector {

static str get_username_by_uid(u64 uid)
{
    static std::unordered_map<u64, str> uid_cache;

    auto it = uid_cache.find(uid);
    if (it != uid_cache.end()) {
        return it->second;
    }

    struct passwd* pw = getpwuid(static_cast<uid_t>(uid));
    str username = pw ? str(pw->pw_name) : std::to_string(uid);

    uid_cache[uid] = username;
    return username;
}

std::unordered_map<u64, SystemCPUData> DataCollector::collectCPU()
{
    std::unordered_map<u64, SystemCPUData> cpu_result;
    std::ifstream cpu_file("/proc/stat");

    if (!cpu_file.is_open()) {
        Utils::log("DataCollector", "ERROR", "Cannot open /proc/stat");
        return cpu_result;
    }

    /*
        0 - total system cpu data
        1..N - core cpu data
    */
    str line;
    while (std::getline(cpu_file, line)) {
        if (line.compare(0, 3, "cpu") != 0) {
            break;
        }

        std::istringstream iss(line);
        str name;
        iss >> name;

        SystemCPUData data = {};

        iss >> data.user >> data.nice >> data.system >> data.idle >> data.iowait >> data.irq
            >> data.softirq >> data.steal >> data.guest >> data.guest_nice;

        u64 cpu_index = 0;

        if (name == "cpu") {
            cpu_index = 0;
        } else {
            try {
                u64 core_id = std::stoull(name.substr(3));
                cpu_index = core_id + 1;
            } catch (...) {
                continue;
            }
        }

        cpu_result[cpu_index] = data;
    }

    return cpu_result;
}

SystemRAMData DataCollector::collectRAM()
{
    SystemRAMData ram_result;
    std::ifstream ram_file("/proc/meminfo");

    if (!ram_file.is_open()) {
        Utils::log("DataCollector", "ERROR", "Cannot open /proc/meminfo");
        return ram_result;
    }
    str name;
    u64 value;
    str t;

    while (ram_file >> name >> value) {
        std::getline(ram_file, t);

        if (name == "MemTotal:") {
            ram_result.total = value;
        } else if (name == "MemAvailable:") {
            ram_result.available = value;
        } else if (name == "Buffers:") {
            ram_result.buffer = value;
        } else if (name == "Cached:") {
            ram_result.cached = value;
        } else if (name == "SwapTotal:") {
            ram_result.swap_total = value;
        } else if (name == "SwapFree:") {
            ram_result.swap_free = value;
        } else if (name == "Shmem:") {
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

void DataCollector::parseProcessStatus(const fs::path& dir_path, ProcessData& process_result)
{
    std::ifstream status_file(dir_path / "status");
    if (!status_file.is_open())
        return;

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
}

void DataCollector::parseProcessStatm(
    const fs::path& dir_path, ProcessData& process_result, long page_size_kb)
{
    std::ifstream statm_file(dir_path / "statm");
    if (!statm_file.is_open())
        return;

    u64 size_pages, resident_pages, shared_pages;
    if (statm_file >> size_pages >> resident_pages >> shared_pages) {
        process_result.virtual_memory_size = size_pages * page_size_kb;
        process_result.resident_set_size = resident_pages * page_size_kb;
        process_result.shared_memory = shared_pages * page_size_kb;
    }
}

void DataCollector::parseProcessStat(const fs::path& dir_path, ProcessData& process_result)
{
    std::ifstream stat_file(dir_path / "stat");
    if (!stat_file.is_open())
        return;

    str line;
    if (std::getline(stat_file, line)) {
        size_t rparen_pos = line.find_last_of(')');
        if (rparen_pos != str::npos && rparen_pos + 2 < line.size()) {
            str remaining = line.substr(rparen_pos + 2);
            std::istringstream iss(remaining);

            char state_char;
            iss >> state_char;

            switch (state_char) {
            case 'R':
                process_result.state = PS::Running;
                break;
            case 'S':
                process_result.state = PS::Sleeping;
                break;
            case 'D':
                process_result.state = PS::WaitingDisk;
                break;
            case 'Z':
                process_result.state = PS::Zombie;
                break;
            case 'T':
                process_result.state = PS::Stopped;
                break;
            default:
                process_result.state = PS::Sleeping;
                break;
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

void DataCollector::parseProcessCmdline(const fs::path& dir_path, ProcessData& process_result)
{
    std::ifstream cmd_file(dir_path / "cmdline");
    if (!cmd_file.is_open())
        return;

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

std::unordered_map<u64, ProcessData> DataCollector::collectProcesses()
{
    std::unordered_map<u64, ProcessData> processes_result;
    static const long page_size_kb = sysconf(_SC_PAGESIZE) / 1024;
    std::error_code ec;

    for (const auto& entry : fs::directory_iterator("/proc", ec)) {
        if (!entry.is_directory(ec)) {
            continue;
        }

        str dir_name = entry.path().filename().string();

        if (!std::all_of(dir_name.begin(), dir_name.end(), ::isdigit)) {
            continue;
        }

        ProcessData process_result;
        process_result.id = std::stoull(dir_name);

        parseProcessStatus(entry.path(), process_result);
        parseProcessStatm(entry.path(), process_result, page_size_kb);
        parseProcessStat(entry.path(), process_result);
        parseProcessCmdline(entry.path(), process_result);

        processes_result.emplace(process_result.id, std::move(process_result));
    }

    return processes_result;
}

SystemData DataCollector::CollectData()
{
    SystemData result;
    std::unordered_map<u64, SystemCPUData> current_cpu = collectCPU();
    result.ram = collectRAM();

    std::ifstream uptime_file("/proc/uptime");
    if (uptime_file.is_open()) {
        double uptime;
        uptime_file >> uptime;
        result.uptime_sec = static_cast<u64>(uptime);
    }

    std::ifstream loadavg_file("/proc/loadavg");
    if (loadavg_file.is_open()) {
        std::string proc_count;
        loadavg_file >> result.load_avg[0] >> result.load_avg[1] >> result.load_avg[2]
            >> proc_count;

        size_t slash_pos = proc_count.find('/');
        if (slash_pos != std::string::npos) {
            result.tasks_running = std::stoul(proc_count.substr(0, slash_pos));
            result.tasks_total = std::stoul(proc_count.substr(slash_pos + 1));
        }
    }

    static const double clock_ticks_per_sec = static_cast<double>(sysconf(_SC_CLK_TCK));

    u64 global_total_delta = 1;

    for (const auto& [id, current_data] : current_cpu) {
        auto prev_it = _previous_cpus_tick.find(id);

        if (prev_it == _previous_cpus_tick.end()) {
            result.cpus_load[id] = 0.0f;
            continue;
        }

        const SystemCPUData& previous_data = prev_it->second;

        const u64 current_total_time = current_data.getTotalTime();
        const u64 previous_total_time = previous_data.getTotalTime();

        if (current_total_time <= previous_total_time) {
            result.cpus_load[id] = 0.0f;
            continue;
        }

        const u64 total_delta_time = current_total_time - previous_total_time;

        if (id == 0) {
            global_total_delta = total_delta_time;
        }

        const u64 current_idle = current_data.getTotalSleepTime();
        const u64 previous_idle = previous_data.getTotalSleepTime();

        u64 idle_delta = 0;
        if (current_idle > previous_idle) {
            idle_delta = current_idle - previous_idle;
        }

        float load = 100.0f
            * (1.0f - static_cast<float>(idle_delta) / static_cast<float>(total_delta_time));

        if (load < 0.0f) {
            load = 0.0f;
        }
        if (load > 100.0f) {
            load = 100.0f;
        }

        result.cpus_load[id] = load;
    }

    auto current_processes = collectProcesses();
    static const int num_cores = sysconf(_SC_NPROCESSORS_ONLN);

    for (auto& [pid, process] : current_processes) {
        if (result.ram.total > 0) {
            process.mem_load
                = (static_cast<float>(process.resident_set_size) / result.ram.total) * 100.0f;
        }

        auto it = _previous_processes_cpu_tick.find(pid);
        if (it != _previous_processes_cpu_tick.end()) {
            if (process._total_time >= it->second.getTotalTime()) {
                const u64 proc_time_delta = process._total_time - it->second.getTotalTime();

                process.cpu_load = (static_cast<float>(proc_time_delta) / global_total_delta)
                    * 100.0f * num_cores;
            } else {
                process.cpu_load = 0.0f;
            }
        } else {
            process.cpu_load = 0.0f;
        }

        process.execution_time = static_cast<double>(process._total_time) / clock_ticks_per_sec;
    }

    result.processes = current_processes;
    _previous_cpus_tick = std::move(current_cpu);

    _previous_processes_cpu_tick.clear();
    for (const auto& [pid, process] : current_processes) {
        ProcessCPUData cpu_data;
        cpu_data.id = process.id;
        cpu_data.use_time = process._use_time;
        cpu_data.system_use_time = process._system_use_time;
        _previous_processes_cpu_tick[pid] = cpu_data;
    }

    return result;
}
}