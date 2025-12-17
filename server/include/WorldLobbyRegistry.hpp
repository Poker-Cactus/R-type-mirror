#pragma once

#include "../../engineCore/include/ecs/World.hpp"

class Lobby;

// Register/unregister mapping between an ECS world and its owning Lobby
void registerWorldLobbyMapping(ecs::World *world, Lobby *lobby);
void unregisterWorldLobbyMapping(ecs::World *world);
Lobby *getLobbyForWorld(ecs::World *world);
