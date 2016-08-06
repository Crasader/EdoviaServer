#pragma once

#include "ZonePool.h"

#include <cassert>

class PlayerSession;
class Zone;

#define sZoneManager ZoneManager::getInstance()

/**
 * @brief Manages all ZonePools
 *
 * Creates all ZonePools.
 * Assigns Players to the correct Zone.
 *
 * @remark Singleton
 */
class ZoneManager
{
    /// The Singleton instnace
    static ZoneManager* instance;

    /// List of all ZonePools
    std::forward_list<ZonePool*> zonePools;
public:
    ZoneManager()
    {
        if (ZoneManager::instance != nullptr)
            assert(false);

        ZoneManager::instance = this;
    }

    /**
     * Returns the ZoneManager
     */
    static ZoneManager* getInstance() { return instance; }

    /**
     * Assigns a PlayerSession to a Zone
     * @param session the PlayerSession which is not yet assigned
     * @return the assigned Zone Pointer
     */
    Zone* assignZone(PlayerSession* session);

    /**
     * Creates a new ZonePool
     * @return Pointer to the just created ZonePool
     */
    ZonePool* createZonePool()
    {
        ZonePool* zonePool = new ZonePool();
        zonePools.push_front(zonePool);
        return zonePool;
    }

    /**
     * Deletes a ZonePool
     */
    void deleteZonePool(ZonePool* zonePool)
    {
        zonePools.remove(zonePool);
        delete zonePool;
    }
};
