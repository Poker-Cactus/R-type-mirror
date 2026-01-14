/*
** EPITECH PROJECT, 2025
** TypeMirror
** File description:
** MainMenu.cpp
*/

#include "MainMenu.hpp"
#include "../../../include/Settings.hpp"
#include "../../../interface/Geometry.hpp"
#include "../../../interface/KeyCodes.hpp"
#include <iostream>
#include <filesystem>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <spawn.h>
#include <sys/wait.h>
#include <signal.h>
#include <vector>
extern char **environ;
#endif

namespace fs = std::filesystem;

// Track Asset Editor PID globally
static pid_t g_assetEditorPid = -1;

// Helper function to find and launch the Asset Editor
static bool LaunchAssetEditor() {
    // Try to find the asset editor binary
    std::vector<std::string> possiblePaths = {
        "./assetEditor",                           // Same directory
        "./build/assetEditor/assetEditor",         // Build directory
        "../assetEditor/assetEditor",              // Relative from client
        "../build/assetEditor/assetEditor",        // Relative build
        "assetEditor/assetEditor",                 // From project root
        "build/assetEditor/assetEditor"            // From project root build
    };
    
#ifdef _WIN32
    // Add .exe extension for Windows
    for (auto& path : possiblePaths) {
        path += ".exe";
    }
#endif
    
    std::string editorPath;
    for (const auto& path : possiblePaths) {
        if (fs::exists(path) && fs::is_regular_file(path)) {
            editorPath = fs::absolute(path).string();
            break;
        }
    }
    
    if (editorPath.empty()) {
        std::cerr << "[MainMenu] Asset Editor not found! Please compile the assetEditor target." << std::endl;
        return false;
    }
    
    std::cout << "[MainMenu] Launching Asset Editor: " << editorPath << std::endl;
    
#ifdef _WIN32
    // Windows: Use ShellExecute for non-blocking launch
    HINSTANCE result = ShellExecuteA(NULL, "open", editorPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
    if ((intptr_t)result <= 32) {
        std::cerr << "[MainMenu] Failed to launch Asset Editor (error: " << (intptr_t)result << ")" << std::endl;
        return false;
    }
    return true;
#else
    // Unix (Linux/macOS): Fork and exec
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        char* args[] = {const_cast<char*>(editorPath.c_str()), nullptr};
        execv(editorPath.c_str(), args);
        // If execv fails
        std::cerr << "[MainMenu] Failed to exec Asset Editor" << std::endl;
        _exit(1);
    } else if (pid < 0) {
        std::cerr << "[MainMenu] Failed to fork for Asset Editor" << std::endl;
        return false;
    }
    // Store PID to kill on exit
    g_assetEditorPid = pid;
    std::cout << "[MainMenu] Asset Editor launched with PID: " << pid << std::endl;
    // Parent continues immediately (non-blocking)
    return true;
#endif
}

MainMenu::MainMenu(std::shared_ptr<IRenderer> renderer)
    : m_renderer(renderer), font(nullptr), planet(nullptr), currentMenuIndex(0)
{
  // mainMenuItems is initialized in the header
}

MainMenu::~MainMenu()
{
#ifndef _WIN32
    // Kill Asset Editor if it's still running
    if (g_assetEditorPid > 0) {
        std::cout << "[MainMenu] Terminating Asset Editor (PID: " << g_assetEditorPid << ")" << std::endl;
        kill(g_assetEditorPid, SIGTERM);
        // Give it a moment to terminate gracefully
        usleep(100000); // 100ms
        // Force kill if still alive
        kill(g_assetEditorPid, SIGKILL);
        waitpid(g_assetEditorPid, nullptr, WNOHANG);
        g_assetEditorPid = -1;
    }
#endif
  if (font != nullptr && m_renderer != nullptr) {
    m_renderer->freeFont(font);
  }
  if (planet != nullptr && m_renderer != nullptr) {
    m_renderer->freeTexture(planet);
  }
  if (clickedSound != nullptr && m_renderer != nullptr) {
    m_renderer->freeSound(clickedSound);
  }
  if (hoverSound != nullptr && m_renderer != nullptr) {
    m_renderer->freeSound(hoverSound);
  }
}

void MainMenu::init()
{
  try {
    const int menuFontSize = 24;
    planet = m_renderer->loadTexture("client/assets/moon-pack/moon1.png");
    font = m_renderer->loadFont("client/assets/font.opf/r-type.otf", menuFontSize);
    clickedSound = m_renderer->loadSound("client/assets/audios/Retro3.mp3");
    hoverSound = m_renderer->loadSound("client/assets/Sounds/Hovering3.wav");
  } catch (const std::exception &e) {
    planet = nullptr;
    font = nullptr;
  }
}

void MainMenu::render(int winWidth, int winHeight)
{
  if (font == nullptr) {
    return;
  }
  for (size_t i = 0; i < mainMenuItems.size(); i++) {
    int textWidth = 0;
    int textHeight = 0;
    m_renderer->getTextSize(font, mainMenuItems[i], textWidth, textHeight);

    int x = (winWidth - textWidth) / 2;
    int y = (winHeight / 2) + (static_cast<int>(i) * 60) - 90;

    Color color = (i == static_cast<size_t>(currentMenuIndex)) ? Color{4, 196, 199, 255} : Color{255, 255, 255, 255};
    m_renderer->drawText(font, mainMenuItems[i], x, y, color);
  }
}

void MainMenu::process(MenuState *currentState, Settings &settings)
{
  (void)settings; // Unused parameter
  // Flèche bas - descendre dans le menu
  if (m_renderer->isKeyJustPressed(KeyCode::KEY_DOWN)) {
    m_renderer->playSound(hoverSound);
    currentMenuIndex = (currentMenuIndex + 1) % mainMenuItems.size();
  }
  // Flèche haut - monter dans le menu
  if (m_renderer->isKeyJustPressed(KeyCode::KEY_UP)) {
    m_renderer->playSound(hoverSound);
    currentMenuIndex = (currentMenuIndex - 1 + mainMenuItems.size()) % mainMenuItems.size();
  }
  if (m_renderer->isKeyJustPressed(KeyCode::KEY_RETURN)) {
    m_renderer->playSound(clickedSound);
    std::string selectedButton = mainMenuItems[currentMenuIndex];
    std::cout << "[MainMenu] Enter pressed on: " << selectedButton << '\n';
    if (selectedButton == "Play") {
      std::cout << "[MainMenu] Changing state to LOBBY" << '\n';
      *currentState = MenuState::LOBBY;
    } else if (selectedButton == "Settings") {
      *currentState = MenuState::SETTINGS;
    } else if (selectedButton == "Profile") {
      *currentState = MenuState::PROFILE;
    } else if (selectedButton == "Asset Editor") {
      if (!LaunchAssetEditor()) {
        std::cerr << "[MainMenu] Could not launch Asset Editor - binary not found" << std::endl;
      }
    } else {
      *currentState = MenuState::EXIT;
    }
  }
}
