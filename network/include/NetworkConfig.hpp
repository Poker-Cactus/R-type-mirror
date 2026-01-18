/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** NetworkConfig - Network configuration constants
*/

#ifndef NETWORKCONFIG_HPP_
#define NETWORKCONFIG_HPP_

namespace NetworkConfig
{
// Buffer configuration
constexpr int RECEIVE_BUFFER_SIZE = 1024;
constexpr int RECEIVE_BUFFER_SIZE_KB = 1024;
constexpr int RECEIVE_BUFFER_MULTIPLIER = 8;

// Player spawn configuration (reuse from GameConfig if possible, or define here)
constexpr float PLAYER_GUN_OFFSET = 20.0F;
constexpr float PLAYER_SPAWN_X = 100.0F;
constexpr float PLAYER_SPAWN_Y = 300.0F;
constexpr float PLAYER_SPAWN_Y_OFFSET = 50.0F;
constexpr int PLAYER_MAX_HP = 100;
constexpr float PLAYER_COLLIDER_WIDTH = 33.0F;
constexpr float PLAYER_COLLIDER_HEIGHT = 17.0F;
constexpr int PLAYER_SPRITE_WIDTH = 140;
constexpr int PLAYER_SPRITE_HEIGHT = 60;

// Ally spawn configuration
constexpr float ALLY_SPAWN_X = 150.0F;
constexpr float ALLY_SPAWN_Y = 350.0F;
constexpr int ALLY_MAX_HP = 100;
// Ally collider dimensions (box)
constexpr float ALLY_COLLIDER_WIDTH = 32.0F;
constexpr float ALLY_COLLIDER_HEIGHT = 32.0F;
constexpr float ALLY_COLLIDER_SIZE = 32.0F; // compatibility alias
constexpr int ALLY_SPRITE_WIDTH = 140;
constexpr int ALLY_SPRITE_HEIGHT = 60;
} // namespace NetworkConfig

#endif /* !NETWORKCONFIG_HPP_ */
