#pragma once
#include <string>
#include <unordered_map>
#include <list>
#include <tuple>
#include <mutex>

class Cache {
  public:
    // Enum used by Connection to decide whether to enable pub/sub invalidation
    enum cacheReq
    {
        ENABLED,
        DISABLED
    };

    // Constructor: sets a fixed maximum number of entries for the cache
    // Default is 100 entries. Once full, the least recently used entry is evicted.
    Cache(size_t maxSize = 100);

    // Thread-safe LRU-aware lookup.
    // If the key is found, it is promoted to "most recently used" and {true, value} is returned.
    // If not found, returns {false, ""}.
    std::tuple<bool, std::string> get(const std::string& key);

    // Thread-safe insert or update.
    // If the key exists, its value is updated and it is promoted to MRU.
    // If it is a new key and the cache is at capacity, the LRU entry is evicted first.
    void put(const std::string& key, const std::string& value);

    // Thread-safe removal of a single entry by key.
    void erase(const std::string& key);

    // Invalidation used by the pub/sub listener.
    // If senderID matches this client's ID, the invalidation is ignored
    // (avoids self-invalidation from our own SET publishing back to us).
    void invalidate(const std::string& key, const std::string& senderID = "");

    // Stores this client's unique ID so we can skip self-invalidation.
    void setClientID(const std::string& id) { clientID = id; }

  private:
    size_t maxSize; // limit for cache entries
    std::list<std::string> lruList; // using a LL with front as MRU and back as LRU
    // we store key as string and value as a pair of <string,list iterator>
    // key string is our key val, value string is our value val and the list iterator points to the node of the LL so we can move it as needed
    std::unordered_map<std::string,std::pair<std::string, std::list<std::string>::iterator>> cacheMap;
    std::mutex cacheLock; 
    std::string clientID;  // used to skip self invalidation
};
