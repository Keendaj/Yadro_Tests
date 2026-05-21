#pragma once
#include <string>
#include <filesystem>

namespace DataCollector 
{
    using str = std::string;
    namespace fs = std::filesystem;

    using u64 = unsigned long long;
    using u32 = unsigned int;
    using u16 = unsigned short;
    using u8 = char;

    using i64 = long long;
    using i32 = int;
    using i16 = short;
    using i8 = signed char;

    enum class ProcessState
    {
        Sleeping,
        Running,
        Zombie,
        WaitingDisk,
        Stopped
    };
    using PS = ProcessState;

    inline str to_string(PS state) {
        switch (state) {
            case ProcessState::Sleeping:    return "S";
            case ProcessState::Running:     return "R";
            case ProcessState::Zombie:      return "Z";
            case ProcessState::WaitingDisk: return "D";
            default:                        return "E";
        }
    }
}



