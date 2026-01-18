# R-Type Network Protocol Specification

**Internet-Draft:** rtype-protocol  
**Intended Status:** Informational  
**Expiration Date:** July 18, 2026  
**Date:** January 18, 2026  
**Version:** 1.0  
**Authors:** R-Type Development Team  
**Status:** Standards Track (Game Protocol)

---

## Abstract

This document specifies the R-Type Network Protocol (RTNP), a UDP-based protocol for real-time multiplayer communication in the R-Type networked game. RTNP employs a strict client-server architecture with server-authoritative game state management. The protocol uses JSON [RFC 8259] for message serialization and Cap'n Proto for binary message wrapping.

## Status of This Document

This document describes the R-Type Network Protocol as currently implemented. This protocol is intended for use in game development and may be adopted by other projects using similar architectures.

## Copyright Notice

Copyright (c) 2026 R-Type Development Team. All rights reserved.

---

## Table of Contents

1. [Introduction](#1-introduction)
2. [Terminology and Definitions](#2-terminology-and-definitions)
3. [Protocol Architecture](#3-protocol-architecture)
4. [Transport Layer](#4-transport-layer)
5. [Message Format and Serialization](#5-message-format-and-serialization)
6. [Message Types and Specifications](#6-message-types-and-specifications)
7. [Connection Management](#7-connection-management)
8. [Game State Synchronization](#8-game-state-synchronization)
9. [Implementation Requirements](#9-implementation-requirements)
10. [Performance Characteristics](#10-performance-characteristics)
11. [Security Considerations](#11-security-considerations)
12. [IANA Considerations](#12-iana-considerations)
13. [References](#13-references)
14. [Appendices](#14-appendices)

---

## 1. Introduction

### 1.1 Purpose

This document specifies the R-Type Network Protocol (RTNP), a UDP-based protocol for real-time multiplayer communication in the R-Type game engine. RTNP implements a strict client-server architecture with server-authoritative game state management, designed for low-latency synchronous multiplayer gaming.

### 1.2 Requirements Language

The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL NOT", "SHOULD", "SHOULD NOT", "RECOMMENDED", "NOT RECOMMENDED", "MAY", and "OPTIONAL" in this document are to be interpreted as described in RFC 2119 [RFC2119] and RFC 8174 [RFC8174].

### 1.3 Scope

This specification defines:

- The UDP-based transport mechanism for RTNP communication
- Message format and serialization using JSON [RFC8259] with Cap'n Proto binary wrapping
- Message type specifications and field requirements
- Connection establishment, maintenance, and termination procedures
- Game state synchronization mechanisms
- Server authority principles for conflict resolution

This specification does NOT define:

- Client-side rendering or visualization
- Game entity logic or component behavior implementation
- Network security mechanisms (encryption, authentication)
- Anti-cheat or validation strategies

### 1.4 Terminology

**Client**
: A game instance running on a player's machine that initiates connections to the Server and sends Player Input.

**Server**
: The authoritative instance maintaining the canonical game state, processing inputs, and broadcasting state updates.

**Player**
: A human operator controlling one or more Client instances.

**Entity**
: A game object with a unique networkId and associated Components.

**Component**
: A data structure attached to an Entity containing specific game object properties (position, velocity, health, sprite data, etc.).

**Snapshot**
: A game state update message sent by the Server containing the current state of one or more Entities.

**Game Tick**
: A discrete time interval (typically 16-33 milliseconds) in which the Server processes inputs, updates logic, and broadcasts Snapshots.

**Message**
: A complete communication unit formatted as JSON per [RFC8259].

**Packet**
: A UDP datagram containing serialized Message data, optionally wrapped with Cap'n Proto.

**Player Input**
: Commands issued by a Player (movement, attacks, actions) transmitted in PlayerInput Messages.

**Network ID**
: A server-assigned unique 32-bit identifier for each Entity in the game world.

**Server Authority**
: The principle that Server state is the canonical source of truth; Client state is temporary and subject to correction.

---

## 2. Terminology and Definitions

This section restates key terms in formal RFC style for clarity.

**Authoritative Source** (noun):
: The Server instance whose game state is the canonical version used for all authoritative decisions.

**Best Effort Delivery** (noun):
: A delivery model where messages are sent without acknowledgment or retransmission guarantees; messages MAY be lost, duplicated, or reordered.

**Game Tick** (noun):
: A discrete time interval in which the Server processes one complete cycle of game logic, input processing, and state broadcasting.

**Server Epoch** (noun):
: A monotonically increasing counter starting at 0 incremented once per Game Tick and included in all Snapshots for ordering purposes.

**Network Entity** (noun):
: A game object that exists in the networked game world and is synchronized between Server and Clients via Snapshots.

---

## 3. Protocol Architecture

### 3.1 Client-Server Model

RTNP implements a strict client-server communication model:

**Server Role (REQUIRED)**
: The Server MUST be the authoritative source for all game state. All game logic execution, collision detection, entity updates, and state changes MUST occur on the Server.

**Client Role (REQUIRED)**
: Clients MUST send input commands to the Server and MUST accept game state updates from the Server. Clients MUST NOT perform authoritative game logic.

**Communication Pattern (REQUIRED)**
: The Server and Clients MUST use bidirectional UDP communication. Input flows from Client→Server; state updates flow from Server→Clients (broadcast to all connected Clients).

### 3.2 Design Principles

The following design principles inform this specification:

1. **Simplicity**: Use JSON [RFC8259] for message interchange; avoid complex serialization schemes.

2. **Server Authority**: All game logic and state decisions MUST originate from the Server; Clients render received state without modification.

3. **Best Effort**: RTNP operates over UDP without reliability mechanisms. Implementations MUST NOT assume message delivery, ordering, or non-duplication.

4. **Full State Broadcast**: The Server SHOULD broadcast complete Entity snapshots (not delta updates) at regular intervals to all Clients.

5. **Stateless Inputs**: Player Input messages SHOULD be stateless (not cumulative); each input message is independent.

6. **Tick Synchronization**: The Server SHOULD process inputs and broadcast state at a regular tick interval (commonly 16-33 milliseconds).

### 3.3 Protocol Stack

RTNP operates within the following protocol stack:

```
┌──────────────────────────────────────────┐
│    R-Type Game Logic (ECS Engine)        │
├──────────────────────────────────────────┤
│    RTNP Message Layer (JSON)             │
│    [RFC 8259 Conformant]                 │
├──────────────────────────────────────────┤
│    Cap'n Proto Binary Serialization      │
│    (Optional message wrapping)           │
├──────────────────────────────────────────┤
│    UDP Transport Layer                   │
│    (RFC 768)                             │
├──────────────────────────────────────────┤
│    IPv4 / IPv6                           │
│    (RFC 791 / RFC 2460)                  │
└──────────────────────────────────────────┘
```

**Note**: Cap'n Proto is used only as a thin serialization wrapper around JSON strings. The actual message structure is pure JSON.

---

## 4. Transport Layer

### 4.1 UDP Protocol

RTNP operates over UDP (User Datagram Protocol) as defined in RFC 768 [RFC768]. UDP is selected for this protocol because:

- **Low Latency**: No connection establishment overhead
- **Stateless**: Simplifies server scalability
- **Game-Appropriate**: Game state synchronization tolerates occasional packet loss

**UDP Characteristics Relevant to RTNP**:

- Connectionless protocol (no handshake required)
- No reliability guarantees; packets MAY be lost
- No packet ordering guarantees; packets MAY arrive out-of-sequence
- No flow control; senders are not throttled by receiver
- Minimal protocol overhead (8-byte header)

### 4.2 Port Configuration

Implementations of RTNP MUST use the following port configuration:

**Server Port (REQUIRED)**
: The Server MUST listen on UDP port 4242 or a configured alternative port.

**Client Ports (REQUIRED)**
: Clients MUST use ephemeral UDP ports assigned by the operating system.

Port values MAY be configured via implementation-specific settings files.

### 4.3 Datagram Size Constraints

**Maximum Datagram Size (REQUIRED)**
: Datagrams sent or received by RTNP implementations MUST NOT exceed 8192 bytes. Implementations MUST either truncate or fragment messages exceeding this size.

**Typical Datagram Sizes**
: In practice, datagrams have the following characteristics:

- Player Input messages: ~150 bytes
- Snapshot messages with 10-20 entities: 500-2000 bytes
- Snapshot messages with 50+ entities: 2000-4000 bytes

**Receive Buffer Configuration (RECOMMENDED)**
: Implementations SHOULD configure UDP receive buffers to at least 8 megabytes to handle burst traffic from multiple clients:

```c++
// Example using ASIO (C++)
static constexpr int MEGABYTE = 1024 * 1024;
static constexpr int BUFFER_SIZE_MB = 8;
asio::socket_base::receive_buffer_size option(MEGABYTE * BUFFER_SIZE_MB);
socket.set_option(option);
```

### 4.4 Unreliability Model

RTNP does NOT provide reliability. Implementations MUST operate with the following assumptions:

1. **Packet Loss**: Any datagram MAY be lost in transit. Implementations MUST NOT use acknowledgments or retransmissions.

2. **Out-of-Order Delivery**: Datagrams MAY arrive in any order or be duplicated. Implementations MUST use sequence numbers (Epoch field in Snapshots) to detect and discard obsolete packets.

3. **No Congestion Control**: Senders MUST NOT implement TCP-style congestion control. Game servers MAY send at a constant rate (typically 60 Hz).

4. **Handling Loss**: Clients MUST tolerate input message loss. The Server SHOULD broadcast Snapshots frequently enough (every tick) that temporary input loss does not significantly impact gameplay.

---

## 5. Message Format and Serialization

### 5.1 Overview

RTNP messages are represented as JSON objects as defined in RFC 8259 [RFC8259]. Messages MAY optionally be wrapped in a Cap'n Proto [CAPNP] binary container for serialization.

### 5.2 JSON Message Structure

All RTNP messages MUST be valid JSON objects with a mandatory "type" field identifying the message category:

```json
{
  "type": "message_category",
  "field1": value1,
  "field2": value2
}
```

**Requirements (REQUIRED)**:

- Every message MUST contain a "type" field (string value)
- The "type" field MUST be the first field in the JSON object
- Additional fields depend on the specific message type
- All messages MUST be valid UTF-8 encoded JSON

### 5.3 Cap'n Proto Serialization (Optional)

Implementations MAY optionally wrap JSON messages using Cap'n Proto serialization. The Cap'n Proto schema is:

```capnp
@0xbf5147b1f1e3c5d2;

struct NetworkMessage {
  messageType @0 :Text;  # Contains the entire JSON message as a UTF-8 string
}
```

**Note**: The field name `messageType` is historical and misleading; it contains the complete JSON message, not just the type.

### 5.4 Character Encoding

All text fields in RTNP messages MUST use UTF-8 character encoding [RFC3629]. Non-ASCII characters MUST be encoded using JSON escape sequences as defined in RFC 8259 Section 7.

---

## 6. Message Types and Specifications

### 6.1 Message Type Directory

The following message types are defined in this protocol:

| Type | Direction | Frequency | Purpose | Section |
|------|-----------|-----------|---------|---------|
| `assign_id` | Server→Client | Once | Client ID assignment | 6.2 |
| `player_input` | Client→Server | ~60 Hz | Player control input | 6.3 |
| `snapshot` | Server→Client | ~60 Hz | Game state update | 6.4 |
| `connect_request` | Client→Server | Once | Connection initiation | 6.5 |
| `disconnect` | Client→Server | Once | Connection termination | 6.6 |

### 6.2 assign_id Message

**Direction (REQUIRED)**: Server → Client  
**Frequency (REQUIRED)**: Once per client connection  
**Purpose**: Inform the client of its server-assigned unique identifier

**Message Format**:

```json
{
  "type": "assign_id",
  "client_id": <integer>
}
```

**Field Definitions**:

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `type` | string | YES | MUST be "assign_id" |
| `client_id` | integer | YES | Non-negative integer in range [0, 2^32-1] |

**Semantics**:

- The Server MUST send an `assign_id` message to a Client when the Client first sends any message to the Server.
- The `client_id` field MUST be a unique identifier for the lifetime of the Client's connection.
- The Server MUST NOT assign the same `client_id` to two concurrently connected Clients.
- Multiple Clients MAY reuse the same `client_id` if they connect sequentially (non-overlapping lifetimes).

**Example**:

```json
{
  "type": "assign_id",
  "client_id": 5
}
```

### 6.3 player_input Message

**Direction (REQUIRED)**: Client → Server  
**Frequency (RECOMMENDED)**: Every Game Tick (~60 Hz or 16 ms intervals)  
**Purpose**: Transmit player control input state to the Server

**Message Format**:

```json
{
  "type": "player_input",
  "client_id": <integer>,
  "keys": {
    "up": <boolean>,
    "down": <boolean>,
    "left": <boolean>,
    "right": <boolean>,
    "space": <boolean>
  }
}
```

**Field Definitions**:

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `type` | string | YES | MUST be "player_input" |
| `client_id` | integer | YES | Client's ID (from `assign_id` message) |
| `keys` | object | YES | Keyboard state object |
| `keys.up` | boolean | YES | True if up arrow is pressed |
| `keys.down` | boolean | YES | True if down arrow is pressed |
| `keys.left` | boolean | YES | True if left arrow is pressed |
| `keys.right` | boolean | YES | True if right arrow is pressed |
| `keys.space` | boolean | YES | True if space bar is pressed |

**Semantics**:

- Clients MUST send `player_input` messages regularly (every Game Tick is RECOMMENDED).
- The `client_id` field MUST match the ID assigned by the Server in the `assign_id` message.
- Each key field represents the instantaneous state of that key (true=pressed, false=not pressed).
- Input messages SHOULD be stateless; each message is independent and does not accumulate with previous messages.

**Example**:

```json
{
  "type": "player_input",
  "client_id": 1,
  "keys": {
    "up": false,
    "down": false,
    "left": true,
    "right": false,
    "space": false
  }
}
```

### 6.4 snapshot Message

**Direction (REQUIRED)**: Server → Client  
**Frequency (RECOMMENDED)**: Every Game Tick (~60 Hz or 16 ms intervals)  
**Purpose**: Transmit the complete game state to all connected Clients

**Message Format**:

```json
{
  "type": "snapshot",
  "epoch": <integer>,
  "entities": [
    {
      "networkId": <integer>,
      "type": <string>,
      "position": { "x": <number>, "y": <number> },
      "velocity": { "x": <number>, "y": <number> },
      "sprite": {
        "spriteId": <integer>,
        "spriteSheetId": <integer>,
        "scale": <number>,
        "rotation": <number>
      },
      "health": <integer>,
      "maxHealth": <integer>
    }
  ]
}
```

**Field Definitions**:

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `type` | string | YES | MUST be "snapshot" |
| `epoch` | integer | YES | Monotonically increasing tick counter |
| `entities` | array | YES | Array of entity objects (MAY be empty) |

**Entity Field Definitions** (per entity object):

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `networkId` | integer | YES | Unique entity identifier |
| `type` | string | YES | Entity type (e.g., "player_ship", "enemy", "bullet") |
| `position` | object | NO | 2D position (x, y coordinates) |
| `velocity` | object | NO | 2D velocity vector (x, y components) |
| `sprite` | object | NO | Sprite rendering data |
| `health` | integer | NO | Current health points |
| `maxHealth` | integer | NO | Maximum health points |

**Position/Velocity Object**:

```json
{
  "x": <number>,  // Floating point coordinate
  "y": <number>   // Floating point coordinate
}
```

**Sprite Object**:

```json
{
  "spriteId": <integer>,        // Index into sprite sheet
  "spriteSheetId": <integer>,   // Which sprite sheet file
  "scale": <number>,            // Scale factor (1.0 = normal size)
  "rotation": <number>          // Rotation in degrees (0-360)
}
```

**Semantics**:

- The Server MUST include all networked Entities in the Snapshot.
- The `epoch` field MUST increase by exactly 1 for each consecutive Snapshot sent by the Server.
- Clients MUST discard Snapshots with an `epoch` less than the most recently received Snapshot's `epoch` (to handle out-of-order delivery).
- Entity fields that are not applicable to a particular entity type MAY be omitted.
- The Server MUST ensure the Snapshot JSON is valid UTF-8 and represents valid JSON.

**Example**:

```json
{
  "type": "snapshot",
  "epoch": 42,
  "entities": [
    {
      "networkId": 0,
      "type": "player_ship",
      "position": { "x": 100.0, "y": 200.0 },
      "velocity": { "x": 2.0, "y": 0.0 },
      "sprite": {
        "spriteId": 1,
        "spriteSheetId": 0,
        "scale": 1.0,
        "rotation": 0.0
      },
      "health": 100,
      "maxHealth": 100
    }
  ]
}
```

### 6.5 connect_request Message

**Direction (REQUIRED)**: Client → Server  
**Frequency (REQUIRED)**: Once per connection (initial message)  
**Purpose**: Initiate a connection and provide initial client information

**Message Format**:

```json
{
  "type": "connect_request",
  "player_name": <string>,
  "version": <string>
}
```

**Field Definitions**:

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `type` | string | YES | MUST be "connect_request" |
| `player_name` | string | YES | Player's chosen name (1-32 characters) |
| `version` | string | NO | Protocol version (e.g., "1.0") |

**Semantics**:

- The Client SHOULD send `connect_request` as the first message.
- The Server SHOULD respond with an `assign_id` message.
- The `player_name` MUST be between 1 and 32 characters in length.
- Servers MAY reject connections with invalid player names.

### 6.6 disconnect Message

**Direction (REQUIRED)**: Client → Server  
**Frequency (RECOMMENDED)**: Once when disconnecting (final message)  
**Purpose**: Gracefully notify the Server of Client disconnection

**Message Format**:

```json
{
  "type": "disconnect",
  "client_id": <integer>,
  "reason": <string>
}
```

**Field Definitions**:

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `type` | string | YES | MUST be "disconnect" |
| `client_id` | integer | YES | Client's assigned ID |
---

## 7. Connection Management

### 7.1 Connection Establishment

The following sequence describes how a Client establishes a connection to the Server:

**Step 1: Client Sends Initial Message**

The Client transmits a message to the Server's UDP address and port (default 4242).

**Step 2: Server Assigns Client ID**

Upon receiving the first message from a new Client address, the Server generates a unique `client_id` and sends an `assign_id` message:

```json
{
  "type": "assign_id",
  "client_id": 0
}
```

**Step 3: Bidirectional Communication**

The Client may now send `player_input` messages and receive `snapshot` messages. The Server begins broadcasting `snapshot` messages to the Client.

### 7.2 Ongoing Communication

After connection establishment:

- **Client** sends `player_input` messages every Game Tick (~60 Hz or 16 ms)
- **Server** broadcasts `snapshot` messages every Game Tick (~60 Hz or 16 ms) to all connected Clients
- No acknowledgment or confirmation messages are required

### 7.3 Connection Termination

**Graceful Disconnection (RECOMMENDED)**:

The Client sends a `disconnect` message before closing the UDP socket.

The Server SHOULD process this immediately and clean up associated Client state.

**Timeout Disconnection (REQUIRED)**:

If the Server does not receive any message from a Client for a timeout period (RECOMMENDED: 30 seconds), the Server MUST:

1. Remove the Client from its active client list
2. Cease sending `snapshot` messages to that Client's address
3. Mark the Client's entities for destruction in the game world

### 7.4 Connection State Diagram

```
    Client                          Server
      |                              |
      |---- connect_request -------> |
      |                              | (assigns client_id)
      | <------ assign_id ---------- |
      |                              |
      | <------ snapshot ----------- |
      | <------ snapshot ----------- | (continuous)
      |                              |
      | ---- player_input ---------> |
      | ---- player_input ---------> | (continuous)
      |                              |
      | ---- disconnect -----------> |
      |                              | (cleanup)
```

---

## 8. Game State Synchronization

### 8.1 Server Authority Model

RTNP implements a strict server-authoritative model:

**Server Responsibilities (REQUIRED)**:

1. MUST maintain the canonical game state
2. MUST process all Player Inputs and update game logic
3. MUST compute entity positions, velocities, and state changes
4. MUST broadcast Snapshots to all connected Clients
5. MUST ensure all Clients see the same game world (with latency tolerance)

**Client Responsibilities (REQUIRED)**:

1. MUST render the game state received from the Server
2. MUST NOT perform authoritative game logic
3. MUST NOT predict future entity positions
4. MUST NOT interpolate between Snapshots
5. MUST discard out-of-date Snapshots (use `epoch` field)

### 8.2 Snapshot Transmission

**Frequency (RECOMMENDED)**:

The Server SHOULD broadcast a `snapshot` message to all connected Clients at a regular interval:

- **Rate**: 60 Hz (one snapshot every 16 milliseconds)
- **Jitter Tolerance**: ±2 milliseconds acceptable

**Snapshot Size**:

- **Minimal** (5 entities): ~500 bytes
- **Small** (20 entities): ~1500 bytes
- **Medium** (50 entities): ~3500 bytes
- **Large** (100+ entities): up to 8192 bytes (packet limit)

### 8.3 Entity State Representation

Each Entity in a Snapshot includes the following fields:

```json
{
  "networkId": 42,
  "type": "player_ship",
  "position": { "x": 100.0, "y": 200.0 },
  "velocity": { "x": 2.0, "y": 0.0 },
  "sprite": {
    "spriteId": 0,
    "spriteSheetId": 0,
    "scale": 1.0,
    "rotation": 0.0
  },
  "health": 100,
  "maxHealth": 100
}
```

**Position Coordinates**:

- Measured in pixels relative to the game world origin (0, 0)
- Floating-point values for smooth movement
- Server establishes coordinate system; Clients MUST use it as-is

**Sprite Data**:

- `spriteId`: Index into sprite sheet (enum-like value)
- `spriteSheetId`: Identifier of the texture resource to use
- `scale`: Uniform scale factor (1.0 = normal size)
- `rotation`: Rotation angle in degrees (0-360)

### 8.4 Entity Lifecycle Management

**Entity Creation**:

1. Server creates an Entity in its game world
2. Server assigns a unique `networkId` to the Entity
3. Entity appears in the next `snapshot` message's `entities` array
4. Client receives Snapshot, creates local Entity copy
5. Client establishes mapping: `networkId` → local entity ID

**Entity Update**:

1. Server modifies Entity components
2. Updated component values appear in next `snapshot`
3. Client receives Snapshot, queries and updates local Entity
4. Rendering system uses updated component values

**Entity Destruction**:

1. Server destroys Entity from its world
2. Server records the Entity's `networkId` for removal notification
3. Next Snapshot includes destroyed IDs
4. Client receives Snapshot, finds Entity by `networkId`
5. Client removes Entity from local world and updates mappings

### 8.5 Handling Out-of-Order Snapshots

**Epoch-Based Ordering (REQUIRED)**:

Clients MUST use the `epoch` field to detect and discard stale Snapshots:

```cpp
// Pseudocode
void handleSnapshot(const json &snapshot) {
  uint32_t received_epoch = snapshot["epoch"];
  if (received_epoch <= last_received_epoch) {
    return;  // Discard out-of-order snapshot
  }
  last_received_epoch = received_epoch;
  // Process snapshot...
}
```

**Epoch Guarantees**:

- The Server MUST increment `epoch` by exactly 1 for each consecutive Snapshot
- `epoch` starts at 0 and MUST NOT wrap around to 0
- Clients MUST assume newer `epoch` values represent newer game state

---

## 9. Implementation Requirements

### 9.1 Server Implementation

**Message Processing (REQUIRED)**:

| Message | Sender | Action |
|---------|--------|--------|
| `connect_request` | Client | Assign `client_id`, send `assign_id` |
| `player_input` | Client | Update player's input state |
| `disconnect` | Client | Clean up client state |

**Snapshot Broadcasting (REQUIRED)**:

The Server MUST:

1. Maintain a collection of all connected Clients
2. Build a complete game state Snapshot every tick
3. Broadcast the Snapshot to all connected Clients
4. Increment the `epoch` field for each broadcast

**Timeout Handling (REQUIRED)**:

Servers MUST implement a timeout mechanism to clean up inactive clients after 30 seconds of inactivity.

### 9.2 Client Implementation

**Message Reception (REQUIRED)**:

Clients MUST:

1. Listen for incoming messages on a UDP socket
2. Parse JSON messages
3. Handle `assign_id` messages (store `client_id`)
4. Handle `snapshot` messages (update local game state)

**Input Transmission (REQUIRED)**:

Clients MUST:

1. Periodically read the local input state
2. Package input as `player_input` JSON message
3. Send to Server at regular intervals (every Game Tick, ~60 Hz)

**State Application (REQUIRED)**:

Clients MUST:

1. Parse `snapshot` messages
2. For each Entity in the Snapshot:
   - Create local Entity if not already present
   - Update position, velocity, sprite, and other components
3. For entities in `destroyed` array:
   - Remove the local Entity by `networkId`

---

## 10. Performance Characteristics

### 10.1 Bandwidth Estimation

**Single-Client Scenario** (1 client, 10 networked entities):

- **Client Upstream** (C→S): ~150 bytes/message × 60 msg/s = **9 KB/s**
- **Client Downstream** (S→C): ~1500 bytes/message × 60 msg/s = **90 KB/s**

**Multi-Client Scenario** (10 simultaneous clients, 50 total entities):

- **Server Upstream** (S→C total): ~5000 bytes/message × 60 msg/s × 10 clients = **3 MB/s**
- **Client Individual** (S→C): ~90 KB/s

### 10.2 Latency Characteristics

**Network Latency Components**:

- **Client→Server**: Round-trip latency (RTT) for input message
- **Server Processing**: ~1-5 milliseconds
- **Server→Client**: RTT for snapshot message

**Perceived Player Latency**:

```
Perceived Latency = RTT + 1 Frame Delay
                  = RTT + (1000 ms / 60 Hz)
                  = RTT + 16 ms
```

**Example**: RTT = 30 ms → Perceived Latency = **46 ms**

### 10.3 CPU Usage

**Server CPU** (per connected client):

- Receiving and parsing `player_input`: ~0.1 ms
- Processing input in game logic: ~0.5-2 ms
- Building Snapshot: ~0.2 ms
- **Total per client**: ~1-2.5 ms per tick

**Client CPU**:

- Receiving and parsing `snapshot`: ~0.1 ms
- Updating local ECS world: ~0.2-0.5 ms
- **Total**: ~0.3-0.6 ms per tick

---

## 11. Security Considerations

### 11.1 Lack of Authentication

**Important Note**: RTNP does NOT include authentication. Implementations MUST implement their own security layer:

- Game servers on public networks SHOULD require authentication
- Servers SHOULD validate player names and reject suspicious values
- Servers SHOULD track connected clients and implement rate-limiting

### 11.2 Lack of Encryption

RTNP messages are transmitted in plaintext UDP. Implementations requiring privacy MUST:

1. Use DTLS (Datagram Transport Layer Security) over UDP
2. Or implement application-level encryption
3. Or restrict RTNP to private networks (LAN)

### 11.3 Lack of Integrity Checking

RTNP does NOT include checksums or signatures beyond UDP's checksum. Implementations in adversarial environments SHOULD add:

1. Message authentication codes (MACs)
2. Digital signatures
3. Rate limiting to detect flood attacks

### 11.4 Recommended Security Practices

- **Validation**: Always validate received JSON
- **Bounds Checking**: Ensure received values are reasonable
- **Rate Limiting**: Reject clients sending excessive messages
- **Input Filtering**: Validate player names
- **Timeout Cleanup**: Remove inactive clients
- **Server Authority**: Never accept client-originated state changes

---

## 12. IANA Considerations

This document does not require IANA allocations. The R-Type Network Protocol is a game-specific protocol.

---

## 13. References

### Normative References

[RFC768]  
: Postel, J., "User Datagram Protocol", STD 6, RFC 768, August 1980.

[RFC2119]  
: Bradner, S., "Key words for use in RFCs to Indicate Requirement Levels", BCP 14, RFC 2119, March 1997.

[RFC3629]  
: Yergeau, F., "UTF-8, a transformation format of ISO 10646", STD 63, RFC 3629, November 2003.

[RFC8259]  
: Bray, T., Ed., "The JavaScript Object Notation (JSON) Data Interchange Format", STD 90, RFC 8259, December 2017.

[RFC8174]  
: Leiba, B., "Ambiguity of Uppercase vs Lowercase in RFC 2119 Key Words", BCP 14, RFC 8174, May 2020.

### Informative References

[CAPNP]  
: Cap'n Proto: Insanely Fast Data Serialization Format, https://capnproto.org/

[ASIO]  
: ASIO C++ Library, https://think-async.com/Asio/

[NLOHMANN_JSON]  
: JSON for Modern C++, https://github.com/nlohmann/json

---

## 14. Appendices

### Appendix A: Cap'n Proto Schema

```capnp
@0xbf5147b1f1e3c5d2;

struct NetworkMessage {
  messageType @0 :Text;  # Contains the entire JSON message as a UTF-8 string
}
```

**Note**: The field name `messageType` is historical; it contains the complete JSON message, not just the type.

### Appendix B: JSON Schema Definitions

#### B.1 assign_id Message Schema

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "type": "object",
  "required": ["type", "client_id"],
  "properties": {
    "type": { "type": "string", "const": "assign_id" },
    "client_id": { "type": "integer", "minimum": 0 }
  }
}
```

#### B.2 player_input Message Schema

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "type": "object",
  "required": ["type", "client_id", "keys"],
  "properties": {
    "type": { "type": "string", "const": "player_input" },
    "client_id": { "type": "integer", "minimum": 0 },
    "keys": {
      "type": "object",
      "required": ["up", "down", "left", "right", "space"],
      "properties": {
        "up": { "type": "boolean" },
        "down": { "type": "boolean" },
        "left": { "type": "boolean" },
        "right": { "type": "boolean" },
        "space": { "type": "boolean" }
      }
    }
  }
}
```

#### B.3 snapshot Message Schema

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "type": "object",
  "required": ["type", "epoch", "entities"],
  "properties": {
    "type": { "type": "string", "const": "snapshot" },
    "epoch": { "type": "integer", "minimum": 0 },
    "entities": {
      "type": "array",
      "items": {
        "type": "object",
        "required": ["networkId", "type"],
        "properties": {
          "networkId": { "type": "integer", "minimum": 0 },
          "type": { "type": "string" },
          "position": {
            "type": "object",
            "properties": {
              "x": { "type": "number" },
              "y": { "type": "number" }
            }
          },
          "velocity": {
            "type": "object",
            "properties": {
              "x": { "type": "number" },
              "y": { "type": "number" }
            }
          },
          "sprite": {
            "type": "object",
            "properties": {
              "spriteId": { "type": "integer", "minimum": 0 },
              "spriteSheetId": { "type": "integer", "minimum": 0 },
              "scale": { "type": "number", "minimum": 0 },
              "rotation": { "type": "number" }
            }
          },
          "health": { "type": "integer" },
          "maxHealth": { "type": "integer", "minimum": 1 }
        }
      }
    }
  }
}
```

### Appendix C: Implementation Examples

#### C.1 Server Snapshot Broadcasting (Pseudocode)

```cpp
void server_broadcast_snapshot(World &world, AllClients &clients) {
  json snapshot = {
    {"type", "snapshot"},
    {"epoch", current_epoch},
    {"entities", json::array()}
  };
  
  for (Entity &entity : world.getNetworkedEntities()) {
    json entity_json = {
      {"networkId", entity.networkId},
      {"type", entity.type},
      {"position", {{"x", entity.position.x}, {"y", entity.position.y}}},
      {"velocity", {{"x", entity.velocity.x}, {"y", entity.velocity.y}}},
      {"sprite", entity.sprite.toJson()},
      {"health", entity.health}
    };
    snapshot["entities"].push_back(entity_json);
  }
  
  string serialized = serialize(snapshot.dump());
  for (Client &client : clients) {
    send_to_client(client, serialized);
  }
  
  current_epoch++;
}
```

#### C.2 Client Snapshot Reception (Pseudocode)

```cpp
void client_handle_snapshot(const string &data) {
  json snapshot = json::parse(deserialize(data));
  
  if (snapshot["epoch"] <= last_epoch) {
    return;  // Discard old snapshot
  }
  last_epoch = snapshot["epoch"];
  
  for (const auto &entity_json : snapshot["entities"]) {
    uint32_t networkId = entity_json["networkId"];
    Entity *entity;
    
    if (networkId_map.contains(networkId)) {
      entity = networkId_map[networkId];
    } else {
      entity = world.createEntity();
      networkId_map[networkId] = entity;
    }
    
    entity->position = {entity_json["position"]["x"], entity_json["position"]["y"]};
    entity->velocity = {entity_json["velocity"]["x"], entity_json["velocity"]["y"]};
    entity->sprite = Sprite::from_json(entity_json["sprite"]);
    entity->health = entity_json["health"];
  }
}
```

### Appendix D: Known Limitations

#### D.1 No Reliability

All messages are best-effort UDP:
- Lost packets are not retransmitted
- No acknowledgments
- No guaranteed delivery

**Impact**: ~1-3% packet loss on typical networks results in ~16ms of stale data per lost snapshot.

#### D.2 No Client Prediction

Clients render exactly what the server sends:
- No extrapolation between snapshots
- No input prediction
- Pure server authority

**Impact**: Input feels delayed by RTT + 1 frame (~46ms for 30ms RTT).

#### D.3 No Delta Compression

Every snapshot is a full state dump:
- Bandwidth scales with entity count
- Redundant data sent every frame

**Impact**: Limits scalability to ~10-20 clients on typical networks.

#### D.4 No Connection State Management

Server does not track connection health:
- No timeout detection in protocol
- No heartbeat messages
- Dead clients remain in client list

**Impact**: Manual server restart needed for cleanup; no "player disconnected" notifications.

#### D.5 JSON Overhead

JSON is human-readable but inefficient:
- Text encoding vs binary
- Field names sent every message
- Parsing overhead

**Impact**: 2-5x larger messages vs binary protocol; ~10-20% CPU for JSON parsing.

---

### Appendix E: Revision History

| Version | Date | Author | Status | Changes |
|---------|------|--------|--------|---------|
| 1.0 | 2025-01-18 | R-Type Team | Informational | Initial RFC-compliant specification |

---

**Document Status**: Informational (Internet-Draft)  
**Feedback**: Submit issues to R-Type repository

© 2025 R-Type Development Team - EPITECH PROJECT
