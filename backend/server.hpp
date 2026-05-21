#pragma once
#include <atomic>
#include <mutex>
#include <thread>

#include "utils/logger.hpp"
#include "utils/types.hpp"

namespace DataCollector {
/**
 * @brief Сервер Unix Domain Sockets (UDS) для трансляции системных метрик.
 * Создает локальный сокет, принимает подключения
 * и отправляет им потоковые данные в формате JSON.
 */
class UdsServer {
private:
    str _socket_path;
    int _server_fd = -1;
    int _client_fd = -1;

    std::atomic<bool> _is_running { false };
    std::thread _accept_thread;
    std::mutex _client_mutex;

    /**
     * @brief Фоновый цикл для принятия новых подключений от клиентов.
     */
    void acceptLoop();

public:
    /**
     * @brief Инициализирует сервер с заданным путем к сокету.
     * @param socket_path Путь к файлу сокета (например, "/tmp/system_data.sock").
     */
    UdsServer(const str& socket_path);
    ~UdsServer();

    UdsServer(const UdsServer&) = delete;
    UdsServer& operator=(const UdsServer&) = delete;
    UdsServer(UdsServer&&) = delete;
    UdsServer& operator=(UdsServer&&) = delete;

    /**
     * @brief Запускает сервер в отдельном потоке.
     * @return true при успешном запуске, false в случае ошибки (bind, listen).
     */
    bool Start();

    /**
     * @brief Останавливает сервер, закрывает соединения и удаляет файл сокета.
     */
    void Stop();

    /**
     * @brief Отправляет JSON-строку текущему подключенному клиенту.
     * @param json_data Подготовленная строка с данными.
     */
    void Broadcast(const str& json_data);
};
}; // namespace DataCollector