# Code Nomenclature Guide

Professional coding standards and naming conventions for the R-Type project.

---

## Table of Contents

- [General Principles](#general-principles)
- [File Naming](#file-naming)
- [C++ Naming Conventions](#c-naming-conventions)
- [Code Organization](#code-organization)
- [Documentation Standards](#documentation-standards)
- [Git Conventions](#git-conventions)

---

## General Principles

### Core Rules

1. **Consistency** - Follow existing patterns in the codebase
2. **Clarity** - Names should be self-documenting
3. **Brevity** - Keep names concise but meaningful
4. **English Only** - All code, comments, and documentation in English

### Avoid

❌ Abbreviations (except well-known ones: `id`, `http`, `udp`)  
❌ Single-letter variables (except loop indices: `i`, `j`, `k`)  
❌ Hungarian notation  
❌ Cryptic names

---

## File Naming

### Source Files

| Type | Convention | Example |
|------|-----------|---------|
| **C++ Header** | PascalCase | `Registry.hpp`, `IInputManager.hpp` |
| **C++ Source** | PascalCase | `Registry.cpp`, `Components.cpp` |
| **Main files** | lowercase | `main.cpp`, `main.hpp` |

### Documentation Files

| Type | Convention | Example |
|------|-----------|---------|
| **Markdown** | SCREAMING_SNAKE_CASE | `README.md`, `HOW_TO_START.md` |
| **Config** | lowercase | `conanfile.txt`, `conan_profile` |

### Directory Structure

```
project/
├── include/           # Public headers
│   └── ModuleName/   # PascalCase module folders
├── src/              # Implementation files
├── tests/            # Test files (mirror src/ structure)
└── docs/             # Documentation
```

---

## C++ Naming Conventions

### Classes and Structs

**Format:** `PascalCase`

```cpp
class PlayerEntity { };
class UdpServer { };
struct GameState { };

// Interfaces: prefix with 'I'
class IInputManager { };
class IRenderManager { };
```

### Functions and Methods

**Format:** `camelCase`

```cpp
void updatePosition();
bool isAlive() const;
int getPlayerId() const;
void setVelocity(float x, float y);

// Boolean getters: use 'is', 'has', 'can'
bool isActive() const;
bool hasComponent() const;
bool canMove() const;
```

### Variables

**Format:** `camelCase`

```cpp
// Local variables
int playerScore = 0;
float deltaTime = 0.016f;
std::string userName;

// Member variables: prefix with 'm_'
class Player {
private:
    int m_health;
    float m_speed;
    std::string m_name;
};

// Static variables: prefix with 's_'
static int s_instanceCount;

// Global variables: prefix with 'g_' (avoid when possible)
int g_globalConfig;
```

### Constants and Enums

**Format:** `SCREAMING_SNAKE_CASE`

```cpp
// Constants
const int MAX_PLAYERS = 4;
constexpr float PI = 3.14159f;
const std::string DEFAULT_SERVER_IP = "127.0.0.1";

// Enumerations
enum class ComponentType {
    POSITION,
    VELOCITY,
    SPRITE,
    COLLIDER
};

enum class ConnectionState {
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    ERROR
};
```

### Namespaces

**Format:** `lowercase`

```cpp
namespace rtype {
    namespace network {
        class UdpServer { };
    }
    namespace ecs {
        class Registry { };
    }
}

// Usage
using namespace rtype::network;
```

### Template Parameters

**Format:** `PascalCase`

```cpp
template<typename T>
class Container { };

template<typename Component>
void addComponent(Component&& component);

// Common names
template<typename T, typename U>
template<typename Key, typename Value>
template<typename InputIt, typename OutputIt>
```

### Type Aliases

**Format:** `PascalCase`

```cpp
using EntityId = std::uint32_t;
using ComponentList = std::vector<Component>;
using NetworkCallback = std::function<void(const Packet&)>;
```

---

## Code Organization

### Header Files

```cpp
/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** ClassName.hpp - Brief description
*/

#ifndef CLASS_NAME_HPP_
#define CLASS_NAME_HPP_

// System includes
#include <iostream>
#include <vector>

// External library includes
#include <asio.hpp>

// Project includes
#include "OtherClass.hpp"

namespace rtype {

/**
 * @brief Brief class description
 * 
 * Detailed description of the class purpose,
 * responsibilities, and usage examples.
 */
class ClassName {
public:
    // Public types
    using Callback = std::function<void()>;

    // Constructors/Destructor
    ClassName();
    explicit ClassName(int value);
    ~ClassName();

    // Public methods
    void publicMethod();

private:
    // Private methods
    void privateMethod();

    // Member variables
    int m_value;
    std::string m_name;
};

} // namespace rtype

#endif // CLASS_NAME_HPP_
```

### Source Files

```cpp
/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** ClassName.cpp
*/

#include "ClassName.hpp"

namespace rtype {

ClassName::ClassName()
    : m_value(0)
    , m_name("default")
{
    // Constructor implementation
}

void ClassName::publicMethod()
{
    // Method implementation
}

} // namespace rtype
```

### Code Style

**Indentation:** 4 spaces (no tabs)  
**Line Length:** Max 100 characters  
**Braces:** K&R style (opening brace on same line)

```cpp
// Good
if (condition) {
    doSomething();
} else {
    doSomethingElse();
}

// Function definitions
void function()
{
    // Implementation
}

// Classes
class MyClass {
public:
    void method();
};
```

---

## Documentation Standards

### Comments

```cpp
// Single-line comment for brief explanations

/*
 * Multi-line comment for detailed
 * explanations or temporary notes
 */

/**
 * @brief Doxygen-style documentation for public APIs
 * 
 * @param input Description of parameter
 * @return Description of return value
 * @throws ExceptionType When this exception is thrown
 */
```

### Function Documentation

```cpp
/**
 * @brief Sends a packet to the specified endpoint
 * 
 * Serializes the packet data and transmits it asynchronously
 * over the UDP socket to the target endpoint.
 * 
 * @param packet The data packet to send
 * @param endpoint Target network endpoint
 * @return true if send was initiated successfully
 * @throws NetworkException if socket is closed
 */
bool sendPacket(const Packet& packet, const Endpoint& endpoint);
```

### TODO Comments

```cpp
// TODO(username): Add error handling here
// FIXME(username): Memory leak in destruction
// NOTE: This is a workaround for SDL2 bug #1234
// OPTIMIZE: Consider using unordered_map for O(1) lookup
```

---

## Git Conventions

### Branch Naming

**Format:** `type/short-description`

```bash
feat/udp-server          # New feature
fix/memory-leak          # Bug fix
refactor/ecs-system      # Code refactoring
docs/api-documentation   # Documentation
test/unit-tests          # Tests
chore/update-deps        # Maintenance
```

### Commit Messages

**Format:** `[TYPE] Brief description`

```bash
[ADD] Implement UDP server with ASIO
[FIX] Resolve memory leak in Registry destructor
[REFACTOR] Extract network logic into separate class
[DOCS] Add API documentation for ECS components
[TEST] Add unit tests for collision system
[CHORE] Update Conan dependencies to latest versions
```

**Types:**
- `[ADD]` - New feature or functionality
- `[FIX]` - Bug fix
- `[REFACTOR]` - Code restructuring without behavior change
- `[DOCS]` - Documentation changes
- `[TEST]` - Adding or updating tests
- `[CHORE]` - Maintenance tasks (deps, build, etc.)
- `[PERF]` - Performance improvements
- `[STYLE]` - Code style/formatting changes

### Commit Body

```
[ADD] Implement player movement system

- Add keyboard input handling
- Implement velocity-based movement
- Add collision detection with boundaries
- Update player sprite position based on velocity

Closes #42
```

---

## Examples

### Complete Class Example

```cpp
/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** UdpServer.hpp - Asynchronous UDP server implementation
*/

#ifndef UDP_SERVER_HPP_
#define UDP_SERVER_HPP_

#include <asio.hpp>
#include <functional>
#include <memory>
#include <string>

namespace rtype::network {

/**
 * @brief Asynchronous UDP server for game networking
 * 
 * Handles incoming UDP packets and dispatches them to
 * registered callbacks. Supports multiple concurrent connections.
 */
class UdpServer {
public:
    using PacketCallback = std::function<void(const std::string&)>;
    using ErrorCallback = std::function<void(const std::error_code&)>;

    /**
     * @brief Constructs UDP server on specified port
     * 
     * @param ioContext ASIO IO context for async operations
     * @param port Server listening port
     */
    UdpServer(asio::io_context& ioContext, uint16_t port);
    
    ~UdpServer();

    /**
     * @brief Registers callback for incoming packets
     * 
     * @param callback Function called when packet arrives
     */
    void onPacketReceived(PacketCallback callback);

    /**
     * @brief Sends data to specified endpoint
     * 
     * @param data Packet data to send
     * @param endpoint Target network endpoint
     * @return true if send initiated successfully
     */
    bool send(const std::string& data, const asio::ip::udp::endpoint& endpoint);

private:
    void startReceive();
    void handleReceive(const std::error_code& error, std::size_t bytesTransferred);

    asio::ip::udp::socket m_socket;
    asio::ip::udp::endpoint m_remoteEndpoint;
    std::array<char, 1024> m_receiveBuffer;
    PacketCallback m_packetCallback;
};

} // namespace rtype::network

#endif // UDP_SERVER_HPP_
```

---

## Tools and Automation

### Recommended Tools

- **Linter:** `clang-tidy`
- **Formatter:** `clang-format` (configured in `.clang-format`)
- **Static Analysis:** `cppcheck`
- **Documentation:** `Doxygen`

### Pre-commit Checks

```bash
# Format code
clang-format -i src/**/*.cpp include/**/*.hpp

# Run linter
clang-tidy src/**/*.cpp

# Run tests
ctest --output-on-failure
```

---

## References

- [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)
- [Modern C++ Best Practices](https://github.com/cpp-best-practices/cppbestpractices)

---

**Last Updated:** November 2025  
**Maintained By:** R-Type Development Team
