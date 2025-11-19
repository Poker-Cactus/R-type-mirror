/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Game Components
*/

#pragma once

namespace Components {

    struct Position {
        float x;
        float y;
    };

    struct Velocity {
        float vx;
        float vy;
    };

    struct Health {
        int current;
        int max;
    };

    struct Sprite {
        int textureId;
        int width;
        int height;
    };

}
