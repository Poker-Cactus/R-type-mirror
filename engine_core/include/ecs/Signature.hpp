/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Signature.hpp
*/

#ifndef ECS_SIGNATURE_HPP_
#define ECS_SIGNATURE_HPP_

#include <bitset>

// Nombre maximum de types de composants différents
constexpr std::size_t MAX_COMPONENTS = 64;

// Signature = bitset qui indique quels composants une entité possède
using Signature = std::bitset<MAX_COMPONENTS>;

#endif // ECS_SIGNATURE_HPP_
