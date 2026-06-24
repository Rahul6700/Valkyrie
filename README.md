# Valkyrie

**C++20 client for Valkey / Redis** — communicates over TCP using the RESP protocol with a client-side LRU cache and pub/sub-based cache invalidation.

## Features

- **Full RESP Protocol** — encode commands and decode server replies (simple strings, bulk strings, integers, errors, arrays)
- **Key-Value Operations** — `SET`, `GET`, `DEL`, `MSET`, `MGET`, `EXPIRE`
- **Client-Side LRU Cache** — thread-safe, fixed-size, evicts least recently used entries
- **Pub/Sub Cache Invalidation** — broadcasts changes via a dedicated subscriber socket; skips self-invalidation using a unique client ID
- **Dual TCP Connections** — one for data commands, one for pub/sub subscriptions
- **No External Dependencies** — only the C++ standard library and POSIX sockets

## Architecture

| Layer | Component | Responsibility |
|-------|-----------|----------------|
| Application | `ValkeyClient` | Public API — orchestrates commands, cache, and connections |
| Network | `Connection` | Manages two TCP sockets and the background subscriber thread |
| Protocol | `RespProtocol` | Serializes / deserializes RESP-formatted data |
| Data | `Cache` | Thread-safe LRU cache with mutex-guarded access |
| Utility | `Utils` | Client ID generation (PID + high-res timestamp) |

<img width="2653" height="1546" alt="Screenshot" src="https://github.com/user-attachments/assets/02b1c970-cef8-4988-8fab-f2c5879a8520" />

1. `ValkeyClient` is constructed with a host, port, and cache mode
2. `connect()` creates a TCP socket for data commands
3. If caching is enabled, a second TCP socket opens and a `subscriberThread` starts listening for invalidation messages
4. A unique `clientID` is generated via `Utils::generateID()` for self-invalidation detection

## Data Flow

### SET Command

1. The command `["SET", "key", "value"]` is RESP-encoded via `RespProtocol::encode`
2. The encoded string is sent over the TCP socket
3. The server replies with `+OK`, decoded to `"OK"` by `RespProtocol::decode`
4. On success with caching enabled, the cache is updated immediately and a `PUBLISH cache_invalidation <key>` is sent over the subscriber socket

### GET Command with Cache Check

1. `ValkeyClient::get()` checks the local LRU cache first via `Cache::get(key)`
2. **Cache hit** — returns the cached value immediately (no network I/O)
3. **Cache miss** — queries the server, decodes the response, stores it in the cache, and returns

## Prerequisites

| Dependency | Details |
|------------|---------|
| **Compiler** | C++20 (GCC 10+, Clang 10+, MSVC 2022+) |
| **Build System** | CMake >= 3.16 |
| **Operating System** | Linux (POSIX sockets required) |
| **Runtime** | A running Valkey or Redis server |
| **Network** | TCP connectivity to the server |

No external C++ libraries are required.

## Build & Run

```bash
mkdir -p build && cd build
cmake ..
make
./valkyrie
```

The demo connects to `127.0.0.1:6379` (default Valkey/Redis port).

## Usage

```cpp
#include "ValkeyClient.hpp"
#include "Cache.hpp"
#include <iostream>

int main() {
    // Create a client with caching enabled
    ValkeyClient client("127.0.0.1", 6379, Cache::ENABLED);

    if (!client.connect()) {
        std::cerr << "Failed to connect\n";
        return 1;
    }

    // Basic SET / GET
    std::cout << client.set("name", "Valkyrie") << "\n"; // "OK"
    std::cout << client.get("name") << "\n";             // "Valkyrie"

    // Delete
    std::cout << client.del("name") << "\n";             // "1"

    // Multi-key operations
    client.mset({{"a", "1"}, {"b", "2"}, {"c", "3"}});

    std::vector<std::string> vals = client.mget({"a", "b", "c"});
    for (const auto& v : vals)
        std::cout << v << " ";                           // "1 2 3"

    // Expiration
    client.set("temp", "data");
    client.expire("temp", 10);

    // Disable caching
    ValkeyClient nocache("127.0.0.1", 6379, Cache::DISABLED);
    nocache.connect();
    std::cout << nocache.get("a") << "\n";

    client.close();
    return 0;
}
```

