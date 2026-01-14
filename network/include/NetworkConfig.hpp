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
constexpr float PLAYER_COLLIDER_SIZE = 32.0F;
constexpr int PLAYER_SPRITE_WIDTH = 140;
constexpr int PLAYER_SPRITE_HEIGHT = 60;

// AI spawn configuration
constexpr float AI_SPAWN_X = 150.0F;
constexpr float AI_SPAWN_Y = 350.0F;
constexpr int AI_MAX_HP = 100;
constexpr float AI_COLLIDER_SIZE = 32.0F;
constexpr int AI_SPRITE_WIDTH = 140;
constexpr int AI_SPRITE_HEIGHT = 60;
} // namespace NetworkConfig

#endif /* !NETWORKCONFIG_HPP_ */
