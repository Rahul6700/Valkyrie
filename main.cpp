#include <iostream>
#include <string>
#include <vector>
#include "Connection.hpp"
#include "RespProtocol.hpp"
#include "ValkeyClient.hpp"

int main()
{
  //std::cout << "hello world" << std::endl;
  //std::vector<std::string> vec = {"SET","hello","world"};
  //std::string hi = "-ERR value is not an integer or out of range\r\n";
  //std::string ans = RespProtocol::decode(hi);
  //std::cout << ans << std::endl;
  
  //default port for valkey -> 6379 and 127.0.0.1 cuz server is running locally rn
  // Valkey obj("127.0.0.1",6379);
  // if(obj.connect()) std::cout << "connected from main" << std::endl;
  // else std::cout << "error connection from main" << std::endl;
  // obj.close();
  // return 0;
  ValkeyClient client("127.0.0.1",6379);
  client.connect();
  //std::cout << client.set("yo","hehe") << std::endl;
  std::cout << client.del("yo") << std::endl;
}
