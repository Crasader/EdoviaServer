#pragma once

#include <forward_list>
#include <uv.h>

#include "MemoryPool/MemoryPool.h"

#include "Connection.h"

/**
 * @brief Networking-Management Class
 *
 * Acts as an adapter for a networking interface. Connections are
 * of the type Connection.
 */
class Network
{
    /// List of connections, used for cleaning up on shutdown
    std::forward_list<Connection*> mConnections;

    struct NetworkBuffer { uint8_t buffer[2048]; };

    /// Memory pool for efficient managing of uv receive buffers
    MemoryPool<NetworkBuffer, sizeof(NetworkBuffer)*1024> mPool;

    /// uv networking loop
    uv_loop_t uv_loop;

    /// TCP server binding
    uv_tcp_t uv_server;

    /// used for sending stop notification
    uv_async_t uv_async;


    /**
     * Creates a new Connection object, and keeps track of it
     * @return Pointer to the new Connection object
     */
    Connection* createConnection()
    {
        Connection* connection = new Connection(&uv_loop, this);
        mConnections.push_front(connection);
        return connection;
    }

    /**
     * Stops the uv main loop, not to be called from other threads
     */
    void stop()
    {
        uv_stop(&uv_loop);
    }

    /**
     * Allocates/gets memory in the memory pool
     * Memory should be returned with deallocateBuffer after use
     * @param handle uv_handle_t handle.data should point to a Connection pointer
     * @param suggested_size ignored
     * @param buf pointer to uv_buf_t struct
     */
    inline static void allocationCallback(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
    {
        Network* network = (reinterpret_cast<Connection*>(handle->data))->getNetwork();

        NetworkBuffer* buffer = network->mPool.allocate();
        *buf = uv_buf_init((char*)buffer, sizeof(NetworkBuffer));
    }

    /**
     * Callback for uv. Called when a new client connects
     * Creates a Connect object and starts streaming data to it
     * @param server uv server handle
     * @param status zero on success
     */
    void onNewConnection(uv_stream_t* server, int status);

public:
    Network();
    ~Network();

    /**
     * Starts listening for network connections
     * @param port listening port
     */
    void listen(int port);

    /**
     * Will stop the network asynchronously
     * Thread Safe
     */
    inline void sendStop()
    {
        uv_async_send(&uv_async);
    }

    /**
     * Returns back memory allocated with allocationCallback
     * @param buffer uv_buf_t structure
     */
    inline void deallocateBuffer(const uv_buf_t* buffer)
    {
        mPool.deallocate((NetworkBuffer*)buffer->base);
    }

    /**
     * Destroys a connection object and removes it's reference from the list
     * @param connection Connection object
     */
    void destroyConnection(Connection* connection) { mConnections.remove(connection); delete connection; }
};
