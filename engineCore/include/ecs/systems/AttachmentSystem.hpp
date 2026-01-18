/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** AttachmentSystem - Updates positions of attached entities
*/

#ifndef ECS_ATTACHMENT_SYSTEM_HPP_
#define ECS_ATTACHMENT_SYSTEM_HPP_

#include "../Entity.hpp"
#include "../ISystem.hpp"
#include "../World.hpp"
#include "../components/Attachment.hpp"
#include "../components/Transform.hpp"
#include <vector>

namespace ecs
{

/**
 * @brief System that updates positions of attached entities (like turrets)
 * Attached entities maintain their relative position to their parent entity
 */
class AttachmentSystem : public ISystem
{
public:
  void update(World &world, float deltaTime) override
  {
    (void)deltaTime;  // Suppress unused parameter warning
    std::vector<Entity> entities;
    world.getEntitiesWithSignature(getSignature(), entities);

    for (auto entity : entities) {
      auto &attachment = world.getComponent<Attachment>(entity);
      auto &transform = world.getComponent<Transform>(entity);

      // Check if parent still exists
      if (!world.isAlive(attachment.parentId)) {
        // Parent is dead, destroy the attachment
        world.destroyEntity(entity);
        continue;
      }

      // Update position based on parent's position + offset
      auto &parentTransform = world.getComponent<Transform>(attachment.parentId);
      transform.x = parentTransform.x + attachment.offsetX;
      transform.y = parentTransform.y + attachment.offsetY;

      // Optionally inherit parent's rotation if needed
      // transform.rotation = parentTransform.rotation;
    }
  }

  [[nodiscard]] ComponentSignature getSignature() const override
  {
    ComponentSignature sig;
    sig.set(getComponentId<Attachment>());
    sig.set(getComponentId<Transform>());
    return sig;
  }
};

} // namespace ecs

#endif // ECS_ATTACHMENT_SYSTEM_HPP_
