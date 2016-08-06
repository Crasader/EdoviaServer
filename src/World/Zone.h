#pragma once

#include <forward_list>
#include <mutex>

#include "utility/utility.h"

class PlayerSession;

/**
 * @brief Represents a Zone containg multiple objects which need periodical updates
 *
 * A Zone can hold multiple objects like Players, Monsters etc.
 *
 * All objects will be periodically updated. Also the Zone (and the accociated PlayerSessions)
 * take care of processing packets.
 *
 * A Zone belongs to a ZonePool, which will update the Zone
 */
class Zone
{
    /// For protecting the list of PlayerSessions
    std::mutex mSessionListMutex;

    /// List of PlayerSessions accociated with this Zone
    std::forward_list<PlayerSession*> mSessionList;

public:
    Zone() {}
    ~Zone() {}

    /**
     * The ZonePool will call this function for updating the whole Zone
     */
    void update(TimePoint difference);

    /**
     * Adds a Session to this Zone and starts updating it
     * @param playerSession
     * @remark Thread-Safe
     */
    void addSession(PlayerSession* playerSession)
    {
        std::lock_guard<std::mutex> lock(mSessionListMutex);
        mSessionList.push_front(playerSession);
    }

    /**
     * Removes a Session from this Zone and stops updating it
     * @param playerSession
     * @remark Thread-Safe
     */
    void removeSession(PlayerSession* playerSession)
    {
        std::lock_guard<std::mutex> lock(mSessionListMutex);
        mSessionList.remove(playerSession);
    }

    void sendPositionUpdate(PlayerSession* session, float x, float y, float z);
};
