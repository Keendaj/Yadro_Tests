#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <httplib.h>
#include <nlohmann/json.hpp>

#define PARSER_REALISATION
#include "parser.hpp"

#define READER_REALISATION
#include "reader.hpp"

using json = nlohmann::json;
using namespace MediaReader;

std::mutex json_mutex;
json global_media_json = json::object();

void scanner_thread_func(Config config) {
    Reader reader(config);
    
    while (true) {
        json current_scan_result = reader.scan(); 

        {
            std::unique_lock<std::mutex> lock(json_mutex);
            global_media_json = current_scan_result;
        }

        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::cout << "[" << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S") 
                  << "] Directory scanned. JSON updated." << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(config.scan_interval));
    }
}

int main(int argc, char* argv[]) {
    ReaderConfigParser parser(argc, argv);
    Config config = parser.parse();

    std::thread scanner(scanner_thread_func, config);
    scanner.detach();

    httplib::Server svr;

    svr.Get("/media_files", [](const httplib::Request& req, httplib::Response& res) {
        std::string response_body;
        
        {
            std::unique_lock<std::mutex> lock(json_mutex);
            response_body = global_media_json.dump(4);
        }
        
        res.set_content(response_body, "application/json");
        
        std::cout << "Served GET /media_files to " << req.remote_addr << std::endl;
    });

    std::cout << "Starting HTTP Server on http://localhost:1234" << std::endl;
    std::cout << "Scanning directory: " << config.scan_catalog.string() << std::endl;
    
    if (!svr.listen("localhost", 1234)) {
        std::cerr << "Failed to start HTTP server. Port might be in use." << std::endl;
        return 1;
    }

    return 0;
}