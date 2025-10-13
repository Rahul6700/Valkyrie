#include "ValkeyClient.hpp"
#include "Connection.hpp"

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

bool ValkeyClient::set(const std::string& key, const std::string& value)
{
  if(!connection.connected)
  {
    std::cout << "connection to server not active" std::endl;
    return false;
  }


}



