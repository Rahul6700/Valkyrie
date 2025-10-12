#include <string>
#include <vector>
#pragma once

namespace RespProtocol {
    //encoding and decoding function for the protocol
    //encode takes and array like this ["SET", "rahul", "green"] -> [operation, key, value] and converts it to RESP format
    //decode will take the valkey server response and convert it to a simple reply
    std::string encode(const std::vector<std::string>& arr);
    std::string decode(const std::string& reply);
};

