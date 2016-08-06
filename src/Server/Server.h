#pragma once

#include <atomic>

class Network;

/**
 * @brief Takes care of initializing and stopping the software
 */
class Server
{
    /// flag if the server is currently stopping
    static std::atomic<bool> mStopping;

    /// The networking object, will be changed to a pool sometime
    Network* mNetwork;

public:
    Server();
    ~Server();

    /**
     * Run the server
     *
     * @remark blocks
     */
    void run();

    /**
     * Asynchronously stops the server
     */
    void stopServer();

    /**
     * Returns if the server is currently stopping
     * static for easy access
     * @return true if the server is stopping
     */
    static bool isStopping() { return mStopping; }
};

