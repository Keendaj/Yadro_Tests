#pragma once
#include <cstdint>
#include <filesystem>
#include <string>

namespace DataCollector {
using str = std::string;
namespace fs = std::filesystem;

using u64 = uint64_t;
using u32 = uint32_t;
using u16 = uint16_t;
using u8 = uint8_t;

using i64 = int64_t;
using i32 = int32_t;
using i16 = int16_t;
using i8 = int8_t;

/**
 * @brief Перечисление состояний жизненного цикла процесса в Linux.
 * * Отражает статусы, извлекаемые из поля state файла /proc/[pid]/stat.
 */
enum class ProcessState {
    Sleeping, ///< S: Процесс ожидает события (прерываемый сон)
    Running, ///< R: Процесс выполняется или находится в очереди на выполнение
    Zombie, ///< Z: Процесс завершен, но родитель еще не считал код возврата
    WaitingDisk, ///< D: Непрерываемый сон (обычно ожидание IO/диска)
    Stopped ///< T: Процесс остановлен сигналом
};
using PS = ProcessState;

/**
 * @brief Преобразует состояние процесса в строковый символ (как в утилите
 * top/htop).
 * @param state Состояние процесса (enum ProcessState).
 * @return str Односимвольная строка ("R", "S", "Z", "D", "E" для ошибок).
 */
inline str to_string(PS state)
{
    switch (state) {
    case ProcessState::Sleeping:
        return "S";
    case ProcessState::Running:
        return "R";
    case ProcessState::Zombie:
        return "Z";
    case ProcessState::WaitingDisk:
        return "D";
    default:
        return "E";
    }
}
}; // namespace DataCollector
