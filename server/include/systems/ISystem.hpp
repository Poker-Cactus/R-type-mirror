/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Game Systems Interface
*/

#pragma once

namespace Systems {

    class ISystem {
    public:
        virtual ~ISystem() = default;
        virtual void update(float deltaTime) = 0;
    };

}
