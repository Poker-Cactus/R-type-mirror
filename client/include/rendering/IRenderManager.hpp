/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Rendering Manager Interface
*/

#pragma once

namespace Rendering {

    class IRenderManager {
    public:
        virtual ~IRenderManager() = default;
        virtual void render() = 0;
        virtual void clear() = 0;
    };

}
