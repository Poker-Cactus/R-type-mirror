/*
** EPITECH PROJECT, 2025
** TypeMirror
** File description:
** Game.cpp
*/

#include "Game.hpp"
#include <iostream>

Game::Game() : module(nullptr), renderer(nullptr), isRunning(false), currentState(GameState::MENU) {}

Game::~Game()
{
    shutdown();
}

bool Game::init()
{
    try {
        module =
            std::make_unique<Module<IRenderer>>("./build/libs/sdl2_module.so", "createRenderer", "destroyRenderer");
        renderer = module->create();

        if (renderer == nullptr) {
            return false;
        }
        renderer->setWindowTitle("ChaD");
        renderer->setFullscreen(true);

        menu = std::make_unique<Menu>(renderer);
        menu->init();

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
    if (this->menu && this->currentState == GameState::MENU)
        this->menu->processInput();
}

void Game::update(float dt)
{
    (void)dt;
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
        // TODO: render game
        break;
    case GameState::PAUSED:
        // TODO: render pause menu
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