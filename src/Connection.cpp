#include "Connection.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>
#include <string>

// not importing 'namespace std' since we'll have a naming conflict, we have a user defined 'close' func and a 'close' system call

//defining all the Connection class functions

//the constructor
Connection::Connection(const std::string& h, int p)
{
  this->host = h;
  this->port = p;
  sockfd = -1;
}

bool Connection::connect()
{
  //creating socket
  sockfd = socket(AF_INET, SOCK_STREAM, 0); // AF_INET -> IPV4 and SOCK_STREAM -> TCP
  if(sockfd < 0)
  {
    std::cout << "error creating socket in Connection.connect" << std::endl;
    return false;
  }

  // we need to specify server host and port dets
  // we'll store all these details in a struct called sockaddr_in
  struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET; // TCP protocol
    serverAddress.sin_port = htons(port); // port number

  //we currently have out host as a string, like "127.0.01". we need to convert it to binary cuz the system call accepts a binary input
  // we use the inet_pton function to convert IPV4/6 addresses from string to binary -> return 1 on success, 0 if not a valid IP addr str, and -ve if some other error
  if (inet_pton(AF_INET, host.c_str(), &serverAddress.sin_addr) <= 0) {
    std::cout << "invalid address" << std::endl;
    ::close(sockfd);
    return false;
  }

  // now we connect to the server using the 'connect' system call (not the connect function we made)
  // to tell the compiler to use the global system call and not our conenct function, we say ::connect()
  // we give parameters as the socket fd, a ptr poiting to our host and port dets structure and the size of the struct obj
  if (::connect(sockfd, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
    std::cerr << "Connection failed" << std::endl;
    ::close(sockfd);
    return false;
  }

  //std::cout << "connection successful to server" << std::endl;
  connected = true;
  return true;
}

//function closing the connection
void Connection::close()
{
  //check if socket is open first
  if(sockfd >= 0)
  {
    ::close(sockfd); //this is global system call, not our func
    sockfd = -1;
    connected = false;
    //std::cout << "connection closed" << std::endl;
  }
}

//destructor
Connection::~Connection()
{
  close();
}

bool Connection::sendData(const std::string& data)
{
    if (!connected) {
        std::cout << "error: not connected to the server" << std::endl;
        return false;
    }
    // send all the data
    size_t totalSent = 0;
    size_t dataLen = data.size();
    const char* buffer = data.c_str(); // converting our data into a C styled char array

    while (totalSent < dataLen) { //while loop till all the data is not sent
        ssize_t sent = send(sockfd, buffer + totalSent, dataLen - totalSent, 0); // calling the send system call
        if (sent < 0) {
            std::cout << "error sending data to server" << std::endl;
            return false;
        }
        totalSent += sent;
    }
    return true; //if all the data is sent through successfully
}

std::string Connection::receive()
{
  char buffer[1024]; //we'll store the received data in a buffer
  ssize_t received_data = recv(sockfd,buffer,sizeof(buffer),0); // use the 'recv' sys call to recieve data
  if(received_data <= 0) return "";
  else return std::string(buffer, received_data);          
}
