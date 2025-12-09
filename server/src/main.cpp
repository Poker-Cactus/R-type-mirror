/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** main.cpp
*/

#include "../../network/include/AsioServer.hpp"
#include "../include/Server.hpp"
#include <csignal>
#include <iostream>

int main()
{
  try {
    Server server(std::make_shared<AsioServer>(4241));
  } catch (const std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }
  return 0;
}
