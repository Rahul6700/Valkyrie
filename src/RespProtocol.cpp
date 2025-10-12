#include "RespProtocol.hpp"
#include <vector>
#include <string>

// to encode the ["SET", "key", "value"] into RESP format
// its encoded to -> *3\r\n$3\r\nSET\r\n$3\r\nkey\r\n$5\r\nvalue\r\n
// *3\r\n → array of 3 elements
// $3\r\nSET\r\n → first string "SET" (length 3 so $3)
// $3\r\nkey\r\n → second string "key"
// $5\r\nvalue\r\n → third string "value" (len = 5, so $5)

namespace RespProtocol {

//encode function taking in an str vector as input
std::string encode(const std::vector<std::string>& arr)
{
  std::string encodedString;

  encodedString += "*" + std::to_string(arr.size()) + "\r\n";

  for (const auto& val : arr) 
  {
      encodedString += "$" + std::to_string(val.size()) + "\r\n"; 
      encodedString += val + "\r\n";                             
  }

  return encodedString;
}

// RESP decode func
std::string decode(const std::string& reply) {
    if (reply.empty()) return "";

    char prefix = reply[0];

    if (prefix == '+') {
        // Simple string: +OK\r\n
        return reply.substr(1, reply.size() - 3); 
    }
    else if (prefix == '-') {
        // Error: -ERR something\r\n
        return "Error: " + reply.substr(1, reply.size() - 3);
    }
    else if (prefix == ':') {
        // Integer: :1000\r\n
        return reply.substr(1, reply.size() - 3);
    }
    else if (prefix == '$') {
        // Bulk string: $<len>\r\n<data>\r\n
        size_t pos = reply.find("\r\n");
        if (pos == std::string::npos) return "";
        int len = std::stoi(reply.substr(1, pos - 1));
        if (len == -1) return "";  // Null bulk string
        return reply.substr(pos + 2, len);
    }
    else if (prefix == '*') {
        // Array: *<num>\r\n ... flatten to space-separated string
        std::vector<std::string> elements;
        size_t pos = 1;

        while (pos < reply.size()) {
            size_t crlf = reply.find("\r\n", pos);
            if (crlf == std::string::npos) break;

            std::string line = reply.substr(pos, crlf - pos);

            if (!line.empty() && line[0] == '$') {
                int len = std::stoi(line.substr(1));
                if (len > 0) {
                    std::string data = reply.substr(crlf + 2, len);
                    elements.push_back(data);
                    pos = crlf + 2 + len + 2; // move past $len\r\n<data>\r\n
                } else {
                    pos = crlf + 2; // skip empty bulk string
                }
            } else {
                pos = crlf + 2;
            }
        }
        // Flatten elements into single string
        std::string result;
        for (size_t i = 0; i < elements.size(); ++i) {
            result += elements[i];
            if (i != elements.size() - 1) result += " ";
        }

        return result;
    }

    return ""; // in case of some unknowb type
}

}; // closing the namespace
