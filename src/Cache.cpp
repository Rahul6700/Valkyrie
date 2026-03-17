#include <string>
#include <unordered_map>
#include <tuple>
#include <mutex>
#include "Cache.hpp"


// use cacheMap and cacheLock

    // this is our lookup function
    // takes in the key str by ref
    // returns a tuple <bool,string>
    // if the kv pair is found in cache it return <true,value>
    // if not found it returns <false,"">
    std::tuple<bool, std::string> Cache::lookup (const std::string& key)
    {
        std::lock_guard<std::mutex> lock(cacheLock);

        auto it = cacheMap.find(key);
        if(it != cacheMap.end()) // if found
        {
            return {true, it->second};
        }
        return {false, ""};
    }
    
    // removes the kv pair taking in a key
    void Cache::erase (const std::string& key)
    {
        std::lock_guard<std::mutex> lock(cacheLock);
        cacheMap.erase(key);
    }

    bool Cache::insert (const std::string& key, const std::string& value)
    {
        std::lock_guard<std::mutex> lock(cacheLock);

        // logic to check if there exists space to insert a new kv pair
        cacheMap[key] = value;
        return true;
    }

// std::string generateID()
// {
//     auto time = std::chrono::high_resolution_clock::now().time_since_epoch().count(); // gets the time, typically long long
//     return std::to_string(getpid()) + "-" + std::to_string(time); // pid + '-' + time
// }

    std::tuple<bool, std::string> Cache::checkCache(const std::string& key)
    {
        auto [found, val] = lookup(key);
        
        if(!found) return {false, ""};

        return {true,val};
    }

