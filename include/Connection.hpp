#pragma once //ensures the header is included only once per compilation
#include <string>
#include <netinet/in.h>
#include <thread>
#include "Cache.hpp"

class Connection {
  public:
    //constructor and destructor
    Connection(const std::string &host, int port, Cache::cacheReq cache);
    ~Connection();

    //methods
    bool connect(); //to establish a connection with the valkey server
    bool isConnected() const { return connected; } // method to track whether the connection is active
    bool isSubConnected() const { return subConnected; } // func to check if the sub Conn is active, returns the val of the isSubConnected Var
    bool sendData(const std::string& data); //to send data through tcp
    std::string receive(); //to recieve data through tcp
    void close(); //close the tcp connection
    
    //this function creates a tcp connection to the same port and host
    //this conenction is used for the pub/sub system
    bool connectSubscriber();
    Cache cache; // client-side fixed-size LRU cache
    std::thread subscriberThread; // background thread listening on pub/sub for invalidation
    // the pub/sub system sends messages in this format -> "message cache_updates <key>"
    int subsockfd; // socket fd for the 2nd tcp connection
    std::string clientID; // unique ID generated for every client, set in connections.cpp

    Cache& getCache() { return cache; }
    bool isCacheEnabled() const { return cacheMode == Cache::ENABLED; }

  private:
    int sockfd; //socket file descriptor
    std::string host; //the valkey server ip
    int port; // the valkey server port
    Cache::cacheReq cacheMode;
    bool connected = false; // initialised to false
    bool subConnected = false;
    std::string parseMessage(const std::string& msg);
};
