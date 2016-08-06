#include <iostream>
#include <cstdlib>
#include "spdlog/spdlog.h"

// include after spdlog, so the log macro doesn't conflict with types of spdlog
#include "Logger.h"

Logger* Logger::instance = nullptr;

Logger::Logger()
{
    if (Logger::instance != nullptr)
    {
        std::cout << "Logger instance already exists" << std::endl;
        exit(-1);
    }

    Logger::instance = this;

    try
    {
        spdlog::set_async_mode(8192);

        std::vector<spdlog::sink_ptr> sinks;
        sinks.push_back(std::make_shared<spdlog::sinks::ansicolor_sink>(std::make_shared<spdlog::sinks::stdout_sink_mt>()));
        sinks.push_back(std::make_shared<spdlog::sinks::daily_file_sink_mt>("logfile", "txt", 23, 59));
        logger = std::make_shared<spdlog::logger>("name", begin(sinks), end(sinks));
        spdlog::register_logger(logger);

        spdlog::set_pattern("[%d.%m.%C %H:%M:%S][%t] %v");
    }
    catch (const spdlog::spdlog_ex& ex)
    {
        std::cout << "Log failed: " << ex.what() << std::endl;
    }
}

Logger::~Logger()
{
    spdlog::drop_all();
}
