#include "Connection.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>
#include <string>
#include <future>

// not importing 'namespace std' since we'll have a naming conflict, we have a user defined 'close' func and a 'close' system call

//defining all the Connection class functions

//the constructor
Connection::Connection(const std::string& h, int p)
{
  this->host = h;
  this->port = p;
  sockfd = -1;
}

// this function takes the TCP output from the pub/sub channel and extracts the key from it and returns it
std::string Connection::parseMessage(const std::string& msg) {
    //for the message format -> "message cache_updates <key>"
    size_t pos = msg.rfind(' ');
    if (pos != std::string::npos)
        return msg.substr(pos + 1);
    return "";
}

bool Connection::connectSubscriber()
{
  subsockfd = socket(AF_INET, SOCK_STREAM, 0);
  if(subsockfd < 0)
  {
    std::cout << "error creating socker in Connection.connectSubscriber";
    return false;
  }

  struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET; // TCP protocol
    serverAddress.sin_port = htons(port); // port number

  //we currently have out host as a string, like "127.0.01". we need to convert it to binary cuz the system call accepts a binary input
  // we use the inet_pton function to convert IPV4/6 addresses from string to binary -> return 1 on success, 0 if not a valid IP addr str, and -ve if some other error
  if (inet_pton(AF_INET, host.c_str(), &serverAddress.sin_addr) <= 0) {
    std::cout << "invalid address at pub/sub" << std::endl;
    ::close(subsockfd);
    return false;
  }

  // now we connect to the server using the 'connect' system call (not the connect function we made)
  // to tell the compiler to use the global system call and not our conenct function, we say ::connect()
  // we give parameters as the socket fd, a ptr poiting to our host and port dets structure and the size of the struct obj
  if (::connect(subsockfd, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
    std::cerr << "Connection failed at pub/sub" << std::endl;
    ::close(subsockfd);
    return false;
  }

// this part creates a new thread called 'subscriberThread' which is then detached
// the thread runs seperately on the background
// a while loop is running, cuz of which the thread is always listening for info from the server
// since the recv() system call which we use to recieve data from the server is blocking, e.i, if the recv() is running, the entire program is paused till it finishes running
// we dont want that, so we create a seperate thread and let it run in parallal


// we'll use a future and promise to solve the subscriberThread - main thread wait issue
// the promise object is attached to the subscriber thread and the future to the main thread
// the promise has a .set_value() function that will set a condiitonal val once its created
// the future object has a .get() methood that keeps waiting for the promise to set a value
// so once the thread is created the promise sets a true and till then the main waits
// once the val is set to true, the .get() recieves the val and the main thread continues

  std::promise<void> promiseObj;
  std::future<void> futureObj = promiseObj.get_future();
subscriberThread = std::thread([this, promise = std::move(promiseObj)]() mutable
{

  std::cout << "subscriber thread creating..." << std::endl;

  promise.set_value(); // basically signalling im ready
                      
  std::cout << "successfully created subscriberThread" << std::endl;

  char buffer[1024];
      while (true)
      {
        int bytes = recv(subsockfd, buffer, sizeof(buffer)-1, 0);
        if (bytes <= 0) break;
        buffer[bytes] = '\0';
        std::string key = Connection::parseMessage(std::string(buffer));
        //cache.invalidate(key); // to implement the invalidate function which invalidates the cache for this key
      }
    });
      futureObj.get(); // listening for the promise's value 
  std::cout << "successfully set up pub/sub tcp connection" << std::endl;
 // subscriberThread.join(); // this should wait for the thread to finish being created before the function retuning
  return true;
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

  if(!Connection::connectSubscriber()) return false;

  //std::cout << "connection successful to server" << std::endl;
  connected = true;
  return true;
}

//function closing the connection
void Connection::close()
{
  //check if socket is open first
  if(subsockfd >= 0)
  {
    shutdown(subsockfd, SHUT_RDWR); // immediately stops the blocking recv()
    ::close(subsockfd); //this is global system call, not our func
    subsockfd = -1;
    connected = false;
    //std::cout << "connection closed" << std::endl;
  }
  if(subscriberThread.joinable())
  {
    subscriberThread.join(); // put the thread to sleep after its ready
    std::cout << "successfully killed thread" << std::endl;
  }
  if(sockfd >= 0)
  {
    shutdown(sockfd, SHUT_RDWR);
    ::close(sockfd);
    sockfd = -1;
    std::cout << "closed pub/sub connection" << std::endl;
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
