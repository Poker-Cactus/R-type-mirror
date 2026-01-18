/**
 * @file WorldLobbyRegistry.hpp
 * @brief Registry to map ECS worlds to owning lobbies.
 */

#pragma once

#include "../../engineCore/include/ecs/World.hpp"

class Lobby;

/**
 * @brief Register a mapping between an ECS world and its lobby.
 * @param world ECS world instance.
 * @param lobby Owning lobby instance.
 */
void registerWorldLobbyMapping(ecs::World *world, Lobby *lobby);
/**
 * @brief Unregister a mapping for an ECS world.
 * @param world ECS world instance.
 */
void unregisterWorldLobbyMapping(ecs::World *world);
/**
 * @brief Look up the owning lobby for an ECS world.
 * @param world ECS world instance.
 * @return Owning lobby or nullptr.
 */
Lobby *getLobbyForWorld(ecs::World *world);
