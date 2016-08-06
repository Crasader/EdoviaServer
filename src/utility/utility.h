#pragma once

#include <cstdint>
#include <chrono>
#include <thread>

typedef uint32_t TimePoint;

/**
 *
 * @return current time in Milliseconds
 */
inline TimePoint getTimeMilliseconds()
{
    auto time_since_epoch = std::chrono::steady_clock::now().time_since_epoch();

    return (TimePoint)std::chrono::duration_cast<std::chrono::milliseconds>(time_since_epoch).count();
}

/**
 * Sleeps the calling thread for the amount
 * @param milliseconds amount in milliseconds
 */
inline void sleepMilliseconds(TimePoint milliseconds)
{
    return std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}