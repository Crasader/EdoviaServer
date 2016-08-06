#include "ZonePool.h"

#include "Zone.h"
#include "Server/Server.h"

static const TimePoint const_tickrate = 20;
static const TimePoint const_sleeptime = 1000/const_tickrate;


void ZonePool::update(TimePoint difference)
{
    std::lock_guard<std::mutex> lock(mZoneListMutex);

    for(auto& zone: mZones)
    {
        zone->update(difference);
    }
}

void ZonePool::addZone(Zone* zone)
{
    std::lock_guard<std::mutex> lock(mZoneListMutex);
    mZones.push_front(zone);
}

void ZonePool::removeZone(Zone* zone)
{
    std::lock_guard<std::mutex> lock(mZoneListMutex);
    mZones.remove(zone);
}

void ZonePool::run()
{
    TimePoint currentTime = 0;
    TimePoint previousTime = getTimeMilliseconds();

    TimePoint previousSleepTime = 0;

    while (!Server::isStopping())
    {
        currentTime = getTimeMilliseconds();

        TimePoint timeDifference = currentTime-previousTime;

        previousTime = currentTime;

        update(timeDifference);

        if (timeDifference <= const_sleeptime + previousSleepTime)
        {
            previousSleepTime = const_sleeptime + previousSleepTime - timeDifference;

            sleepMilliseconds(previousSleepTime);
        }
        else
        {
            previousSleepTime = 0;
        }
    }
}
