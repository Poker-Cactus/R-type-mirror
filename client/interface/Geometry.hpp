/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** Geometry.hpp
*/

#pragma once

/// Simple 2D point with integer coordinates
struct Point {
  int x;
  int y;
};

/// Rectangle defined by position and size
struct Rect {
  int x;
  int y;
  int width;
  int height;
};

/// Circle defined by center point and radius
struct Circle {
  int centerX;
  int centerY;
  int radius;
};
