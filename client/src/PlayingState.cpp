/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** PlayingState.cpp
*/

#include "PlayingState.hpp"
#include "../../engineCore/include/ecs/components/Transform.hpp"
#include <iostream>

PlayingState::PlayingState(IRenderer *renderer, const std::shared_ptr<ecs::World> &world)
    : renderer(renderer), world(world), background(nullptr)
{
}

PlayingState::~PlayingState()
{
    cleanup();
}

bool PlayingState::init()
{
    if (!renderer) {
        std::cerr << "PlayingState: Renderer is null" << std::endl;
        return false;
    }

    // Initialiser le background parallaxe
    background = std::make_unique<ParallaxBackground>(renderer);
    if (!background->init()) {
        std::cerr << "PlayingState: Failed to initialize parallax background" << std::endl;
        return false;
    }

    std::cout << "PlayingState: Initialized successfully" << std::endl;
    
    // TODO: Initialiser les autres systèmes de jeu
    // - Créer le joueur
    // - Initialiser le système de spawn
    // - Charger le niveau
    
    return true;
}

void PlayingState::update(float dt)
{
    // Mettre à jour le background
    if (background) {
        background->update(dt);
    }

    // TODO: Mettre à jour les systèmes de jeu
    // - Mettre à jour les entités
    // - Vérifier les collisions
    // - Spawn des ennemis
    // - Mettre à jour l'IA
    // - Mettre à jour les projectiles
}

void PlayingState::render()
{
    // Dessiner le background en premier
    if (background) {
        background->render();
    }

    // Minimal prototype rendering: draw all entities that have a Transform.
    if (!world || !renderer) {
        return;
    }

    ecs::ComponentSignature sig;
    sig.set(ecs::getComponentId<ecs::Transform>());

    std::vector<ecs::Entity> entities;
    world->getEntitiesWithSignature(sig, entities);

    for (auto entity : entities) {
        const auto &t = world->getComponent<ecs::Transform>(entity);
        renderer->drawRect(static_cast<int>(t.x), static_cast<int>(t.y), 32, 32, {255, 0, 0, 255});
    }
}

void PlayingState::processInput()
{
    // TODO: Gérer les entrées du joueur
    // - Mouvement du vaisseau
    // - Tir
    // - Pause (Échap)
}

void PlayingState::cleanup()
{
    if (background) {
        background->cleanup();
        background.reset();
    }

    // TODO: Nettoyer les autres ressources
    // - Détruire les entités
    // - Libérer les textures
    // - Nettoyer les systèmes
    
    std::cout << "PlayingState: Cleaned up" << std::endl;
}
