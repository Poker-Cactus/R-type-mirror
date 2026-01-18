/**
 * @file WorldLobbyRegistry.cpp
 * @brief Registry to map ECS worlds to owning lobbies.
 */

#include "WorldLobbyRegistry.hpp"
#include "Lobby.hpp"
#include <unordered_map>

// Simple global registry. Not thread-safe but server runs single-threaded game loop.
static std::unordered_map<ecs::World *, Lobby *> g_worldLobbyMap;

void registerWorldLobbyMapping(ecs::World *world, Lobby *lobby)
{
  if (world == nullptr) {
    return;
  }
  g_worldLobbyMap[world] = lobby;
}

void unregisterWorldLobbyMapping(ecs::World *world)
{
  if (world == nullptr) {
    return;
  }
  g_worldLobbyMap.erase(world);
}

Lobby *getLobbyForWorld(ecs::World *world)
{
  if (world == nullptr) {
    return nullptr;
  }
  auto it = g_worldLobbyMap.find(world);
  return it != g_worldLobbyMap.end() ? it->second : nullptr;
}
