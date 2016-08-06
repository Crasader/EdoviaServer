#pragma once

#include <memory>

#include "concurrentqueue/concurrentqueue.h"

#include "Packet.h"
#include "utility/utility.h"


class Connection;
class Zone;

/**
 * @brief Represents the connection of a Player
 *
 * The Session is associated to a Network-Connection
 * as well as to a Zone.
 * The network connection sends incoming packets to
 * the session and places them in a queue for the Zone thread.
 * The Zone updates the Session and can so process packets directly
 * in the Zones thread.
 * Also the Session forwards write requests asynchrounsly
 * to the corresponding connection.
 *
 */
class PlayerSession
{
    /// the associated network connection
    Connection* mConnection;

    /// the current associated zone
    Zone* mZone = nullptr;

    /**
     * A container holding a packet with it's information
     * as well as a callback for the function which is
     * responsible for processing the packet. The callback
     * is inside the PlayerSession class
     */
    struct PacketContainer
    {
        std::shared_ptr<Packet> packet;
        void (PlayerSession::*callback)(std::shared_ptr<Packet>);
    };

    /// A concurrent queue for the packets
    moodycamel::ConcurrentQueue<PacketContainer> mPacketReceiveQueue;

public:
    /**
     * Initializes a new PlayerSession with the specified network connection
     * @param connection the network connection
     */
    PlayerSession(Connection* connection);

    /**
     * Deletes the session. Also removes the Session from the associated
     * zone (if any).
     */
    ~PlayerSession();

    /**
     * Tells the ZoneManager to assign a Zone for this player
     * After that the Session will be updated from the assigned Zone
     */
    void enterGame();

    /**
     * Queues a incoming packet, to be fetched from the Zone thread
     * @param callback to the function responsible for processing the packet
     * @param packet data
     */
    void queuePacket(void (PlayerSession::*callback)(std::shared_ptr<Packet>), std::shared_ptr<Packet> packet)
    {
        PacketContainer container = {packet, callback};
        mPacketReceiveQueue.enqueue(container);
    }

    /**
     * Sends a packet to the player.
     * @param packet data
     * @remark thread-safe
     */
    void sendPacket(std::shared_ptr<Packet> packet);

    /**
     * Updates the Session, to be called from the corresponding Zone
     */
    void update(TimePoint difference);

    void handleMovementPacket(std::shared_ptr<Packet> packet);
};
