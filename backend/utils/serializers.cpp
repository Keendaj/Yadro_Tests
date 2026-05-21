#include "serializers.hpp"
#include <nlohmann/json.hpp>

namespace DataCollector {
using json = nlohmann::json;

void to_json(json& j, const SystemRAMData& ram)
{
    j = json {
        { "total", ram.total },
        { "available", ram.available },
        { "used", ram.used },
        { "cached", ram.cached },
        { "buffer", ram.buffer },
        { "swap_total", ram.swap_total },
        { "swap_used", ram.swap_used },
        { "swap_free", ram.swap_free }
    };
}

void to_json(json& j, const ProcessData& proc)
{
    j = json {
        { "id", proc.id },
        { "user", proc.user },
        { "priority", proc.priority },
        { "state", to_string(proc.state) },
        { "virtual_memory", proc.virtual_memory_size },
        { "resident_memory", proc.resident_set_size },
        { "cpu_load", proc.cpu_load },
        { "mem_load", proc.mem_load },
        { "execution_time", proc.execution_time },
        { "command", proc.command }
    };
}

void to_json(json& j, const SystemData& data)
{
    json processes_array = json::array();
    for (const auto& [pid, proc] : data.processes) {
        processes_array.push_back(proc);
    }

    json cores_array = json::array();

    size_t total_items = data.cpus_load.size();
    for (u64 i = 1; i < total_items; ++i) {
        auto it = data.cpus_load.find(i);
        if (it != data.cpus_load.end()) {
            cores_array.push_back(it->second);
        } else {
            cores_array.push_back(0.0f);
        }
    }

    float total_load = 0.0f;
    auto it_total = data.cpus_load.find(0);
    if (it_total != data.cpus_load.end()) {
        total_load = it_total->second;
    }

    j = json {
        { "ram", data.ram },
        { "cpu_load", total_load },
        { "cpu_cores", cores_array },
        { "processes", processes_array },
        { "load_avg", { data.load_avg[0], data.load_avg[1], data.load_avg[2] } },
        { "uptime_sec", data.uptime_sec },
        { "tasks_total", data.tasks_total },
        { "tasks_running", data.tasks_running }
    };
}

} // namespace DataCollector