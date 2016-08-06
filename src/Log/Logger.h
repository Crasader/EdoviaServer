#pragma once

#include "spdlog/logger.h"

#define log (Logger::getInstance()->getLogger())

class Logger
{
    static Logger* instance;

    std::shared_ptr<spdlog::logger> logger;

public:
    Logger();
    ~Logger();

    static inline Logger* getInstance() { return instance; }

    inline std::shared_ptr<spdlog::logger> getLogger() { return logger; }
};
