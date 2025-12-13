/*
** EPITECH PROJECT, 2025
** TypeMirror
** File description:
** Game.cpp
*/

#include "Game.hpp"
#include <iostream>

Game::Game() : module(nullptr), renderer(nullptr), isRunning(false), currentState(GameState::PLAYING) {}

Game::~Game()
{
    shutdown();
}

bool Game::init()
{
    try {
    #ifdef __APPLE__
            module =
                std::make_unique<Module<IRenderer>>("./build/libs/sdl2_module.dylib", "createRenderer", "destroyRenderer");
    #else
            module =
                std::make_unique<Module<IRenderer>>("./build/libs/sdl2_module.so", "createRenderer", "destroyRenderer");
    #endif
        renderer = module->create();

        if (renderer == nullptr) {
            return false;
        }
        renderer->setWindowTitle("ChaD");
        renderer->setFullscreen(true);

        menu = std::make_unique<Menu>(renderer);
        menu->init();

        playingState = std::make_unique<PlayingState>(renderer);
        if (!playingState->init()) {
            std::cerr << "Failed to initialize playing state" << std::endl;
            return false;
        }

        isRunning = true;
        return true;
    } catch (const std::exception &e) {
        return false;
    }
}

void Game::run()
{
    if (!isRunning || renderer == nullptr) {
        return;
    }

    while (isRunning) {
        processInput();

        float dt = renderer->getDeltaTime();
        update(dt);

        render();
    }
}

void Game::shutdown()
{
    if (menu) {
        menu->cleanup();
        menu.reset();
    }

    if (playingState) {
        playingState->cleanup();
        playingState.reset();
    }

    if (module && renderer) {
        module->destroy(renderer);
        renderer = nullptr;
    }
    module.reset();
    isRunning = false;
}

void Game::processInput()
{
    if (!renderer->pollEvents()) {
        isRunning = false;
    }
    
    switch (currentState) {
    case GameState::MENU:
        if (menu) {
            menu->processInput();
        }
        break;
    case GameState::PLAYING:
        if (playingState) {
            playingState->processInput();
        }
        break;
    case GameState::PAUSED:
        // TODO: handle pause input
        break;
    }
}

void Game::update(float dt)
{
    switch (currentState) {
    case GameState::MENU:
        // Menu est statique, pas besoin d'update
        break;
    case GameState::PLAYING:
        if (playingState) {
            playingState->update(dt);
        }
        break;
    case GameState::PAUSED:
        // Pause ne met pas à jour le jeu
        break;
    }
}

void Game::render()
{
    renderer->clear({0, 0, 0, 255});

    switch (currentState) {
    case GameState::MENU:
        if (menu) {
            menu->render();
        }
        break;
    case GameState::PLAYING:
        if (playingState) {
            playingState->render();
        }
        break;
    case GameState::PAUSED:
        // TODO: render pause menu
        if (playingState) {
            playingState->render();
        }
        break;
    }

    renderer->present();
}

void Game::setState(GameState newState)
{
    currentState = newState;
}

Game::GameState Game::getState() const
{
    return currentState;
}
