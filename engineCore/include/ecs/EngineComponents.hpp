/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** EngineComponents - Consolidated header for all ECS components
*/

#ifndef ENGINECOMPONENTS_HPP_
#define ENGINECOMPONENTS_HPP_

// IWYU pragma: begin_exports
// Core ECS
#include "ComponentSignature.hpp"
#include "Entity.hpp"
#include "World.hpp"

// Components
#include "components/Attraction.hpp"
#include "components/Ally.hpp"
#include "components/Collider.hpp"
#include "components/Follower.hpp"
#include "components/GunOffset.hpp"
#include "components/Health.hpp"
#include "components/Input.hpp"
#include "components/Networked.hpp"
#include "components/Pattern.hpp"
#include "components/PlayerId.hpp"
#include "components/Score.hpp"
#include "components/Sprite.hpp"
#include "components/Transform.hpp"
#include "components/Velocity.hpp"

// Core systems
#include "systems/MovementSystem.hpp"
// IWYU pragma: end_exports

#endif /* !ENGINECOMPONENTS_HPP_ */
