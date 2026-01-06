/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** ProfileMenu.hpp
*/

#pragma once
#include "../../../interface/IRenderer.hpp"

class ProfileMenu
{
public:
  ProfileMenu() = default;
  ~ProfileMenu(){};
  void init(IRenderer *renderer);
  void render(int winWidth, int winHeight, IRenderer *renderer);
  void process(IRenderer *renderer);

private:
};