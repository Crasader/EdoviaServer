#include "ZoneManager.h"

#include "Zone.h"

ZoneManager* ZoneManager::instance = nullptr;

Zone* ZoneManager::assignZone(PlayerSession* session)
{
    for(auto& zonePool : zonePools)
    {
        for (auto& zone : *zonePool->getZoneList())
        {
            // return the first one found for now

            zone->addSession(session);

            return zone;
        }
    }

    assert(false);
    return nullptr;
}