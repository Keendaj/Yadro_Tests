#define UDS_SERVER_REALISATION
#define DATA_COLLECTOR_REALISATION
#define DATA_STRUCTURE_REALISATION
#define UTILS_LOGGER_REALISATION

#include "utils/logger.hpp"
#include "utils/serializers.hpp"
#include "server.hpp"
#include "data_collector.hpp"

#include <thread>
#include <chrono>

using namespace DataCollector;

int main()
{
    Utils::log("Main", "INFO", "Starting Data Collector Backend...");

    UdsServer server("/tmp/system_data.sock");
    if (!server.Start()) {
        Utils::log("Main", "ERROR", "Failed to start UDS server. Exiting.");
        return 1;
    }

    DataCollector::DataCollector collector;

    Utils::log("Main", "INFO", "Entering main loop...");

    while (true)
    {
        auto start_time = std::chrono::steady_clock::now();

        SystemData current_data = collector.CollectData();

        nlohmann::json json_payload = current_data;
        
        str json_string = json_payload.dump(); 

        server.Broadcast(json_string);

        auto end_time = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        auto sleep_duration = std::chrono::milliseconds(1000) - elapsed;
        if (sleep_duration.count() > 0) {
            std::this_thread::sleep_for(sleep_duration);
        }
    }

    server.Stop();
    return 0;
}