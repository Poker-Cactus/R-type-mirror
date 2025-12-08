/*
** EPITECH PROJECT, 2025
** TypeMirror
** File description:
** ProcessInputMenu.cpp
*/

#include "../Menu.hpp"
#include "../../interface/KeyCodes.hpp"
#include <iostream>

void Menu::processLoading() {
    if (renderer->isKeyJustPressed(KeyCode::KEY_RETURN)) {
        currentState = MenuState::MAIN_MENU;
    }
}

void Menu::processMainMenu() {
    // Flèche bas - descendre dans le menu
    if (renderer->isKeyJustPressed(KeyCode::KEY_DOWN)) {
        currentMenuIndex = (currentMenuIndex + 1) % mainMenuItems.size();
    }
    // Flèche haut - monter dans le menu
    if (renderer->isKeyJustPressed(KeyCode::KEY_UP)) {
        currentMenuIndex = (currentMenuIndex - 1 + mainMenuItems.size()) % mainMenuItems.size();
    }
    if (renderer->isKeyJustPressed(KeyCode::KEY_RETURN)) {
        std::string selectedButton = mainMenuItems[currentMenuIndex];
        if (selectedButton == "Play") {
            currentState = MenuState::LOBBY;
        } else if (selectedButton == "Settings") {
            currentState = MenuState::SETTINGS;
        } else if (selectedButton == "Profile") {
            currentState = MenuState::PROFILE;
        } else {

        }
    }
}

void Menu::processProfile() {

}

void Menu::processLobby() {

}

void Menu::processSettings() {

}

void Menu::processBack() {
    if (renderer->isKeyJustPressed(KeyCode::KEY_BACKSPACE)) {
        if (currentState == MenuState::LOADING) {
            return;
        }
        if (currentState == MenuState::MAIN_MENU) {
            currentState = MenuState::LOADING;
            return;
        }
        currentState = MenuState::MAIN_MENU;
    }
}
