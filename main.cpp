#include <iostream>
#include <string>
#include <vector>
#include "Connection.hpp"
#include "RespProtocol.hpp"
#include "ValkeyClient.hpp"
#include "Cache.hpp"

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
  ValkeyClient client("127.0.0.1",6379,Cache::ENABLED);
  client.connect();
  // std::cout << client.set("yo","hehe") << std::endl;
  // //std::cout << client.del("yo") << std::endl;
  // std::cout << client.get("yo") << std::endl;
  // std::cout << client.mset({{"hi","there"},{"rahul","senthil"},{"timmy","mummy"}}) << std::endl;
  // std::vector<std::string> arr;
  // arr = client.mget({"hi","rahul","timmy","yo"});
  // for(const auto i : arr)
  // {
  //   std::cout << i << std::endl;
  // }
  //std::cout << client.set("rahul","hi") << std::endl;
  //std::cout << client.get("rahul") << std::endl;
  //if(Cache::insert("water", "bottle")) std::cout << "cache inserted" << std::endl;
  //auto [a,b] = Cache::checkCache("water");
  //if(!a) std::cout << "no cache found" << std::endl;
  //else std::cout << "cache found and val is " << b << std::endl;
  client.close();
}


