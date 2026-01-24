#include <string>
#include <unordered_map>
#include <tuple>
#include <mutex>

// TODO:
// make the cache system
// we need the following:
// 1. an unordered_map of limited size
// 2. a function for lookup
// 3. a function to write to cache
// 4. a function that invalidates cache
// 5. a mutex lock for thread safety
// 6. optionally TTL and LRU eviction


class Cache {
  public:
    // this enum will be used with the connect constructor to decide whether we wanna use the cache or not
    enum cacheReq
    {
        ENABLED,
        DISABLED
    };

  private:
    // this <str,str> map is what will act as the actual cache
    std::unordered_map<std::string,std::string> cacheMap;
    std::mutex cacheLock;

    // this is our lookup function
    // takes in the key str by ref
    // returns a tuple <bool,string>
    // if the kv pair is found in cache it return <true,value>
    // if not found it returns <false,"">
    std::tuple<bool, std::string> lookup (const std::string& key);

    // removes the kv pair taking in a key
    void erase (const std::string& key);

    bool insert (const std::string& key, const std::string& value);

};
