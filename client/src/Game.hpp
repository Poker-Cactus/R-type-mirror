#pragma once
#include "../ModuleLoader.hpp"
#include "../interface/IRenderer.hpp"
#include "Menu.hpp"
#include "PlayingState.hpp"
#include <memory>

class Game
{
  public:
    enum class GameState { MENU, PLAYING, PAUSED };

    Game();
    ~Game();

    bool init();
    void run();
    void shutdown();

    void setState(GameState newState);
    GameState getState() const;

  private:
    void processInput();
    void update(float dt);
    void render();

    std::unique_ptr<Module<IRenderer>> module;
    IRenderer *renderer = nullptr;

    bool isRunning = false;
    GameState currentState = GameState::PLAYING;
    std::unique_ptr<Menu> menu;
    std::unique_ptr<PlayingState> playingState;
};
