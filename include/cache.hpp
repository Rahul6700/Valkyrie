#include <string>
#include <unordered_map>
#include <tuple>

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
    // hi
    
  private:
    // this <str,str> map is what will act as the actual cache
    std::unordered_map<std::string,std::string> cacheMap;

    // this is our lookup function
    // takes in the key str by ref
    // returns a tuple <bool,string>
    // if the kv pair is found in cache it return <true,value>
    // if not found it returns <false,"">
    std::tuple<bool found, std::string> lookup (const std::string& key)
    {
        auto it = cacheMap.find(key);
        if(it != cacheMap.end()) // if found
        {
            return {true, it->second};
        }
        else return {false, ""};
    }
    
    // removes the kv pair taking in a key
    void remove (const std::string& key)
    {
        cacheMap.remove(key);
    }

    std::bool insert (const std::string& key, const std::string& value)
    {
        // logic to check if there exists space to insert a new kv pair
        
        cacheMap[key] = value;
        return true;
    }

}
