#include "Zone.h"

#include "Network/Packet.h"
#include "Network/PlayerSession.h"
#include "Network/OpcodeHandler.h"

void Zone::update(TimePoint difference)
{
    {
        std::lock_guard<std::mutex> lock(mSessionListMutex);

        for (auto& session : mSessionList)
        {
            session->update(difference);
        }
    }
}

void Zone::sendPositionUpdate(PlayerSession* session, float x, float y, float z)
{
    for (auto& sessionIterator : mSessionList)
    {
        if (sessionIterator == session)
            continue;

        std::shared_ptr<Packet> packet = std::make_shared<Packet>((int)OpcodeHandler::Opcodes::SC_MOVE);
        Packet& packetp = *packet.get();
        packetp << x;
        packetp << y;
        packetp << z;

        sessionIterator->sendPacket(packet);
    }
}