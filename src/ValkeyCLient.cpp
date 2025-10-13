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


