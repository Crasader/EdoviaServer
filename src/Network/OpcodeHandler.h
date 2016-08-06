#pragma once

#include <functional>
#include <memory>

#include "Packet.h"
#include "PlayerSession.h"

/**
 * @brief Utility class for all packet types
 *
 * Holds all opcodes and has a list of all
 * callbacks for each opcode, which are responsible
 * for processing a packet.
 *
 * @remark There should be always only one object of the class, accessed via the global opcodeHandler.
 */
class OpcodeHandler
{
public:
    /**
     * All Opcodes and their number (same on client and server)
     */
    enum class Opcodes
    {
        RESERVED = 0,
        RESERVED_2 = 10,
        CS_MOVEPACKET = 11,
        SC_MOVEPACKET = 12,
        NUM,
    };

    /**
     * Only public because of out of class declaration
     */
    OpcodeHandler();

private:
    /**
     * Defines a callback for a packet
     */
    struct PacketCallback
    {
        /**
         * Callback pointing to a PlayerSession function
         * and having a shared_ptr of Packet as argument
         */
        void (PlayerSession::*callback)(std::shared_ptr<Packet>);
    };

    /// Array of all callbacks associated with an opcode by it's index
    PacketCallback callbacks[static_cast<int>(Opcodes::NUM)];

    /**
     * Associates all opcodes with a callback
     */
    void initializeOpcodes();

public:
    /**
     * Called by a Connection object
     * Queues an incoming packet for processing
     * @param session PlayerSession which should process the packet
     * @param packet the packet
     */
    void processPacket(PlayerSession* session, std::shared_ptr<Packet> packet)
    {
        // TODO additional checks
        if (callbacks[packet->getOpcode()].callback == 0)
            return;

        session->queuePacket(callbacks[packet->getOpcode()].callback, packet);
    }
};

extern OpcodeHandler opcodeHandler;