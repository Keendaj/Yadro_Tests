#pragma once
#include "options.hpp"

#ifdef PARSER_REALISATION
    #include <iostream>
    #include <string>
    #include <unistd.h>  
    #include <pwd.h>  
    #include <cstdlib> 
    #include <chrono> 
    #include <iomanip>
#endif

namespace MediaReader
{
    class ReaderConfigParser
    {
        private:
            int argc;
            char** argv;

            str getHomeDirectory() const;
        public:
            ReaderConfigParser(int argc, char * argv[]);

            Config parse() const;    
    };

    #ifdef PARSER_REALISATION

        ReaderConfigParser::ReaderConfigParser(int argc, char* argv[]) : argc(argc), argv(argv) {}

        str ReaderConfigParser::getHomeDirectory() const {
            const char* home_env = std::getenv("HOME");
            if (home_env) {
                return str(home_env);
            }
            
            struct passwd* pwd = getpwuid(getuid());
            if (pwd) {
                return str(pwd->pw_dir);
            }

            return "/";
        }

        Config ReaderConfigParser::parse() const {
            Config config;
            
            config.scan_catalog = fs::path(getHomeDirectory());
            config.scan_interval = 60; 

            optind = 1;             
            int opt;
            while ((opt = getopt(argc, argv, "d:i:")) != -1) {
                switch (opt) {
                    case 'd':
                        config.scan_catalog = fs::path(optarg);
                        break;
                    case 'i':
                    {
                        int interval = std::stoi(optarg);
                        if (interval > 0) {
                            
                            config.scan_interval = interval;
                            break;
                        }
                        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

                        std::cerr << "[" << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S") 
                                  << "] Invterval must be > 0" << std::endl;
                        break;
                    }
                    default:
                        std::exit(1);
                }
            }
            
            return config;
        }
    #endif
}