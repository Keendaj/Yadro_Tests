#pragma once
#include "options.hpp"
#include <nlohmann/json.hpp>

#ifdef READER_REALISATION    
    #include <iostream>
    #include <algorithm>
    #include <exception>
    #include <fstream>
    #include <chrono>
    #include <thread>
    #include <iomanip>
#endif

namespace MediaReader
{
    
    using json = nlohmann::json;
    class Reader{

    private:
        Config config;
    public:
        Reader(Config setup_config);


        json scan() const;
        bool create(const json& result_json) const;
        bool scanAndCreateFromHomeCatalog() const;
        void run() const;

        int getScanInterval() const;
        fs::path getPath() const;

        bool setScanInterval(const int newInterval);
        void setPath(const str& newPath);
        void setPath(const fs::path& newPath);
    };

    #ifdef READER_REALISATION
        

        Reader::Reader(Config setup_config) : config(setup_config) {}

        json Reader::scan() const {
            json result_json;

            for (const auto& [type, name] : MEDIATYPES_DESCRIPTION) {
                result_json[name] = json::array();
            }
            
            if(!fs::exists(config.scan_catalog) || !fs::is_directory(config.scan_catalog))
            {
                throw std::runtime_error("The path doesn't exists or isn't a directory:");
            }

            try {
                auto options = fs::directory_options::skip_permission_denied;
                for (const auto& entry : fs::recursive_directory_iterator(config.scan_catalog, options)) {
                    if (!entry.is_regular_file()) {
                        continue;
                    }

                    str ext = entry.path().extension().string();
                    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

                    auto it = EXTENTIONS.find(ext);
                    if (it != EXTENTIONS.end()) {
                        result_json[MEDIATYPES_DESCRIPTION.at(it->second)].push_back(entry.path().filename().string());
                    }
                }
            } catch (const std::exception& e) {
                throw;
            }
            
            return result_json;
        }

        bool Reader::create(const json& result_json) const {
            fs::path output_file = config.scan_catalog / ".media_files";
            std::ofstream out(output_file);

            auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

            if (!out.is_open()) {
                std::cerr << "[" << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S") 
                          << "] Couldn't open file to write: " << output_file << std::endl;
                return false; 
            } 

            out << result_json.dump(4);
            std::cout << "[" << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S") 
                      << "] FileUpdated: " << output_file << std::endl;
            return true;
        }

        bool Reader::scanAndCreateFromHomeCatalog() const
        {
            json result_json;
            try {
                result_json  = scan(); 
            }
            catch (const std::exception& e) {
                std::cerr << "[" << std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) 
                        << "] " << e.what() << std::endl;
                return false;
            }

            if(!create(result_json)) {
                return false;
            }
            
            return true;
        }

        int Reader::getScanInterval() const {
            return config.scan_interval; 
        }

        fs::path Reader::getPath() const {
            return config.scan_catalog;
        }

        bool Reader::setScanInterval(const int newInterval) {
            if (newInterval <= 0) {
                std::cerr << "[" << std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) 
                        << "] Invterval must be > 0" << std::endl;
                return false;
            } 
            config.scan_interval = newInterval;
            return true;
        }

        void Reader::setPath(const str& newPath) {
            config.scan_catalog = fs::path(newPath);
        }

        void Reader::setPath(const fs::path& newPath) {
            config.scan_catalog = newPath;
        }
        

        void Reader::run() const {
            std::cout << "StartReader...\n"
                      << "Catalog:  " << config.scan_catalog.string() << "\n"
                      << "Interval: " << config.scan_interval << " seconds.\n\n";

            while (true) {
                scanAndCreateFromHomeCatalog();
                std::this_thread::sleep_for(std::chrono::seconds(config.scan_interval));
            }
        }
    #endif
}
