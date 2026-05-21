#pragma once
#include <unordered_map>

#include "types.hpp"

namespace DataCollector {
/**
 * @brief Статистика процессорного времени.
 * Парсится из файла /proc/stat.
 */
struct SystemCPUData {
public:
    u64 user = 0; ///< Тики в пользовательском режиме (user)
    u64 nice = 0; ///< Тики в пользовательском режиме с пониженным приоритетом
    u64 system = 0; ///< Тики в режиме ядра (system)
    u64 idle = 0; ///< Тики простоя (idle)
    u64 iowait = 0; ///< Тики ожидания ввода-вывода (iowait)
    u64 irq = 0; ///< Тики обработки аппаратных прерываний
    u64 softirq = 0; ///< Тики обработки программных прерываний
    u64 steal = 0; ///< Тики, затраченные на другие виртуальные машины
    u64 guest = 0; ///< Тики гостевой ОС
    u64 guest_nice = 0; ///< Тики гостевой ОС с пониженным приоритетом

    /** @brief Подсчёт общего времени.
     * @return u64 Сумма всех тиков
     */
    u64 getTotalTime() const;
    /** @brief Подсчёт времени, когда процессор выполнял полезную работу.
     * @return u64 Сумма всех тиков, кроме idle и iowait
     */
    u64 getTotalUseTime() const;
    /** @brief  Подсчёт времени простоя.
     * @return u64 Сумма idle и iowait
     */
    u64 getTotalSleepTime() const;
};

/**
 * @brief Состояние оперативной памяти и файла подкачки (Swap).
 * Значения хранятся в килобайтах (Kb), парсятся из /proc/meminfo.
 */
struct SystemRAMData {
public:
    u64 total = 0; ///< Общий объем физической RAM
    u64 available = 0; ///< Память, доступная для запуска новых процессов без ухода в Swap
    u64 buffer = 0; ///< Память под буферы блочных устройств
    u64 shmem = 0; ///< Разделяемая память (tmpfs, shm)
    u64 cached = 0; ///< Память под кэш страниц файловой системы
    u64 used = 0; ///< Вычисляемое значение занятой памяти (total - available)

    u64 swap_total = 0; ///< Общий объем Swap
    u64 swap_free = 0; ///< Свободный объем Swap
    u64 swap_used = 0; ///< Используемый объем Swap (swap_total - swap_free)
};

/**
 * @brief Промежуточная структура для хранения тиков процессорного времени
 * конкретного процесса. Используется для вычисления дельты времени между двумя
 * циклами сбора данных и последующего расчета процента загрузки (cpu_load).
 */
struct ProcessCPUData {
public:
    u64 id = 0; ///< PID (идентификатор) процесса
    u64 use_time = 0; ///< Тики, проведенные процессом в пользовательском режиме (utime)
    u64 system_use_time = 0; ///< Тики, проведенные процессом в режиме ядра (stime)

    /**
     * @brief Возвращает общую сумму тиков, затраченных на выполнение процесса.
     * @return u64 Сумма пользовательского и системного времени (use_time +
     * system_use_time).
     */
    u64 getTotalTime() const;
};

/**
 * @brief Полная метрика конкретного процесса.
 */
struct ProcessData {
public:
    u64 _use_time = 0; ///< [Внутреннее] Тики в user-space (utime)
    u64 _system_use_time = 0; ///< [Внутреннее] Тики в kernel-space (stime)
    u64 _total_time = 0; ///< [Внутреннее] Сумма тиков процесса

    u64 id = 0; ///< PID процесса
    str user = ""; ///< Имя владельца (пользователя)
    i8 priority = 0; ///< Приоритет планирования (nice)
    u64 virtual_memory_size = 0; ///< VIRT: Виртуальная память (в килобайтах)
    u64 resident_set_size = 0; ///< RES: Резидентная (физическая) память (в килобайтах)
    u64 shared_memory = 0; ///< SHR: Разделяемая память (в килобайтах)
    float cpu_load = 0; ///< Использование CPU (в процентах, 0.0 - 100.0 * cores)
    float mem_load = 0; ///< Использование RAM (в процентах, 0.0 - 100.0)
    double execution_time = 0.0; ///< TIME+: Общее время исполнения на CPU (в секундах)
    PS state = PS::Sleeping; ///< Текущее состояние процесса
    str command = ""; ///< Команда запуска с аргументами
};

/**
 * @brief Агрегированная структура состояния всей системы.
 * Подготавливается коллектором для сериализации и отправки клиенту.
 */
struct SystemData {
    std::unordered_map<u64, float> cpus_load; ///< Загрузка CPU: 0 - общая, 1..N - ядра
    SystemRAMData ram; ///< Данные об ОЗУ
    std::unordered_map<u64, ProcessData> processes; ///< Активные процессы (ключ - PID)

    float load_avg[3] = { 0.0f, 0.0f, 0.0f }; ///< Средняя загрузка (Load Avg) за 1, 5, 15 минут
    u64 uptime_sec = 0; ///< Время работы ОС (uptime) в секундах
    u32 tasks_total = 0; ///< Общее число потоков/процессов в системе
    u32 tasks_running = 0; ///< Число активно выполняемых (running) задач
};
}; // namespace DataCollector
