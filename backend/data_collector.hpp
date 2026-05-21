#pragma once
#include <unordered_map>

#include "utils/data_structures.hpp"
#include "utils/logger.hpp"
#include "utils/types.hpp"

namespace DataCollector {
/**
 * @brief Оркестратор сбора системной метрики.
 * Класс инкапсулирует логику чтения виртуальной файловой системы /proc.
 * Он хранит предыдущие "тики" процессоров и процессов (stateful),
 * что позволяет корректно вычислять дельту времени и проценты загрузки CPU.
 */
class DataCollector {
private:
    std::unordered_map<u64, SystemCPUData> _previous_cpus_tick;
    std::unordered_map<u64, ProcessCPUData> _previous_processes_cpu_tick;

    /**
     * @brief Читает файл статуса процесса для извлечения UID владельца.
     * @param dir_path Путь к директории процесса (например, "/proc/1234").
     * @param process_result Ссылка на объект процесса для записи результата.
     */
    void parseProcessStatus(const fs::path& dir_path, ProcessData& process_result);

    /**
     * @brief Читает данные о памяти процесса из файла statm.
     * @param dir_path Путь к директории процесса (например, "/proc/1234").
     * @param process_result Ссылка на объект процесса для записи результата.
     * @param page_size_kb Размер страницы памяти ОС в килобайтах.
     */
    void parseProcessStatm(
        const fs::path& dir_path, ProcessData& process_result, long page_size_kb);

    /**
     * @brief Читает файл stat для извлечения состояния, приоритета и тиков CPU.
     * @param dir_path Путь к директории процесса (например, "/proc/1234").
     * @param process_result Ссылка на объект процесса для записи результата.
     */
    void parseProcessStat(const fs::path& dir_path, ProcessData& process_result);

    /**
     * @brief Читает команду запуска с аргументами из файла cmdline.
     * @param dir_path Путь к директории процесса (например, "/proc/1234").
     * @param process_result Ссылка на объект процесса для записи результата.
     * Заменяет null-байты на пробелы для формирования читаемой строки.
     */
    void parseProcessCmdline(const fs::path& dir_path, ProcessData& process_result);

protected:
    /**
     * @brief Собирает сырые тики процессора.
     * @return Словарь, где ключ 0 - общий CPU, 1..N - отдельные ядра.
     */
    std::unordered_map<u64, SystemCPUData> collectCPU();

    /** @brief Собирает актуальную статистику использования ОЗУ и Swap. */
    SystemRAMData collectRAM();

    /** @brief Итерируется по /proc и собирает данные по всем активным PID. */
    std::unordered_map<u64, ProcessData> collectProcesses();

public:
    /**
     * @brief Выполняет полный цикл сбора данных.
     * Вызывает все внутренние методы `collect...()`, рассчитывает загрузки (в
     * %) на основе дельты времени с прошлого вызова и обновляет внутренний кэш.
     * @return SystemData Готовый пакет метрик системы.
     */
    SystemData CollectData();
};

}; // namespace DataCollector