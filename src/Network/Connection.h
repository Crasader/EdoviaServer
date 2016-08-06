#pragma once

#include <uv.h>
#include <vector>

#include "concurrentqueue/concurrentqueue.h"

/**
 * @brief Struct consisting of a data pointer and it's size.
 */
struct RawPacket
{
    uint16_t packetLength;

    char* data;
};

class Network;
class Packet;
class PlayerSession;

/**
 * @brief Represents a duplex network connection
 *
 * A Connection object is always associated with a Network object.
 * It handles reading and writing of data.
 *
 * A packet consists of a header and a body. The header
 * holds the length of the body (two bytes). The body consists
 * of the opcode (two bytes) and variable length of data bytes.
 *
 * Header
 * -----------------------------
 * Field          | length
 * -------------- | -------------
 * Length of body | 2 Bytes
 *
 * Body
 * -----------------------------
 * Field          | length
 * -------------- | -------------
 * Opcode (type-number of Packet) | 2 Bytes
 * Data           | variable length
 *
 * So a packet has a minimum length (specified in the header)
 * of 3 bytes (opcode + one byte of data). And a real
 * tcp body size of minimum 5 bytes.
 *
 */
class Connection
{
    friend class Network;

    /// Reference to the Network Object the connection is accociated with
    Network* mNetwork;

    /// uv tcp handle
    uv_tcp_t* uv_client;

    /// number of bytes read for the packet which is currently processed
    uint16_t mCurrentPacketBytesRead = 0;

    /// size the packet will be when it's completly processed
    uint16_t mCurrentPacketSize = 0;

    /// the length field of a packet consists two bytes, true if only one read
    bool mCurrentPacketSizeReadHalf = false;

    /// bytes of the received data. Resets when a whole packet is read
    std::vector<uint8_t> mPacketBuffer;

    /// the associated playersession responsible for asynchronously processing the final packet
    /// assigned by ZoneManager
    PlayerSession* mPlayerSession = nullptr;

    /// Queue for packets which should be send to the client
    moodycamel::ConcurrentQueue<std::shared_ptr<Packet>> mPacketWriteQueue;

    /// For notifying the Connection (Network Thread) when a packet should be written
    uv_async_t* uv_async_write_notification;

    /**
     * Constructs a Connection object
     * @param uv_loop uv_loop of the Network object
     * @param network the Network object which creates the connection
     */
    Connection(uv_loop_t* uv_loop, Network* network);

    /**
     * Destructs and clears all accociated handles
     */
    ~Connection();

    /**
     * Disconnects the connection
     * Also destroys the object, and removes it's reference from the Network object
     */
    void disconnect();

    /**
     * UV Read callback
     * Called when new data belonging to this Connection is available
     * @param stream uv stream
     * @param nread size of data to read, < 0 on error, 0 if need to read again later
     * @param buf uv buffer object, the function deallocates it
     */
    void readCallback(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);

    /**
     * Reads a packet when the header is not or not fully received
     * Forwards the buffer to readPacketBody when header is fully received
     * @param buffer incoming buffer
     * @param size size of buffer
     */
    void readPacket(const char* buffer, ssize_t size);

    /**
     * Handles the incoming data for the packet body (after header received)
     * @param buffer incoming buffer
     * @param size size of buffer
     */
    void readPacketBody(const char* buffer, ssize_t size);

    /**
     * Called by readPacketBody when a packet is fully received
     * Process the packet by packing it and sending it to a PlayerSession
     */
    void processPacket();

    /**
     * Send a packet to the client
     * @param packet packet if it's opcode and data
     * @remark not thread-safe
     */
    void sendPacket(std::shared_ptr<Packet> packet);

    /**
     * Sends all packets in the send-queue
     *
     * Called when the async handler uv_async_write_notification is
     * notified
     */
    void sendQueuedPackets();

    /**
     * Gets the tcp handle
     * @return uv tcp handle
     */
    uv_tcp_t* getUVClient() { return uv_client; }

    /**
     * Gets the Network object associated with this connection
     * @return Pointer to Network
     */
    Network* getNetwork() { return mNetwork; }

public:
    /**
     * Queues a packet and sends it asynchronously by the network thread
     * @param packet shared pointer to a packet
     * @remark thread-safe
     */
    void queueSendPacket(std::shared_ptr<Packet> packet) { mPacketWriteQueue.enqueue(packet); uv_async_send(uv_async_write_notification); }
};
