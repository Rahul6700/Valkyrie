#pragma once //ensures the header is included only once per compilation
#include <string>
#include <netinet/in.h>

class Connection {
  public:
    //constructor and destructor
    Connection(const std::string &host, int port);
    ~Connection();

    //methods
    bool connect(); //to establish a connection with the valkey server
    bool isConnected() const { return connected; } // method to track whether the connection is active
    bool sendData(const std::string& data); //to send data through tcp
    std::string receive(); //to recieve data through tcp
    void close(); //close the tcp connection
                  
  private:
    int sockfd; //socket file descriptor
    std::string host; //the valkey server ip
    int port; // the valkey server port
    bool connected = false; // initialised to false
};
