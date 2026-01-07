/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** ProfileMenu.hpp
*/

#pragma once
#include "../../../interface/IRenderer.hpp"
#include <memory>

class ProfileMenu
{
  public:
    ProfileMenu(std::shared_ptr<IRenderer> m_renderer);
    ~ProfileMenu();

    void init();
    void render(int winWidth, int winHeight);
    void process();

  private:
    std::shared_ptr<IRenderer> m_renderer;
};
