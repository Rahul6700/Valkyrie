#include <string>
#include "Connection.hpp"
#pragma once

class ValkeyClient {
public:
  //constructor and destructor
  // as soon as the ValkeyClient constructor is called a Connection obj is also initialized as part of the ValkeyClient class
  // when we call a ValkeyClient constructor we also need to pass the host ip and the port number
  // the host and port we pass with the constructor will bind to the Connection obj (the obj is called 'connection')
  // so now when we can use the Connection::connect method on this obj and when we do the host and port we defined will be used
  ValkeyClient(const std::string& host, int port) : connection(host,port){};
  //~ValkeyClient(); use default one

  //connection methods
  bool connect();
  void close();

  //set, get and del functions for interacting with the db
  std::string get(const std::string& key);
  std::string set(const std::string& key, const std::string& value);
  std::string del(const std::string& key);

private:
  Connection connection; // a pvt connection obj to be used for all ValkeyClient activities
}; 
