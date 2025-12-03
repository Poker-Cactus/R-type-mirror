/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** World.cpp
*/

#include "ecs/World.hpp"

namespace ecs
{
void World::getEntitiesWithSignature(const ComponentSignature &signature, std::vector<Entity> &entities)
{
  entities.clear();

  // This would require tracking all active entities
  // For now, this is a placeholder - you'll need to implement entity tracking
  // in your World or ComponentManager to iterate through all entities

  // Example implementation if you had a list of all entity IDs:
  // for (Entity entity : allEntities) {
  //     const ComponentSignature& entitySig = getEntitySignature(entity);
  //     if ((entitySig & signature) == signature) {
  //         entities.push_back(entity);
  //     }
  // }
}
} // namespace ecs