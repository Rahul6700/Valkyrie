#include <string>
#pragma once

class ValkeyClient {
public:
  //constructor and destructor
  ValkeyClient();
  ~ValkeyClient();

  //connection methods
  bool connect();
  bool disconnect();

  //set, get and del functions for interacting with the db
  std::string get(const std::string key);
  bool set(const std::string key, const std::string value);
  bool del(const std::string key);
}; 