## API Reference

### ValkeyClient

| Method | Signature | Returns | Description |
|--------|-----------|---------|-------------|
| Constructor | `ValkeyClient(host, port, cacheMode)` | — | Stores connection parameters and cache preference |
| `connect` | `bool connect()` | `true` on success | Establishes TCP connections; spawns subscriber thread if caching enabled |
| `close` | `void close()` | — | Shuts down sockets and joins subscriber thread |
| `set` | `std::string set(key, value)` | `"OK"` or error | Stores a key-value pair; updates cache and publishes invalidation |
| `get` | `std::string get(key)` | Value or `"(nil)"` | Checks cache first; queries server on miss |
| `del` | `std::string del(key)` | Number of keys removed | Deletes a key from the server |
| `mset` | `std::string mset(pairs)` | `"OK"` or error | Sets multiple key-value pairs atomically |
| `mget` | `std::vector\<std::string\> mget(keys)` | Vector of values | Retrieves multiple keys in order |
| `expire` | `std::string expire(key, seconds)` | `"1"` on success | Sets a TTL on an existing key |

### Cache

| Method | Signature | Returns | Description |
|--------|-----------|---------|-------------|
| Constructor | `Cache(maxSize = 100)` | — | Creates an LRU cache with the given capacity |
| `get` | `std::tuple\<bool, std::string\> get(key)` | `{found, value}` | Thread-safe lookup; promotes key to MRU on hit |
| `put` | `void put(key, value)` | — | Inserts or updates; evicts LRU entry if at capacity |
| `erase` | `void erase(key)` | — | Removes a single entry |
| `invalidate` | `void invalidate(key, senderID = "")` | — | Erases entry if `senderID` differs from `clientID` |

## Cache Invalidation

Valkyrie uses pub/sub to keep client caches coherent:

1. On a successful `SET`, the client publishes `cache_invalidation <key>` to the server over the subscriber socket
2. The server broadcasts the message to all subscribed clients
3. Each client's `subscriberThread` receives the broadcast and calls `Cache::invalidate(key, senderID)`
4. If `senderID` matches the local `clientID`, the cache is **not** invalidated — it was already updated by the originating `SET` (avoids redundant work)
5. If IDs differ, the entry is erased, forcing the next `get()` to fetch fresh data

### LRU Eviction

The cache combines a `std::unordered_map` for O(1) lookup with a `std::list` for access-order tracking:

- **Front** of list = Most Recently Used (MRU)
- **Back** of list = Least Recently Used (LRU)
- At capacity, the LRU entry is evicted before inserting a new one
- Every `get()` or `put()` on an existing key promotes it to MRU

## Project Structure

```
Valkyrie/
├── CMakeLists.txt            # Build config (C++20, CMake >= 3.16)
├── main.cpp                  # Demo entry point
├── include/                  # Public headers
│   ├── ValkeyClient.hpp
│   ├── Connection.hpp
│   ├── RespProtocol.hpp
│   ├── Cache.hpp
│   └── Utils.hpp
├── src/                      # Implementation files
│   ├── ValkeyClient.cpp
│   ├── Connection.cpp
│   ├── RespProtocol.cpp
│   ├── Cache.cpp
│   └── Utils.cpp
├── ARCHITECTURE.md           # Architecture deep-dive
├── DATA_FLOW.md              # Data flow documentation
└── TODO.md                   # Known issues and technical debt
```

## Known Limitations

- **Fixed 1024-byte receive buffer** — large responses may be truncated
- **Hardcoded host/port** — must be set at compile time; no CLI or config file support yet
- **No automated tests yet** — To implement
