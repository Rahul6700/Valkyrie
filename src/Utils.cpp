#include "Utils.hpp"
#include <chrono> // for time
#include <unistd.h>  // getpid
#include <string>                     

// helper fuctions general

namespace Utils
{

// generates random clientID used in conenction obj
    std::string generateID()
    {
        auto time = std::chrono::high_resolution_clock::now().time_since_epoch().count(); // gets the time, typically long long
        return std::to_string(getpid()) + "-" + std::to_string(time); // pid + '-' + time
    }

}

