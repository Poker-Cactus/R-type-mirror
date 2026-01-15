/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** IColorBlindSupport.hpp
*/

#pragma once

#include "../include/ColorBlindFilter.hpp"

class IColorBlindSupport
{
public:
  virtual ~IColorBlindSupport() = default;

  virtual void setColorBlindMode(ColorBlindMode mode) = 0;

  virtual ColorBlindMode getColorBlindMode() const = 0;
};
