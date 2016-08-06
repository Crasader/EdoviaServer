#include "Server.h"

#include <thread>

#include "Log/Logger.h"
#include "Network/Network.h"
#include "World/Zone.h"
#include "World/ZonePool.h"
#include "World/ZoneManager.h"

std::atomic<bool> Server::mStopping{false};

Server::Server()
{
    mNetwork = new Network();
}

Server::~Server()
{
    delete mNetwork;
}

void Server::run()
{
    // Start Network Thread
    std::thread networkThread([this](){
        mNetwork->listen(40000);
    });

    // Create zones (only one for now)
    Zone* zone = new Zone();

    ZoneManager* zoneManager = new ZoneManager();
    ZonePool* zonePool = zoneManager->createZonePool();
    zonePool->addZone(zone);

    std::thread zonePoolThread([this, zonePool]() {
        zonePool->run();
    });

    // Shutting down
    zonePoolThread.join();
    zoneManager->deleteZonePool(zonePool);
    delete zoneManager;

    networkThread.join();
    delete zone;


    log->info("Server stopping");
}

void Server::stopServer()
{
    // already stopping
    if (Server::mStopping == true)
        return;

    Server::mStopping = true;

    log->info("Start stopping server");

    mNetwork->sendStop();
}
