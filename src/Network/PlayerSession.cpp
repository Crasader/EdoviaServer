#include "PlayerSession.h"

#include "Connection.h"
#include "Server/Server.h"
#include "World/Zone.h"
#include "World/ZoneManager.h"

PlayerSession::PlayerSession(Connection* connection) : mConnection(connection)
{

}

PlayerSession::~PlayerSession()
{
    if (mZone && !Server::isStopping())
        mZone->removeSession(this);
}

void PlayerSession::enterGame()
{
    mZone = sZoneManager->assignZone(this);
}

void PlayerSession::update(TimePoint difference)
{
    PacketContainer container;

    while (mPacketReceiveQueue.try_dequeue(container))
    {
        (this->*container.callback)(container.packet);
    }
}

void PlayerSession::sendPacket(std::shared_ptr<Packet> packet)
{
    mConnection->queueSendPacket(packet);
}

void PlayerSession::handleMovementPacket(std::shared_ptr<Packet> packet)
{
    float x,y,z;
    Packet p = *packet.get();
    p >> x;
    p >> y;
    p >> z;
    //log->info("New Movement packet! {} {} {}", x, y, z);

    mZone->sendPositionUpdate(this, x, y, z);
}
