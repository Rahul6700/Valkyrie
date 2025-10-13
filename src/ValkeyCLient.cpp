#include "ValkeyClient.hpp"
#include "Connection.hpp"

// use the connect function defined in the Connection class
ValkeyClient::connect()
{
  Connection obj;
  if(obj.connect()) return true;
  else return false;
}

// usingg the disconnect function used in the Connection class
ValkeyClient::close()
{
  Connection obj;
  obj.close();
}

bool ValkeyClient::set(const std::string key, const std::string value)
{
  
}



