# R-Type Network Protocol Specification

**Version:** 1.0  
**Date:** December 14, 2025  
**Status:** Implementation Documentation  
**Authors:** R-Type Development Team

---

## Table of Contents

1. [Introduction](#1-introduction)
2. [Protocol Overview](#2-protocol-overview)
3. [Transport Layer](#3-transport-layer)
4. [Message Format](#4-message-format)
5. [Message Types](#5-message-types)
6. [Connection Flow](#6-connection-flow)
7. [Game State Synchronization](#7-game-state-synchronization)
8. [Implementation Details](#8-implementation-details)
9. [Performance Characteristics](#9-performance-characteristics)

---

## 1. Introduction

### 1.1 Purpose

This document describes the R-Type Network Protocol (RTNP), a UDP-based protocol for real-time multiplayer communication in the R-Type game. The protocol uses JSON for message serialization and implements a simple client-server architecture with server-authoritative game state.

### 1.2 Requirements Language

The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL NOT", "SHOULD", "SHOULD NOT", "RECOMMENDED", "MAY", and "OPTIONAL" in this document are to be interpreted as described in [RFC 2119](https://www.rfc-editor.org/rfc/rfc2119).

### 1.3 Terminology

- **Client**: A game instance that sends input and receives state updates
- **Server**: The authoritative instance that processes input and broadcasts game state
- **Entity**: A game object identified by a unique 32-bit unsigned integer (networkId)
- **Component**: A data structure attached to an entity (Transform, Sprite, Health, etc.)
- **Snapshot**: A complete representation of all networked entities at a given moment
- **Client ID**: Server-assigned unique identifier for each connected client

---

## 2. Protocol Overview

### 2.1 Architecture

RTNP follows a strict client-server architecture:

- **Server** is the authoritative source for all game state
- **Clients** send input commands at ~60 Hz
- **Server** broadcasts full snapshots to all clients at ~60 Hz
- No reliability layer - all messages are best-effort UDP
- No client-side prediction - clients render exactly what server sends

### 2.2 Design Philosophy

1. **Simplicity**: JSON messages over raw UDP
2. **Server Authority**: All game logic runs on server
3. **Best Effort**: No ACK, no retransmission, no ordering guarantees
4. **Full State**: Server sends complete state snapshots (no delta compression)
5. **Visual Authority**: Server dictates all visual properties (sprites, sizes)

### 2.3 Protocol Stack

```
┌─────────────────────────────────┐
│   R-Type Game Logic (ECS)       │
├─────────────────────────────────┤
│   JSON Messages                 │
├─────────────────────────────────┤
│   Cap'n Proto (minimal wrapper) │
├─────────────────────────────────┤
│   UDP (Raw datagrams)           │
├─────────────────────────────────┤
│   IP                            │
└─────────────────────────────────┘
```

**Note**: Cap'n Proto is used only as a thin serialization wrapper around JSON strings. The actual message structure is pure JSON.

---

## 3. Transport Layer

### 3.1 UDP Transport

RTNP operates over UDP (User Datagram Protocol) as specified in [RFC 768](https://www.rfc-editor.org/rfc/rfc768).

**Characteristics**:
- No connection establishment (connectionless)
- No reliability guarantees
- No packet ordering
- No flow control
- Minimal overhead

### 3.2 Port Allocation

- **Default Server Port**: 4242 (UDP)
- **Client Ports**: Ephemeral (OS-assigned)

### 3.3 Packet Size

- **Maximum Packet Size**: 8192 bytes (BUFFER_SIZE constant)
- **Typical Snapshot Size**: 500-2000 bytes (depends on entity count)
- **Input Message Size**: ~150 bytes

The server sets an 8MB receive buffer to handle burst traffic:
```cpp
static constexpr int MEGABYTE = 1024 * 1024;
static constexpr int BUFFER_SIZE_MB = 8;
asio::socket_base::receive_buffer_size option(MEGABYTE * BUFFER_SIZE_MB);
```

---

## 4. Message Format

### 4.1 Serialization

All messages are JSON objects serialized to UTF-8 strings, then wrapped in a Cap'n Proto message for transmission:

```capnp
@0xbf5147b1f1e3c5d2;

struct NetworkMessage {
  messageType @0 :Text;  # Contains the entire JSON string
}
```

**Important**: The `messageType` field contains the **entire JSON message** as a string, not just the type. The field name is misleading but kept for compatibility.

### 4.2 Message Structure

Every JSON message has a `"type"` field that identifies the message purpose:

```json
{
  "type": "message_type_name",
  // ... additional fields specific to message type
}
```

---

## 5. Message Types

### 5.1 Message Type Overview

| Type | Direction | Rate | Description |
|------|-----------|------|-------------|
| `assign_id` | S→C | Once per connection | Server assigns client ID |
| `player_input` | C→S | ~60 Hz | Client input state |
| `snapshot` | S→C | ~60 Hz | Full game state |

### 5.2 assign_id

**Direction**: Server → Client  
**Sent**: Once when client first sends data to server  
**Purpose**: Inform client of its server-assigned ID

**Format**:
```json
{
  "type": "assign_id",
  "client_id": 0
}
```

**Fields**:
- `client_id` (number): Unique identifier assigned by server (starts at 0, increments)

**Example**:
```json
{
  "type": "assign_id",
  "client_id": 2
}
```

### 5.3 player_input

**Direction**: Client → Server  
**Rate**: Every 16ms (~60 Hz)  
**Purpose**: Send keyboard input state to server

**Format**:
```json
{
  "type": "player_input",
  "entity_id": 0,
  "input": {
    "up": false,
    "down": false,
    "left": false,
    "right": false,
    "shoot": false
  }
}
```

**Fields**:
- `entity_id` (number): Client's assigned ID (received from `assign_id`)
- `input` (object): Boolean flags for each input
  - `up` (boolean): W or Up Arrow pressed
  - `down` (boolean): S or Down Arrow pressed
  - `left` (boolean): A or Left Arrow pressed
  - `right` (boolean): D or Right Arrow pressed
  - `shoot` (boolean): Space pressed

**Example**:
```json
{
  "type": "player_input",
  "entity_id": 1,
  "input": {
    "up": false,
    "down": false,
    "left": true,
    "right": false,
    "shoot": true
  }
}
```

**Optimization**: Client only logs when input changes or every 500ms to reduce console spam.

### 5.4 snapshot

**Direction**: Server → Clients (broadcast)  
**Rate**: Every frame (~60 Hz)  
**Purpose**: Synchronize complete game state to all clients

**Format**:
```json
{
  "type": "snapshot",
  "entities": [
    {
      "id": 123,
      "transform": {
        "x": 100.5,
        "y": 200.3,
        "rotation": 0.0,
        "scale": 1.0
      },
      "collider": {
        "w": 32.0,
        "h": 32.0
      },
      "sprite": {
        "spriteId": 0,
        "width": 140,
        "height": 60
      },
      "health": {
        "hp": 80,
        "maxHp": 100
      },
      "score": {
        "points": 1500
      }
    }
  ],
  "destroyed": [42, 56]
}
```

**Top-Level Fields**:
- `entities` (array): List of all networked entities
- `destroyed` (array, optional): Network IDs of entities destroyed since last snapshot

**Entity Fields** (all optional except `id` and `transform`):
- `id` (number): Unique network identifier for the entity
- `transform` (object): **REQUIRED** - Spatial properties
  - `x` (number): World X position
  - `y` (number): World Y position  
  - `rotation` (number): Rotation in radians
  - `scale` (number): Uniform scale factor
- `collider` (object): Collision box
  - `w` (number): Width in pixels
  - `h` (number): Height in pixels
- `sprite` (object): Visual appearance (server-authoritative)
  - `spriteId` (number): Enum value identifying sprite type
    - `0`: PLAYER_SHIP
    - `1`: ENEMY_SHIP
    - `2`: PROJECTILE
    - `3`: POWER_UP
    - `4`: OBSTACLE
    - `5`: EXPLOSION
  - `width` (number): Sprite width in pixels
  - `height` (number): Sprite height in pixels
- `health` (object): Entity health (for HUD display)
  - `hp` (number): Current health points
  - `maxHp` (number): Maximum health points
- `score` (object): Player score (for HUD display)
  - `points` (number): Current score value

**Entity Lifecycle**:
1. Entity appears in `entities` array when created on server
2. Entity updates sent every frame while alive
3. Entity ID appears in `destroyed` array when killed
4. Client removes entity from local world when seeing ID in `destroyed`

**Example - Minimal Entity**:
```json
{
  "id": 5,
  "transform": {
    "x": 400.0,
    "y": 300.0,
    "rotation": 1.57,
    "scale": 1.0
  }
}
```

**Example - Full Player Entity**:
```json
{
  "id": 0,
  "transform": {
    "x": 100.0,
    "y": 300.0,
    "rotation": 0.0,
    "scale": 1.0
  },
  "collider": {
    "w": 32.0,
    "h": 32.0
  },
  "sprite": {
    "spriteId": 0,
    "width": 140,
    "height": 60
  },
  "health": {
    "hp": 100,
    "maxHp": 100
  },
  "score": {
    "points": 0
  }
}
```

---

## 6. Connection Flow

### 6.1 Connection Establishment

No explicit handshake. Connection is established implicitly:

```
Client                                Server
  |                                      |
  |  player_input (first packet)         |
  |------------------------------------->|
  |                                      |
  |  (Server creates client entry)       |
  |  (Server spawns player entity)       |
  |                                      |
  |         assign_id                    |
  |<-------------------------------------|
  |                                      |
  |  (Client stores client_id)           |
  |                                      |
  |  player_input (with client_id)       |
  |------------------------------------->|
  |                                      |
  |         snapshot (broadcast)         |
  |<-------------------------------------|
  |                                      |
```

**Steps**:
1. Client starts sending `player_input` messages to server (entity_id initially 0)
2. Server receives first packet from unknown endpoint
3. Server assigns new client ID (auto-incremented counter)
4. Server spawns player entity in game world
5. Server sends `assign_id` message to client
6. Client updates its entity_id for future input messages
7. Client receives snapshots and renders game state

### 6.2 No Heartbeat Mechanism

The protocol does not implement heartbeats or timeout detection. Clients that stop sending input simply become inactive. The server does not track connection state beyond the initial ID assignment.

### 6.3 Disconnection

There is no explicit disconnect message. Clients that leave simply stop sending packets. The server does not detect or handle disconnections - dead clients remain in the client list indefinitely.

---

## 7. Game State Synchronization

### 7.1 Server Authority Model

- **Server** runs the complete game simulation
- **Server** processes all input and determines outcomes
- **Server** assigns all visual properties (sprite IDs, sizes)
- **Clients** are "dumb terminals" that render what they receive

### 7.2 Snapshot Broadcasting

The server broadcasts a complete snapshot to all connected clients every frame:

**Frequency**: ~16ms (60 FPS)  
**Content**: All entities with `Networked` component  
**Method**: Full state (not delta-compressed)

**Server Implementation** (NetworkSendSystem):
```cpp
void NetworkSendSystem::update(ecs::World &world, float deltaTime) {
  m_timeSinceLastSend += deltaTime;
  if (m_timeSinceLastSend >= SEND_INTERVAL) {  // 16ms
    // Build JSON snapshot of all networked entities
    // Broadcast to all connected clients
    m_timeSinceLastSend = 0.0f;
  }
}
```

### 7.3 Client Rendering

Clients apply received state directly to their local ECS world:

**Client Implementation** (ClientNetworkReceiveSystem):
```cpp
void handleSnapshot(ecs::World &world, const nlohmann::json &json) {
  // For each entity in snapshot:
  //   1. Find or create local entity
  //   2. Update transform from JSON
  //   3. Add/update components (sprite, health, score)
  
  // For each destroyed entity:
  //   1. Find local entity by networkId
  //   2. Destroy local entity
  //   3. Remove from networkId mapping
}
```

**No Prediction**: Clients do NOT predict future positions. They render exactly what the server sent.

**No Interpolation**: Clients do NOT smooth between snapshots. Each snapshot is rendered as-is.

### 7.4 Entity Lifecycle

**Creation**:
1. Server creates entity in its ECS world
2. Server adds `Networked` component with unique ID
3. Entity appears in next snapshot's `entities` array
4. Client receives snapshot, creates local entity copy
5. Client maps networkId → local entity ID

**Update**:
1. Server modifies entity components (position, health, etc.)
2. Updated values sent in next snapshot
3. Client receives snapshot, updates local entity

**Destruction**:
1. Server destroys entity (via DeathSystem, collision, etc.)
2. Server tracks destroyed networkIds
3. Destroyed IDs sent in snapshot's `destroyed` array
4. Client destroys local entity copy
5. Client removes from networkId mapping

---

## 8. Implementation Details

### 8.1 Technology Stack

- **Language**: C++20
- **Network Library**: ASIO (standalone, header-only)
- **Serialization**: nlohmann/json
- **Message Wrapper**: Cap'n Proto (minimal usage)
- **Architecture**: Entity Component System (ECS)

### 8.2 Threading Model

**Server**:
- Multi-threaded I/O with ASIO strand for thread safety
- Thread pool size = `std::thread::hardware_concurrency()`
- Game logic runs on main thread
- Network I/O runs on thread pool

**Client**:
- Single-threaded (game loop on main thread)
- Network I/O handled synchronously

### 8.3 Server Components

**AsioServer**:
```cpp
class AsioServer : public ANetworkManager {
  // UDP socket listening on port 4242
  // Thread pool for async I/O
  // Client tracking: map<clientId, endpoint>
  // Incoming message queue (thread-safe)
};
```

**NetworkSendSystem** (Server):
```cpp
class NetworkSendSystem : public ecs::ISystem {
  // Runs every frame
  // Queries all entities with Networked component
  // Builds JSON snapshot
  // Broadcasts to all clients
};
```

**NetworkReceiveSystem** (Server):
```cpp
// Server does NOT have a receive system
// Input processing is handled by InputProcessingSystem
// which reads from the server's message queue
```

### 8.4 Client Components

**AsioClient**:
```cpp
class AsioClient : public ANetworkManager {
  // UDP socket connected to server
  // Send: player_input messages
  // Receive: assign_id and snapshot messages
};
```

**NetworkSendSystem** (Client):
```cpp
class NetworkSendSystem : public ecs::ISystem {
  // Runs every 16ms
  // Reads local Input component
  // Sends player_input JSON to server
};
```

**ClientNetworkReceiveSystem**:
```cpp
class ClientNetworkReceiveSystem : public ecs::ISystem {
  // Runs every frame
  // Polls incoming messages
  // Parses JSON
  // Updates local ECS world
  // Tracks networkId → entity mapping
};
```

### 8.5 Sprite System (Server-Driven)

The server dictates all visual properties. Clients NEVER infer sprite types from game logic:

**Server** (Entity Creation):
```cpp
ecs::Sprite sprite;
sprite.spriteId = ecs::SpriteId::PLAYER_SHIP;  // Enum value
sprite.width = 140;
sprite.height = 60;
world->addComponent(player, sprite);
```

**Snapshot** (Transmission):
```json
{
  "sprite": {
    "spriteId": 0,
    "width": 140,
    "height": 60
  }
}
```

**Client** (Rendering):
```cpp
// Receive sprite data from server
ecs::Sprite sprite = ecs::Sprite::fromJson(spriteJson);
// Apply to entity
world.addComponent(entity, sprite);
// Renderer reads sprite component and draws correct texture
```

This ensures visual consistency and allows server-side gameplay experiments without client updates.

---

## 9. Performance Characteristics

### 9.1 Bandwidth Usage

**Per-Client Estimates** (1 client, 10 entities):
- **Upstream (C→S)**: ~150 bytes/packet × 60 Hz = **9 KB/s**
- **Downstream (S→C)**: ~1500 bytes/packet × 60 Hz = **90 KB/s**

**Scaling** (10 clients, 50 entities):
- **Server Upstream**: 150 bytes × 10 clients × 60 Hz = **90 KB/s**
- **Server Downstream**: 5000 bytes × 10 clients × 60 Hz = **3 MB/s**

### 9.2 Latency

**Typical Latencies**:
- LAN: 1-5ms
- Local Internet: 10-30ms
- Regional Internet: 30-80ms

**Perceived Lag**: Actual RTT + 1 frame (no prediction)
- Example: 30ms RTT → player sees result 30ms + 16ms = **46ms** after input

### 9.3 Packet Loss Handling

The protocol has NO packet loss mitigation:
- Lost input → Server uses previous input state
- Lost snapshot → Client renders stale state until next snapshot
- Typical loss (1-3%) is acceptable for 60 Hz updates

### 9.4 Optimization Opportunities (Not Implemented)

Potential improvements for future versions:
- Delta compression (send only changed entities)
- Quantization (reduce float precision)
- Binary protocol (replace JSON with binary format)
- Interest management (send only visible entities)
- Client prediction (extrapolate between snapshots)
- Snapshot interpolation (smooth rendering)

---

## Appendix A: Code Examples

### A.1 Server - Player Entity Creation

```cpp
void AsioServer::createPlayerEntity(std::uint32_t clientId) {
  ecs::Entity player = m_world->createEntity();
  
  // Assign player role
  m_world->addComponent(player, ecs::PlayerControlled{});
  
  // Set spawn position
  ecs::Transform transform;
  transform.x = 100.0F;
  transform.y = 300.0F + (clientId * 50.0F);  // Stack players vertically
  transform.rotation = 0.0F;
  transform.scale = 1.0F;
  m_world->addComponent(player, transform);
  
  // SERVER ASSIGNS VISUAL IDENTITY
  ecs::Sprite sprite;
  sprite.spriteId = ecs::SpriteId::PLAYER_SHIP;
  sprite.width = 140;
  sprite.height = 60;
  m_world->addComponent(player, sprite);
  
  // Network identity
  ecs::Networked networked;
  networked.networkId = clientId;  // Use clientId as stable network ID
  m_world->addComponent(player, networked);
  
  std::cout << "[Server] Player entity created for client " << clientId << "\n";
}
```

### A.2 Server - Snapshot Broadcasting

```cpp
void NetworkSendSystem::update(ecs::World &world, float deltaTime) {
  m_timeSinceLastSend += deltaTime;
  if (m_timeSinceLastSend >= 0.016f) {  // 60 Hz
    nlohmann::json snapshot;
    snapshot["type"] = "snapshot";
    snapshot["entities"] = nlohmann::json::array();
    
    // Query all networked entities
    std::vector<ecs::Entity> entities;
    world.getEntitiesWithSignature(getSignature(), entities);
    
    for (const auto &entity : entities) {
      const auto &networked = world.getComponent<ecs::Networked>(entity);
      const auto &transform = world.getComponent<ecs::Transform>(entity);
      
      nlohmann::json entityJson;
      entityJson["id"] = networked.networkId;
      entityJson["transform"] = {
        {"x", transform.x},
        {"y", transform.y},
        {"rotation", transform.rotation},
        {"scale", transform.scale}
      };
      
      // Optional components
      if (world.hasComponent<ecs::Sprite>(entity)) {
        const auto &sprite = world.getComponent<ecs::Sprite>(entity);
        entityJson["sprite"] = sprite.toJson();
      }
      
      snapshot["entities"].push_back(entityJson);
    }
    
    // Serialize and broadcast
    const std::string jsonStr = snapshot.dump();
    const auto serialized = m_networkManager->getPacketHandler()->serialize(jsonStr);
    
    for (const auto &[clientId, _] : m_networkManager->getClients()) {
      m_networkManager->send(
        std::span<const std::byte>(
          reinterpret_cast<const std::byte *>(serialized.data()),
          serialized.size()
        ),
        clientId
      );
    }
    
    m_timeSinceLastSend = 0.0f;
  }
}
```

### A.3 Client - Input Sending

```cpp
void NetworkSendSystem::sendInputToServer(ecs::Entity entity, const ecs::Input &input) {
  nlohmann::json message;
  message["type"] = "player_input";
  message["entity_id"] = m_clientId;  // Server-assigned ID
  message["input"]["up"] = input.up;
  message["input"]["down"] = input.down;
  message["input"]["left"] = input.left;
  message["input"]["right"] = input.right;
  message["input"]["shoot"] = input.shoot;
  
  std::string jsonStr = message.dump();
  auto serialized = m_networkManager->getPacketHandler()->serialize(jsonStr);
  
  m_networkManager->send(
    std::span<const std::byte>(
      reinterpret_cast<const std::byte *>(serialized.data()),
      serialized.size()
    ),
    0  // Endpoint ID 0 for client→server
  );
}
```

### A.4 Client - Snapshot Processing

```cpp
void ClientNetworkReceiveSystem::handleSnapshot(ecs::World &world, const nlohmann::json &json) {
  if (!json.contains("entities")) return;
  
  for (const auto &entityJson : json["entities"]) {
    const std::uint32_t networkId = entityJson["id"];
    const auto &transformJson = entityJson["transform"];
    
    // Find or create local entity
    ecs::Entity entity;
    auto it = g_networkIdToEntity.find(networkId);
    if (it != g_networkIdToEntity.end()) {
      entity = it->second;
    } else {
      entity = world.createEntity();
      g_networkIdToEntity[networkId] = entity;
      
      ecs::Networked net;
      net.networkId = networkId;
      world.addComponent(entity, net);
    }
    
    // Update transform
    auto &transform = world.getComponent<ecs::Transform>(entity);
    transform.x = transformJson["x"];
    transform.y = transformJson["y"];
    transform.rotation = transformJson["rotation"];
    transform.scale = transformJson["scale"];
    
    // Update sprite if present
    if (entityJson.contains("sprite")) {
      ecs::Sprite sprite = ecs::Sprite::fromJson(entityJson["sprite"]);
      if (!world.hasComponent<ecs::Sprite>(entity)) {
        world.addComponent(entity, sprite);
      } else {
        world.getComponent<ecs::Sprite>(entity) = sprite;
      }
    }
  }
  
  // Handle destroyed entities
  if (json.contains("destroyed")) {
    for (const auto &destroyedId : json["destroyed"]) {
      std::uint32_t networkId = destroyedId;
      auto it = g_networkIdToEntity.find(networkId);
      if (it != g_networkIdToEntity.end()) {
        world.destroyEntity(it->second);
        g_networkIdToEntity.erase(it);
      }
    }
  }
}
```

---

## Appendix B: JSON Schema Definitions

### B.1 assign_id

| Code | Name | Direction | Reliable | Description |
|------|------|-----------|----------|-------------|
| 0x01 | CONNECT_REQUEST | C→S | Yes | Client connection request |
| 0x02 | CONNECT_ACCEPT | S→C | Yes | Connection accepted |
| 0x03 | CONNECT_REJECT | S→C | Yes | Connection rejected |
| 0x04 | DISCONNECT | Both | Yes | Graceful disconnect |
| 0x05 | HEARTBEAT | Both | No | Keep-alive ping |
| 0x10 | INPUT_STATE | C→S | No | Player input commands |
| 0x20 | GAME_SNAPSHOT | S→C | No | Full game state snapshot |
| 0x21 | DELTA_SNAPSHOT | S→C | No | Delta-compressed snapshot |
| 0x30 | SPAWN_ENTITY | S→C | Yes | Entity creation |
| 0x31 | DESTROY_ENTITY | S→C | Yes | Entity destruction |
| 0x32 | UPDATE_COMPONENT | S→C | No | Component state update |
| 0x40 | CHAT_MESSAGE | Both | Yes | Text chat message |
| 0xFF | ERROR | Both | Yes | Error notification |

### 5.2 CONNECT_REQUEST (0x01)

**Direction**: Client → Server  
**Reliable**: Yes

**Payload**:
```capnp
struct ConnectRequest {
  protocolVersion @0 :UInt8;
  clientVersion @1 :Text;
  playerName @2 :Text;
  authToken @3 :Data;  # Optional authentication
}
```

### 5.3 CONNECT_ACCEPT (0x02)

**Direction**: Server → Client  
**Reliable**: Yes

**Payload**:
```capnp
struct ConnectAccept {
  clientId @0 :UInt32;
  serverTick @1 :UInt64;
  playerEntity @2 :UInt32;
}
```

### 5.4 INPUT_STATE (0x10)

**Direction**: Client → Server  
**Reliable**: No

**Payload**:
```capnp
struct InputState {
  tick @0 :UInt64;
  up @1 :Bool;
  down @2 :Bool;
  left @3 :Bool;
  right @4 :Bool;
  shoot @5 :Bool;
}
```

### 5.5 GAME_SNAPSHOT (0x20)

**Direction**: Server → Client  
**Reliable**: No

**Payload**:
```capnp
struct GameSnapshot {
  tick @0 :UInt64;
  entities @1 :List(EntityState);
}

struct EntityState {
  entityId @0 :UInt32;
  posX @1 :Float32;
  posY @2 :Float32;
  velX @3 :Float32;
  velY @4 :Float32;
  spriteId @5 :UInt8;
  health @6 :Int32;
}
```

### 5.6 DELTA_SNAPSHOT (0x21)

**Direction**: Server → Client  
**Reliable**: No

Optimized snapshot containing only changed entities since last acknowledged snapshot.

**Payload**:
```capnp
struct DeltaSnapshot {
  baseTick @0 :UInt64;
  currentTick @1 :UInt64;
  changedEntities @2 :List(EntityDelta);
  destroyedEntities @3 :List(UInt32);
}

struct EntityDelta {
  entityId @0 :UInt32;
  changedFields @1 :UInt16;  # Bitmask of changed components
  posX @2 :Float32;          # Only included if bit 0 set
  posY @3 :Float32;          # Only included if bit 1 set
  # ... other fields
}
```

---

## 6. Connection Management

### 6.1 Connection Establishment

```
Client                                Server
  |                                      |
  |  CONNECT_REQUEST (0x01)              |
  |------------------------------------->|
  |                                      |
  |         CONNECT_ACCEPT (0x02)        |
  |<-------------------------------------|
  |                                      |
  |  (Connection established)            |
```

1. Client sends `CONNECT_REQUEST` with protocol version and credentials
2. Server validates request and responds with:
   - `CONNECT_ACCEPT` if accepted (includes assigned client ID)
   - `CONNECT_REJECT` if rejected (includes reason code)

### 6.2 Heartbeat Mechanism

- Both parties MUST send `HEARTBEAT` messages every 5 seconds during idle periods
- If no message received for 15 seconds, connection is considered dead
- Heartbeat messages carry minimal payload (timestamp only)

### 6.3 Graceful Disconnection

```
Client                                Server
  |                                      |
  |  DISCONNECT (0x04)                   |
  |------------------------------------->|
  |                                      |
  |  DISCONNECT (0x04) - ACK             |
  |<-------------------------------------|
  |                                      |
  |  (Connection closed)                 |
```

---

## 7. Game State Synchronization

### 7.1 Server-Authoritative Model

- Server maintains canonical game state
- Client inputs are commands, not direct state changes
- Server validates all actions and broadcasts results

### 7.2 Snapshot Transmission

**Full Snapshots** (`GAME_SNAPSHOT`):
- Sent to newly connected clients
- Sent every 30 ticks (~500ms) as fallback
- Contains complete world state

**Delta Snapshots** (`DELTA_SNAPSHOT`):
- Sent every tick (60 Hz)
- Contains only changed entities
- References previous snapshot via `baseTick`

### 7.3 Client-Side Prediction

Clients MAY implement prediction:
1. Apply input immediately to local state
2. Receive authoritative state from server
3. Reconcile prediction errors (rollback/interpolation)

### 7.4 Entity Interpolation

Clients SHOULD interpolate between snapshots:
- Render at `serverTick - 2` (100ms buffer)
- Smooth interpolation between known states
- Reduces visual jitter from packet loss

---

## 8. Reliability and Ordering

### 8.1 Reliable Delivery

Messages with `R` flag set require acknowledgment:

```
Sender                              Receiver
  |                                      |
  |  MSG (seq=100, R=1)                  |
  |------------------------------------->|
  |                                      |
  |  ACK (seq=100, A=1)                  |
  |<-------------------------------------|
```

**Timeout and Retransmission**:
- Sender retransmits after 200ms if no ACK received
- Maximum 5 retransmission attempts
- Exponential backoff: 200ms, 400ms, 800ms, 1600ms, 3200ms

### 8.2 Sequence Numbers

- Each endpoint maintains separate send/receive sequence counters
- Sequence numbers wrap at 65535 (2^16)
- Out-of-order packets are buffered up to 32 packets

### 8.3 Duplicate Detection

Receivers MUST discard duplicate packets (same sequence number).

---

## 9. Security Considerations

### 9.1 Authentication

Implementations SHOULD support token-based authentication:
- `CONNECT_REQUEST` includes `authToken` field
- Server validates token against authentication service
- Invalid tokens result in `CONNECT_REJECT`

### 9.2 Replay Attack Prevention

- Timestamps MUST be within ±5 seconds of server time
- Sequence numbers prevent replayed packets
- Server MAY implement additional nonce-based verification

### 9.3 Denial of Service (DoS) Mitigation

Servers SHOULD implement:
- Rate limiting: Max 120 packets/second per client
- Connection limits: Max connections per IP
- CPU throttling: Reject malformed packets quickly

### 9.4 Input Validation

Servers MUST validate:
- All numeric values are within expected ranges
- String lengths do not exceed limits
- Entity IDs exist before applying operations

---

## 10. Performance Considerations

### 10.1 Bandwidth Usage

**Per-Client Bandwidth Estimates**:
- Upstream (Client → Server): ~2 KB/s (input only)
- Downstream (Server → Client): ~15-30 KB/s (snapshots)

**Optimization Techniques**:
- Delta compression reduces downstream by ~70%
- Quantization of floats (16-bit fixed-point)
- Interest management (only send visible entities)

### 10.2 CPU Usage

**Serialization Performance**:
- Cap'n Proto: Zero-copy deserialization
- Arena allocation for temporary objects
- Avoid per-packet memory allocation

### 10.3 Latency Targets

| Network Condition | RTT | Target FPS | Acceptable |
|-------------------|-----|------------|------------|
| LAN | < 10ms | 60 | ✓ |
| Good Internet | 20-50ms | 60 | ✓ |
| Average Internet | 50-100ms | 30-60 | △ |
| Poor Internet | > 100ms | 20-30 | ✗ |

---

## 11. Error Handling

### 11.1 Error Codes

| Code | Name | Description |
|------|------|-------------|
| 0x00 | NO_ERROR | Success |
| 0x01 | PROTOCOL_MISMATCH | Incompatible protocol version |
| 0x02 | SERVER_FULL | Maximum players reached |
| 0x03 | AUTH_FAILED | Invalid credentials |
| 0x04 | TIMEOUT | Connection timed out |
| 0x05 | MALFORMED_PACKET | Invalid packet format |
| 0xFF | INTERNAL_ERROR | Server internal error |

### 11.2 Error Message Format

**ERROR (0xFF) Payload**:
```capnp
struct ErrorMessage {
  code @0 :UInt8;
  message @1 :Text;
  fatal @2 :Bool;  # If true, connection will be terminated
}
```

---

## 12. IANA Considerations

This protocol does not require IANA registration as it operates over user-space UDP ports.

**Recommended Port**: 4242 (UDP)  
**Port Range**: Implementers MAY use any available UDP port.

---

### B.1 assign_id Schema

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "type": "object",
  "required": ["type", "client_id"],
  "properties": {
    "type": {
      "type": "string",
      "const": "assign_id"
    },
    "client_id": {
      "type": "integer",
      "minimum": 0
    }
  }
}
```

### B.2 player_input Schema

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "type": "object",
  "required": ["type", "entity_id", "input"],
  "properties": {
    "type": {
      "type": "string",
      "const": "player_input"
    },
    "entity_id": {
      "type": "integer",
      "minimum": 0
    },
    "input": {
      "type": "object",
      "required": ["up", "down", "left", "right", "shoot"],
      "properties": {
        "up": { "type": "boolean" },
        "down": { "type": "boolean" },
        "left": { "type": "boolean" },
        "right": { "type": "boolean" },
        "shoot": { "type": "boolean" }
      }
    }
  }
}
```

### B.3 snapshot Schema

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "type": "object",
  "required": ["type", "entities"],
  "properties": {
    "type": {
      "type": "string",
      "const": "snapshot"
    },
    "entities": {
      "type": "array",
      "items": {
        "type": "object",
        "required": ["id", "transform"],
        "properties": {
          "id": {
            "type": "integer",
            "minimum": 0
          },
          "transform": {
            "type": "object",
            "required": ["x", "y", "rotation", "scale"],
            "properties": {
              "x": { "type": "number" },
              "y": { "type": "number" },
              "rotation": { "type": "number" },
              "scale": { "type": "number" }
            }
          },
          "collider": {
            "type": "object",
            "properties": {
              "w": { "type": "number", "minimum": 0 },
              "h": { "type": "number", "minimum": 0 }
            }
          },
          "sprite": {
            "type": "object",
            "properties": {
              "spriteId": { "type": "integer", "minimum": 0, "maximum": 5 },
              "width": { "type": "integer", "minimum": 1 },
              "height": { "type": "integer", "minimum": 1 }
            }
          },
          "health": {
            "type": "object",
            "properties": {
              "hp": { "type": "integer" },
              "maxHp": { "type": "integer", "minimum": 1 }
            }
          },
          "score": {
            "type": "object",
            "properties": {
              "points": { "type": "integer", "minimum": 0 }
            }
          }
        }
      }
    },
    "destroyed": {
      "type": "array",
      "items": {
        "type": "integer",
        "minimum": 0
      }
    }
  }
}
```

---

## Appendix C: Cap'n Proto Schema

```capnp
@0xbf5147b1f1e3c5d2;

struct NetworkMessage {
  messageType @0 :Text;  # Contains complete JSON string
}
```

**Note**: Despite the field name `messageType`, this field contains the **entire serialized JSON message**, not just the message type. The Cap'n Proto schema serves only as a thin wrapper for the JSON payload.

---

## Appendix D: References

- [RFC 768](https://www.rfc-editor.org/rfc/rfc768) - User Datagram Protocol
- [RFC 2119](https://www.rfc-editor.org/rfc/rfc2119) - Key words for RFCs
- [ASIO](https://think-async.com/Asio/) - C++ asynchronous I/O library
- [nlohmann/json](https://github.com/nlohmann/json) - JSON for Modern C++
- [Cap'n Proto](https://capnproto.org/) - Serialization format

---

## Appendix E: Revision History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | 2025-12-14 | R-Type Team | Initial specification based on implementation |

---

## Appendix F: Known Limitations

### F.1 No Reliability

All messages are best-effort UDP:
- Lost packets are not retransmitted
- No acknowledgments
- No guaranteed delivery

**Impact**: 
- ~1-3% packet loss on typical networks
- At 60 Hz, 1 lost snapshot = 16ms of stale data
- Generally acceptable for gameplay

### F.2 No Client Prediction

Clients render exactly what the server sends:
- No extrapolation between snapshots
- No input prediction
- Pure server authority

**Impact**:
- Input feels delayed by RTT + 1 frame
- 30ms RTT = ~46ms perceived lag
- Acceptable for LAN/low-latency networks

### F.3 No Delta Compression

Every snapshot is a full state dump:
- Bandwidth scales with entity count
- Redundant data sent every frame

**Impact**:
- 50 entities × 60 Hz = ~90-180 KB/s per client
- Limits scalability to ~10-20 clients
- LAN deployments not affected

### F.4 No Connection State Management

Server does not track connection health:
- No timeout detection
- No graceful disconnect
- Dead clients remain in client list

**Impact**:
- Manual server restart needed to clean up
- No "player disconnected" notifications
- Server memory leaks client entries

### F.5 JSON Overhead

JSON is human-readable but inefficient:
- Text encoding vs binary
- Field names sent every message
- Parsing overhead

**Impact**:
- 2-5x larger messages vs binary protocol
- ~10-20% CPU for JSON parsing
- Acceptable for current scale

---

**Document Status**: Implementation Documentation  
**Reflects**: Actual R-Type codebase as of December 2025

---

© 2025 EPITECH PROJECT - R-Type Development Team

Complete schema definition:

```capnp
@0xbf5147b1f1e3c5d2;

struct NetworkMessage {
  messageType @0 :Text;
  
  union {
    connectRequest @1 :ConnectRequest;
    connectAccept @2 :ConnectAccept;
    connectReject @3 :ConnectReject;
    disconnect @4 :Disconnect;
    inputState @5 :InputState;
    gameSnapshot @6 :GameSnapshot;
    deltaSnapshot @7 :DeltaSnapshot;
    spawnEntity @8 :SpawnEntity;
    destroyEntity @9 :DestroyEntity;
    errorMsg @10 :ErrorMessage;
  }
}

struct ConnectRequest {
  protocolVersion @0 :UInt8;
  clientVersion @1 :Text;
  playerName @2 :Text;
  authToken @3 :Data;
}

struct ConnectAccept {
  clientId @0 :UInt32;
  serverTick @1 :UInt64;
  playerEntity @2 :UInt32;
}

struct ConnectReject {
  reason @0 :Text;
  errorCode @1 :UInt8;
}

struct Disconnect {
  reason @0 :Text;
}

struct InputState {
  tick @0 :UInt64;
  up @1 :Bool;
  down @2 :Bool;
  left @3 :Bool;
  right @4 :Bool;
  shoot @5 :Bool;
}

struct GameSnapshot {
  tick @0 :UInt64;
  entities @1 :List(EntityState);
}

struct EntityState {
  entityId @0 :UInt32;
  posX @1 :Float32;
  posY @2 :Float32;
  velX @3 :Float32;
  velY @4 :Float32;
  spriteId @5 :UInt8;
  health @6 :Int32;
}

struct DeltaSnapshot {
  baseTick @0 :UInt64;
  currentTick @1 :UInt64;
  changedEntities @2 :List(EntityDelta);
  destroyedEntities @3 :List(UInt32);
}

struct EntityDelta {
  entityId @0 :UInt32;
  changedFields @1 :UInt16;
  posX @2 :Float32;
  posY @3 :Float32;
  velX @4 :Float32;
  velY @5 :Float32;
  health @6 :Int32;
}

struct SpawnEntity {
  entityId @0 :UInt32;
  entityType @1 :UInt8;
  posX @2 :Float32;
  posY @3 :Float32;
}

struct DestroyEntity {
  entityId @0 :UInt32;
}

struct ErrorMessage {
  code @0 :UInt8;
  message @1 :Text;
  fatal @2 :Bool;
}
```

---

## Appendix B: Implementation Checklist

### Server Implementation

- [ ] UDP socket binding and listening
- [ ] Client connection tracking
- [ ] Reliable message handling (ACK/retransmit)
- [ ] Snapshot generation (full + delta)
- [ ] Input validation and processing
- [ ] Heartbeat monitoring
- [ ] Rate limiting and DoS protection

### Client Implementation

- [ ] UDP socket creation
- [ ] Connection establishment flow
- [ ] Input state transmission (60 Hz)
- [ ] Snapshot reception and processing
- [ ] Client-side prediction (optional)
- [ ] Entity interpolation
- [ ] Heartbeat transmission
- [ ] Error handling and reconnection

---

## Appendix C: References

- [RFC 768](https://www.rfc-editor.org/rfc/rfc768) - User Datagram Protocol
- [RFC 2119](https://www.rfc-editor.org/rfc/rfc2119) - Key words for RFCs
- [Cap'n Proto](https://capnproto.org/) - Serialization format
- [Valve's Networking](https://developer.valvesoftware.com/wiki/Source_Multiplayer_Networking) - Game networking best practices

---

## Revision History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | 2025-12-14 | R-Type Team | Initial RFC-compliant specification |

---

**Document Status**: Draft  
**Feedback**: Submit issues to GitHub repository

---

© 2025 EPITECH PROJECT - R-Type Development Team
