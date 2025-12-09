/*
** EPITECH PROJECT, 2025
** TypeMirror
** File description:
** LoadingScreen.cpp - Implémentation de la classe interne Menu::LoadingScreen
*/

#include "../Menu.hpp"
#include <cmath>

Menu::LoadingScreen::LoadingScreen(IRenderer *renderer, void *font) : renderer(renderer), font(font) {}

void Menu::LoadingScreen::start()
{
    active = true;
    timer = 0.0f;
    rotation = 0.0f;
}

void Menu::LoadingScreen::stop()
{
    active = false;
    timer = 0.0f;
    rotation = 0.0f;
}

bool Menu::LoadingScreen::update(int winWidth, int winHeight)
{
    if (!active) {
        return true; // Déjà terminé
    }

    timer += renderer->getDeltaTime();

    if (timer >= duration) {
        stop();
        return true; // Loading terminé
    }

    // Fond noir
    renderer->drawRect(0, 0, winWidth, winHeight, {0, 0, 0, 255});

    // Rotation du texte (360 degrés par seconde)
    rotation += renderer->getDeltaTime() * 360.0f;
    if (rotation >= 360.0f) {
        rotation -= 360.0f;
    }

    // Afficher "Loading..." en bas à droite
    if (font != nullptr) {
        std::string text = "Loading...";
        int textWidth = 0;
        int textHeight = 0;
        renderer->getTextSize(font, text, textWidth, textHeight);

        int x = winWidth - textWidth - 50;
        int y = winHeight - textHeight - 30;

        // Calculer l'opacité basée sur la rotation (effet de clignotement)
        float opacity = (std::sin(rotation * 3.14159f / 180.0f) + 1.0f) / 2.0f;
        int alpha = static_cast<int>(100 + opacity * 155);

        renderer->drawText(font, text, x, y, {255, 255, 255, static_cast<unsigned char>(alpha)});
    }

    return false; // Loading en cours
}

bool Menu::LoadingScreen::isActive() const
{
    return active;
}

void Menu::LoadingScreen::setDuration(float newDuration)
{
    duration = newDuration;
}
