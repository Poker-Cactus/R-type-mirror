/*
** EPITECH PROJECT, 2025
** TypeMirror
** File description:
** ProfileMenu.cpp
*/

#include "ProfileMenu.hpp"
#include "../../../../common/include/Common.hpp"
#include <cmath>

ProfileMenu::ProfileMenu(std::shared_ptr<IRenderer> renderer) : m_renderer(std::move(renderer)) {}

ProfileMenu::~ProfileMenu() {}

void ProfileMenu::init()
{
  try {
  } catch (const std::exception &e) {
  }
}

void ProfileMenu::render(UNUSED int winWidth, UNUSED int winHeight)
{
  // TODO: Afficher le lobby
}

void ProfileMenu::process() {}
