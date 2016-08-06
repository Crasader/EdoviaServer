#pragma once

#include <forward_list>
#include <mutex>

#include "utility/utility.h"

class Zone;

/**
 * @brief A ZonePool represents a thread which can hold multiple Zones.
 *
 * Zones can be dynamically added or removed to/from the ZonePool.
 * The ZonePool will update all Zones accociated with it.
 */
class ZonePool
{
    /// For protecting the list of all current Zones
    std::mutex mZoneListMutex;

    /// List of all current accociated Zones
    std::forward_list<Zone*> mZones;

    /**
     * Updates the Pool, called from it's thread
     */
    void update(TimePoint difference);
public:
    ZonePool() {}
    ~ZonePool() {}

    /**
     * Adds a Zone to the ZonePool.
     * The ZonePool will then call zone->update() from it's thread
     * @param zone pointer to a Zone
     */
    void addZone(Zone* zone);

    /**
     * Removes a Zone from the ZonePool and stops updating it
     * @param zone pointer to a Zone
     */
    void removeZone(Zone* zone);

    void run();

    const std::forward_list<Zone*>* getZoneList() { return &mZones; }
};
