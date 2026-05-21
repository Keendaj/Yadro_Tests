#pragma once
#include "utils/types.hpp"
#include "utils/logger.hpp"
#include <thread>
#include <atomic>
#include <mutex>

#ifdef UDS_SERVER_REALISATION
    #include <sys/socket.h>
    #include <sys/un.h>
    #include <unistd.h>
    #include <cstring>
    #include <chrono>
    #include <ctime>
    #include <iostream>
#endif

namespace DataCollector
{
    class UdsServer
    {
        private:
            str _socket_path;
            int _server_fd = -1;
            int _client_fd = -1;
            
            std::atomic<bool> _is_running{false};
            std::thread _accept_thread;
            std::mutex _client_mutex;

            void acceptLoop();

        public:
            UdsServer(const str& socket_path);
            ~UdsServer();

            bool Start();
            void Stop();
            void Broadcast(const str& json_data);
    };

    #ifdef UDS_SERVER_REALISATION

        void UdsServer::acceptLoop()
        {
            while (_is_running)
            {
                struct sockaddr_un client_addr;
                socklen_t client_len = sizeof(client_addr);
                
                int new_client_fd = accept(_server_fd, (struct sockaddr*)&client_addr, &client_len);
                
                if (new_client_fd < 0) {
                    if (_is_running) {
                        Utils::log("UdsServer", "ERROR", "Accept failed");
                    }
                    continue;
                }

                Utils::log("UdsServer", "INFO", "Client connected!");

                {
                    std::lock_guard<std::mutex> lock(_client_mutex);
                    if (_client_fd != -1) {
                        close(_client_fd);
                    }
                    _client_fd = new_client_fd;
                }
            }
        }

        UdsServer::UdsServer(const str& socket_path) : _socket_path(socket_path) {}

        UdsServer::~UdsServer() 
        {
            Stop();
        }

        bool UdsServer::Start()
        {
            _server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
            if (_server_fd < 0) {
                Utils::log("UdsServer", "ERROR", "Failed to create socket");
                return false;
            }

            struct sockaddr_un server_addr;
            memset(&server_addr, 0, sizeof(server_addr));
            server_addr.sun_family = AF_UNIX;
            strncpy(server_addr.sun_path, _socket_path.c_str(), sizeof(server_addr.sun_path) - 1);

            unlink(_socket_path.c_str());

            if (bind(_server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
                Utils::log("UdsServer", "ERROR", "Bind failed");
                close(_server_fd);
                return false;
            }

            if (listen(_server_fd, 1) < 0) {
                Utils::log("UdsServer", "ERROR", "Listen failed");
                close(_server_fd);
                return false;
            }

            _is_running = true;
            _accept_thread = std::thread(&UdsServer::acceptLoop, this);
            
            Utils::log("UdsServer", "INFO", "Listening on " + _socket_path + "...");
            return true;
        }

        void UdsServer::Stop()
        {
            if (!_is_running) return;
            _is_running = false;

            if (_server_fd != -1) {
                shutdown(_server_fd, SHUT_RDWR);
                close(_server_fd);
                _server_fd = -1;
            }

            {
                std::lock_guard<std::mutex> lock(_client_mutex);
                if (_client_fd != -1) {
                    close(_client_fd);
                    _client_fd = -1;
                }
            }

            if (_accept_thread.joinable()) {
                _accept_thread.join();
            }
            unlink(_socket_path.c_str());
        }

        void UdsServer::Broadcast(const str& json_data)
        {
            std::lock_guard<std::mutex> lock(_client_mutex);
            if (_client_fd == -1) return;

            str payload = json_data + "\n";

            ssize_t bytes_sent = send(_client_fd, payload.c_str(), payload.length(), MSG_NOSIGNAL);
            
            if (bytes_sent < 0) {
                Utils::log("UdsServer", "WARNING", "Client disconnected (send failed)");
                close(_client_fd);
                _client_fd = -1;
            }
        }

    #endif
};