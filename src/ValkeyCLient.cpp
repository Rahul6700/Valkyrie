#include "ValkeyClient.hpp"
#include "Connection.hpp"
#include "RespProtocol.hpp"

// 'connection' is the Connection class obj to be used for all the ValkeyClient methods

// use the connect function defined in the Connection class
bool ValkeyClient::connect()
{
  return connection.connect();
}

// usingg the disconnect function used in the Connection class
void ValkeyClient::close()
{
  connection.close();
}

std::string ValkeyClient::set(const std::string& key, const std::string& value)
{
  if(!connection.isConnected()) return "Error: connection to server not active";

  if(key.empty() || value.empty()) return "Error: Key or value cannot be an empty string";

  std::string resp_encoded = RespProtocol::encode({"SET",key,value});
  if(!connection.sendData(resp_encoded)) return "Error: coneection to server not active";

  std::string reply = connection.receive();
  std::string decoded_reply = RespProtocol::decode(reply);
  
  if(decoded_reply != "OK") return "Error: " + decoded_reply; // adding the 'Error' tag to any non-OK respone
  
  return decoded_reply;
}

std::string ValkeyClient::del(const std::string& key)
{
  if(!connection.isConnected()) return "Error: connection to server not active";
  if(key.empty()) return "Error: Key value cannot be an empty string";

  std::string resp_encoded = RespProtocol::encode({"DEL",key});
  if(!connection.sendData(resp_encoded)) return "Error: Connection to server is not active";

  std::string reply = connection.receive();
  std::string decoded_reply = RespProtocol::decode(reply);

  return decoded_reply;
}

std::string ValkeyClient::get(const std::string& key)
{
  if(!connection.isConnected()) return "Error: connection to server not active";
  if(key.empty()) return "Error: Key value cannot be an empty string";

  std::string resp_encoded = RespProtocol::encode({"GET",key});
  if(!connection.sendData(resp_encoded)) return "Error: Connection to server is not active";

  std::string reply = connection.receive();
  std::string decoded_reply = RespProtocol::decode(reply);

  if(decoded_reply == "") return "(nil)";
  return decoded_reply;
}

std::string ValkeyClient::mset(const std::vector<std::pair<std::string,std::string>>& vec)
{
  if(!connection.isConnected()) return "Error: connection to server not active";
  if(vec.empty()) return "Error: Key value cannot be an empty string";

  std::vector<std::string> arr; // since the encode function expects a vector of strings (no pairs)
  arr.push_back("MSET");
  for(auto i : vec)
  {
    arr.push_back(i.first);
    arr.push_back(i.second);
  }
  std::string resp_encoded = RespProtocol::encode(arr);
  if(!connection.sendData(resp_encoded)) return "Error: Connection to server is not active";

  std::string reply = connection.receive();
  std::string decoded_reply = RespProtocol::decode(reply);
  
  if(decoded_reply == "") return "(nil)";
  return decoded_reply;
}

std::vector<std::string> ValkeyClient::mget(const std::vector<std::string>& vec)
{
  if(!connection.isConnected()) return {"Error: connection to server not active"};
  if(vec.empty()) return {"Error: Key value cannot be an empty string"};

  std::vector<std::string> new_vec = vec;
  new_vec.insert(new_vec.begin(), std::string("MGET")); // insert 'MGET' to the front of the array
  
  std::string resp_encoded = RespProtocol::encode(new_vec);
  if(!connection.sendData(resp_encoded)) return {"Error: Connection to server is not active"};
  
  std::string reply = connection.receive();
  std::vector<std::string> decoded_arr = RespProtocol::decodeArray(reply);

  return decoded_arr;
}

std::string ValkeyClient::expire(const std::string& key, int time)
{
  if(!connection.isConnected()) return "Error: connection to server not active";
  if(key == "") return "Error: Key value cannot be an empty string";

  std::string resp_encoded = RespProtocol::encode({"EXPIRE", key, std::to_string(time)});
  if(!connection.sendData(resp_encoded)) return "Error: Connection to server is not active";

  std::string reply = connection.receive();
  std::string decoded_reply = RespProtocol::decode(reply);

  return decoded_reply;
}
