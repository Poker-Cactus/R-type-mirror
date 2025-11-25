/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Input Manager Interface
*/

#pragma once

namespace Input {

    enum class Key {
        UP,
        DOWN,
        LEFT,
        RIGHT,
        SPACE,
        ESCAPE
    };

    class IInputManager {
    public:
        virtual ~IInputManager() = default;
        virtual bool isKeyPressed(Key key) const = 0;
        virtual void update() = 0;
    };

}
