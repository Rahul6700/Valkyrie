#include <string>
#include <unordered_map>
#include <tuple>
#include <mutex>
#include "Cache.hpp"


// use cacheMap and cacheLock
class cache {
  public:

  private:
    // this is our lookup function
    // takes in the key str by ref
    // returns a tuple <bool,string>
    // if the kv pair is found in cache it return <true,value>
    // if not found it returns <false,"">
    std::tuple<bool found, std::string> lookup (const std::string& key)
    {
        cacheLock.lock();
        auto it = cacheMap.find(key);
        if(it != cacheMap.end()) // if found
        {
            return {true, it->second};
        }
        else return {false, ""};
        cacheLock.unlock();
    }
    
    // removes the kv pair taking in a key
    void remove (const std::string& key)
    {
        cacheLock.lock();
        cacheMap.remove(key);
        cacheLock.unlock();
    }

    std::bool insert (const std::string& key, const std::string& value)
    {
        cacheLock.lock();
        // logic to check if there exists space to insert a new kv pair
        
        cacheMap[key] = value;
        cacheLock.unlock();
        return true;
    }
}
