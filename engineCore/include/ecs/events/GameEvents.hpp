/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** GameEvents.hpp - All game events used by systems
*/

#ifndef ECS_GAME_EVENTS_HPP_
#define ECS_GAME_EVENTS_HPP_

#include "../Entity.hpp"
#include "IEvent.hpp"
#include <cstdint>
#include <vector>

namespace ecs
{

/**
 * @brief Event triggered when two entities collide
 */
struct CollisionEvent : public IEvent {
  Entity entityA;
  Entity entityB;
  float impactForce;

  CollisionEvent(Entity a, Entity b, float force = 1.0F) : entityA(a), entityB(b), impactForce(force) {}
};

/**
 * @brief Event triggered when an entity should take damage
 */
struct DamageEvent : public IEvent {
  Entity target;
  Entity source;
  int damageAmount;

  DamageEvent(Entity t, Entity s, int amount) : target(t), source(s), damageAmount(amount) {}
};

/**
 * @brief Event triggered when an entity dies
 */
struct DeathEvent : public IEvent {
  Entity entity;
  Entity killer;

  DeathEvent(Entity e, Entity k = 0) : entity(e), killer(k) {}
};

/**
 * @brief Event triggered when player wants to shoot
 */
struct ShootEvent : public IEvent {
  Entity shooter;
  float directionX;
  float directionY;

  ShootEvent(Entity s, float dx = 1.0f, float dy = 0.0f) : shooter(s), directionX(dx), directionY(dy) {}
};

/**
 * @brief Event triggered to spawn a new entity
 */
struct SpawnEntityEvent : public IEvent {
  enum class EntityType { ENEMY, PROJECTILE, POWERUP, EXPLOSION, CHARGED_PROJECTILE };

  EntityType type;
  float x;
  float y;
  Entity spawner;

  SpawnEntityEvent(EntityType t, float posX, float posY, Entity s = 0) : type(t), x(posX), y(posY), spawner(s) {}
};

/**
 * @brief Event triggered when enemy AI makes a decision
 */
struct EnemyAIEvent : public IEvent {
  enum class Action { MOVE, SHOOT, SPAWN_ALLY, FLEE };

  Entity enemy;
  Action action;
  float targetX;
  float targetY;

  EnemyAIEvent(Entity e, Action a, float x = 0.0f, float y = 0.0f) : enemy(e), action(a), targetX(x), targetY(y) {}
};

/**
 * @brief Event triggered when network snapshot is received
 */
struct SnapshotReceivedEvent : public IEvent {
  uint32_t tick;
  std::vector<Entity> entities;

  SnapshotReceivedEvent(uint32_t t) : tick(t) {}
};

/**
 * @brief Event triggered to send network snapshot
 */
struct SendSnapshotEvent : public IEvent {
  uint32_t clientId;

  SendSnapshotEvent(uint32_t id) : clientId(id) {}
};

/**
 * @brief Event triggered when player input is processed
 */
struct PlayerInputEvent : public IEvent {
  Entity player;
  bool up;
  bool down;
  bool left;
  bool right;
  bool shoot;

  PlayerInputEvent(Entity p) : player(p), up(false), down(false), left(false), right(false), shoot(false) {}
};

/**
 * @brief Event triggered when score changes
 */
struct ScoreEvent : public IEvent {
  Entity player;
  int points;

  ScoreEvent(Entity p, int pts) : player(p), points(pts) {}
};

} // namespace ecs

#endif // ECS_GAME_EVENTS_HPP_
