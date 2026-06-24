# Data Flow in Valkyrie Client

This document describes how data flows through the Valkyrie Client, from user initiation to interaction with the Valkey server and back, including the client-side caching mechanism.

## 1. Client Initialization and Connection

1.  **`main.cpp`**: The application starts, and a `ValkeyClient` object is instantiated:
    *   `ValkeyClient client("127.0.0.1", 6379, Cache::ENABLED);`
    *   This constructor in turn initializes a `Connection` object (`connection`) within the `ValkeyClient`, passing the host, port, and cache enablement status.
2.  **`client.connect()`**:
    *   Calls `Connection::connect()`.
    *   **Main Connection:** `Connection::connect()` creates a main TCP socket (`sockfd`) and establishes a connection to the Valkey server on the specified host and port.
    *   **Subscriber Connection (if caching enabled):** If `Cache::ENABLED`, `Connection::connectSubscriber()` is called.
        *   A separate TCP socket (`subsockfd`) is created and connected to the same Valkey server.
        *   A unique `clientID` is generated using `Utils::generateID()`.
        *   A `subscriberThread` is launched, which continuously listens on `subsockfd` for incoming messages (expected to be cache invalidation notifications).
        *   This thread uses a `std::promise` and `std::future` to signal its readiness to the main thread.

## 2. Data Operations (e.g., `SET` command)

Let's trace the data flow for a `SET` operation: `client.set("key", "value")`.

1.  **`ValkeyClient::set(key, value)`**:
    *   Performs initial checks for active connection and non-empty key/value.
    *   Constructs a command vector: `{"SET", key, value}`.
    *   **Encoding:** Calls `RespProtocol::encode({"SET", key, value})`.
        *   `RespProtocol::encode` converts the command vector into a RESP-formatted string (e.g., `*3\r\n$3\r\nSET\r\n$3\r\nkey\r\n$5\r\nvalue\r\n`).
        *   This RESP string is the `resp_encoded` data.
    *   **Sending Data:** Calls `connection.sendData(resp_encoded)`.
        *   `Connection::sendData` uses the main TCP socket (`sockfd`) to transmit the `resp_encoded` string to the Valkey server.

2.  **Valkey Server Processing (External to Client):**
    *   The Valkey server receives and parses the RESP command.
    *   It executes the `SET` operation, storing the `key` and `value`.
    *   The server then generates a RESP response (e.g., `+OK\r\n`).

3.  **Receiving Response:**
    *   `ValkeyClient::set` calls `connection.receive()`.
    *   `Connection::receive` reads raw data from the main TCP socket (`sockfd`) into a buffer.
    *   The raw server reply (e.g., `+OK\r\n`) is returned.
    *   **Decoding:** Calls `RespProtocol::decode(reply)`.
        *   `RespProtocol::decode` parses the raw RESP reply into a simple string (e.g., `"OK"`).
        *   This `decoded_reply` is then returned to the `ValkeyClient::set` method.

4.  **Cache Invalidation (Publishing):**
    *   If `decoded_reply` is `"OK"`, `ValkeyClient::set` sends a cache invalidation message:
        *   `std::string pubMessage = "PUBLISH cache_invalidation " + key + "\n";`
        *   `::send(connection.subsockfd, pubMessage.c_str(), pubMessage.size(), 0);`
        *   This message is sent over the *subscriber* TCP socket (`subsockfd`) to the Valkey server's pub/sub channel. The server will then broadcast this message to all subscribed clients.

## 3. Cache Invalidation Flow (via Pub/Sub)

1.  **Server Broadcast:** The Valkey server, upon receiving the `PUBLISH` command, broadcasts the message (e.g., `"message cache_updates <key>"`) to all clients subscribed to the `cache_invalidation` channel.
2.  **`subscriberThread` Reception:**
    *   The `subscriberThread` in the `Connection` class (running asynchronously) continuously calls `recv()` on its `subsockfd`.
    *   When a message is received, it's read into a buffer.
    *   `Connection::parseMessage()` extracts the `<key>` from the received message.
    *   **(Currently commented out):** The intention is to call `cache.invalidate(key)` to remove the stale data from the client's local `Cache`.

## 4. Data Retrieval (e.g., `GET` command)

Let's trace the data flow for a `GET` operation: `client.get("key")`.

1.  **`ValkeyClient::get(key)`**:
    *   Performs initial checks for active connection and non-empty key.
    *   **(Missing Cache Check):** Currently, the `GET` operation directly goes to the server. There is no `Cache::checkCache(key)` call to retrieve data from the client-side cache first. This is a point of technical debt.
    *   **Encoding:** Calls `RespProtocol::encode({"GET", key})`.
    *   **Sending Data:** Calls `connection.sendData(resp_encoded)`.
    *   **Receiving Response:** Calls `connection.receive()`.
    *   **Decoding:** Calls `RespProtocol::decode(reply)`.
    *   Returns the `decoded_reply`.

## Summary of Key Data Paths

*   **Client Command -> Server:** `ValkeyClient` -> `RespProtocol::encode` -> `Connection::sendData` (via `sockfd`) -> Valkey Server.
*   **Server Response -> Client:** Valkey Server -> `Connection::receive` (via `sockfd`) -> `RespProtocol::decode` -> `ValkeyClient`.
*   **Cache Invalidation (Publish):** `ValkeyClient` -> `::send` (via `subsockfd`) -> Valkey Server (pub/sub channel).
*   **Cache Invalidation (Subscribe):** Valkey Server -> `Connection::subscriberThread` (via `subsockfd`) -> `Connection::parseMessage` -> (intended `Cache::invalidate`).
