/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ECS Registry
*/

#pragma once

#include <cstddef>

namespace ECS {

    using Entity = std::size_t;

    class Registry {
    public:
        Registry() = default;
        ~Registry() = default;

        Entity createEntity();

        template <typename Component>
        void addComponent(Entity entity, Component&& component);

        template <typename Component>
        Component& getComponent(Entity entity);

        template <typename Component>
        bool hasComponent(Entity entity) const;

        template <typename Component>
        void removeComponent(Entity entity);

    private:
        Entity _nextEntityId = 0;
    };

}
