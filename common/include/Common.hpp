/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Common.hpp - Common constants and macros
*/

#ifndef COMMON_HPP_
#define COMMON_HPP_

#include <cstddef>

#ifdef _MSC_VER
#define UNUSED [[maybe_unused]]
#else
#define UNUSED __attribute__((unused))
#endif

constexpr std::size_t BUFFER_SIZE = 65535;

enum class Difficulty { EASY, MEDIUM, EXPERT };

#endif // COMMON_HPP_
