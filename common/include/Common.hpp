#ifndef COMMON_HPP_
#define COMMON_HPP_

#include <cstddef>
#include <nlohmann/json.hpp>

#ifdef _MSC_VER
#define UNUSED [[maybe_unused]]
#else
#define UNUSED __attribute__((unused))
#endif

constexpr std::size_t BUFFER_SIZE = 65535;

enum class Difficulty { EASY, MEDIUM, EXPERT };
enum class AIDifficulty { WEAK, MEDIUM, STRONG };


// JSON serialization for Difficulty enum
NLOHMANN_JSON_SERIALIZE_ENUM(
  Difficulty, {{Difficulty::EASY, "EASY"}, {Difficulty::MEDIUM, "MEDIUM"}, {Difficulty::EXPERT, "EXPERT"}})

// JSON serialization for AIDifficulty enum
NLOHMANN_JSON_SERIALIZE_ENUM(
  AIDifficulty, {{AIDifficulty::WEAK, "WEAK"}, {AIDifficulty::MEDIUM, "MEDIUM"}, {AIDifficulty::STRONG, "STRONG"}})

#endif // COMMON_HPP_
