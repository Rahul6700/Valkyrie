#include "Cache.hpp"
#include <iostream>

// Constructor: just stores the maximum cache size
Cache::Cache(size_t maxSize)
    : maxSize(maxSize)
{
}

// Lookup a key in the cache.
// If found, the key is spliced to the front of the LRU list (most recently used)
// and {true, value} is returned. Otherwise {false, ""}.
std::tuple<bool, std::string> Cache::get(const std::string& key)
{
    std::lock_guard<std::mutex> lock(cacheLock);

    auto it = cacheMap.find(key);
    if (it == cacheMap.end())
        return {false, ""};

    // Promote this key to the front — it is now the most recently used
    lruList.splice(lruList.begin(), lruList, it->second.second);

    //std::cout << "Fetched from cache" << std::endl;

    return {true, it->second.first};
}

// Insert or update a key-value pair.
// If the key already exists, the value is updated and the key is promoted to MRU.
// If the key is new and the cache is full, the LRU entry (back of list) is evicted
// before inserting the new entry at the front.
void Cache::put(const std::string& key, const std::string& value)
{
    std::lock_guard<std::mutex> lock(cacheLock);

    auto it = cacheMap.find(key);
    if (it != cacheMap.end())
    {
        // Key exists — update value and move to front (MRU)
        it->second.first = value;
        lruList.splice(lruList.begin(), lruList, it->second.second);
        return;
    }

    // New key: make room by evicting the least recently used entry if needed
    if (cacheMap.size() >= maxSize)
    {
        std::string lruKey = lruList.back();
        lruList.pop_back();
        cacheMap.erase(lruKey);
    }

    // Insert the new entry at the front (most recently used)
    lruList.push_front(key);
    cacheMap[key] = {value, lruList.begin()};
}

// Erase a single entry from the cache.
void Cache::erase(const std::string& key)
{
    std::lock_guard<std::mutex> lock(cacheLock);

    auto it = cacheMap.find(key);
    if (it != cacheMap.end())
    {
        lruList.erase(it->second.second);
        cacheMap.erase(it);
    }
}

// Invalidate a key, but only if the invalidation did not originate from this client.
// This prevents the client from invalidating its own cache after a SET it just performed.
void Cache::invalidate(const std::string& key, const std::string& senderID)
{
    if (!senderID.empty() && senderID == clientID)
        return;

    erase(key);
}
